#ifndef BALANCE_CONTROL_H_
#define BALANCE_CONTROL_H_

#include "zf_common_headfile.h"

#define BALANCE_CONTROL_PERIOD_MS              (5U)
#define BALANCE_CONTROL_SERVO_PERIOD_TICKS     (4U)

typedef struct
{
    bool enabled;
    bool safe;
    uint32_t tick_count;
    int16_t left_duty;
    int16_t right_duty;
    float angle_target;
    float angle_output;
    float angular_speed_output;
    bool shutdown;
    fsp_err_t brushless_init_error;
    fsp_err_t brushless_last_error;
} balance_control_status_t;

void balance_control_init(void);
void balance_control_update(void);
void balance_control_stop(void);
void balance_control_get_status(balance_control_status_t *status);
bool balance_control_kill_active(void);   /* CH3 杩愯闂搁棬鎬ュ仠涓紙杞瓙鍋滆浆銆佽埖鏈烘帀鐢点€佷笉缁存寔骞宠　锛?*/

#endif /* BALANCE_CONTROL_H_ */
