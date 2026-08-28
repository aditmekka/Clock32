/**
 ******************************************************************************
 * @file    ds3231.c
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

#include "ds3231.h"
#include "i2c.h"

/* DS3231 register map (timekeeping part). */
#define DS3231_REG_SECONDS    0x00
#define DS3231_REG_MINUTES    0x01
#define DS3231_REG_HOURS      0x02
#define DS3231_REG_DATE       0x04   /* day of month */
#define DS3231_REG_MONTH      0x05
#define DS3231_REG_YEAR       0x06

#define DS3231_I2C_TIMEOUT    100u

/**
 * @brief  Convert a BCD byte to binary (e.g. 0x27 -> 27).
 * @param  bcd BCD encoded value.
 * @retval Binary value.
 */
static uint8_t bcd2bin(uint8_t bcd)
{
    return (uint8_t)(((bcd >> 4) & 0x0Fu) * 10u + (bcd & 0x0Fu));
}

/**
 * @brief  Read the current time/date from the DS3231 into *time.
 * @param  time Pointer to the TimeDate_t to fill.
 * @retval 0 on success, non-zero if the I2C transfer failed or time is NULL.
 */
uint8_t ds3231_get_time(TimeDate_t *time)
{
    uint8_t reg = DS3231_REG_SECONDS;
    uint8_t data[7] = {0u};

    if (time == NULL)
    {
        return 1u;
    }

    /* Point the DS3231 register pointer at the seconds register. */
    if (HAL_I2C_Master_Transmit(&hi2c1, (uint16_t)(DS3231_I2C_ADDR << 1),
                                &reg, 1u, DS3231_I2C_TIMEOUT) != HAL_OK)
    {
        return 1u;
    }

    /* Read seconds..year in one shot (7 bytes). */
    if (HAL_I2C_Master_Receive(&hi2c1, (uint16_t)(DS3231_I2C_ADDR << 1),
                               data, 7u, DS3231_I2C_TIMEOUT) != HAL_OK)
    {
        return 1u;
    }

    uint8_t raw_hours = data[DS3231_REG_HOURS];

    /* Seconds register bit 7 = oscillator stop flag; ignore it here. */
    time->seconds = bcd2bin(data[DS3231_REG_SECONDS] & 0x7Fu);
    time->minutes = bcd2bin(data[DS3231_REG_MINUTES] & 0x7Fu);

    if (raw_hours & 0x40u)
    {
        /* 12-hour mode: bit 5 = PM, bits 4..0 = 1..12. */
        time->hours = bcd2bin(raw_hours & 0x1Fu);
        if ((raw_hours & 0x20u) && (time->hours != 12u))
        {
            time->hours += 12;              /* PM -> add 12 */
        }
        else if (!(raw_hours & 0x20u) && (time->hours == 12u))
        {
            time->hours = 0u;               /* 12 AM -> 0 */
        }
    }
    else
    {
        /* 24-hour mode: bits 5..0 encode the hour. */
        time->hours = bcd2bin(raw_hours & 0x3Fu);
    }

    time->day   = bcd2bin(data[DS3231_REG_DATE] & 0x3Fu);
    time->month = bcd2bin(data[DS3231_REG_MONTH] & 0x1Fu);
    /* The DS3231 only stores a 2-digit year; assume 21st century. */
    time->year  = (uint16_t)(2000u + bcd2bin(data[DS3231_REG_YEAR]));

    return 0u;
}
