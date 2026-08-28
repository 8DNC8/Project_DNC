/* generated HAL source file - do not edit */
#include "hal_data.h"
#define RA_NOT_DEFINED (UINT32_MAX)
#if (RA_NOT_DEFINED) != (RA_NOT_DEFINED)

/* If the TX transfer module is DMAC, define a DMAC TX transfer callback. */
#include "r_dmac.h"
extern void sci_b_spi_tx_dmac_callback(sci_b_spi_instance_ctrl_t const * const p_ctrl);

void g_sci_spi0_tx_transfer_callback (dmac_callback_args_t * p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
    sci_b_spi_tx_dmac_callback(&g_sci_spi0_ctrl);
}
#endif

#if (RA_NOT_DEFINED) != (RA_NOT_DEFINED)

/* If the RX transfer module is DMAC, define a DMAC RX transfer callback. */
#include "r_dmac.h"
extern void sci_b_spi_rx_dmac_callback(sci_b_spi_instance_ctrl_t const * const p_ctrl);

void g_sci_spi0_rx_transfer_callback (dmac_callback_args_t * p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
    sci_b_spi_rx_dmac_callback(&g_sci_spi0_ctrl);
}
#endif
#undef RA_NOT_DEFINED

sci_b_spi_instance_ctrl_t g_sci_spi0_ctrl;

/** SPI extended configuration */
const sci_b_spi_extended_cfg_t g_sci_spi0_cfg_extend =
        { .clk_div =
        {
        /* Actual calculated bitrate: 7812500. */.cks = 0,
          .brr = 7, .bgdm = 1, },
          .clock_source = (sci_b_spi_clock_source_t) 1, .rx_sampling_delay = SCI_B_SPI_RX_SAMPLING_DELAY_CYCLES_0, .tx_fifo_trigger =
                  SCI_B_SPI_TX_FIFO_TRIGGER_DISABLED, };

const spi_cfg_t g_sci_spi0_cfg =
{ .channel = 0, .operating_mode = SPI_MODE_MASTER, .clk_phase = SPI_CLK_PHASE_EDGE_ODD, .clk_polarity =
          SPI_CLK_POLARITY_LOW,
  .mode_fault = SPI_MODE_FAULT_ERROR_DISABLE, .bit_order = SPI_BIT_ORDER_MSB_FIRST,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_tx = NULL,
#else
    .p_transfer_tx   = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_rx = NULL,
#else
    .p_transfer_rx   = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
  .p_callback = sci0_b_spi_callback,
  .p_context = NULL,
#if defined(VECTOR_NUMBER_SCI0_RXI)
    .rxi_irq         = VECTOR_NUMBER_SCI0_RXI,
#else
  .rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI0_TXI)
    .txi_irq         = VECTOR_NUMBER_SCI0_TXI,
#else
  .txi_irq = FSP_INVALID_VECTOR,
#endif
  .tei_irq = VECTOR_NUMBER_SCI0_TEI,
  .eri_irq = VECTOR_NUMBER_SCI0_ERI, .rxi_ipl = (12), .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12), .p_extend =
          &g_sci_spi0_cfg_extend, };
/* Instance structure to use this module. */
const spi_instance_t g_sci_spi0 =
{ .p_ctrl = &g_sci_spi0_ctrl, .p_cfg = &g_sci_spi0_cfg, .p_api = &g_spi_on_sci_b };
ceu_instance_ctrl_t g_ceu0_ctrl;
const ceu_extended_cfg_t g_ceu0_extended_cfg =
{ .capture_format = CEU_CAPTURE_FORMAT_DATA_ENABLE,
  .input_order = CEU_INPUT_ORDER_CB0Y0CR0Y1,
  .output_format = CEU_OUTPUT_FORMAT_YCBCR422,
  .data_bus_width = CEU_DATA_BUS_SIZE_8_BIT,
  .edge_info.dsel = 0,
  .edge_info.hdsel = 0,
  .edge_info.vdsel = 0,
  .hsync_polarity = CEU_HSYNC_POLARITY_HIGH,
  .vsync_polarity = CEU_VSYNC_POLARITY_HIGH,
  .byte_swapping =
  { .swap_8bit_units = (0x2 | 0x4 | 0x0) >> 0x00 & 0x01,
    .swap_16bit_units = (0x2 | 0x4 | 0x0) >> 0x01 & 0x01,
    .swap_32bit_units = (0x2 | 0x4 | 0x0) >> 0x02 & 0x01, },
  .burst_mode = CEU_BURST_TRANSFER_MODE_X8,
  .scale_down_factor = 0x0U,
  .h_output_size = 0,
  .v_output_size = 0,
  .image_area_size = 38400,
  .interrupts_enabled = R_CEU_CEIER_FWFIE_Msk | R_CEU_CEIER_CPEIE_Msk | 0 | 0 | 0 | 0 | 0 | 0 | 0 | 0,
  .ceu_ipl = (14),
  .ceu_irq = VECTOR_NUMBER_CEU_CEUI, };

const capture_cfg_t g_ceu0_cfg =
{ .x_capture_pixels = 0,
  .y_capture_pixels = 0,
  .x_capture_start_pixel = 0,
  .y_capture_start_pixel = 0,
  .bytes_per_pixel = 2,
  .p_callback = g_ceu0_user_callback,
  .p_context = (void*) NULL,
  .p_extend = &g_ceu0_extended_cfg, };

const capture_instance_t g_ceu0 =
{ .p_ctrl = &g_ceu0_ctrl, .p_cfg = &g_ceu0_cfg, .p_api = &g_ceu_on_capture, };
#define RA_NOT_DEFINED (UINT32_MAX)
#if (RA_NOT_DEFINED) != (RA_NOT_DEFINED)

/* If the TX transfer module is DMAC, define a DMAC TX transfer callback. */
#include "r_dmac.h"
extern void sci_b_spi_tx_dmac_callback(sci_b_spi_instance_ctrl_t const * const p_ctrl);

void g_sci_spi5_tx_transfer_callback (dmac_callback_args_t * p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
    sci_b_spi_tx_dmac_callback(&g_sci_spi5_ctrl);
}
#endif

#if (RA_NOT_DEFINED) != (RA_NOT_DEFINED)

/* If the RX transfer module is DMAC, define a DMAC RX transfer callback. */
#include "r_dmac.h"
extern void sci_b_spi_rx_dmac_callback(sci_b_spi_instance_ctrl_t const * const p_ctrl);

void g_sci_spi5_rx_transfer_callback (dmac_callback_args_t * p_args)
{
    FSP_PARAMETER_NOT_USED(p_args);
    sci_b_spi_rx_dmac_callback(&g_sci_spi5_ctrl);
}
#endif
#undef RA_NOT_DEFINED

sci_b_spi_instance_ctrl_t g_sci_spi5_ctrl;

/** SPI extended configuration */
const sci_b_spi_extended_cfg_t g_sci_spi5_cfg_extend =
        { .clk_div =
        {
        /* Actual calculated bitrate: 31250000. */.cks = 0,
          .brr = 1, .bgdm = 1, },
          .clock_source = (sci_b_spi_clock_source_t) 1, .rx_sampling_delay = SCI_B_SPI_RX_SAMPLING_DELAY_CYCLES_0, .tx_fifo_trigger =
                  SCI_B_SPI_TX_FIFO_TRIGGER_DISABLED, };

const spi_cfg_t g_sci_spi5_cfg =
{ .channel = 5, .operating_mode = SPI_MODE_MASTER, .clk_phase = SPI_CLK_PHASE_EDGE_ODD, .clk_polarity =
          SPI_CLK_POLARITY_LOW,
  .mode_fault = SPI_MODE_FAULT_ERROR_DISABLE, .bit_order = SPI_BIT_ORDER_MSB_FIRST,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_tx = NULL,
#else
    .p_transfer_tx   = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_rx = NULL,
#else
    .p_transfer_rx   = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
  .p_callback = sci5_b_spi_callback,
  .p_context = NULL,
#if defined(VECTOR_NUMBER_SCI5_RXI)
    .rxi_irq         = VECTOR_NUMBER_SCI5_RXI,
#else
  .rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI5_TXI)
    .txi_irq         = VECTOR_NUMBER_SCI5_TXI,
#else
  .txi_irq = FSP_INVALID_VECTOR,
#endif
  .tei_irq = VECTOR_NUMBER_SCI5_TEI,
  .eri_irq = VECTOR_NUMBER_SCI5_ERI, .rxi_ipl = (15), .txi_ipl = (15), .tei_ipl = (12), .eri_ipl = (12), .p_extend =
          &g_sci_spi5_cfg_extend, };
/* Instance structure to use this module. */
const spi_instance_t g_sci_spi5 =
{ .p_ctrl = &g_sci_spi5_ctrl, .p_cfg = &g_sci_spi5_cfg, .p_api = &g_spi_on_sci_b };
sci_b_uart_instance_ctrl_t g_uart6_ctrl;

sci_b_baud_setting_t g_uart6_baud_setting =
        {
        /* Baud rate calculated with 0.000% error. */.baudrate_bits_b.abcse = 0,
          .baudrate_bits_b.abcs = 0, .baudrate_bits_b.bgdm = 1, .baudrate_bits_b.cks = 0, .baudrate_bits_b.brr = 74, .baudrate_bits_b.mddr =
                  (uint8_t) 256,
          .baudrate_bits_b.brme = false };

/** UART extended configuration for UARTonSCI HAL driver */
const sci_b_uart_extended_cfg_t g_uart6_cfg_extend =
{ .clock = SCI_B_UART_CLOCK_INT, .rx_edge_start = SCI_B_UART_START_BIT_FALLING_EDGE, .noise_cancel =
          SCI_B_UART_NOISE_CANCELLATION_DISABLE,
  .rx_fifo_trigger = SCI_B_UART_RX_FIFO_TRIGGER_1, .p_baud_setting = &g_uart6_baud_setting, .flow_control =
          SCI_B_UART_FLOW_CONTROL_RTS,
#if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_FF_PIN_0xFF,
                #else
  .flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
  .rs485_setting =
  { .enable = SCI_B_UART_RS485_DISABLE,
    .polarity = SCI_B_UART_RS485_DE_POLARITY_HIGH,
    .assertion_time = 1,
    .negation_time = 1, },
  .delay_cycles = 0, };

/** UART interface configuration */
const uart_cfg_t g_uart6_cfg =
{ .channel = 6, .data_bits = UART_DATA_BITS_8, .parity = UART_PARITY_EVEN, .stop_bits = UART_STOP_BITS_2, .p_callback =
          uart_receiver_callback,
  .p_context = NULL, .p_extend = &g_uart6_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_rx = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
  .rxi_ipl = (15),
  .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (15),
#if defined(VECTOR_NUMBER_SCI6_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI6_RXI,
#else
  .rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI6_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI6_TXI,
#else
  .txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI6_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI6_TEI,
#else
  .tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI6_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI6_ERI,
#else
  .eri_irq = FSP_INVALID_VECTOR,
#endif
        };

/* Instance structure to use this module. */
const uart_instance_t g_uart6 =
{ .p_ctrl = &g_uart6_ctrl, .p_cfg = &g_uart6_cfg, .p_api = &g_uart_on_sci_b };
sci_b_uart_instance_ctrl_t g_uart2_ctrl;

sci_b_baud_setting_t g_uart2_baud_setting =
        {
        /* Baud rate calculated with 0.160% error. */.baudrate_bits_b.abcse = 0,
          .baudrate_bits_b.abcs = 0, .baudrate_bits_b.bgdm = 1, .baudrate_bits_b.cks = 1, .baudrate_bits_b.brr = 194, .baudrate_bits_b.mddr =
                  (uint8_t) 256,
          .baudrate_bits_b.brme = false };

/** UART extended configuration for UARTonSCI HAL driver */
const sci_b_uart_extended_cfg_t g_uart2_cfg_extend =
{ .clock = SCI_B_UART_CLOCK_INT, .rx_edge_start = SCI_B_UART_START_BIT_FALLING_EDGE, .noise_cancel =
          SCI_B_UART_NOISE_CANCELLATION_DISABLE,
  .rx_fifo_trigger = SCI_B_UART_RX_FIFO_TRIGGER_MAX, .p_baud_setting = &g_uart2_baud_setting, .flow_control =
          SCI_B_UART_FLOW_CONTROL_RTS,
#if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_FF_PIN_0xFF,
                #else
  .flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
  .rs485_setting =
  { .enable = SCI_B_UART_RS485_DISABLE,
    .polarity = SCI_B_UART_RS485_DE_POLARITY_HIGH,
    .assertion_time = 1,
    .negation_time = 1, },
  .delay_cycles = 0, };

/** UART interface configuration */
const uart_cfg_t g_uart2_cfg =
{ .channel = 2, .data_bits = UART_DATA_BITS_8, .parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
          NULL,
  .p_context = NULL, .p_extend = &g_uart2_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_rx = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
  .rxi_ipl = (12),
  .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),
#if defined(VECTOR_NUMBER_SCI2_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI2_RXI,
#else
  .rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI2_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI2_TXI,
#else
  .txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI2_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI2_TEI,
#else
  .tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI2_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI2_ERI,
#else
  .eri_irq = FSP_INVALID_VECTOR,
#endif
        };

/* Instance structure to use this module. */
const uart_instance_t g_uart2 =
{ .p_ctrl = &g_uart2_ctrl, .p_cfg = &g_uart2_cfg, .p_api = &g_uart_on_sci_b };
sci_b_uart_instance_ctrl_t g_uart4_ctrl;

sci_b_baud_setting_t g_uart4_baud_setting =
        {
        /* Baud rate calculated with 1.725% error. */.baudrate_bits_b.abcse = 0,
          .baudrate_bits_b.abcs = 0, .baudrate_bits_b.bgdm = 1, .baudrate_bits_b.cks = 0, .baudrate_bits_b.brr = 15, .baudrate_bits_b.mddr =
                  (uint8_t) 256,
          .baudrate_bits_b.brme = false };

/** UART extended configuration for UARTonSCI HAL driver */
const sci_b_uart_extended_cfg_t g_uart4_cfg_extend =
{ .clock = SCI_B_UART_CLOCK_INT, .rx_edge_start = SCI_B_UART_START_BIT_FALLING_EDGE, .noise_cancel =
          SCI_B_UART_NOISE_CANCELLATION_DISABLE,
  .rx_fifo_trigger = SCI_B_UART_RX_FIFO_TRIGGER_MAX, .p_baud_setting = &g_uart4_baud_setting, .flow_control =
          SCI_B_UART_FLOW_CONTROL_RTS,
#if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_FF_PIN_0xFF,
                #else
  .flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
  .rs485_setting =
  { .enable = SCI_B_UART_RS485_DISABLE,
    .polarity = SCI_B_UART_RS485_DE_POLARITY_HIGH,
    .assertion_time = 1,
    .negation_time = 1, },
  .delay_cycles = 0, };

/** UART interface configuration */
const uart_cfg_t g_uart4_cfg =
{ .channel = 4, .data_bits = UART_DATA_BITS_8, .parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
          NULL,
  .p_context = NULL, .p_extend = &g_uart4_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_rx = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
  .rxi_ipl = (12),
  .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),
#if defined(VECTOR_NUMBER_SCI4_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI4_RXI,
#else
  .rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI4_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI4_TXI,
#else
  .txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI4_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI4_TEI,
#else
  .tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI4_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI4_ERI,
#else
  .eri_irq = FSP_INVALID_VECTOR,
#endif
        };

/* Instance structure to use this module. */
const uart_instance_t g_uart4 =
{ .p_ctrl = &g_uart4_ctrl, .p_cfg = &g_uart4_cfg, .p_api = &g_uart_on_sci_b };
sci_b_uart_instance_ctrl_t g_uart9_ctrl;

sci_b_baud_setting_t g_uart9_baud_setting =
        {
        /* Baud rate calculated with 0.160% error. */.baudrate_bits_b.abcse = 0,
          .baudrate_bits_b.abcs = 0, .baudrate_bits_b.bgdm = 1, .baudrate_bits_b.cks = 0, .baudrate_bits_b.brr = 64, .baudrate_bits_b.mddr =
                  (uint8_t) 256,
          .baudrate_bits_b.brme = false };

/** UART extended configuration for UARTonSCI HAL driver */
const sci_b_uart_extended_cfg_t g_uart9_cfg_extend =
{ .clock = SCI_B_UART_CLOCK_INT, .rx_edge_start = SCI_B_UART_START_BIT_FALLING_EDGE, .noise_cancel =
          SCI_B_UART_NOISE_CANCELLATION_DISABLE,
  .rx_fifo_trigger = SCI_B_UART_RX_FIFO_TRIGGER_MAX, .p_baud_setting = &g_uart9_baud_setting, .flow_control =
          SCI_B_UART_FLOW_CONTROL_RTS,
#if 0xFF != 0xFF
                .flow_control_pin       = BSP_IO_PORT_FF_PIN_0xFF,
                #else
  .flow_control_pin = (bsp_io_port_pin_t) UINT16_MAX,
#endif
  .rs485_setting =
  { .enable = SCI_B_UART_RS485_DISABLE,
    .polarity = SCI_B_UART_RS485_DE_POLARITY_HIGH,
    .assertion_time = 1,
    .negation_time = 1, },
  .delay_cycles = 0, };

/** UART interface configuration */
const uart_cfg_t g_uart9_cfg =
{ .channel = 9, .data_bits = UART_DATA_BITS_8, .parity = UART_PARITY_OFF, .stop_bits = UART_STOP_BITS_1, .p_callback =
          NULL,
  .p_context = NULL, .p_extend = &g_uart9_cfg_extend,
#define RA_NOT_DEFINED (1)
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_tx = NULL,
#else
                .p_transfer_tx       = &RA_NOT_DEFINED,
#endif
#if (RA_NOT_DEFINED == RA_NOT_DEFINED)
  .p_transfer_rx = NULL,
#else
                .p_transfer_rx       = &RA_NOT_DEFINED,
#endif
#undef RA_NOT_DEFINED
  .rxi_ipl = (12),
  .txi_ipl = (12), .tei_ipl = (12), .eri_ipl = (12),
#if defined(VECTOR_NUMBER_SCI9_RXI)
                .rxi_irq             = VECTOR_NUMBER_SCI9_RXI,
#else
  .rxi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI9_TXI)
                .txi_irq             = VECTOR_NUMBER_SCI9_TXI,
#else
  .txi_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI9_TEI)
                .tei_irq             = VECTOR_NUMBER_SCI9_TEI,
#else
  .tei_irq = FSP_INVALID_VECTOR,
#endif
#if defined(VECTOR_NUMBER_SCI9_ERI)
                .eri_irq             = VECTOR_NUMBER_SCI9_ERI,
#else
  .eri_irq = FSP_INVALID_VECTOR,
#endif
        };

/* Instance structure to use this module. */
const uart_instance_t g_uart9 =
{ .p_ctrl = &g_uart9_ctrl, .p_cfg = &g_uart9_cfg, .p_api = &g_uart_on_sci_b };
void g_hal_init(void)
{
    g_common_init ();
}
