/**
 ******************************************************************************
 * @file    Timer.c
 * @brief   TIM3定时器初始化 - 自由运行计数器(供超声波测距计时)
 *
 * ============================================================================
 *                          TIM3工作模式
 * ============================================================================
 *
 *   TIM3配置为自由运行向上计数器，不产生中断
 *   供超声波HCSR04驱动读取当前计数值(TIM3->CNT)来测量Echo脉宽
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │                    TIM3自由运行                             │
 *   │                                                             │
 *   │  计数器: 0 → 1 → 2 → ... → 0xFFFF → 0 → 1 → ...         │
 *   │          ↑_____1μs_____↑                                    │
 *   │                                                             │
 *   │  溢出周期: 65535μs ≈ 65.5ms                                │
 *   │  分辨率: 1μs                                                │
 *   │                                                             │
 *   │  HCSR04驱动: 在EXTI中断中读取 TIM3->CNT 获取时间戳          │
 *   │  距离计算: Δt(μs) × 0.173 mm/μs = Δt × 173/1000 mm        │
 *   └─────────────────────────────────────────────────────────────┘
 *
 * ============================================================================
 *                          TIM3配置参数
 * ============================================================================
 *
 *   时钟源: APB1 → 72MHz (STM32F103, APB1时钟×2 = 36×2 = 72MHz)
 *
 *   配置:
 *     预分频: 72-1 → 72MHz/72 = 1MHz (1μs计数一次)
 *     重装载: 0xFFFF → 自由运行，约65.5ms溢出一次
 *
 *   无中断: HCSR04的EXTI中断负责捕获Echo上升沿/下降沿
 *
 ******************************************************************************
 */

#include "Timer.h"                      // 定时器头文件
#include "stm32f10x.h"                 // STM32标准库

/* ============================================================================
 * 函数名: Timer_Init
 * 功能:   初始化TIM3为自由运行计数器 + NVIC优先级分组
 * 说明:   TIM3不产生中断，HCSR04的EXTI中断读取TIM3->CNT计时
 * ============================================================================ */
void Timer_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure; // 定时器基础结构体

    /* 使能TIM3时钟(APB1) */
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    /* NVIC优先级分组(全局设置，只调一次) */
    // 必须在其他外设使用NVIC之前调用
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2); // 2位抢占，2位子

    /* 配置TIM3时基: 1MHz自由运行 */
    // 时钟 = 72MHz / 72 = 1MHz → 计数器每1μs加1
    // 重装载值 = 0xFFFF → 自由运行，约65.5ms溢出一次
    TIM_TimeBaseStructure.TIM_Period        = 0xFFFF;            // 最大周期
    TIM_TimeBaseStructure.TIM_Prescaler     = 72 - 1;            // 1MHz时钟
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;      // 不分频
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;// 向上计数
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;              // 高级定时器用
    TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure);

    /* 启动TIM3(不使能中断，纯自由运行) */
    TIM_Cmd(TIM3, ENABLE);
}
