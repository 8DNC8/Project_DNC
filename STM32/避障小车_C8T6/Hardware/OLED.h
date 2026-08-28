/**
 * @file    OLED.h
 * @brief   OLED 0.96寸 I2C 显示屏驱动
 *
 * 硬件接线：
 *   PB12 -> SCL
 *   PB13 -> SDA
 *   VCC  -> 3.3V 或 5V
 *   GND  -> GND
 */

#ifndef __OLED_H
#define __OLED_H

#include "stm32f10x.h"

/* ======================================================================== */
/*  OLED 初始化                                                              */
/* ======================================================================== */

/**
 * @brief  初始化 OLED 显示屏
 */
void OLED_Init(void);

/**
 * @brief  清屏
 */
void OLED_Clear(void);

/* ======================================================================== */
/*  显示函数                                                                  */
/* ======================================================================== */

/**
 * @brief  OLED 显示一个字符
 * @param  Line   行位置，范围：1~4
 * @param  Column 列位置，范围：1~16
 * @param  Char   要显示的字符
 */
void OLED_ShowChar(uint8_t Line, uint8_t Column, char Char);

/**
 * @brief  OLED 显示字符串
 * @param  Line   行位置，范围：1~4
 * @param  Column 列位置，范围：1~16
 * @param  String 要显示的字符串
 */
void OLED_ShowString(uint8_t Line, uint8_t Column, char *String);

/**
 * @brief  OLED 显示数字（无符号整数）
 * @param  Line   行位置，范围：1~4
 * @param  Column 列位置，范围：1~16
 * @param  Number 要显示的数字
 * @param  Length 数字位数
 */
void OLED_ShowNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/**
 * @brief  OLED 显示数字（有符号整数）
 * @param  Line   行位置，范围：1~4
 * @param  Column 列位置，范围：1~16
 * @param  Number 要显示的数字（带符号）
 * @param  Length 数字位数
 */
void OLED_ShowSignedNum(uint8_t Line, uint8_t Column, int32_t Number, uint8_t Length);

/**
 * @brief  OLED 显示数字（十六进制）
 * @param  Line   行位置，范围：1~4
 * @param  Column 列位置，范围：1~16
 * @param  Number 要显示的数字（十六进制）
 * @param  Length 数字位数
 */
void OLED_ShowHexNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/**
 * @brief  OLED 显示数字（二进制）
 * @param  Line   行位置，范围：1~4
 * @param  Column 列位置，范围：1~16
 * @param  Number 要显示的数字（二进制）
 * @param  Length 数字位数
 */
void OLED_ShowBinNum(uint8_t Line, uint8_t Column, uint32_t Number, uint8_t Length);

/**
 * @brief  OLED 部分清屏
 * @param  Line   行位置，范围：1~4
 * @param  start  起始列，范围：1~16
 * @param  end    结束列，范围：1~16
 */
void OLED_Clear_Part(uint8_t Line, uint8_t start, uint8_t end);

/**
  * @brief  OLED显示中文（16x16点阵）
  * @param  Line   行位置，范围：1~4
  * @param  Column 列位置，范围：1~8（中文占2列宽度）
  * @param  Index  中文在字库中的索引（见OLED_Font.h）
  */
void OLED_ShowChinese(uint8_t Line, uint8_t Column, uint8_t Index);

/**
  * @brief  OLED显示一行中文
  * @param  Line   行位置，范围：1~4
  * @param  Column 起始列位置，范围：1~8（中文占2列宽度）
  * @param  Index  中文索引数组
  * @param  len    中文个数
  */
void OLED_ShowChineseStr(uint8_t Line, uint8_t Column, const uint8_t Index[], uint8_t len);

#endif /* __OLED_H */
