#include "motor.h"

// 初始化两个电机的 PWM（17kHz）与方向控制引脚
void motor_init(void)
{
    pwm_init(MOTOR1_PWM_PIN, 17 * 1000, 0);          // PWM 频率 17kHz
    gpio_init(MOTOR1_DIR_PIN, GPO, 0, GPO_PUSH_PULL);

    pwm_init(MOTOR2_PWM_PIN, 17 * 1000, 0);
    gpio_init(MOTOR2_DIR_PIN, GPO, 0, GPO_PUSH_PULL);
}

// 电机控制：duty 正=前进，负=后退，限幅到 PWM_DUTY_MAX
void motor_control(int32 duty1, int32 duty2)
{
    duty1 = func_limit(duty1, PWM_DUTY_MAX);
    duty2 = func_limit(duty2, PWM_DUTY_MAX);

    // 电机1
    if(duty1 >= 0)              // 前进
    {
        gpio_set_level(MOTOR1_DIR_PIN, 1);
        pwm_set_duty(MOTOR1_PWM_PIN, duty1);
    }
    else                        // 后退
    {
        gpio_set_level(MOTOR1_DIR_PIN, 0);
        pwm_set_duty(MOTOR1_PWM_PIN, -duty1);
    }

    // 电机2
    if(duty2 >= 0)              // 前进
    {
        gpio_set_level(MOTOR2_DIR_PIN, 1);
        pwm_set_duty(MOTOR2_PWM_PIN, duty2);
    }
    else                        // 后退
    {
        gpio_set_level(MOTOR2_DIR_PIN, 0);
        pwm_set_duty(MOTOR2_PWM_PIN, -duty2);
    }
}
