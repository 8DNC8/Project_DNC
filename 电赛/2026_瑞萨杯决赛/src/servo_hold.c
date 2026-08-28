#include "servo_hold.h"

static const bsp_io_port_pin_t g_servo_hold_pins[4] =
{
    SERVO_HOLD_CH1_PIN,
    SERVO_HOLD_CH2_PIN,
    SERVO_HOLD_CH3_PIN,
    SERVO_HOLD_CH4_PIN,
};

/* 姣忚矾鑸垫満鐩爣鑴夊锛坲s锛?*/
static uint16_t g_servo_hold_pulse[4];
static bool g_servo_hold_enabled = false;

static uint16_t servo_hold_clamp(uint16_t pulse_us)
{
    if(pulse_us < SERVO_HOLD_PULSE_MIN_US)
    {
        pulse_us = SERVO_HOLD_PULSE_MIN_US;
    }
    if(pulse_us > SERVO_HOLD_PULSE_MAX_US)
    {
        pulse_us = SERVO_HOLD_PULSE_MAX_US;
    }
    return pulse_us;
}

void servo_hold_init(void)
{
    uint32_t const pin_cfg = (uint32_t) IOPORT_CFG_DRIVE_HS_HIGH |
                             (uint32_t) IOPORT_CFG_PORT_DIRECTION_OUTPUT |
                             (uint32_t) IOPORT_CFG_PORT_OUTPUT_LOW;

    for(uint32_t i = 0; i < 4U; i++)
    {
        fsp_err_t err = g_ioport.p_api->pinCfg(g_ioport.p_ctrl, g_servo_hold_pins[i], pin_cfg);
        assert(FSP_SUCCESS == err);
        g_ioport.p_api->pinWrite(g_ioport.p_ctrl, g_servo_hold_pins[i], BSP_IO_LEVEL_LOW);
        g_servo_hold_pulse[i] = SERVO_HOLD_PULSE_US;
    }
    g_servo_hold_enabled = true;
}

void servo_hold_disable(void)
{
    g_servo_hold_enabled = false;
    for(uint32_t i = 0; i < 4U; i++)
    {
        g_ioport.p_api->pinWrite(g_ioport.p_ctrl, g_servo_hold_pins[i], BSP_IO_LEVEL_LOW);
    }
}

void servo_hold_enable(void)
{
    /* 寮曡剼鍦?init 鏃跺凡閰嶇疆涓鸿緭鍑猴紱閲嶆柊浣胯兘鍚庣敱 balance 鐨?20ms 鍒锋柊鎭㈠鑴夊杈撳嚭 */
    g_servo_hold_enabled = true;
}

void servo_hold_set_pulse(uint8_t channel, uint16_t pulse_us)
{
    if(channel >= 4U)
    {
        return;
    }
    g_servo_hold_pulse[channel] = servo_hold_clamp(pulse_us);
}

uint16_t servo_hold_get_pulse(uint8_t channel)
{
    if(channel >= 4U)
    {
        return SERVO_HOLD_PULSE_US;
    }
    return g_servo_hold_pulse[channel];
}

void servo_hold_refresh(void)
{
    if(!g_servo_hold_enabled)
    {
        return;
    }

    uint16_t remaining[4];
    bool active[4];
    uint16_t elapsed = 0;

    for(uint32_t i = 0; i < 4U; i++)
    {
        remaining[i] = g_servo_hold_pulse[i];
        active[i] = true;
        g_ioport.p_api->pinWrite(g_ioport.p_ctrl, g_servo_hold_pins[i], BSP_IO_LEVEL_HIGH);
    }

    /* 浜嬩欢椹卞姩锛氭寜鑴夊浠庡皬鍒板ぇ渚濇鎶婂搴旈€氶亾鎷変綆 */
    while(true)
    {
        uint16_t next = 0xFFFFU;
        bool any = false;

        for(uint32_t i = 0; i < 4U; i++)
        {
            if(active[i])
            {
                any = true;
                if(remaining[i] < next)
                {
                    next = remaining[i];
                }
            }
        }

        if(!any)
        {
            break;
        }

        if(next > elapsed)
        {
            R_BSP_SoftwareDelay((uint32_t)(next - elapsed), BSP_DELAY_UNITS_MICROSECONDS);
            elapsed = next;
        }

        for(uint32_t i = 0; i < 4U; i++)
        {
            if(active[i] && (remaining[i] == next))
            {
                g_ioport.p_api->pinWrite(g_ioport.p_ctrl, g_servo_hold_pins[i], BSP_IO_LEVEL_LOW);
                active[i] = false;
            }
        }
    }
}

void servo_hold_refresh_pulse(uint32_t pulse_us)
{
    uint16_t us = servo_hold_clamp((uint16_t) pulse_us);
    for(uint32_t i = 0; i < 4U; i++)
    {
        servo_hold_set_pulse((uint8_t) i, us);
    }
    servo_hold_refresh();
}

void servo_hold_run(void)
{
    while(true)
    {
        servo_hold_refresh();
    }
}
