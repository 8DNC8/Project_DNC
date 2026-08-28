/*
 * OLED.c
 *
 *  说明：0.96寸OLED显示屏驱动（SSD1306控制器，128×64分辨率，I2C接口）
 *        硬件连接：PB6→SCL，PB7→SDA（硬件I2C1）
 *        I2C地址：0x78（8位写地址）
 *        支持：ASCII字符（6×8/8×16）、字符串、数字、中文汉字（16×16）
 */

#include "OLED.h"
#include "i2c.h"
#include "oledfont.h"

/* SSD1306初始化命令序列（27条） */
uint8_t CMD_Data[] = {
    0xAE, 0x00, 0x10, 0x40, 0xB0, 0x81, 0xFF, 0xA1, 0xA6, 0xA8, 0x3F,
    0xC8, 0xD3, 0x00, 0xD5, 0x80, 0xD8, 0x05, 0xD9, 0xF1, 0xDA, 0x12,
    0xD8, 0x30, 0x8D, 0x14, 0xAF
};

/**
 * @brief  发送SSD1306初始化命令序列
 */
void WriteCmd(void)
{
    uint8_t i;
    for (i = 0; i < 27; i++)
    {
        HAL_I2C_Mem_Write(&hi2c1, 0x78, 0x00, I2C_MEMADD_SIZE_8BIT,
                          CMD_Data + i, 1, 0x100);
    }
}

/**
 * @brief  向OLED写一条命令
 * @param  cmd  命令字节
 */
void OLED_WR_CMD(uint8_t cmd)
{
    HAL_I2C_Mem_Write(&hi2c1, 0x78, 0x00, I2C_MEMADD_SIZE_8BIT, &cmd, 1, 0x100);
}

/**
 * @brief  向OLED写一个显示数据
 * @param  data  数据字节（显存数据）
 */
void OLED_WR_DATA(uint8_t data)
{
    HAL_I2C_Mem_Write(&hi2c1, 0x78, 0x40, I2C_MEMADD_SIZE_8BIT, &data, 1, 0x100);
}

/**
 * @brief  OLED初始化（上电延时200ms后发送初始化命令）
 */
void OLED_Init(void)
{
    HAL_Delay(200);
    WriteCmd();
}

/**
 * @brief  清屏（所有显存写0）
 */
void OLED_Clear(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        OLED_WR_CMD(0xb0 + i);  /* 设置页地址（0~7） */
        OLED_WR_CMD(0x00);      /* 设置列地址低4位 */
        OLED_WR_CMD(0x10);      /* 设置列地址高4位 */
        for (n = 0; n < 128; n++)
            OLED_WR_DATA(0);
    }
}

/**
 * @brief  开启OLED显示
 */
void OLED_Display_On(void)
{
    OLED_WR_CMD(0x8D);  /* 电荷泵设置 */
    OLED_WR_CMD(0x14);  /* 开启电荷泵 */
    OLED_WR_CMD(0xAF);  /* 开启显示 */
}

/**
 * @brief  关闭OLED显示
 */
void OLED_Display_Off(void)
{
    OLED_WR_CMD(0x8D);  /* 电荷泵设置 */
    OLED_WR_CMD(0x10);  /* 关闭电荷泵 */
    OLED_WR_CMD(0xAE);  /* 关闭显示 */
}

/**
 * @brief  设置OLED显示光标位置
 * @param  x  列坐标（0~127）
 * @param  y  页坐标（0~7）
 */
void OLED_Set_Pos(uint8_t x, uint8_t y)
{
    OLED_WR_CMD(0xb0 + y);                    /* 设置页地址 */
    OLED_WR_CMD(((x & 0xf0) >> 4) | 0x10);    /* 设置列地址高4位 */
    OLED_WR_CMD(x & 0x0f);                    /* 设置列地址低4位 */
}

/**
 * @brief  全屏点亮（所有显存写1，用于测试）
 */
void OLED_On(void)
{
    uint8_t i, n;
    for (i = 0; i < 8; i++)
    {
        OLED_WR_CMD(0xb0 + i);
        OLED_WR_CMD(0x00);
        OLED_WR_CMD(0x10);
        for (n = 0; n < 128; n++)
            OLED_WR_DATA(1);
    }
}

/**
 * @brief  幂运算（内部使用）
 * @param  m  底数
 * @param  n  指数
 * @retval m^n
 */
unsigned int oled_pow(uint8_t m, uint8_t n)
{
    unsigned int result = 1;
    while (n--) result *= m;
    return result;
}

/**
 * @brief  显示无符号整数
 * @param  x      列坐标（0~127）
 * @param  y      页坐标（0~7）
 * @param  num    要显示的数字（0~4294967295）
 * @param  len    显示位数
 * @param  size2  字号（16=8×16，其他=6×8）
 */
void OLED_ShowNum(uint8_t x, uint8_t y, unsigned int num, uint8_t len, uint8_t size2)
{
    uint8_t t, temp;
    uint8_t enshow = 0;
    for (t = 0; t < len; t++)
    {
        temp = (num / oled_pow(10, len - t - 1)) % 10;
        if (enshow == 0 && t < (len - 1))
        {
            if (temp == 0)
            {
                OLED_ShowChar(x + (size2 / 2) * t, y, ' ', size2);
                continue;
            }
            else enshow = 1;
        }
        OLED_ShowChar(x + (size2 / 2) * t, y, temp + '0', size2);
    }
}

/**
 * @brief  显示单个ASCII字符
 * @param  x          列坐标（0~127）
 * @param  y          页坐标（0~7）
 * @param  chr        字符（ASCII可见字符）
 * @param  Char_Size  字号（16=8×16，其他=6×8）
 */
void OLED_ShowChar(uint8_t x, uint8_t y, uint8_t chr, uint8_t Char_Size)
{
    unsigned char c = 0, i = 0;
    c = chr - ' ';  /* 计算字库偏移（从空格开始） */
    if (x > 128 - 1) { x = 0; y = y + 2; }

    if (Char_Size == 16)
    {
        /* 8×16字体：上半部分8字节在当前页，下半部分8字节在下一页 */
        OLED_Set_Pos(x, y);
        for (i = 0; i < 8; i++)
            OLED_WR_DATA(F8X16[c * 16 + i]);
        OLED_Set_Pos(x, y + 1);
        for (i = 0; i < 8; i++)
            OLED_WR_DATA(F8X16[c * 16 + i + 8]);
    }
    else
    {
        /* 6×8字体：6字节在同一页 */
        OLED_Set_Pos(x, y);
        for (i = 0; i < 6; i++)
            OLED_WR_DATA(F6x8[c][i]);
    }
}

/**
 * @brief  显示ASCII字符串
 * @param  x          起始列坐标
 * @param  y          页坐标
 * @param  chr        字符串指针
 * @param  Char_Size  字号（16=8×16，其他=6×8）
 */
void OLED_ShowString(uint8_t x, uint8_t y, uint8_t *chr, uint8_t Char_Size)
{
    unsigned char j = 0;
    while (chr[j] != '\0')
    {
        OLED_ShowChar(x, y, chr[j], Char_Size);
        x += 8;
        if (x > 120) { x = 0; y += 2; }  /* 超出屏幕自动换行 */
        j++;
    }
}

/**
 * @brief  显示一个16×16中文汉字
 * @param  x   列坐标（0~112）
 * @param  y   页坐标（0~6，偶数页）
 * @param  no  汉字在字库Hzk数组中的索引
 * @note   每个汉字占2页×16列=32字节
 */
void OLED_ShowCHinese(uint8_t x, uint8_t y, uint8_t no)
{
    uint8_t t;
    OLED_Set_Pos(x, y);
    for (t = 0; t < 16; t++)
    {
        OLED_WR_DATA(Hzk[2 * no][t]);
    }
    OLED_Set_Pos(x, y + 1);
    for (t = 0; t < 16; t++)
    {
        OLED_WR_DATA(Hzk[2 * no + 1][t]);
    }
}
