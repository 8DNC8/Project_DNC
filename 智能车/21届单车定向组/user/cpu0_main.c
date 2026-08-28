
#include "zf_common_headfile.h"
#pragma section all "cpu0_dsram"
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU0的RAM中


// **************************** 代码区域 ****************************

/**
 * @brief  CPU0主函数（核心0入口）
 * @note   执行流程：
 *         1. 系统时钟和调试串口初始化
 *         2. 外设初始化（蜂鸣器、按键、舵机、接收机、无刷驱动、显示屏、IMU）
 *         3. 平衡PID参数初始化、惯导初始化
 *         4. 延时1秒等待外设稳定
 *         5. 初始化1ms周期中断（CCU60_CH0）
 *         6. 等待所有核心初始化完成
 *         7. 主循环：显示调试数据 → 执行控制 → 菜单处理
 */
int core0_main(void)
{
    clock_init();                   // 获取时钟频率<务必保留>
    debug_init();                   // 初始化默认调试串口

    Buzzer_init();                  // 蜂鸣器初始化
    Key_init();                     // 按键初始化
    Steer_init();                   // 舵机初始化
    uart_receiver_init();           // SBUS接收机初始化
    small_driver_uart_init();       // 无刷驱动初始化
    ips_init(IPS200_TYPE_SPI);     // 显示屏初始化
    Imu_init();                     // IMU初始化
    balance_cascade_init();         // 平衡及陀螺仪参数初始化
    guandao_Init();                 // 惯导初始化
    system_delay_ms(1000);          // 延时1秒等待外设稳定
    pit_ms_init(CCU60_CH0, 1);     // 初始化1ms周期中断

    cpu_wait_event_ready();         // 等待所有核心初始化完毕
    while (TRUE)
    {
        Balance_1_text();           // 平衡调试数据显示
        new_ctrl();                 // 遥控器控制检测
        Menu();                     // 菜单处理
    }
}


/**
 * @brief  CCU60通道0中断服务函数（1ms周期执行）
 * @note   这是系统的核心控制中断，每1ms执行一次，包含：
 *         1. 开启中断嵌套
 *         2. 清除中断标志
 *         3. 系统计时自增
 *         4. 每5ms扫描一次按键
 *         5. IMU姿态解算
 *         6. 编码器数据采集
 *         7. 惯导数据采集
 *         8. 惯导读取与复现
 *         9. 无遥控器接管时执行自动控制程序（Sub_select）
 *         10. 车头随动模式时执行Body_keep
 */
IFX_INTERRUPT(cc60_pit_ch0_isr, 0, CCU6_0_CH0_ISR_PRIORITY)
{
    interrupt_global_enable(0);     // 开启中断嵌套
    pit_clear_flag(CCU60_CH0);      // 清除中断标志

    sys_times++;                     // 系统计时自增（ms）

    if (sys_times % 5 == 0)
    {
        Key_scan();                  // 每5ms扫描一次按键
    }

    Imu_attitude_scan();             // IMU姿态解算
    QUD_encoder_pulse_get();         // 编码器数据采集
    INS_data_get();                  // 惯导数据采集
    guandao_task();                  // 惯导读取与复现

    if (CTRL_flag == 0)              // 默认无遥控器接管时执行自动程序
    {
        Sub_select(SUB_flag);
    }

    if (Mode_chage == 1)             // 车头随动模式（用于惯导推车）
    {
        Body_keep();
    }
}

#pragma section all restore
// **************************** 代码区域 ****************************
