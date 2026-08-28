#ifndef DUOJI_H
#define DUOJI_H

#include "zf_common_headfile.h"

#define SERVO_PWM_PIN          PWM_TIM_A1_CH1_B3
#define SERVO_FREQ             (50)

#define SERVO_ANGLE_0          (250)
#define SERVO_ANGLE_180        (1250)
#define SERVO_MID_DUTY         (750)
#define SERVO_DUTY_MIN         (550)
#define SERVO_DUTY_MAX         (950)

#define SERVO_ANGLE_MIN        (0.0f)
#define SERVO_ANGLE_MAX        (180.0f)
#define SERVO_ANGLE_CENTER     (90.0f)
#define SERVO_DIR              (-1.0f)

#define BALL_CTRL_DT           (0.02f)
#define GRAVITY_MS2            (9.8f)
#define RAD_TO_DEG             (57.2957795f)
#define PIPE_ANGLE_MAX_DEG     (25.0f)
#define PIPE_NEG_GAIN          (1.80f)

#define ENC_ACCEL_FF_GAIN      (1.0f)
#define SPEED_LPF_ALPHA        (0.35f)
#define ACCEL_LPF_ALPHA        (0.35f)

#define FB_ENABLE              (1)
#define K230_BALL_SIGN         (1.0f)
#define BALL_VEL_LPF_ALPHA     (0.20f)
#define BALL_ERR_SUM_LIMIT     (1000.0f)

#define BALL_SET_X_DEFAULT     (320)
#define BALL_SET_Y_DEFAULT     (240)

/* Integer tuning scale:
 * servo_p: 20 means 0.20 deg/mm
 * servo_d: 8  means 0.08 deg/(mm/s)
 * servo_i: kept small, normally 0
 */
#define SERVO_P_DEFAULT        (15)
#define SERVO_D_DEFAULT        (22)
#define SERVO_I_DEFAULT        (1)

#define TRACK_BALL_TARGET      (0)

#define H3_DUTY1_DEFAULT       (-65)
#define H3_DUTY2_DEFAULT       (120)
#define H3_DUTY3_DEFAULT       (-75)
#define H3_TIME1_MS_DEFAULT    (680)
#define H3_TIME2_MS_DEFAULT    (950)
#define H3_TIME3_MS_DEFAULT    (351)
#define H3_FOLLOW_TARGET_DEFAULT (-70)

extern int16 servo_p;
extern int16 servo_d;
extern int16 servo_i;
extern int16 servo_set_x;
extern int16 servo_set_y;
extern uint8 servo_tune_item;
extern int16 servo_manual_duty;
extern uint8 q4_tune_item;
extern uint8 h3_tune_item;
extern int16 h3_duty_offset[3];
extern uint16 h3_time_ms[3];
extern int16 h3_follow_target;

extern uint8 ball_balance_enable;
extern int16 ball_target_d10;

void duoji_init(void);
void duoji_ball_control(void);
void h3_start(void);
void h3_stop(void);
uint8 h3_is_busy(void);
uint8 h3_get_phase(void);
void h3_tune_change(uint8 item, int8 dir);
void servo_tune_change(uint8 item, int8 dir);
void q4_tune_change(uint8 item, int8 dir);

#endif
