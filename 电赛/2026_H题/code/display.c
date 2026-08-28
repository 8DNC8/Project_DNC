#include "display.h"
#include "gray.h"
#include "control.h"
#include "encoder.h"
#include "uart_k230.h"
#include "DUOJI.h"

uint16 page_id[PAGE_NUM];
uint8  now_page = 0;

/*
 * IPS200 2.0" landscape (320x240), 8x16 font, 40 cols x 15 rows.
 * Color: default green; PGx label + IR status = yellow.
 */

/* ---- top bar: title (green) + PGx (yellow) ---- */
static void topbar(const char *title, uint8 pg)
{
    char pg_str[4];
    pg_str[0] = 'P'; pg_str[1] = 'G'; pg_str[2] = '0' + pg; pg_str[3] = 0;

    /* title in default green */
    ips200_show_string(0, 0, title);

    /* PGx in yellow */
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);
    ips200_show_string(320 - 24, 0, pg_str);
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);

    uint8 i;
    for(i = 0; i < 40; i += 2)
        ips200_show_char(i * 8, 14, '-');
}

void display_init(void)
{
    ips200_set_dir(IPS200_CROSSWISE_180);
    ips200_init(IPS200_TYPE_SPI);
    ips200_set_font(IPS200_8X16_FONT);
    ips200_set_color(RGB565_GREEN, RGB565_BLACK);
    ips200_clear();

    page_id[0] = 0;
    page_id[1] = 1;
    page_id[2] = 2;
    page_id[3] = 3;
    page_id[4] = 4;
    page_id[5] = 5;
    page_id[6] = 6;

    ips200_clear();
}

/* ========== PG:0 STATUS + MODE MENU ========== */
static void display_status_page(void)
{
    char buf[20];
    uint8 i;

    topbar("STATUS", 0);

    /* ERR */
    ips200_show_string(0,  32, "ERR ");
    ips200_show_int   (40, 32, gray_offset, 4);

    /* ENC */
    ips200_show_string(0,   48, "ENC.L ");
    ips200_show_int   (56,  48, (int)enc_dist_cm[0], 4);
    ips200_show_string(104, 48, "cm");
    ips200_show_string(168, 48, "ENC.R ");
    ips200_show_int   (224, 48, (int)enc_dist_cm[1], 4);
    ips200_show_string(272, 48, "cm");

    /* TIME */
    {
        float lap_sec;
        if(car_move)
            lap_sec = (start_systime > 0) ? (float)(systime_20ms - start_systime) * 0.02f : 0.0f;
        else if(lap_done && start_systime > 0)
            lap_sec = (float)(lap_time_20ms - start_systime) * 0.02f;
        else
            lap_sec = 0.0f;
        ips200_show_string(0,   64, "TIME ");
        ips200_show_float (48,  64, lap_sec, 4, 1);
        ips200_show_string(88,  64, "s");
    }

    /* ---- 8-ch IR sensor status (yellow) ---- */
    ips200_set_color(RGB565_YELLOW, RGB565_BLACK);

    ips200_show_string(0, 80, "IR ");
    for(i = 0; i < GRAY_CH_NUM; i++)
    {
        buf[0] = '0' + gray_raw[i];
        buf[1] = 0;
        ips200_show_string(32 + i * 24, 80, buf);
    }

    ips200_set_color(RGB565_GREEN, RGB565_BLACK);

    /* MODE:1 .. MODE:5 menu with cursor */
    ips200_show_string(0, 98, "---------------MODE SEL---------------");

    for(i = 0; i < 5; i++)
    {
        if(i == menu_cursor)
            ips200_show_string(0, 114 + i * 16, ">");
        else
            ips200_show_string(0, 114 + i * 16, " ");

        buf[0] = 'M'; buf[1] = 'O'; buf[2] = 'D'; buf[3] = 'E';
        buf[4] = ':'; buf[5] = '0' + (i + 1); buf[6] = 0;
        ips200_show_string(16, 114 + i * 16, buf);
    }

    /* current task status */
    ips200_show_string(0, 200, "TASK ");
    if(task_mode == TASK_NONE)
        ips200_show_string(48, 200, "NONE ");
    else if(task_mode == TASK_MODE1)
        ips200_show_string(48, 200, "MODE:1");
    else if(task_mode == TASK_MODE2)
        ips200_show_string(48, 200, "MODE:2");
    else if(task_mode == TASK_MODE3)
        ips200_show_string(48, 200, "MODE:3");
    else if(task_mode == TASK_MODE4)
        ips200_show_string(48, 200, "MODE:4");
    else if(task_mode == TASK_MODE5)
        ips200_show_string(48, 200, "MODE:5");

    ips200_show_string(0, 224, "K2:v K3:^ K4:OK");
}

/* ========== PG:1 MODE:4 PID TUNE ========== */
static void display_mode4_tune_page(void)
{
    topbar("MODE4 PID", 1);

    ips200_show_string(0,  32, mode4_tune_item == 0 ? ">KP  " : " KP  ");
    ips200_show_int   (48, 32, mode4_line_kp, 4);

    ips200_show_string(0,  48, mode4_tune_item == 1 ? ">KI  " : " KI  ");
    ips200_show_int   (48, 48, mode4_line_ki, 4);

    ips200_show_string(0,  64, mode4_tune_item == 2 ? ">KD  " : " KD  ");
    ips200_show_int   (48, 64, mode4_line_kd, 4);

    ips200_show_string(0,  80, mode4_tune_item == 3 ? ">SPD " : " SPD ");
    ips200_show_int   (48, 80, mode4_speed_set, 4);

    ips200_show_string(0,  96, mode4_tune_item == 4 ? ">RUP " : " RUP ");
    ips200_show_int   (48, 96, mode4_ramp_up_ms, 5);
    ips200_show_string(96, 96, "ms");

    ips200_show_string(0, 112, mode4_tune_item == 5 ? ">RDN " : " RDN ");
    ips200_show_int   (48, 112, mode4_ramp_down_ms, 5);
    ips200_show_string(96, 112, "ms");

    ips200_show_string(0, 128, mode4_tune_item == 6 ? ">M4T " : " M4T ");
    ips200_show_int   (48, 128, mode4_stop_time_sec, 4);
    ips200_show_string(88, 128, "s");

    ips200_show_string(0, 224, "K2:>  K3:+  K4:-");
}

/* ========== PG:2 LINE TUNE ========== */
static void display_tune_page(void)
{
    topbar("LINE TUNE", 2);

    ips200_show_string(0,  32, tune_item == 0 ? ">SPD " : " SPD ");
    ips200_show_int   (48, 32, motor_speed_set, 4);

    ips200_show_string(0,  48, tune_item == 1 ? ">KP  " : " KP  ");
    ips200_show_int   (48, 48, line_track_kp, 4);

    ips200_show_string(0,  64, tune_item == 2 ? ">KD  " : " KD  ");
    ips200_show_int   (48, 64, line_track_kd, 4);

    ips200_show_string(0,  80, tune_item == 3 ? ">TLM " : " TLM ");
    ips200_show_int   (48, 80, line_turn_limit, 5);

    ips200_show_string(0,  96, tune_item == 4 ? ">LST " : " LST ");
    ips200_show_int   (48, 96, line_lost_turn, 5);

    ips200_show_string(152, 32, tune_item == 5 ? ">KPT " : " KPT ");
    ips200_show_int   (200, 32, line_track_kp_turn, 4);

    ips200_show_string(152, 48, tune_item == 6 ? ">KDT " : " KDT ");
    ips200_show_int   (200, 48, line_track_kd_turn, 4);

    ips200_show_string(152, 64, tune_item == 7 ? ">KIT " : " KIT ");
    ips200_show_int   (200, 64, line_track_ki_turn, 4);

    ips200_show_string(152, 80, tune_item == 8 ? ">TH  " : " TH  ");
    ips200_show_int   (200, 80, line_turn_err_th, 4);

    ips200_show_string(152, 96, tune_item == 9 ? ">XCN " : " XCN ");
    ips200_show_int   (200, 96, cross_black_thresh, 4);

    ips200_show_string(0, 224, "K2:>  K3:+  K4:-");
}

/* ========== PG:3 K230 VISION ========== */
static void display_k230_page(void)
{
    topbar("K230", 3);

    ips200_show_string(0,   32, "LINK ");
    ips200_show_string(48,  32, k230_link_ok ? "OK" : "NO");
    ips200_show_string(152, 32, "FRAME ");
    ips200_show_uint  (208, 32, k230_frame_cnt, 6);

    ips200_show_string(0,   48, "BALL ");
    ips200_show_string(48,  48, ball_online ? "YES" : "NO ");
    ips200_show_string(152, 48, "CNT ");
    ips200_show_int   (192, 48, ball_cnt, 3);

    ips200_show_string(0,   64, "B.X ");
    ips200_show_int   (40,  64, ball_x, 5);
    ips200_show_string(152, 64, "B.Y ");
    ips200_show_int   (192, 64, ball_y, 5);

    ips200_show_string(0,   80, "D10 ");
    ips200_show_int   (40,  80, ball_d10, 5);

    ips200_show_string(0,   96, "MD ");
    ips200_show_int   (32,  96, model, 2);
    ips200_show_string(152,96, "TGT ");
    ips200_show_int   (192,96, ball_target_d10, 5);

    ips200_show_string(0, 224, "K2:STOP        K4:TRACK");
}

/* ========== PG:4 SERVO TUNE ========== */
static void display_servo_tune_page(void)
{
    topbar("SERVO", 4);

    ips200_show_string(0,  32, servo_tune_item == 0 ? ">SVP " : " SVP "); ips200_show_int(48, 32, servo_p, 4);
    ips200_show_string(0,  48, servo_tune_item == 1 ? ">SVD " : " SVD "); ips200_show_int(48, 48, servo_d, 4);
    ips200_show_string(0,  64, servo_tune_item == 2 ? ">SVI " : " SVI "); ips200_show_int(48, 64, servo_i, 4);

    ips200_show_string(152, 32, servo_tune_item == 3 ? ">SX  " : " SX  "); ips200_show_int(200, 32, servo_set_x, 4);
    ips200_show_string(152, 48, servo_tune_item == 4 ? ">SY  " : " SY  "); ips200_show_int(200, 48, servo_set_y, 4);
    ips200_show_string(152, 64, servo_tune_item == 5 ? ">DIR " : " DIR ");
    ips200_show_int(200, 64, servo_manual_duty, 5);

    ips200_show_string(0, 224, "K2:>  K3:+  K4:-  SERVO");
}

/* ========== PG:5 Q4 BALL PID ========== */
static void display_q4_tune_page(void)
{
    topbar("Q4 BALL", 5);

    ips200_show_string(0,  32, q4_tune_item == 0 ? ">SVP " : " SVP "); ips200_show_int(48, 32, servo_p, 4);
    ips200_show_string(0,  48, q4_tune_item == 1 ? ">SVD " : " SVD "); ips200_show_int(48, 48, servo_d, 4);
    ips200_show_string(0,  64, q4_tune_item == 2 ? ">SVI " : " SVI "); ips200_show_int(48, 64, servo_i, 4);

    ips200_show_string(0,   96, "ERR ");
    ips200_show_int   (40,  96, ball_d10, 5);
    ips200_show_string(152, 96, "CNT ");
    ips200_show_int   (192, 96, ball_cnt, 3);

    ips200_show_string(0,  112, "BALL ");
    ips200_show_string(48, 112, ball_online ? "YES " : "NO  ");
    ips200_show_string(152,112, "LINK ");
    ips200_show_string(200,112, k230_link_ok ? "OK " : "NO ");

    ips200_show_string(0, 224, "K2:>  K3:+  K4:-  Q4 PID");
}

/* ========== PG:6 H3 OPEN LOOP TUNE ========== */
static void display_h3_tune_page(void)
{
    topbar("H3 TUNE", 6);

    ips200_show_string(0,  32, h3_tune_item == 0 ? ">D1  " : " D1  ");
    ips200_show_int(48, 32, h3_duty_offset[0], 5);
    ips200_show_string(152, 32, h3_tune_item == 3 ? ">T1  " : " T1  ");
    ips200_show_int(200, 32, h3_time_ms[0], 5);

    ips200_show_string(0,  48, h3_tune_item == 1 ? ">D2  " : " D2  ");
    ips200_show_int(48, 48, h3_duty_offset[1], 5);
    ips200_show_string(152, 48, h3_tune_item == 4 ? ">T2  " : " T2  ");
    ips200_show_int(200, 48, h3_time_ms[1], 5);

    ips200_show_string(0,  64, h3_tune_item == 2 ? ">D3  " : " D3  ");
    ips200_show_int(48, 64, h3_duty_offset[2], 5);
    ips200_show_string(152, 64, h3_tune_item == 5 ? ">T3  " : " T3  ");
    ips200_show_int(200, 64, h3_time_ms[2], 5);

    ips200_show_string(0,  96, h3_tune_item == 6 ? ">TGT " : " TGT ");
    ips200_show_int(48, 96, h3_follow_target, 5);
    ips200_show_string(152, 96, "PH ");
    ips200_show_int(184, 96, h3_get_phase(), 2);

    ips200_show_string(0, 224, "K2:>  K3:+  K4:-  H3");
}

/* ---- page dispatch ---- */
void display_update(void)
{
    if(now_page == 0)        display_status_page();
    else if(now_page == 1)   display_mode4_tune_page();
    else if(now_page == 2)   display_tune_page();
    else if(now_page == 3)   display_k230_page();
    else if(now_page == 4)   display_servo_tune_page();
    else if(now_page == 5)   display_q4_tune_page();
    else                     display_h3_tune_page();
}
