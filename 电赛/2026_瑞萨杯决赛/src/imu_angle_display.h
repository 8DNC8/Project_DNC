#ifndef IMU_ANGLE_DISPLAY_H_
#define IMU_ANGLE_DISPLAY_H_

#include "zf_common_headfile.h"

#define IMU_ANGLE_DISPLAY_INTERVAL_TICKS    (50U)

void imu_angle_display_init(void);
void imu_angle_display_update(float yaw, float pitch, float roll);
void imu_angle_display_force_refresh(void);
void imu_angle_display_update_balance(void);

#endif /* IMU_ANGLE_DISPLAY_H_ */
