#include "beep.h"

uint8 beep_flag = 0;
int16 beep_cnt  = 0;

// 初始化蜂鸣器引脚为推挽输出
void beep_init(void)
{
    gpio_init(BEEP_PIN, GPO, 0, GPO_PUSH_PULL);
}

// 打开蜂鸣器并置位工作标志
void beep_on(void)
{
    gpio_high(BEEP_PIN);
    beep_flag = 1;
}

// 关闭蜂鸣器
void beep_off(void)
{
    gpio_low(BEEP_PIN);
}

// 每 20ms 调用一次，蜂鸣 0.1s 后自动关闭
void beep_tick(void)
{
    if(beep_flag)
    {
        beep_cnt++;
        if(beep_cnt >= 5)        // 5 * 20ms = 100ms = 0.1s
        {
            beep_flag = 0;
            beep_cnt  = 0;
            beep_off();
        }
    }
}
