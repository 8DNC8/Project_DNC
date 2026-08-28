/**
 ******************************************************************************
 * @file    Motor.h
 * @brief   TB6612FNG双路电机驱动 - 头文件
 *
 * ============================================================================
 *                              快速参考
 * ============================================================================
 *
 *   TB6612FNG vs L298N 对比:
 *   ┌──────────┬──────────────────┬──────────────────┐
 *   │          │     L298N        │    TB6612FNG      │
 *   ├──────────┼──────────────────┼──────────────────┤
 *   │ 驱动管   │ BJT(双极型)      │ MOSFET(场效应)    │
 *   │ 压降     │ ~2V(发热大)      │ ~0.3V(效率高)     │
 *   │ 体积     │ 大               │ 小(约1/4)         │
 *   │ 需要STBY │ 不需要           │ 需要(高电平使能)   │
 *   │ 最大电流 │ 2A               │ 1.2A(峰值3.2A)    │
 *   └──────────┴──────────────────┴──────────────────┘
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │  STM32F103C8T6              TB6612FNG                       │
 *   ├─────────────────────────────────────────────────────────────┤
 *   │  PA0 (TIM2_CH1)    ───►   PWMA   (左电机PWM)               │
 *   │  PA1               ───►   AIN1   (左电机方向A)             │
 *   │  PA2               ───►   AIN2   (左电机方向B)             │
 *   │  PA3 (TIM2_CH4)    ───►   PWMB   (右电机PWM)               │
 *   │  PA4               ───►   BIN1   (右电机方向A)             │
 *   │  PA5               ───►   BIN2   (右电机方向B)             │
 *   │  PA6               ───►   STBY   (待机控制，高电平使能)     │
 *   └─────────────────────────────────────────────────────────────┘
 *
 *   PWM参数:
 *     频率 = 72MHz / 72 / 1000 = 1kHz (周期1ms)
 *     占空比 = CCR值 / 1000 (0~100%)
 *     范围: 0~999
 *
 *   速度参数宏:
 *     SPEED_STOP  = 0    (停止)
 *     SPEED_LOW   = 300  (低速 ~30%)
 *     SPEED_MID   = 550  (中速 ~55%)
 *     SPEED_HIGH  = 750  (高速 ~75%)
 *     SPEED_FULL  = 999  (全速 100%)
 *
 ******************************************************************************
 */

#ifndef __MOTOR_H                       // 防止重复包含
#define __MOTOR_H

#include "stm32f10x.h"                  // STM32标准库

/* ============================================================================
 *                              引脚定义
 * ============================================================================ */

#define MOTOR_GPIO_PORT     GPIOA        // GPIO端口: GPIOA

#define MOTOR_A_PWM_PIN     GPIO_Pin_0   // PA0 → TIM2_CH1 → PWMA (左电机PWM)
#define MOTOR_A_IN1_PIN     GPIO_Pin_1   // PA1 → AIN1 (左电机方向A)
#define MOTOR_A_IN2_PIN     GPIO_Pin_2   // PA2 → AIN2 (左电机方向B)
#define MOTOR_B_PWM_PIN     GPIO_Pin_3   // PA3 → TIM2_CH4 → PWMB (右电机PWM)
#define MOTOR_B_IN1_PIN     GPIO_Pin_4   // PA4 → BIN1 (右电机方向A)
#define MOTOR_B_IN2_PIN     GPIO_Pin_5   // PA5 → BIN2 (右电机方向B)
#define MOTOR_STBY_PIN      GPIO_Pin_6   // PA6 → STBY (待机控制，高电平=使能)

/* ============================================================================
 *                              PWM参数
 * ============================================================================ */

// TIM2时钟 = 72MHz
// 预分频72 → 1MHz (1us计数一次)
// 重装载值1000 → 1kHz PWM周期
#define MOTOR_TIM_PRESCALER  71          // 预分频: 72MHz/72 = 1MHz
#define MOTOR_TIM_PERIOD     999         // 重装载: 1MHz/1000 = 1kHz

/* ============================================================================
 *                              速度宏
 * ============================================================================ */

#define SPEED_STOP           0          // 停止 (0%)
#define SPEED_LOW            300         // 低速 (~30%)
#define SPEED_MID            550         // 中速 (~55%)
#define SPEED_HIGH           750         // 高速 (~75%)
#define SPEED_FULL           999         // 全速 (100%)

/* ============================================================================
 *                              方向宏
 * ============================================================================ */

#define DIR_FORWARD          1           // 前进
#define DIR_BACKWARD         2           // 后退
#define DIR_STOP             0           // 停止

/* ============================================================================
 *                              函数声明
 * ============================================================================ */

/**
 * @brief  初始化电机驱动
 * @note   配置GPIO + TIM2 PWM输出 + STBY使能
 */
void Motor_Init(void);

/**
 * @brief  设置左电机速度和方向
 * @param  speed - 速度值
 *         >0: 正转(前进), 0~999
 *         <0: 反转(后退), -999~0
 *         =0: 停止
 */
void Motor_Left_Set(int16_t speed);

/**
 * @brief  设置右电机速度和方向
 * @param  speed - 速度值(同左电机)
 */
void Motor_Right_Set(int16_t speed);

/**
 * @brief  直线前进
 * @param  speed - 速度(0~999)
 */
void Car_Forward(uint16_t speed);

/**
 * @brief  直线后退
 * @param  speed - 速度(0~999)
 */
void Car_Backward(uint16_t speed);

/**
 * @brief  差速左转
 * @param  base_speed  - 基础速度
 * @param  turn_offset - 转向偏移量
 */
void Car_TurnLeft(uint16_t base_speed, uint16_t turn_offset);

/**
 * @brief  差速右转
 * @param  base_speed  - 基础速度
 * @param  turn_offset - 转向偏移量
 */
void Car_TurnRight(uint16_t base_speed, uint16_t turn_offset);

/**
 * @brief  停车(双轮滑行/Coast)
 */
void Car_Stop(void);
void Motor_Brake(void);

#endif /* __MOTOR_H */
