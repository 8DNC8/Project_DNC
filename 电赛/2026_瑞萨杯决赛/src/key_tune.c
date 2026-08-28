#include "key_tune.h"

#include "camera_display.h"
#include "imu_process.h"
#include "nag_navigation.h"

#define KEY_TUNE_KEY_COUNT       (4U)
#define KEY_TUNE_DEBOUNCE_TICKS  (4U)

typedef struct
{
    bsp_io_port_pin_t pin;
    bsp_io_level_t stable_level;
    bsp_io_level_t last_raw_level;
    uint8_t debounce_count;
    bool press_event;
    bool release_event;
} key_tune_key_t;

static key_tune_key_t g_key_tune_keys[KEY_TUNE_KEY_COUNT] =
{
    {BSP_IO_PORT_05_PIN_01, BSP_IO_LEVEL_HIGH, BSP_IO_LEVEL_HIGH, 0U, false, false},
    {BSP_IO_PORT_08_PIN_11, BSP_IO_LEVEL_HIGH, BSP_IO_LEVEL_HIGH, 0U, false, false},
    {BSP_IO_PORT_05_PIN_02, BSP_IO_LEVEL_HIGH, BSP_IO_LEVEL_HIGH, 0U, false, false},
    {BSP_IO_PORT_08_PIN_12, BSP_IO_LEVEL_HIGH, BSP_IO_LEVEL_HIGH, 0U, false, false},
};

static key_tune_page_t g_key_tune_page = KEY_TUNE_PAGE_RUN;
static key_tune_item_t g_key_tune_item = KEY_TUNE_ITEM_ANGLE_P;
static bool g_key_tune_changed = false;

static const key_tune_item_t g_key_tune_pid_items[] =
{
    KEY_TUNE_ITEM_ANGLE_P,
    KEY_TUNE_ITEM_ANGLE_I,
    KEY_TUNE_ITEM_ANGLE_D,
    KEY_TUNE_ITEM_GYRO_P,
    KEY_TUNE_ITEM_ZERO,
};

static const key_tune_item_t g_key_tune_imu_items[] =
{
    KEY_TUNE_ITEM_CORRECT_KP,
    KEY_TUNE_ITEM_CORRECT_KI,
};

static const key_tune_item_t g_key_tune_nag_items[] =
{
    KEY_TUNE_ITEM_NAG_RECORD,
    KEY_TUNE_ITEM_NAG_SAVE,
    KEY_TUNE_ITEM_NAG_REPLAY,
};

static const key_tune_item_t g_key_tune_str_items[] =
{
    KEY_TUNE_ITEM_NAG_STRAIGHT,
    KEY_TUNE_ITEM_NAG_TURN_GAIN,
    KEY_TUNE_ITEM_NAG_TURN_TRIM,
    KEY_TUNE_ITEM_NAG_SPEED,
};

/* 鍔ㄤ綔绫婚」鐩病鏈夎繛缁€硷紝杩斿洖杩欎釜鍗犱綅鎸囬拡 */
static float g_key_tune_dummy_value = 0.0f;

static float key_tune_limit(float value, float min, float max)
{
    if(value < min)
    {
        return min;
    }
    if(value > max)
    {
        return max;
    }
    return value;
}

static float * key_tune_item_value_ptr(key_tune_item_t item)
{
    switch(item)
    {
        case KEY_TUNE_ITEM_ANGLE_P:
            return &imu_roll_balance.angle_cycle.p;
        case KEY_TUNE_ITEM_ANGLE_I:
            return &imu_roll_balance.angle_cycle.i;
        case KEY_TUNE_ITEM_ANGLE_D:
            return &imu_roll_balance.angle_cycle.d;
        case KEY_TUNE_ITEM_GYRO_P:
            return &imu_roll_balance.angular_speed_cycle.p;
        case KEY_TUNE_ITEM_ZERO:
            return &imu_roll_balance.posture_value.mechanical_zero;
        case KEY_TUNE_ITEM_CORRECT_KP:
            return &imu_roll_balance.posture_value.correct_kp;
        case KEY_TUNE_ITEM_CORRECT_KI:
            return &imu_roll_balance.posture_value.correct_ki;
        case KEY_TUNE_ITEM_NAG_TURN_GAIN:
            return &nag_navigation_turn_gain;
        case KEY_TUNE_ITEM_NAG_TURN_TRIM:
            return &nag_navigation_turn_trim;
        case KEY_TUNE_ITEM_NAG_SPEED:
            return &nag_navigation_travel_speed;
        case KEY_TUNE_ITEM_NAG_SPEED_P:
            return &imu_roll_balance.speed_cycle.p;
        case KEY_TUNE_ITEM_NAG_RECORD:
        case KEY_TUNE_ITEM_NAG_SAVE:
        case KEY_TUNE_ITEM_NAG_REPLAY:
        case KEY_TUNE_ITEM_NAG_STRAIGHT:
            return &g_key_tune_dummy_value;
        default:
            return &imu_roll_balance.angle_cycle.p;
    }
}

static float * key_tune_current_value_ptr(void)
{
    return key_tune_item_value_ptr(g_key_tune_item);
}

static float key_tune_current_delta(void)
{
    return key_tune_get_item_step(g_key_tune_item);
}

static void key_tune_select_first_item_on_page(void)
{
    switch(g_key_tune_page)
    {
        case KEY_TUNE_PAGE_PID:
            g_key_tune_item = g_key_tune_pid_items[0];
            break;
        case KEY_TUNE_PAGE_IMU:
            g_key_tune_item = g_key_tune_imu_items[0];
            break;
        case KEY_TUNE_PAGE_NAG:
            g_key_tune_item = g_key_tune_nag_items[0];
            break;
        case KEY_TUNE_PAGE_STR:
            g_key_tune_item = g_key_tune_str_items[0];
            break;
        default:
            g_key_tune_item = KEY_TUNE_ITEM_ANGLE_P;
            break;
    }
}

static void key_tune_select_next_item(void)
{
    if(KEY_TUNE_PAGE_PID == g_key_tune_page)
    {
        for(uint32_t i = 0; i < (sizeof(g_key_tune_pid_items) / sizeof(g_key_tune_pid_items[0])); i++)
        {
            if(g_key_tune_pid_items[i] == g_key_tune_item)
            {
                g_key_tune_item = g_key_tune_pid_items[(i + 1U) % (sizeof(g_key_tune_pid_items) / sizeof(g_key_tune_pid_items[0]))];
                return;
            }
        }
        g_key_tune_item = g_key_tune_pid_items[0];
    }
    else if(KEY_TUNE_PAGE_IMU == g_key_tune_page)
    {
        for(uint32_t i = 0; i < (sizeof(g_key_tune_imu_items) / sizeof(g_key_tune_imu_items[0])); i++)
        {
            if(g_key_tune_imu_items[i] == g_key_tune_item)
            {
                g_key_tune_item = g_key_tune_imu_items[(i + 1U) % (sizeof(g_key_tune_imu_items) / sizeof(g_key_tune_imu_items[0]))];
                return;
            }
        }
        g_key_tune_item = g_key_tune_imu_items[0];
    }
    else if(KEY_TUNE_PAGE_NAG == g_key_tune_page)
    {
        for(uint32_t i = 0; i < (sizeof(g_key_tune_nag_items) / sizeof(g_key_tune_nag_items[0])); i++)
        {
            if(g_key_tune_nag_items[i] == g_key_tune_item)
            {
                g_key_tune_item = g_key_tune_nag_items[(i + 1U) % (sizeof(g_key_tune_nag_items) / sizeof(g_key_tune_nag_items[0]))];
                return;
            }
        }
        g_key_tune_item = g_key_tune_nag_items[0];
    }
    else if(KEY_TUNE_PAGE_STR == g_key_tune_page)
    {
        for(uint32_t i = 0; i < (sizeof(g_key_tune_str_items) / sizeof(g_key_tune_str_items[0])); i++)
        {
            if(g_key_tune_str_items[i] == g_key_tune_item)
            {
                g_key_tune_item = g_key_tune_str_items[(i + 1U) % (sizeof(g_key_tune_str_items) / sizeof(g_key_tune_str_items[0]))];
                return;
            }
        }
        g_key_tune_item = g_key_tune_str_items[0];
    }
}

static void key_tune_apply_delta(float delta)
{
    float * value = key_tune_current_value_ptr();

    switch(g_key_tune_item)
    {
        case KEY_TUNE_ITEM_ANGLE_P:
            *value = key_tune_limit(*value + delta, 0.0f, 1500.0f);
            break;
        case KEY_TUNE_ITEM_ANGLE_I:
            *value = key_tune_limit(*value + delta, 0.0f, 10.0f);
            break;
        case KEY_TUNE_ITEM_ANGLE_D:
            *value = key_tune_limit(*value + delta, 0.0f, 200.0f);
            break;
        case KEY_TUNE_ITEM_GYRO_P:
            *value = key_tune_limit(*value + delta, 0.0f, 5.0f);
            break;
        case KEY_TUNE_ITEM_ZERO:
            *value = key_tune_limit(*value + delta, -10.0f, 10.0f);
            break;
        case KEY_TUNE_ITEM_CORRECT_KP:
            *value = key_tune_limit(*value + delta, 0.0f, 5.0f);
            break;
        case KEY_TUNE_ITEM_CORRECT_KI:
            *value = key_tune_limit(*value + delta, 0.0f, 0.2f);
            break;
        case KEY_TUNE_ITEM_NAG_TURN_GAIN:
            *value = key_tune_limit(*value + delta, -20.0f, 20.0f);
            break;
        case KEY_TUNE_ITEM_NAG_TURN_TRIM:
            *value = key_tune_limit(*value + delta, -250.0f, 250.0f);
            break;
        case KEY_TUNE_ITEM_NAG_SPEED:
            *value = key_tune_limit(*value + delta, 0.0f, 1000.0f);
            break;
        case KEY_TUNE_ITEM_NAG_SPEED_P:
            *value = key_tune_limit(*value + delta, 0.0f, 20.0f);
            break;
        default:
            break;
    }

    g_key_tune_changed = true;
}

static bool key_tune_take_event(uint8_t index)
{
    bool event = g_key_tune_keys[index].release_event;
    g_key_tune_keys[index].release_event = false;
    return event;
}

void key_tune_init(void)
{
    const uint32_t pin_cfg = ((uint32_t) IOPORT_CFG_PORT_DIRECTION_INPUT |
                              (uint32_t) IOPORT_CFG_PULLUP_ENABLE);

    for(uint32_t i = 0; i < KEY_TUNE_KEY_COUNT; i++)
    {
        (void) g_ioport.p_api->pinCfg(g_ioport.p_ctrl, g_key_tune_keys[i].pin, pin_cfg);
        g_key_tune_keys[i].stable_level = BSP_IO_LEVEL_HIGH;
        g_key_tune_keys[i].last_raw_level = BSP_IO_LEVEL_HIGH;
        g_key_tune_keys[i].debounce_count = 0U;
        g_key_tune_keys[i].press_event = false;
        g_key_tune_keys[i].release_event = false;
    }

}

void key_tune_update(void)
{
    for(uint32_t i = 0; i < KEY_TUNE_KEY_COUNT; i++)
    {
        bsp_io_level_t raw_level = BSP_IO_LEVEL_HIGH;
        (void) g_ioport.p_api->pinRead(g_ioport.p_ctrl, g_key_tune_keys[i].pin, &raw_level);

        if(raw_level == g_key_tune_keys[i].last_raw_level)
        {
            if(g_key_tune_keys[i].debounce_count < KEY_TUNE_DEBOUNCE_TICKS)
            {
                g_key_tune_keys[i].debounce_count++;
            }
        }
        else
        {
            g_key_tune_keys[i].last_raw_level = raw_level;
            g_key_tune_keys[i].debounce_count = 0U;
        }

        if((g_key_tune_keys[i].debounce_count >= KEY_TUNE_DEBOUNCE_TICKS) &&
           (raw_level != g_key_tune_keys[i].stable_level))
        {
            bsp_io_level_t old_level = g_key_tune_keys[i].stable_level;
            g_key_tune_keys[i].stable_level = raw_level;
            if((old_level == BSP_IO_LEVEL_HIGH) && (raw_level == BSP_IO_LEVEL_LOW))
            {
                g_key_tune_keys[i].press_event = true;
            }
            else if((old_level == BSP_IO_LEVEL_LOW) && (raw_level == BSP_IO_LEVEL_HIGH))
            {
                g_key_tune_keys[i].release_event = true;
            }
        }
    }

    if(camera_display_mode_active())
    {
        /* 鎽勫儚澶?璺熼殢妯″紡锛氭寜閿拷鐣ワ紝浣嗚鎶婁簨浠舵竻鎺夛紝閬垮厤閫€鍑烘ā寮忓悗娈嬬暀璇Е鍙?*/
        (void) key_tune_take_event(0U);
        (void) key_tune_take_event(1U);
        (void) key_tune_take_event(2U);
        (void) key_tune_take_event(3U);
    }
    else if(key_tune_take_event(0U))    /* K1 */
    {
        if((KEY_TUNE_PAGE_NAG == g_key_tune_page) || (KEY_TUNE_PAGE_STR == g_key_tune_page))
        {
            switch(g_key_tune_item)
            {
                case KEY_TUNE_ITEM_NAG_RECORD:
                    nag_navigation_start_record();
                    break;
                case KEY_TUNE_ITEM_NAG_SAVE:
                    nag_navigation_stop_record();
                    break;
                case KEY_TUNE_ITEM_NAG_REPLAY:
                    nag_navigation_start_replay();
                    break;
                case KEY_TUNE_ITEM_NAG_STRAIGHT:
                    nag_navigation_start_straight();
                    break;
                case KEY_TUNE_ITEM_NAG_TURN_GAIN:
                case KEY_TUNE_ITEM_NAG_TURN_TRIM:
                case KEY_TUNE_ITEM_NAG_SPEED:
                case KEY_TUNE_ITEM_NAG_SPEED_P:
                    key_tune_apply_delta(-key_tune_current_delta());
                    break;
                default:
                    break;
            }
            g_key_tune_changed = true;
        }
        else if(KEY_TUNE_PAGE_RUN != g_key_tune_page)
        {
            key_tune_apply_delta(-key_tune_current_delta());
        }
    }
    if(key_tune_take_event(1U))    /* K2 */
    {
        if((KEY_TUNE_PAGE_NAG == g_key_tune_page) || (KEY_TUNE_PAGE_STR == g_key_tune_page))
        {
            if((KEY_TUNE_ITEM_NAG_TURN_GAIN == g_key_tune_item) ||
               (KEY_TUNE_ITEM_NAG_TURN_TRIM == g_key_tune_item) ||
               (KEY_TUNE_ITEM_NAG_SPEED == g_key_tune_item) ||
               (KEY_TUNE_ITEM_NAG_SPEED_P == g_key_tune_item))
            {
                key_tune_apply_delta(key_tune_current_delta());
            }
            else
            {
                key_tune_select_next_item();
            }
            g_key_tune_changed = true;
        }
        else if(KEY_TUNE_PAGE_RUN != g_key_tune_page)
        {
            key_tune_apply_delta(key_tune_current_delta());
        }
    }
    if(key_tune_take_event(2U))    /* K3 */
    {
        if(KEY_TUNE_PAGE_RUN != g_key_tune_page)
        {
            key_tune_select_next_item();
            g_key_tune_changed = true;
        }
    }
    if(key_tune_take_event(3U))    /* K4锛氬惊鐜垏椤?RUN -> PID -> IMU -> NAG -> STR -> RUN */
    {
        g_key_tune_page = (key_tune_page_t)(((int) g_key_tune_page + 1) % (int) KEY_TUNE_PAGE_MAX);
        key_tune_select_first_item_on_page();
        g_key_tune_changed = true;
    }
}

void key_tune_get_status(key_tune_status_t *status)
{
    if(NULL != status)
    {
        status->page = g_key_tune_page;
        status->item = g_key_tune_item;
        status->value = *key_tune_current_value_ptr();
        status->step = key_tune_get_item_step(g_key_tune_item);
        status->changed = g_key_tune_changed;
        g_key_tune_changed = false;
    }
}

const char * key_tune_get_item_name(key_tune_item_t item)
{
    switch(item)
    {
        case KEY_TUNE_ITEM_ANGLE_P:
            return "AP";
        case KEY_TUNE_ITEM_ANGLE_I:
            return "AI";
        case KEY_TUNE_ITEM_ANGLE_D:
            return "AD";
        case KEY_TUNE_ITEM_GYRO_P:
            return "GP";
        case KEY_TUNE_ITEM_ZERO:
            return "Z0";
        case KEY_TUNE_ITEM_CORRECT_KP:
            return "CKP";
        case KEY_TUNE_ITEM_CORRECT_KI:
            return "CKI";
        case KEY_TUNE_ITEM_NAG_RECORD:
            return "REC";
        case KEY_TUNE_ITEM_NAG_SAVE:
            return "SAV";
        case KEY_TUNE_ITEM_NAG_REPLAY:
            return "REP";
        case KEY_TUNE_ITEM_NAG_STRAIGHT:
            return "STR";
        case KEY_TUNE_ITEM_NAG_TURN_GAIN:
            return "GAIN";
        case KEY_TUNE_ITEM_NAG_TURN_TRIM:
            return "TRIM";
        case KEY_TUNE_ITEM_NAG_SPEED:
            return "SPD";
        case KEY_TUNE_ITEM_NAG_SPEED_P:
            return "SPDP";
        default:
            return "--";
    }
}

const char * key_tune_get_page_name(key_tune_page_t page)
{
    switch(page)
    {
        case KEY_TUNE_PAGE_RUN:
            return "RUN";
        case KEY_TUNE_PAGE_PID:
            return "PID";
        case KEY_TUNE_PAGE_IMU:
            return "IMU";
        case KEY_TUNE_PAGE_NAG:
            return "NAG";
        case KEY_TUNE_PAGE_STR:
            return "STR";
        default:
            return "---";
    }
}

bool key_tune_item_is_selected(key_tune_item_t item)
{
    return item == g_key_tune_item;
}

float key_tune_get_item_value(key_tune_item_t item)
{
    return *key_tune_item_value_ptr(item);
}

float key_tune_get_item_step(key_tune_item_t item)
{
    switch(item)
    {
        case KEY_TUNE_ITEM_ANGLE_P:
            return 10.0f;
        case KEY_TUNE_ITEM_ANGLE_I:
            return 0.1f;
        case KEY_TUNE_ITEM_ANGLE_D:
            return 1.0f;
        case KEY_TUNE_ITEM_GYRO_P:
            return 0.05f;
        case KEY_TUNE_ITEM_ZERO:
            return 0.1f;
        case KEY_TUNE_ITEM_CORRECT_KP:
            return 0.05f;
        case KEY_TUNE_ITEM_CORRECT_KI:
            return 0.001f;
        case KEY_TUNE_ITEM_NAG_TURN_GAIN:
            return 0.5f;
        case KEY_TUNE_ITEM_NAG_TURN_TRIM:
            return 10.0f;
        case KEY_TUNE_ITEM_NAG_SPEED:
            return 20.0f;
        case KEY_TUNE_ITEM_NAG_SPEED_P:
            return 0.5f;
        default:
            return 1.0f;
    }
}
