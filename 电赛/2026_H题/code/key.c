#include "key.h"
#include "display.h"
#include "gray.h"
#include "control.h"
#include "beep.h"
#include "DUOJI.h"

/* key scan and action mapping; short-press triggers beep feedback */
void key_process(void)
{
    key_scanner();

    /* KEY_1: switch display page */
    if(key_get_state(KEY_1) == KEY_SHORT_PRESS)
    {
        beep_on();
        now_page++;
        if(now_page >= PAGE_NUM) now_page = 0;
        ips200_clear();
    }

    /* ===== PG:0 STATUS + MODE SELECTION ===== */
    if(now_page == 0)
    {
        /* KEY2 = cursor down */
        if(key_get_state(KEY_2) == KEY_SHORT_PRESS)
        {
            beep_on();
            menu_cursor++;
            if(menu_cursor > 4) menu_cursor = 0;
        }
        /* KEY3 = cursor up */
        if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
        {
            beep_on();
            if(menu_cursor == 0) menu_cursor = 4;
            else menu_cursor--;
        }
        /* KEY4 = confirm */
        if(key_get_state(KEY_4) == KEY_SHORT_PRESS)
        {
            beep_on();
            menu_confirm();
        }
    }

    /* ===== PG:1 MODE:4 PID TUNING (7 items: 0..6) ===== */
    else if(now_page == 1)
    {
        if(key_get_state(KEY_2) == KEY_SHORT_PRESS)
        {
            beep_on();
            mode4_tune_item++;
            if(mode4_tune_item >= 7) mode4_tune_item = 0;
            ips200_clear();
        }
        if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
        {
            beep_on();
            mode4_tune_change(+1);
        }
        if(key_get_state(KEY_4) == KEY_SHORT_PRESS)
        {
            beep_on();
            mode4_tune_change(-1);
        }
    }

    /* ===== PG:2 LINE TUNING (11 items: 0..10) ===== */
    else if(now_page == 2)
    {
        if(key_get_state(KEY_2) == KEY_SHORT_PRESS)
        {
            beep_on();
            tune_item++;
            if(tune_item >= 10) tune_item = 0;
            ips200_clear();
        }
        if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
        {
            beep_on();
            line_tune_change(1);
        }
        if(key_get_state(KEY_4) == KEY_SHORT_PRESS)
        {
            beep_on();
            line_tune_change(-1);
        }
    }

    /* ===== PG:4 SERVO PID TUNING ===== */
    else if(now_page == 4)
    {
        if(key_get_state(KEY_2) == KEY_SHORT_PRESS)
        {
            beep_on();
            servo_tune_item++;
            if(servo_tune_item >= 6) servo_tune_item = 0;
            ips200_clear();
        }
        if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
        {
            beep_on();
            servo_tune_change(servo_tune_item, +1);
        }
        if(key_get_state(KEY_4) == KEY_SHORT_PRESS)
        {
            beep_on();
            servo_tune_change(servo_tune_item, -1);
        }
    }

    /* ===== PG:5 Q4 BALL PID TUNING ===== */
    else if(now_page == 5)
    {
        if(key_get_state(KEY_2) == KEY_SHORT_PRESS)
        {
            beep_on();
            q4_tune_item++;
            if(q4_tune_item >= 3) q4_tune_item = 0;
            ips200_clear();
        }
        if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
        {
            beep_on();
            q4_tune_change(q4_tune_item, +1);
        }
        if(key_get_state(KEY_4) == KEY_SHORT_PRESS)
        {
            beep_on();
            q4_tune_change(q4_tune_item, -1);
        }
    }

    /* ===== PG:6 H3 OPEN LOOP TUNING ===== */
    else if(now_page == 6)
    {
        if(key_get_state(KEY_2) == KEY_SHORT_PRESS)
        {
            beep_on();
            h3_tune_item++;
            if(h3_tune_item >= 7) h3_tune_item = 0;
            ips200_clear();
        }
        if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
        {
            beep_on();
            h3_tune_change(h3_tune_item, +1);
        }
        if(key_get_state(KEY_4) == KEY_SHORT_PRESS)
        {
            beep_on();
            h3_tune_change(h3_tune_item, -1);
        }
    }

    /* ===== PG:3 K230 INFO (read-only, plus stop and old track) ===== */
    else
    {
        if(key_get_state(KEY_2) == KEY_SHORT_PRESS)
        {
            beep_on();
            if(car_move)
            {
                lap_time_20ms = systime_20ms;
                lap_done = 1;
            }
            task_stop();
        }
        if(key_get_state(KEY_3) == KEY_SHORT_PRESS)
        {
            beep_on();
            task3_start();
        }
        if(key_get_state(KEY_4) == KEY_SHORT_PRESS)
        {
            beep_on();
            task4_start();
        }
    }
}
