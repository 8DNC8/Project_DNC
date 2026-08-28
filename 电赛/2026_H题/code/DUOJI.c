#include "DUOJI.h"
#include "control.h"
#include "encoder.h"
#include "uart_k230.h"
#include <math.h>

int16 servo_p = SERVO_P_DEFAULT;
int16 servo_d = SERVO_D_DEFAULT;
int16 servo_i = SERVO_I_DEFAULT;
int16 servo_set_x = BALL_SET_X_DEFAULT;
int16 servo_set_y = BALL_SET_Y_DEFAULT;
uint8 servo_tune_item = 0;
int16 servo_manual_duty = 0;
uint8 q4_tune_item = 0;
uint8 h3_tune_item = 0;
int16 h3_duty_offset[3] = {
    H3_DUTY1_DEFAULT,
    H3_DUTY2_DEFAULT,
    H3_DUTY3_DEFAULT
};
uint16 h3_time_ms[3] = {
    H3_TIME1_MS_DEFAULT,
    H3_TIME2_MS_DEFAULT,
    H3_TIME3_MS_DEFAULT
};
int16 h3_follow_target = H3_FOLLOW_TARGET_DEFAULT;

uint8 ball_balance_enable = 0;
int16 ball_target_d10 = TRACK_BALL_TARGET;

static float chassis_speed_mps = 0.0f;
static float chassis_accel_mps2 = 0.0f;
static float chassis_speed_last_mps = 0.0f;
static float ball_pos_last_mm = 0.0f;
static float ball_vel_mmps = 0.0f;
static float ball_err_sum = 0.0f;
static uint8 ball_first_frame = 1;
static float servo_angle_deg = SERVO_ANGLE_CENTER;

static uint8 h3_running = 0;
static uint8 h3_phase = 0;
static uint16 h3_tick_cnt = 0;

static float clampf(float value, float min_value, float max_value)
{
    if(value < min_value) return min_value;
    if(value > max_value) return max_value;
    return value;
}

static uint32 servo_duty_from_angle(float angle_deg)
{
    float duty;

    angle_deg = clampf(angle_deg, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX);
    duty = (float)SERVO_ANGLE_0
         + (float)(SERVO_ANGLE_180 - SERVO_ANGLE_0) * angle_deg / 180.0f;

    if(duty < (float)SERVO_DUTY_MIN) duty = (float)SERVO_DUTY_MIN;
    if(duty > (float)SERVO_DUTY_MAX) duty = (float)SERVO_DUTY_MAX;

    return (uint32)duty;
}

static void servo_write_angle(float angle_deg)
{
    servo_angle_deg = clampf(angle_deg, SERVO_ANGLE_MIN, SERVO_ANGLE_MAX);
    pwm_set_duty(SERVO_PWM_PIN, servo_duty_from_angle(servo_angle_deg));
}

static void servo_write_duty_offset(int16 duty_offset)
{
    int32 duty = SERVO_MID_DUTY + duty_offset;

    if(duty < SERVO_DUTY_MIN) duty = SERVO_DUTY_MIN;
    if(duty > SERVO_DUTY_MAX) duty = SERVO_DUTY_MAX;

    pwm_set_duty(SERVO_PWM_PIN, (uint32)duty);
}

static void ball_balance_reset_estimator(void)
{
    ball_first_frame = 1;
    chassis_speed_mps = 0.0f;
    chassis_accel_mps2 = 0.0f;
    chassis_speed_last_mps = 0.0f;
    ball_vel_mmps = 0.0f;
    ball_err_sum = 0.0f;
}

static uint16 h3_get_tick_max(uint8 phase)
{
    uint16 tick_max;

    if(phase >= 3) return 1;

    tick_max = h3_time_ms[phase] / 20;
    if(tick_max == 0) tick_max = 1;

    return tick_max;
}

static float encoder_calc_accel(void)
{
    float speed_mps;
    float raw_accel;
    float pulse_avg;

    pulse_avg = (float)(encoder[0] + encoder[1]) * 0.5f;
    speed_mps = pulse_avg * PULSE_TO_CM / 100.0f / BALL_CTRL_DT;
    raw_accel = (speed_mps - chassis_speed_last_mps) / BALL_CTRL_DT;
    chassis_speed_last_mps = speed_mps;

    chassis_speed_mps += SPEED_LPF_ALPHA * (speed_mps - chassis_speed_mps);
    chassis_accel_mps2 += ACCEL_LPF_ALPHA * (raw_accel - chassis_accel_mps2);

    return chassis_accel_mps2;
}

void duoji_init(void)
{
    pwm_init(SERVO_PWM_PIN, SERVO_FREQ, SERVO_MID_DUTY);
    servo_write_angle(SERVO_ANGLE_CENTER);
}

void h3_start(void)
{
    h3_running = 1;
    h3_phase = 0;
    h3_tick_cnt = 0;
    ball_balance_enable = 0;
    ball_target_d10 = TRACK_BALL_TARGET;
    ball_balance_reset_estimator();
}

void h3_stop(void)
{
    h3_running = 0;
    h3_phase = 0;
    h3_tick_cnt = 0;
}

uint8 h3_is_busy(void)
{
    return h3_running;
}

uint8 h3_get_phase(void)
{
    return h3_phase;
}

static void h3_exec(void)
{
    if(!h3_running) return;

    if(h3_phase >= 3)
    {
        h3_running = 0;
        h3_tick_cnt = 0;
        ball_target_d10 = h3_follow_target;
        ball_balance_reset_estimator();
        ball_balance_enable = 1;
        return;
    }

    servo_write_duty_offset(h3_duty_offset[h3_phase]);

    h3_tick_cnt++;
    if(h3_tick_cnt >= h3_get_tick_max(h3_phase))
    {
        h3_tick_cnt = 0;
        h3_phase++;
    }
}

void duoji_ball_control(void)
{
    float accel_mps2;
    float theta_ff_deg;
    float theta_fb_deg = 0.0f;
    float theta_cmd_deg;
    float ball_pos_mm;
    float pos_err_mm;

    if(servo_tune_item == 4)
    {
        int32 duty = SERVO_MID_DUTY + servo_manual_duty;
        if(duty < SERVO_DUTY_MIN) duty = SERVO_DUTY_MIN;
        if(duty > SERVO_DUTY_MAX) duty = SERVO_DUTY_MAX;
        pwm_set_duty(SERVO_PWM_PIN, (uint32)duty);
        return;
    }

    if(h3_running)
    {
        h3_exec();
        return;
    }

    if(!ball_balance_enable)
    {
        ball_balance_reset_estimator();
        servo_write_angle(SERVO_ANGLE_CENTER);
        return;
    }

    accel_mps2 = encoder_calc_accel();
    theta_ff_deg = -ENC_ACCEL_FF_GAIN
                 * asinf(clampf(accel_mps2 / GRAVITY_MS2, -0.99f, 0.99f))
                 * RAD_TO_DEG;

    if(ball_online)
    {
        ball_pos_mm = (float)ball_d10 * K230_BALL_SIGN;
        pos_err_mm = (float)ball_target_d10 - ball_pos_mm;

        if(ball_first_frame)
        {
            ball_pos_last_mm = ball_pos_mm;
            ball_vel_mmps = 0.0f;
            ball_first_frame = 0;
        }
        else
        {
            float raw_ball_vel = (ball_pos_mm - ball_pos_last_mm) / BALL_CTRL_DT;
            ball_pos_last_mm = ball_pos_mm;
            ball_vel_mmps += BALL_VEL_LPF_ALPHA * (raw_ball_vel - ball_vel_mmps);
        }

        ball_err_sum += pos_err_mm;
        ball_err_sum = clampf(ball_err_sum, -BALL_ERR_SUM_LIMIT, BALL_ERR_SUM_LIMIT);

        theta_fb_deg = (float)servo_p * 0.01f * pos_err_mm
                     + (float)servo_d * 0.01f * (-ball_vel_mmps)
                     + (float)servo_i * 0.001f * ball_err_sum;
    }
    else
    {
        ball_first_frame = 1;
        ball_err_sum = 0.0f;
    }

    theta_cmd_deg = theta_ff_deg + theta_fb_deg;
    if(theta_cmd_deg < 0.0f)
    {
        theta_cmd_deg *= PIPE_NEG_GAIN;
    }
    theta_cmd_deg = clampf(theta_cmd_deg, -PIPE_ANGLE_MAX_DEG, PIPE_ANGLE_MAX_DEG);

    servo_write_angle(SERVO_ANGLE_CENTER + SERVO_DIR * theta_cmd_deg);
}

void servo_tune_change(uint8 item, int8 dir)
{
    switch(item)
    {
        case 0: servo_p += dir; break;
        case 1: servo_d += dir; break;
        case 2: servo_i += dir; break;
        case 3: servo_set_x += dir; break;
        case 4: servo_set_y += dir; break;
        case 5: servo_manual_duty += dir * 5; break;
        default: servo_tune_item = 0; break;
    }

    if(servo_p < -100) servo_p = -100;
    if(servo_p >  100) servo_p =  100;
    if(servo_d < -100) servo_d = -100;
    if(servo_d >  100) servo_d =  100;
    if(servo_i < -100) servo_i = -100;
    if(servo_i >  100) servo_i =  100;
    if(servo_manual_duty < -200) servo_manual_duty = -200;
    if(servo_manual_duty >  200) servo_manual_duty =  200;
}

void q4_tune_change(uint8 item, int8 dir)
{
    switch(item)
    {
        case 0: servo_p += dir; break;
        case 1: servo_d += dir; break;
        case 2: servo_i += dir; break;
        default: q4_tune_item = 0; break;
    }

    if(servo_p < -100) servo_p = -100;
    if(servo_p >  100) servo_p =  100;
    if(servo_d < -100) servo_d = -100;
    if(servo_d >  100) servo_d =  100;
    if(servo_i < -100) servo_i = -100;
    if(servo_i >  100) servo_i =  100;
}

void h3_tune_change(uint8 item, int8 dir)
{
    switch(item)
    {
        case 0: h3_duty_offset[0] += dir * 5; break;
        case 1: h3_duty_offset[1] += dir * 5; break;
        case 2: h3_duty_offset[2] += dir * 5; break;
        case 3: h3_time_ms[0] = (uint16)((int32)h3_time_ms[0] + dir * 20); break;
        case 4: h3_time_ms[1] = (uint16)((int32)h3_time_ms[1] + dir * 20); break;
        case 5: h3_time_ms[2] = (uint16)((int32)h3_time_ms[2] + dir * 20); break;
        case 6: h3_follow_target += dir * 5; break;
        default: h3_tune_item = 0; break;
    }

    if(h3_duty_offset[0] < -200) h3_duty_offset[0] = -200;
    if(h3_duty_offset[0] >  200) h3_duty_offset[0] =  200;
    if(h3_duty_offset[1] < -200) h3_duty_offset[1] = -200;
    if(h3_duty_offset[1] >  200) h3_duty_offset[1] =  200;
    if(h3_duty_offset[2] < -200) h3_duty_offset[2] = -200;
    if(h3_duty_offset[2] >  200) h3_duty_offset[2] =  200;

    if(h3_time_ms[0] < 20) h3_time_ms[0] = 20;
    if(h3_time_ms[0] > 2000) h3_time_ms[0] = 2000;
    if(h3_time_ms[1] < 20) h3_time_ms[1] = 20;
    if(h3_time_ms[1] > 2000) h3_time_ms[1] = 2000;
    if(h3_time_ms[2] < 20) h3_time_ms[2] = 20;
    if(h3_time_ms[2] > 2000) h3_time_ms[2] = 2000;

    if(h3_follow_target < -100) h3_follow_target = -100;
    if(h3_follow_target >  100) h3_follow_target =  100;
}
