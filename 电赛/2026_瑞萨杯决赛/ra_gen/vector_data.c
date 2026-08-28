/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = sci_b_uart_rxi_isr, /* SCI9 RXI (Receive data full) */
            [1] = sci_b_uart_txi_isr, /* SCI9 TXI (Transmit data empty) */
            [2] = sci_b_uart_tei_isr, /* SCI9 TEI (Transmit end) */
            [3] = sci_b_uart_eri_isr, /* SCI9 ERI (Receive error) */
            [4] = sci_b_uart_rxi_isr, /* SCI4 RXI (Receive data full) */
            [5] = sci_b_uart_txi_isr, /* SCI4 TXI (Transmit data empty) */
            [6] = sci_b_uart_tei_isr, /* SCI4 TEI (Transmit end) */
            [7] = sci_b_uart_eri_isr, /* SCI4 ERI (Receive error) */
            [8] = sci_b_uart_rxi_isr, /* SCI2 RXI (Receive data full) */
            [9] = sci_b_uart_txi_isr, /* SCI2 TXI (Transmit data empty) */
            [10] = sci_b_uart_tei_isr, /* SCI2 TEI (Transmit end) */
            [11] = sci_b_uart_eri_isr, /* SCI2 ERI (Receive error) */
            [12] = sci_b_uart_rxi_isr, /* SCI6 RXI (Receive data full) */
            [13] = sci_b_uart_txi_isr, /* SCI6 TXI (Transmit data empty) */
            [14] = sci_b_uart_tei_isr, /* SCI6 TEI (Transmit end) */
            [15] = sci_b_uart_eri_isr, /* SCI6 ERI (Receive error) */
            [16] = sci_b_spi_rxi_isr, /* SCI5 RXI (Receive data full) */
            [17] = sci_b_spi_txi_isr, /* SCI5 TXI (Transmit data empty) */
            [18] = sci_b_spi_tei_isr, /* SCI5 TEI (Transmit end) */
            [19] = sci_b_spi_eri_isr, /* SCI5 ERI (Receive error) */
            [20] = ceu_isr, /* CEU CEUI (CEU interrupt) */
            [21] = r_icu_isr, /* ICU IRQ26 (External pin interrupt 26) */
            [22] = sci_b_spi_rxi_isr, /* SCI0 RXI (Receive data full) */
            [23] = sci_b_spi_txi_isr, /* SCI0 TXI (Transmit data empty) */
            [24] = sci_b_spi_tei_isr, /* SCI0 TEI (Transmit end) */
            [25] = sci_b_spi_eri_isr, /* SCI0 ERI (Receive error) */
            [26] = r_icu_isr, /* ICU IRQ25 (External pin interrupt 25) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_SCI9_RXI,GROUP0), /* SCI9 RXI (Receive data full) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SCI9_TXI,GROUP1), /* SCI9 TXI (Transmit data empty) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SCI9_TEI,GROUP2), /* SCI9 TEI (Transmit end) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI9_ERI,GROUP3), /* SCI9 ERI (Receive error) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_SCI4_RXI,GROUP4), /* SCI4 RXI (Receive data full) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_SCI4_TXI,GROUP5), /* SCI4 TXI (Transmit data empty) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_SCI4_TEI,GROUP6), /* SCI4 TEI (Transmit end) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_SCI4_ERI,GROUP7), /* SCI4 ERI (Receive error) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_SCI2_RXI,GROUP0), /* SCI2 RXI (Receive data full) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_SCI2_TXI,GROUP1), /* SCI2 TXI (Transmit data empty) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_SCI2_TEI,GROUP2), /* SCI2 TEI (Transmit end) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_SCI2_ERI,GROUP3), /* SCI2 ERI (Receive error) */
            [12] = BSP_PRV_VECT_ENUM(EVENT_SCI6_RXI,GROUP4), /* SCI6 RXI (Receive data full) */
            [13] = BSP_PRV_VECT_ENUM(EVENT_SCI6_TXI,GROUP5), /* SCI6 TXI (Transmit data empty) */
            [14] = BSP_PRV_VECT_ENUM(EVENT_SCI6_TEI,GROUP6), /* SCI6 TEI (Transmit end) */
            [15] = BSP_PRV_VECT_ENUM(EVENT_SCI6_ERI,GROUP7), /* SCI6 ERI (Receive error) */
            [16] = BSP_PRV_VECT_ENUM(EVENT_SCI5_RXI,GROUP0), /* SCI5 RXI (Receive data full) */
            [17] = BSP_PRV_VECT_ENUM(EVENT_SCI5_TXI,GROUP1), /* SCI5 TXI (Transmit data empty) */
            [18] = BSP_PRV_VECT_ENUM(EVENT_SCI5_TEI,GROUP2), /* SCI5 TEI (Transmit end) */
            [19] = BSP_PRV_VECT_ENUM(EVENT_SCI5_ERI,GROUP3), /* SCI5 ERI (Receive error) */
            [20] = BSP_PRV_VECT_ENUM(EVENT_CEU_CEUI,GROUP4), /* CEU CEUI (CEU interrupt) */
            [21] = BSP_PRV_VECT_ENUM(EVENT_ICU_IRQ26,GROUP5), /* ICU IRQ26 (External pin interrupt 26) */
            [22] = BSP_PRV_VECT_ENUM(EVENT_SCI0_RXI,GROUP6), /* SCI0 RXI (Receive data full) */
            [23] = BSP_PRV_VECT_ENUM(EVENT_SCI0_TXI,GROUP7), /* SCI0 TXI (Transmit data empty) */
            [24] = BSP_PRV_VECT_ENUM(EVENT_SCI0_TEI,GROUP0), /* SCI0 TEI (Transmit end) */
            [25] = BSP_PRV_VECT_ENUM(EVENT_SCI0_ERI,GROUP1), /* SCI0 ERI (Receive error) */
            [26] = BSP_PRV_VECT_ENUM(EVENT_ICU_IRQ25,GROUP2), /* ICU IRQ25 (External pin interrupt 25) */
        };
        #endif
        #endif
