#ifndef IMU_PROCESS_H
#define IMU_PROCESS_H

#include "zf_common_headfile.h"

#define IMU_PROCESS_DEFAULT_CYCLE_S (0.005f)

typedef struct
{
    float rot_mat[3][3];
} imu_quaternion_data_t;

typedef struct
{
    float qua[4];
    float acc_filtered[3];
} imu_quaternion_process_t;

typedef struct
{
    float acc_err[3];
} imu_quaternion_parameter_t;

typedef struct
{
    imu_quaternion_process_t pro;
    imu_quaternion_data_t data;
    imu_quaternion_parameter_t parameter;
} imu_quaternion_module_t;

typedef struct
{
    float p;
    float i;
    float d;
    float p_value_last;
    float i_value;
    float i_value_pro;
    float i_value_max;
    float out;
    float out_max;
    float incremental_data[2];
} imu_pid_cycle_t;

typedef struct
{
    float correct_kp;
    float correct_ki;
    float call_cycle;
    float mechanical_zero;
    float yaw;
    float rol;
    float pit;
} imu_posture_value_t;

typedef struct
{
    imu_quaternion_module_t quaternion;
    imu_posture_value_t posture_value;
    imu_pid_cycle_t angular_speed_cycle;
    imu_pid_cycle_t angle_cycle;
    imu_pid_cycle_t speed_cycle;
    imu_pid_cycle_t track_cycle;
} imu_cascade_t;

extern imu_cascade_t imu_roll_balance;
extern imu_cascade_t imu_roll_balance_resave;
extern imu_cascade_t imu_pitch_balance;
extern imu_cascade_t imu_pitch_balance_resave;
extern imu_cascade_t imu_track;

void imu_process_init(float call_cycle_s);
void imu_process_update(float call_cycle_s);
void imu_quaternion_module_init(imu_cascade_t *cascade_value);
void imu_quaternion_module_calculate(imu_cascade_t *cascade_value);
void imu_pid_control(imu_pid_cycle_t *pid_cycle, float target, float real);
void imu_pid_control_incremental(imu_pid_cycle_t *pid_cycle, float target, float real);

#endif
