/*
 * dht11.h
 *
 *  说明：DHT11温湿度传感器驱动头文件
 *        硬件连接：PB9 → DHT11 DATA
 */
#ifndef __DHT11_H
#define __DHT11_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx_hal.h"

/**
 * @brief  微秒级延时
 */
void Delay_us(uint32_t udelay);

/**
 * @brief  发送DHT11起始信号
 */
void DHT11_Start(void);

/**
 * @brief  检测DHT11响应
 * @retval 0成功，1失败
 */
uint8_t DHT11_Check(void);

/**
 * @brief  读取一位数据
 */
uint8_t DHT11_Read_Bit(void);

/**
 * @brief  读取一个字节
 */
uint8_t DHT11_Read_Byte(void);

/**
 * @brief  读取温湿度数据
 * @param  temp 输出温度（℃）
 * @param  humi 输出湿度（%RH）
 * @retval 0成功，1失败
 */
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi);

#ifdef __cplusplus
}
#endif

#endif
