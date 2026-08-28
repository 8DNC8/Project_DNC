/**
 ******************************************************************************
 * @file    Encoder.c
 * @brief   MG310霍尔编码器驱动 - TIM硬件正交编码模式
 *
 * 引脚:
 *   左编码器: PA8(TIM1_CH1)=A相(绿)  PA9(TIM1_CH2)=B相(蓝)
 *   右编码器: PB6(TIM4_CH1)=A相(绿)  PB7(TIM4_CH2)=B相(蓝)
 *
 * 工作原理:
 *   TIM1和TIM4配置为编码器模式(TI12), 硬件自动对A/B相脉冲做4倍频计数.
 *   正转时CNT递增, 反转时CNT递减 → 直接读取CNT即得带符号的位移.
 *   无需EXIT中断, 无CPU开销.
 *
 * 距离计算:
 *   distance_mm = (左脉冲 + 右脉冲) / 2 * 每脉冲距离
 *   4倍频后每圈计数 = ENC_PPR × 4
 ******************************************************************************
 */

#include "Encoder.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_tim.h"

/* 32位累加器，防止16位CNT溢出导致距离跳变 */
static int32_t  g_enc_left_acc  = 0;    /* 带符号，正=前进，负=后退 */
static int32_t  g_enc_right_acc = 0;
static uint16_t g_enc_left_last  = 0;
static uint16_t g_enc_right_last = 0;

/* 总里程累加器（永远只增不减，前进后退都累加） */
static uint32_t g_enc_left_dist  = 0;    /* 左轮总脉冲数（绝对值累加） */
static uint32_t g_enc_right_dist = 0;    /* 右轮总脉冲数（绝对值累加） */

/* ============================================================================
 * 函数: Encoder_Init
 * 功能: 初始化编码器GPIO + TIM1/TIM4硬件正交编码模式
 * ============================================================================ */
void Encoder_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;

    /* ---- 1. 使能时钟 ---- */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB |
                           RCC_APB2Periph_TIM1, ENABLE);
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4, ENABLE);

    /* ---- 2. 配置GPIO为浮空输入 ---- */
    /* 左编码器: PA8, PA9 */
    GPIO_InitStructure.GPIO_Pin   = ENC_LEFT_A_PIN | ENC_LEFT_B_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ENC_LEFT_PORT, &GPIO_InitStructure);

    /* 右编码器: PB6, PB7 */
    GPIO_InitStructure.GPIO_Pin   = ENC_RIGHT_A_PIN | ENC_RIGHT_B_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IN_FLOATING;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(ENC_RIGHT_PORT, &GPIO_InitStructure);

    /* ---- 3. 配置TIM1编码器模式 ---- */
    TIM_TimeBaseStructure.TIM_Period        = 0xFFFF;              /* 16位计数器 */
    TIM_TimeBaseStructure.TIM_Prescaler     = 0;                   /* 不分频 */
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;  /* 编码器模式会覆盖此设置 */
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM1, &TIM_TimeBaseStructure);

    /* 编码器模式: TI1+TI2双边沿都计数(4倍频) */
    TIM_EncoderInterfaceConfig(TIM1,
        TIM_EncoderMode_TI12,
        TIM_ICPolarity_Rising,
        TIM_ICPolarity_Rising);

    TIM_Cmd(TIM1, ENABLE);

    /* ---- 4. 配置TIM4编码器模式 ---- */
    TIM_TimeBaseStructure.TIM_Period        = 0xFFFF;
    TIM_TimeBaseStructure.TIM_Prescaler     = 0;
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM4, &TIM_TimeBaseStructure);

    TIM_EncoderInterfaceConfig(TIM4,
        TIM_EncoderMode_TI12,
        TIM_ICPolarity_Rising,
        TIM_ICPolarity_Rising);

    TIM_Cmd(TIM4, ENABLE);
}

/* ============================================================================
 * 函数: Encoder_Update
 * 功能: 每控制周期调用, 将TIM CNT增量累加到32位变量
 * 说明: uint16_t CNT通过 (int16_t)(now - last) 正确处理方向,
 *       只要 Δ<32768 (5ms内绝不可能超) 就不会有歧义
 *       同时累加绝对脉冲数到总里程累加器（前进后退都算）
 * ============================================================================ */
void Encoder_Update(void)
{
    uint16_t left_now  = TIM1->CNT;
    uint16_t right_now = TIM4->CNT;

    int16_t left_delta  = (int16_t)(left_now  - g_enc_left_last);
    int16_t right_delta = (int16_t)(right_now - g_enc_right_last);

    /* 带符号累加（用于位置跟踪） */
    g_enc_left_acc  += left_delta;
    g_enc_right_acc += right_delta;

    /* 绝对脉冲累加（用于总里程，前进后退都算） */
    g_enc_left_dist  += (left_delta  >= 0) ? (uint32_t)left_delta  : (uint32_t)(-left_delta);
    g_enc_right_dist += (right_delta >= 0) ? (uint32_t)right_delta : (uint32_t)(-right_delta);

    g_enc_left_last  = left_now;
    g_enc_right_last = right_now;
}

/* ============================================================================
 * 查询函数
 * ============================================================================ */

/**
 * @brief  获取左编码器脉冲计数 (32位累加值)
 * @return 带符号脉冲数 (正=前进, 负=后退)
 */
int32_t Encoder_GetLeft(void)
{
    return g_enc_left_acc;
}

/**
 * @brief  获取右编码器脉冲计数 (32位累加值)
 * @return 带符号脉冲数 (正=前进, 负=后退)
 */
int32_t Encoder_GetRight(void)
{
    return g_enc_right_acc;
}

/**
 * @brief  获取行驶距离 (总里程, 永远只增不减)
 * @return 左右轮平均行驶距离(单位0.1mm)
 * @note   使用绝对脉冲累加器, 前进后退都算里程, 距离永远增加
 */
int32_t Encoder_GetDistance(void)
{
    /* 使用总里程累加器（绝对脉冲数），不是带符号的累加器 */
    uint32_t avg = (g_enc_left_dist + g_enc_right_dist) / 2;
    return (int32_t)(((int64_t)avg * WHEEL_CIRC_MM * 10 + ENC_COUNTS_PER_REV / 2)
                     / ENC_COUNTS_PER_REV);
}

/**
 * @brief  清零编码器计数 (累加器 + TIM CNT)
 * @note   同时清零带符号累加器和总里程累加器
 */
void Encoder_Reset(void)
{
    g_enc_left_acc  = 0;
    g_enc_right_acc = 0;
    g_enc_left_dist  = 0;
    g_enc_right_dist = 0;
    g_enc_left_last  = 0;
    g_enc_right_last = 0;
    TIM1->CNT = 0;
    TIM4->CNT = 0;
}
