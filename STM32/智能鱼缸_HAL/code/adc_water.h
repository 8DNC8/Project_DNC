/*
 * adc_water.h
 *
 *  说明：水位传感器ADC采集驱动头文件
 *        硬件连接：PA0 → 水位传感器模拟输出
 */
#ifndef __ADC_WATER_H
#define __ADC_WATER_H

#include "stm32f1xx_hal.h"

/**
 * @brief  水位传感器ADC初始化（PA0，ADC1通道0）
 */
void ADC_Water_Init(void);

/**
 * @brief  读取ADC原始值
 * @retval 0~4095（对应0~3.3V）
 */
uint32_t ADC_Water_Read(void);

/**
 * @brief  读取ADC并转换为电压值
 * @retval 电压（V），0~3.3V
 */
float ADC_Water_GetVoltage(void);

/**
 * @brief  读取ADC并转换为水位高度
 * @retval 水位高度（mm），0~50
 */
uint8_t ADC_Water_GetHeight(void);

#endif
