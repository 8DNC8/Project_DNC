#ifndef ENCODER_H
#define ENCODER_H

#include "zf_common_headfile.h"

// 编码器1 定时器 / 脉冲(A相) / 方向(B相) 引脚
#define ENCODER1_TIMER  TIM_G7
#define ENCODER1_LSB    TIMG7_ENCODER1_CH1_A26
#define ENCODER1_DIR    B27

// 编码器2 定时器 / 脉冲(A相) / 方向(B相) 引脚
#define ENCODER2_TIMER  TIM_G6
#define ENCODER2_LSB    TIMG6_ENCODER1_CH1_B10
#define ENCODER2_DIR    B11

// 编码器脉冲数转厘米系数，可按实际走 50cm 偏差微调
#define PULSE_TO_CM     ( 0.03814 )

// 初始化两个编码器方向接口
void encoder_init(void);

// 读取并清零两个编码器计数值（*e0、*e1 输出，int16 与工程原数组一致）
void encoder_read(volatile int16 *e0, volatile int16 *e1);

// 累计行走距离（单位 cm），自开机起累加；[0]=左轮 [1]=右轮
// 取脉冲绝对值累加，即“走过的总里程”，与正反转方向无关
extern float enc_dist_cm[2];

// 读取某轮累计行走距离（cm）；ch: 0=左轮 1=右轮
float encoder_get_distance_cm(uint8 ch);

#endif
