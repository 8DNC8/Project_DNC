#include "imu.h"

float yaw_zero = 0;
float imu_yaw_rel = 0;

static float imu_yaw_cont = 0;       // 连续偏航角（不环绕，供限幅使用）
static float imu_yaw_raw_prev = 0;   // 上一次读取的原始 yaw

// 角度硬限幅到 [-limit_value, limit_value]（不环绕）
float angle_limit(float angle, float limit_value)
{
    if (angle >  limit_value) angle =  limit_value;
    if (angle < -limit_value) angle = -limit_value;
    return angle;
}

// 初始化 IMU 并等待自动标定，记录上电基准
void imu_init(void)
{
    imu660rc_init(IMU660RC_QUARTERNION_120HZ);
    system_delay_ms(3000);           // 等待模块内部自动标定地偏，期间保持静止
    imu_yaw_raw_prev = imu660rc_yaw;
    imu_yaw_cont   = 0;
    yaw_zero       = 0;              // 上电基准即连续角 0
}

// 读原始 yaw 并累加为连续角（处理 0/360 跨圈），返回连续偏航角
float imu_get_yaw(void)
{
    float raw   = imu660rc_yaw;
    float delta = raw - imu_yaw_raw_prev;
    if (delta >  180) delta -= 360;  // 原始 yaw 359→1 正向跨圈
    else if (delta < -180) delta += 360; // 原始 yaw 1→359 反向跨圈
    imu_yaw_cont     += delta;
    imu_yaw_raw_prev  = raw;
    return imu_yaw_cont;
}

// 相对偏航角：上电为 0，顺时针 0→+180（封顶），逆时针 0→-180（封底）
// 采用连续角 + 硬限幅，不做环绕，故不会显示 360。
float imu_get_relative_yaw(void)
{
    imu_get_yaw();                   // 先刷新连续角
    imu_yaw_rel = angle_limit(imu_yaw_cont - yaw_zero, 180.0f);
    return imu_yaw_rel;
}

// 以当前连续 yaw 重新归零
void imu_reset_yaw(void)
{
    yaw_zero = imu_yaw_cont;
}
