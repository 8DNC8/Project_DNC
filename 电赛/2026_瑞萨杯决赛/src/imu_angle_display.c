#include "imu_angle_display.h"

#include "brushless_driver.h"
#include "imu_process.h"
#include "key_tune.h"
#include "nag_navigation.h"
#include "remote_control.h"
#include "voice_control.h"

#define REMOTE_DISPLAY_INTERVAL_TICKS (20U)

static uint32_t imu_angle_display_tick_count = 0;
static key_tune_page_t imu_angle_display_last_page = KEY_TUNE_PAGE_MAX;
static bool imu_angle_display_remote_mode = false;

static void imu_angle_display_show_1dec(uint16 x, uint16 y, float value)
{
    char text[8];
    int32_t scaled = (int32_t)(value * 10.0f);

    if(scaled < -9999)
    {
        scaled = -9999;
    }
    else if(scaled > 9999)
    {
        scaled = 9999;
    }

    uint32_t abs_scaled = (uint32_t)((scaled < 0) ? -scaled : scaled);
    uint32_t integer = abs_scaled / 10U;
    uint32_t decimal = abs_scaled % 10U;

    text[0] = (scaled < 0) ? '-' : ' ';
    text[1] = (char)('0' + (integer / 100U) % 10U);
    text[2] = (char)('0' + (integer / 10U) % 10U);
    text[3] = (char)('0' + integer % 10U);
    text[4] = '.';
    text[5] = (char)('0' + decimal);
    text[6] = ' ';
    text[7] = '\0';

    ips200_show_string(x, y, text);
}

static void imu_angle_display_show_2dec(uint16 x, uint16 y, float value)
{
    char text[9];
    int32_t scaled = (int32_t)(value * 100.0f);

    if(scaled < -9999)
    {
        scaled = -9999;
    }
    else if(scaled > 99999)
    {
        scaled = 99999;
    }

    uint32_t abs_scaled = (uint32_t)((scaled < 0) ? -scaled : scaled);
    uint32_t integer = abs_scaled / 100U;
    uint32_t decimal = abs_scaled % 100U;

    text[0] = (scaled < 0) ? '-' : ' ';
    text[1] = (char)('0' + (integer / 100U) % 10U);
    text[2] = (char)('0' + (integer / 10U) % 10U);
    text[3] = (char)('0' + integer % 10U);
    text[4] = '.';
    text[5] = (char)('0' + (decimal / 10U) % 10U);
    text[6] = (char)('0' + decimal % 10U);
    text[7] = ' ';
    text[8] = '\0';

    ips200_show_string(x, y, text);
}

static void imu_angle_display_show_int6(uint16 x, uint16 y, int32_t value)
{
    char text[8];

    if(value < -99999)
    {
        value = -99999;
    }
    else if(value > 999999)
    {
        value = 999999;
    }

    uint32_t abs_value = (uint32_t)((value < 0) ? -value : value);
    text[0] = (value < 0) ? '-' : ' ';
    text[1] = (char)('0' + (abs_value / 10000U) % 10U);
    text[2] = (char)('0' + (abs_value / 1000U) % 10U);
    text[3] = (char)('0' + (abs_value / 100U) % 10U);
    text[4] = (char)('0' + (abs_value / 10U) % 10U);
    text[5] = (char)('0' + abs_value % 10U);
    text[6] = ' ';
    text[7] = '\0';

    ips200_show_string(x, y, text);
}

static void imu_angle_display_show_pid_value(uint16 x, uint16 y, key_tune_item_t item)
{
    float value = key_tune_get_item_value(item);

    if((KEY_TUNE_ITEM_GYRO_P == item) || (KEY_TUNE_ITEM_ZERO == item))
    {
        imu_angle_display_show_2dec(x, y, value);
    }
    else
    {
        imu_angle_display_show_1dec(x, y, value);
    }
}

static void imu_angle_display_clear_page_rows(void)
{
    /* 40 个空格：8x16 字体下 40*8=320px，正好铺满横屏整行宽度 */
    static const char blank_row[41] = "                                        ";
    static const uint16 rows[] = {10U, 45U, 55U, 75U, 95U, 100U, 125U, 130U, 150U, 165U, 175U, 195U, 205U, 215U};

    for(uint32 i = 0; i < (sizeof(rows) / sizeof(rows[0])); i++)
    {
        ips200_show_string(0, rows[i], blank_row);
    }
}

/* 右上角显示当前页页码：PG:0 / PG:1 ...，与左上标题配合指示所在页 */
static void imu_angle_display_draw_page_indicator(key_tune_page_t page)
{
    char txt[5];

    txt[0] = 'P';
    txt[1] = 'G';
    txt[2] = ':';
    txt[3] = (char)('0' + ((int)page));   /* 当前页号 0..4，单数字 */
    txt[4] = '\0';

    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(288, 10, txt);
}

/* RUN 页最后一行：黄色显示语音当前状态（上电默认 Status:balance，说"前进"变为 Status:GO 等）。
 * 仅 RUN 页调用，其它页不显示状态行。由 voice_control_display() 负责绘制。 */
static void imu_angle_display_draw_run_status_line(void)
{
    voice_control_display();
}

static void imu_angle_display_draw_run_page(void)
{
    imu_angle_display_clear_page_rows();
    ips200_show_string(10, 10, "PAGE0 RUN");
    imu_angle_display_draw_page_indicator(KEY_TUNE_PAGE_RUN);
    ips200_show_string(10, 55, "Yaw:");
    ips200_show_string(10, 95, "Pit:");
    ips200_show_string(10, 130, "LS:");
    ips200_show_string(125, 130, "RS:");
    ips200_show_string(10, 165, "K4 PAGE");
    imu_angle_display_draw_run_status_line();
}

static void imu_angle_display_draw_pid_page(void)
{
    imu_angle_display_clear_page_rows();
    ips200_show_string(10, 10, "PAGE1 PID");
    imu_angle_display_draw_page_indicator(KEY_TUNE_PAGE_PID);
    ips200_show_string(10, 45, "K3 ITEM  K2+ K1-");
    ips200_show_string(35, 75, "AP:");
    ips200_show_string(35, 100, "AI:");
    ips200_show_string(35, 125, "AD:");
    ips200_show_string(35, 150, "GP:");
    ips200_show_string(35, 175, "Z0:");
    ips200_show_string(10, 210, "K4 NEXT");
}

static void imu_angle_display_draw_imu_page(void)
{
    imu_angle_display_clear_page_rows();
    ips200_show_string(10, 10, "PAGE2 IMU");
    imu_angle_display_draw_page_indicator(KEY_TUNE_PAGE_IMU);
    ips200_show_string(10, 45, "K3 ITEM  K2+ K1-");
    ips200_show_string(35, 75, "CKP:");
    ips200_show_string(35, 100, "CKI:");
    ips200_show_string(35, 125, "Yaw:");
    ips200_show_string(35, 150, "Pit:");
    ips200_show_string(35, 175, "Rol:");
    ips200_show_string(10, 210, "K4 NEXT");
}

static void imu_angle_display_draw_nag_page(void)
{
    imu_angle_display_clear_page_rows();
    ips200_show_string(10, 10, "PAGE3 NAG");
    imu_angle_display_draw_page_indicator(KEY_TUNE_PAGE_NAG);
    ips200_show_string(10, 45, "K3 ITEM  K1 GO  K4 NEXT");
    ips200_show_string(35, 75, "REC");
    ips200_show_string(35, 100, "SAV");
    ips200_show_string(35, 125, "REP");
    ips200_show_string(10, 175, "ST:");
    ips200_show_string(95, 175, "MI:");
    ips200_show_string(185, 175, "CNT:");
    ips200_show_string(10, 205, "FO:");
}

static void imu_angle_display_draw_str_page(void)
{
    imu_angle_display_clear_page_rows();
    ips200_show_string(10, 10, "PAGE4 STR");
    imu_angle_display_draw_page_indicator(KEY_TUNE_PAGE_STR);
    ips200_show_string(10, 45, "K3 ITEM  K1-  K2+");
    ips200_show_string(35, 75, "STR");
    ips200_show_string(35, 100, "GAIN:");
    ips200_show_string(35, 125, "TRIM:");
    ips200_show_string(35, 150, "SPD:");
    ips200_show_string(10, 190, "ST:");
    ips200_show_string(95, 190, "Yaw:");
    ips200_show_string(10, 215, "FO:");
}

static void imu_angle_display_draw_remote_page(void)
{
    ips200_clear();

    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(104, 10, "REMOTE CONTROL");

    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_show_string(10, 45, "CH1:");
    ips200_show_string(165, 45, "CH2:");
    ips200_show_string(10, 75, "CH3:");
    ips200_show_string(165, 75, "CH4:");
    ips200_show_string(10, 105, "CH5:");
    ips200_show_string(165, 105, "CH6:");
    ips200_show_string(10, 145, "STEER:");
    ips200_show_string(165, 145, "THROTTLE:");
    ips200_show_string(10, 190, "LINK:");
    ips200_show_string(165, 190, "CAL:");
    ips200_show_string(10, 220, "CH3 RUN:");
}

static void imu_angle_display_update_remote_page(void)
{
    ips200_set_color(RGB565_WHITE, RGB565_BLACK);
    imu_angle_display_show_int6(50, 45, (int32_t)uart_receiver.channel[0]);
    imu_angle_display_show_int6(205, 45, (int32_t)uart_receiver.channel[1]);
    imu_angle_display_show_int6(50, 75, (int32_t)uart_receiver.channel[2]);
    imu_angle_display_show_int6(205, 75, (int32_t)uart_receiver.channel[3]);
    imu_angle_display_show_int6(50, 105, (int32_t)uart_receiver.channel[4]);
    imu_angle_display_show_int6(205, 105, (int32_t)uart_receiver.channel[5]);
    imu_angle_display_show_int6(65, 145, (int32_t)uart_receiver.steering);
    imu_angle_display_show_int6(245, 145, (int32_t)uart_receiver.throttle);

    if(0U != uart_receiver.state)
    {
        ips200_set_color(RGB565_GREEN, RGB565_BLACK);
        ips200_show_string(58, 190, "ONLINE ");
    }
    else
    {
        ips200_set_color(RGB565_RED, RGB565_BLACK);
        ips200_show_string(58, 190, "OFFLINE");
    }

    if(0U != uart_receiver.calibration_ready)
    {
        ips200_set_color(RGB565_GREEN, RGB565_BLACK);
        ips200_show_string(205, 190, "READY ");
    }
    else
    {
        ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
        ips200_show_string(205, 190, "WAIT  ");
    }

    if(0U != uart_receiver.run_enabled)
    {
        ips200_set_color(RGB565_GREEN, RGB565_BLACK);
        ips200_show_string(82, 220, "ON ");
    }
    else
    {
        ips200_set_color(RGB565_RED, RGB565_BLACK);
        ips200_show_string(82, 220, "OFF");
    }

    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
}

static void imu_angle_display_select_page(key_tune_page_t page)
{
    if(page == imu_angle_display_last_page)
    {
        return;
    }

    imu_angle_display_last_page = page;

    /* 切页先整屏清屏，再完整重画当前页，彻底消除上一页文字/数值残留。
     * 显式复位画笔为页面统一颜色（绿），避免摄像头/调试等残留其它笔色。 */
    ips200_clear();
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);

    switch(page)
    {
        case KEY_TUNE_PAGE_PID:
            imu_angle_display_draw_pid_page();
            break;
        case KEY_TUNE_PAGE_IMU:
            imu_angle_display_draw_imu_page();
            break;
        case KEY_TUNE_PAGE_NAG:
            imu_angle_display_draw_nag_page();
            break;
        case KEY_TUNE_PAGE_STR:
            imu_angle_display_draw_str_page();
            break;
        default:
            imu_angle_display_draw_run_page();
            break;
    }
}

static void imu_angle_display_show_select(uint16 y, key_tune_item_t item)
{
    ips200_show_string(10, y, key_tune_item_is_selected(item) ? "->" : "  ");
}

void imu_angle_display_init(void)
{
    imu_angle_display_tick_count = 0;
    imu_angle_display_last_page = KEY_TUNE_PAGE_RUN;
    imu_angle_display_remote_mode = false;
    ips200_clear();
    imu_angle_display_draw_run_page();
}

void imu_angle_display_update(float yaw, float pitch, float roll)
{
    (void) roll;

    bool remote_mode = uart_receiver_is_enabled();
    uint32_t display_interval = remote_mode ? REMOTE_DISPLAY_INTERVAL_TICKS :
                                              IMU_ANGLE_DISPLAY_INTERVAL_TICKS;
    if(remote_mode != imu_angle_display_remote_mode)
    {
        imu_angle_display_remote_mode = remote_mode;
        imu_angle_display_tick_count = display_interval - 1U;

        if(remote_mode)
        {
            imu_angle_display_draw_remote_page();
        }
        else
        {
            imu_angle_display_last_page = KEY_TUNE_PAGE_MAX;
            ips200_clear();
        }
    }

    imu_angle_display_tick_count++;
    if(imu_angle_display_tick_count < display_interval)
    {
        return;
    }
    imu_angle_display_tick_count = 0;

    if(imu_angle_display_remote_mode)
    {
        imu_angle_display_update_remote_page();
        return;
    }

    key_tune_status_t tune_status;
    key_tune_get_status(&tune_status);
    imu_angle_display_select_page(tune_status.page);

    if(KEY_TUNE_PAGE_PID == tune_status.page)
    {
        imu_angle_display_show_select(75, KEY_TUNE_ITEM_ANGLE_P);
        imu_angle_display_show_select(100, KEY_TUNE_ITEM_ANGLE_I);
        imu_angle_display_show_select(125, KEY_TUNE_ITEM_ANGLE_D);
        imu_angle_display_show_select(150, KEY_TUNE_ITEM_GYRO_P);
        imu_angle_display_show_select(175, KEY_TUNE_ITEM_ZERO);

        imu_angle_display_show_pid_value(75, 75, KEY_TUNE_ITEM_ANGLE_P);
        imu_angle_display_show_pid_value(75, 100, KEY_TUNE_ITEM_ANGLE_I);
        imu_angle_display_show_pid_value(75, 125, KEY_TUNE_ITEM_ANGLE_D);
        imu_angle_display_show_pid_value(75, 150, KEY_TUNE_ITEM_GYRO_P);
        imu_angle_display_show_pid_value(75, 175, KEY_TUNE_ITEM_ZERO);

        /* 右侧步进提示已关闭，避免占用 NAG/状态区显示空间 */
    }
    else if(KEY_TUNE_PAGE_IMU == tune_status.page)
    {
        imu_angle_display_show_select(75, KEY_TUNE_ITEM_CORRECT_KP);
        imu_angle_display_show_select(100, KEY_TUNE_ITEM_CORRECT_KI);

        imu_angle_display_show_2dec(75, 75, key_tune_get_item_value(KEY_TUNE_ITEM_CORRECT_KP));
        imu_angle_display_show_2dec(75, 100, key_tune_get_item_value(KEY_TUNE_ITEM_CORRECT_KI));

        imu_angle_display_show_1dec(65, 125, yaw);
        imu_angle_display_show_1dec(65, 150, pitch);
        imu_angle_display_show_1dec(65, 175, roll);
    }
    else if(KEY_TUNE_PAGE_NAG == tune_status.page)
    {
        imu_angle_display_show_select(75, KEY_TUNE_ITEM_NAG_RECORD);
        imu_angle_display_show_select(100, KEY_TUNE_ITEM_NAG_SAVE);
        imu_angle_display_show_select(125, KEY_TUNE_ITEM_NAG_REPLAY);

        switch(nag_navigation_get_state())
        {
            case NAG_STATE_RECORDING:
                ips200_show_string(35, 175, "REC");
                break;
            case NAG_STATE_REPLAYING:
                ips200_show_string(35, 175, "RUN");
                break;
            case NAG_STATE_STRAIGHT:
                ips200_show_string(35, 175, "STR");
                break;
            default:
                ips200_show_string(35, 175, "IDL");
                break;
        }
        imu_angle_display_show_1dec(120, 175, nag_navigation_get_mileage());
        imu_angle_display_show_int6(215, 175, (int32) nag_navigation_get_record_count());
        imu_angle_display_show_1dec(35, 205, nag_navigation_get_final_out());
    }
    else if(KEY_TUNE_PAGE_STR == tune_status.page)
    {
        imu_angle_display_show_select(75, KEY_TUNE_ITEM_NAG_STRAIGHT);
        imu_angle_display_show_select(100, KEY_TUNE_ITEM_NAG_TURN_GAIN);
        imu_angle_display_show_select(125, KEY_TUNE_ITEM_NAG_TURN_TRIM);
        imu_angle_display_show_select(150, KEY_TUNE_ITEM_NAG_SPEED);

        imu_angle_display_show_2dec(95, 100, nag_navigation_turn_gain);
        imu_angle_display_show_int6(95, 125, (int32) nag_navigation_turn_trim);
        imu_angle_display_show_int6(95, 150, (int32) nag_navigation_travel_speed);

        switch(nag_navigation_get_state())
        {
            case NAG_STATE_STRAIGHT:
                ips200_show_string(35, 190, "STR");
                break;
            default:
                ips200_show_string(35, 190, "IDL");
                break;
        }
        imu_angle_display_show_1dec(130, 190, yaw);
        imu_angle_display_show_1dec(35, 215, nag_navigation_get_final_out());
    }
    else
    {
        brushless_driver_status_t motor_status;
        brushless_driver_get_status(&motor_status);

        imu_angle_display_show_1dec(65, 55, yaw);
        imu_angle_display_show_1dec(65, 95, pitch);
        imu_angle_display_show_int6(40, 130, motor_status.left_speed);
        imu_angle_display_show_int6(155, 130, motor_status.right_speed);

        /* RUN 页最后一行：黄色状态行，随语音命令实时变化（Status:GO 等） */
        imu_angle_display_draw_run_status_line();
    }
}

void imu_angle_display_update_balance(void)
{
}

void imu_angle_display_force_refresh(void)
{
    imu_angle_display_last_page = KEY_TUNE_PAGE_MAX;
    imu_angle_display_remote_mode = false;
}
