/*
 * KE_1.c
 *
 *  Created on: 2026年4月8日
 *      Author: Super_burger
 *  说明：科目一（单车定向）控制逻辑，包含车身平衡控制、车头随动、调试显示
 */

#include "zf_common_headfile.h"

/* ==================== 宏定义 ==================== */
#define SPEED_MAX           1300    /* 直道最大速度 */
#define SPEED_MIN           1100    /* 弯道最小速度 */
#define TURN_DECEL_THRESHOLD  20.0f  /* 弯道减速触发阈值（度），误差大于此值降到最低速 */
#define STRAIGHT_THRESHOLD    0.5f   /* 直行判定阈值（度），误差小于此值保持最高速 */
#define TURN_LINEAR_RANGE    19.5f   /* 线性减速区间长度 = TURN_DECEL_THRESHOLD - STRAIGHT_THRESHOLD */
#define U_TURN_TIME_MS       6000    /* 掉头触发时间（ms），运行到此时间后目标角变为-180度 */
#define STEER_STEP_SIZE      2       /* 转向角步进量（度/次） */

/* ==================== 全局变量 ==================== */
uint32 Mode_chage = 0;     /* 模式切换标志（0=正常模式，1=车头随动模式） */
uint32 sys_times = 0;      /* 系统计时（ms），在中断中自增 */
float  End_error = 0;      /* 最终误差 */
float  Taget_angle = 0;    /* 目标转向角度（度），注意：变量名保留原拼写以兼容其他文件 */


/**
 * @brief  科目一车身控制主函数（平衡+转向+速度闭环）
 * @note   在1ms中断中调用，执行以下功能：
 *         1. 首次调用初始化目标角和PID参数
 *         2. IMU数据低通滤波
 *         3. 根据转向误差自动减速（弯道减速、直道加速）
 *         4. 电机速度闭环控制
 *         5. 运行6秒后自动掉头（目标角设为-180度）
 *         6. 三级PID控制：角速度环(1ms) → 角度环(5ms) → 转向环(20ms)
 *         7. 舵机输出 = 中值 + 角速度环输出
 */
void Body_ctrl_1(void)
{
    static int32 I = 0;   /* 函数调用计时（ms），用于控制掉头时机 */
    I++;

    /* 首次调用初始化 */
    static bool once = false;
    if (!once)
    {
        Taget_angle = 0;              /* 初始目标角为0度（直行） */
        balance_mode_parameter(1);     /* 分配科目一PID参数 */
        once = true;
    }

    /* IMU数据一阶低通滤波 */
    Imu_lowpass_filter();

    /* ===== 弯道自动减速逻辑 ===== */
    /* 计算当前转向误差（取绝对值） */
    float turn_err = fabsf(AngleErrorNormalize(Taget_angle - roll_balance_cascade.posture_value.yaw));
    float speed_target = SPEED_MAX;

    if (turn_err > TURN_DECEL_THRESHOLD)
    {
        /* 误差 > 20度 → 降到最低速 */
        speed_target = SPEED_MIN;
    }
    else if (turn_err > STRAIGHT_THRESHOLD)
    {
        /* 误差在 0.5~20度 之间 → 线性减速 */
        speed_target = SPEED_MAX - (SPEED_MAX - SPEED_MIN) * (turn_err - STRAIGHT_THRESHOLD) / TURN_LINEAR_RANGE;
    }
    /* 误差 < 0.5度 → 直道，保持最高速 */

    /* 电机速度闭环控制 */
    CYT2_S_motor_loop_ctrl(speed_target);

    /* 运行到设定时间后自动掉头 */
    if (I >= U_TURN_TIME_MS)
    {
        Taget_angle = -180;  /* 目标角设为-180度，实现掉头 */
    }

    /* ===== 三级PID控制 ===== */

    /* 角速度环（每1ms执行） */
    if (sys_times % 1 == 0)
    {
        /* 根据IMU型号选择对应的陀螺仪轴 */
        if (Imu_type == 1)
        {
            pid_control(&roll_balance_cascade.angular_speed_cycle,
                        roll_balance_cascade.angle_cycle.out,
                        imu660ra_gyro_x);
        }
        else if (Imu_type == 2)
        {
            pid_control(&roll_balance_cascade.angular_speed_cycle,
                        roll_balance_cascade.angle_cycle.out,
                        imu660rb_gyro_y);
        }
        else if (Imu_type == 3)
        {
            pid_control(&roll_balance_cascade.angular_speed_cycle,
                        roll_balance_cascade.angle_cycle.out,
                        imu963ra_gyro_y);
        }
    }

    /* 角度环（每5ms执行） */
    if (sys_times % 5 == 0)
    {
        pid_control(&roll_balance_cascade.angle_cycle,
                    roll_balance_cascade.turn_cycle.out - roll_balance_cascade.posture_value.mechanical_zero,
                    -roll_balance_cascade.posture_value.rol);
    }

    /* 转向环（每20ms执行） */
    if (sys_times % 20 == 0)
    {
        /* 目标角步进逼近，防止突变 */
        pid_control(&roll_balance_cascade.turn_cycle,
                    AngleErrorNormalize(StepApproach(Taget_angle, STEER_STEP_SIZE) - roll_balance_cascade.posture_value.yaw),
                    0);
    }

    /* 舵机输出 = 舵机中值 + 角速度环输出 */
    Steer_set(SERVO_MOTOR_MID + roll_balance_cascade.angular_speed_cycle.out);
}


/**
 * @brief  车头随动保持模式（仅依靠角速度和角度环维持车头随动，无速度和转向控制）
 * @note   在1ms中断中调用，用于惯导推车场景：
 *         1. 首次调用分配科目一PID参数
 *         2. IMU数据低通滤波
 *         3. 角速度环PID（每1ms）
 *         4. 角度环PID（每5ms），目标角为0（保持水平）
 *         5. 舵机输出 = 中值 + 角速度环输出
 *         注意：此模式不控制电机速度和转向，仅保持车头稳定
 */
void Body_keep(void)
{
    static int32 I = 0;   /* 函数调用计时 */
    I++;

    /* 首次调用初始化PID参数 */
    static bool once = false;
    if (!once)
    {
        balance_mode_parameter(1);   /* 分配科目一PID参数 */
        once = true;
    }

    /* IMU数据一阶低通滤波 */
    Imu_lowpass_filter();

    /* 角速度环（每1ms执行） */
    if (sys_times % 1 == 0)
    {
        /* 根据IMU型号选择对应的陀螺仪轴 */
        if (Imu_type == 1)
        {
            pid_control(&roll_balance_cascade.angular_speed_cycle,
                        roll_balance_cascade.angle_cycle.out,
                        imu660ra_gyro_x);
        }
        else if (Imu_type == 2)
        {
            pid_control(&roll_balance_cascade.angular_speed_cycle,
                        roll_balance_cascade.angle_cycle.out,
                        imu660rb_gyro_y);
        }
        else if (Imu_type == 3)
        {
            pid_control(&roll_balance_cascade.angular_speed_cycle,
                        roll_balance_cascade.angle_cycle.out,
                        imu963ra_gyro_y);
        }
    }

    /* 角度环（每5ms执行），目标角为0（保持水平） */
    if (sys_times % 5 == 0)
    {
        pid_control(&roll_balance_cascade.angle_cycle,
                    0 - roll_balance_cascade.posture_value.mechanical_zero,
                    -roll_balance_cascade.posture_value.rol);
    }

    /* 舵机输出 = 中值 + 角速度环输出 */
    Steer_set(SERVO_MOTOR_MID + roll_balance_cascade.angular_speed_cycle.out);
}


/**
 * @brief  科目一平衡调试数据显示函数（在IPS屏幕上显示关键参数）
 * @note   在主循环中调用，显示以下参数：
 *         - YAW：偏航角（度）
 *         - ROLL：横滚角（度）
 *         - Gun：遥控器通道2数据（油门/转向）
 *         - SPEED：电机速度（左轮速度数据取反）
 *         - DIS：惯导里程（米）
 */
void Balance_1_text(void)
{
    ips_show_string(8 * 0,  16 * 12, "YAW:");    ips_show_float(8 * 10, 16 * 12, roll_balance_cascade.posture_value.yaw, 3, 6);
    ips_show_string(8 * 0,  16 * 13, "ROLL:");   ips_show_float(8 * 10, 16 * 13, roll_balance_cascade.posture_value.rol, 3, 6);
    ips_show_string(8 * 0,  16 * 15, "Gun");     ips_show_float(8 * 10, 16 * 15, uart_receiver.channel[2], 3, 6);
    ips_show_string(8 * 0,  16 * 16, "SPEED:");  ips_show_float(8 * 10, 16 * 16, -motor_value.receive_left_speed_data, 3, 6);
    ips_show_string(8 * 0,  16 * 17, "DIS");     ips_show_float(8 * 10, 16 * 17, guandao_lucheng, 3, 6);
}
