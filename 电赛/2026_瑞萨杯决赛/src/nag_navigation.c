#include "nag_navigation.h"

#include "brushless_driver.h"
#include "imu_process.h"
#include "nag_flash.h"

/* ===== 鍙皟鍙傛暟 ===== */
#define NAG_MAX_SIZE              (500U)     /* 姣忛〉缂撳啿璁板綍鏉℃暟锛屽啓婊″嵆钀戒竴椤?*/
#define NAG_READ_MAX_SIZE         (10000U)   /* 澶嶇幇鐢?Nav_read 鏁扮粍澶у皬 */
#define NAG_END_PAGE              (1U)       /* 鍏冩暟鎹〉锛氬瓨鎬荤偣鏁?Save_index */
#define NAG_START_PAGE            (95U)      /* 鏁版嵁璧峰椤碉紝浠?95 寰€涓嬪啓 */
#define NAG_SET_MILEAGE           (5.0f)     /* 姣忕疮璁″灏戦噷绋嬭涓€涓偣锛坈m锛?*/
#define NAG_WHEEL_DIAMETER_CM     (6.4f)     /* 杞﹁疆鐩村緞锛坈m锛夛紝瀵归綈 CYT4BB7 */
#define NAG_PI                    (3.14159265f)
#define NAG_UPDATE_PERIOD_S       (0.005f)   /* 姣?5ms 璋冪敤涓€娆?*/
#define NAG_DEFAULT_TURN_GAIN     (-8.5f)

/* 鍏冩暟鎹湪椤靛唴鐨勫浐瀹氬亸绉伙細Save_index 瀛樺湪涓嬫爣 NAG_MAX_SIZE+2 澶勶紙瀵归綈 CYT4BB7锛?*/
#define NAG_META_INDEX            (NAG_MAX_SIZE + 2U)

typedef enum
{
    NAG_SYS_IDLE   = 0,
    NAG_SYS_RECORD = 1,
    NAG_SYS_REPLAY = 3,
    NAG_SYS_STRAIGHT = 4,
} nag_sys_index_t;

typedef struct
{
    float Final_Out;
    float Mileage_All;
    float Angle_Run;
    uint16_t size;
    uint16_t Run_index;
    uint16_t Save_index;
    uint8_t Save_state;
    uint8_t Flash_page_index;
    uint8_t sys_index;
    bool Nag_Stop_f;
} nag_navigation_t;

static nag_navigation_t g_nag;
static float g_nag_nav_read[NAG_READ_MAX_SIZE];

float nag_navigation_turn_gain = NAG_DEFAULT_TURN_GAIN;
float nag_navigation_turn_trim = 60.0f;
float nag_navigation_travel_speed = 400.0f;

static float nag_absf(float value)
{
    return (value < 0.0f) ? -value : value;
}

/* 瑙掑害褰掍竴鍒?(-180, 180] */
static double nag_angle_plan(double angle)
{
    while(angle > 180.0)
    {
        angle -= 360.0;
    }
    while(angle <= -180.0)
    {
        angle += 360.0;
    }
    return angle;
}

/* 鐢ㄦ棤鍒峰乏鍙宠疆閫熷害鍙嶉绉垎閲岀▼銆? * 褰曞埗/澶嶇幇鍙渶瑕佲€滆蛋杩囦簡澶氬皯璺濈鈥濓紝涓嶉渶瑕佹柟鍚戠鍙枫€傝繖閲岀敤宸﹀彸杞€熷害缁濆鍊肩殑骞冲潎鍊硷紝
 * 閬垮厤宸﹀彸鐢垫満鍙嶉涓€姝ｄ竴璐熴€佹垨鍙嶅悜鎺ㄨ溅鏃堕噷绋嬭鎶垫秷鎴?0銆?*/
static void nag_accumulate_mileage(void)
{
    brushless_driver_status_t st;
    brushless_driver_get_status(&st);

    float car_speed_rpm = (nag_absf((float) st.left_speed) + nag_absf((float) st.right_speed)) * 0.5f;
    g_nag.Mileage_All += (car_speed_rpm / 60.0f) * NAG_WHEEL_DIAMETER_CM * NAG_PI * NAG_UPDATE_PERIOD_S;
}

/* 鎶婂綋鍓嶇紦鍐茶惤鐩樺埌褰撳墠鏁版嵁椤?*/
static void nag_write_data_page(void)
{
    if(nag_flash_check(0, g_nag.Flash_page_index))
    {
        nag_flash_erase_page(0, g_nag.Flash_page_index);
    }
    nag_flash_write_page_from_buffer(0, g_nag.Flash_page_index, NAG_FLASH_PAGE_LENGTH);
    nag_flash_buffer_clear();
}

/* 鎶婃€荤偣鏁?Save_index 鍐欏埌鍏冩暟鎹〉 */
static void nag_save_meta(void)
{
    nag_flash_buffer_clear();
    nag_flash_union_buffer[NAG_META_INDEX].uint32_type = (uint32_t) g_nag.Save_index;

    if(nag_flash_check(0, NAG_END_PAGE))
    {
        nag_flash_erase_page(0, NAG_END_PAGE);
    }
    nag_flash_write_page_from_buffer(0, NAG_END_PAGE, NAG_FLASH_PAGE_LENGTH);
    nag_flash_buffer_clear();
}

/* 浠?RAM flash 鎶婃墍鏈夊綍鍒剁偣杞藉叆 Nav_read锛屽苟鎭㈠ Save_index */
static void nag_load_records(void)
{
    nag_flash_read_page_to_buffer(0, NAG_END_PAGE, NAG_FLASH_PAGE_LENGTH);
    uint32_t raw = nag_flash_union_buffer[NAG_META_INDEX].uint32_type;
    g_nag.Save_index = (raw != 0xFFFFFFFFU) ? (uint16_t) raw : 0U;

    uint16_t remaining = g_nag.Save_index;
    uint8_t page = NAG_START_PAGE;
    uint16_t dest = 0;

    while((remaining > 0U) && (page > NAG_END_PAGE) && (dest < NAG_READ_MAX_SIZE))
    {
        nag_flash_read_page_to_buffer(0, page, NAG_FLASH_PAGE_LENGTH);

        uint16_t n = (remaining > (uint16_t) NAG_MAX_SIZE) ? (uint16_t) NAG_MAX_SIZE : remaining;
        for(uint16_t i = 0; i < n; i++)
        {
            g_nag_nav_read[dest++] = (float) nag_flash_union_buffer[i].int32_type;
        }
        remaining = (uint16_t)(remaining - n);
        page--;
    }
    g_nag.Save_state = 1;
}

/* 褰曞埗锛氭瘡 5cm 璁颁竴涓?yaw锛坸100 瀛?int32锛?*/
static void nag_run_save(void)
{
    nag_accumulate_mileage();

    if(g_nag.size >= NAG_MAX_SIZE)
    {
        nag_write_data_page();
        g_nag.size = 0;
        if(g_nag.Flash_page_index > NAG_END_PAGE)
        {
            g_nag.Flash_page_index--;
        }
    }

    if(g_nag.Mileage_All >= NAG_SET_MILEAGE)
    {
        int32_t save = (int32_t)(imu_roll_balance.posture_value.yaw * 100.0f);
        nag_flash_union_buffer[g_nag.size++].int32_type = save;
        g_nag.Save_index++;
        g_nag.Mileage_All -= NAG_SET_MILEAGE;
    }
}

/* 澶嶇幇锛氭瘡 5cm 鍓嶈繘鍒颁笅涓€涓綍鍒剁偣锛岃緭鍑烘湞鍚戝亸宸?*/
static void nag_run_replay(void)
{
    nag_accumulate_mileage();

    if(g_nag.Mileage_All < NAG_SET_MILEAGE)
    {
        g_nag.Final_Out = (float) nag_angle_plan((double) imu_roll_balance.posture_value.yaw -
                                                 (double) g_nag.Angle_Run);
        return;
    }

    g_nag.Mileage_All -= NAG_SET_MILEAGE;

    if(g_nag.Run_index >= (uint16_t)(g_nag.Save_index - 1U))
    {
        /* 鎵€鏈夊綍鍒剁偣宸茶蛋瀹?*/
        g_nag.Nag_Stop_f = true;
        g_nag.Final_Out = 0.0f;
        return;
    }

    g_nag.Run_index++;
    g_nag.Angle_Run = g_nag_nav_read[g_nag.Run_index] / 100.0f;
    g_nag.Final_Out = (float) nag_angle_plan((double) imu_roll_balance.posture_value.yaw -
                                             (double) g_nag.Angle_Run);
}

static void nag_run_straight(void)
{
    nag_accumulate_mileage();
    g_nag.Final_Out = (float) nag_angle_plan((double) imu_roll_balance.posture_value.yaw -
                                             (double) g_nag.Angle_Run);
}

void nag_navigation_init(void)
{
    memset(&g_nag, 0, sizeof(g_nag));
    nag_flash_init();
    g_nag.Flash_page_index = NAG_START_PAGE;
    nag_navigation_turn_gain = NAG_DEFAULT_TURN_GAIN;
    nag_navigation_turn_trim = 60.0f;
    nag_navigation_travel_speed = 400.0f;
}

void nag_navigation_start_record(void)
{
    g_nag.size = 0;
    g_nag.Save_index = 0;
    g_nag.Run_index = 0;
    g_nag.Mileage_All = 0.0f;
    g_nag.Angle_Run = 0.0f;
    g_nag.Final_Out = 0.0f;
    g_nag.Save_state = 0;
    g_nag.Nag_Stop_f = false;
    g_nag.Flash_page_index = NAG_START_PAGE;

    /* 娓呯┖鏁版嵁椤?+ 鍏冩暟鎹〉锛屼繚璇佸共鍑€ */
    for(uint8_t p = NAG_END_PAGE; p <= NAG_START_PAGE; p++)
    {
        nag_flash_erase_page(0, p);
    }
    nag_flash_buffer_clear();

    g_nag.sys_index = NAG_SYS_RECORD;
}

void nag_navigation_stop_record(void)
{
    if(NAG_SYS_RECORD != g_nag.sys_index)
    {
        return;
    }

    nag_write_data_page();
    nag_save_meta();
    g_nag.sys_index = NAG_SYS_IDLE;
}

void nag_navigation_start_replay(void)
{
    nag_load_records();

    if(g_nag.Save_index == 0U)
    {
        return; /* 娌℃湁鍙鐜扮殑鏁版嵁 */
    }

    g_nag.Run_index = 0;
    g_nag.Mileage_All = 0.0f;
    g_nag.Angle_Run = g_nag_nav_read[0] / 100.0f;
    g_nag.Final_Out = 0.0f;
    g_nag.Nag_Stop_f = false;
    g_nag.sys_index = NAG_SYS_REPLAY;
}

void nag_navigation_start_straight(void)
{
    g_nag.Run_index = 0;
    g_nag.Mileage_All = 0.0f;
    g_nag.Angle_Run = imu_roll_balance.posture_value.yaw;
    g_nag.Final_Out = 0.0f;
    g_nag.Nag_Stop_f = false;
    nag_navigation_travel_speed = 150.0f;
    g_nag.sys_index = NAG_SYS_STRAIGHT;
}

void nag_navigation_update(void)
{
    switch(g_nag.sys_index)
    {
        case NAG_SYS_RECORD:
            nag_run_save();
            break;
        case NAG_SYS_REPLAY:
            nag_run_replay();
            break;
        case NAG_SYS_STRAIGHT:
            nag_run_straight();
            break;
        default:
            break;
    }
}

float nag_navigation_get_final_out(void)
{
    return g_nag.Final_Out;
}

nag_navigation_state_t nag_navigation_get_state(void)
{
    switch(g_nag.sys_index)
    {
        case NAG_SYS_RECORD:
            return NAG_STATE_RECORDING;
        case NAG_SYS_REPLAY:
            return g_nag.Nag_Stop_f ? NAG_STATE_IDLE : NAG_STATE_REPLAYING;
        case NAG_SYS_STRAIGHT:
            return NAG_STATE_STRAIGHT;
        default:
            return NAG_STATE_IDLE;
    }
}

uint16_t nag_navigation_get_record_count(void)
{
    return g_nag.Save_index;
}

float nag_navigation_get_mileage(void)
{
    return g_nag.Mileage_All;
}

float nag_navigation_get_angle_run(void)
{
    return g_nag.Angle_Run;
}
