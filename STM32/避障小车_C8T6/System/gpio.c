#include "gpio.h"

#include "stm32f10x_rcc.h"

/**
 * @brief  初始化 L298N 控制所需的 GPIO
 *
 *  PA0 / PA3：复用推挽输出（AF_PP），输出 TIM2 PWM 信号给 ENA / ENB
 *  PA1 / PA2 / PA4 / PA5：普通推挽输出（Out_PP），控制电机转向
 */
void GPIO_Configuration(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能 GPIOA 时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    /* --- 方向控制引脚：IN1 IN2 IN3 IN4 --- */
    GPIO_InitStructure.GPIO_Pin   = MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN |
                                    MOTOR_B_IN3_PIN | MOTOR_B_IN4_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_GPIO_PORT, &GPIO_InitStructure);

    /* 上电默认停止：所有方向引脚拉低 */
    GPIO_ResetBits(MOTOR_GPIO_PORT,
                   MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN |
                   MOTOR_B_IN3_PIN | MOTOR_B_IN4_PIN);

    /* --- PWM 使能引脚：ENA(PA0)  ENB(PA3)，复用推挽 --- */
    GPIO_InitStructure.GPIO_Pin   = MOTOR_A_ENA_PIN | MOTOR_B_ENB_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_GPIO_PORT, &GPIO_InitStructure);
}


