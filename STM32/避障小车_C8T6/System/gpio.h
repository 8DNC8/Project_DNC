#ifndef __GPIO_H
#define __GPIO_H

#include "stm32f10x.h"

/* ============================================================
 *  L298N 引脚定义
 *  接线说明：
 *    ENA  -> PA0  (TIM2_CH1, PWM 控制左电机速度)
 *    IN1  -> PA1  (左电机方向)
 *    IN2  -> PA2  (左电机方向)
 *    ENB  -> PA3  (TIM2_CH4, PWM 控制右电机速度)
 *    IN3  -> PA4  (右电机方向)
 *    IN4  -> PA5  (右电机方向)
 * ============================================================ */

#define MOTOR_GPIO_PORT     GPIOA

/* 使能引脚（PWM，复用推挽） */
#define MOTOR_A_ENA_PIN     GPIO_Pin_0   /* PA0 -> TIM2_CH1 */
#define MOTOR_B_ENB_PIN     GPIO_Pin_3   /* PA3 -> TIM2_CH4 */

/* 方向控制引脚（普通推挽输出） */
#define MOTOR_A_IN1_PIN     GPIO_Pin_1   /* PA1 */
#define MOTOR_A_IN2_PIN     GPIO_Pin_2   /* PA2 */
#define MOTOR_B_IN3_PIN     GPIO_Pin_4   /* PA4 */
#define MOTOR_B_IN4_PIN     GPIO_Pin_5   /* PA5 */

/* PWM 参数
 *   定时器时钟 = 72 MHz（APB1 × 2）
 *   PWM 周期  = (TIM_PRESCALER + 1) × (TIM_PERIOD + 1) / 72MHz
 *             = 72 × 1000 / 72MHz = 1 ms → 1 kHz
 *   占空比范围 0 ~ 999
 */
#define TIM_PRESCALER       71
#define TIM_PERIOD          999

/* 速度宏（占空比值 0~999） */
#define SPEED_STOP          0
#define SPEED_LOW           400     /* 约 40% */
#define SPEED_MID           600     /* 约 60% */
#define SPEED_HIGH          800     /* 约 80% */
#define SPEED_FULL          999     /* 100%   */

/* 方向宏 */
#define DIR_FORWARD         1
#define DIR_BACKWARD        2
#define DIR_STOP            0

/* 函数声明 */
void GPIO_Configuration(void);

#endif /* __GPIO_H */






