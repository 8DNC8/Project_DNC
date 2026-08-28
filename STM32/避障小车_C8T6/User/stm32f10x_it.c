/**
 ******************************************************************************
 * @file    stm32f10x_it.c
 * @brief   STM32中断处理文件
 ******************************************************************************
 */

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_it.h"               // 中断处理头文件

/* ============================================================================
 *                          外部变量 (来自 main.c)
 * ============================================================================ */

extern volatile uint32_t g_sys_tick;          /* SysTick毫秒计数 */

/* ============================================================================
 *                          异常处理函数(默认空实现)
 * ============================================================================ */

/**
 * @brief  复位处理
 */
void NMI_Handler(void)
{
}

/**
 * @brief  硬件Fault处理
 */
void HardFault_Handler(void)
{
    /* 死循环，便于调试 */
    while (1)
    {
    }
}

/**
 * @brief  内存管理Fault
 */
void MemManage_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief  总线Fault
 */
void BusFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief  使用Fault
 */
void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief  SVCall调用
 */
void SVC_Handler(void)
{
}

/**
 * @brief  调试Monitor
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  PendSV可悬挂调用
 */
void PendSV_Handler(void)
{
}

/**
 * @brief  SysTick定时器 (1ms)
 */
void SysTick_Handler(void)
{
    g_sys_tick++;
}

/* ============================================================================
 * 函数名: EXTI4_IRQHandler
 * 功能:   EXTI Line4中断服务程序
 * 说明:   处理PB4(Echo)的上升沿和下降沿中断
 *         实际处理逻辑在HCSR04.c的HCSR04_EXTI_Handler()中
 * ============================================================================ */
void EXTI4_IRQHandler(void)
{
    /* 声明外部函数(在HCSR04.c中定义) */
    extern void HCSR04_EXTI_Handler(void);

    HCSR04_EXTI_Handler();
}
