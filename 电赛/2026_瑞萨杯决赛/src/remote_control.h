#ifndef REMOTE_CONTROL_H_
#define REMOTE_CONTROL_H_

#include "hal_data.h"

#define UART_RECEIVER_BAUDRATE          (100000U)
#define UART_RECEIVER_RX_PIN            (BSP_IO_PORT_09_PIN_09)
#define UART_RECEIVER_CHANNEL_NUM       (6U)

#define UART_RECEIVER_FRAME_LEN         (25U)
#define UART_RECEIVER_FRAME_START       (0x0FU)
#define UART_RECEIVER_FRAME_END         (0x00U)
#define UART_RECEIVER_FRAME_LOST        (0x04U)
#define UART_RECEIVER_FAILSAFE          (0x08U)

#define UART_RECEIVER_CH1_INDEX         (0U)
#define UART_RECEIVER_CH2_INDEX         (1U)
#define UART_RECEIVER_CH3_INDEX         (2U)
#define UART_RECEIVER_INPUT_MIN         (172)
#define UART_RECEIVER_INPUT_MAX         (1811)
#define UART_RECEIVER_RUN_THRESHOLD     ((UART_RECEIVER_INPUT_MIN + UART_RECEIVER_INPUT_MAX) / 2)
#define UART_RECEIVER_STEERING_DEAD_ZONE (20)
#define UART_RECEIVER_THROTTLE_DEAD_ZONE (40)
#define UART_RECEIVER_OUTPUT_MAX        (1000)
#define UART_RECEIVER_CALIBRATION_FRAMES (50U)
#define UART_RECEIVER_STEERING_REVERSE  (0)
#define UART_RECEIVER_THROTTLE_REVERSE  (0)

typedef struct
{
    uint16_t channel[UART_RECEIVER_CHANNEL_NUM];
    int16_t  steering;
    int16_t  throttle;
    uint16_t steering_center;
    uint16_t throttle_center;
    uint8_t  calibration_ready;
    uint8_t  state;
    uint8_t  run_enabled;
    uint8_t  finsh_flag;
    uint8_t  last_rx_byte;
    uint32_t frame_count;
    uint32_t error_count;
    uint32_t rx_byte_count;
    uint32_t callback_count;
    uint32_t break_count;
    uint32_t pin_low_count;
} uart_receiver_struct;

extern volatile uart_receiver_struct uart_receiver;

fsp_err_t uart_receiver_init(void);
void uart_receiver_process(void);
void uart_receiver_callback(uart_callback_args_t * p_args);
void uart_receiver_set_enabled(bool enabled);   /* 璇煶"閬ユ帶妯″紡"浣胯兘锛屽叾瀹冩寚浠ょ鐢?*/
bool uart_receiver_is_enabled(void);            /* 鏌ヨ褰撳墠鏄惁鍏佽閬ユ帶鍣ㄨ緭鍑烘帶鍒堕噺 */

#endif
