#include "get_time.h"
#include "ds3231.h"

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