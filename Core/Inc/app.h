/**
 ******************************************************************************
 * @file    app.h
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

#ifndef APP_H
#define APP_H

#include "main.h"
#include "cmsis_os2.h"
#include "get_time.h"

typedef enum{
    FACE_CLOCK,
    FACE_STOPWATCH,
    FACE_TIMER,
    FACE_BATTERY,
    FACE_COUNT              /* number of faces, keep last */
}Face_t;

typedef enum{
    SW_START_PAUSE,
    SW_RESET
}SwMenuSel_t;

typedef enum{
    TM_IDLE,
    TM_RUNNING,
    TM_PAUSED,
    TM_DONE
}TmState_t;

typedef enum{
    TM_FIELD_MM,
    TM_FIELD_SS,
    TM_FIELD_RESET,
    TM_FIELD_START_PAUSE
}TmField_t;

typedef enum{
    CLK_FIELD_HOURS,
    CLK_FIELD_MINUTES,
    CLK_FIELD_SECONDS,
    CLK_FIELD_DAY,
    CLK_FIELD_MONTH,
    CLK_FIELD_YEAR,
    CLK_FIELD_COUNT         /* number of fields, keep last */
}ClkField_t;

typedef struct{
    Face_t      face;
    _Bool       focused;
    ClkField_t  clock_field;
    _Bool       clock_editing;
    TimeDate_t  clock_time;     /* frozen while the face is focused */
    _Bool       sw_running;
    uint32_t    sw_elapsed_ms;
    SwMenuSel_t sw_menu;
    TmState_t   timer_state;
    TmField_t   timer_field;
    _Bool       timer_editing;
    uint32_t    timer_set_ms;
    uint32_t    timer_remaining_ms;
    _Bool       alarm_active;
    uint8_t     battery_pct;
    uint16_t    battery_mv;
}AppState_t;

void app_init(osMessageQueueId_t q, osMutexId_t m);
void app_run(void *arg);
void app_render(void);
_Bool app_alarm_active(void);

#endif /* APP_H */