#ifndef SERVO_HOLD_H_
#define SERVO_HOLD_H_

#include "hal_data.h"

#define SERVO_HOLD_CH1_PIN       (BSP_IO_PORT_01_PIN_05)   /* P105 / P7-1 */
#define SERVO_HOLD_CH2_PIN       (BSP_IO_PORT_01_PIN_04)   /* P104 / P7-3 */
#define SERVO_HOLD_CH3_PIN       (BSP_IO_PORT_13_PIN_06)   /* PD06 / P7-5 */
#define SERVO_HOLD_CH4_PIN       (BSP_IO_PORT_01_PIN_02)   /* P102 / P7-7 */

#define SERVO_HOLD_PULSE_US      (1500U)   /* 鑸垫満鏈烘闆剁偣鑴夊 */
#define SERVO_HOLD_PERIOD_US     (20000U)  /* 50Hz 鍛ㄦ湡 */

#define SERVO_HOLD_PULSE_MIN_US  (500U)
#define SERVO_HOLD_PULSE_MAX_US  (2500U)

void servo_hold_init(void);
void servo_hold_enable(void);                                   /* 鎭㈠鑸垫満杈撳嚭锛坉isable 鍚庨噸鏂颁娇鑳斤紝閰嶅悎鎬ュ仠鎭㈠锛?*/
void servo_hold_disable(void);                                  /* 鍥涜矾鎷変綆骞跺仠姝㈣緭鍑篜WM锛屽浣嶅悗鎭㈠ */
void servo_hold_set_pulse(uint8_t channel, uint16_t pulse_us);   /* channel: 0..3 */
uint16_t servo_hold_get_pulse(uint8_t channel);
void servo_hold_refresh(void);                                   /* 20ms 鍛ㄦ湡鍒锋柊 4 璺嫭绔嬭剦瀹?*/
void servo_hold_refresh_pulse(uint32_t pulse_us);                /* 鍏煎锛? 璺悓鑴夊锛堟祴璇曠敤锛?*/
void servo_hold_run(void);

#endif /* SERVO_HOLD_H_ */
