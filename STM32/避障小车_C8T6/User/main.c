/**
 ******************************************************************************
 * @file    main.c
 * @brief   STM32F103C8 自动避障循迹小车 — v8 (弧线绕障 + 回线增强版)
 *
 * 赛道: 起点(T型) ══════ 黑线 ══════ 终点(T型)
 *
 * 完整流程:
 *   出发 → 循迹前进
 *     ├─ 前方30cm内有障碍物 → 报警 → 左弧线绕障 → 右扫线找黑线 → 继续
 *     └─ 无障碍
 *   → 终点(≥6路黑) → 制动 → 等5s → 180°调头
 *   → 循迹返回 → 起点T型 → 停车 → 显示成绩
 *
 * 硬件:
 *   电机:  PA0~PA6   | 灰度: PB9~PB11(S0~S2) + PA7(AS,ADC) | 超声: PB5/PB4
 * 编码器: PA8/PA9左 PB6/PB7右(TIM1/TIM4正交编码)|蜂鸣器:PB3|OLED:PB12/PB13
 *
 * 绕障策略(弧线版, 恢复自v6):
 *   阶段1: 左弧线800ms — 左轮慢(250)右轮快(350), 向左前方画弧绕过水杯
 *   阶段2: 锁死右转 — 右轮锁240, 左轮450→500渐进加速, 向右扫寻黑线
 *   关键: 盲转600ms后才检测黑线防误触; 找到线后直行150ms平稳交PD
 *
 * 回线增强(v8):
 *   180°调头找到线后, IR_RETURNING前400ms使用增强PD
 ******************************************************************************
 */

#include "stm32f10x.h"
#include <stdlib.h>
#include "Motor.h"
#include "Grayscale.h"
#include "HCSR04.h"
#include "OLED.h"
#include "Timer.h"
#include "Delay.h"
#include "Encoder.h"

/* ============================================================================
 *                              PD/速度 参数
 * ============================================================================ */
#define BASE_SPEED          340
#define KD                   15        /* 微分系数, 抑制超调 */
#define SPEED_DROP_ERR        3        /* 开始降速的误差阈值 */
#define SPEED_MIN            150       /* 最低速度(大弯时) */
#define SHARP_TURN_ERR        5        /* 急弯误差阈值, 触发内轮反转 */
#define SHARP_SPEED          170       /* 急弯基础速度 */
#define INNER_REVERSE_SPD    150       /* 急弯时内轮反转速度 */
#define SPEED_MAX            999       /* PWM硬上限 */
#define CTRL_PERIOD_MS         5       /* 控制周期5ms = 200Hz */
#define OLED_PERIOD_MS        50       /* OLED刷新周期 */

/* ============================================================================
 *                         绕障 参数 (弧线绕障版)
 * ============================================================================ */
#define OBSTACLE_DIST        250       /* 检测距离(mm), 30cm内触发 */
#define OBS_BRAKE_SPEED      600       /* 绕障急刹速度 */
#define BRAKE_COUNT           20       /* 急刹100ms */
#define OBS_PAUSE_CYC         40       /* 制动后暂停200ms */

/* 绕障弧线: 阶段1左弧线(800ms) → 阶段2锁死右转(盲转1200ms后才检测黑线) */
#define ARC_LEFT_TIME_MS     800
#define ARC_LEFT_CYC       (ARC_LEFT_TIME_MS / CTRL_PERIOD_MS)
#define ARC_L_SPD          370       /* 阶段1 右轮(快) */
#define ARC_R_SLOW         250       /* 阶段1 左轮(慢) */
#define ARC_R_SCAN_R_SPD   240       /* 阶段2 右轮(锁死正向,绝不左转) */
#define ARC_R_SCAN_L_START 450       /* 阶段2 左轮起始(差160, 渐进增大) */
#define ARC_R_SCAN_L_STEP    2       /* 阶段2 左轮每周期+2 */
#define ARC_R_SCAN_L_MAX   500       /* 阶段2 左轮上限(差240) */
#define ARC_R_SCAN_BLIND     180       /* 阶段2前600ms盲转,不检测黑线 */
#define AVOID_REENTER_MS   150       /* 回线后直冲时间 */
#define AVOID_REENTER_CYC  (AVOID_REENTER_MS / CTRL_PERIOD_MS)

/* ============================================================================
 *                           T型路口调头 参数
 * ============================================================================ */
#define BRAKE_SPEED          450       /* 急刹速度 */
#define BRAKE_COUNT_IR        20       /* 急刹周期数(100ms) */
#define IR_WAIT_MS          5000      /* 停车等待5s */
#define IR_WAIT_CYCLES      (IR_WAIT_MS / CTRL_PERIOD_MS)
#define TURN_180_MINCYC      300       /*盲转1200ms后开始检测, 找到为止 */
#define RETURN_DEBOUNCE_MS  2000      /* 返回起点消抖 */
#define RETURN_DEBOUNCE     (RETURN_DEBOUNCE_MS / CTRL_PERIOD_MS)
#define IR_REENTER_MS        200       /* 旋转找到线后直行缓冲200ms */
#define IR_REENTER_CYC       (IR_REENTER_MS / CTRL_PERIOD_MS)
#define RE_ENTER_SPD         300       /* 回线缓冲速度 */

/* 回线后PD增强: IR_RETURNING前400ms使用更强的KP/KD快速锁定 */
#define REENTER_BOOST_CYCLES  80

/* ============================================================================
 *                              蜂鸣器 参数
 * ============================================================================ */
#define BEEP_ON_CYC           15        /* 蜂鸣75ms */
#define BEEP_GAP_CYC          10        /* 间隔50ms */

/* ============================================================================
 *                              硬件 宏
 * ============================================================================ */
#define BUZZER_PORT          GPIOB
#define BUZZER_PIN           GPIO_Pin_3

/* ============================================================================
 *                              状态机 枚举
 * ============================================================================ */

/* 绕障状态: 弧线绕障策略 */
typedef enum {
    OBS_NORMAL = 0,       /* 正常循迹 */
    OBS_BRAKING,          /* 急刹100ms */
    OBS_STOPPED,          /* 暂停200ms(报警 + 稳定车姿) */
    OBS_ARC_AROUND,       /* 弧线绕障: 左弧线→右扫线 */
    OBS_RE_ENTER          /* 压线后直冲150ms平稳回线 */
} ObstacleState_t;

/* T型路口状态 */
typedef enum {
    IR_NORMAL = 0, IR_BRAKING, IR_WAITING,
    IR_TURNING, IR_RE_ENTER, IR_RETURNING, IR_DONE
} IR_State_t;

/* 报警状态 */
typedef enum {
    ALARM_IDLE = 0,
    ALARM_BEEP1_ON, ALARM_BEEP1_OFF,
    ALARM_BEEP2_ON, ALARM_BEEP2_DONE
} AlarmState_t;

/* ============================================================================
 *                              全局变量
 * ============================================================================ */
static int8_t   g_last_err  = 0;       /* PD: 上一周期误差 */
static int16_t  g_last_corr = 0;       /* PD: 上一周期修正量(低通滤波用) */

/* 绕障 */
static ObstacleState_t g_obs_st = OBS_NORMAL;
static uint16_t        g_obs_cn = 0;   /* 当前阶段内计数(每阶段复位) */
static uint8_t         g_obs_triggered = 0;  /* 本次障碍是否已触发, 防重复 */
static uint8_t         g_obs_debounce  = 0;  /* 超声波消抖计数 */

/* T型路口 */
static IR_State_t g_ir_st = IR_NORMAL;
static uint16_t   g_ir_cn = 0;
static uint16_t   g_reenter_boost = 0;  /* IR_RETURNING回线增强计数 */

/* 报警 */
static AlarmState_t g_alarm_st  = ALARM_IDLE;
static uint16_t     g_alarm_cn  = 0;

static uint32_t g_final_ms  = 0;       /* 第二次T型路口锁定的最终成绩 */
static uint32_t g_start_ms  = 0;       /* 出发时刻 */
static int16_t  g_front_dist = -1;     /* 超声波最新距离(-1=无效) */


volatile uint32_t g_sys_tick = 0;      /* SysTick毫秒, 中断自增 */

/* OLED 显示变量 */
static uint8_t  g_ch[8];                    /* 8路灰度状态 */
static int16_t  g_err, g_lspd, g_rspd, g_kp; /* 误差 + 轮速 + KP */
static uint8_t  g_mode, g_sub;             /* 模式 + 子状态 */

/* DWT寄存器 (CMSIS旧版未包含) */
#define DWT_CTRL    (*(volatile uint32_t *)0xE0001000)
#define DWT_CYCCNT  (*(volatile uint32_t *)0xE0001004)

/* ============================================================================
 *                         SysTick 毫秒时钟
 * ============================================================================ */
static void Clock_Init(void)
{
    SysTick_Config(SystemCoreClock / 1000);   /* 1ms一次中断 */
    /* 开启DWT周期计数器, 用于Delay_us不干扰SysTick */
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT_CYCCNT = 0;
    DWT_CTRL   |= 1;  /* CYCCNTENA = bit 0 */
}
static uint32_t Clock_GetMs(void) { return g_sys_tick; }

/* ============================================================================
 *                         蜂鸣器 (PB3, 低电平驱动)
 *   先置ODR再切输出 → 防止上电瞬间误响
 * ============================================================================ */
static void Buzzer_Init(void)
{
    GPIO_InitTypeDef s;
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    GPIO_SetBits(BUZZER_PORT, BUZZER_PIN);       /* 默认高=关 */
    s.GPIO_Pin   = BUZZER_PIN;
    s.GPIO_Mode  = GPIO_Mode_Out_PP;
    s.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(BUZZER_PORT, &s);
}
static void Buzzer_On(void)  { GPIO_ResetBits(BUZZER_PORT, BUZZER_PIN); }
static void Buzzer_Off(void) { GPIO_SetBits  (BUZZER_PORT, BUZZER_PIN); }

/* ============================================================================
 *                         报警双响 (非阻塞状态机)
 *     响75ms → 停50ms → 响75ms → 停 → ...
 * ============================================================================ */
static void Alarm_Tick(void)
{
    switch (g_alarm_st)
    {
        case ALARM_IDLE:
            Buzzer_Off();
            break;
        case ALARM_BEEP1_ON:
            Buzzer_On();
            if (++g_alarm_cn >= BEEP_ON_CYC)
            { g_alarm_st = ALARM_BEEP1_OFF; g_alarm_cn = 0; }
            break;
        case ALARM_BEEP1_OFF:
            Buzzer_Off();
            if (++g_alarm_cn >= BEEP_GAP_CYC)
            { g_alarm_st = ALARM_BEEP2_ON; g_alarm_cn = 0; }
            break;
        case ALARM_BEEP2_ON:
            Buzzer_On();
            if (++g_alarm_cn >= BEEP_ON_CYC)
            { g_alarm_st = ALARM_BEEP2_DONE; g_alarm_cn = 0; }
            break;
        case ALARM_BEEP2_DONE:
            Buzzer_Off();
            break;
    }
}

/* ============================================================================
 *                          Tracking_Control
 *          主控制逻辑: 传感器→避障→T型路口→PD→输出
 * ============================================================================ */
static void Tracking_Control(void)
{
    GS_Status_t sensor;
    int16_t lspd, rspd, kp, cp, cd, corr, sf;

    /* ---- 0. 读取传感器 (一次读取, 三处共用: 避障/T型/PD) ---- */
    sensor = GS_Read();
    { uint8_t _i; for (_i = 0; _i < 8; _i++) g_ch[_i] = sensor.ch[_i]; }

    /* ========================================================================
     * 1. 超声波避障 触发检测 (带消抖)
     *    连续2次检测到障碍物才触发, 过滤上电杂波/偶发误读
     * ======================================================================== */
    if (g_obs_st == OBS_NORMAL
        && (g_ir_st == IR_NORMAL || g_ir_st == IR_RETURNING)
        && g_front_dist > 0 && g_front_dist < OBSTACLE_DIST
        && !g_obs_triggered)
    {
        if (++g_obs_debounce >= 2)
        {
            g_obs_st        = OBS_BRAKING;
            g_obs_cn        = 0;
            g_obs_triggered = 1;
            g_alarm_st      = ALARM_BEEP1_ON;
            g_alarm_cn      = 0;
            g_obs_debounce  = 0;
        }
    }
    else if (!(g_front_dist > 0 && g_front_dist < OBSTACLE_DIST))
    {
        g_obs_debounce = 0;  /* 无障碍时清零消抖 */
    }

    /* 障碍物已通过且回常态 → 清除标记, 允许下次触发 */
    if (g_obs_st == OBS_NORMAL
        && !(g_front_dist > 0 && g_front_dist < OBSTACLE_DIST))
    {
        g_obs_triggered = 0;
    }

    Alarm_Tick();

    /* ======================================================================
     * 2. 绕障状态机 — 弧线绕障策略
     *    制动→暂停→左弧线绕障→右扫线找黑线→平稳交接
     * ====================================================================== */
    switch (g_obs_st)
    {
        case OBS_NORMAL:
            /* 正常循迹, 继续后面的T型/PD流程 */
            g_sub = 0;
            break;

        /* ---- 预处理 ---- */
        case OBS_BRAKING:
            Motor_Left_Set(-OBS_BRAKE_SPEED);
            Motor_Right_Set(-OBS_BRAKE_SPEED);
            g_lspd = -OBS_BRAKE_SPEED; g_rspd = -OBS_BRAKE_SPEED;
            g_mode = 2; g_sub = 1;
            if (++g_obs_cn >= BRAKE_COUNT)
            { g_obs_st = OBS_STOPPED; g_obs_cn = 0; }
            return;

        case OBS_STOPPED:
            Motor_Left_Set(0); Motor_Right_Set(0);
            g_lspd = 0; g_rspd = 0;
            g_mode = 2; g_sub = 5;
            if (++g_obs_cn >= OBS_PAUSE_CYC)
            { g_obs_st = OBS_ARC_AROUND; g_obs_cn = 0; }
            return;

        /* ---- 弧线绕障: 阶段1左弧线 → 阶段2锁死右转 ---- */
        case OBS_ARC_AROUND:
        {
            if (g_obs_cn < ARC_LEFT_CYC)
            {
                /* 阶段1: 左弧线绕障 */
                Motor_Left_Set( ARC_R_SLOW);
                Motor_Right_Set(ARC_L_SPD);
                g_lspd = ARC_R_SLOW; g_rspd = ARC_L_SPD;
                g_mode = 2; g_sub = 7;
            }
            else
            {
                /* 阶段2: 锁死右转, 盲转200ms后才检测黑线 */
                int16_t p2 = (int16_t)(g_obs_cn - ARC_LEFT_CYC);
                int16_t ls  = ARC_R_SCAN_L_START + p2 * ARC_R_SCAN_L_STEP;
                if (ls > ARC_R_SCAN_L_MAX) ls = ARC_R_SCAN_L_MAX;
                Motor_Left_Set(ls);
                Motor_Right_Set(ARC_R_SCAN_R_SPD);
                g_lspd = ls; g_rspd = ARC_R_SCAN_R_SPD;
                g_mode = 2; g_sub = 8;
            }
            g_obs_cn++;

            /* 盲转期过后检测黑线, 找不到就一直右转 */
            if (g_obs_cn >= ARC_LEFT_CYC + ARC_R_SCAN_BLIND
                && !GS_AllWhite(&sensor))
            { g_obs_st = OBS_RE_ENTER; g_obs_cn = 0; return; }
            return;
        }

        /* ---- 回线交接: 低速直冲 → 平稳交给PD ---- */
        case OBS_RE_ENTER:
            Motor_Left_Set( RE_ENTER_SPD);
            Motor_Right_Set(RE_ENTER_SPD);
            g_lspd = RE_ENTER_SPD; g_rspd = RE_ENTER_SPD;
            g_mode = 2; g_sub = 10;
            if (++g_obs_cn >= AVOID_REENTER_CYC)
            {
                g_obs_st    = OBS_NORMAL;
                g_obs_cn    = 0;
                g_last_err  = 0;
                g_last_corr = 0;
            }
            else { return; }
            break;
    }

    /* ======================================================================
     * 3. T型路口状态机
     *    制动 → 等待5s → 后退1s → 180°旋转(传感器引导) → 返回 → 停车
     * ====================================================================== */
    switch (g_ir_st)
    {
        case IR_NORMAL:
            /* ≥5路黑 → T型路口 */
            if (GS_IsTJunction(&sensor))
            {
                g_ir_st = IR_BRAKING;
                g_ir_cn = 0;
            }
            break;

        case IR_BRAKING:
            /* 双轮反转急刹400, 100ms */
            Motor_Left_Set(-BRAKE_SPEED);
            Motor_Right_Set(-BRAKE_SPEED);
            g_lspd = -BRAKE_SPEED; g_rspd = -BRAKE_SPEED;
            g_mode = 2; g_sub = 1;
            if (++g_ir_cn >= BRAKE_COUNT_IR)
            { g_ir_st = IR_WAITING; g_ir_cn = 0; }
            return;

        case IR_WAITING:
            /* 停车等待5s, 直接进入旋转 (不后退) */
            Motor_Left_Set(0); Motor_Right_Set(0);
            g_lspd = 0; g_rspd = 0;
            g_mode = 2; g_sub = 2;
            if (++g_ir_cn >= IR_WAIT_CYCLES)
            { g_ir_st = IR_TURNING; g_ir_cn = 0; }
            return;

        case IR_TURNING:
            /* 原地180°调头: 左轮正转560, 右轮反转200 */
            Motor_Left_Set(600);
            Motor_Right_Set(-250);
            g_lspd =  600; g_rspd = -250;
            g_mode = 2; g_sub = 3;
            g_ir_cn++;

            /* 盲转1200ms后, CH2~CH5检测到线即停, 找不到就一直转 */
            if (g_ir_cn > TURN_180_MINCYC
                && (sensor.ch[2] || sensor.ch[3] || sensor.ch[4] || sensor.ch[5]))
            {
                g_ir_st         = IR_RE_ENTER;
                g_ir_cn         = 0;
                g_reenter_boost = 0;
                Encoder_Reset();
            }
            return;

        case IR_RE_ENTER:
            /* 找到黑线后直行200ms缓冲, 平稳过渡到PD循迹 */
            Motor_Left_Set(RE_ENTER_SPD);
            Motor_Right_Set(RE_ENTER_SPD);
            g_lspd = RE_ENTER_SPD; g_rspd = RE_ENTER_SPD;
            g_mode = 2; g_sub = 12;
            if (++g_ir_cn >= IR_REENTER_CYC)
            {
                g_ir_st         = IR_RETURNING;
                g_ir_cn         = 0;
                g_last_err      = 0;
                g_last_corr     = 0;
                g_reenter_boost = 0;
                g_obs_triggered = 0;  /* 返程允许再次避障 */
            }
            return;

        case IR_RETURNING:
            /* 返回起点, 2s消抖后≥5路黑 → 到站 */
            if (++g_ir_cn >= RETURN_DEBOUNCE
                && GS_IsTJunction(&sensor))
            { g_ir_st = IR_DONE; g_ir_cn = 0; }
            break;

        case IR_DONE:
            if (g_ir_cn == 0)
                g_final_ms = Clock_GetMs() - g_start_ms;
            /* 制动 → 停车 */
            if (g_ir_cn < BRAKE_COUNT_IR)
            {
                Motor_Left_Set(-BRAKE_SPEED);
                Motor_Right_Set(-BRAKE_SPEED);
                g_lspd = -BRAKE_SPEED; g_rspd = -BRAKE_SPEED;
                g_mode = 2; g_sub = 1;
                g_ir_cn++;
            }
            else
            {
                Motor_Left_Set(0); Motor_Right_Set(0);
                g_lspd = 0; g_rspd = 0;
                g_mode = 2; g_sub = 4;
            }
            return;
    }

    /* 非循迹状态 不执行PD (IR_RETURNING 需要PD循迹回起点) */
    if (g_ir_st != IR_NORMAL && g_ir_st != IR_RETURNING) return;

    /* ======================================================================
     * 4. 动态KP + 回线增强
     *   IR_RETURNING前400ms: KP=100~160, KD=22, 低通9:1 → 快速锁定
     *   正常: KP=80~130, KD=15, 低通7:3
     * ====================================================================== */
    if (g_ir_st == IR_RETURNING && g_reenter_boost < REENTER_BOOST_CYCLES)
    {
        /* 回线增强: 中间稳(80), 偏了快拽(→160), 偏远了猛拉(200) */
             if (abs(sensor.error) <= 1) kp = 100;
        else if (abs(sensor.error) >= 7) kp = 200;
        else kp = 100 + (200 - 100) * (abs(sensor.error) - 1) / (7 - 1);

        cp   = sensor.error * kp;
        cd   = (sensor.error - g_last_err) * 22;
        corr = cp + cd;
        corr = (corr * 9 + g_last_corr * 1) / 10;  /* 9:1 低通 */
        g_reenter_boost++;
    }
    else
    {
        /* 正常循迹: 中间(CH3/CH4)稳如直线, 偏了立即修正, 偏远了猛拉 */
             if (abs(sensor.error) <= 1) kp = 80;
        else if (abs(sensor.error) >= 7) kp = 160;
        else kp = 80 + (160 - 80) * (abs(sensor.error) - 1) / (7 - 1);

        cp   = sensor.error * kp;
        cd   = (sensor.error - g_last_err) * KD;
        corr = cp + cd;
        corr = (corr * 7 + g_last_corr * 3) / 10;  /* 7:3 低通 */
    }
    g_last_corr = corr;

    /* ======================================================================
     * 6. 自适应速度 — 弯道降速, 减小惯性
     *   直线: 350 (全速)
     *   中弯(3≤|e|<5): 350→150 线性降速
     *   急弯(|e|≥5): 170 (大幅降速 + 内轮反转)
     * ====================================================================== */
         if (abs(sensor.error) >= SHARP_TURN_ERR) sf = SHARP_SPEED;
    else if (abs(sensor.error) >= SPEED_DROP_ERR)
    {
        sf = BASE_SPEED - (BASE_SPEED - SPEED_MIN)
           * (abs(sensor.error) - SPEED_DROP_ERR) / (7 - SPEED_DROP_ERR);
        if (sf < SPEED_MIN) sf = SPEED_MIN;
    }
    else sf = BASE_SPEED;

    /* ======================================================================
     * 7. 差速转向 + 限幅
     *   左轮 = 基础速度 + 修正量(正=右偏需左转=左轮加速)
     *   右轮 = 基础速度 - 修正量
     *   急弯时内轮反转(负值), 普通弯内轮停转(0)
     * ====================================================================== */
    lspd = sf + corr;
    rspd = sf - corr;

    if (abs(sensor.error) >= SHARP_TURN_ERR)
    {
        /* 急弯: 内轮可反转, 形成差速, 转弯半径大幅缩小 */
        if (lspd < 0) lspd = -INNER_REVERSE_SPD;
        if (rspd < 0) rspd = -INNER_REVERSE_SPD;
    }
    else
    {
        /* 普通弯/直线: 内轮最多停转, 不反转 */
        if (lspd < 0) lspd = 0;
        if (rspd < 0) rspd = 0;
    }

    /* PWM硬限幅 ±999 */
    if (lspd >  SPEED_MAX) lspd =  SPEED_MAX;
    if (lspd < -SPEED_MAX) lspd = -SPEED_MAX;
    if (rspd >  SPEED_MAX) rspd =  SPEED_MAX;
    if (rspd < -SPEED_MAX) rspd = -SPEED_MAX;

    g_last_err = sensor.error;

    /* ======================================================================
     * 8. 输出到电机
     * ====================================================================== */
    Motor_Left_Set(lspd);
    Motor_Right_Set(rspd);
    g_lspd = lspd; g_rspd = rspd;
    g_err  = sensor.error;
    g_kp   = kp;
    g_mode = 1; g_sub = 0;
}

/* ============================================================================
 *                          OLED_Update
 *   每50ms刷新: 传感器/轮速/KP/时间/距离/状态
 * ============================================================================ */
static void OLED_Update(void)
{
    /* IR_DONE后时间冻结; 否则实时显示 */
    uint32_t elap;
    if (g_ir_st == IR_DONE)
        elap = g_final_ms;              /* 锁定最终成绩 */
    else
        elap = Clock_GetMs() - g_start_ms;

    int32_t dist = Encoder_GetDistance();

    /* 第1行: 8路灰度状态 + 误差 + 模式 */
    { uint8_t _i; OLED_ShowString(1, 1, "S");
      for (_i = 0; _i < 8; _i++) OLED_ShowNum(1, (uint8_t)(2 + _i), g_ch[_i], 1); }
    OLED_ShowString(1, 10, "E");
    OLED_ShowSignedNum(1, 12, g_err, 2);
    OLED_ShowChar(1, 15, 'M'); OLED_ShowNum(1, 16, g_mode, 1);

    /* 第2行: 左右轮速度 + 超声波距离 */
    OLED_ShowString(2, 1, "L"); OLED_ShowSignedNum(2, 2, g_lspd, 3);
    OLED_ShowString(2, 6, "R"); OLED_ShowSignedNum(2, 7, g_rspd, 3);
    OLED_ShowString(2, 11,"U");
    if (g_front_dist > 0) {
        OLED_ShowNum(2, 12, (uint32_t)g_front_dist / 10, 2);  /* cm */
        OLED_ShowString(2, 14, "cm");
    } else {
        OLED_ShowString(2, 12, "---");
    }

    /* 第3行: 运行时间 + 行驶距离 */
    OLED_ShowString(3, 1, "T");
    OLED_ShowNum(3, 2, elap / 1000, 3);
    OLED_ShowChar(3, 5, '.');
    OLED_ShowNum(3, 6, (elap % 1000) / 100, 1);
    OLED_ShowChar(3, 7, 's');
    OLED_ShowString(3, 9, "D");
    OLED_ShowSignedNum(3, 10, dist / 100, 3);         /* 单位cm */
    OLED_ShowChar(3, 13, '.');
    OLED_ShowSignedNum(3, 14, (dist / 10) % 10, 1);   /* 小数点后1位 */
    OLED_ShowString(3, 15, "cm");

    /* 第4行: 根据模式/子状态显示对应信息 */
    if (g_ir_st == IR_DONE)
    {
        OLED_ShowString(4, 1, "Done! T=");
        OLED_ShowNum(4, 8, g_final_ms / 1000, 3);
        OLED_ShowChar(4, 11, '.');
        OLED_ShowNum(4, 12, (g_final_ms % 1000) / 100, 1);
        OLED_ShowChar(4, 13, 's');
    }
    else if (g_mode == 2)
    {
        /* 特殊模式: 根据子状态显示 */
        switch (g_sub)
        {
            case 1:  OLED_ShowString(4, 1, "!!BRAKE!!      "); break;
            case 2:  OLED_ShowString(4, 1, "!!WAIT!!       "); break;
            case 3:  OLED_ShowString(4, 1, "!!TURN!!       "); break;
            case 4:  OLED_ShowString(4, 1, "Done! T=");
                     OLED_ShowNum(4, 8, elap / 1000, 2);
                     OLED_ShowChar(4, 10, 's');
                     break;
            case 5:  OLED_ShowString(4, 1, "!!STOP!!       "); break;
            case 7:  OLED_ShowString(4, 1, "!!ARC L!!      "); break;  /* 左弧线绕障 */
            case 8:  OLED_ShowString(4, 1, "!!SCAN R!!     "); break;  /* 右扫寻线 */
            case 10: OLED_ShowString(4, 1, "!!RE-ENTER!!   "); break;  /* 回线交接 */
            case 12: OLED_ShowString(4, 1, "!!IR RE-EN!!   "); break;  /* 调头回线缓冲 */
            default: OLED_ShowString(4, 1, "!!STOP!!       "); break;
        }
    }
    else
    {
        /* 正常循迹: 显示中间4路原始ADC值(调试用) */
        OLED_ShowString(4, 1, "2:"); OLED_ShowNum(4, 3, g_raw_adc[2], 4);
        OLED_ShowString(4, 8, "3:"); OLED_ShowNum(4, 10, g_raw_adc[3], 4);
        /* 注: 若需看更多通道, 可改为 g_raw_adc[4], g_raw_adc[5] */
    }
}

/* ============================================================================
 *                          main
 * ============================================================================ */
int main(void)
{
    /* 禁用JTAG, 释放PB3给蜂鸣器 */
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
    GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);

    /* 初始化所有外设 */
    Clock_Init();
    Timer_Init();
    Delay_ms(200);              /* OLED上电稳定时间 */
    OLED_Init(); OLED_Clear();
    OLED_ShowString(1, 1, "Init OLED...OK");
    Motor_Init();      OLED_ShowString(2, 1, "Init Motor..OK");
    GS_Init();         OLED_ShowString(3, 1, "Init GS.....OK");
    HC_SR04_Init();
    Buzzer_Init();
    Encoder_Init();
    OLED_ShowString(4, 1, "Init Ext...OK");
    Delay_ms(300);

    /* 启动画面 */
    OLED_Clear();
    OLED_ShowString(1, 1, "== Line Car ==");
    OLED_ShowString(2, 1, "TB6612 Ready");
    OLED_ShowString(3, 1, "8ch Grayscale");
    OLED_ShowString(4, 1, "2s Auto Start");
    Delay_ms(2000);
    OLED_Clear();

    /* 清零状态 + 记录出发时间 */
    Encoder_Reset();
    g_start_ms = Clock_GetMs();

    {
        uint32_t next_cycle = g_start_ms;

        while (1)
        {
            static uint16_t oled_tick = 0;

            /* OLED + 超声波每100ms更新 */
            oled_tick += CTRL_PERIOD_MS;
            if (oled_tick >= OLED_PERIOD_MS)
            {
                oled_tick = 0;
                g_front_dist = sonar_get_mm();
                sonar_start();
                OLED_Update();
            }

            Tracking_Control();
            Encoder_Update();
            sonar_timeout_check();

            /* 精确周期控制: 等待到下一个5ms边界 */
            next_cycle += CTRL_PERIOD_MS;
            while ((int32_t)(Clock_GetMs() - next_cycle) < 0);
        }
    }
}
