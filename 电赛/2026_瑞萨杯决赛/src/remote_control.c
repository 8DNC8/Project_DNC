#include "remote_control.h"

#include "zf_common_headfile.h"
#include <string.h>

#define UART_RECEIVER_RX_BUFFER_SIZE    (128U)
#define UART_RECEIVER_TIMEOUT_TICKS     (20U)
#define UART_RECEIVER_PRINT_TICKS       (5U)
#define UART_RECEIVER_DIAG_TICKS        (200U)

extern sci_b_baud_setting_t g_uart6_baud_setting;

volatile uart_receiver_struct uart_receiver;

static volatile uint8_t g_receiver_rx_buffer[UART_RECEIVER_RX_BUFFER_SIZE];
static volatile uint8_t g_receiver_rx_head;
static volatile uint8_t g_receiver_rx_tail;
static uint8_t g_receiver_frame[UART_RECEIVER_FRAME_LEN];
static uint8_t g_receiver_frame_length;
static uint32_t g_receiver_process_tick;
static uint32_t g_receiver_last_frame_tick;
static uint32_t g_receiver_last_print_tick;
static uint32_t g_receiver_last_diag_tick;
static uint8_t g_receiver_last_state;
static bool g_receiver_opened;
static bool g_receiver_control_enabled = false;  /* 榛樿绂佺敤锛岄渶璇煶"閬ユ帶妯″紡"浣胯兘 */
static uint32_t g_receiver_steering_sum;
static uint32_t g_receiver_throttle_sum;
static uint16_t g_receiver_calibration_count;

static void uart_receiver_ring_reset(void)
{
    g_receiver_rx_head = 0U;
    g_receiver_rx_tail = 0U;
}

static void uart_receiver_ring_push(uint8_t data)
{
    uint8_t next_head = (uint8_t)((g_receiver_rx_head + 1U) % UART_RECEIVER_RX_BUFFER_SIZE);

    if(next_head == g_receiver_rx_tail)
    {
        uart_receiver.error_count++;
        return;
    }

    g_receiver_rx_buffer[g_receiver_rx_head] = data;
    g_receiver_rx_head = next_head;
}

static bool uart_receiver_ring_pop(uint8_t * data)
{
    if(g_receiver_rx_tail == g_receiver_rx_head)
    {
        return false;
    }

    *data = g_receiver_rx_buffer[g_receiver_rx_tail];
    g_receiver_rx_tail = (uint8_t)((g_receiver_rx_tail + 1U) % UART_RECEIVER_RX_BUFFER_SIZE);
    return true;
}

static int16_t uart_receiver_map_channel(uint16_t raw, uint16_t center, int32_t dead_zone)
{
    int32_t input = (int32_t)raw;
    int32_t input_center = (int32_t)center;
    int32_t center_min = UART_RECEIVER_INPUT_MIN + dead_zone + 1;
    int32_t center_max = UART_RECEIVER_INPUT_MAX - dead_zone - 1;

    if(input_center < center_min)
    {
        input_center = center_min;
    }
    else if(input_center > center_max)
    {
        input_center = center_max;
    }

    int32_t dead_low = input_center - dead_zone;
    int32_t dead_high = input_center + dead_zone;

    if(input < UART_RECEIVER_INPUT_MIN)
    {
        input = UART_RECEIVER_INPUT_MIN;
    }
    else if(input > UART_RECEIVER_INPUT_MAX)
    {
        input = UART_RECEIVER_INPUT_MAX;
    }

    if(input < dead_low)
    {
        int32_t output = ((dead_low - input) * UART_RECEIVER_OUTPUT_MAX) /
                         (dead_low - UART_RECEIVER_INPUT_MIN);
        return (int16_t)-output;
    }

    if(input > dead_high)
    {
        int32_t output = ((input - dead_high) * UART_RECEIVER_OUTPUT_MAX) /
                         (UART_RECEIVER_INPUT_MAX - dead_high);
        return (int16_t)output;
    }

    return 0;
}

static void uart_receiver_update_control(void)
{
    uart_receiver.run_enabled = ((0U != uart_receiver.state) &&
                                 (uart_receiver.channel[UART_RECEIVER_CH3_INDEX] >
                                  UART_RECEIVER_RUN_THRESHOLD)) ? 1U : 0U;

    if((0U == uart_receiver.state) || (0U == uart_receiver.calibration_ready) ||
       (0U == uart_receiver.run_enabled) || !g_receiver_control_enabled)
    {
        uart_receiver.steering = 0;
        uart_receiver.throttle = 0;
        return;
    }

    int16_t steering = uart_receiver_map_channel(uart_receiver.channel[UART_RECEIVER_CH1_INDEX],
                                                 uart_receiver.steering_center,
                                                 UART_RECEIVER_STEERING_DEAD_ZONE);
    int16_t throttle = uart_receiver_map_channel(uart_receiver.channel[UART_RECEIVER_CH2_INDEX],
                                                 uart_receiver.throttle_center,
                                                 UART_RECEIVER_THROTTLE_DEAD_ZONE);

#if UART_RECEIVER_STEERING_REVERSE
    steering = (int16_t)-steering;
#endif
#if UART_RECEIVER_THROTTLE_REVERSE
    throttle = (int16_t)-throttle;
#endif

    uart_receiver.steering = steering;
    uart_receiver.throttle = throttle;
}

static void uart_receiver_analysis(uint8_t const * buffer)
{
    uart_receiver.channel[0] = (uint16_t)(((uint16_t)buffer[1] |
                                           ((uint16_t)buffer[2] << 8)) & 0x07FFU);
    uart_receiver.channel[1] = (uint16_t)((((uint16_t)buffer[2] >> 3) |
                                           ((uint16_t)buffer[3] << 5)) & 0x07FFU);
    uart_receiver.channel[2] = (uint16_t)((((uint16_t)buffer[3] >> 6) |
                                           ((uint16_t)buffer[4] << 2) |
                                           ((uint16_t)buffer[5] << 10)) & 0x07FFU);
    uart_receiver.channel[3] = (uint16_t)((((uint16_t)buffer[5] >> 1) |
                                           ((uint16_t)buffer[6] << 7)) & 0x07FFU);
    uart_receiver.channel[4] = (uint16_t)((((uint16_t)buffer[6] >> 4) |
                                           ((uint16_t)buffer[7] << 4)) & 0x07FFU);
    uart_receiver.channel[5] = (uint16_t)((((uint16_t)buffer[7] >> 7) |
                                           ((uint16_t)buffer[8] << 1) |
                                           ((uint16_t)buffer[9] << 9)) & 0x07FFU);

    uint8_t abnormal = (uint8_t)(UART_RECEIVER_FRAME_LOST | UART_RECEIVER_FAILSAFE);
    uart_receiver.state = ((buffer[23] & abnormal) == 0U) ? 1U : 0U;

    if((0U == uart_receiver.state) && (0U == uart_receiver.calibration_ready))
    {
        g_receiver_steering_sum = 0U;
        g_receiver_throttle_sum = 0U;
        g_receiver_calibration_count = 0U;
    }

    if((0U != uart_receiver.state) && (0U == uart_receiver.calibration_ready))
    {
        g_receiver_steering_sum += uart_receiver.channel[UART_RECEIVER_CH1_INDEX];
        g_receiver_throttle_sum += uart_receiver.channel[UART_RECEIVER_CH2_INDEX];
        g_receiver_calibration_count++;

        if(g_receiver_calibration_count >= UART_RECEIVER_CALIBRATION_FRAMES)
        {
            uart_receiver.steering_center = (uint16_t)(g_receiver_steering_sum /
                                                       UART_RECEIVER_CALIBRATION_FRAMES);
            uart_receiver.throttle_center = (uint16_t)(g_receiver_throttle_sum /
                                                       UART_RECEIVER_CALIBRATION_FRAMES);
            uart_receiver.calibration_ready = 1U;
            printf("REMOTE calibrated center CH1:%u CH2:%u\r\n",
                   (unsigned)uart_receiver.steering_center,
                   (unsigned)uart_receiver.throttle_center);
        }
    }

    uart_receiver_update_control();
    uart_receiver.finsh_flag = 1U;
    uart_receiver.frame_count++;
    g_receiver_last_frame_tick = g_receiver_process_tick;
}

static void uart_receiver_parse_byte(uint8_t data)
{
    if(0U == g_receiver_frame_length)
    {
        if(UART_RECEIVER_FRAME_START == data)
        {
            g_receiver_frame[0] = data;
            g_receiver_frame_length = 1U;
        }
        return;
    }

    g_receiver_frame[g_receiver_frame_length++] = data;
    if(g_receiver_frame_length < UART_RECEIVER_FRAME_LEN)
    {
        return;
    }

    g_receiver_frame_length = 0U;
    if((UART_RECEIVER_FRAME_START == g_receiver_frame[0]) &&
       (UART_RECEIVER_FRAME_END == g_receiver_frame[UART_RECEIVER_FRAME_LEN - 1U]))
    {
        uart_receiver_analysis(g_receiver_frame);
    }
    else
    {
        uart_receiver.error_count++;
        if(UART_RECEIVER_FRAME_START == data)
        {
            g_receiver_frame[0] = data;
            g_receiver_frame_length = 1U;
        }
    }
}

fsp_err_t uart_receiver_init(void)
{
    uint32_t const rx_pin_cfg = (uint32_t)IOPORT_CFG_PERIPHERAL_PIN |
                                (uint32_t)IOPORT_CFG_PULLUP_ENABLE |
                                (uint32_t)IOPORT_CFG_PIM_TTL |
                                (uint32_t)IOPORT_PERIPHERAL_SCI0_2_4_6_8;

    memset((void *)&uart_receiver, 0, sizeof(uart_receiver));
    uart_receiver_ring_reset();
    g_receiver_frame_length = 0U;
    g_receiver_process_tick = 0U;
    g_receiver_last_frame_tick = 0U;
    g_receiver_last_print_tick = 0U;
    g_receiver_last_diag_tick = 0U;
    g_receiver_last_state = 0U;
    g_receiver_opened = false;
    g_receiver_control_enabled = false;
    g_receiver_steering_sum = 0U;
    g_receiver_throttle_sum = 0U;
    g_receiver_calibration_count = 0U;

    fsp_err_t err = g_ioport.p_api->pinCfg(g_ioport.p_ctrl, UART_RECEIVER_RX_PIN, rx_pin_cfg);
    if(FSP_SUCCESS != err)
    {
        return err;
    }

    err = R_SCI_B_UART_BaudCalculate(UART_RECEIVER_BAUDRATE,
                                     false,
                                     5000U,
                                     &g_uart6_baud_setting);
    if(FSP_SUCCESS != err)
    {
        return err;
    }

    err = g_uart6.p_api->open(g_uart6.p_ctrl, g_uart6.p_cfg);
    if(FSP_SUCCESS != err)
    {
        return err;
    }

    err = g_uart6.p_api->callbackSet(g_uart6.p_ctrl, uart_receiver_callback, NULL, NULL);
    if(FSP_SUCCESS != err)
    {
        (void)g_uart6.p_api->close(g_uart6.p_ctrl);
        return err;
    }

    g_receiver_opened = true;
    return FSP_SUCCESS;
}

void uart_receiver_process(void)
{
    if(!g_receiver_opened)
    {
        return;
    }

    g_receiver_process_tick++;

    bsp_io_level_t rx_level = BSP_IO_LEVEL_HIGH;
    if(FSP_SUCCESS == g_ioport.p_api->pinRead(g_ioport.p_ctrl, UART_RECEIVER_RX_PIN, &rx_level))
    {
        if(BSP_IO_LEVEL_LOW == rx_level)
        {
            uart_receiver.pin_low_count++;
        }
    }

    uint8_t data;
    while(uart_receiver_ring_pop(&data))
    {
        uart_receiver_parse_byte(data);
    }

    if((g_receiver_process_tick - g_receiver_last_frame_tick) >= UART_RECEIVER_TIMEOUT_TICKS)
    {
        uart_receiver.state = 0U;
        uart_receiver_update_control();
    }

    bool state_changed = (g_receiver_last_state != uart_receiver.state);
    bool print_due = ((g_receiver_process_tick - g_receiver_last_print_tick) >= UART_RECEIVER_PRINT_TICKS);

    if(((0U != uart_receiver.finsh_flag) && print_due) || state_changed)
    {
        if(0U != uart_receiver.state)
        {
            printf("REMOTE CH1-CH6:%u %u %u %u %u %u MAP steer:%d throttle:%d centers:%u,%u cal:%u run:%u\r\n",
                   (unsigned)uart_receiver.channel[0],
                   (unsigned)uart_receiver.channel[1],
                   (unsigned)uart_receiver.channel[2],
                   (unsigned)uart_receiver.channel[3],
                   (unsigned)uart_receiver.channel[4],
                   (unsigned)uart_receiver.channel[5],
                   (int)uart_receiver.steering,
                   (int)uart_receiver.throttle,
                   (unsigned)uart_receiver.steering_center,
                   (unsigned)uart_receiver.throttle_center,
                   (unsigned)uart_receiver.calibration_ready,
                   (unsigned)uart_receiver.run_enabled);
        }
        else
        {
            printf("REMOTE disconnected/failsafe.\r\n");
        }

        g_receiver_last_print_tick = g_receiver_process_tick;
        g_receiver_last_state = uart_receiver.state;
    }

    if((g_receiver_process_tick - g_receiver_last_diag_tick) >= UART_RECEIVER_DIAG_TICKS)
    {
        printf("REMOTE DIAG bytes:%lu frames:%lu errors:%lu cb:%lu break:%lu low:%lu last:0x%02X state:%u\r\n",
               (unsigned long)uart_receiver.rx_byte_count,
               (unsigned long)uart_receiver.frame_count,
               (unsigned long)uart_receiver.error_count,
               (unsigned long)uart_receiver.callback_count,
               (unsigned long)uart_receiver.break_count,
               (unsigned long)uart_receiver.pin_low_count,
               (unsigned)uart_receiver.last_rx_byte,
               (unsigned)uart_receiver.state);
        g_receiver_last_diag_tick = g_receiver_process_tick;
    }

    uart_receiver.finsh_flag = 0U;
}

void uart_receiver_callback(uart_callback_args_t * p_args)
{
    if((NULL == p_args) || (6U != p_args->channel))
    {
        return;
    }

    uart_receiver.callback_count++;

    switch(p_args->event)
    {
        case UART_EVENT_RX_CHAR:
            uart_receiver.last_rx_byte = (uint8_t)(p_args->data & 0xFFU);
            uart_receiver.rx_byte_count++;
            uart_receiver_ring_push(uart_receiver.last_rx_byte);
            break;

        case UART_EVENT_ERR_PARITY:
        case UART_EVENT_ERR_FRAMING:
        case UART_EVENT_ERR_OVERFLOW:
            uart_receiver.last_rx_byte = (uint8_t)(p_args->data & 0xFFU);
            uart_receiver.error_count++;
            g_receiver_frame_length = 0U;
            uart_receiver_ring_reset();
            break;

        case UART_EVENT_BREAK_DETECT:
            uart_receiver.last_rx_byte = (uint8_t)(p_args->data & 0xFFU);
            uart_receiver.break_count++;
            g_receiver_frame_length = 0U;
            uart_receiver_ring_reset();
            break;

        default:
            break;
    }
}

void uart_receiver_set_enabled(bool enabled)
{
    if(g_receiver_control_enabled != enabled)
    {
        g_receiver_control_enabled = enabled;
        /* 鍒囨崲鏃剁珛鍗虫竻闆惰緭鍑猴紝閬垮厤涓婁竴娆＄殑娌归棬/鏂瑰悜娈嬬暀 */
        uart_receiver.steering = 0;
        uart_receiver.throttle = 0;
        printf("REMOTE control %s.\r\n", enabled ? "ENABLED" : "disabled");
    }
}

bool uart_receiver_is_enabled(void)
{
    return g_receiver_control_enabled;
}
