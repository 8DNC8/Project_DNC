#include "uart_k230.h"

// ==================== 对外状态 ====================
volatile int16  ball_x = 0;
volatile int16  ball_y = 0;
volatile int16  ball_d10 = 0;      // K230 offset: 像素差(x-320) 或 d(cm*10)
volatile uint8  ball_cnt = 0;
volatile uint8  ball_online = 0;
volatile uint8  k230_link_ok = 0;
volatile uint32 k230_frame_cnt = 0;

// ==================== 接收缓冲（行模式，对齐学长可用方案） ====================
#define K230_RX_BUF_LEN  ( 64 )
static uint8  rx_buf[K230_RX_BUF_LEN];
static uint16 rx_len = 0;
                                                                                                                                                                                                                                                                                                                                                                                   
static volatile uint16 ball_lost_tick = 0;  // 距上一有效球帧的 20ms 计数
static volatile uint16 link_lost_tick = 0;  // 距上一任意有效帧（含心跳）的 20ms 计数

// 从一行 JSON 文本中解析 "x": / "y": / "n": 整数
// 顺序无关，兼容 "x":1 与 "x": 1（含空格），参考学长 yuntai.c 思路
static int16 parse_json_int10(uint8 *buf, uint16 len, char key, uint8 *got_key)
{
    uint16 i;
    *got_key = 0;

    for(i = 0; i + 2 < len; i++)
    {
        if(buf[i] == '"' && buf[i + 1] == key && buf[i + 2] == '"')
        {
            uint16 j = i + 3;
            int32 sign = 1;
            int32 ipart = 0;
            int32 dpart = 0;
            uint8 got_digit = 0;

            while(j < len && (buf[j] == ':' || buf[j] == ' ')) j++;
            if(j < len && buf[j] == '-') { sign = -1; j++; }

            while(j < len && buf[j] >= '0' && buf[j] <= '9')
            {
                ipart = ipart * 10 + (buf[j] - '0');
                j++;
                got_digit = 1;
            }

            if(j < len && buf[j] == '.')
            {
                j++;
                if(j < len && buf[j] >= '0' && buf[j] <= '9')
                {
                    dpart = buf[j] - '0';
                    got_digit = 1;
                }
            }

            if(got_digit)
            {
                *got_key = 1;
                return (int16)(sign * (ipart * 10 + dpart));
            }
        }
    }

    return 0;
}

// Parse K230 JSON line:
//   主格式(end(1).py): {"d":-1.2,"n":1}     ← d=偏移量cm, 1位小数
//   备用格式(untitled_2.py): {"x":30,"y":220,"n":1} ← x=像素偏移
//   无球:                  {"d":0.0,"n":0}
// 解析器兼容两种格式: 优先用 d，没有 d 就用 x
static void k230_parse_line(uint8 *buf, uint16 len)
{
    uint8 got_d = 0, got_x = 0, got_y = 0, got_n = 0;
    int16 vd10 = parse_json_int10(buf, len, 'd', &got_d);
    int16 vx10 = parse_json_int10(buf, len, 'x', &got_x);
    int16 vy10 = parse_json_int10(buf, len, 'y', &got_y);
    int16 vn10 = parse_json_int10(buf, len, 'n', &got_n);
    uint8 vn = (uint8)(vn10 / 10);

    // 优先用 d（cm偏移，parse_json_int10 返回 值*10），没有 d 就用 x
    if(got_d)
    {
        ball_d10 = vd10;                          // d 格式：直接取 0.1cm 偏移量
    }
    else if(got_x)
    {
        // x 格式：K230 发的 x 已经是偏移量(best_cx-CENTER_X)
        ball_d10 = (int16)(vx10 / 10);             // 像素偏移，右正左负
    }

    if(got_y)
    {
        ball_y = (int16)(vy10 / 10);              // y 像素坐标
    }

    // 只要有位置数据(d或x) + 球数(n) 就算有效帧
    if((got_d || got_x) && got_n)
    {
        k230_frame_cnt++;
        k230_link_ok = 1;
        link_lost_tick = 0;

        ball_cnt = vn;

        // ball_x 用于显示，直接复用偏移量
        if(got_x)
        {
            ball_x = (int16)(vx10 / 10);
        }
        else
        {
            ball_x = (int16)(K230_IMG_W / 2 + ball_d10);
        }
        if(!got_y)
        {
            ball_y = K230_IMG_H / 2;
        }

        if(ball_cnt > 0)
        {
            ball_online = 1;
            ball_lost_tick = 0;
        }
    }
}

// 串口接收中断回调：逐字节累积到 '\n' 完成一行解析
// 注：UART2 中断已在 user/isr.c 的 UART2_IRQHandler 中统一清中断，此处无需再清
static void k230_uart_callback(uint32 event, void *ptr)
{
    uint8 dat;
    if(0 == (event & UART_INTERRUPT_STATE_RX)) return;

    while(uart_query_byte(K230_UART, &dat))
    {
        if(dat == '\n' || rx_len >= (K230_RX_BUF_LEN - 1))
        {
            rx_buf[rx_len] = '\0';
            k230_parse_line(rx_buf, rx_len);
            rx_len = 0;
        }
        else
        {
            rx_buf[rx_len++] = dat;
        }
    }
}

void k230_init(void)
{
    uart_init(K230_UART, K230_UART_BAUD, K230_UART_TX, K230_UART_RX);
    uart_set_callback(K230_UART, k230_uart_callback, NULL);
    uart_set_interrupt_config(K230_UART, UART_INTERRUPT_CONFIG_RX_ENABLE);
}

// 20ms 节拍调用：有效球帧 / 链路超时判定
void k230_tick_20ms(void)
{
    if(ball_lost_tick < 0xFFFF) ball_lost_tick++;
    if(ball_lost_tick > K230_LOST_TICK)
    {
        ball_online = 0;
        ball_cnt = 0;
    }

    if(link_lost_tick < 0xFFFF) link_lost_tick++;
    if(link_lost_tick > K230_LOST_TICK)
    {
        k230_link_ok = 0;          // 1s 没有任何有效帧 = 通信断开
    }
}
