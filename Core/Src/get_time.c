/**
 ******************************************************************************
 * @file    get_time.c
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