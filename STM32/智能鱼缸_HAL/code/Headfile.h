/*
 * Headfile.h
 *
 *  说明：全局头文件汇总，统一包含项目所需的所有头文件
 *        在main.c中包含此文件即可引入所有驱动和库
 */
#ifndef __Headfile_H__
#define __Headfile_H__

#include "stm32f1xx_hal.h"
#include "main.h"
#include "gpio.h"
#include <stdarg.h>
#include <stdint.h>
#include "string.h"
#include "stdio.h"
#include "oled.h"
#include "tim.h"
#include "i2c.h"
#include "dht11.h"
#include "adc_water.h"
#include "usart.h"

#endif
