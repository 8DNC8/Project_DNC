/* generated common source file - do not edit */
#include "common_data.h"
icu_instance_ctrl_t g_external_irq25_ctrl;

/** External IRQ extended configuration for ICU HAL driver */
const icu_extended_cfg_t g_external_irq25_ext_cfg =
{ .filter_src = EXTERNAL_IRQ_DIGITAL_FILTER_PCLK_DIV, };

const external_irq_cfg_t g_external_irq25_cfg =
{ .channel = 25, .trigger = EXTERNAL_IRQ_TRIG_RISING, .filter_enable = false, .clock_source_div =
          EXTERNAL_IRQ_CLOCK_SOURCE_DIV_64,
  .p_callback = NULL,
  /** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
  .p_context = (void*) &NULL,
#endif
  .p_extend = (void*) &g_external_irq25_ext_cfg,
  .ipl = (14),
#if defined(VECTOR_NUMBER_ICU_IRQ25)
    .irq                 = VECTOR_NUMBER_ICU_IRQ25,
#else
  .irq = FSP_INVALID_VECTOR,
#endif
        };
/* Instance structure to use this module. */
const external_irq_instance_t g_external_irq25 =
{ .p_ctrl = &g_external_irq25_ctrl, .p_cfg = &g_external_irq25_cfg, .p_api = &g_external_irq_on_icu };
icu_instance_ctrl_t g_external_irq26_ctrl;

/** External IRQ extended configuration for ICU HAL driver */
const icu_extended_cfg_t g_external_irq26_ext_cfg =
{ .filter_src = EXTERNAL_IRQ_DIGITAL_FILTER_PCLK_DIV, };

const external_irq_cfg_t g_external_irq26_cfg =
{ .channel = 26, .trigger = EXTERNAL_IRQ_TRIG_FALLING, .filter_enable = false, .clock_source_div =
          EXTERNAL_IRQ_CLOCK_SOURCE_DIV_64,
  .p_callback = NULL,
  /** If NULL then do not add & */
#if defined(NULL)
    .p_context           = NULL,
#else
  .p_context = (void*) &NULL,
#endif
  .p_extend = (void*) &g_external_irq26_ext_cfg,
  .ipl = (12),
#if defined(VECTOR_NUMBER_ICU_IRQ26)
    .irq                 = VECTOR_NUMBER_ICU_IRQ26,
#else
  .irq = FSP_INVALID_VECTOR,
#endif
        };
/* Instance structure to use this module. */
const external_irq_instance_t g_external_irq26 =
{ .p_ctrl = &g_external_irq26_ctrl, .p_cfg = &g_external_irq26_cfg, .p_api = &g_external_irq_on_icu };
ioport_instance_ctrl_t g_ioport_ctrl;
const ioport_instance_t g_ioport =
{ .p_api = &g_ioport_on_ioport, .p_ctrl = &g_ioport_ctrl, .p_cfg = &g_bsp_pin_cfg, };
void g_common_init(void)
{
}
