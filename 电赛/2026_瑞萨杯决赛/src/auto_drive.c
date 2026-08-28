#include "auto_drive.h"

#include "balance_control.h"
#include "brushless_driver.h"
#include "imu_process.h"
#include <math.h>
#include <string.h>

/* 涓诲惊鐜?5ms 涓€娆★紝200 tick = 1 绉?*/
#define AUTO_DRIVE_TICK_MS          (5U)
#define AUTO_DRIVE_TICKS_PER_SEC    (1000U / AUTO_DRIVE_TICK_MS)

/* ---- 鍙傛暟榛樿鍊?/ 鑼冨洿 / 姝ラ暱 ---- */
#define AUTO_DRIVE_SPEED_DEFAULT    (600.0f)
#define AUTO_DRIVE_SPEED_MIN        (0.0f)
#define AUTO_DRIVE_SPEED_MAX        (1000.0f)
#define AUTO_DRIVE_SPEED_STEP       (20.0f)

#define AUTO_DRIVE_TIME_DEFAULT     (3.0f)   /* T1 琛岄┒鏃堕棿榛樿 3 绉?*/
#define AUTO_DRIVE_HOLD_DEFAULT     (2.0f)   /* T2 缁存寔鏃堕棿榛樿 2 绉?*/
#define AUTO_DRIVE_TIME_MIN         (0.0f)
#define AUTO_DRIVE_TIME_MAX         (30.0f)
#define AUTO_DRIVE_TIME_STEP        (0.1f)   /* T1/T2 姝ヨ繘 0.1 绉?*/

/* 鑸悜淇濇寔锛氫笌璇煶鍓嶈繘淇濇寔涓€鑷达紙yaw 鐩爣閿佸畾锛屾瘮渚嬪樊閫熺籂姝ｈ窇鍋忥級锛? * 閬垮厤鍥哄畾閫熷害鐩磋鏃惰溅浣撳ぉ鐒跺乏/鍙冲亸銆?*/
#define AUTO_DRIVE_HEADING_GAIN     (-8.5f)
#define AUTO_DRIVE_TURN_MAX         (250.0f)

/* HOLD 鍋滄闃舵锛歍2 鏃堕棿鍒板悗杩橀渶杞€熶綆浜庨槇鍊煎苟鎸佺画鑻ュ共 tick锛堝仠绋筹級鎵嶅垏 DRIVE_CONT锛? * 閬垮厤"T2 鍐呮病鍋滀綇灏辨帴鐫€璺?锛涘仠绋冲垽瀹氭渶澶氬啀绛?HOLD_TIMEOUT_TICKS 鍏滃簳寮哄埗鍒囪蛋銆?*/
#define AUTO_DRIVE_HOLD_STOP_SPEED    (15.0f)
#define AUTO_DRIVE_HOLD_SETTLE_TICKS  (6U)
#define AUTO_DRIVE_HOLD_TIMEOUT_TICKS (120U)

static float g_speed = AUTO_DRIVE_SPEED_DEFAULT;
static float g_drive_time_s = AUTO_DRIVE_TIME_DEFAULT;
static float g_hold_time_s = AUTO_DRIVE_HOLD_DEFAULT;
static auto_drive_item_t g_selected_item = AUTO_DRIVE_ITEM_SPEED;
static auto_drive_state_t g_state = AUTO_DRIVE_STATE_IDLE;
static uint32_t g_phase_tick = 0U;
static float g_heading_target = 0.0f;
static uint16_t g_hold_settle = 0U;   /* HOLD 鍋滅ǔ杩炵画璁℃暟 */
static auto_drive_command_t g_command;
static bool g_display_dirty = true;
static auto_drive_state_t g_last_state = AUTO_DRIVE_STATE_IDLE;   /* 璇婃柇锛氱姸鎬佸彉鍖栨墦鍗?*/

static float auto_drive_limit(float value, float min, float max)
{
    if(value < min)
    {
        return min;
    }
    if(value > max)
    {
        return max;
    }
    return value;
}

/* 鍘熷湴/鐩磋鑸悜淇濇寔杈撳嚭锛氬綋鍓?yaw 鐩稿鐩爣鑸悜鐨勮宸?* 澧炵泭锛岄檺骞呭埌宸€熻寖鍥?*/
static int16_t auto_drive_heading_turn(void)
{
    float error = imu_roll_balance.posture_value.yaw - g_heading_target;
    while(error > 180.0f)
    {
        error -= 360.0f;
    }
    while(error <= -180.0f)
    {
        error += 360.0f;
    }
    return (int16_t) auto_drive_limit(error * AUTO_DRIVE_HEADING_GAIN,
                                      -AUTO_DRIVE_TURN_MAX,
                                      AUTO_DRIVE_TURN_MAX);
}

void auto_drive_init(void)
{
    g_speed = AUTO_DRIVE_SPEED_DEFAULT;
    g_drive_time_s = AUTO_DRIVE_TIME_DEFAULT;
    g_hold_time_s = AUTO_DRIVE_HOLD_DEFAULT;
    g_selected_item = AUTO_DRIVE_ITEM_SPEED;
    g_state = AUTO_DRIVE_STATE_IDLE;
    g_phase_tick = 0U;
    g_heading_target = 0.0f;
    g_hold_settle = 0U;
    g_display_dirty = true;
    memset(&g_command, 0, sizeof(g_command));
}

void auto_drive_start(void)
{
    g_state = AUTO_DRIVE_STATE_DRIVE_1;
    g_phase_tick = 0U;
    g_heading_target = imu_roll_balance.posture_value.yaw;
    g_hold_settle = 0U;
    g_command.active = true;
    g_command.speed_target = g_speed;
    g_command.turn = auto_drive_heading_turn();
    g_display_dirty = true;
    printf("[AD] start SPD=%d T1=%d T2=%d\r\n",
           (int)g_speed,
           (int)(g_drive_time_s * 10.0f),
           (int)(g_hold_time_s * 10.0f));
}

void auto_drive_stop(void)
{
    g_state = AUTO_DRIVE_STATE_IDLE;
    g_phase_tick = 0U;
    g_hold_settle = 0U;
    g_last_state = AUTO_DRIVE_STATE_IDLE;
    memset(&g_command, 0, sizeof(g_command));
}

void auto_drive_update(void)
{
    /* CH3 鎬ュ仠涓細鐘舵€佹満鏃堕棿鍐荤粨锛堥樁娈佃鏃朵笉鎺ㄨ繘锛夛紝瑙ｉ櫎鍚庝粠鏆傚仠澶勭户缁?*/
    if(balance_control_kill_active())
    {
        return;
    }

    if(AUTO_DRIVE_STATE_IDLE == g_state)
    {
        return;
    }

    g_phase_tick++;

    uint32_t drive_ticks = (uint32_t)(g_drive_time_s * (float) AUTO_DRIVE_TICKS_PER_SEC);
    uint32_t hold_ticks  = (uint32_t)(g_hold_time_s  * (float) AUTO_DRIVE_TICKS_PER_SEC);

    /* T1/T2 璁句负 0 绉掓椂鐩存帴璺宠繃瀵瑰簲闃舵 */
    if((AUTO_DRIVE_STATE_DRIVE_1 == g_state) &&
       ((0U == drive_ticks) || (g_phase_tick >= drive_ticks)))
    {
        g_state = AUTO_DRIVE_STATE_HOLD;
        g_phase_tick = 0U;
        g_hold_settle = 0U;
    }
    else if((AUTO_DRIVE_STATE_HOLD == g_state) && (0U == hold_ticks))
    {
        g_state = AUTO_DRIVE_STATE_DRIVE_CONT;
        g_phase_tick = 0U;
    }
    else if(AUTO_DRIVE_STATE_HOLD == g_state)
    {
        /* T2 鍒扮偣鍚庢鏌ユ槸鍚︾湡鐨勫仠绋筹細杞€熶綆浜庨槇鍊煎苟鎸佺画 6 tick 鎵嶅垏 DRIVE_CONT锛?         * 鍚﹀垯缁х画鍒跺姩锛涜秴杩?T2+120 tick(600ms) 鍏滃簳寮哄埗鍒囪蛋锛岄槻姝㈠垽瀹氬紓甯稿崱鍦?HOLD銆?*/
        brushless_driver_status_t hold_motor;
        brushless_driver_get_status(&hold_motor);
        float hold_avg = (fabsf((float) hold_motor.left_speed) +
                          fabsf((float) hold_motor.right_speed)) * 0.5f;
        if(hold_avg <= AUTO_DRIVE_HOLD_STOP_SPEED)
        {
            g_hold_settle++;
        }
        else
        {
            g_hold_settle = 0U;
        }

        bool hold_stopped = (g_hold_settle >= AUTO_DRIVE_HOLD_SETTLE_TICKS);
        bool hold_timed_out = (g_phase_tick >= (hold_ticks + AUTO_DRIVE_HOLD_TIMEOUT_TICKS));
        if((g_phase_tick >= hold_ticks) && (hold_stopped || hold_timed_out))
        {
            g_state = AUTO_DRIVE_STATE_DRIVE_CONT;
            g_phase_tick = 0U;
        }
    }

    /* 璇婃柇锛氱姸鎬佸彉鍖栨椂鎵撳嵃涓€娆?*/
    if(g_state != g_last_state)
    {
        printf("[AD] state %d->%d tick=%lu\r\n",
               (int)g_last_state, (int)g_state, (unsigned long)g_phase_tick);
        g_last_state = g_state;
    }

    g_command.active = true;
    switch(g_state)
    {
        case AUTO_DRIVE_STATE_DRIVE_1:
        case AUTO_DRIVE_STATE_DRIVE_CONT:
            g_command.speed_target = g_speed;
            g_command.turn = auto_drive_heading_turn();
            break;
        case AUTO_DRIVE_STATE_HOLD:
            g_command.speed_target = 0.0f;
            g_command.turn = 0;
            break;
        default:
            g_command.speed_target = 0.0f;
            g_command.turn = 0;
            break;
    }
}

void auto_drive_get_command(auto_drive_command_t *command)
{
    if(NULL != command)
    {
        *command = g_command;
    }
}

auto_drive_state_t auto_drive_get_state(void)
{
    return g_state;
}

bool auto_drive_is_active(void)
{
    return (AUTO_DRIVE_STATE_IDLE != g_state);
}

const char * auto_drive_get_state_text(void)
{
    switch(g_state)
    {
        case AUTO_DRIVE_STATE_DRIVE_1:    return "DRIVE";
        case AUTO_DRIVE_STATE_HOLD:       return "HOLD ";
        case AUTO_DRIVE_STATE_DRIVE_CONT: return "RUN  ";
        default:                          return "IDLE ";
    }
}

void auto_drive_key_cycle_item(void)
{
    g_selected_item = (auto_drive_item_t)(((int)g_selected_item + 1) % (int) AUTO_DRIVE_ITEM_MAX);
    g_display_dirty = true;
}

void auto_drive_key_adjust_steps(int32_t steps)
{
    float * value_ptr = NULL;
    float min = 0.0f;
    float max = 0.0f;
    float step = 0.0f;

    switch(g_selected_item)
    {
        case AUTO_DRIVE_ITEM_SPEED:
            value_ptr = &g_speed;
            min = AUTO_DRIVE_SPEED_MIN;
            max = AUTO_DRIVE_SPEED_MAX;
            step = AUTO_DRIVE_SPEED_STEP;
            break;
        case AUTO_DRIVE_ITEM_DRIVE_TIME:
            value_ptr = &g_drive_time_s;
            min = AUTO_DRIVE_TIME_MIN;
            max = AUTO_DRIVE_TIME_MAX;
            step = AUTO_DRIVE_TIME_STEP;
            break;
        case AUTO_DRIVE_ITEM_HOLD_TIME:
            value_ptr = &g_hold_time_s;
            min = AUTO_DRIVE_TIME_MIN;
            max = AUTO_DRIVE_TIME_MAX;
            step = AUTO_DRIVE_TIME_STEP;
            break;
        default:
            return;
    }

    *value_ptr = auto_drive_limit(*value_ptr + ((float)steps * step), min, max);
    g_display_dirty = true;
}

void auto_drive_key_restart(void)
{
    g_state = AUTO_DRIVE_STATE_DRIVE_1;
    g_phase_tick = 0U;
    g_heading_target = imu_roll_balance.posture_value.yaw;
    g_display_dirty = true;
}

auto_drive_item_t auto_drive_get_selected_item(void)
{
    return g_selected_item;
}

bool auto_drive_item_is_selected(auto_drive_item_t item)
{
    return (item == g_selected_item);
}

const char * auto_drive_get_item_name(auto_drive_item_t item)
{
    switch(item)
    {
        case AUTO_DRIVE_ITEM_SPEED:      return "SPD:";
        case AUTO_DRIVE_ITEM_DRIVE_TIME: return "T1 :";
        case AUTO_DRIVE_ITEM_HOLD_TIME:  return "T2 :";
        default:                         return "-- :";
    }
}

float auto_drive_get_item_value(auto_drive_item_t item)
{
    switch(item)
    {
        case AUTO_DRIVE_ITEM_SPEED:      return g_speed;
        case AUTO_DRIVE_ITEM_DRIVE_TIME: return g_drive_time_s;
        case AUTO_DRIVE_ITEM_HOLD_TIME:  return g_hold_time_s;
        default:                         return 0.0f;
    }
}

float auto_drive_get_item_step(auto_drive_item_t item)
{
    switch(item)
    {
        case AUTO_DRIVE_ITEM_SPEED:      return AUTO_DRIVE_SPEED_STEP;
        case AUTO_DRIVE_ITEM_DRIVE_TIME: return AUTO_DRIVE_TIME_STEP;
        case AUTO_DRIVE_ITEM_HOLD_TIME:  return AUTO_DRIVE_TIME_STEP;
        default:                         return 1.0f;
    }
}

/* ==================== 鏄剧ず ==================== */

/* 娓呮帀鍙傛暟椤电敤鍒扮殑鎵€鏈夎锛岄伩鍏嶄笂涓€甯ф枃瀛楁畫鐣?*/
static void auto_drive_clear_rows(void)
{
    static const char blank[41] = "                                        ";
    static const uint16 rows[] = {10U, 45U, 70U, 95U, 125U, 155U, 180U};

    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    for(uint32 i = 0; i < (sizeof(rows) / sizeof(rows[0])); i++)
    {
        ips200_show_string(0, rows[i], blank);
    }
}

/* 涓€琛屼竴涓彲璋冨弬鏁帮細琛岄 "->" 閫変腑鏍囪 + 鍚嶇О + 瀹氬鏁板€?+ 鍗曚綅銆? * one_decimal=1 鏃舵暟鍊兼寜 1 浣嶅皬鏁版樉绀猴紙T1/T2锛?.1s 姝ラ暱锛夛紝鍚﹀垯鏄剧ず鏁存暟锛圫PD锛夈€?*/
static void auto_drive_draw_value_row(uint16 y, auto_drive_item_t item,
                                      float value, bool one_decimal, const char *unit)
{
    char text[8];

    ips200_show_string(10, y, auto_drive_item_is_selected(item) ? "->" : "  ");
    ips200_show_string(35, y, auto_drive_get_item_name(item));

    if(auto_drive_item_is_selected(item))
    {
        ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    }
    else
    {
        ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    }

    if(one_decimal)
    {
        /* 瀹氬 4 瀛楃 "dd.d"锛氫袱浣嶆暣鏁帮紙楂樹綅绌烘牸琛ラ綈锛? 灏忔暟鐐?+ 1 浣嶅皬鏁般€?         * 鎵嬪姩鎷煎瓧绗︿覆鏄剧ず 1 浣嶅皬鏁帮紝涓嶇敤 ips200_show_float锛堟浘瀵艰嚧灞忓箷寮傚父锛夈€?         * +0.05f 娑堥櫎 0.1 绱姞鐨勬诞鐐规埅鏂宸紙濡?2.8999999 -> 2.9锛夈€?*/
        int32_t scaled = (int32_t)((value * 10.0f) + 0.05f);
        if(scaled < 0)
        {
            scaled = 0;
        }
        else if(scaled > 300)
        {
            scaled = 300;   /* 涓婇檺 30.0s */
        }
        text[0] = (char)('0' + (scaled / 100) % 10);
        text[1] = (char)('0' + (scaled / 10) % 10);
        text[2] = '.';
        text[3] = (char)('0' + scaled % 10);
        text[4] = '\0';
        if('0' == text[0])
        {
            text[0] = ' ';   /* 楂樹綅琛ョ┖鏍硷紝淇濇寔瀹氬 */
        }
        ips200_show_string(75, y, text);
    }
    else
    {
        ips200_show_int(75, y, (int32_t)value, 4);
    }

    ips200_show_string(107, y, unit);
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
}

void auto_drive_draw_page(void)
{
    auto_drive_clear_rows();

    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(10, 10, "PAGE CAM");

    auto_drive_draw_value_row(45, AUTO_DRIVE_ITEM_SPEED, g_speed, false, "    ");
    auto_drive_draw_value_row(70, AUTO_DRIVE_ITEM_DRIVE_TIME, g_drive_time_s, true, "s   ");
    auto_drive_draw_value_row(95, AUTO_DRIVE_ITEM_HOLD_TIME, g_hold_time_s, true, "s   ");

    ips200_show_string(10, 125, "ST:");
    ips200_show_string(35, 125, balance_control_kill_active() ? "KILL " : auto_drive_get_state_text());

    ips200_show_string(10, 155, "K3 ITEM  K2+  K1-");
    ips200_show_string(10, 180, "K4 RESTART");

    g_display_dirty = false;
}

void auto_drive_update_display(void)
{
    if(g_display_dirty)
    {
        auto_drive_draw_page();
        return;
    }
    /* 鍙傛暟鏈彉鍖栨椂鍙埛鏂扮姸鎬佽锛堝畾瀹芥枃鏈紝鏃犳畫鐣欙級锛汣H3 鎬ュ仠涓樉绀?KILL */
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(35, 125, balance_control_kill_active() ? "KILL " : auto_drive_get_state_text());
}
