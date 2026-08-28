#include "balance_control.h"

#include "brushless_driver.h"
#include "camera_display.h"
#include "imu_process.h"
#include "key_tune.h"
#include "nag_navigation.h"
#include "servo_hold.h"
#include "voice_control.h"
#include "remote_control.h"

#include <math.h>
#include <string.h>

#define BALANCE_CONTROL_ENABLE_MOTOR_OUTPUT    (1)
#define BALANCE_CONTROL_WARMUP_TICKS           (100U)
#define BALANCE_CONTROL_MAX_DUTY               (1800)
#define BALANCE_CONTROL_SAFE_ANGLE_DEG         (180.0f)  /* PID 璋冭瘯锛氫复鏃跺彇娑?35 搴﹀€惧€掑仠鏈洪檺鍒?*/
#define BALANCE_CONTROL_TARGET_PITCH_DEG       (0.0f)
#define BALANCE_CONTROL_MOTOR_PERIOD_TICKS     (1U)
#define BALANCE_CONTROL_DUTY_FILTER_OLD        (0.50f)
#define BALANCE_CONTROL_DUTY_FILTER_NEW        (0.50f)
#define BALANCE_CONTROL_DUTY_SLEW_STEP         (300)
#define BALANCE_CONTROL_VOICE_SPEED             (400.0f)   /* 鎻愰珮璇煶鍓嶈繘/鍚庨€€椹卞姩鍔涳紝瀵归綈鎯甯哥敤鐩爣閫熷害 */
#define BALANCE_CONTROL_REMOTE_TURN_GAIN        (0.60f)    /* 閬ユ帶鍣?steering -1000~1000 鈫?杞悜 -600~600 */
#define BALANCE_CONTROL_REMOTE_TURN_MAX         (600.0f)
#define BALANCE_CONTROL_REMOTE_DEAD_ZONE        (30)       /* 閬ユ帶鍣ㄦ补闂?鏂瑰悜姝诲尯锛岄伩鍏嶆憞鏉嗕腑浣嶆紓绉?*/
#define BALANCE_CONTROL_VOICE_HEADING_GAIN      (-8.5f)
#define BALANCE_CONTROL_VOICE_SPIN_DUTY         (300)      /* 鍘熷湴鏃嬭浆鏃跺乏鍙宠疆鐨勫弽鍚戝樊閫熷崰绌烘瘮锛岀粷瀵瑰€?*/
#define BALANCE_CONTROL_VOICE_SPIN_MIN_DUTY     (120)      /* 鍑忛€熷尯鏈€浣庤浆鍚戝崰绌烘瘮锛岄槻姝㈠埌鐩爣鍓嶅仠浣?*/
#define BALANCE_CONTROL_VOICE_TURN_DEG           (90.0f)    /* 姣忔潯宸﹁浆/鍙宠浆鍛戒护鐨勭洰鏍囪浆瑙?*/
#define BALANCE_CONTROL_VOICE_TURN_BRAKE_DEG     (82.0f)    /* 鎻愬墠鍒跺姩锛屽埄鐢ㄦ儻鎬у仠鍦ㄧ害 90掳 */
#define BALANCE_CONTROL_VOICE_TURN_SLOW_DEG     (18.0f)    /* 鎺ヨ繎鐩爣鏃剁殑鍑忛€熷尯锛岄伩鍏嶈浆杩囧ご */
#define BALANCE_CONTROL_VOICE_TURN_TIMEOUT      (400U)     /* yaw 寮傚父鏃舵渶澶氭棆杞?2 绉掞紝闅忓悗寮哄埗鍙嶅悜鍒跺姩 */
#define BALANCE_CONTROL_BRAKE_SPEED_THRESHOLD   (15.0f)    /* 浣庝簬姝よ疆閫熷苟绋冲畾涓€娈垫椂闂村悗閫€鍑轰富鍔ㄥ埗鍔?*/
#define BALANCE_CONTROL_BRAKE_SETTLE_TICKS      (6U)
#define BALANCE_CONTROL_BRAKE_TIMEOUT_TICKS     (120U)     /* 涓诲姩鍒跺姩鏈€闀?600ms锛岃秴鏃跺悗鍙繚鐣欏钩琛?*/
#define BALANCE_CONTROL_BRAKE_TURN_MAX_TICKS    (30U)      /* 杞悜鍙嶅埗鍔ㄦ渶澶?150ms锛岄槻姝㈠弽鍚戞棆杞?*/
#define BALANCE_CONTROL_BRAKE_LEG_DIVISOR       (12.0f)    /* 灏忎簬鏅€氶€熷害鐜櫎鏁帮紝浣垮弽鍚戝帇鑵挎洿杩呴€?*/
#define BALANCE_CONTROL_BRAKE_TURN_DUTY         (300)
#define BALANCE_CONTROL_KILL_RESUME_ANGLE_DEG   (45.0f)    /* CH3 鎬ュ仠瑙ｉ櫎鍚庯細濮挎€佽秴杩囨瑙掑害涓嶆仮澶嶅钩琛¤緭鍑猴紝闃插€掕溅鎸ｆ墡 */

typedef enum
{
    BALANCE_VOICE_IDLE = 0,
    BALANCE_VOICE_FORWARD,
    BALANCE_VOICE_BACKWARD,
    BALANCE_VOICE_TURN,
} balance_voice_mode_t;

typedef enum
{
    BALANCE_BRAKE_IDLE = 0,
    BALANCE_BRAKE_LONGITUDINAL,
    BALANCE_BRAKE_TURN,
} balance_brake_mode_t;

static balance_control_status_t g_balance_status;
static int16_t g_balance_filtered_duty = 0;
static bool g_balance_replaying = false;
static float g_balance_steer_filter = 0.0f;
static balance_voice_mode_t g_balance_voice_mode = BALANCE_VOICE_IDLE;
static uint32_t g_balance_voice_sequence = 0U;
static float g_balance_voice_heading_target = 0.0f;
static int16_t g_balance_voice_spin_dir = 0;   /* 鍘熷湴鏃嬭浆鏂瑰悜锛氬疄杞︾害瀹? +1=宸︽棆  -1=鍙虫棆  0=涓嶆棆杞?*/
static float g_balance_voice_turn_last_yaw = 0.0f;
static float g_balance_voice_turn_accumulated = 0.0f;
static uint32_t g_balance_voice_turn_ticks = 0U;  /* 鍘熷湴鏃嬭浆绱鏃堕暱璁℃暟锛岀敤浜庤秴鏃朵繚鎶?*/
static balance_brake_mode_t g_balance_brake_mode = BALANCE_BRAKE_IDLE;
static int16_t g_balance_brake_direction = 0;
static uint16_t g_balance_brake_ticks = 0U;
static uint16_t g_balance_brake_settle = 0U;
static bool g_balance_shutdown = false;
static bool g_balance_cam_park = false;   /* 鎽勫儚澶寸敾闈㈡ā寮忥細鏂數闈欑疆锛堝叧鑸垫満+鏃犲埛锛屼笉缁存寔骞宠　锛?*/
static int16_t g_balance_cam_speed_sign = 0;   /* 鎽勫儚澶存ā寮忚疆閫?鍓嶈繘鏂瑰悜"绗﹀彿鏍囧畾锛?1/-1锛?=鏈爣瀹?*/
static bool g_balance_kill_resume_pending = false;  /* CH3 鎬ュ仠瑙ｉ櫎鍚庯紝绛夊緟杞﹁韩濮挎€佹仮澶嶆甯稿啀鎭㈠骞宠　杈撳嚭 */

static int16_t balance_limit_duty(float duty)
{
    if(duty > (float)BALANCE_CONTROL_MAX_DUTY)
    {
        return BALANCE_CONTROL_MAX_DUTY;
    }
    if(duty < -(float)BALANCE_CONTROL_MAX_DUTY)
    {
        return -BALANCE_CONTROL_MAX_DUTY;
    }
    return (int16_t)duty;
}

static float balance_limit_float(float value, float min, float max)
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

static float balance_angle_error(float current, float target)
{
    float error = current - target;
    while(error > 180.0f)
    {
        error -= 360.0f;
    }
    while(error <= -180.0f)
    {
        error += 360.0f;
    }
    return error;
}

static void balance_reset_speed_pid(void)
{
    imu_roll_balance.speed_cycle.i_value = 0.0f;
    imu_roll_balance.speed_cycle.p_value_last = 0.0f;
    imu_roll_balance.speed_cycle.out = 0.0f;
    g_balance_steer_filter = 0.0f;
}

static void balance_finish_brake(void)
{
    g_balance_brake_mode = BALANCE_BRAKE_IDLE;
    g_balance_brake_direction = 0;
    g_balance_brake_ticks = 0U;
    g_balance_brake_settle = 0U;
    balance_reset_speed_pid();
}

static void balance_start_turn_brake(int16_t spin_direction)
{
    g_balance_brake_mode = BALANCE_BRAKE_TURN;
    g_balance_brake_direction = spin_direction;
    g_balance_brake_ticks = 0U;
    g_balance_brake_settle = 0U;
    g_balance_voice_mode = BALANCE_VOICE_IDLE;
    g_balance_voice_spin_dir = 0;
    g_balance_voice_turn_accumulated = 0.0f;
    g_balance_voice_turn_ticks = 0U;
    balance_reset_speed_pid();
}

static void balance_update_voice_command(void)
{
    uint32_t sequence = voice_control_get_command_sequence();
    if(sequence == g_balance_voice_sequence)
    {
        return;
    }

    g_balance_voice_sequence = sequence;
    balance_reset_speed_pid();

    voice_command_t cmd = voice_control_get_command();
    bool voice_mode_active = voice_control_mode_active();

    /* 閬ユ帶鍣ㄤ娇鑳介椄闂細鍙湁"閬ユ帶妯″紡"璇煶鎸囦护鎵嶅厑璁搁仴鎺у櫒杈撳嚭鎺у埗閲忥紝
     * 鏀跺埌鍏跺畠浠讳綍鎸囦护锛堣繍鍔?鍋滄/妯″紡鍒囨崲锛夐兘绔嬪嵆绂佺敤閬ユ帶鍣ㄣ€?*/
    uart_receiver_set_enabled(VOICE_COMMAND_REMOTE == cmd);

    switch(cmd)
    {
        case VOICE_COMMAND_FORWARD:
            if(!voice_mode_active) { break; }
            balance_finish_brake();
            g_balance_voice_mode = BALANCE_VOICE_FORWARD;
            g_balance_voice_heading_target = imu_roll_balance.posture_value.yaw;
            break;
        case VOICE_COMMAND_BACKWARD:
            if(!voice_mode_active) { break; }
            balance_finish_brake();
            g_balance_voice_mode = BALANCE_VOICE_BACKWARD;
            g_balance_voice_heading_target = imu_roll_balance.posture_value.yaw;
            break;
        case VOICE_COMMAND_LEFT:
            if(!voice_mode_active) { break; }
            balance_finish_brake();
            g_balance_voice_mode = BALANCE_VOICE_TURN;
            g_balance_voice_spin_dir = +1;   /* 鍘熷湴宸︽棆锛堝疄杞︽柟鍚戯細鍘?+1 涓哄乏鏃嬶紝宸插璋冿級 */
            g_balance_voice_turn_last_yaw = imu_roll_balance.posture_value.yaw;
            g_balance_voice_turn_accumulated = 0.0f;
            g_balance_voice_turn_ticks = 0U;
            break;
        case VOICE_COMMAND_RIGHT:
            if(!voice_mode_active) { break; }
            balance_finish_brake();
            g_balance_voice_mode = BALANCE_VOICE_TURN;
            g_balance_voice_spin_dir = -1;   /* 鍘熷湴鍙虫棆锛堝疄杞︽柟鍚戯細鍘?-1 涓哄彸鏃嬶紝宸插璋冿級 */
            g_balance_voice_turn_last_yaw = imu_roll_balance.posture_value.yaw;
            g_balance_voice_turn_accumulated = 0.0f;
            g_balance_voice_turn_ticks = 0U;
            break;
        case VOICE_COMMAND_STOP:
            if(!voice_mode_active) { break; }
            balance_finish_brake();
            /* 鍋滆溅锛氬仠姝竴鍒囪繍鍔ㄨ緭鍑猴紝浣嗕繚鐣欏钩琛℃帶鍒惰灏忚溅绔欎綇銆?
             * 娉ㄦ剰锛氫笉鑳藉啀璁剧疆 g_balance_shutdown 鈥斺€?鏃ч€昏緫浼氭案涔呭叧闂數鏈?鑸垫満锛?
             * 涓旇鏍囧織浠庝笉澶嶄綅锛屽鑷存鍚庝换浣曡闊冲姩浣滐紙鍓嶈繘/鎽勫儚澶存ā寮忕瓑锛夐兘鏃犳硶椹卞姩灏忚溅銆?*/
            g_balance_voice_mode = BALANCE_VOICE_IDLE;
            g_balance_voice_heading_target = 0.0f;
            g_balance_voice_spin_dir = 0;
            g_balance_voice_turn_accumulated = 0.0f;
            g_balance_voice_turn_ticks = 0U;
            break;
        default:
            balance_finish_brake();
            /* 鍏朵綑鍛戒护锛堟憚鍍忓ご/鎯/璺宠穬/閬ユ帶绛夛級涓嶇畻杩愬姩鎸囦护 */
            g_balance_voice_mode = BALANCE_VOICE_IDLE;
            g_balance_voice_spin_dir = 0;
            g_balance_voice_turn_accumulated = 0.0f;
            g_balance_voice_turn_ticks = 0U;
            break;
    }
}

static int16_t balance_filter_duty(int16_t duty)
{
    float filtered = ((float)g_balance_filtered_duty * BALANCE_CONTROL_DUTY_FILTER_OLD) +
                     ((float)duty * BALANCE_CONTROL_DUTY_FILTER_NEW);
    int16_t next = balance_limit_duty(filtered);
    int16_t delta = (int16_t)(next - g_balance_filtered_duty);

    if(delta > BALANCE_CONTROL_DUTY_SLEW_STEP)
    {
        next = (int16_t)(g_balance_filtered_duty + BALANCE_CONTROL_DUTY_SLEW_STEP);
    }
    else if(delta < -BALANCE_CONTROL_DUTY_SLEW_STEP)
    {
        next = (int16_t)(g_balance_filtered_duty - BALANCE_CONTROL_DUTY_SLEW_STEP);
    }

    g_balance_filtered_duty = next;
    return next;
}

static void balance_reset_pid(void)
{
    imu_roll_balance.angle_cycle.i_value = 0.0f;
    imu_roll_balance.angle_cycle.p_value_last = 0.0f;
    imu_roll_balance.angle_cycle.out = 0.0f;

    imu_roll_balance.angular_speed_cycle.i_value = 0.0f;
    imu_roll_balance.angular_speed_cycle.p_value_last = 0.0f;
    imu_roll_balance.angular_speed_cycle.out = 0.0f;
}

static bool balance_motor_send_due(void)
{
    return ((g_balance_status.tick_count % BALANCE_CONTROL_MOTOR_PERIOD_TICKS) == 0U);
}

static void balance_set_zero_output(void)
{
    g_balance_status.left_duty = 0;
    g_balance_status.right_duty = 0;
    g_balance_status.angular_speed_output = 0.0f;
    g_balance_status.angle_output = 0.0f;
    g_balance_filtered_duty = 0;
}

void balance_control_init(void)
{
    memset(&g_balance_status, 0, sizeof(g_balance_status));
    g_balance_filtered_duty = 0;
    g_balance_voice_mode = BALANCE_VOICE_IDLE;
    g_balance_voice_sequence = voice_control_get_command_sequence();
    g_balance_voice_heading_target = 0.0f;
    g_balance_voice_spin_dir = 0;
    g_balance_voice_turn_last_yaw = 0.0f;
    g_balance_voice_turn_accumulated = 0.0f;
    g_balance_voice_turn_ticks = 0U;
    g_balance_brake_mode = BALANCE_BRAKE_IDLE;
    g_balance_brake_direction = 0;
    g_balance_brake_ticks = 0U;
    g_balance_brake_settle = 0U;
    g_balance_shutdown = false;
    g_balance_kill_resume_pending = false;
    g_balance_cam_park = false;

    servo_hold_init();
    g_balance_status.brushless_init_error = brushless_driver_init();
    g_balance_status.brushless_last_error = g_balance_status.brushless_init_error;
    g_balance_status.enabled = (FSP_SUCCESS == g_balance_status.brushless_init_error);
    g_balance_status.safe = false;
    g_balance_status.angle_target = BALANCE_CONTROL_TARGET_PITCH_DEG;

    balance_reset_pid();
}

void balance_control_stop(void)
{
    balance_set_zero_output();
    g_balance_status.safe = false;
    balance_reset_pid();
    g_balance_status.brushless_last_error = brushless_driver_set_duty(0, 0);
}

void balance_control_update(void)
{
    g_balance_status.tick_count++;

    /* ===== CH3 杩愯闂搁棬鎬ュ仠锛堟渶楂樹紭鍏堢骇锛屼换浣曟ā寮忕敓鏁堬級=====
     * 鎺ユ敹鏈哄湪绾夸笖鏍囧畾瀹屾垚鍚庯細
     *   CH3 浣庝綅 => 鎬ュ仠锛氳疆瀛?duty 娓呴浂銆佽埖鏈烘帀鐢碉紙servo_hold_disable锛夈€?
     *              涓嶇淮鎸佸钩琛★紙杞︿細鍊掞級锛岀瓑鏁堢數婧愬紑鍏冲叧闂紱
     *   CH3 楂樹綅 => 瑙ｉ櫎鎬ュ仠锛氭仮澶嶈埖鏈轰緵鐢点€佽吙鍥炴満姊伴浂鐐广€佸Э鎬佹甯稿悗鎭㈠骞宠　锛?
     *              姝ｅ湪鎵ц鐨勬ā寮忥紙濡傛憚鍍忓ご瀹氭椂琛岄┒锛変粠鏆傚仠澶勭户缁€?*/
    if((0U != uart_receiver.state) && (0U != uart_receiver.calibration_ready))
    {
        if(0U == uart_receiver.run_enabled)
        {
            if(!g_balance_shutdown)
            {
                g_balance_shutdown = true;
                printf("[KILL] CH3 low -> emergency stop.\r\n");
            }
        }
        else if(g_balance_shutdown)
        {
            g_balance_shutdown = false;
            g_balance_kill_resume_pending = true;   /* 鍏堢瓑杞﹁韩濮挎€佹甯稿啀鎭㈠骞宠　杈撳嚭 */
            servo_hold_enable();
            servo_hold_set_pulse(0, SERVO_HOLD_PULSE_US);
            servo_hold_set_pulse(1, SERVO_HOLD_PULSE_US);
            servo_hold_set_pulse(2, SERVO_HOLD_PULSE_US);
            servo_hold_set_pulse(3, SERVO_HOLD_PULSE_US);
            balance_reset_pid();
            balance_reset_speed_pid();
            printf("[KILL] CH3 high -> resume.\r\n");
        }
    }

    if(g_balance_shutdown)
    {
        servo_hold_disable();
        brushless_driver_process();
        balance_set_zero_output();
        balance_reset_pid();
        balance_reset_speed_pid();
        g_balance_status.safe = false;
        g_balance_status.shutdown = true;
        fsp_err_t stop_err = brushless_driver_set_duty(0, 0);
        if(FSP_ERR_IN_USE != stop_err)
        {
            g_balance_status.brushless_last_error = stop_err;
        }
        return;
    }

    /* ===== 鎽勫儚澶寸敾闈㈡ā寮忥紙'5'锛夛細鍏抽棴鑸垫満+鏃犲埛锛屼笉缁存寔骞宠　銆佷笉浜х敓閫熷害 =====
     * balance_control 妫€娴嬪埌 camera_display_view_active() 鏃惰繘鍏?鏂數闈欑疆"锛?
     * 鑸垫満鎺夌數銆佹棤鍒?duty 娓呴浂銆佽烦杩囧钩琛?PID锛涢€€鍑虹敾闈㈡ā寮忥紙鍒囪窡闅?鎭㈠椤甸潰锛夋椂
     * 鎭㈠鑸垫満渚涚數锛屽苟澶嶇敤濮挎€佸畨鍏ㄩ椄锛堣溅鍊掍簡鍏堢瓑濮挎€佸洖姝ｅ啀鎭㈠骞宠　杈撳嚭锛夈€?*/
    if(camera_display_view_active() && !g_balance_cam_park)
    {
        g_balance_cam_park = true;
        servo_hold_disable();
        balance_set_zero_output();
        balance_reset_pid();
        balance_reset_speed_pid();
        g_balance_status.safe = false;
        printf("[BAL] camera view -> servo+brushless off.\r\n");
    }
    else if(!camera_display_view_active() && g_balance_cam_park)
    {
        g_balance_cam_park = false;
        servo_hold_enable();
        servo_hold_set_pulse(0, SERVO_HOLD_PULSE_US);
        servo_hold_set_pulse(1, SERVO_HOLD_PULSE_US);
        servo_hold_set_pulse(2, SERVO_HOLD_PULSE_US);
        servo_hold_set_pulse(3, SERVO_HOLD_PULSE_US);
        balance_reset_pid();
        balance_reset_speed_pid();
        g_balance_kill_resume_pending = true;   /* 澶嶇敤濮挎€佸畨鍏ㄩ椄锛氳溅鍊掍簡鍏堢瓑鍥炴 */
        printf("[BAL] camera view exit -> resume.\r\n");
    }

    if(g_balance_cam_park)
    {
        brushless_driver_process();   /* 淇濇寔鏃犲埛 UART 鏀跺彂娲昏穬锛岄伩鍏嶈秴鏃堕噸鍚?*/
        fsp_err_t stop_err = brushless_driver_set_duty(0, 0);
        if(FSP_ERR_IN_USE != stop_err)
        {
            g_balance_status.brushless_last_error = stop_err;
        }
        return;
    }

    if((g_balance_status.tick_count % BALANCE_CONTROL_SERVO_PERIOD_TICKS) == 0U)
    {
        servo_hold_refresh();
    }

    if(!g_balance_status.enabled)
    {
        return;
    }

    brushless_driver_process();

    bool angle_safe = (fabsf(imu_roll_balance.posture_value.pit) < BALANCE_CONTROL_SAFE_ANGLE_DEG) &&
                      (fabsf(imu_roll_balance.posture_value.rol) < BALANCE_CONTROL_SAFE_ANGLE_DEG);
    if(g_balance_kill_resume_pending)
    {
        /* CH3 鎬ュ仠瑙ｉ櫎鍚庯細杞﹁韩瑙掑害寮傚父锛堣溅鍊掍簡锛夋椂淇濇寔闆惰緭鍑恒€佽吙鍥為浂鐐癸紝
         * 鐩村埌濮挎€佸洖鍒板畨鍏ㄨ寖鍥存墠鎭㈠骞宠　锛岄伩鍏嶅€掑湪鍦颁笂鍏ㄥ姏鎸ｆ墡鎹熷潖鐢垫満/鑸垫満銆?*/
        if((fabsf(imu_roll_balance.posture_value.pit) < BALANCE_CONTROL_KILL_RESUME_ANGLE_DEG) &&
           (fabsf(imu_roll_balance.posture_value.rol) < BALANCE_CONTROL_KILL_RESUME_ANGLE_DEG))
        {
            g_balance_kill_resume_pending = false;
        }
        else
        {
            angle_safe = false;
        }
    }
    bool warmup_done = (g_balance_status.tick_count >= BALANCE_CONTROL_WARMUP_TICKS);

    g_balance_status.safe = angle_safe && warmup_done;
    if(!g_balance_status.safe)
    {
        balance_set_zero_output();
        balance_reset_pid();
        /* 鑵垮洖鏈烘闆剁偣锛岄伩鍏嶆憯鍊?澶辨帶鏃惰吙鍋滃湪鍋忔枩浣嶇疆 */
        g_balance_steer_filter = 0.0f;
        servo_hold_set_pulse(0, SERVO_HOLD_PULSE_US);
        servo_hold_set_pulse(1, SERVO_HOLD_PULSE_US);
        servo_hold_set_pulse(2, SERVO_HOLD_PULSE_US);
        servo_hold_set_pulse(3, SERVO_HOLD_PULSE_US);
        if(balance_motor_send_due())
        {
            g_balance_status.brushless_last_error = brushless_driver_set_duty(0, 0);
        }
        return;
    }

    /* 澶嶇幇鏃堕€熷害鐜帴鑵匡細瀹炶溅娴嬭瘯 S1-/S2+/S3-/S4+ 浼氬€掗€€锛?
     * 鍥犳鍓嶈繘閲忎负姝ｆ椂鏁翠綋鍙嶅悜涓?S1+/S2-/S3+/S4-銆?*/
    nag_navigation_state_t nag_state = nag_navigation_get_state();
    balance_update_voice_command();
    if(g_balance_shutdown)
    {
        servo_hold_disable();
        balance_set_zero_output();
        balance_reset_pid();
        balance_reset_speed_pid();
        g_balance_status.safe = false;
        g_balance_status.shutdown = true;
        g_balance_status.brushless_last_error = brushless_driver_set_duty(0, 0);
        return;
    }
    camera_follow_command_t follow_command;
    camera_display_get_follow_command(&follow_command);
    bool camera_following = follow_command.active;
    voice_command_t voice_command = voice_control_get_command();
    bool inertial_mode = (VOICE_COMMAND_INS == voice_command) ||
                         (NAG_STATE_REPLAYING == nag_state) ||
                         (NAG_STATE_STRAIGHT == nag_state);
    bool voice_mode_active = voice_control_mode_active();
    bool voice_manual_allowed = voice_mode_active && !camera_following && !inertial_mode;
    bool voice_override = voice_mode_active &&
                          ((VOICE_COMMAND_FORWARD == voice_command) ||
                           (VOICE_COMMAND_BACKWARD == voice_command) ||
                           (VOICE_COMMAND_LEFT == voice_command) ||
                           (VOICE_COMMAND_RIGHT == voice_command));

    /* 閬ユ帶鍣ㄦ縺娲绘潯浠讹細璇煶宸蹭娇鑳?+ 鎺ユ敹鏈哄湪绾?+ 鏍″噯瀹屾垚 */
    bool remote_active = uart_receiver_is_enabled() &&
                         (0U != uart_receiver.state) &&
                         (0U != uart_receiver.calibration_ready);

    if(!voice_manual_allowed)
    {
        g_balance_voice_mode = BALANCE_VOICE_IDLE;
    }
    bool voice_motion = voice_manual_allowed &&
                        (BALANCE_VOICE_IDLE != g_balance_voice_mode);
    bool braking = (BALANCE_BRAKE_IDLE != g_balance_brake_mode);
    bool replaying = camera_following || voice_motion || braking || remote_active ||
                     ((!voice_override) &&
                      ((NAG_STATE_REPLAYING == nag_state) ||
                       (NAG_STATE_STRAIGHT == nag_state)));
    if(balance_motor_send_due())
    {
        if(replaying)
        {
            if(!g_balance_replaying)
            {
                imu_roll_balance.speed_cycle.i_value = 0.0f;
                imu_roll_balance.speed_cycle.p_value_last = 0.0f;
                imu_roll_balance.speed_cycle.out = 0.0f;
                g_balance_steer_filter = 0.0f;
                g_balance_cam_speed_sign = 0;   /* 姣忔杩涘叆澶嶇幇/鎽勫儚澶撮┍鍔ㄦ椂閲嶆柊鏍囧畾鍓嶈繘鏂瑰悜 */
                if(camera_following)
                {
                    /* 璇婃柇锛氭憚鍍忓ご锛堝畾鏃惰椹讹級椹卞姩婵€娲绘椂鎵撳嵃涓€娆＄洰鏍囬€熷害 */
                    printf("[BAL] cam drive on speed=%d\r\n", (int) follow_command.speed_target);
                }
            }

            float speed_target = nag_navigation_travel_speed;
            if(camera_following)
            {
                speed_target = follow_command.speed_target;
            }
            else if(BALANCE_VOICE_FORWARD == g_balance_voice_mode)
            {
                speed_target = BALANCE_CONTROL_VOICE_SPEED;
            }
            else if(BALANCE_VOICE_BACKWARD == g_balance_voice_mode)
            {
                speed_target = -BALANCE_CONTROL_VOICE_SPEED;
            }
            else if(BALANCE_VOICE_TURN == g_balance_voice_mode)
            {
                speed_target = 0.0f;
            }
            else if(braking)
            {
                speed_target = 0.0f;
            }
            else if(remote_active)
            {
                int16_t throttle = uart_receiver.throttle;
                int16_t abs_throttle = (throttle >= 0) ? throttle : (int16_t)(-throttle);
                if(abs_throttle < BALANCE_CONTROL_REMOTE_DEAD_ZONE)
                {
                    throttle = 0;
                }
                speed_target = (float)throttle;
            }
            brushless_driver_status_t motor_status;
            brushless_driver_get_status(&motor_status);
            float left_speed = fabsf((float)motor_status.left_speed);
            float right_speed = fabsf((float)motor_status.right_speed);
            float car_speed = (left_speed + right_speed) * 0.5f;
            if(BALANCE_BRAKE_LONGITUDINAL == g_balance_brake_mode)
            {
                car_speed = ((float)motor_status.left_speed +
                             (float)motor_status.right_speed) * 0.5f;
            }
            else if(BALANCE_BRAKE_TURN == g_balance_brake_mode)
            {
                car_speed = 0.0f;
            }
            else if(speed_target < 0.0f)
            {
                car_speed = -car_speed;
            }
            else if(camera_following && (fabsf(speed_target) < 0.01f))
            {
                /* 鎽勫儚澶达紙瀹氭椂琛岄┒锛塇OLD 鍋滄闃舵锛氱敤甯︾鍙疯疆閫熷弽棣堝仛涓诲姩鍒跺姩銆?
                 * 鑻ュ彧鐢?|杞€焲 鍙嶉锛屼竴鏃﹀埗鍔ㄥ姏鎶婅溅鎺ㄥ緱鍊掓簻锛寍杞€焲 浠嶄负姝ｏ紝
                 * 鑵夸細涓€鐩村弽鍚戝帇杞︼紝閫犳垚"鍋滄鍚庡線鍚庨€€"鐨勫け鎺у€掕溅锛?
                 * 鐢ㄥ墠杩涙柟鍚戞爣瀹?g_balance_cam_speed_sign)褰掍竴鍖栧悗鐨勫甫绗﹀彿杞€燂紝
                 * 鍊掓簻鏃惰宸嚜鍔ㄥ弽鍙凤紝鑵垮洖姝ｆ妸杞︽媺鍋溿€?*/
                float raw_avg = ((float)motor_status.left_speed +
                                 (float)motor_status.right_speed) * 0.5f;
                if(g_balance_cam_speed_sign > 0)
                {
                    car_speed = raw_avg;
                }
                else if(g_balance_cam_speed_sign < 0)
                {
                    car_speed = -raw_avg;
                }
                else
                {
                    car_speed = (left_speed + right_speed) * 0.5f;   /* 鏈爣瀹氾細閫€鍥?|杞€焲 */
                }
            }
            else if((!camera_following) && (fabsf(speed_target) < 0.01f))
            {
                /* 璇煶杞悜绛夊師鍦板姩浣滐細0 鐩爣鏃舵妸杞€熷弽棣堟竻闆讹紝閬垮厤鑵胯鍒跺姩銆?*/
                car_speed = 0.0f;
            }
            else if(camera_following)
            {
                /* 琛岄┒闃舵锛氭爣瀹?鍓嶈繘鏂瑰悜"鐨勮疆閫熺鍙凤紙涓嶄緷璧栭┍鍔ㄦ澘姝ｈ礋绾﹀畾锛夛紝
                 * 渚?HOLD 鍒跺姩闃舵褰掍竴鍖栦娇鐢ㄣ€?*/
                float raw_avg = ((float)motor_status.left_speed +
                                 (float)motor_status.right_speed) * 0.5f;
                if(fabsf(raw_avg) > 10.0f)
                {
                    int16_t new_sign = (int16_t)((raw_avg > 0.0f) ? 1 : -1);
                    if(new_sign != g_balance_cam_speed_sign)
                    {
                        printf("[BAL] cam speed sign=%d\r\n", (int)new_sign);
                        g_balance_cam_speed_sign = new_sign;
                    }
                }
            }

            if(braking)
            {
                g_balance_brake_ticks++;
                if(((left_speed + right_speed) * 0.5f) <= BALANCE_CONTROL_BRAKE_SPEED_THRESHOLD)
                {
                    g_balance_brake_settle++;
                }
                else
                {
                    g_balance_brake_settle = 0U;
                }

                bool turn_brake_timeout = (BALANCE_BRAKE_TURN == g_balance_brake_mode) &&
                                          (g_balance_brake_ticks >= BALANCE_CONTROL_BRAKE_TURN_MAX_TICKS);
                bool longitudinal_reversed = (BALANCE_BRAKE_LONGITUDINAL == g_balance_brake_mode) &&
                                             ((car_speed * (float)g_balance_brake_direction) <= 0.0f);
                if(longitudinal_reversed ||
                   (g_balance_brake_settle >= BALANCE_CONTROL_BRAKE_SETTLE_TICKS) ||
                   turn_brake_timeout ||
                   (g_balance_brake_ticks >= BALANCE_CONTROL_BRAKE_TIMEOUT_TICKS))
                {
                    balance_finish_brake();
                    braking = false;
                    car_speed = 0.0f;
                }
            }

            imu_pid_control(&imu_roll_balance.speed_cycle, speed_target, car_speed);
            /* HOLD 鍋滄闃舵锛堟憚鍍忓ご瀹氭椂琛岄┒锛宻peed_target=0锛変笌涓诲姩鍒跺姩鍏辩敤鏇寸嫚鐨勮吙鍘嬶細
             * leg_divisor 28鈫?2锛岄檺骞?60鈫?0锛屽埗鍔ㄥ姏鏇村己锛岃В鍐?鍑忛€熸參銆佹粦琛岃繙"鍋滀笉涓嬫潵銆?*/
            bool cam_hold = camera_following && (fabsf(speed_target) < 0.01f);
            float leg_divisor = (braking || cam_hold) ? BALANCE_CONTROL_BRAKE_LEG_DIVISOR : 28.0f;
            float leg_limit = cam_hold ? 80.0f : 60.0f;
            float leg_output = balance_limit_float(imu_roll_balance.speed_cycle.out / leg_divisor,
                                                   -leg_limit,
                                                   leg_limit);
            if(braking)
            {
                g_balance_steer_filter = leg_output;
            }
            else if(cam_hold)
            {
                /* TOF 璺熼殢绱ф€ュ埗鍔紙speed=0锛夛細姣旀甯歌椹舵洿蹇搷搴旓紙0.5/0.5 vs 0.8/0.2锛夛紝
                 * 浣嗕笉鍋氬埌鍗虫椂锛坆raking 妯″紡锛夛紝閬垮厤鑵跨獊鍙樺鑷村钩琛″け绋炽€?*/
                g_balance_steer_filter = (g_balance_steer_filter * 0.5f) + (leg_output * 0.5f);
            }
            else
            {
                g_balance_steer_filter = (g_balance_steer_filter * 0.8f) + (leg_output * 0.2f);
            }
        }
        else
        {
            balance_reset_speed_pid();
        }

        int16_t leg_us = (int16_t) g_balance_steer_filter;
        servo_hold_set_pulse(0, (uint16_t)((int32_t) SERVO_HOLD_PULSE_US + leg_us));
        servo_hold_set_pulse(1, (uint16_t)((int32_t) SERVO_HOLD_PULSE_US - leg_us));
        servo_hold_set_pulse(2, (uint16_t)((int32_t) SERVO_HOLD_PULSE_US + leg_us));
        servo_hold_set_pulse(3, (uint16_t)((int32_t) SERVO_HOLD_PULSE_US - leg_us));
    }
    g_balance_replaying = replaying;
    g_balance_status.shutdown = g_balance_shutdown;

    imu_pid_control(&imu_roll_balance.angle_cycle,
                    BALANCE_CONTROL_TARGET_PITCH_DEG - imu_roll_balance.posture_value.mechanical_zero,
                    -imu_roll_balance.posture_value.pit);
    g_balance_status.angle_target = BALANCE_CONTROL_TARGET_PITCH_DEG -
                                    imu_roll_balance.posture_value.mechanical_zero;
    imu_pid_control(&imu_roll_balance.angular_speed_cycle,
                    imu_roll_balance.angle_cycle.out,
                    (float)imu660rb_gyro_y);

    g_balance_status.angle_output = imu_roll_balance.angle_cycle.out;
    g_balance_status.angular_speed_output = imu_roll_balance.angular_speed_cycle.out;

    int16_t duty = balance_filter_duty(balance_limit_duty(imu_roll_balance.angular_speed_cycle.out));

    /* 鎯宸€熻浆鍚戯細Final_Out 淇鑸悜锛孴RIM 鎶垫秷杞︿綋/杞粍澶╃劧璺戝亸銆?*/
    int16_t turn = 0;
    if(camera_following)
    {
        turn = follow_command.turn;
    }
    else if(BALANCE_BRAKE_TURN == g_balance_brake_mode)
    {
        /* 鍘熷湴杞悜鍋滆溅锛氭柦鍔犱笌鍘熸棆杞柟鍚戠浉鍙嶇殑宸€熸壄鐭┿€?*/
        turn = (int16_t)(-g_balance_brake_direction * BALANCE_CONTROL_BRAKE_TURN_DUTY);
    }
    else if(voice_motion)
    {
        if((BALANCE_VOICE_FORWARD == g_balance_voice_mode) ||
           (BALANCE_VOICE_BACKWARD == g_balance_voice_mode))
        {
            float heading_error = balance_angle_error(imu_roll_balance.posture_value.yaw,
                                                      g_balance_voice_heading_target);
            turn = (int16_t) balance_limit_float(heading_error * BALANCE_CONTROL_VOICE_HEADING_GAIN,
                                                 -250.0f,
                                                 250.0f);
        }
        else if(BALANCE_VOICE_TURN == g_balance_voice_mode)
        {
            float current_yaw = imu_roll_balance.posture_value.yaw;
            float yaw_step = balance_angle_error(current_yaw, g_balance_voice_turn_last_yaw);
            g_balance_voice_turn_last_yaw = current_yaw;

            /* 绱鐩搁偦閲囨牱闂寸殑瀹為檯杞锛屽彲鑷劧璺ㄨ秺 +180/-180 杈圭晫锛屼篃涓嶄細鍥犺秺杩?
             * 鐩爣瑙掑悗璇樊鍐嶆鍙樺ぇ鑰岀户缁粫鍦堛€傚拷鐣ュ緢灏忕殑濮挎€佸櫔澹帮紝闃叉闈欐鏃剁疮鍔犮€?*/
            if(fabsf(yaw_step) >= 0.05f)
            {
                g_balance_voice_turn_accumulated += fabsf(yaw_step);
            }

            float turn_remaining = BALANCE_CONTROL_VOICE_TURN_DEG - g_balance_voice_turn_accumulated;
            if(g_balance_voice_turn_accumulated >= BALANCE_CONTROL_VOICE_TURN_BRAKE_DEG)
            {
                int16_t completed_spin_direction = g_balance_voice_spin_dir;
                balance_start_turn_brake(completed_spin_direction);
                /* 鍒拌鐨勫綋鍓嶆帶鍒跺懆鏈熺珛鍗冲弽鍚戝埗鍔紝涓嶅啀澶氳浆涓€涓懆鏈熴€?*/
                turn = (int16_t)(-completed_spin_direction * BALANCE_CONTROL_BRAKE_TURN_DUTY);
            }
            else
            {
                g_balance_voice_turn_ticks++;
                if(g_balance_voice_turn_ticks >= BALANCE_CONTROL_VOICE_TURN_TIMEOUT)
                {
                    /* 瓒呮椂涔熻繘鍏ュ弽鍚戝埗鍔紝閬垮厤鏈€鍚庝竴涓棆杞寚浠ょ户缁┍鍔ㄨ疆瀛愩€?*/
                    int16_t timed_out_spin_direction = g_balance_voice_spin_dir;
                    balance_start_turn_brake(timed_out_spin_direction);
                    turn = (int16_t)(-timed_out_spin_direction * BALANCE_CONTROL_BRAKE_TURN_DUTY);
                }
                else
                {
                    int16_t spin = (int16_t)(g_balance_voice_spin_dir * BALANCE_CONTROL_VOICE_SPIN_DUTY);
                    float slow_k = turn_remaining / BALANCE_CONTROL_VOICE_TURN_SLOW_DEG;
                    if(slow_k > 1.0f)
                    {
                        slow_k = 1.0f;
                    }
                    turn = (int16_t)((float)spin * slow_k);
                    if((turn > 0) && (turn < BALANCE_CONTROL_VOICE_SPIN_MIN_DUTY))
                    {
                        turn = BALANCE_CONTROL_VOICE_SPIN_MIN_DUTY;
                    }
                    else if((turn < 0) && (turn > -BALANCE_CONTROL_VOICE_SPIN_MIN_DUTY))
                    {
                        turn = -BALANCE_CONTROL_VOICE_SPIN_MIN_DUTY;
                    }
                }
            }
        }
    }
    else if(voice_override)
    {
        turn = 0;
    }
    else if(remote_active)
    {
        int16_t steering = uart_receiver.steering;
        int16_t abs_steering = (steering >= 0) ? steering : (int16_t)(-steering);
        if(abs_steering < BALANCE_CONTROL_REMOTE_DEAD_ZONE)
        {
            steering = 0;
        }
        turn = (int16_t) balance_limit_float((float)steering * BALANCE_CONTROL_REMOTE_TURN_GAIN,
                                             -BALANCE_CONTROL_REMOTE_TURN_MAX,
                                             BALANCE_CONTROL_REMOTE_TURN_MAX);
    }
    else
    {
        turn = (int16_t) balance_limit_float((nag_navigation_get_final_out() * nag_navigation_turn_gain) +
                                             nag_navigation_turn_trim,
                                             -250.0f,
                                             250.0f);
    }

    g_balance_status.left_duty  = balance_limit_duty((int16_t)(-duty + turn));
    g_balance_status.right_duty = balance_limit_duty((int16_t)(-duty - turn));

    if(balance_motor_send_due())
    {
        fsp_err_t err = brushless_driver_set_duty(g_balance_status.left_duty,
                                                  g_balance_status.right_duty);
        if(FSP_ERR_IN_USE != err)
        {
            g_balance_status.brushless_last_error = err;
        }
    }
}

void balance_control_get_status(balance_control_status_t *status)
{
    if(NULL != status)
    {
        *status = g_balance_status;
    }
}

bool balance_control_kill_active(void)
{
    return g_balance_shutdown;
}
