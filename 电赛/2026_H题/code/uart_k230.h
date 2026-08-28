#ifndef UART_K230_H
#define UART_K230_H

#include "zf_common_headfile.h"

// ==================== 硬件与协议配置 ====================
#define K230_UART        ( UART_2 )         // K230 使用的串口号（与学长可用工程一致）
#define K230_UART_BAUD   ( 38400 )          // 波特率：对齐 K230 端 end(1).py YbUart(38400)
#define K230_UART_TX     ( UART2_TX_B15 )   // 单片机 TX = PB15 -> K230 GPIO12(RX2)
#define K230_UART_RX     ( UART2_RX_B16 )   // 单片机 RX = PB16 <- K230 GPIO11(TX2)

// K230 检测坐标系（rgb888p），与 detect_ball_and_move.py 中 rgb888p_size 一致
#define K230_IMG_W       ( 640 )            // 画面宽，中心 = 320
#define K230_IMG_H       ( 480 )            // 画面高，越靠下越近

#define K230_LOST_TICK   ( 50 )             // 超时：50 个 20ms = 1s 没收到有效球帧判定丢失

// 协议：ASCII JSON 行，以 '\n' 结尾
// 主格式（K230 end(1).py）：{"d":-1.2,"n":1}
//   d = 球心偏移量(cm)，右正左负，1位小数
//   n = 检测到的球数量（0 = 无球，> 0 = 有球）
// 备用格式（untitled_2.py）：{"x":30,"y":220,"n":1}
//   x = 球心像素偏移(已减中心)，y = y坐标
// 注：K230 端 end(1).py 用 YbUart(baudrate=38400) 发送

// ==================== 对外状态（volatile：串口中断中更新） ====================
extern volatile int16  ball_x;         // 最近的球 球心 x（0~639，画面宽640，中心320）
extern volatile int16  ball_y;
extern volatile int16  ball_d10;      // K230 offset d, 0.1cm units, right positive
extern volatile uint8  ball_cnt;       // K230 当前看到的球数量
extern volatile uint8  ball_online;    // 1 = 当前有球目标（收到 cnt>0 帧且未超时）
extern volatile uint8  k230_link_ok;   // 1 = 串口通信正常（1s 内收到过任意有效帧，含心跳）
extern volatile uint32 k230_frame_cnt; // 累计收到的有效帧数（调试用）

// ==================== 接口 ====================
void k230_init(void);          // 初始化串口 + 中断（main 中调用一次）
void k230_tick_20ms(void);     // 超时管理（在 pit_callback 20ms 节拍中调用）

#endif
