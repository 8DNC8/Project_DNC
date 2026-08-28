#ifndef BRUSHLESS_DRIVER_H_
#define BRUSHLESS_DRIVER_H_

#include "hal_data.h"

#define BRUSHLESS_DRIVER_BAUDRATE     (460800U)
#define BRUSHLESS_DRIVER_TX_PIN       (BSP_IO_PORT_07_PIN_14)   /* P714 / P6-6 / SCI4_TXD4 -> driver RX */
#define BRUSHLESS_DRIVER_RX_PIN       (BSP_IO_PORT_07_PIN_15)   /* P715 / P6-4 / SCI4_RXD4 -> driver TX */

typedef struct
{
    int16_t left_speed;
    int16_t right_speed;
    uint32_t rx_byte_count;
    uint32_t rx_frame_count;
    uint32_t rx_error_count;
    uint32_t tx_frame_count;
    fsp_err_t last_tx_error;
    uint8_t last_rx_byte;
} brushless_driver_status_t;

fsp_err_t brushless_driver_init(void);
fsp_err_t brushless_driver_set_duty(int16_t left_duty, int16_t right_duty);
fsp_err_t brushless_driver_request_speed(void);
void brushless_driver_process(void);
void brushless_driver_get_status(brushless_driver_status_t * p_status);
void brushless_driver_uart_callback(uart_callback_args_t * p_args);

#endif /* BRUSHLESS_DRIVER_H_ */
