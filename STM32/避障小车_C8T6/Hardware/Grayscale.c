/**
 ******************************************************************************
 * @file    Grayscale.c
 * @brief   8路灰度循迹传感器驱动 (CD4051多路复用 + ADC)
 *
 * 引脚:
 *   PB9→S0, PB10→S1, PB11→S2 (通道选择, GPIO输出)
 *   PA7→AS (模拟信号, ADC12_IN7, 飞线)
 *
 * 工作原理:
 *   1. 设置S0/S1/S2选通通道n (0~7)
 *   2. 等待CD4051切换稳定 (~15us)
 *   3. ADC单次转换读取AS电压
 *   4. 8路轮询完毕 → 每路与校准阈值比较 → 二值化
 *   5. 加权求和 + 归一化 → 误差 -15~+15
 *
 * 自动校准:
 *   启动时晃动小车使传感器覆盖黑线和白地, 记录每路max/min,
 *   阈值 = (max + min) / 2. 高ADC值=黑线(吸光), 低ADC值=白地(反光).
 ******************************************************************************
 */

#include "Grayscale.h"
#include "stm32f10x_rcc.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_adc.h"

/* ============================================================================
 *                              静态变量
 * ============================================================================ */

/* 全局: 最近一次8路原始ADC值 (供OLED调试) */
uint16_t g_raw_adc[GS_NUM_CHANNELS];

/* 动态阈值: 每帧根据当前max/min自动计算, 自适应光线 */
static uint16_t g_gs_dynamic_th = 2000;

/* 上次有效误差 (全白脱线时使用) */
static int8_t   g_gs_last_error = 0;

/* 8路权重 (加大跨度, 修正更灵敏) */
static const int8_t g_gs_weight[GS_NUM_CHANNELS] = {
    -10, -7, -4, -1,    /* CH0~CH3: 左半 */
    +1,  +4, +7, +10    /* CH4~CH7: 右半 */
};

/* ============================================================================
 * 函数: GS_SelectChannel
 * 功能: 通过S0/S1/S2选通指定通道
 * ============================================================================ */
static void GS_SelectChannel(uint8_t ch)
{
    if (ch & 0x01) GPIO_SetBits  (GS_S0_PORT, GS_S0_PIN);
    else           GPIO_ResetBits(GS_S0_PORT, GS_S0_PIN);

    if (ch & 0x02) GPIO_SetBits  (GS_S1_PORT, GS_S1_PIN);
    else           GPIO_ResetBits(GS_S1_PORT, GS_S1_PIN);

    if (ch & 0x04) GPIO_SetBits  (GS_S2_PORT, GS_S2_PIN);
    else           GPIO_ResetBits(GS_S2_PORT, GS_S2_PIN);
}

/* ============================================================================
 * 函数: GS_Init
 * 功能: 初始化GPIO(S0/S1/S2输出) + ADC1(PA7模拟输入)
 * ============================================================================ */
void GS_Init(void)
{
    GPIO_InitTypeDef  GPIO_InitStructure;
    ADC_InitTypeDef   ADC_InitStructure;

    /* ---- 1. 使能时钟 ---- */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | GS_ADC_GPIO_RCC | GS_ADC_RCC, ENABLE);

    /* ---- 2. S0/S1/S2: 推挽输出 ---- */
    GPIO_InitStructure.GPIO_Pin   = GS_CHANNEL_MASK;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GS_S0_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(GS_S0_PORT, GS_CHANNEL_MASK);  /* 默认选CH0 */

    /* ---- 3. PA7: 模拟输入 ---- */
    GPIO_InitStructure.GPIO_Pin   = GS_ADC_PIN;
    GPIO_InitStructure.GPIO_Mode  = GPIO_Mode_AIN;
    GPIO_Init(GS_ADC_PORT, &GPIO_InitStructure);

    /* ---- 4. ADC1配置 ---- */
    RCC_ADCCLKConfig(RCC_PCLK2_Div6);             /* 72M/6 = 12MHz */

    ADC_DeInit(ADC1);
    ADC_InitStructure.ADC_Mode               = ADC_Mode_Independent;
    ADC_InitStructure.ADC_ScanConvMode       = DISABLE;
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE;
    ADC_InitStructure.ADC_ExternalTrigConv   = ADC_ExternalTrigConv_None;
    ADC_InitStructure.ADC_DataAlign          = ADC_DataAlign_Right;
    ADC_InitStructure.ADC_NbrOfChannel       = 1;
    ADC_Init(ADC1, &ADC_InitStructure);

    ADC_RegularChannelConfig(ADC1, GS_ADC_CHANNEL, 1, ADC_SampleTime_55Cycles5);

    ADC_Cmd(ADC1, ENABLE);

    /* 复位校准 */
    ADC_ResetCalibration(ADC1);
    while (ADC_GetResetCalibrationStatus(ADC1))
        ;
    ADC_StartCalibration(ADC1);
    while (ADC_GetCalibrationStatus(ADC1))
        ;

}

/* ============================================================================
 * 函数: GS_ReadChannel
 * 功能: 选通通道, 等待稳定, 读取ADC值
 * ============================================================================ */
static uint16_t GS_ReadChannel(uint8_t ch)
{
    uint16_t i;

    GS_SelectChannel(ch);

    /* CD4051切换 + 模拟信号稳定: 约50us */
    for (i = 0; i < 3600; i++) __NOP();   /* 3600nop @72MHz ≈ 50us */

    /* 第一次转换: 丢弃, 让ADC从旧通道过渡到新通道 */
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    (void)ADC_GetConversionValue(ADC1);    /* 丢弃 */

    /* 第二次转换: 信号已稳定, 取此值 */
    ADC_ClearFlag(ADC1, ADC_FLAG_EOC);
    ADC_SoftwareStartConvCmd(ADC1, ENABLE);
    while (!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
    return ADC_GetConversionValue(ADC1);
}

/* ============================================================================
 * 函数: GS_Calibrate
 * 功能: 自动校准 — 持续采集每路max/min, 求中值做阈值
 * 调用: 启动时调用, duration_ms约2000ms
 * ============================================================================ */
void GS_Calibrate(uint32_t duration_ms)
{
    (void)duration_ms;  /* 已改为每帧自动阈值, 校准函数保留兼容 */
}

/* ============================================================================
 * 函数: GS_Read
 * 功能: 读取8路传感器, 每帧自动阈值二值化, 计算加权误差
 * ============================================================================ */
GS_Status_t GS_Read(void)
{
    GS_Status_t st;
    uint8_t ch, count;
    int16_t weighted;
    uint16_t frame_min = 4095, frame_max = 0, threshold;

    /* 1. 读取8路ADC, 同时找本帧max/min */
    for (ch = 0; ch < GS_NUM_CHANNELS; ch++)
    {
        st.raw[ch] = GS_ReadChannel(ch);
        g_raw_adc[ch] = st.raw[ch];
        if (st.raw[ch] < frame_min) frame_min = st.raw[ch];
        if (st.raw[ch] > frame_max) frame_max = st.raw[ch];
    }

    /* 2. 动态阈值: 本帧max/min中点, 自适应光线变化 */
    if ((frame_max - frame_min) > 200)  /* 黑白差异>0.16V, 有效 */
    {
        threshold = (frame_max + frame_min) / 2;
        g_gs_dynamic_th = threshold;    /* 保存供下次全黑/全白时使用 */
    }
    else
    {
        threshold = g_gs_dynamic_th;    /* 全黑或全白, 沿用上次有效阈值 */
    }

    /* 3. 二值化: 高于阈值=黑线=1 */
    for (ch = 0; ch < GS_NUM_CHANNELS; ch++)
        st.ch[ch] = (st.raw[ch] > threshold) ? 1 : 0;

    /* 4. 统计压线数量 */
    count = 0;
    for (ch = 0; ch < GS_NUM_CHANNELS; ch++)
        count += st.ch[ch];

    if (count == 0)
    {
        /* 全白: 脱线保护 */
        st.error = g_gs_last_error * 2;
        if (st.error >  10) st.error =  10;
        if (st.error < -10) st.error = -10;
    }
    else
    {
        /* 加权误差 = Σ(ch × weight) / count */
        weighted = 0;
        for (ch = 0; ch < GS_NUM_CHANNELS; ch++)
            weighted += (int16_t)st.ch[ch] * g_gs_weight[ch];

        st.error = (int8_t)(weighted / (int8_t)count);
        g_gs_last_error = st.error;
    }

    return st;
}

/* ============================================================================
 * 函数: GS_AllBlack
 * 功能: 8路全检测到黑线 (十字路口/斑马线/T型终点)
 * ============================================================================ */
uint8_t GS_AllBlack(GS_Status_t *s)
{
    uint8_t ch;
    for (ch = 0; ch < GS_NUM_CHANNELS; ch++)
        if (!s->ch[ch]) return 0;
    return 1;
}

/* ============================================================================
 * 函数: GS_AllWhite
 * 功能: 8路全白 (脱线)
 * ============================================================================ */
uint8_t GS_AllWhite(GS_Status_t *s)
{
    uint8_t ch;
    for (ch = 0; ch < GS_NUM_CHANNELS; ch++)
        if (s->ch[ch]) return 0;
    return 1;
}

/* ============================================================================
 * 函数: GS_IsTJunction
 * 功能: ≥6路检测到黑线 → 视为T型路口
 * ============================================================================ */
uint8_t GS_IsTJunction(GS_Status_t *s)
{
    uint8_t ch, cnt = 0;
    for (ch = 0; ch < GS_NUM_CHANNELS; ch++)
        if (s->ch[ch]) cnt++;
    return (cnt >= 6) ? 1 : 0;
}

/* ============================================================================
 * 函数: GS_GetThreshold
 * 功能: 获取某路校准阈值 (调试用)
 * ============================================================================ */
uint16_t GS_GetThreshold(uint8_t idx)
{
    (void)idx;
    return g_gs_dynamic_th;
}
