/*
 * adc_water.c
 *
 *  说明：水位传感器ADC采集驱动
 *        使用STM32内部ADC1通道0（PA0），12位分辨率，0~3.3V对应0~4095
 *        传感器输出电压经线性换算得到水位高度
 *
 *  换算关系：
 *    ADC原始值 → 电压（V）= adc_value × 3.3 / 4095
 *    电压 → 水位（mm）= voltage / 5.0 × 50
 *    （传感器5V供电，满量程5V对应50mm；STM32 ADC最大输入3.3V，对应33mm）
 */

#include "adc_water.h"
#include "main.h"

/* adc.c中定义的ADC1句柄 */
extern ADC_HandleTypeDef hadc1;

/**
 * @brief  水位传感器ADC初始化
 * @note   配置ADC1为单次转换、软件触发、右对齐、通道0（PA0）
 *         采样时间13.5个周期（较快，适合水位这种慢变化信号）
 */
void ADC_Water_Init(void)
{
    ADC_ChannelConfTypeDef sConfig = {0};

    /* 使能ADC1时钟（APB2） */
    __HAL_RCC_ADC1_CLK_ENABLE();

    /* ADC基础配置 */
    hadc1.Instance = ADC1;
    hadc1.Init.ScanConvMode = DISABLE;              /* 单通道模式 */
    hadc1.Init.ContinuousConvMode = DISABLE;        /* 单次转换 */
    hadc1.Init.DiscontinuousConvMode = DISABLE;     /* 禁用间断模式 */
    hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START; /* 软件触发转换 */
    hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;     /* 数据右对齐 */
    hadc1.Init.NbrOfConversion = 1;                 /* 转换1个通道 */
    if (HAL_ADC_Init(&hadc1) != HAL_OK)
    {
        Error_Handler();
    }

    /* 配置PA0为ADC通道0 */
    sConfig.Channel = ADC_CHANNEL_0;                /* 通道0对应PA0 */
    sConfig.Rank = ADC_REGULAR_RANK_1;              /* 第1个转换 */
    sConfig.SamplingTime = ADC_SAMPLETIME_13CYCLES_5; /* 采样时间13.5周期 */
    if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
    {
        Error_Handler();
    }
}

/**
 * @brief  读取ADC原始值
 * @retval ADC转换结果（0~4095，对应0~3.3V）
 * @note   软件触发单次转换，等待转换完成（超时100ms）
 */
uint32_t ADC_Water_Read(void)
{
    HAL_ADC_Start(&hadc1);
    if (HAL_ADC_PollForConversion(&hadc1, 100) == HAL_OK)
    {
        return HAL_ADC_GetValue(&hadc1);
    }
    return 0;  /* 转换超时返回0 */
}

/**
 * @brief  读取ADC并转换为电压值
 * @retval 电压值（V），范围0~3.3V
 * @note   12位ADC，参考电压3.3V：voltage = adc_value × 3.3 / 4095
 */
float ADC_Water_GetVoltage(void)
{
    uint32_t adc_value = ADC_Water_Read();
    return (float)adc_value * 3.3f / 4095.0f;
}

/**
 * @brief  读取ADC并转换为水位高度
 * @retval 水位高度（mm），范围0~50
 * @note   传感器5V供电，满量程5V对应50mm高度
 *         注意：STM32 ADC最大输入3.3V，实际可测最大水位约33mm
 *         若传感器输出超过3.3V需加分压电阻，否则ADC读数饱和
 */
uint8_t ADC_Water_GetHeight(void)
{
    float voltage = ADC_Water_GetVoltage();

    const float    max_voltage = 5.0f;   /* 传感器满量程电压（V） */
    const uint8_t  max_height  = 50;     /* 满量程水位高度（mm） */

    /* 限幅保护 */
    if (voltage > max_voltage) voltage = max_voltage;
    if (voltage < 0) voltage = 0;

    /* 线性换算并四舍五入 */
    return (uint8_t)(voltage / max_voltage * max_height + 0.5f);
}
