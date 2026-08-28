#include "pid.h"

pid_increment_struct pid_increment;
pid_increment_struct pid_mode4;

// init PID default params (kp/ki/kd tuned for car feel)
void pid_init(void)
{
    pid_increment.kp = 70;
    pid_increment.ki = 30;
    pid_increment.kd = 20;

    /* MODE:4/5 PID: gentler gains for lower-speed tracking */
    pid_mode4.kp = 55;
    pid_mode4.ki = 20;
    pid_mode4.kd = 15;
}

// incremental PID calculation: output limited to PWM_DUTY_MAX
void pid_increment_calc(pid_increment_struct *data, int16 encoder)
{
    float temp_out;

    data->ek2 = data->ek1;                                  // save pre-previous error
    data->ek1 = data->ek;                                   // save previous error
    data->ek  = data->set_speed - encoder;                  // compute current error

    // incremental PID operation
    data->out_increment = (int16)(data->kp * (data->ek - data->ek1)
                                + data->ki * data->ek
                                + data->kd * (data->ek - 2 * data->ek1 + data->ek2));
    // compute new output
    temp_out = data->out + data->out_increment;
    // limit output, must not exceed max duty
    data->out = func_limit(temp_out, PWM_DUTY_MAX);
}

// reset PID state, keep kp/ki/kd
void pid_reset(pid_increment_struct *data)
{
    data->out_increment = 0;
    data->out           = 0;
    data->set_speed     = 0;
    data->ek            = 0;
    data->ek1           = 0;
    data->ek2           = 0;
}
