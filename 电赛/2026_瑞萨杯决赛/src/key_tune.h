#ifndef KEY_TUNE_H_
#define KEY_TUNE_H_

#include "zf_common_headfile.h"

typedef enum
{
    KEY_TUNE_PAGE_RUN = 0,
    KEY_TUNE_PAGE_PID,
    KEY_TUNE_PAGE_IMU,
    KEY_TUNE_PAGE_NAG,
    KEY_TUNE_PAGE_STR,
    KEY_TUNE_PAGE_MAX
} key_tune_page_t;

typedef enum
{
    KEY_TUNE_ITEM_ANGLE_P = 0,
    KEY_TUNE_ITEM_ANGLE_I,
    KEY_TUNE_ITEM_ANGLE_D,
    KEY_TUNE_ITEM_GYRO_P,
    KEY_TUNE_ITEM_ZERO,
    KEY_TUNE_ITEM_CORRECT_KP,
    KEY_TUNE_ITEM_CORRECT_KI,
    KEY_TUNE_ITEM_NAG_RECORD,
    KEY_TUNE_ITEM_NAG_SAVE,
    KEY_TUNE_ITEM_NAG_REPLAY,
    KEY_TUNE_ITEM_NAG_STRAIGHT,
    KEY_TUNE_ITEM_NAG_TURN_GAIN,
    KEY_TUNE_ITEM_NAG_TURN_TRIM,
    KEY_TUNE_ITEM_NAG_SPEED,
    KEY_TUNE_ITEM_NAG_SPEED_P,
    KEY_TUNE_ITEM_MAX
} key_tune_item_t;

typedef struct
{
    key_tune_page_t page;
    key_tune_item_t item;
    float value;
    float step;
    bool changed;
} key_tune_status_t;

void key_tune_init(void);
void key_tune_update(void);
void key_tune_get_status(key_tune_status_t *status);
const char * key_tune_get_item_name(key_tune_item_t item);
const char * key_tune_get_page_name(key_tune_page_t page);
bool key_tune_item_is_selected(key_tune_item_t item);
float key_tune_get_item_value(key_tune_item_t item);
float key_tune_get_item_step(key_tune_item_t item);

#endif /* KEY_TUNE_H_ */
