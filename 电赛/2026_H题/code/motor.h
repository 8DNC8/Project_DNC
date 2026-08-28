#ifndef MOTOR_H
#define MOTOR_H

#include "zf_common_headfile.h"

// motor count and base speed for MODE:2 (default 22, then 15 after 13 s)
#define MOTOR_NUM       ( 2 )
#define MOTOR_SPEED     ( 22  )

// motor 1: PWM channel + direction pin
#define MOTOR1_PWM_PIN  PWM_TIM_A0_CH0_A0
#define MOTOR1_DIR_PIN  A1

// motor 2: PWM channel + direction pin
#define MOTOR2_PWM_PIN  PWM_TIM_A0_CH2_B12
#define MOTOR2_DIR_PIN  B13

// init motor PWM and direction pins (DRV8701 driver)
void motor_init(void);

// motor control: positive duty = forward, negative = reverse, auto-limited to PWM_DUTY_MAX
void motor_control(int32 duty1, int32 duty2);
  
#endif
