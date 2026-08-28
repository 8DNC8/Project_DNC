#include "control.h"
#include "DUOJI.h"

float head_angle = 0.0f;
float head_offset = 0.0f;
float head_offset_sum = 0.0f;
int32 turn_duty = 0;
uint8 turn_mode_sharp = 0;
uint8 turn_predict_flag = 0;
uint8 turn_entry_cnt = 0;
int32 gray_offset_sum = 0;

volatile int16 encoder[MOTOR_NUM] = {0};
volatile int32 motor_duty[MOTOR_NUM] = {0};

uint8 model = 0;
uint8 step = 0;
uint8 loss_cnt = 0;
uint8 find_cnt = 0;
uint8 car_move = 0;
uint8 lap_count = 0;
uint8 q_mode = 1;

uint16 stop_time_sec = 18;
uint8 full_black_cnt = 0;
uint8 last_full_black = 0;
uint32 start_systime = 0;
uint32 lap_time_20ms = 0;
uint8 lap_done = 0;
uint8 cross_black_thresh = 3;
uint32 systime_20ms = 0;
uint16 startup_timer = 0;
uint8 state_lock_flag = 0;
uint32 state_lock_cnt = 0;
uint8 q4_start_pending = 0;
uint16 q4_balance_wait_cnt = 0;

/* MODE selection */
uint8 menu_cursor = 0;   /* 0..4 => MODE:1..MODE:5 */
uint8 task_mode  = TASK_NONE;

/* cross-black stop + reverse state */
uint8  cross_stop_triggered = 0;
uint16 cross_debounce_cnt   = 0;
uint16 reverse_tick         = 0;

#define Q4_BALANCE_WAIT_TICKS      (10)
#define CROSS_DEBOUNCE_TICKS       (3)    /* 3 * 20ms = 60ms debounce */
#define MODE2_STOP_ENABLE_TICKS    (750)  /* 15 s after start */
#define REVERSE_TICKS              (10)   /* 200ms reverse */
#define REVERSE_DUTY               (300)  /* reverse motor duty */

/* ---- internal helpers ---- */

static void stop_all(void)
{
    h3_stop();
    q4_start_pending = 0;
    q4_balance_wait_cnt = 0;
    cross_stop_triggered = 0;
    cross_debounce_cnt = 0;
    reverse_tick = 0;
    car_move = 0;
    pid_increment.out = 0;
    pid_mode4.out = 0;
    turn_duty = 0;
    ball_balance_enable = 0;
    ball_target_d10 = TRACK_BALL_TARGET;
    motor_control(0, 0);
    task_mode = TASK_NONE;
}

void task_stop(void)
{
    stop_all();
}

/* ---- MODE start functions ---- */

void mode2_start(void)
{
    stop_all();
    task_mode = TASK_MODE2;
    model = 1;
    gray_offset_sum = 0;
    full_black_cnt = 0;
    last_full_black = 0;
    lap_done = 0;
    start_systime = systime_20ms;
    startup_timer = 0;
    cross_stop_triggered = 0;
    cross_debounce_cnt = 0;
    reverse_tick = 0;

    pid_reset(&pid_increment);
    pid_increment.set_speed = motor_speed_set;

    car_move = 1;
}

void task3_start(void)
{
    stop_all();
    task_mode = TASK_MODE3;
    q_mode = 0;
    model = 0;
    gray_offset_sum = 0;
    lap_done = 0;
    start_systime = systime_20ms;
    startup_timer = 0;
    h3_start();
}

void task4_start(void)
{
    stop_all();
    task_mode = TASK_MODE4;
    q_mode = 1;
    model = 1;
    gray_offset_sum = 0;
    ball_balance_enable = 1;
    ball_target_d10 = TRACK_BALL_TARGET;

    full_black_cnt = 0;
    last_full_black = 0;
    lap_done = 0;
    start_systime = systime_20ms;
    startup_timer = 0;

    pid_reset(&pid_mode4);
    pid_mode4.set_speed = 0;

    q4_start_pending = 1;
    q4_balance_wait_cnt = 0;

    stop_time_sec = mode4_stop_time_sec;
}

/* MODE:5 is same as MODE:4 but 30 s runtime */
void mode5_start(void)
{
    stop_all();
    task_mode = TASK_MODE5;
    q_mode = 1;
    model = 1;
    gray_offset_sum = 0;
    ball_balance_enable = 1;
    ball_target_d10 = TRACK_BALL_TARGET;

    full_black_cnt = 0;
    last_full_black = 0;
    lap_done = 0;
    start_systime = systime_20ms;
    startup_timer = 0;

    pid_reset(&pid_mode4);
    pid_mode4.set_speed = 0;

    q4_start_pending = 1;
    q4_balance_wait_cnt = 0;

    stop_time_sec = 30;
}

/* PG:0 KEY4 confirm action */
void menu_confirm(void)
{
    switch(menu_cursor)
    {
        case 0: /* MODE:1 - do nothing */
            stop_all();
            task_mode = TASK_MODE1;
            break;
        case 1: /* MODE:2 */
            mode2_start();
            break;
        case 2: /* MODE:3 */
            task3_start();
            break;
        case 3: /* MODE:4 */
            task4_start();
            break;
        case 4: /* MODE:5 */
            mode5_start();
            break;
        default:
            break;
    }
}

/* ---- state lock ---- */

void state_lock(void)
{
    state_lock_flag = 1;
    state_lock_cnt = systime_20ms;
}

uint8 state_lock_check(void)
{
    if(state_lock_flag)
    {
        if(systime_20ms - state_lock_cnt < 50)
        {
            return 1;
        }

        state_lock_flag = 0;
    }

    return 0;
}

/* ---- cross-black detection ---- */
/* returns 1 if >= cross_black_thresh sensors see black simultaneously */
static uint8 cross_black_detect(void)
{
    uint8 i;
    uint8 black_cnt = 0;

    for(i = 0; i < GRAY_CH_NUM; i++)
    {
        if(gray_raw[i]) black_cnt++;
    }

    return (black_cnt >= cross_black_thresh) ? 1 : 0;
}

/* ---- line tracking turn control ---- */

static void update_line_turn_control(void)
{
    int16 gray_delta;
    int16 eff_delta;
    int16 effective_offset;
    int16 kp;
    int16 ki;
    int16 kd;
    int32 new_turn_duty;
    float speed_factor = 1.0f;
    int16 current_speed = (int16)((encoder[0] + encoder[1]) / 2);

    if(current_speed < SPEED_LOW_THRESH)
    {
        speed_factor = 1.3f;
    }
    else if(current_speed > SPEED_HIGH_THRESH)
    {
        speed_factor = 0.8f;
    }

    if(lost_flag)
    {
        if(last_gray_offset > 0)      turn_duty =  line_lost_turn;
        else if(last_gray_offset < 0) turn_duty = -line_lost_turn;
        else                          turn_duty = 0;

        gray_offset_sum = 0;
        turn_predict_flag = 0;
        return;
    }

    gray_delta = (int16)(gray_offset - last_gray_offset);
    effective_offset = gray_offset;

    if(func_abs(effective_offset) < line_dead_zone)
    {
        effective_offset = 0;
        if(gray_offset_sum > 0)      gray_offset_sum -= 5;
        else if(gray_offset_sum < 0) gray_offset_sum += 5;
        if(func_abs(gray_offset_sum) < 5) gray_offset_sum = 0;
    }

    {
        static int16 last_eff_offset = 0;
        eff_delta = (int16)(effective_offset - last_eff_offset);
        last_eff_offset = effective_offset;
    }

    if(turn_mode_sharp && func_abs(gray_delta) >= TURN_PREDICT_THRESH)
    {
        turn_predict_flag = 1;
    }
    else if(!turn_mode_sharp)
    {
        turn_predict_flag = 0;
    }

    if(!turn_mode_sharp)
    {
        if(func_abs(gray_offset) >= line_turn_err_th)
        {
            turn_entry_cnt++;
            if(turn_entry_cnt >= 2) turn_mode_sharp = 1;
        }
        else
        {
            turn_entry_cnt = 0;
        }
    }
    else
    {
        turn_entry_cnt = 0;
        if(func_abs(gray_offset) < line_turn_err_th - 10)
        {
            turn_mode_sharp = 0;
            gray_offset_sum /= 2;
        }
    }

    if(turn_mode_sharp)
    {
        gray_offset_sum += effective_offset;
        gray_offset_sum = func_limit(gray_offset_sum, 300);
    }
    else
    {
        gray_offset_sum += effective_offset;
        gray_offset_sum = func_limit(gray_offset_sum, 200);
        if(((effective_offset > 0) && (gray_offset_sum < 0)) ||
           ((effective_offset < 0) && (gray_offset_sum > 0)))
        {
            gray_offset_sum = 0;
        }
    }

    /* use MODE:4/5 steering PID when those tasks are active */
    if(task_mode == TASK_MODE4 || task_mode == TASK_MODE5)
    {
        kp = mode4_line_kp;
        ki = mode4_line_ki;
        kd = mode4_line_kd;
    }
    else
    {
        kp = turn_mode_sharp ? line_track_kp_turn : line_track_kp;
        ki = turn_mode_sharp ? line_track_ki_turn : line_track_ki;
        kd = turn_mode_sharp ? line_track_kd_turn : line_track_kd;
    }

    if(!turn_mode_sharp && (task_mode != TASK_MODE4 && task_mode != TASK_MODE5))
    {
        kp = (int16)(kp * speed_factor);
        ki = (int16)(ki * speed_factor);
        kd = (int16)(kd * speed_factor);
    }

    new_turn_duty = (int32)effective_offset * kp
                  + (int32)eff_delta * kd
                  + (int32)gray_offset_sum * ki / 10;

    if(turn_mode_sharp)
    {
        new_turn_duty += (int32)effective_offset * func_abs(effective_offset) / 35;
    }

    {
        int32 turn_diff = new_turn_duty - turn_duty;
        if(turn_diff > turn_rate_limit)       turn_duty += turn_rate_limit;
        else if(turn_diff < -turn_rate_limit) turn_duty -= turn_rate_limit;
        else                                  turn_duty = new_turn_duty;
    }

    turn_duty = func_limit(turn_duty, line_turn_limit);
}

/* ---- 20 ms control tick ---- */

void pit_callback(uint32 event, void *ptr)
{
    int32 ramp_v;
    uint32 remaining_ticks;
    uint32 ramp_up_ticks;
    uint32 ramp_down_ticks;
    uint16 mode4_effective_stop_sec;
    pid_increment_struct *active_pid;
    int16 active_speed;

    (void)event;
    (void)ptr;

    systime_20ms++;

    gray_read_raw();
    gray_calc_line_position();
    encoder_read(&encoder[0], &encoder[1]);

    head_offset = 0.0f;
    head_offset_sum = 0.0f;

    beep_tick();
    k230_tick_20ms();
    duoji_ball_control();

    /* Q4 balance wait for MODE:4/5 */
    if(q4_start_pending)
    {
        q4_balance_wait_cnt++;
        if(q4_balance_wait_cnt >= Q4_BALANCE_WAIT_TICKS)
        {
            q4_start_pending = 0;
            q4_balance_wait_cnt = 0;
            car_move = 1;
            startup_timer = 0;
        }
    }

    /* ---- MODE:2 cross-black detection + reverse stop ---- */
    if(task_mode == TASK_MODE2 && car_move && !cross_stop_triggered && startup_timer >= MODE2_STOP_ENABLE_TICKS)
    {
        if(cross_black_detect())
        {
            cross_debounce_cnt++;
            if(cross_debounce_cnt >= CROSS_DEBOUNCE_TICKS)
            {
                /* trigger stop: brief reverse, then stop */
                cross_stop_triggered = 1;
                reverse_tick = REVERSE_TICKS;
                lap_time_20ms = systime_20ms;
                lap_done = 1;
            }
        }
        else
        {
            cross_debounce_cnt = 0;
        }
    }

    /* reverse phase after cross-black trigger */
    if(cross_stop_triggered && reverse_tick > 0)
    {
        reverse_tick--;
        motor_control(-REVERSE_DUTY, -REVERSE_DUTY);
        return;
    }

    if(cross_stop_triggered && reverse_tick == 0)
    {
        /* reverse finished, full stop */
        stop_all();
        return;
    }

    if(!car_move)
    {
        pid_increment.out = 0;
        pid_mode4.out = 0;
        turn_duty = 0;
        motor_control(0, 0);
        return;
    }

    /* Determine effective params based on task mode */
    if(task_mode == TASK_MODE4 || task_mode == TASK_MODE5)
    {
        active_pid   = &pid_mode4;
        active_speed = mode4_speed_set;
        mode4_effective_stop_sec = stop_time_sec;
        ramp_up_ticks   = mode4_ramp_up_ms / 20;
        ramp_down_ticks = mode4_ramp_down_ms / 20;
    }
    else
    {
        active_pid   = &pid_increment;
        active_speed = motor_speed_set;
        ramp_up_ticks   = 80;
        ramp_down_ticks = 0;
        mode4_effective_stop_sec = stop_time_sec;
    }

    if(model == 1)
    {
        startup_timer++;

        if(task_mode == TASK_MODE2)
        {
            /* MODE:2: speed = motor_speed_set (PG:2 SPD), drop to 15 after 13 s */
            /* ramp-up: 0 -> active_speed over 1.6 s (80 ticks) */
            if(startup_timer < 80)
            {
                ramp_v = (int32)active_speed * startup_timer / 80;
                if(ramp_v < 1) ramp_v = 1;
            }
            /* smooth drop: active_speed -> 15 between 13 s and 14 s */
            else if(startup_timer >= 650 && startup_timer < 700)
            {
                int32 frac = (int32)(startup_timer - 650);
                ramp_v = active_speed + (15 - active_speed) * frac / 50;
            }
            /* steady 15 after 14 s */
            else if(startup_timer >= 700)
            {
                ramp_v = 15;
            }
            /* full speed cruising: active_speed */
            else
            {
                ramp_v = active_speed;
            }
        }
        else if(task_mode == TASK_MODE4 || task_mode == TASK_MODE5)
        {
            /* Slow ramp-up */
            if(startup_timer < ramp_up_ticks)
            {
                ramp_v = (int32)active_speed * startup_timer / (int32)ramp_up_ticks;
                if(ramp_v < 1) ramp_v = 1;
            }
            else
            {
                /* Slow ramp-down before stop */
                remaining_ticks = (uint32)mode4_effective_stop_sec * 50;
                if(startup_timer < remaining_ticks)
                {
                    remaining_ticks -= startup_timer;
                }
                else
                {
                    remaining_ticks = 0;
                }

                if(remaining_ticks < ramp_down_ticks && remaining_ticks > 0)
                {
                    ramp_v = (int32)active_speed * (int32)remaining_ticks / (int32)ramp_down_ticks;
                    if(ramp_v < 1) ramp_v = 1;
                }
                else
                {
                    ramp_v = active_speed;
                }
            }
        }
        else
        {
            ramp_v = (int32)active_speed * startup_timer / (int32)ramp_up_ticks;
            if(ramp_v > active_speed) ramp_v = active_speed;
        }

        active_pid->set_speed = (int16)ramp_v;

        pid_increment_calc(active_pid, (int16)((encoder[0] + encoder[1]) / 2));
        update_line_turn_control();
    }
    else
    {
        active_pid->out = 0;
        pid_increment.out = 0;
        pid_mode4.out = 0;
        turn_duty = 0;
    }

    /* Check stop condition — only for MODE:4/5 (time-based); MODE:2 uses cross-black */
    if(task_mode == TASK_MODE4 || task_mode == TASK_MODE5)
    {
        if(startup_timer >= (uint32)mode4_effective_stop_sec * 50)
        {
            car_move = 0;
            active_pid->out = 0;
            turn_duty = 0;
            lap_time_20ms = systime_20ms;
            lap_done = 1;
            task_mode = TASK_NONE;
        }
    }

    motor_duty[0] = (int32)(active_pid->out + turn_duty);
    motor_duty[1] = (int32)(active_pid->out - turn_duty);

    motor_control(motor_duty[0], motor_duty[1]);
}
