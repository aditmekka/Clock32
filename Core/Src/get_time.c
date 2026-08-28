/**
 ******************************************************************************
 * @file    get_time.c
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

#include "get_time.h"
#include "ds3231.h"

/* NOTE: Planned migration - time will be read from the STM32 internal RTC
 *       instead of the DS3231 once the switch is made. */
TimeDate_t get_time(void) {
    TimeDate_t current_time;

    /* Read the current time/date from the DS3231 RTC over I2C1. */
    if (ds3231_get_time(&current_time) != 0) {
        /* I2C read failed - fall back to a sane default. */
        current_time.hours = 0;
        current_time.minutes = 0;
        current_time.seconds = 0;
        current_time.day = 27;
        current_time.month = 8;
        current_time.year = 2026;
    }

    return current_time;
}