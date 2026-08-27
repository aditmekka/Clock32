/*
 * u8g2_stm32_hal.h
 *
 *  Created on: Jul 22, 2026
 *      Author: aditmekka
 */

#ifndef __U8G2_STM32_HAL_H__
#define __U8G2_STM32_HAL_H__

#include "stm32f1xx_hal.h"
#include "u8g2.h"

// Configure your display here
extern I2C_HandleTypeDef hi2c1;

uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

#endif