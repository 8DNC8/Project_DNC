#ifndef CONTROL_H
#define CONTROL_H

#include "zf_common_headfile.h"
#include "motor.h"       // MOTOR_NUM, motor_control
#include "pid.h"         // pid_increment_struct, pid_increment, pid_mode4
#include "gray.h"        // motor_speed_set, line_*, lost_flag, gray_offset, mode4_*
#include "beep.h"        // beep_tick
#include "encoder.h"     // encoder_read
#include "uart_k230.h"   // K230 vision (display coords)

/* task mode identifiers */
#define TASK_NONE    0
#define TASK_MODE1   1   /* MODE:1 - do nothing */
#define TASK_MODE2   2   /* MODE:2 - track spd 22, drop to 15 after 13 s */
#define TASK_MODE3   3   /* MODE:3 - H3 servo three-stage */
#define TASK_MODE4   4   /* MODE:4 - slow track spd 13, slow ramp, separate PID */
#define TASK_MODE5   5   /* MODE:5 - same as MODE:4 but 30 s runtime */

/* car posture and control state */
extern float head_angle;        // car head angle
extern float head_offset;       // expected vs actual head angle deviation
extern float head_offset_sum;   // deviation integral
extern int32 turn_duty;         // steering duty
extern uint8 turn_mode_sharp;   // 1 = currently in sharp-turn gain mode (for status display)
extern uint8 turn_predict_flag; // 1 = predicted imminent turn (pre-slow)

// tracking integral term (eliminates static error)
extern int32 gray_offset_sum;   // gray offset integral

extern volatile int16 encoder[MOTOR_NUM];    // actual speed (left/right, volatile: read in ISR)
extern volatile int32 motor_duty[MOTOR_NUM]; // motor duty (volatile: read in ISR)

extern uint8 model;      // 0:straight 1:tracking 2:turn
extern uint8 step;       // current turn index
extern uint8 loss_cnt;   // lost-line count
extern uint8 find_cnt;   // found-line count
extern uint8 car_move;   // car running flag
extern uint8 lap_count;  // completed lap count
extern uint8 q_mode;     // 0=Q3 mode(H3 open-loop positioning) 1=Q4 mode(track+ball balance)
extern uint16 stop_time_sec;     // tracking stop time (seconds), key-adjustable

/* MODE selection cursor on PG:0: 0=MODE:1 .. 4=MODE:5 */
extern uint8 menu_cursor;

/* current running task mode */
extern uint8 task_mode;

// full-black cross-line timing
extern uint8  full_black_cnt;   // full-black cross encounter count
extern uint8  last_full_black;  // previous frame full-black state (rising-edge detect)
extern uint32 start_systime;    // systime when KEY4 pressed
extern uint32 lap_time_20ms;    // systime when timing completes
extern uint8  lap_done;         // timing done flag
extern uint8  cross_black_thresh; // cross-line stop: how many sensors must be black simultaneously (tunable via XCN)
#define TOTAL_LAP   ( 3 ) // target lap count (reference)
extern uint32 systime_20ms;   // 20ms tick counter
extern uint16 startup_timer;  // two-phase startup timer (20ms ticks), 125=fast, >125=slow
extern uint8  state_lock_flag;
extern uint32 state_lock_cnt;

// state lock: lock 1s (50 * 20ms)
void state_lock(void);
uint8 state_lock_check(void);

// 20ms timer callback (registered to PIT_TIM_G12), i.e. main control tick
void task_stop(void);
void task3_start(void);
void task4_start(void);
void mode2_start(void);
void mode5_start(void);
void menu_confirm(void);
void pit_callback(uint32 event, void *ptr);

#endif
