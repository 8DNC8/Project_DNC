#include "gray.h"
#include "motor.h"
#include "control.h"

uint8 gray_raw[GRAY_CH_NUM] = {0};

static const gpio_pin_enum gray_pin[GRAY_CH_NUM] = {
    GRAY_PIN_D1, GRAY_PIN_D2, GRAY_PIN_D3, GRAY_PIN_D4,
    GRAY_PIN_D5, GRAY_PIN_D6, GRAY_PIN_D7, GRAY_PIN_D8
};

static const int16 gray_weight[GRAY_CH_NUM] = {
    GRAY_W1, GRAY_W2, GRAY_W3, GRAY_W4,
    GRAY_W5, GRAY_W6, GRAY_W7, GRAY_W8
};

volatile int16 gray_offset = 0;
volatile int16 last_gray_offset = 0;
volatile uint16 lost_flag = 0;

uint8 lost_cnt = 0;
uint8 lost_thresh = 3;

int16 motor_speed_set = MOTOR_SPEED;
int16 line_track_kp = LINE_TRACK_KP;
int16 line_track_ki = LINE_TRACK_KI;
int16 line_track_kd = LINE_TRACK_KD;
int16 line_turn_limit = LINE_TURN_LIMIT;
int16 line_lost_turn = LINE_LOST_TURN;
int16 line_track_kp_turn = LINE_TRACK_KP_TURN;
int16 line_track_ki_turn = LINE_TRACK_KI_TURN;
int16 line_track_kd_turn = LINE_TRACK_KD_TURN;
int16 line_turn_err_th = LINE_TURN_ERR_TH;
int16 line_dead_zone = LINE_DEAD_ZONE;
int16 turn_rate_limit = TURN_RATE_LIMIT;
uint8 tune_item = 0;

int16 d1, d2, d3, d4, d5, d6, d7, d8;
float turn_angle[TRACK_NUM] = {-50.0f, 50.5f};

/* ---- MODE:4 steering PID params ---- */
int16 mode4_line_kp      = MODE4_TRACK_KP;
int16 mode4_line_ki      = MODE4_TRACK_KI;
int16 mode4_line_kd      = MODE4_TRACK_KD;
int16 mode4_speed_set    = MODE4_SPEED_DEFAULT;
uint16 mode4_stop_time_sec = MODE4_TIME_SEC;
uint16 mode4_ramp_up_ms  = MODE4_RAMP_UP_MS;
uint16 mode4_ramp_down_ms = MODE4_RAMP_DOWN_MS;
uint8  mode4_tune_item   = 0;

void data_init(void)
{
    uint8 i;

    for(i = 0; i < GRAY_CH_NUM; i++)
    {
        gpio_init(gray_pin[i], GPI, GPIO_HIGH, GPI_PULL_UP);
    }
}

void data_switch(void)
{
    d1 = gpio_get_level(GRAY_PIN_D1);
    d2 = gpio_get_level(GRAY_PIN_D2);
    d3 = gpio_get_level(GRAY_PIN_D3);
    d4 = gpio_get_level(GRAY_PIN_D4);
    d5 = gpio_get_level(GRAY_PIN_D5);
    d6 = gpio_get_level(GRAY_PIN_D6);
    d7 = gpio_get_level(GRAY_PIN_D7);
    d8 = gpio_get_level(GRAY_PIN_D8);
}

void gray_init(void)
{
    data_init();
}

void gray_read_raw(void)
{
    data_switch();

    gray_raw[0] = (d1 == GRAY_BLACK_LEVEL) ? 1 : 0;
    gray_raw[1] = (d2 == GRAY_BLACK_LEVEL) ? 1 : 0;
    gray_raw[2] = (d3 == GRAY_BLACK_LEVEL) ? 1 : 0;
    gray_raw[3] = (d4 == GRAY_BLACK_LEVEL) ? 1 : 0;
    gray_raw[4] = (d5 == GRAY_BLACK_LEVEL) ? 1 : 0;
    gray_raw[5] = (d6 == GRAY_BLACK_LEVEL) ? 1 : 0;
    gray_raw[6] = (d7 == GRAY_BLACK_LEVEL) ? 1 : 0;
    gray_raw[7] = (d8 == GRAY_BLACK_LEVEL) ? 1 : 0;

}

void gray_calc_line_position(void)
{
    int32 weighted_sum = 0;
    uint8 on_cnt = 0;
    uint8 i;

    for(i = 0; i < GRAY_CH_NUM; i++)
    {
        if(gray_raw[i])
        {
            weighted_sum += gray_weight[i];
            on_cnt++;
        }
    }

    if(on_cnt == 0)
    {
        lost_cnt++;
        if(lost_cnt >= lost_thresh)
        {
            lost_flag = 1;
        }
        return;
    }

    lost_cnt = 0;
    lost_flag = 0;
    last_gray_offset = gray_offset;

    if(on_cnt == GRAY_CH_NUM)
    {
        gray_offset = 0;
        return;
    }

    gray_offset = (int16)(weighted_sum / on_cnt);
}

void line_tune_change(int16 dir)
{
    switch(tune_item)
    {
        case 0:  motor_speed_set    += dir; break;
        case 1:  line_track_kp      += dir; break;
        case 2:  line_track_kd      += dir; break;
        case 3:  line_turn_limit    += dir * 50; break;
        case 4:  line_lost_turn     += dir * 20; break;
        case 5:  line_track_kp_turn += dir; break;
        case 6:  line_track_kd_turn += dir; break;
        case 7:  line_track_ki_turn += dir; break;
        case 8:  line_turn_err_th   += dir; break;
        case 9:  cross_black_thresh += dir; break;
        default: tune_item = 0; break;
    }

    if(motor_speed_set < 0)      motor_speed_set = 0;
    if(motor_speed_set > 30)     motor_speed_set = 30;
    if(line_track_kp < 0)        line_track_kp = 0;
    if(line_track_kp > 80)       line_track_kp = 80;
    if(line_track_ki < 0)        line_track_ki = 0;
    if(line_track_ki > 20)       line_track_ki = 20;
    if(line_track_kd < 0)        line_track_kd = 0;
    if(line_track_kd > 150)      line_track_kd = 150;
    if(line_dead_zone < 0)       line_dead_zone = 0;
    if(line_dead_zone > 70)      line_dead_zone = 70;
    if(turn_rate_limit < 50)     turn_rate_limit = 50;
    if(turn_rate_limit > 1000)   turn_rate_limit = 1000;
    if(line_track_kp_turn < 0)   line_track_kp_turn = 0;
    if(line_track_kp_turn > 150) line_track_kp_turn = 150;
    if(line_track_ki_turn < 0)   line_track_ki_turn = 0;
    if(line_track_ki_turn > 30)  line_track_ki_turn = 30;
    if(line_track_kd_turn < 0)   line_track_kd_turn = 0;
    if(line_track_kd_turn > 80)  line_track_kd_turn = 80;
    if(line_turn_err_th < 1)     line_turn_err_th = 1;
    if(line_turn_err_th > 70)    line_turn_err_th = 70;
    if(line_turn_limit < 100)    line_turn_limit = 100;
    if(line_turn_limit > 5000)   line_turn_limit = 5000;
    if(line_lost_turn < 0)       line_lost_turn = 0;
    if(line_lost_turn > 2000)    line_lost_turn = 2000;
    if(cross_black_thresh < 2)   cross_black_thresh = 2;
    if(cross_black_thresh > 8)   cross_black_thresh = 8;
    if(mode4_stop_time_sec < 3)  mode4_stop_time_sec = 3;
    if(mode4_stop_time_sec > 60) mode4_stop_time_sec = 60;
}

void mode4_tune_change(int16 dir)
{
    switch(mode4_tune_item)
    {
        case 0: mode4_line_kp += dir; break;
        case 1: mode4_line_ki += dir; break;
        case 2: mode4_line_kd += dir; break;
        case 3: mode4_speed_set    += dir; break;
        case 4: mode4_ramp_up_ms   += (uint16)(dir * 100); break;
        case 5: mode4_ramp_down_ms += (uint16)(dir * 100); break;
        case 6: mode4_stop_time_sec += dir; break;
        default: mode4_tune_item = 0; break;
    }

    if(mode4_line_kp < 0)   mode4_line_kp = 0;
    if(mode4_line_kp > 80)  mode4_line_kp = 80;
    if(mode4_line_ki < 0)   mode4_line_ki = 0;
    if(mode4_line_ki > 20)  mode4_line_ki = 20;
    if(mode4_line_kd < 0)   mode4_line_kd = 0;
    if(mode4_line_kd > 150) mode4_line_kd = 150;
    if(mode4_ramp_up_ms < 200)   mode4_ramp_up_ms = 200;
    if(mode4_ramp_up_ms > 5000)  mode4_ramp_up_ms = 5000;
    if(mode4_ramp_down_ms < 200) mode4_ramp_down_ms = 200;
    if(mode4_ramp_down_ms > 5000) mode4_ramp_down_ms = 5000;
    if(mode4_speed_set < 3)   mode4_speed_set = 3;
    if(mode4_speed_set > 25)  mode4_speed_set = 25;
    if(mode4_stop_time_sec < 3)  mode4_stop_time_sec = 3;
    if(mode4_stop_time_sec > 60) mode4_stop_time_sec = 60;
}
