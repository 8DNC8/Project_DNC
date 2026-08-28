/**
 ******************************************************************************
 * @file    MPU6050.c
 * @brief   MPU6050六轴陀螺仪+加速度计驱动
 *
 * ============================================================================
 *                          MPU6050模块介绍
 * ============================================================================
 *
 *   MPU6050是InvenSense公司生产的六轴运动处理传感器
 *   集成三轴陀螺仪 + 三轴加速度计
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │                    MPU6050 模块                            │
 *   │                                                             │
 *   │        ┌──────────────────────┐                            │
 *   │        │                      │                            │
 *   │        │      ┌────────┐      │                            │
 *   │        │      │ MPU    │      │                            │
 *   │        │      │ 6050   │      │                            │
 *   │        │      └────────┘      │                            │
 *   │        │                      │                            │
 *   │        └──────────────────────┘                            │
 *   │              VCC SDA SCL GND AD0                           │
 *   └─────────────────────────────────────────────────────────────┘
 *
 * ============================================================================
 *                          陀螺仪原理
 * ============================================================================
 *
 *   陀螺仪测量角速度(°/s)，即物体旋转的快慢
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │                      角速度方向                            │
 *   │                                                             │
 *   │           Z轴(偏航)      X轴(俯仰)      Y轴(横滚)          │
 *   │              ↑               ↺               ↻            │
 *   │              │             /               \               │
 *   │              │            /                 \              │
 *   │              │           │                   |             │
 *   │                                                             │
 *   │   绕Z轴旋转=转头(左右看)  绕X轴=点头    绕Y轴=摇头          │
 *   │                                                             │
 *   └─────────────────────────────────────────────────────────────┘
 *
 *   在循迹小车中:
 *     - Z轴角速度用于检测转弯角度
 *     - 可用于: 弯道计数、转角反馈、姿态稳定
 *
 * ============================================================================
 *                          加速度计原理
 * ============================================================================
 *
 *   加速度计测量加速度(m/s²)，包含重力加速度
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │                      三轴加速度                            │
 *   │                                                             │
 *   │        X轴 ←─────────●─────────→ X轴                      │
 *   │                    重力                                    │
 *   │                    ↓g                                     │
 *   │        Y轴 ←─────────┼─────────→ Y轴                      │
 *   │                      │                                     │
 *   │                      ↓                                     │
 *   │                   Z轴                                      │
 *   │                                                             │
 *   └─────────────────────────────────────────────────────────────┘
 *
 *   静止时: Z轴≈+g, X/Y轴≈0
 *   运动时: 各轴包含运动加速度和重力分量
 *
 * ============================================================================
 *                          I2C通信协议
 * ============================================================================
 *
 *   I2C是两线串行通信协议: SCL(时钟线) + SDA(数据线)
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │                      I2C时序                               │
 *   │                                                             │
 *   │  SCL ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─┐ ┌─────┐               │
 *   │      │ │ │ │ │ │ │ │ │ │ │ │ │ │ │ │ │     │               │
 *   │      └─┘ └─┘ └─┘ └─┘ └─┘ └─┘ └─┘ └─┘ └─────┘               │
 *   │                                                             │
 *   │  SDA ──┐   ┌───┐   ┌───┐   ┌───┐   ┌───┐   ┌───┐          │
 *   │      │ D7│ D6│ D5│ D4│ D3│ D2│ D1│ D0│ACK│                  │
 *   │      └───┘   └───┘   └───┘   └───┘   └───┘   └───┘          │
 *   │                                                             │
 *   │        起始条件      数据位           停止条件              │
 *   └─────────────────────────────────────────────────────────────┘
 *
 *   本驱动使用软件模拟I2C(GPIO模拟)，不依赖硬件I2C外设
 *
 * ============================================================================
 *                          硬件接线
 * ============================================================================
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │  STM32F103C8T6              MPU6050                        │
 *   ├─────────────────────────────────────────────────────────────┤
 *   │  PB12 (GPIO开漏)       ───►   SCL (I2C时钟, 与OLED共用)  │
 *   │  PB13 (GPIO开漏)       ───►   SDA (I2C数据, 与OLED共用)  │
 *   │  3.3V                  ───►   VCC (注意:必须3.3V!)         │
 *   │  GND                   ───►   GND                          │
 *   │  GND                   ───►   AD0 (接地 → 地址=0xD0)      │
 *   └─────────────────────────────────────────────────────────────┘
 *
 *   重要: MPU6050必须使用3.3V供电！接5V会损坏芯片！
 *
 * ============================================================================
 *                          寄存器配置
 * ============================================================================
 *
 *   主要寄存器:
 *     0x6B PWR_MGMT1  - 电源管理
 *     0x19 SMPLRT_DIV - 采样率分频
 *     0x1A CONFIG      - 配置文件(低通滤波器)
 *     0x1B GYRO_CONFIG - 陀螺仪配置(量程)
 *     0x1C ACCEL_CONFIG - 加速度计配置(量程)
 *     0x3B ACCEL_XOUT_H - 加速度计数据
 *     0x47 GYRO_ZOUT_H  - 陀螺仪Z轴数据
 *     0x75 WHO_AM_I    - 设备ID(应为0x68)
 *
 *   本驱动配置:
 *     陀螺仪量程: ±500°/s
 *     加速度计量程: ±4g
 *     采样率: 125Hz
 *     低通滤波: 98Hz
 *
 ******************************************************************************
 */

#include "MPU6050.h"                    // MPU6050头文件
#include "Delay.h"                      // 延时函数
#include "stm32f10x_rcc.h"              // RCC时钟
#include "stm32f10x_gpio.h"             // GPIO配置

/* ============================================================================
 *                              引脚定义
 * ============================================================================ */

#define MPU6050_I2C_PORT     GPIOB       // GPIO端口: GPIOB
#define MPU6050_SCL_PIN       GPIO_Pin_12 // SCL引脚: PB12 (与OLED共用)
#define MPU6050_SDA_PIN       GPIO_Pin_13 // SDA引脚: PB13 (与OLED共用)
#define MPU6050_ADDR          0xD0       // I2C地址(7位)=0x68, 写操作=0xD0

/* ============================================================================
 *                              寄存器定义
 * ============================================================================ */

#define MPU6050_REG_PWR_MGMT1     0x6B   // 电源管理寄存器1
#define MPU6050_REG_SMPLRT_DIV    0x19   // 采样率分频寄存器
#define MPU6050_REG_CONFIG        0x1A   // 配置寄存器
#define MPU6050_REG_GYRO_CONFIG   0x1B   // 陀螺仪配置寄存器
#define MPU6050_REG_ACCEL_CONFIG  0x1C   // 加速度计配置寄存器
#define MPU6050_REG_ACCEL_XOUT_H  0x3B   // 加速度计X轴高字节
#define MPU6050_REG_GYRO_ZOUT_H   0x47   // 陀螺仪Z轴高字节
#define MPU6050_REG_WHO_AM_I      0x75   // 设备ID寄存器

/* ============================================================================
 *                              静态变量
 * ============================================================================ */

static int16_t s_gyro_z_offset = 0;      // 【Z轴零偏】
                                              // 校准过程中记录静止时的Z轴值
                                              // 实际读数减去零偏，消除偏移误差

/* ============================================================================
 *                          软件I2C底层操作
 * ============================================================================ */

/* GPIO操作宏定义 */
#define I2C_SCL_H()   GPIO_SetBits(MPU6050_I2C_PORT, MPU6050_SCL_PIN)     // SCL=1
#define I2C_SCL_L()   GPIO_ResetBits(MPU6050_I2C_PORT, MPU6050_SCL_PIN)   // SCL=0
#define I2C_SDA_H()   GPIO_SetBits(MPU6050_I2C_PORT, MPU6050_SDA_PIN)     // SDA=1
#define I2C_SDA_L()   GPIO_ResetBits(MPU6050_I2C_PORT, MPU6050_SDA_PIN)   // SDA=0
#define I2C_SDA_READ() GPIO_ReadInputDataBit(MPU6050_I2C_PORT, MPU6050_SDA_PIN) // 读SDA

/**
 * @brief  设置SDA为输出模式
 */
static void I2C_SDA_Out(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = MPU6050_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD; // 开漏输出(可双向)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MPU6050_I2C_PORT, &GPIO_InitStructure);
}

/**
 * @brief  设置SDA为输入模式
 */
static void I2C_SDA_In(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin   = MPU6050_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPU;    // 上拉输入
    GPIO_Init(MPU6050_I2C_PORT, &GPIO_InitStructure);
}

/**
 * @brief  I2C起始信号
 * @note  SCL高时，SDA从高变低
 */
static void I2C_Start(void)
{
    I2C_SDA_Out();                       // SDA设为输出
    I2C_SDA_H();                         // SDA=1
    I2C_SCL_H();                         // SCL=1
    Delay_us(5);                         // 等待稳定
    I2C_SDA_L();                         // SDA: 1→0 (起始信号)
    Delay_us(5);                         // 保持
    I2C_SCL_L();                         // SCL=0 (准备开始传输)
}

/**
 * @brief  I2C停止信号
 * @note  SCL高时，SDA从低变高
 */
static void I2C_Stop(void)
{
    I2C_SDA_Out();                       // SDA设为输出
    I2C_SCL_L();                         // SCL=0
    I2C_SDA_L();                         // SDA=0
    Delay_us(5);                         // 保持
    I2C_SCL_H();                         // SCL=1
    Delay_us(5);                         // 等待稳定
    I2C_SDA_H();                         // SDA: 0→1 (停止信号)
    Delay_us(5);                         // 停止后总线空闲
}

/**
 * @brief  I2C发送一个字节
 * @param  byte - 要发送的数据
 * @return 0=收到ACK, 1=收到NACK
 */
static uint8_t I2C_SendByte(uint8_t byte)
{
    uint8_t i;
    I2C_SDA_Out();                       // SDA设为输出

    // 发送8位数据(高位在前)
    for (i = 0; i < 8; i++)
    {
        if (byte & 0x80)                  // 最高位为1?
            I2C_SDA_H();                  // SDA=1
        else
            I2C_SDA_L();                  // SDA=0
        byte <<= 1;                       // 左移，准备下一位
        Delay_us(2);                      // 等待SDA稳定
        I2C_SCL_H();                      // SCL产生上升沿，通知从机采样
        Delay_us(5);                      // 保持高电平
        I2C_SCL_L();                      // SCL低，准备下一位
        Delay_us(2);                      // 准备
    }

    // 读取应答位
    I2C_SDA_In();                         // SDA设为输入(从机可以拉低)
    Delay_us(2);                          // 等待应答
    I2C_SCL_H();                          // SCL高，应答位有效
    Delay_us(5);                          // 保持
    i = I2C_SDA_READ();                  // 读取应答位
    I2C_SCL_L();                          // SCL低
    Delay_us(2);                          // 准备

    return i;                             // 0=ACK, 1=NACK
}

/**
 * @brief  I2C接收一个字节
 * @param  ack - 1=发送ACK, 0=发送NACK
 * @return 接收到的数据
 */
static uint8_t I2C_ReadByte(uint8_t ack)
{
    uint8_t i, byte = 0;
    I2C_SDA_In();                         // SDA设为输入

    // 接收8位数据
    for (i = 0; i < 8; i++)
    {
        I2C_SCL_L();                      // SCL低
        Delay_us(2);                      // 等待
        I2C_SCL_H();                      // SCL高，主机读取数据
        Delay_us(2);                      // 等待数据稳定
        byte <<= 1;                       // 左移，准备接收
        if (I2C_SDA_READ())               // 读取SDA
            byte |= 0x01;                 // 置位最低位
    }

    // 发送应答位
    I2C_SCL_L();                          // SCL低
    I2C_SDA_Out();                        // SDA设为输出
    if (ack)
        I2C_SDA_L();                      // ACK=0 (继续接收)
    else
        I2C_SDA_H();                      // NACK=1 (停止接收)
    Delay_us(2);                           // 等待
    I2C_SCL_H();                          // SCL高，应答位有效
    Delay_us(5);                          // 保持
    I2C_SCL_L();                           // SCL低
    Delay_us(2);                           // 准备

    return byte;                          // 返回接收的数据
}

/* ============================================================================
 *                          MPU6050寄存器操作
 * ============================================================================ */

/**
 * @brief  写寄存器
 * @param  reg - 寄存器地址
 * @param  data - 要写入的数据
 */
static void MPU6050_WriteReg(uint8_t reg, uint8_t data)
{
    I2C_Start();                          // 发送起始信号
    I2C_SendByte(MPU6050_ADDR);           // 发送设备地址(写)
    I2C_SendByte(reg);                    // 发送寄存器地址
    I2C_SendByte(data);                   // 发送数据
    I2C_Stop();                           // 发送停止信号
}

/**
 * @brief  读寄存器
 * @param  reg - 寄存器地址
 * @return 读到的数据
 */
static uint8_t MPU6050_ReadReg(uint8_t reg)
{
    uint8_t data;
    I2C_Start();                          // 发送起始信号
    I2C_SendByte(MPU6050_ADDR);           // 发送设备地址(写)
    I2C_SendByte(reg);                    // 发送寄存器地址
    I2C_Start();                          // 发送重复起始
    I2C_SendByte(MPU6050_ADDR + 1);       // 发送设备地址(读)
    data = I2C_ReadByte(0);               // 读取数据，发送NACK
    I2C_Stop();                            // 发送停止信号
    return data;
}

/**
 * @brief  连续读多个寄存器
 * @param  reg - 起始寄存器地址
 * @param  buf - 数据缓冲区
 * @param  len - 要读取的字节数
 */
static void MPU6050_ReadRegs(uint8_t reg, uint8_t *buf, uint8_t len)
{
    uint8_t i;
    I2C_Start();                          // 发送起始信号
    I2C_SendByte(MPU6050_ADDR);           // 发送设备地址(写)
    I2C_SendByte(reg);                    // 发送寄存器地址
    I2C_Start();                          // 发送重复起始
    I2C_SendByte(MPU6050_ADDR + 1);       // 发送设备地址(读)
    for (i = 0; i < len; i++)            // 连续读取
    {
        // 最后一个字节发送NACK，其余发送ACK
        buf[i] = I2C_ReadByte((i < len - 1) ? 1 : 0);
    }
    I2C_Stop();                            // 发送停止信号
}

/* ============================================================================
 * 函数名: MPU6050_I2C_Init
 * 功能:   初始化I2C GPIO
 * ============================================================================ */
static void MPU6050_I2C_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    /* 使能GPIOB时钟 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    /* 配置SCL和SDA为开漏输出 */
    // 开漏输出: 不能输出高电平，需要外接上拉电阻
    // 优点: 可以实现线与逻辑，方便实现I2C的时钟同步和仲裁
    GPIO_InitStructure.GPIO_Pin   = MPU6050_SCL_PIN | MPU6050_SDA_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_OD;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MPU6050_I2C_PORT, &GPIO_InitStructure);

    /* 空闲状态: SCL和SDA都为高 */
    I2C_SCL_H();
    I2C_SDA_H();
}

/* ============================================================================
 * 函数名: MPU6050_Init
 * 功能:   初始化MPU6050(含零偏校准)
 * ============================================================================ */
void MPU6050_Init(void)
{
    uint16_t i;
    int32_t sum = 0;                      // 累加和(用于计算平均值)
    uint8_t buf[2];                       // 数据缓冲区

    /* 初始化I2C GPIO */
    MPU6050_I2C_Init();
    Delay_ms(50);                         // 等待模块上电稳定

    /* 复位MPU6050 */
    MPU6050_WriteReg(MPU6050_REG_PWR_MGMT1, 0x80); // 写0x80=复位
    Delay_ms(100);                        // 等待复位完成

    /* 唤醒: 使用X轴陀螺仪作为时钟源 */
    MPU6050_WriteReg(MPU6050_REG_PWR_MGMT1, 0x01); // 0x01=正常模式

    /* 配置采样率: 125Hz */
    // 采样率 = 1kHz / (7+1) = 125Hz
    MPU6050_WriteReg(MPU6050_REG_SMPLRT_DIV, 0x07);

    /* 配置低通滤波器: 带宽98Hz */
    MPU6050_WriteReg(MPU6050_REG_CONFIG, 0x02);

    /* 配置陀螺仪量程: ±500°/s */
    // 灵敏度 = 65.5 LSB/(°/s)
    MPU6050_WriteReg(MPU6050_REG_GYRO_CONFIG, 0x08); // 0x08=±500°/s

    /* 配置加速度计量程: ±4g */
    MPU6050_WriteReg(MPU6050_REG_ACCEL_CONFIG, 0x08); // 0x08=±4g

    /* 零偏校准: 静止时采样200次取平均 */
    Delay_ms(200);                         // 等待传感器稳定
    sum = 0;
    for (i = 0; i < 200; i++)             // 采样200次
    {
        MPU6050_ReadRegs(MPU6050_REG_GYRO_ZOUT_H, buf, 2); // 读Z轴高字节
        sum += (int16_t)((buf[0] << 8) | buf[1]); // 合并高低字节
        Delay_ms(5);                      // 采样间隔5ms
    }
    s_gyro_z_offset = (int16_t)(sum / 200); // 计算零偏(平均值)
}

/* ============================================================================
 * 函数名: MPU6050_Check
 * 功能:   检测MPU6050是否在线
 * 返回:   1=在线, 0=离线
 * ============================================================================ */
uint8_t MPU6050_Check(void)
{
    // WHO_AM_I寄存器应该返回0x68
    return (MPU6050_ReadReg(MPU6050_REG_WHO_AM_I) == 0x68) ? 1 : 0;
}

/* ============================================================================
 * 函数名: MPU6050_ReadData
 * 功能:   读取完整传感器数据
 * 参数:   data - 数据结构指针
 * ============================================================================ */
void MPU6050_ReadData(MPU6050_Data_t *data)
{
    uint8_t buf[6];
    // 读取加速度计X和Y轴
    MPU6050_ReadRegs(MPU6050_REG_ACCEL_XOUT_H, buf, 6);
    data->accel_x = (int16_t)((buf[0] << 8) | buf[1]); // X轴加速度
    data->accel_y = (int16_t)((buf[2] << 8) | buf[3]); // Y轴加速度
    // 读取陀螺仪Z轴(已去零偏)
    MPU6050_ReadRegs(MPU6050_REG_GYRO_ZOUT_H, buf, 2);
    data->gyro_z = (int16_t)((buf[0] << 8) | buf[1]) - s_gyro_z_offset;
}

/* ============================================================================
 * 函数名: MPU6050_GetGyroZ
 * 功能:   读取Z轴角速度(原始值，已去零偏)
 * 返回:   角速度原始值(int16)
 * ============================================================================ */
int16_t MPU6050_GetGyroZ(void)
{
    uint8_t buf[2];
    MPU6050_ReadRegs(MPU6050_REG_GYRO_ZOUT_H, buf, 2);
    // 高字节<<8 | 低字节，减去零偏
    return (int16_t)((buf[0] << 8) | buf[1]) - s_gyro_z_offset;
}

/* ============================================================================
 * 函数名: MPU6050_GetGyroZ_Deg
 * 功能:   读取Z轴角速度(deg/s)
 * 返回:   角速度，单位°/s
 * ============================================================================ */
float MPU6050_GetGyroZ_Deg(void)
{
    // ±500°/s量程，灵敏度65.5 LSB/(°/s)
    return (float)MPU6050_GetGyroZ() / 65.5f;
}
