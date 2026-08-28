/*
 * dht11.c
 *
 *  说明：DHT11温湿度传感器单总线驱动
 *        硬件连接：PB9 → DHT11 DATA
 *        通信协议：单总线，主机发送起始信号→DHT11响应→传输40位数据
 *        数据格式：湿度整数(8bit) + 湿度小数(8bit) + 温度整数(8bit) + 温度小数(8bit) + 校验和(8bit)
 *        精度：温度±2℃，湿度±5%RH；采样周期≥1s
 */

#include "dht11.h"
#include "main.h"
#include "stm32f1xx_it.h"
#include "tim.h"

/* DHT11引脚定义（在main.h中由CubeMX生成） */
#define DHT11_PORT  dht11_GPIO_Port
#define DHT11_PIN   dht11_Pin

/* 引脚电平操作宏 */
#define DHT11_HIGH()   HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_SET)    /* 拉高 */
#define DHT11_LOW()    HAL_GPIO_WritePin(DHT11_PORT, DHT11_PIN, GPIO_PIN_RESET)  /* 拉低 */
#define DHT11_READ()   HAL_GPIO_ReadPin(DHT11_PORT, DHT11_PIN)                   /* 读取引脚 */

/**
 * @brief  微秒级延时（基于SysTick VAL寄存器）
 * @param  udelay  延时微秒数
 * @note   SysTick配置为72MHz、1ms中断（72000周期重装载）
 *         通过读取VAL向下计数器实现精确us延时
 */
void Delay_us(uint32_t udelay)
{
    uint32_t startval, tickn, delays, wait;

    startval = SysTick->VAL;
    tickn = HAL_GetTick();
    delays = udelay * 72;  /* 72MHz → 72个时钟周期 = 1us */

    if (delays > startval)
    {
        /* 需要等待的周期数超过当前VAL剩余值，需跨越重装载边界 */
        while (HAL_GetTick() == tickn) {}  /* 等待VAL重装载 */
        wait = 72000 + startval - delays;
        while (wait < SysTick->VAL) {}
    }
    else
    {
        /* 不需要跨越边界，直接等待VAL递减到目标值 */
        wait = startval - delays;
        while (wait < SysTick->VAL && HAL_GetTick() == tickn) {}
    }
}

/**
 * @brief  配置DHT11数据引脚为推挽输出模式
 */
void Set_Pin_Output(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

/**
 * @brief  配置DHT11数据引脚为上拉输入模式
 */
void Set_Pin_Input(GPIO_TypeDef *GPIOx, uint16_t GPIO_Pin)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_Pin;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(GPIOx, &GPIO_InitStruct);
}

/**
 * @brief  发送DHT11起始信号
 * @note   主机拉低总线≥18ms，然后拉高30us等待DHT11响应
 */
void DHT11_Start(void)
{
    Set_Pin_Output(DHT11_PORT, DHT11_PIN);  /* 配置为输出 */
    DHT11_LOW();                            /* 拉低总线 */
    HAL_Delay(20);                          /* 保持≥18ms */
    DHT11_HIGH();                           /* 拉高总线 */
    Delay_us(30);                           /* 等待30us */
}

/**
 * @brief  检测DHT11响应信号
 * @retval 0：响应成功；1：响应失败（超时）
 * @note   DHT11收到起始信号后，拉低总线80us再拉高80us作为响应
 */
uint8_t DHT11_Check(void)
{
    uint8_t retry = 0;
    Set_Pin_Input(DHT11_PORT, DHT11_PIN);   /* 配置为输入 */

    /* 等待总线变低（DHT11拉低80us） */
    while (DHT11_READ() && retry < 100)
    {
        retry++;
        Delay_us(1);
    }
    if (retry >= 100) return 1;
    else retry = 0;

    /* 等待总线变高（DHT11拉高80us） */
    while (!DHT11_READ() && retry < 100)
    {
        retry++;
        Delay_us(1);
    }
    if (retry >= 100) return 1;

    return 0;
}

/**
 * @brief  从DHT11读取一位数据
 * @retval 读到的位值（0或1）
 * @note   DHT11每一位以50us低电平开始，高电平持续时间决定数据：
 *         26~28us高电平 → 0；70us高电平 → 1
 *         等待40us后读取引脚，若仍为高则为1，否则为0
 */
uint8_t DHT11_Read_Bit(void)
{
    uint8_t retry = 0;

    /* 等待50us低电平结束 */
    while (DHT11_READ() && retry < 100)
    {
        retry++;
        Delay_us(1);
    }
    retry = 0;

    /* 等待高电平开始 */
    while (!DHT11_READ() && retry < 100)
    {
        retry++;
        Delay_us(1);
    }

    /* 延时40us后采样：高电平仍在→1，已结束→0 */
    Delay_us(40);
    if (DHT11_READ()) return 1;
    else return 0;
}

/**
 * @brief  从DHT11读取一个字节（8位）
 * @retval 读到的字节值
 * @note   高位先传（MSB first）
 */
uint8_t DHT11_Read_Byte(void)
{
    uint8_t i, dat;
    dat = 0;
    for (i = 0; i < 8; i++)
    {
        dat <<= 1;
        dat |= DHT11_Read_Bit();
    }
    return dat;
}

/**
 * @brief  读取DHT11温湿度数据
 * @param  temp  输出：温度值（℃）
 * @param  humi  输出：湿度值（%RH）
 * @retval 0：读取成功；1：读取失败（无响应）
 * @note   数据格式：[湿度整数][湿度小数][温度整数][温度小数][校验和]
 *         校验：前4字节之和的低8位应等于第5字节
 */
uint8_t DHT11_Read_Data(uint8_t *temp, uint8_t *humi)
{
    uint8_t buf[5];
    uint8_t i;

    DHT11_Start();
    if (DHT11_Check() == 0)
    {
        for (i = 0; i < 5; i++)
        {
            buf[i] = DHT11_Read_Byte();
        }
        /* 校验和验证 */
        if (buf[0] + buf[1] + buf[2] + buf[3] == buf[4])
        {
            *humi = buf[0];  /* 湿度整数部分 */
            *temp = buf[2];  /* 温度整数部分 */
        }
    }
    else
    {
        return 1;  /* DHT11无响应 */
    }

    return 0;
}
