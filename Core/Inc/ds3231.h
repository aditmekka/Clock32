/**
 ******************************************************************************
 * @file    ds3231.h
 * @brief   Driver for the DS3231 precision RTC over I2C1.
 * @author  aditmekka
 ******************************************************************************
 */

#ifndef DS3231_H
#define DS3231_H

#include "get_time.h"

/* DS3231 7-bit I2C address. */
#define DS3231_I2C_ADDR     (0x68)

/**
 * @brief  Read the current time/date from the DS3231 into *time.
 * @param  time Pointer to the TimeDate_t to fill.
 * @retval 0 on success, non-zero if the I2C transfer failed or time is NULL.
 */
uint8_t ds3231_get_time(TimeDate_t *time);

#endif /* DS3231_H */
