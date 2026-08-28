/*
 * Common_peripherals.c
 *
 *  Created on: 2026年4月
 *      Author: Super_burger
 *  说明：通用外设驱动集合，包含蜂鸣器、按键、LED、电机、编码器、舵机、遥控器等
 */

#include "zf_common_headfile.h"

#define wheel_diameter  (0.07f)     /* 车轮直径（米），用于里程计算 */

/* ==================== 蜂鸣器 ==================== */

/**
 * @brief  蜂鸣器GPIO初始化
 * @note   配置为推挽输出，初始低电平（不响）
 */
void Buzzer_init(void)
{
    gpio_init(Buzzer_pin, GPO, 0, GPO_PUSH_PULL);
}

/**
 * @brief  蜂鸣器响指定时间后关闭（阻塞式）
 * @param  TIME1  蜂鸣器持续时间（ms）
 */
void Buzzer_check(int TIME1)
{
    gpio_set_level(Buzzer_pin, 1);   /* 开 */
    system_delay_ms(TIME1);
    gpio_set_level(Buzzer_pin, 0);   /* 关 */
}

/* ==================== 按键与LED ==================== */

/* 按键当前状态（1=未按下，0=按下，上拉输入） */
uint8 key1_state = 1;
uint8 key2_state = 1;
uint8 key3_state = 1;
uint8 key4_state = 1;

/* 按键上一次状态（用于检测边沿） */
uint8 key1_state_last = 0;
uint8 key2_state_last = 0;
uint8 key3_state_last = 0;
uint8 key4_state_last = 0;

/* 按键按下标志（检测到上升沿时置1，需手动清除） */
uint8 key1_flag;
uint8 key2_flag;
uint8 key3_flag;
uint8 key4_flag;

/**
 * @brief  按键与LED初始化
 * @note   按键配置为上拉输入，LED配置为推挽输出（初始高电平=灭）
 *         Switch1/Switch2 配置为浮空输入
 */
void Key_init(void)
{
    gpio_init(KEY1, GPI, 1, GPI_PULL_UP);
    gpio_init(KEY2, GPI, 1, GPI_PULL_UP);
    gpio_init(KEY3, GPI, 1, GPI_PULL_UP);
    gpio_init(KEY4, GPI, 1, GPI_PULL_UP);

    gpio_init(LED1, GPO, 1, GPO_PUSH_PULL);
    gpio_init(LED2, GPO, 1, GPO_PUSH_PULL);
    gpio_init(LED3, GPO, 1, GPO_PUSH_PULL);
    gpio_init(LED4, GPO, 1, GPO_PUSH_PULL);

    gpio_init(Switch1, GPI, 1, GPI_FLOATING_IN);
    gpio_init(Switch2, GPI, 1, GPI_FLOATING_IN);
}

/**
 * @brief  按键扫描函数（在中断中周期调用）
 * @note   检测按键上升沿（从未按下→按下），检测到后置位对应flag
 *         使用状态机方式，保存上一次状态用于边沿检测
 */
void Key_scan(void)
{
    /* 保存上一次状态 */
    key1_state_last = key1_state;
    key2_state_last = key2_state;
    key3_state_last = key3_state;
    key4_state_last = key4_state;

    /* 读取当前状态 */
    key1_state = gpio_get_level(KEY1);
    key2_state = gpio_get_level(KEY2);
    key3_state = gpio_get_level(KEY3);
    key4_state = gpio_get_level(KEY4);

    /* 上升沿检测（当前按下 && 上一次未按下） */
    if (key1_state && !key1_state_last) { key1_flag = 1; }
    if (key2_state && !key2_state_last) { key2_flag = 1; }
    if (key3_state && !key3_state_last) { key3_flag = 1; }
    if (key4_state && !key4_state_last) { key4_flag = 1; }
}

/* 按键标志清除函数 */
void key1_clear(void) { key1_flag = 0; }
void key2_clear(void) { key2_flag = 0; }
void key3_clear(void) { key3_flag = 0; }
void key4_clear(void) { key4_flag = 0; }

/* ==================== 电机驱动（CYT2无刷驱动） ==================== */

/**
 * @brief  电机开环控制（直接设置PWM占空比）
 * @param  SPEED  目标速度（PWM值，范围 M_MIN~M_MAX）
 * @note   先限幅再输出，负值表示反转
 */
void CYT2_S_motor_ctrl(int32 SPEED)
{
    SPEED = SPEED > M_MAX ? M_MAX : (SPEED < M_MIN ? M_MIN : SPEED); /* 限幅 */
    small_driver_set_duty((int16)-SPEED, 0);
}

/**
 * @brief  电机速度闭环控制（PID）
 * @param  T_SPEED  目标速度
 * @note   使用速度环PID计算输出，再调用开环控制函数输出
 *         反馈值为电机左轮速度数据（取反）
 */
void CYT2_S_motor_loop_ctrl(float T_SPEED)
{
    pid_control(&roll_balance_cascade.speed_cycle, T_SPEED, -motor_value.receive_left_speed_data);
    CYT2_S_motor_ctrl(roll_balance_cascade.speed_cycle.out);
}

/**
 * @brief  电机调试界面（按键调节速度并显示）
 * @note   在主循环中调用：
 *         KEY1：速度+100，KEY2：速度-100
 *         KEY3：速度+500，KEY4：速度-500
 *         屏幕显示当前速度设定值
 */
void Motor_text(void)
{
    static int32 S_PSEED = 0;

    Key_scan();

    if (key1_flag) { key1_flag = 0; S_PSEED += 100; }
    if (key2_flag) { key2_flag = 0; S_PSEED -= 100; }
    if (key3_flag) { key3_flag = 0; S_PSEED += 500; }
    if (key4_flag) { key4_flag = 0; S_PSEED -= 500; }

    ips_show_string(8 * 0, 16 * 1, "Motor_text");
    ips_show_string(8 * 0, 16 * 3, "V:");
    ips_show_int(8 * 5, 16 * 3, S_PSEED, 5);

    CYT2_S_motor_ctrl(S_PSEED);
}

/* ==================== 编码器 ==================== */

float A_SPEED = 0;      /* 编码器计算出的当前速度 */
int32 Distance = 0;      /* 累计距离 */

/**
 * @brief  编码器初始化
 * @note   使用方向编码器模式（encoder_dir_init），非正交编码器模式
 */
void QUD_encoder_init(void)
{
    encoder_dir_init(ENCODER_1, ENCODER_1_A, ENCODER_1_B);
}

int32 g_encoder_raw = 0;   /* 编码器原始计数值 */

/**
 * @brief  获取编码器脉冲数据（在中断中周期调用）
 * @note   当前实现直接使用无刷驱动返回的速度数据作为速度反馈
 *         （注释掉的正交编码器读取代码保留供参考）
 */
void QUD_encoder_pulse_get(void)
{
    A_SPEED = -motor_value.receive_left_speed_data;
}

/**
 * @brief  根据编码器速度计算单位时间内行驶距离
 * @param  A_SPEED  编码器速度（脉冲数）
 * @return  单位时间内行驶距离（米）
 * @note   计算公式：距离 = 转数 × 周长
 *         转数 = 脉冲数 / 1024(编码器线数) / 4(四倍频)
 *         周长 = π × 直径(0.07m)
 */
float Cal_Distance(int32 A_SPEED)
{
    float wheel_c   = 3.1415926f * 0.070f;          /* 车轮周长 = π × 直径 */
    float wheel_rev = ((float)A_SPEED) / 1024.0f / 4.0f;  /* 转数 = 脉冲/线数/倍频 */
    return wheel_rev * wheel_c;
}

/**
 * @brief  根据电机转速（RPM）计算单位时间行驶距离（米）
 * @param  speed  电机转速（RPM，负值表示反转）
 * @return  单位时间行驶距离（米）
 * @note   距离 = 转速(RPM)/60 × 直径 × π × 0.001
 *         适用于无刷驱动直接返回RPM的场景
 */
float CYT2_get_distance_mag(int16 speed)
{
    float speed_f = -(float)speed;  /* 取反，与实际方向一致 */
    return (speed_f / 60.0f * wheel_diameter * PI * 0.001f);
}

/**
 * @brief  编码器调试界面（按键调节速度并显示编码器数据）
 * @note   在主循环中调用：
 *         KEY1：速度+50，KEY2：速度-50
 *         KEY3：速度+500，KEY4：速度-500
 *         屏幕显示设定速度、电机实际速度、累计里程
 */
void Encoder_text(void)
{
    static int32 S_PSEED = 0;

    Key_scan();

    if (key1_flag) { key1_flag = 0; S_PSEED += 50; }
    if (key2_flag) { key2_flag = 0; S_PSEED -= 50; }
    if (key3_flag) { key3_flag = 0; S_PSEED += 500; }
    if (key4_flag) { key4_flag = 0; S_PSEED -= 500; }

    ips_show_string(8 * 0, 16 * 1, "Encoder_text");
    ips_show_string(8 * 0, 16 * 3, "V:");
    ips_show_int(8 * 7, 16 * 3, S_PSEED, 5);
    ips_show_string(8 * 0, 16 * 5, "B_V:");
    ips_show_int(8 * 7, 16 * 5, -motor_value.receive_left_speed_data, 5);
    ips_show_string(8 * 0, 16 * 6, "D:");
    ips200_show_float(8 * 7, 16 * 6, guandao_lucheng, 5, 5);

    CYT2_S_motor_ctrl(S_PSEED);
}

/* ==================== 舵机 ==================== */

/**
 * @brief  舵机PWM初始化
 * @note   初始化舵机PWM输出，频率为SERVO_MOTOR_FREQ，初始占空比为中值
 */
void Steer_init(void)
{
    pwm_init(SERVO_MOTOR_PWM, SERVO_MOTOR_FREQ, (uint32)SERVO_MOTOR_DUTY(SERVO_MOTOR_MID));
}

/**
 * @brief  设置舵机角度
 * @param  angle  目标角度（度，范围 SERVO_MOTOR_LMAX~SERVO_MOTOR_RMAX）
 * @note   先限幅再计算占空比输出
 */
void Steer_set(int angle)
{
    if (angle > SERVO_MOTOR_RMAX) { angle = SERVO_MOTOR_RMAX; } /* 限幅 */
    if (angle < SERVO_MOTOR_LMAX) { angle = SERVO_MOTOR_LMAX; }
    pwm_set_duty(SERVO_MOTOR_PWM, (uint32)SERVO_MOTOR_DUTY(angle));
}

/**
 * @brief  舵机调试界面（按键调节角度并显示）
 * @note   在主循环中调用：
 *         KEY1：角度+10，KEY2：角度-10
 *         KEY3：角度+1，KEY4：角度-1
 *         屏幕显示当前角度设定值
 */
void Steer_text(void)
{
    static int32 angle = SERVO_MOTOR_MID;

    Key_scan();

    if (key1_flag) { key1_flag = 0; angle += 10; }
    if (key2_flag) { key2_flag = 0; angle -= 10; }
    if (key3_flag) { key3_flag = 0; angle += 1; }
    if (key4_flag) { key4_flag = 0; angle -= 1; }

    ips_show_string(0, 100, "Steer_text");
    ips_show_int(100, 16 * 3, angle, 5);
    Steer_set(angle);
}

/* ==================== 遥控器（SBUS） ==================== */

int CTRL_flag = 0;   /* 遥控器控制标志（1=遥控器接管，0=自动程序运行） */

/**
 * @brief  遥控器通道值线性映射为角度（-180~180度）
 * @param  channel  遥控器通道值（范围 192~1777，中值 992）
 * @return  映射后的角度（-360~360度）
 * @note   包含死区处理：中值±15范围内返回0
 *         左半区（192~977）映射为 -360~0
 *         右半区（1007~1777）映射为 0~360
 */
int16_t Remap_Angle_Linear(int16_t channel)
{
    const int16_t CH_MIN    = 192;
    const int16_t CH_MAX    = 1777;
    const int16_t CH_MID    = 992;
    const int16_t ANG_MAX   = 360;   /* 最大映射角度 */
    const int16_t DEAD_ZONE = 15;    /* 死区范围（中值±15） */

    /* 输入限幅 */
    if (channel < CH_MIN) channel = CH_MIN;
    if (channel > CH_MAX) channel = CH_MAX;

    /* 死区内返回0 */
    if (channel > CH_MID - DEAD_ZONE && channel < CH_MID + DEAD_ZONE)
        return 0;

    float angle;
    if (channel <= CH_MID - DEAD_ZONE)
    {
        /* 左半区：CH_MIN ~ CH_MID-DEAD_ZONE 映射为 -ANG_MAX ~ 0 */
        int16_t left_max = CH_MID - DEAD_ZONE;
        angle = -ANG_MAX + (channel - CH_MIN) * (ANG_MAX / (float)(left_max - CH_MIN));
    }
    else
    {
        /* 右半区：CH_MID+DEAD_ZONE ~ CH_MAX 映射为 0 ~ ANG_MAX */
        int16_t right_min = CH_MID + DEAD_ZONE;
        angle = (channel - right_min) * (ANG_MAX / (float)(CH_MAX - right_min));
    }

    /* 输出限幅 */
    if (angle > ANG_MAX) angle = ANG_MAX;
    if (angle < -ANG_MAX) angle = -ANG_MAX;

    return (int16_t)angle;
}

/**
 * @brief  旋钮步进角度选择器（通过遥控器通道切换预设角度）
 * @param  ch  遥控器通道值（192~1777）
 * @return  当前选中的预设角度
 * @note   使用状态机检测旋钮从中间位置拨到极端位置再回到中间的动作
 *         每完成一次动作切换到下一个预设角度
 *         当前预设角度数组：{180}（可扩展为多个角度）
 */
float KnobStepAngle(int16_t ch)
{
    static const float angles[] = {180};   /* 预设角度数组 */

    static uint8_t idx = 0;            /* 当前角度索引 */
    static uint8_t state = 0;          /* 状态机状态：0=等待拨出，1=等待回中 */
    static float current_angle = 0.0f; /* 当前输出角度 */

    const int16_t MID  = 992;
    const int16_t DEAD = 30;           /* 中间位置死区 */
    const int16_t EXIT = 200;          /* 拨出触发阈值 */

    /* 输入限幅 */
    if (ch < 192) ch = 192;
    if (ch > 1777) ch = 1777;

    int16_t offset = ch - MID;
    uint8_t is_mid  = (offset > -DEAD && offset < DEAD);   /* 是否在中间位置 */
    uint8_t is_exit = (offset > EXIT || offset < -EXIT);    /* 是否拨出到极端位置 */

    if (state == 0)
    {
        /* 等待拨出：从中间拨到极端位置 */
        if (!is_mid && is_exit)
        {
            state = 1;
        }
    }
    else if (state == 1)
    {
        /* 等待回中：从极端回到中间位置，切换角度 */
        if (is_mid)
        {
            current_angle = angles[idx];
            idx = (idx + 1) % 4;   /* 循环切换（数组可扩展到4个角度） */
            state = 0;
        }
    }

    return current_angle;
}

/**
 * @brief  遥控器数据调试输出（串口打印6个通道数据）
 * @note   在主循环中调用，当收到一帧完整SBUS数据时：
 *         - 连接正常：打印CH1~CH6通道值
 *         - 连接断开：打印断开提示
 */
void GUN_ctrl_text(void)
{
    if (1 == uart_receiver.finsh_flag)
    {
        if (1 == uart_receiver.state)
        {
            printf("CH1-CH6 data: ");
            for (int i = 0; i < 6; i++)
            {
                printf("%d ", uart_receiver.channel[i]);
            }
            printf("\r\n");
        }
        else
        {
            printf("Remote control has been disconnected.\r\n");
        }
        uart_receiver.finsh_flag = 0;   /* 清除接收完成标志 */
    }
}

/**
 * @brief  遥控器控制逻辑（检测遥控器是否接管）
 * @note   在主循环中调用，当CH2通道值>=1500时：
 *         - 电机停止（速度=0）
 *         - 舵机回中
 *         - CTRL_flag置1（暂停自动程序）
 *         否则CTRL_flag清0（恢复自动程序）
 */
void new_ctrl(void)
{
    if (1 == uart_receiver.finsh_flag)
    {
        if (1 == uart_receiver.state)
        {
            /* CH2通道>=1500表示遥控器接管 */
            if (uart_receiver.channel[2] >= 1500)
            {
                CYT2_S_motor_ctrl(0);       /* 电机停止 */
                Steer_set(SERVO_MOTOR_MID);  /* 舵机回中 */
                CTRL_flag = 1;                /* 标记遥控器接管 */
            }
            else
            {
                CTRL_flag = 0;                /* 恢复自动程序 */
            }
        }
        uart_receiver.finsh_flag = 0;   /* 清除接收完成标志 */
    }
}
