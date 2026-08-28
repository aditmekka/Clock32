/**
 ******************************************************************************
 * @file    ds3231.h
 * @brief   Driver for the DS3231 precision RTC over I2C1.
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

/*
 * NOTE: Planned migration - this DS3231 driver is expected to be replaced by
 *       the STM32 internal RTC (RTC peripheral). Kept for now.
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
