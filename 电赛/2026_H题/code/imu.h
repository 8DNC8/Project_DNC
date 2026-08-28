#ifndef IMU_H
#define IMU_H

#include "zf_common_headfile.h"

extern float yaw_zero;    // 上电/归零时的 yaw 基准（原始 yaw）
extern float imu_yaw_rel; // 相对偏航角（右正左负，范围 -180~180）

// 角度限幅到 [-limit_value, limit_value]
float angle_limit(float angle, float limit_value);

// 初始化 IMU（含 3s 标定延时）并记录 yaw 基准
void imu_init(void);

// 返回原始 yaw（库全局 imu660rc_yaw）
float imu_get_yaw(void);

// 返回相对偏航角：上电为 0，右转正、左转负（范围 -180~180）
float imu_get_relative_yaw(void);

// 以当前 yaw 重新归零
void imu_reset_yaw(void);

#endif
