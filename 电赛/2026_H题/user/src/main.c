#include "zf_common_headfile.h"

#include "motor.h"
#include "encoder.h"
#include "gray.h"
#include "pid.h"
#include "control.h"
#include "beep.h"
#include "display.h"
#include "key.h"
#include "uart_k230.h"
#include "DUOJI.h"

int main(void)
{
    clock_init(SYSTEM_CLOCK_80M);
    debug_init();

    gray_init();
    key_init(10);
    beep_init();
    motor_init();
    encoder_init();
    pid_init();
    display_init();
    k230_init();
    duoji_init();

    pit_ms_init(PIT_TIM_G12, 20, pit_callback, NULL);
    interrupt_set_priority(TIMG12_INT_IRQn, 7);
    interrupt_set_priority(GPIOA_INT_IRQn, 1);

    while(true)
    {

        key_process();
        display_update();
    }
}
