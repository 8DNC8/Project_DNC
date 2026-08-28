#include "imu_process.h"

#include <math.h>

#define IMU_GYRO_X_RAW            (imu660rb_gyro_x)
#define IMU_GYRO_Y_RAW            (-imu660rb_gyro_y)
#define IMU_GYRO_Z_RAW            (-imu660rb_gyro_z)
#define IMU_GYRO_FACTOR           (14.3f)

#define IMU_ACC_X_RAW             (imu660rb_acc_x)
#define IMU_ACC_Y_RAW             (-imu660rb_acc_y)
#define IMU_ACC_Z_RAW             (-imu660rb_acc_z)
#define IMU_ACC_FACTOR            (4098.0f)

#define IMU_DEG_TO_RAD            (0.01745329252f)
#define IMU_RAD_TO_DEG            (57.29577951f)

imu_cascade_t imu_roll_balance;
imu_cascade_t imu_roll_balance_resave;
imu_cascade_t imu_pitch_balance;
imu_cascade_t imu_pitch_balance_resave;
imu_cascade_t imu_track;

static float imu_limit_float(float value, float min, float max)
{
    return (value < min) ? min : ((value > max) ? max : value);
}

static void imu_acc_lowpass_filter(float raw_x,
                                   float raw_y,
                                   float raw_z,
                                   float *filtered_x,
                                   float *filtered_y,
                                   float *filtered_z,
                                   float alpha)
{
    *filtered_x = alpha * *filtered_x + (1.0f - alpha) * raw_x;
    *filtered_y = alpha * *filtered_y + (1.0f - alpha) * raw_y;
    *filtered_z = alpha * *filtered_z + (1.0f - alpha) * raw_z;
}

static void imu_acc_normalize(float *ax, float *ay, float *az)
{
    float norm = sqrtf((*ax * *ax) + (*ay * *ay) + (*az * *az));

    if(norm < 0.1f)
    {
        *ax = 0.0f;
        *ay = 0.0f;
        *az = 1.0f;
    }
    else
    {
        *ax /= norm;
        *ay /= norm;
        *az /= norm;
    }
}

static bool imu_is_static_state(float ax_g, float ay_g, float az_g)
{
    float norm = sqrtf((ax_g * ax_g) + (ay_g * ay_g) + (az_g * az_g));

    return (norm >= 0.9f && norm <= 1.1f);
}

void imu_quaternion_module_init(imu_cascade_t *cascade_value)
{
    cascade_value->quaternion.pro.qua[0] = 1.0f;
    cascade_value->quaternion.pro.qua[1] = 0.0f;
    cascade_value->quaternion.pro.qua[2] = 0.0f;
    cascade_value->quaternion.pro.qua[3] = 0.0f;

    cascade_value->posture_value.yaw = 0.0f;
    cascade_value->posture_value.rol = 0.0f;
    cascade_value->posture_value.pit = 0.0f;

    cascade_value->quaternion.pro.acc_filtered[0] = (float)IMU_ACC_X_RAW / IMU_ACC_FACTOR;
    cascade_value->quaternion.pro.acc_filtered[1] = (float)IMU_ACC_Y_RAW / IMU_ACC_FACTOR;
    cascade_value->quaternion.pro.acc_filtered[2] = (float)IMU_ACC_Z_RAW / IMU_ACC_FACTOR;

    cascade_value->quaternion.parameter.acc_err[0] = 0.0f;
    cascade_value->quaternion.parameter.acc_err[1] = 0.0f;
    cascade_value->quaternion.parameter.acc_err[2] = 0.0f;
}

void imu_process_init(float call_cycle_s)
{
    memset(&imu_roll_balance, 0, sizeof(imu_roll_balance));
    memset(&imu_roll_balance_resave, 0, sizeof(imu_roll_balance_resave));
    memset(&imu_pitch_balance, 0, sizeof(imu_pitch_balance));
    memset(&imu_pitch_balance_resave, 0, sizeof(imu_pitch_balance_resave));
    memset(&imu_track, 0, sizeof(imu_track));

    imu_roll_balance.posture_value.call_cycle = call_cycle_s;
    imu_roll_balance.posture_value.mechanical_zero = 0.11f;
    imu_roll_balance.posture_value.correct_kp = 0.4f;
    imu_roll_balance.posture_value.correct_ki = 0.015f;

    imu_roll_balance.angular_speed_cycle.i_value_max = 1000.0f;
    imu_roll_balance.angular_speed_cycle.i_value_pro = 0.1f;
    imu_roll_balance.angular_speed_cycle.out_max = 10000.0f;
    imu_roll_balance.angle_cycle.i_value_max = 1000.0f;
    imu_roll_balance.angle_cycle.i_value_pro = 2.0f;
    imu_roll_balance.angle_cycle.out_max = 10000.0f;
    imu_roll_balance.speed_cycle.i_value_max = 500.0f;
    imu_roll_balance.speed_cycle.i_value_pro = 0.005f;
    imu_roll_balance.speed_cycle.out_max = 2000.0f;   /* 速度环输出接舵机(腿)，对齐 CYT4BB7 */

    imu_roll_balance.angular_speed_cycle.p = 0.85f;
    imu_roll_balance.angle_cycle.p = 600.0f;
    imu_roll_balance.angle_cycle.i = 1.0f;   /* 角度环 I 项：原地保持/抗漂移，和 CYT4BB7 一致 */
    imu_roll_balance.angle_cycle.d = 55.0f;
    imu_roll_balance.speed_cycle.p = 5.0f;     /* 速度环 P（对齐 CYT4BB7） */
    imu_roll_balance.speed_cycle.i = 0.0f;
    imu_roll_balance.speed_cycle.d = 0.0f;

    imu_pitch_balance.posture_value.call_cycle = call_cycle_s;
    imu_pitch_balance.posture_value.mechanical_zero = 0.0f;
    imu_pitch_balance.posture_value.correct_kp = 0.4f;
    imu_pitch_balance.posture_value.correct_ki = 0.015f;

    imu_pitch_balance.angular_speed_cycle.i_value_max = 1000.0f;
    imu_pitch_balance.angular_speed_cycle.i_value_pro = 0.3f;
    imu_pitch_balance.angular_speed_cycle.out_max = 10000.0f;
    imu_pitch_balance.angle_cycle.i_value_max = 300.0f;
    imu_pitch_balance.angle_cycle.i_value_pro = 0.8f;
    imu_pitch_balance.angle_cycle.out_max = 300.0f;
    imu_pitch_balance.speed_cycle.i_value_max = 4000.0f;
    imu_pitch_balance.speed_cycle.i_value_pro = 0.05f;
    imu_pitch_balance.speed_cycle.out_max = 1500.0f;
    imu_pitch_balance.angle_cycle.i = 1.0f;

    imu_track.track_cycle.p = 10.0f;

    memcpy(&imu_roll_balance_resave, &imu_roll_balance, sizeof(imu_roll_balance_resave));
    memcpy(&imu_pitch_balance_resave, &imu_pitch_balance, sizeof(imu_pitch_balance_resave));

    imu_quaternion_module_init(&imu_roll_balance);
    imu_quaternion_module_init(&imu_pitch_balance);
}

void imu_process_update(float call_cycle_s)
{
    imu660rb_get_gyro();
    imu660rb_get_acc();

    /* 用主循环实测周期替换固定 0.005s：摄像头模式刷图会拉长循环，
     * 固定步长会让陀螺积分“多转”导致角度虚增、平衡崩溃。 */
    if((call_cycle_s <= 0.0f) || (call_cycle_s > 0.25f))
    {
        call_cycle_s = IMU_PROCESS_DEFAULT_CYCLE_S;
    }
    imu_roll_balance.posture_value.call_cycle = call_cycle_s;

    imu_quaternion_module_calculate(&imu_roll_balance);

    imu_pitch_balance.posture_value.yaw = imu_roll_balance.posture_value.yaw;
    imu_pitch_balance.posture_value.rol = imu_roll_balance.posture_value.rol;
    imu_pitch_balance.posture_value.pit = imu_roll_balance.posture_value.pit;
}

void imu_quaternion_module_calculate(imu_cascade_t *cascade_value)
{
    static float first_count_time = 0.0f;
    float x = ((float) IMU_GYRO_X_RAW / IMU_GYRO_FACTOR) * IMU_DEG_TO_RAD;
    float y = ((float) IMU_GYRO_Y_RAW / IMU_GYRO_FACTOR) * IMU_DEG_TO_RAD;
    float z = ((float) IMU_GYRO_Z_RAW / IMU_GYRO_FACTOR) * IMU_DEG_TO_RAD;

    float ax_g = (float)IMU_ACC_X_RAW / IMU_ACC_FACTOR;
    float ay_g = (float)IMU_ACC_Y_RAW / IMU_ACC_FACTOR;
    float az_g = (float)IMU_ACC_Z_RAW / IMU_ACC_FACTOR;
    bool static_state = imu_is_static_state(ax_g, ay_g, az_g);
    float acc_alpha = static_state ? 0.8f : 0.5f;

    imu_acc_lowpass_filter(ax_g, ay_g, az_g,
                           &cascade_value->quaternion.pro.acc_filtered[0],
                           &cascade_value->quaternion.pro.acc_filtered[1],
                           &cascade_value->quaternion.pro.acc_filtered[2],
                           acc_alpha);

    float ax = cascade_value->quaternion.pro.acc_filtered[0];
    float ay = cascade_value->quaternion.pro.acc_filtered[1];
    float az = cascade_value->quaternion.pro.acc_filtered[2];

    imu_acc_normalize(&ax, &ay, &az);

    float q0 = cascade_value->quaternion.pro.qua[0];
    float q1 = cascade_value->quaternion.pro.qua[1];
    float q2 = cascade_value->quaternion.pro.qua[2];
    float q3 = cascade_value->quaternion.pro.qua[3];

    float gx = 2.0f * ((q1 * q3) - (q0 * q2));
    float gy = 2.0f * ((q0 * q1) + (q2 * q3));
    float gz = (q0 * q0) - (q1 * q1) - (q2 * q2) + (q3 * q3);

    float ex = (ay * gz) - (az * gy);
    float ey = (az * gx) - (ax * gz);
    float ez = (ax * gy) - (ay * gx);

    float kp = static_state ? cascade_value->posture_value.correct_kp : (cascade_value->posture_value.correct_kp * 0.8f);
    float ki = cascade_value->posture_value.correct_ki;

    if(first_count_time < 0.5f)
    {
        first_count_time += cascade_value->posture_value.call_cycle;
        kp = 100.0f;
    }

    float integral_gain = static_state ? 1.0f : 0.1f;
    cascade_value->quaternion.parameter.acc_err[0] += (ex * cascade_value->posture_value.call_cycle) * integral_gain;
    cascade_value->quaternion.parameter.acc_err[1] += (ey * cascade_value->posture_value.call_cycle) * integral_gain;
    cascade_value->quaternion.parameter.acc_err[2] += (ez * cascade_value->posture_value.call_cycle) * integral_gain;

    cascade_value->quaternion.parameter.acc_err[0] = imu_limit_float(cascade_value->quaternion.parameter.acc_err[0], -1.0f, 1.0f);
    cascade_value->quaternion.parameter.acc_err[1] = imu_limit_float(cascade_value->quaternion.parameter.acc_err[1], -1.0f, 1.0f);
    cascade_value->quaternion.parameter.acc_err[2] = imu_limit_float(cascade_value->quaternion.parameter.acc_err[2], -1.0f, 1.0f);

    x += (kp * ex) + (ki * cascade_value->quaternion.parameter.acc_err[0]);
    y += (kp * ey) + (ki * cascade_value->quaternion.parameter.acc_err[1]);
    z += (kp * ez) + (ki * cascade_value->quaternion.parameter.acc_err[2]);

    cascade_value->quaternion.pro.qua[0] += ((-q1 * x - q2 * y - q3 * z) * cascade_value->posture_value.call_cycle * 0.5f);
    cascade_value->quaternion.pro.qua[1] += (( q0 * x + q2 * z - q3 * y) * cascade_value->posture_value.call_cycle * 0.5f);
    cascade_value->quaternion.pro.qua[2] += (( q0 * y - q1 * z + q3 * x) * cascade_value->posture_value.call_cycle * 0.5f);
    cascade_value->quaternion.pro.qua[3] += (( q0 * z + q1 * y - q2 * x) * cascade_value->posture_value.call_cycle * 0.5f);

    q0 = cascade_value->quaternion.pro.qua[0];
    q1 = cascade_value->quaternion.pro.qua[1];
    q2 = cascade_value->quaternion.pro.qua[2];
    q3 = cascade_value->quaternion.pro.qua[3];

    float length = sqrtf((q0 * q0) + (q1 * q1) + (q2 * q2) + (q3 * q3));
    if(length > 0.001f)
    {
        q0 /= length;
        q1 /= length;
        q2 /= length;
        q3 /= length;
        cascade_value->quaternion.pro.qua[0] = q0;
        cascade_value->quaternion.pro.qua[1] = q1;
        cascade_value->quaternion.pro.qua[2] = q2;
        cascade_value->quaternion.pro.qua[3] = q3;
    }

    float q0_2 = q0 * q0;
    float q1_2 = q1 * q1;
    float q2_2 = q2 * q2;
    float q3_2 = q3 * q3;

    cascade_value->quaternion.data.rot_mat[0][0] = q0_2 + q1_2 - q2_2 - q3_2;
    cascade_value->quaternion.data.rot_mat[0][1] = 2.0f * ((q1 * q2) + (q0 * q3));
    cascade_value->quaternion.data.rot_mat[0][2] = 2.0f * ((q1 * q3) - (q0 * q2));
    cascade_value->quaternion.data.rot_mat[1][0] = 2.0f * ((q1 * q2) - (q0 * q3));
    cascade_value->quaternion.data.rot_mat[1][1] = q0_2 - q1_2 + q2_2 - q3_2;
    cascade_value->quaternion.data.rot_mat[1][2] = 2.0f * ((q2 * q3) + (q0 * q1));
    cascade_value->quaternion.data.rot_mat[2][0] = 2.0f * ((q1 * q3) + (q0 * q2));
    cascade_value->quaternion.data.rot_mat[2][1] = 2.0f * ((q2 * q3) - (q0 * q1));
    cascade_value->quaternion.data.rot_mat[2][2] = q0_2 - q1_2 - q2_2 + q3_2;

    cascade_value->posture_value.rol = atan2f(cascade_value->quaternion.data.rot_mat[1][2],
                                              cascade_value->quaternion.data.rot_mat[2][2]) * IMU_RAD_TO_DEG;
    cascade_value->posture_value.pit = -asinf(imu_limit_float(cascade_value->quaternion.data.rot_mat[0][2], -1.0f, 1.0f)) * IMU_RAD_TO_DEG;
    cascade_value->posture_value.yaw = atan2f(cascade_value->quaternion.data.rot_mat[0][1],
                                              cascade_value->quaternion.data.rot_mat[0][0]) * IMU_RAD_TO_DEG;
}

void imu_pid_control(imu_pid_cycle_t *pid_cycle, float target, float real)
{
    float proportion_value = target - real;
    float differential_value = proportion_value - pid_cycle->p_value_last;

    pid_cycle->i_value += proportion_value * pid_cycle->i_value_pro;
    pid_cycle->i_value = imu_limit_float(pid_cycle->i_value, -pid_cycle->i_value_max, pid_cycle->i_value_max);
    pid_cycle->out = (pid_cycle->p * proportion_value) + (pid_cycle->i * pid_cycle->i_value) + (pid_cycle->d * differential_value);
    pid_cycle->out = imu_limit_float(pid_cycle->out, -pid_cycle->out_max, pid_cycle->out_max);
    pid_cycle->p_value_last = proportion_value;
}

void imu_pid_control_incremental(imu_pid_cycle_t *pid_cycle, float target, float real)
{
    float error = target - real;
    float proportion_value = error - pid_cycle->incremental_data[0];
    float differential_value = error - (2.0f * pid_cycle->incremental_data[0]) + pid_cycle->incremental_data[1];

    pid_cycle->out += (pid_cycle->p * proportion_value) + (pid_cycle->i * error) + (pid_cycle->d * differential_value);
    pid_cycle->out = imu_limit_float(pid_cycle->out, -pid_cycle->out_max, pid_cycle->out_max);
    pid_cycle->incremental_data[1] = pid_cycle->incremental_data[0];
    pid_cycle->incremental_data[0] = error;
    pid_cycle->i_value = error;
}
