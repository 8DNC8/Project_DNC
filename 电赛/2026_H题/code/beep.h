#ifndef BEEP_H
#define BEEP_H

#include "zf_common_headfile.h"

// 蜂鸣器引脚
#define BEEP_PIN    A18

extern uint8 beep_flag;     // 蜂鸣器工作标志位
extern int16 beep_cnt;      // 蜂鸣器计时

// 初始化蜂鸣器引脚
void beep_init(void);

// 打开蜂鸣器（置位标志，由 beep_tick 在 0.5s 后自动关闭）
void beep_on(void);

// 立即关闭蜂鸣器
void beep_off(void);

// 在 20ms 控制节拍中调用，累计 0.5s 后自动关闭蜂鸣器
void beep_tick(void);

#endif
