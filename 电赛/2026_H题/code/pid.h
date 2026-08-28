#ifndef PID_H
#define PID_H

#include "zf_common_headfile.h"

// incremental PID params and state
typedef struct
{
    float kp, ki, kd;     // incremental PID params
    float out_increment;  // incremental output delta
    float out;            // output value
    int16 set_speed;      // target speed
    int16 ek, ek1, ek2;   // current, previous, pre-previous error
} pid_increment_struct;

extern pid_increment_struct pid_increment;
extern pid_increment_struct pid_mode4;   // separate PID for MODE:4/5

// init PID with default params
void pid_init(void);

// incremental PID calculation (encoder = current actual speed)
void pid_increment_calc(pid_increment_struct *data, int16 encoder);

// reset PID state (clear output, error history), keep kp/ki/kd
void pid_reset(pid_increment_struct *data);

#endif
