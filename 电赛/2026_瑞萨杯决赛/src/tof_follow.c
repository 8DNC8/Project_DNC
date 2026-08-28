#include "tof_follow.h"

#include "balance_control.h"
#include "imu_process.h"
#include "zf_common_headfile.h"
#include <math.h>
#include <string.h>

/* ---- 璺熼殢鍙傛暟 ---- */
#define TOF_FOLLOW_SPEED           (450.0f)   /* 鍓嶈繘鐩爣閫熷害 */
#define TOF_FOLLOW_THRESHOLD_MM    (500U)     /* 涓ユ牸 500mm锛?=500 鍋?/ >500 璧帮紝鏃犲洖宸?*/
#define TOF_FOLLOW_MAX_VALID_MM    (4000U)    /* DL1B 鏈夋晥閲忕▼涓婇檺锛岃秴杩囪涓烘棤鏁?*/
#define TOF_FOLLOW_POLL_TICKS      (10U)      /* 10 * 5ms = 50ms 杞涓€娆?TOF */
#define TOF_FOLLOW_BRAKE_TICKS     (20U)      /* 20 * 5ms = 100ms 绱ф€ュ埗鍔ㄦ寔缁椂闂?*/
#define TOF_FOLLOW_HEADING_GAIN    (-8.5f)    /* yaw 璇樊鈫掑樊閫熷鐩?*/
#define TOF_FOLLOW_TURN_MAX        (250.0f)

typedef enum
{
    TOF_FOLLOW_STATE_STOP = 0,   /* 闈欐锛堝崐绫冲唴 / 娴嬩笉鍒颁汉锛?*/
    TOF_FOLLOW_STATE_GO,         /* 鐩寸嚎鍓嶈繘 400 */
    TOF_FOLLOW_STATE_BRAKE,      /* 绱ф€ュ埗鍔細GO 杩涘叆鍗婄背鍐呮椂瑙﹀彂锛?00ms 鍚庡垏 STOP */
} tof_follow_state_t;

static tof_follow_state_t g_state = TOF_FOLLOW_STATE_STOP;
static float g_heading_target = 0.0f;
static bool g_heading_locked = false;       /* 鑸悜鏄惁宸查攣瀹氾紙棣栨 start 鏃堕攣锛屾案涓嶉噸閿侊級 */
static uint32_t g_poll_tick = 0U;
static uint32_t g_brake_tick = 0U;
static tof_follow_command_t g_command;

static float tof_follow_limit(float value, float min, float max)
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

/* 鐩磋鑸悜淇濇寔杈撳嚭锛氬綋鍓?yaw 鐩稿鐩爣鑸悜鐨勮宸?* 澧炵泭锛岄檺骞呭埌宸€熻寖鍥?*/
static int16_t tof_follow_heading_turn(void)
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
    return (int16_t) tof_follow_limit(error * TOF_FOLLOW_HEADING_GAIN,
                                      -TOF_FOLLOW_TURN_MAX,
                                      TOF_FOLLOW_TURN_MAX);
}

/* TOF 褰撳墠璺濈鏄惁鏈夋晥锛堝凡鍒濆鍖?+ 鏈夋渶鏂拌鏁?+ 鍦ㄩ噺绋嬪唴锛?*/
static bool tof_follow_distance_valid(void)
{
    return (0U != dl1b_init_flag) &&
           (0U != dl1b_finsh_flag) &&
           (dl1b_distance_mm <= TOF_FOLLOW_MAX_VALID_MM);
}

void tof_follow_init(void)
{
    g_state = TOF_FOLLOW_STATE_STOP;
    g_heading_target = 0.0f;
    g_heading_locked = false;
    g_poll_tick = 0U;
    g_brake_tick = 0U;
    memset(&g_command, 0, sizeof(g_command));
}

void tof_follow_start(void)
{
    g_state = TOF_FOLLOW_STATE_STOP;   /* 杩涘叆鍏堥潤姝細浜哄湪鍗婄背鍐呮椂涓嶅墠杩?*/
    /* 鑸悜閿佸畾锛氫粎棣栨杩涘叆鏃堕攣瀹氾紝鍚庣画閲嶆柊杩涘叆璺熼殢妯″紡涓嶉噸閿侊紝
     * 淇濇寔"涓婄數鍚庨娆¤繘鍏ユ椂鐨勬柟鍚?浣滀负姘镐箙鑸悜鐩爣銆?*/
    if(!g_heading_locked)
    {
        g_heading_target = imu_roll_balance.posture_value.yaw;
        g_heading_locked = true;
        printf("[TOF] heading locked: %d\r\n", (int)g_heading_target);
    }
    g_poll_tick = 0U;
    g_brake_tick = 0U;
    memset(&g_command, 0, sizeof(g_command));
    g_command.active = true;
    printf("[TOF] follow start, heading=%d\r\n", (int)g_heading_target);
}

void tof_follow_stop(void)
{
    g_state = TOF_FOLLOW_STATE_STOP;
    g_brake_tick = 0U;
    memset(&g_command, 0, sizeof(g_command));
}

void tof_follow_update(void)
{
    /* CH3 鎬ュ仠涓細鐘舵€佹満鍐荤粨锛岃В闄ゅ悗浠庡綋鍓嶇姸鎬佺户缁?*/
    if(balance_control_kill_active())
    {
        return;
    }

    g_poll_tick++;
    if(g_poll_tick >= TOF_FOLLOW_POLL_TICKS)
    {
        g_poll_tick = 0U;
        dl1b_get_distance();
    }

    /* ---- 鐘舵€佹満锛堜弗鏍?500mm锛屾棤鍥炲樊锛?--- */
    if(!tof_follow_distance_valid())
    {
        /* 娴嬩笉鍒颁汉锛堣秴閲忕▼/鏃犳晥锛夛細瀹夊叏鍋滆溅 */
        g_state = TOF_FOLLOW_STATE_STOP;
        g_brake_tick = 0U;
    }
    else if(TOF_FOLLOW_STATE_GO == g_state)
    {
        if(dl1b_distance_mm <= TOF_FOLLOW_THRESHOLD_MM)
        {
            /* 杩涘叆鍗婄背鍐咃細绔嬪嵆鍒?BRAKE锛岃緭鍑?speed=0 瑙﹀彂 cam_hold 涓诲姩鍒跺姩 */
            g_state = TOF_FOLLOW_STATE_BRAKE;
            g_brake_tick = 0U;
        }
    }
    else if(TOF_FOLLOW_STATE_BRAKE == g_state)
    {
        /* 鍒跺姩鏈熼棿涓嶅搷搴旇窛绂诲彉鍖栵紝寮哄埗淇濇寔 100ms 璁╄溅鍋滅ǔ */
        g_brake_tick++;
        if(g_brake_tick >= TOF_FOLLOW_BRAKE_TICKS)
        {
            g_state = TOF_FOLLOW_STATE_STOP;
        }
    }
    else  /* STOP */
    {
        if(dl1b_distance_mm > TOF_FOLLOW_THRESHOLD_MM)
        {
            /* 浜鸿蛋杩滆秴杩囧崐绫筹細绔嬪嵆鎭㈠鍓嶈繘 */
            g_state = TOF_FOLLOW_STATE_GO;
        }
    }

    /* ---- 杈撳嚭鍛戒护 ---- */
    g_command.active = true;
    if(TOF_FOLLOW_STATE_GO == g_state)
    {
        g_command.speed_target = TOF_FOLLOW_SPEED;
        g_command.turn = tof_follow_heading_turn();
    }
    else
    {
        /* STOP 鍜?BRAKE 閮借緭鍑?speed=0 / turn=0锛?         * balance_control 妫€娴嬪埌 camera_following && speed_target鈮? 鏃惰繘鍏?cam_hold锛?         * 鐢ㄥ甫绗﹀彿杞€熷弽棣堝仛涓诲姩鍒跺姩锛屽€掓簻鏃惰嚜鍔ㄧ籂鍋忥紙涓嶅悗閫€澶锛夈€?*/
        g_command.speed_target = 0.0f;
        g_command.turn = 0;
    }
}

void tof_follow_get_command(tof_follow_command_t *command)
{
    if(NULL != command)
    {
        *command = g_command;
    }
}

/* ---- 鏄剧ず ---- */

static const char * tof_follow_state_text(void)
{
    if(balance_control_kill_active())
    {
        return "KILL";
    }
    switch(g_state)
    {
        case TOF_FOLLOW_STATE_GO:    return "GO  ";
        case TOF_FOLLOW_STATE_BRAKE: return "BRK ";
        default:                     return "STOP";
    }
}

static void tof_follow_draw_state_row(void)
{
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(35, 70, tof_follow_state_text());
}

static void tof_follow_draw_distance_row(void)
{
    if(tof_follow_distance_valid())
    {
        ips200_set_color(RGB565_WHITE, RGB565_BLACK);
        ips200_show_uint(35, 45, dl1b_distance_mm, 4);
        ips200_show_string(75, 45, "mm  ");
    }
    else
    {
        ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
        ips200_show_string(35, 45, "--- ");
        ips200_show_string(75, 45, "    ");   /* 娓呮帀娈嬬暀鐨?"mm" */
    }
}

void tof_follow_draw_page(void)
{
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(10, 10, "TOF FOLLOW");
    ips200_show_string(10, 45, "D:");
    ips200_show_string(10, 70, "ST:");
    ips200_show_string(10, 95, "SPD:400");
    tof_follow_draw_state_row();
    tof_follow_draw_distance_row();
}

void tof_follow_update_display(void)
{
    tof_follow_draw_state_row();
    tof_follow_draw_distance_row();
}
