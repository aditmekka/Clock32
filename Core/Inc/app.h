/**
 ******************************************************************************
 * @file    app.h
 * @brief   tbd
 * @author  aditmekka
 * @version 1.0
 * @date    August 2026
 * @license MIT License (Free to use with attribution)
 *
 * @copyright
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND.
 ******************************************************************************
 */

#ifndef APP_H
#define APP_H

#include "main.h"
#include "cmsis_os2.h"

typedef enum{
    FACE_CLOCK,
    FACE_STOPWATCH,
    FACE_TIMER
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

typedef struct{
    Face_t      face;
    _Bool       focused;
    _Bool       sw_running;
    uint32_t    sw_elapsed_ms;
    SwMenuSel_t sw_menu;
    TmState_t   timer_state;
    TmField_t   timer_field;
    _Bool       timer_editing;
    uint32_t    timer_set_ms;
    uint32_t    timer_remaining_ms;
    _Bool       alarm_active;
}AppState_t;

void app_init(osMessageQueueId_t q, osMutexId_t m);
void app_run(void *arg);
void app_render(void);
_Bool app_alarm_active(void);

#endif /* APP_H */