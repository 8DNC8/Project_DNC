#ifndef GRAY_H
#define GRAY_H

#include "zf_common_headfile.h"

#define GRAY_PIN_D1        (B23)
#define GRAY_PIN_D2        (B22)
#define GRAY_PIN_D3        (B21)
#define GRAY_PIN_D4        (B19)
#define GRAY_PIN_D5        (B24)
#define GRAY_PIN_D6        (B8)
#define GRAY_PIN_D7        (B26)
#define GRAY_PIN_D8        (B14)
#define GRAY_CH_NUM        (8)

#define GRAY_BLACK_LEVEL   (0)

#define GRAY_W1            (-70)
#define GRAY_W2            (-50)
#define GRAY_W3            (-30)
#define GRAY_W4            (-10)
#define GRAY_W5            (10)
#define GRAY_W6            (30)
#define GRAY_W7            (50)
#define GRAY_W8            (70)

#define LINE_TRACK_KP      (8)
#define LINE_TRACK_KI      (1)
#define LINE_TRACK_KD      (12)
#define LINE_TURN_LIMIT    (2500)
#define LINE_LOST_TURN     (500)

#define LINE_DEAD_ZONE     (15)
#define TURN_RATE_LIMIT    (120)

#define LINE_TRACK_KP_TURN (4)
#define LINE_TRACK_KI_TURN (2)
#define LINE_TRACK_KD_TURN (8)
#define LINE_TURN_ERR_TH   (45)
#define LINE_TURN_DELTA_TH (20)

#define SPEED_LOW_THRESH   (8)
#define SPEED_HIGH_THRESH  (18)
#define TURN_PREDICT_THRESH (15)
#define TURN_SPEED_REDUCE  (0.85f)

#define TRACK_NUM          (2)

/* ---- MODE:4 default PID & timing ---- */
#define MODE4_TRACK_KP      (6)
#define MODE4_TRACK_KI      (1)
#define MODE4_TRACK_KD      (8)
#define MODE4_SPEED_DEFAULT (15)
#define MODE4_TIME_SEC      (9)
#define MODE4_RAMP_UP_MS    (2000)   /* 2 s ramp-up */
#define MODE4_RAMP_DOWN_MS  (1500)   /* 1.5 s ramp-down */

extern uint8 gray_raw[GRAY_CH_NUM];

extern volatile int16 gray_offset;
extern volatile int16 last_gray_offset;
extern volatile uint16 lost_flag;
extern int16 d1, d2, d3, d4, d5, d6, d7, d8;

extern int16 motor_speed_set;
extern int16 line_track_kp;
extern int16 line_track_ki;
extern int16 line_track_kd;
extern int16 line_turn_limit;
extern int16 line_lost_turn;
extern int16 line_track_kp_turn;
extern int16 line_track_ki_turn;
extern int16 line_track_kd_turn;
extern int16 line_turn_err_th;
extern int16 line_dead_zone;
extern int16 turn_rate_limit;
extern uint8 tune_item;

extern uint8 lost_cnt;
extern uint8 lost_thresh;
extern float turn_angle[TRACK_NUM];

/* MODE:4 steering PID params */
extern int16 mode4_line_kp;
extern int16 mode4_line_ki;
extern int16 mode4_line_kd;
extern int16 mode4_speed_set;
extern uint16 mode4_stop_time_sec;
extern uint16 mode4_ramp_up_ms;
extern uint16 mode4_ramp_down_ms;

/* MODE:4 PID tune cursor */
extern uint8 mode4_tune_item;

void gray_init(void);
void gray_read_raw(void);
void gray_calc_line_position(void);
void line_tune_change(int16 dir);
void mode4_tune_change(int16 dir);
void data_init(void);
void data_switch(void);

#endif
