/**
 ******************************************************************************
 * @file    HCSR04.c
 * @brief   HC-SR04超声波测距模块驱动(非阻塞EXTI版)
 *
 * ============================================================================
 *                          工作原理
 * ============================================================================
 *
 *   旧方式(已弃用): 阻塞等待Echo信号(20~60ms), 期间CPU无法执行其他任务
 *   本驱动:        使用EXTI中断捕获Echo上升沿和下降沿, 零阻塞
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │                非阻塞测距流程                               │
 *   │                                                             │
 *   │  主循环                          EXTI中断                   │
 *   │    │                                │                      │
 *   │    ├── sonar_start()                │                      │
 *   │    │   └─ 发送Trig脉冲(15μs)        │                      │
 *   │    │                                │                      │
 *   │    │ (继续执行循迹/显示等)           │                      │
 *   │    │                                │                      │
 *   │    │                    Echo上升沿 ─┤                      │
 *   │    │                    记录CNT值   │                      │
 *   │    │                                │                      │
 *   │    │                    Echo下降沿 ─┤                      │
 *   │    │                    计算距离    │                      │
 *   │    │                    更新结果    │                      │
 *   │    │                                │                      │
 *   │    ├── sonar_get_mm()               │                      │
 *   │    │   └─ 读取最新距离(非阻塞)      │                      │
 *   └────┴────────────────────────────────┘                      │
 *   └─────────────────────────────────────────────────────────────┘
 *
 * ============================================================================
 *                          距离计算
 * ============================================================================
 *
 *   TIM3自由运行, 1μs计数一次
 *   Echo高电平时间 = (end_cnt - start_cnt) μs (uint16_t自动处理溢出)
 *   声速 = 346m/s = 0.346mm/μs
 *   距离 = 声速 × 时间 / 2 (往返)
 *        = 0.346 × Δt / 2
 *        = Δt × 0.173 mm
 *        = Δt × 173 / 1000 mm
 *
 *   例: Δt = 5780μs → 距离 = 5780 × 173 / 1000 = 999mm ≈ 100cm
 *
 * ============================================================================
 *                          距离公式修正说明
 * ============================================================================
 *
 *   旧版公式(有BUG):
 *     distance_mm = (time_end - time_start) * 17 / 100
 *     其中 time 是10μs单位的计数值
 *     实际计算 = ticks × 10μs × 0.173mm/μs = ticks × 1.73mm
 *     但公式给出 = ticks × 0.17mm  ← 差了约10倍！
 *
 *   新版公式(正确):
 *     distance_mm = delta * 173 / 1000
 *     其中 delta 是1μs单位的计数值(TIM3->CNT)
 *     实际计算 = delta × 0.173mm  ← 正确！
 *
 * ============================================================================
 *                          硬件接线
 * ============================================================================
 *
 *   ┌─────────────────────────────────────────────────────────────┐
 *   │  STM32F103C8T6              HC-SR04                        │
 *   ├─────────────────────────────────────────────────────────────┤
 *   │  PB5 (GPIO输出)        ───►   Trig (触发信号)              │
 *   │  PB4 (EXTI4双边沿)     ◄───   Echo (回波信号)              │
 *   │  5V                    ───►   VCC (需要5V供电!)             │
 *   │  GND                   ───►   GND                          │
 *   └─────────────────────────────────────────────────────────────┘
 *
 ******************************************************************************
 */

#include "HCSR04.h"                     // 超声波头文件
#include "Delay.h"                      // 延时函数
#include "stm32f10x_rcc.h"              // 时钟配置
#include "stm32f10x_gpio.h"             // GPIO配置
#include "stm32f10x_exti.h"             // EXTI外部中断
#include "misc.h"                       // NVIC配置

/* ============================================================================
 *                              引脚定义
 * ============================================================================ */

#define HCSR04_TRIG_PORT  GPIOB        // Trig引脚端口: GPIOB
#define HCSR04_TRIG_PIN   GPIO_Pin_5   // Trig引脚: PB5 (输出)
#define HCSR04_ECHO_PORT  GPIOB        // Echo引脚端口: GPIOB
#define HCSR04_ECHO_PIN   GPIO_Pin_4   // Echo引脚: PB4 (EXTI4输入)

/* ============================================================================
 *                              测距状态机
 * ============================================================================ */

typedef enum {
    SONAR_IDLE = 0,      // 空闲，可以启动新测量
    SONAR_WAIT_HIGH,     // 已发Trig，等待Echo上升沿
    SONAR_WAIT_LOW,      // Echo已高，等待下降沿
    SONAR_DONE           // 测量完成，结果可用
} SonarState_t;

static volatile SonarState_t g_sonar_state  = SONAR_IDLE;  // 测距状态
static volatile int16_t  g_sonar_dist_mm    = -1;          // 最新距离(mm), -1=无效
static volatile uint16_t g_sonar_start_cnt = 0;           // Echo上升沿时的TIM3计数值
static volatile uint32_t g_sonar_wait_cnt  = 0;           // 等待超时计数器(每5ms加1)

/* ============================================================================
 * 函数名: HC_SR04_Init
 * 功能:   初始化超声波模块(GPIO + EXTI)
 * 说明:   PB5→Trig(推挽输出), PB4→Echo(下拉输入+EXTI4双边沿中断)
 * ============================================================================ */
void HC_SR04_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;   // GPIO初始化结构体
    EXTI_InitTypeDef  EXTI_InitStructure;   // EXTI初始化结构体
    NVIC_InitTypeDef  NVIC_InitStructure;   // NVIC初始化结构体

    /* 使能时钟: GPIOB + AFIO(EXTI需要) */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);

    /* 配置Trig引脚: PB5 推挽输出 */
    GPIO_InitStructure.GPIO_Pin   = HCSR04_TRIG_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;   // 推挽输出
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(HCSR04_TRIG_PORT, &GPIO_InitStructure);

    /* 配置Echo引脚: PB4 下拉输入 */
    GPIO_InitStructure.GPIO_Pin   = HCSR04_ECHO_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_IPD;      // 下拉输入
    GPIO_Init(HCSR04_ECHO_PORT, &GPIO_InitStructure);

    /* 初始状态: Trig低电平 */
    GPIO_ResetBits(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN);

    /* 连接PB4到EXTI Line 4 */
    // AFIO映射: 告诉EXTI模块, Line4的信号来自GPIOB的Pin4
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);

    /* 配置EXTI Line 4: 双边沿触发(上升沿+下降沿) */
    EXTI_InitStructure.EXTI_Line    = EXTI_Line4;              // Line 4
    EXTI_InitStructure.EXTI_Mode    = EXTI_Mode_Interrupt;     // 中断模式
    EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Rising_Falling; // 双边沿
    EXTI_InitStructure.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStructure);

    /* 清除可能残留的EXTI中断标志 */
    EXTI_ClearITPendingBit(EXTI_Line4);

    /* 配置NVIC: EXTI4中断 */
    NVIC_InitStructure.NVIC_IRQChannel                   = EXTI4_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;  // 抢占优先级2
    NVIC_InitStructure.NVIC_IRQChannelSubPriority        = 1;  // 子优先级1
    NVIC_InitStructure.NVIC_IRQChannelCmd                = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
}

/* ============================================================================
 * 函数名: sonar_start
 * 功能:   启动一次距离测量(非阻塞)
 * 说明:   发送Trig脉冲后立即返回，测量结果由EXTI中断自动更新
 *         调用后约15μs返回，不阻塞主控制循环
 * ============================================================================ */
void sonar_start(void)
{
    /* 只在空闲/完成状态下才能启动新测量 */
    if (g_sonar_state == SONAR_WAIT_HIGH || g_sonar_state == SONAR_WAIT_LOW)
    {
        return;  // 正在测量中，不能打断
    }

    /* 清除可能残留的EXTI中断标志，防止误触发 */
    EXTI_ClearITPendingBit(EXTI_Line4);

    /* 设置状态: 等待Echo上升沿 */
    g_sonar_state   = SONAR_WAIT_HIGH;
    g_sonar_wait_cnt = 0;

    /* 发送Trig触发脉冲: ≥10μs高电平 */
    GPIO_SetBits  (HCSR04_TRIG_PORT, HCSR04_TRIG_PIN);   // Trig=1
    Delay_us(15);                                         // 延时15μs(确保≥10μs)
    GPIO_ResetBits(HCSR04_TRIG_PORT, HCSR04_TRIG_PIN);   // Trig=0
}

/* ============================================================================
 * 函数名: sonar_get_mm
 * 功能:   获取最近一次测距结果
 * 返回:   距离(mm), >0=有效, -1=无效/超出范围/未测量
 * 说明:   非阻塞，直接读取EXTI中断更新后的结果
 * ============================================================================ */
int16_t sonar_get_mm(void)
{
    return g_sonar_dist_mm;
}

/* ============================================================================
 * 函数名: sonar_timeout_check
 * 功能:   超时保护，需在主循环中每5ms调用一次
 * 说明:   如果测量状态卡住超过500ms(传感器断线/异常)，自动重置
 * ============================================================================ */
void sonar_timeout_check(void)
{
    if (g_sonar_state == SONAR_WAIT_HIGH || g_sonar_state == SONAR_WAIT_LOW)
    {
        g_sonar_wait_cnt++;
        if (g_sonar_wait_cnt > 100)   // 100 × 5ms = 500ms超时
        {
            g_sonar_state   = SONAR_IDLE;
            g_sonar_dist_mm = -1;      // 标记无效
        }
    }
}

/* ============================================================================
 * 函数名: HCSR04_EXTI_Handler
 * 功能:   EXTI中断处理，由stm32f10x_it.c中的EXTI4_IRQHandler调用
 * 说明:   Echo上升沿 → 记录TIM3计数值(开始计时)
 *         Echo下降沿 → 计算距离(结束计时)
 * ============================================================================ */
void HCSR04_EXTI_Handler(void)
{
    if (EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        EXTI_ClearITPendingBit(EXTI_Line4);  // 必须清除中断标志

        /* 检查Echo引脚当前电平，判断是上升沿还是下降沿 */
        if (GPIO_ReadInputDataBit(HCSR04_ECHO_PORT, HCSR04_ECHO_PIN) == Bit_SET
            && g_sonar_state == SONAR_WAIT_HIGH)
        {
            /* ---- Echo上升沿: 超声波开始返回 ---- */
            g_sonar_start_cnt = TIM3->CNT;     // 记录当前TIM3计数值(1μs单位)
            g_sonar_state = SONAR_WAIT_LOW;    // 切换到等待下降沿
        }
        else if (GPIO_ReadInputDataBit(HCSR04_ECHO_PORT, HCSR04_ECHO_PIN) == Bit_RESET
                 && g_sonar_state == SONAR_WAIT_LOW)
        {
            /* ---- Echo下降沿: 超声波接收完成 ---- */
            uint16_t end_cnt = TIM3->CNT;      // 读取结束计数值
            uint16_t delta = end_cnt - g_sonar_start_cnt; // uint16_t自动处理溢出

            /* 距离计算: Δt(μs) × 0.173 mm/μs = Δt × 173 / 1000 mm */
            int32_t dist = (int32_t)delta * 173 / 1000;

            /* 范围检查: 20mm(2cm) ~ 4000mm(400cm) */
            if (dist >= 20 && dist <= 4000)
            {
                g_sonar_dist_mm = (int16_t)dist;
            }
            else
            {
                g_sonar_dist_mm = -1;   // 超出范围(太近/太远)，标记无效
            }

            g_sonar_state = SONAR_DONE;  // 测量完成
        }
        /* 其他情况: 意外的中断(噪声/干扰)，忽略 */
    }
}
