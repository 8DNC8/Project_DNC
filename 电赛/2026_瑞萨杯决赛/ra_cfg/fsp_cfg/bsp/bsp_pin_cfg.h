/* generated configuration header file - do not edit */
#ifndef BSP_PIN_CFG_H_
#define BSP_PIN_CFG_H_
#include "r_ioport.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

#define USER_LED (BSP_IO_PORT_01_PIN_10)
#define GPIO_IMU_CS (BSP_IO_PORT_06_PIN_04)
#define Camera_SCL (BSP_IO_PORT_07_PIN_06)
#define Camera_SDA (BSP_IO_PORT_07_PIN_07)
#define LCD_CS (BSP_IO_PORT_11_PIN_00)
#define LCD_DC (BSP_IO_PORT_11_PIN_01)
#define LCD_RST (BSP_IO_PORT_11_PIN_02)

extern const ioport_cfg_t g_bsp_pin_cfg; /* R7KA8P1KFLCAC.pincfg */

void BSP_PinConfigSecurityInit();

/* Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER
#endif /* BSP_PIN_CFG_H_ */
