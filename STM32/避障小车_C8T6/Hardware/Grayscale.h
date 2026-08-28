/**
 ******************************************************************************
 * @file    Grayscale.h
 * @brief   8路灰度循迹传感器驱动 - 头文件
 *
 * 传感器型号: 龙邱 8路模拟量灰度循迹模块 (CD4051多路复用)
 *
 * 接线:
 *   PB9  → S0 (通道选择bit0)     PB10 → S1 (通道选择bit1)
 *   PB11 → S2 (通道选择bit2)     PA7  → AS (模拟输出, ADC12_IN7, 需飞线)
 *
 * 通道选择: S2 S1 S0 = 000→CH0, 001→CH1, ..., 111→CH7
 *
 * 权重:
 *   CH0=-10(最左)  CH1=-7  CH2=-4  CH3=-1
 *   CH4=+1  CH5=+4  CH6=+7  CH7=+10(最右)
 *
 * 误差范围: -10 ~ +10
 ******************************************************************************
 */

#ifndef __GRAYSCALE_H
#define __GRAYSCALE_H

#include "stm32f10x.h"

/* ============================================================================
 *                              引脚定义
 * ============================================================================ */

#define GS_S0_PORT          GPIOB
#define GS_S0_PIN           GPIO_Pin_9    /* PB9 → S0 通道选择bit0 */
#define GS_S1_PORT          GPIOB
#define GS_S1_PIN           GPIO_Pin_10   /* PB10 → S1 通道选择bit1 */
#define GS_S2_PORT          GPIOB
#define GS_S2_PIN           GPIO_Pin_11   /* PB11 → S2 通道选择bit2 */
#define GS_CHANNEL_MASK     (GS_S0_PIN | GS_S1_PIN | GS_S2_PIN)

#define GS_ADC_PORT         GPIOA
#define GS_ADC_PIN          GPIO_Pin_7    /* PA7 → AS (飞线, ADC12_IN7) */
#define GS_ADC_CHANNEL      ADC_Channel_7
#define GS_ADC_GPIO_RCC     RCC_APB2Periph_GPIOA
#define GS_ADC_RCC          RCC_APB2Periph_ADC1

/* ============================================================================
 *                              参数
 * ============================================================================ */

#define GS_NUM_CHANNELS     8

/* ============================================================================
 *                              数据结构
 * ============================================================================ */

typedef struct {
    uint8_t  ch[8];                       /* 每路二值化: 0=白地, 1=黑线 */
    uint16_t raw[8];                      /* 每路原始ADC值 (调试用) */
    int8_t   error;                       /* 加权误差 -15 ~ +15 */
} GS_Status_t;

/* 全局变量: 最近一次8路原始ADC值 (供OLED调试) */
extern uint16_t g_raw_adc[8];

/* ============================================================================
 *                              函数声明
 * ============================================================================ */

void GS_Init(void);
void GS_Calibrate(uint32_t duration_ms);

/**
 * @brief  读取8路传感器并计算误差
 * @return 包含ch[0..7], error的GS_Status_t
 */
GS_Status_t GS_Read(void);

uint8_t GS_AllBlack(GS_Status_t *s);
uint8_t GS_AllWhite(GS_Status_t *s);
uint8_t GS_IsTJunction(GS_Status_t *s);

/**
 * @brief  获取校准后的每路阈值 (调试用)
 * @param  idx - 通道号(0~7)
 * @return 该通道的ADC阈值
 */
uint16_t GS_GetThreshold(uint8_t idx);

#endif /* __GRAYSCALE_H */
