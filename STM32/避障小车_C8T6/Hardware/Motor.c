/**
 ******************************************************************************
 * @file    Motor.c
 * @brief   TB6612FNG双路电机驱动 - 实现文件
 *
 * ============================================================================
 *                          TB6612FNG vs L298N 区别
 * ============================================================================
 *
 *   1. TB6612多了一个STBY(待机)引脚:
 *      - STBY=高电平 → 电机正常工作
 *      - STBY=低电平 → 两个电机都禁用(省电模式)
 *      - L298N没有这个引脚，上电就能用
 *
 *   2. 方向控制逻辑不同:
 *      L298N:  IN1=0,IN2=0 → 刹车
 *      TB6612: AIN1=0,AIN2=0 → 滑行(自由转动)
 *      TB6612: AIN1=1,AIN2=1 → 刹车(短路制动)
 *
 * ============================================================================
 *                          TB6612方向控制真值表
 * ============================================================================
 *
 *   AIN1  AIN2  PWMA    模式          说明
 *   ────  ────  ────    ────          ────
 *    0     0     X      滑行(Coast)   电机自由转动，不施加力
 *    1     0    HIGH    正转(CW)      电流从AO1→AO2
 *    0     1    HIGH    反转(CCW)     电流从AO2→AO1
 *    1     1     X      刹车(Brake)   电机短路制动，快速停转
 *
 *   注意: STBY必须为高电平，上述控制才有效！
 *
 * ============================================================================
 *                          PWM调速原理
 * ============================================================================
 *
 *   PWM = Pulse Width Modulation (脉宽调制)
 *
 *   占空比 = 高电平时间 / 周期时间
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │  占空比10%  │  ▔▔▔▁▁▁▁▁▁  │  电机低速旋转                │
 *   │  占空比50%  │  ▔▔▔▔▔▁▁▁▁▁  │  电机中速旋转                │
 *   │  占空比90%  │  ▔▔▔▔▔▔▔▔▔▁  │  电机高速旋转                │
 *   │  占空比0%   │  ▁▁▁▁▁▁▁▁▁▁  │  电机停止                    │
 *   └─────────────────────────────────────────────────────────────┘
 *
 *   TIM2配置:
 *     预分频: 72-1 → 1MHz (1微秒计数一次)
 *     重装载: 999 → 1kHz PWM周期 (1毫秒)
 *     占空比范围: 0~999
 *
 * ============================================================================
 *                          硬件接线
 * ============================================================================
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │  STM32F103C8T6              TB6612FNG电机驱动               │
 *   ├─────────────────────────────────────────────────────────────┤
 *   │  PA0 (TIM2_CH1, PWM)    ───►   PWMA  (左电机PWM使能)       │
 *   │  PA1 (GPIO输出)         ───►   AIN1  (左电机方向A)         │
 *   │  PA2 (GPIO输出)         ───►   AIN2  (左电机方向B)         │
 *   │  PA3 (TIM2_CH4, PWM)    ───►   PWMB  (右电机PWM使能)       │
 *   │  PA4 (GPIO输出)         ───►   BIN1  (右电机方向A)         │
 *   │  PA5 (GPIO输出)         ───►   BIN2  (右电机方向B)         │
 *   │  PA6 (GPIO输出)         ───►   STBY  (待机控制)            │
 *   ├─────────────────────────────────────────────────────────────┤
 *   │  TB6612FNG                                            电机  │
 *   │  AO1  ──────────────────────────────────────────►  左电机  │
 *   │  AO2  ──────────────────────────────────────────►  左电机  │
 *   │  BO1  ──────────────────────────────────────────►  右电机  │
 *   │  BO2  ──────────────────────────────────────────►  右电机  │
 *   └─────────────────────────────────────────────────────────────┘
 *
 *   电源接线:
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │  TB6612 VM  ───► 电池正极(6~12V，电机供电)                │
 *   │  TB6612 VCC ───► 3.3V(逻辑供电)                           │
 *   │  TB6612 GND ───► GND(与STM32共地！)                       │
 *   └─────────────────────────────────────────────────────────────┘
 *
 ******************************************************************************
 */

#include "Motor.h"                      // 电机驱动头文件
#include "stm32f10x_rcc.h"              // 时钟配置
#include "stm32f10x_gpio.h"             // GPIO配置
#include "stm32f10x_tim.h"              // 定时器配置

/* ============================================================================
 *                              内部函数
 * ============================================================================ */

/**
 * @brief  限幅函数
 * @param  val   - 输入值
 * @param  min_v - 最小值
 * @param  max_v - 最大值
 * @return 限幅后的值
 * @note   确保返回值在[min_v, max_v]范围内
 */
static int16_t Clamp(int16_t val, int16_t min_v, int16_t max_v)
{
    if (val > max_v) return max_v;     // 超过上限，返回上限
    if (val < min_v) return min_v;     // 低于下限，返回下限
    return val;                         // 在范围内，原值返回
}

/* ============================================================================
 * 函数名: Motor_Init
 * 功能:   初始化电机驱动GPIO、TIM2 PWM、STBY引脚
 * 说明:   TB6612比L298N多一个STBY引脚，必须拉高才能驱动电机
 * ============================================================================ */
void Motor_Init(void)
{
    GPIO_InitTypeDef        GPIO_InitStructure;      // GPIO初始化结构体
    TIM_TimeBaseInitTypeDef TIM_TimeBaseStructure;  // 定时器初始化结构体
    TIM_OCInitTypeDef       TIM_OCInitStructure;    // PWM输出初始化结构体

    /* 使能时钟 */
    // GPIOA: PA0~PA6用于电机控制
    // TIM2:  PA0(CH1)和PA3(CH4)用作PWM输出
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);   // 使能GPIOA时钟
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);     // 使能TIM2时钟

    /* 配置方向控制引脚: AIN1/AIN2/BIN1/BIN2 → 推挽输出 */
    GPIO_InitStructure.GPIO_Pin   = MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN |
                                    MOTOR_B_IN1_PIN | MOTOR_B_IN2_PIN;
                                          // PA1, PA2, PA4, PA5
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
                                          // 推挽输出(可以输出高低电平)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
                                          // 50MHz翻转速度
    GPIO_Init(MOTOR_GPIO_PORT, &GPIO_InitStructure);

    /* 配置STBY引脚: PA6 → 推挽输出 */
    // TB6612特有: STBY=高电平时电机才能工作，低电平时进入待机(省电)
    GPIO_InitStructure.GPIO_Pin   = MOTOR_STBY_PIN;
                                          // PA6
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
                                          // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_GPIO_PORT, &GPIO_InitStructure);

    /* 初始状态: 所有方向引脚置低 + STBY拉高(使能) */
    // 方向引脚全低 → 两个电机滑行(不转)
    // STBY拉高 → TB6612退出待机模式，可以接受控制
    GPIO_ResetBits(MOTOR_GPIO_PORT,
                   MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN |
                   MOTOR_B_IN1_PIN | MOTOR_B_IN2_PIN);
    GPIO_SetBits(MOTOR_GPIO_PORT, MOTOR_STBY_PIN);
                                          // STBY=1，使能TB6612

    /* 配置PWM输出引脚: PWMA/PWMB → 复用推挽输出 */
    // 这些引脚需要输出PWM波形，由TIM2控制
    GPIO_InitStructure.GPIO_Pin   = MOTOR_A_PWM_PIN | MOTOR_B_PWM_PIN;
                                          // PA0, PA3
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AF_PP;
                                          // 复用推挽输出(由外设TIM2控制)
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(MOTOR_GPIO_PORT, &GPIO_InitStructure);

    /* 配置TIM2时基: PWM频率 = 72MHz / 72 / 1000 = 1kHz */
    TIM_TimeBaseStructure.TIM_Period        = MOTOR_TIM_PERIOD;    // 999 (重装载值)
    TIM_TimeBaseStructure.TIM_Prescaler     = MOTOR_TIM_PRESCALER; // 71 (预分频)
    TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;        // 不分频
    TIM_TimeBaseStructure.TIM_CounterMode   = TIM_CounterMode_Up;  // 向上计数
    TIM_TimeBaseStructure.TIM_RepetitionCounter = 0;               // 高级定时器用
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure);

    /* 配置PWM输出通道 */
    TIM_OCInitStructure.TIM_OCMode      = TIM_OCMode_PWM1;
                                          // PWM模式1: CNT<CCR时输出高
    TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable;  // 使能输出
    TIM_OCInitStructure.TIM_Pulse       = SPEED_STOP;              // 初始占空比0
    TIM_OCInitStructure.TIM_OCPolarity  = TIM_OCPolarity_High;    // 高电平有效

    /* 配置CH1 → PA0 → PWMA(左电机) */
    TIM_OC1Init(TIM2, &TIM_OCInitStructure);      // 初始化通道1
    TIM_OC1PreloadConfig(TIM2, TIM_OCPreload_Enable); // 使能预装载

    /* 配置CH4 → PA3 → PWMB(右电机) */
    TIM_OC4Init(TIM2, &TIM_OCInitStructure);      // 初始化通道4
    TIM_OC4PreloadConfig(TIM2, TIM_OCPreload_Enable); // 使能预装载

    /* 使能TIM2自动重装载 */
    TIM_ARRPreloadConfig(TIM2, ENABLE);

    /* 启动TIM2 */
    TIM_Cmd(TIM2, ENABLE);
}

/* ============================================================================
 * 函数名: Motor_Left_Set
 * 功能:   设置左电机速度和方向
 * 参数:   speed - 速度值
 *         >0: 正转(前进)，范围 0~999
 *         <0: 反转(后退)，范围 -999~0
 *         =0: 停止(滑行)
 * 说明:   TB6612控制逻辑(与L298N的区别):
 *           L298N停止: IN1=0, IN2=0 → 刹车
 *           TB6612停止: AIN1=0, AIN2=0 → 滑行(自由转动)
 *           TB6612刹车: AIN1=1, AIN2=1 → 短路制动
 *         这里speed=0用滑行模式，因为PWM=0时电机本来就不转
 * ============================================================================ */
void Motor_Left_Set(int16_t speed)
{
    /* 限幅保护 */
    speed = Clamp(speed, -MOTOR_TIM_PERIOD, MOTOR_TIM_PERIOD);

    if (speed > 0)
    {
        /* 正转: AIN1=1, AIN2=0 (TB6612正转模式) */
        // 电流从AO1流出，经电机流入AO2
        GPIO_SetBits  (MOTOR_GPIO_PORT, MOTOR_A_IN1_PIN);  // AIN1高
        GPIO_ResetBits(MOTOR_GPIO_PORT, MOTOR_A_IN2_PIN);  // AIN2低
        TIM_SetCompare1(TIM2, (uint16_t)speed);             // PWM占空比=速度
    }
    else if (speed < 0)
    {
        /* 反转: AIN1=0, AIN2=1 (TB6612反转模式) */
        // 电流从AO2流出，经电机流入AO1(方向相反)
        GPIO_ResetBits(MOTOR_GPIO_PORT, MOTOR_A_IN1_PIN);  // AIN1低
        GPIO_SetBits  (MOTOR_GPIO_PORT, MOTOR_A_IN2_PIN);  // AIN2高
        TIM_SetCompare1(TIM2, (uint16_t)(-speed));          // PWM占空比=|速度|
    }
    else
    {
        /* 停止: AIN1=0, AIN2=0 → 滑行(Coast) */
        // 电机自由转动，不施加力
        // PWM=0时电机本身也不驱动，所以用滑行模式即可
        GPIO_ResetBits(MOTOR_GPIO_PORT, MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN);
        TIM_SetCompare1(TIM2, 0);                           // PWM=0
    }
}

/* ============================================================================
 * 函数名: Motor_Right_Set
 * 功能:   设置右电机速度和方向
 * 参数:   speed - 速度值(同左电机)
 * ============================================================================ */
void Motor_Right_Set(int16_t speed)
{
    /* 限幅保护 */
    speed = Clamp(speed, -MOTOR_TIM_PERIOD, MOTOR_TIM_PERIOD);

    if (speed > 0)
    {
        /* 正转: BIN1=0, BIN2=1 (右电机接线反了，此处逻辑与左电机相反) */
        GPIO_ResetBits(MOTOR_GPIO_PORT, MOTOR_B_IN1_PIN);  // BIN1=0
        GPIO_SetBits  (MOTOR_GPIO_PORT, MOTOR_B_IN2_PIN);  // BIN2=1
        TIM_SetCompare4(TIM2, (uint16_t)speed);
    }
    else if (speed < 0)
    {
        /* 反转: BIN1=1, BIN2=0 (右电机接线反了，此处逻辑与左电机相反) */
        GPIO_SetBits  (MOTOR_GPIO_PORT, MOTOR_B_IN1_PIN);  // BIN1=1
        GPIO_ResetBits(MOTOR_GPIO_PORT, MOTOR_B_IN2_PIN);  // BIN2=0
        TIM_SetCompare4(TIM2, (uint16_t)(-speed));
    }
    else
    {
        /* 停止: BIN1=0, BIN2=0 → 滑行(Coast) */
        GPIO_ResetBits(MOTOR_GPIO_PORT, MOTOR_B_IN1_PIN | MOTOR_B_IN2_PIN);
        TIM_SetCompare4(TIM2, 0);
    }
}

/* ============================================================================
 *                          组合动作函数
 * ============================================================================ */

/**
 * @brief  直线前进
 * @param  speed - 速度(0~999)
 */
void Car_Forward(uint16_t speed)
{
    if (speed > MOTOR_TIM_PERIOD) speed = MOTOR_TIM_PERIOD; // 限幅
    Motor_Left_Set ((int16_t)speed);    // 左轮正转
    Motor_Right_Set((int16_t)speed);    // 右轮正转
}

/**
 * @brief  直线后退
 * @param  speed - 速度(0~999)
 */
void Car_Backward(uint16_t speed)
{
    if (speed > MOTOR_TIM_PERIOD) speed = MOTOR_TIM_PERIOD; // 限幅
    Motor_Left_Set (-(int16_t)speed);   // 左轮反转
    Motor_Right_Set(-(int16_t)speed);   // 右轮反转
}

/**
 * @brief  差速左转
 * @param  base_speed  - 基础速度
 * @param  turn_offset - 转向偏移量(减慢左轮)
 */
void Car_TurnLeft(uint16_t base_speed, uint16_t turn_offset)
{
    int16_t left  = (int16_t)base_speed - (int16_t)turn_offset; // 左轮减速
    int16_t right = (int16_t)base_speed;                         // 右轮不变
    Motor_Left_Set (left);
    Motor_Right_Set(right);
}

/**
 * @brief  差速右转
 * @param  base_speed  - 基础速度
 * @param  turn_offset - 转向偏移量(减慢右轮)
 */
void Car_TurnRight(uint16_t base_speed, uint16_t turn_offset)
{
    int16_t left  = (int16_t)base_speed;                         // 左轮不变
    int16_t right = (int16_t)base_speed - (int16_t)turn_offset;  // 右轮减速
    Motor_Left_Set (left);
    Motor_Right_Set(right);
}

/**
 * @brief  停车(双轮滑行)
 */
void Car_Stop(void)
{
    Motor_Left_Set (0);                 // 左轮停止
    Motor_Right_Set(0);                 // 右轮停止
}

/**
 * @brief  双轮短路制动(TB6612 Brake模式)
 * @note   IN1=1, IN2=1 → 电机短路, 快速停转
 *         与Motor_Set(0)的区别:
 *           Motor_Set(0)  → IN1=0, IN2=0 → 滑行(Coast), 电机自由转动
 *           Motor_Brake() → IN1=1, IN2=1 → 制动(Brake), 电机被短路强制停止
 *         用途: 方向切换前调用, 确保电机完全停止后再反转
 */
void Motor_Brake(void)
{
    /* 左电机制动: AIN1=1, AIN2=1 → 短路制动 */
    GPIO_SetBits(MOTOR_GPIO_PORT, MOTOR_A_IN1_PIN | MOTOR_A_IN2_PIN);
    TIM_SetCompare1(TIM2, 0);

    /* 右电机制动: BIN1=1, BIN2=1 → 短路制动 */
    GPIO_SetBits(MOTOR_GPIO_PORT, MOTOR_B_IN1_PIN | MOTOR_B_IN2_PIN);
    TIM_SetCompare4(TIM2, 0);
}
