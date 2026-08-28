/**
 ******************************************************************************
 * @file    app.c
 * @brief   tbd
 * @author  aditmekka
 * @version 1.0
 * @date    August 2026
 * @license GNU GPL v3
 *
 * @copyright
 *
 * This file is part of Clock32.
 *
 * Clock32 is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Clock32 is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Clock32.  If not, see <https://www.gnu.org/licenses/>.
 ******************************************************************************
 */

#include "app.h"
#include "cmsis_os2.h"
#include "hmi.h"
#include "display_draw.h"
#include "main.h"
#include "stm32f1xx_hal.h"
#include "u8g2.h"

static osMessageQueueId_t   app_queue;
static osMutexId_t          app_mutex;
static AppState_t           st;
static uint32_t             last_tick;

static void app_update_time(void){
    uint32_t now = HAL_GetTick();
    uint32_t dt = now - last_tick;
    last_tick = now;

    if(st.sw_running)   st.sw_elapsed_ms += dt;

    if(st.timer_state == TM_RUNNING){
        if(dt >= st.timer_remaining_ms){
            st.timer_remaining_ms = 0;
            st.timer_state = TM_DONE;
            st.alarm_active = true;
        }else{
            st.timer_remaining_ms -= dt;
        }
    }
}

static void timer_edit_field(int8_t detents){
    uint32_t m = st.timer_set_ms / 60000u;
    uint32_t s = (st.timer_set_ms / 1000u) % 60u;

    if(st.timer_field == TM_FIELD_MM){
        int mm = (int)m + detents;
        if(mm < 0)   mm = 0;
        if(mm > 99)  mm = 99;
        st.timer_set_ms = (uint32_t)mm * 60000u + s * 1000u;
    }else{ /* TM_FIELD_SS */
        int ss = (int)s + detents;
        if(ss < 0)  ss = 59;
        if(ss > 59) ss = 0;
        st.timer_set_ms = m * 60000u + (uint32_t)ss * 1000u;
    }

    st.timer_remaining_ms = st.timer_set_ms;
}

static void fsm_long_press(void){
    st.focused = false;
}

static void fsm_press(void){
    if (!st.focused) {                  /* focus current face */
        if (st.face == FACE_STOPWATCH || st.face == FACE_TIMER)
            st.focused = true;
        return;
    }

    switch(st.face){
    case FACE_STOPWATCH:
        if(st.sw_menu == SW_START_PAUSE) st.sw_running = !st.sw_running;
        else if(st.sw_menu == SW_RESET){
            st.sw_running = false; 
            st.sw_elapsed_ms = 0;
        } 
        break;
    case FACE_TIMER:
        switch(st.timer_state){
        case TM_IDLE:
            switch(st.timer_field){
            case TM_FIELD_MM:
            case TM_FIELD_SS:
                st.timer_editing = !st.timer_editing;   /* select / deselect field */
                break;
            case TM_FIELD_START_PAUSE:
                if(st.timer_remaining_ms > 0){
                    st.timer_editing = false;
                    st.timer_state = TM_RUNNING;
                }
                break;
            case TM_FIELD_RESET:
                st.timer_editing = false;
                st.timer_remaining_ms = st.timer_set_ms; /* back to configured time */
                break;
            default:
                break;
            }
            break;
        case TM_RUNNING:
            if(st.timer_field == TM_FIELD_RESET){
                st.timer_editing = false;
                st.timer_remaining_ms = st.timer_set_ms; /* reset to configured time */
                st.timer_state = TM_IDLE;
            }else{
                st.timer_state = TM_PAUSED;   /* START_PAUSE: pause */
            }
            break;
        case TM_PAUSED:
            switch(st.timer_field){
            case TM_FIELD_START_PAUSE:
                st.timer_state = TM_RUNNING;
                break;
            case TM_FIELD_RESET:
                st.timer_editing = false;
                st.timer_remaining_ms = st.timer_set_ms;
                st.timer_state = TM_IDLE;
                break;
            default:
                break;   /* can't edit fields while paused */
            }
            break;
        case TM_DONE:
            st.timer_state = TM_IDLE;
            st.timer_editing = false;
            st.timer_remaining_ms = st.timer_set_ms;
            st.alarm_active = false;
            break;
        default:
            break;
        }
        break;
    default:
        break;
    }
}

static void fsm_rotated(int8_t detents){
    if(!st.focused){
        int f = ((int)st.face + detents + 3) % 3;
        st.face = (Face_t)f;
    }else{
        switch(st.face){
        case FACE_STOPWATCH:
            if(detents != 0){
                st.sw_menu = (st.sw_menu == SW_RESET) ? SW_START_PAUSE : SW_RESET;
            }
            break;
        case FACE_TIMER:
            if(st.timer_state == TM_IDLE){
                if(st.timer_editing){
                    timer_edit_field(detents);   /* edit selected MM/SS value */
                }else{
                    int c = ((int)st.timer_field + detents + 4) % 4;
                    st.timer_field = (TmField_t)c; /* MM -> SS -> RESET -> START */
                }
            }else{
                /* Not idle: only the two action boxes are reachable. */
                if(detents != 0){
                    st.timer_field = (st.timer_field == TM_FIELD_RESET)
                                     ? TM_FIELD_START_PAUSE : TM_FIELD_RESET;
                }
            }
            break;
        default:
            break;
        }
    }
}

void app_init(osMessageQueueId_t q, osMutexId_t m){
    app_queue = q;
    app_mutex = m;
    st.face                = FACE_CLOCK;
    st.focused             = false;
    st.sw_running          = false;
    st.sw_elapsed_ms       = 0;
    st.sw_menu             = SW_START_PAUSE;
    st.timer_state         = TM_IDLE;
    st.timer_field         = TM_FIELD_MM;
    st.timer_editing       = false;
    st.timer_set_ms        = 5u * 60u * 1000u;   /* default 05:00 */
    st.timer_remaining_ms  = st.timer_set_ms;
    st.alarm_active        = false;
    last_tick              = HAL_GetTick();
}

void app_run(void *arg){
    (void)arg;
    EncoderEventMsg_t ev;
    for(;;){
        osStatus_t s = osMessageQueueGet(app_queue, &ev, NULL, 10u);

        app_update_time();

        if(s == osOK){
            switch (ev.event){
                case EV_ENC_ROTATED:    fsm_rotated(ev.value);  break;
                case EV_ENC_PRESS:      fsm_press();        break;
                case EV_ENC_LONG_PRESS: fsm_long_press();   break;
                default: break;
            }
        }
    }
}

void app_render(void){
    AppState_t snap;
    osMutexAcquire(app_mutex, osWaitForever);
    snap = st;
    osMutexRelease(app_mutex);

    u8g2_ClearBuffer(&u8g2);
    draw_face(&snap);
    u8g2_SendBuffer(&u8g2);
}

_Bool app_alarm_active(void){
    return st.alarm_active;
}
