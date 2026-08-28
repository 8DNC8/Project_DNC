#include "brushless_driver.h"

#include <string.h>

#define BRUSHLESS_FRAME_HEAD              (0xA5U)
#define BRUSHLESS_CMD_SET_DUTY            (0x01U)
#define BRUSHLESS_CMD_SPEED_FEEDBACK      (0x02U)
#define BRUSHLESS_FRAME_SIZE              (7U)
#define BRUSHLESS_MAX_DUTY                (10000)
#define BRUSHLESS_MIN_DUTY                (-10000)
#define BRUSHLESS_RX_PIN_LEVEL_DIAG       (0U)
#define BRUSHLESS_UART_LOOPBACK_DIAG      (0U)
#define BRUSHLESS_RX_RING_SIZE            (128U)
#define BRUSHLESS_RX_PROCESS_LIMIT        (16U)
#define BRUSHLESS_SPEED_REQUEST_PERIOD    (10U)    /* 10 * 5ms = 50ms，供速度闭环使用 */
#define BRUSHLESS_STARTUP_IGNORE_TICKS    (60U)
#define BRUSHLESS_RX_TIMEOUT_TICKS        (400U)   /* 400 * 5ms = 2s 无有效帧则自动重启 SCI4 */
#define BRUSHLESS_CSR_RX_ERROR_MASK       (R_SCI_B0_CSR_ORER_Msk | R_SCI_B0_CSR_FER_Msk | R_SCI_B0_CSR_PER_Msk)
#define BRUSHLESS_CFCLR_CLEAR_ERRORS      (BRUSHLESS_CSR_RX_ERROR_MASK)
#define BRUSHLESS_CFCLR_CLEAR_RDRF        (0x80000000U)

#if defined(VECTOR_NUMBER_SCI4_RXI) && defined(VECTOR_NUMBER_SCI4_TXI) && \
    defined(VECTOR_NUMBER_SCI4_TEI) && defined(VECTOR_NUMBER_SCI4_ERI)
#define BRUSHLESS_DRIVER_SCI4_AVAILABLE   (1)
#else
#define BRUSHLESS_DRIVER_SCI4_AVAILABLE   (0)
#endif

#if BRUSHLESS_DRIVER_SCI4_AVAILABLE
static sci_b_uart_instance_ctrl_t g_brushless_uart4_ctrl;
static sci_b_baud_setting_t g_brushless_uart4_baud_setting;

static const sci_b_uart_extended_cfg_t g_brushless_uart4_cfg_extend =
{
    .clock = SCI_B_UART_CLOCK_INT,
    .rx_edge_start = SCI_B_UART_START_BIT_FALLING_EDGE,
    .noise_cancel = SCI_B_UART_NOISE_CANCELLATION_DISABLE,
    .p_baud_setting = &g_brushless_uart4_baud_setting,
    .rx_fifo_trigger = SCI_B_UART_RX_FIFO_TRIGGER_1,
    .flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
    .flow_control = SCI_B_UART_FLOW_CONTROL_RTS,
    .rs485_setting =
    {
        .enable = SCI_B_UART_RS485_DISABLE,
        .polarity = SCI_B_UART_RS485_DE_POLARITY_HIGH,
        .assertion_time = 1,
        .negation_time = 1,
    },
    .delay_cycles = 0,
};

static const uart_cfg_t g_brushless_uart4_cfg =
{
    .channel = 4,
    .data_bits = UART_DATA_BITS_8,
    .parity = UART_PARITY_OFF,
    .stop_bits = UART_STOP_BITS_1,
    .rxi_ipl = 15,
    .rxi_irq = VECTOR_NUMBER_SCI4_RXI,
    .txi_ipl = 12,
    .txi_irq = VECTOR_NUMBER_SCI4_TXI,
    .tei_ipl = 12,
    .tei_irq = VECTOR_NUMBER_SCI4_TEI,
    .eri_ipl = 15,
    .eri_irq = VECTOR_NUMBER_SCI4_ERI,
    .p_transfer_rx = NULL,
    .p_transfer_tx = NULL,
    .p_callback = brushless_driver_uart_callback,
    .p_context = NULL,
    .p_extend = &g_brushless_uart4_cfg_extend,
};

static const uart_instance_t g_brushless_uart4 =
{
    .p_ctrl = &g_brushless_uart4_ctrl,
    .p_cfg = &g_brushless_uart4_cfg,
    .p_api = &g_uart_on_sci_b,
};
#endif

static volatile bool g_brushless_tx_done = true;
static volatile bool g_brushless_opened = false;
static uint8_t g_brushless_tx_buffer[BRUSHLESS_FRAME_SIZE];
static uint8_t g_brushless_rx_buffer[BRUSHLESS_FRAME_SIZE];
static uint8_t g_brushless_rx_count = 0;
static uint32_t g_brushless_process_count = 0;
static bool g_brushless_speed_requested = false;
#if BRUSHLESS_RX_PIN_LEVEL_DIAG
static bsp_io_level_t g_brushless_last_rx_level = BSP_IO_LEVEL_LOW;
#endif
static volatile uint8_t g_brushless_rx_ring[BRUSHLESS_RX_RING_SIZE];
static volatile uint8_t g_brushless_rx_ring_head = 0;
static volatile uint8_t g_brushless_rx_ring_tail = 0;
static brushless_driver_status_t g_brushless_status;
static uint32_t g_brushless_last_frame_tick = 0;

static int16_t brushless_limit_duty(int16_t duty)
{
    if(duty > BRUSHLESS_MAX_DUTY)
    {
        return BRUSHLESS_MAX_DUTY;
    }
    if(duty < BRUSHLESS_MIN_DUTY)
    {
        return BRUSHLESS_MIN_DUTY;
    }
    return duty;
}

static uint8_t brushless_sum_check(uint8_t const * p_data)
{
    uint8_t sum = 0;
    for(uint32_t i = 0; i < (BRUSHLESS_FRAME_SIZE - 1U); i++)
    {
        sum = (uint8_t)(sum + p_data[i]);
    }
    return sum;
}

#if BRUSHLESS_DRIVER_SCI4_AVAILABLE
static void brushless_ring_reset(void)
{
    g_brushless_rx_ring_head = 0;
    g_brushless_rx_ring_tail = 0;
}

static void brushless_ring_push_from_isr(uint8_t data)
{
    uint8_t next_head = (uint8_t)((g_brushless_rx_ring_head + 1U) % BRUSHLESS_RX_RING_SIZE);
    if(next_head == g_brushless_rx_ring_tail)
    {
        g_brushless_status.rx_error_count++;
        return;
    }

    g_brushless_rx_ring[g_brushless_rx_ring_head] = data;
    g_brushless_rx_ring_head = next_head;
}

static bool brushless_ring_pop(uint8_t * p_data)
{
    if(g_brushless_rx_ring_tail == g_brushless_rx_ring_head)
    {
        return false;
    }

    *p_data = g_brushless_rx_ring[g_brushless_rx_ring_tail];
    g_brushless_rx_ring_tail = (uint8_t)((g_brushless_rx_ring_tail + 1U) % BRUSHLESS_RX_RING_SIZE);
    return true;
}

static void brushless_enable_rx_irqs(void)
{
    /* 同时使能 RXI 和 ERI：让 FSP 的 ERI 中断当场处理接收错误（ORER/FER/PER），
     * 而不是关掉 ERI 再由主循环轮询清错误标志（那样会跟 RXI 中断竞争，偶发卡死）。 */
    R_BSP_IrqStatusClear(VECTOR_NUMBER_SCI4_ERI);
    R_BSP_IrqStatusClear(VECTOR_NUMBER_SCI4_RXI);
    R_BSP_IrqEnable(VECTOR_NUMBER_SCI4_RXI);
    R_BSP_IrqEnable(VECTOR_NUMBER_SCI4_ERI);
}

static void brushless_clear_rx_error_flags(void)
{
    R_SCI_B0_Type * p_reg = g_brushless_uart4_ctrl.p_reg;
    if(NULL == p_reg)
    {
        return;
    }

    uint32_t csr = p_reg->CSR;
    if(0U == (csr & BRUSHLESS_CSR_RX_ERROR_MASK))
    {
        return;
    }

    g_brushless_status.rx_error_count++;
    g_brushless_rx_count = 0;
    brushless_ring_reset();
    (void) p_reg->RDR_BY;
    p_reg->CFCLR |= (BRUSHLESS_CFCLR_CLEAR_ERRORS | BRUSHLESS_CFCLR_CLEAR_RDRF);
    R_BSP_IrqStatusClear(VECTOR_NUMBER_SCI4_RXI);
    R_BSP_IrqStatusClear(VECTOR_NUMBER_SCI4_ERI);
}

static void brushless_parse_rx_byte(uint8_t data)
{
    g_brushless_status.rx_byte_count++;
    g_brushless_status.last_rx_byte = data;

    if((0U == g_brushless_rx_count) && (BRUSHLESS_FRAME_HEAD != data))
    {
        return;
    }

    if((BRUSHLESS_FRAME_HEAD == data) && (0U != g_brushless_rx_count))
    {
        g_brushless_rx_count = 0;
    }

    g_brushless_rx_buffer[g_brushless_rx_count++] = data;
    if(g_brushless_rx_count < BRUSHLESS_FRAME_SIZE)
    {
        return;
    }

    g_brushless_rx_count = 0;
    if(BRUSHLESS_FRAME_HEAD != g_brushless_rx_buffer[0])
    {
        g_brushless_status.rx_error_count++;
        return;
    }

    if(brushless_sum_check(g_brushless_rx_buffer) != g_brushless_rx_buffer[6])
    {
        g_brushless_status.rx_error_count++;
        return;
    }

    if(BRUSHLESS_CMD_SPEED_FEEDBACK == g_brushless_rx_buffer[1])
    {
        g_brushless_status.left_speed = (int16_t)((uint16_t)(((uint16_t) g_brushless_rx_buffer[2] << 8) |
                                                              (uint16_t) g_brushless_rx_buffer[3]));
        g_brushless_status.right_speed = (int16_t)((uint16_t)(((uint16_t) g_brushless_rx_buffer[4] << 8) |
                                                               (uint16_t) g_brushless_rx_buffer[5]));
        g_brushless_status.rx_frame_count++;
    }
    else
    {
        g_brushless_status.rx_error_count++;
    }

    g_brushless_rx_count = 0;
}

static void brushless_driver_recover(void)
{
    /* 长时间没收到有效速度帧时自动重启 SCI4，把偶发的“卡死”变成短暂停顿后自愈。
     * 只有 TX 空闲时才做完整 close+open（FSP 的 close 内部会等 TEND，TX 卡住时 close 会卡死）。 */
    if(g_brushless_tx_done)
    {
        g_brushless_uart4.p_api->close(g_brushless_uart4.p_ctrl);
        g_brushless_uart4.p_api->open(g_brushless_uart4.p_ctrl, g_brushless_uart4.p_cfg);
        brushless_enable_rx_irqs();
        g_brushless_opened = true;
    }
    else
    {
        /* TX 忙/卡住：只做轻量恢复，避免 close 卡死 */
        brushless_enable_rx_irqs();
    }

    g_brushless_rx_count = 0;
    brushless_ring_reset();
    g_brushless_speed_requested = false;
    g_brushless_last_frame_tick = g_brushless_process_count;
}
#endif

static fsp_err_t brushless_send_frame(uint8_t command, int16_t left_value, int16_t right_value)
{
#if !BRUSHLESS_DRIVER_SCI4_AVAILABLE
    FSP_PARAMETER_NOT_USED(command);
    FSP_PARAMETER_NOT_USED(left_value);
    FSP_PARAMETER_NOT_USED(right_value);
    g_brushless_status.last_tx_error = FSP_ERR_UNSUPPORTED;
    return FSP_ERR_UNSUPPORTED;
#else
    if(!g_brushless_opened)
    {
        g_brushless_status.last_tx_error = FSP_ERR_NOT_OPEN;
        return FSP_ERR_NOT_OPEN;
    }

    if(!g_brushless_tx_done)
    {
        g_brushless_status.last_tx_error = FSP_ERR_IN_USE;
        return FSP_ERR_IN_USE;
    }

    g_brushless_tx_buffer[0] = BRUSHLESS_FRAME_HEAD;
    g_brushless_tx_buffer[1] = command;
    g_brushless_tx_buffer[2] = (uint8_t)(((uint16_t) left_value >> 8) & 0xFFU);
    g_brushless_tx_buffer[3] = (uint8_t)((uint16_t) left_value & 0xFFU);
    g_brushless_tx_buffer[4] = (uint8_t)(((uint16_t) right_value >> 8) & 0xFFU);
    g_brushless_tx_buffer[5] = (uint8_t)((uint16_t) right_value & 0xFFU);
    g_brushless_tx_buffer[6] = brushless_sum_check(g_brushless_tx_buffer);

    g_brushless_tx_done = false;
    fsp_err_t err = g_brushless_uart4.p_api->write(g_brushless_uart4.p_ctrl,
                                                   g_brushless_tx_buffer,
                                                   BRUSHLESS_FRAME_SIZE);
    if(FSP_SUCCESS != err)
    {
        g_brushless_tx_done = true;
        g_brushless_status.last_tx_error = err;
        return err;
    }

    g_brushless_status.tx_frame_count++;
    g_brushless_status.last_tx_error = FSP_SUCCESS;
    return FSP_SUCCESS;
#endif
}

fsp_err_t brushless_driver_init(void)
{
#if !BRUSHLESS_DRIVER_SCI4_AVAILABLE
    memset(&g_brushless_status, 0, sizeof(g_brushless_status));
    g_brushless_status.last_tx_error = FSP_ERR_UNSUPPORTED;
    return FSP_ERR_UNSUPPORTED;
#else
#if BRUSHLESS_RX_PIN_LEVEL_DIAG
    uint32_t const rx_diag_pin_cfg = (uint32_t) IOPORT_CFG_PORT_DIRECTION_INPUT |
                                     (uint32_t) IOPORT_CFG_PULLUP_ENABLE |
                                     (uint32_t) IOPORT_CFG_PIM_TTL;

    memset(&g_brushless_status, 0, sizeof(g_brushless_status));
    g_brushless_tx_done = true;
    g_brushless_rx_count = 0;
    g_brushless_process_count = 0;
    g_brushless_speed_requested = false;
    brushless_ring_reset();

    fsp_err_t diag_err = g_ioport.p_api->pinCfg(g_ioport.p_ctrl, BRUSHLESS_DRIVER_RX_PIN, rx_diag_pin_cfg);
    if(FSP_SUCCESS != diag_err)
    {
        g_brushless_status.last_tx_error = diag_err;
        return diag_err;
    }

    (void) g_ioport.p_api->pinRead(g_ioport.p_ctrl, BRUSHLESS_DRIVER_RX_PIN, &g_brushless_last_rx_level);
    g_brushless_status.left_speed = (BSP_IO_LEVEL_HIGH == g_brushless_last_rx_level) ? 1 : 0;
    g_brushless_status.last_rx_byte = (uint8_t) g_brushless_status.left_speed;
    g_brushless_status.last_tx_error = FSP_SUCCESS;
    g_brushless_opened = true;
    return FSP_SUCCESS;
#else
    uint32_t const tx_pin_cfg = (uint32_t) IOPORT_CFG_PERIPHERAL_PIN |
                                (uint32_t) IOPORT_PERIPHERAL_SCI0_2_4_6_8;
    uint32_t const rx_pin_cfg = (uint32_t) IOPORT_CFG_PERIPHERAL_PIN |
                                (uint32_t) IOPORT_CFG_PULLUP_ENABLE |
                                (uint32_t) IOPORT_CFG_PIM_TTL |
                                (uint32_t) IOPORT_PERIPHERAL_SCI0_2_4_6_8;

    fsp_err_t err = g_ioport.p_api->pinCfg(g_ioport.p_ctrl, BRUSHLESS_DRIVER_TX_PIN, tx_pin_cfg);
    if(FSP_SUCCESS != err)
    {
        return err;
    }

    err = g_ioport.p_api->pinCfg(g_ioport.p_ctrl, BRUSHLESS_DRIVER_RX_PIN, rx_pin_cfg);
    if(FSP_SUCCESS != err)
    {
        return err;
    }

    memset(&g_brushless_status, 0, sizeof(g_brushless_status));
    g_brushless_tx_done = true;
    g_brushless_rx_count = 0;
    g_brushless_process_count = 0;

    err = R_SCI_B_UART_BaudCalculate(BRUSHLESS_DRIVER_BAUDRATE,
                                     false,
                                     5000U,
                                     &g_brushless_uart4_baud_setting);
    if(FSP_SUCCESS != err)
    {
        return err;
    }

    err = g_brushless_uart4.p_api->open(g_brushless_uart4.p_ctrl, g_brushless_uart4.p_cfg);
    if(FSP_SUCCESS != err)
    {
        return err;
    }

    brushless_enable_rx_irqs();
    g_brushless_opened = true;
    return FSP_SUCCESS;
#endif
#endif
}

fsp_err_t brushless_driver_set_duty(int16_t left_duty, int16_t right_duty)
{
#if BRUSHLESS_RX_PIN_LEVEL_DIAG || BRUSHLESS_UART_LOOPBACK_DIAG
    FSP_PARAMETER_NOT_USED(left_duty);
    FSP_PARAMETER_NOT_USED(right_duty);
    return FSP_SUCCESS;
#else
    return brushless_send_frame(BRUSHLESS_CMD_SET_DUTY,
                                brushless_limit_duty(left_duty),
                                brushless_limit_duty(right_duty));
#endif
}

fsp_err_t brushless_driver_request_speed(void)
{
#if BRUSHLESS_RX_PIN_LEVEL_DIAG
    return FSP_SUCCESS;
#else
    return brushless_send_frame(BRUSHLESS_CMD_SPEED_FEEDBACK, 0, 0);
#endif
}

void brushless_driver_process(void)
{
#if BRUSHLESS_DRIVER_SCI4_AVAILABLE
    if(!g_brushless_opened)
    {
        return;
    }

#if BRUSHLESS_RX_PIN_LEVEL_DIAG
    bsp_io_level_t rx_level = BSP_IO_LEVEL_LOW;
    if(FSP_SUCCESS != g_ioport.p_api->pinRead(g_ioport.p_ctrl, BRUSHLESS_DRIVER_RX_PIN, &rx_level))
    {
        g_brushless_status.rx_error_count++;
        return;
    }

    g_brushless_status.rx_byte_count++;
    if(rx_level != g_brushless_last_rx_level)
    {
        g_brushless_status.rx_frame_count++;
        g_brushless_last_rx_level = rx_level;
    }

    if(BSP_IO_LEVEL_HIGH == rx_level)
    {
        g_brushless_status.rx_error_count++;
        g_brushless_status.left_speed = 1;
        g_brushless_status.last_rx_byte = 1;
    }
    else
    {
        g_brushless_status.right_speed++;
        g_brushless_status.left_speed = 0;
        g_brushless_status.last_rx_byte = 0;
    }
#else
    brushless_clear_rx_error_flags();

    g_brushless_process_count++;
    if(g_brushless_process_count < BRUSHLESS_STARTUP_IGNORE_TICKS)
    {
        brushless_ring_reset();
        g_brushless_rx_count = 0;
        return;
    }

    uint32_t frames_before = g_brushless_status.rx_frame_count;
    for(uint32_t i = 0; i < BRUSHLESS_RX_PROCESS_LIMIT; i++)
    {
        uint8_t data = 0;
        if(!brushless_ring_pop(&data))
        {
            break;
        }

        brushless_parse_rx_byte(data);
    }
    if(g_brushless_status.rx_frame_count != frames_before)
    {
        g_brushless_last_frame_tick = g_brushless_process_count;
    }

    uint32_t ticks_since_frame = g_brushless_process_count - g_brushless_last_frame_tick;

    if((!g_brushless_speed_requested) ||
       (ticks_since_frame >= BRUSHLESS_SPEED_REQUEST_PERIOD) ||
       BRUSHLESS_UART_LOOPBACK_DIAG)
    {
        if(FSP_SUCCESS == brushless_driver_request_speed())
        {
            g_brushless_speed_requested = true;
        }
    }

    if(ticks_since_frame >= BRUSHLESS_RX_TIMEOUT_TICKS)
    {
        brushless_driver_recover();
    }
#endif
#endif
}

void brushless_driver_get_status(brushless_driver_status_t * p_status)
{
    if(NULL != p_status)
    {
        *p_status = g_brushless_status;
    }
}

void brushless_driver_uart_callback(uart_callback_args_t * p_args)
{
#if !BRUSHLESS_DRIVER_SCI4_AVAILABLE
    FSP_PARAMETER_NOT_USED(p_args);
#else
    if((NULL == p_args) || (4U != p_args->channel))
    {
        return;
    }

    switch(p_args->event)
    {
        case UART_EVENT_TX_COMPLETE:
            g_brushless_tx_done = true;
            break;

        case UART_EVENT_RX_CHAR:
            brushless_ring_push_from_isr((uint8_t)(p_args->data & 0xFFU));
            break;

        case UART_EVENT_RX_COMPLETE:
            break;

        case UART_EVENT_ERR_PARITY:
        case UART_EVENT_ERR_FRAMING:
        case UART_EVENT_ERR_OVERFLOW:
            g_brushless_status.rx_error_count++;
            g_brushless_rx_count = 0;
            brushless_ring_reset();
            break;

        default:
            break;
    }
#endif
}
