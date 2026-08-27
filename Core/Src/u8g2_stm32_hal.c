/*
 * u8g2_stm32_hal.c
 *
 *  Created on: Jul 22, 2026
 *      Author: aditmekka
 */

#include "u8g2_stm32_hal.h"
#include "main.h"

extern I2C_HandleTypeDef hi2c1;

uint8_t u8x8_gpio_and_delay_stm32(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr){
    switch(msg) {
        case U8X8_MSG_DELAY_MILLI:
            HAL_Delay(arg_int);
            break;
        case U8X8_MSG_GPIO_AND_DELAY_INIT:
            break;
        default:
            return 0;
    }
    return 1;
}

uint8_t u8x8_byte_stm32_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
    static uint8_t buffer[32];
    static uint8_t buf_idx;

    switch(msg) {
        case U8X8_MSG_BYTE_SEND:
        {
            uint8_t *data = (uint8_t *)arg_ptr;
            while (arg_int > 0) {
                buffer[buf_idx++] = *data;
                data++;
                arg_int--;
            }
            break;
        }
        case U8X8_MSG_BYTE_INIT:
            /* I2C already initialized by HAL */
            break;
        case U8X8_MSG_BYTE_START_TRANSFER:
            buf_idx = 0;
            break;
        case U8X8_MSG_BYTE_END_TRANSFER:
            if (buf_idx > 0) {
                HAL_I2C_Master_Transmit(&hi2c1,
                    u8x8_GetI2CAddress(u8x8) << 1,
                    buffer, buf_idx, HAL_MAX_DELAY);
            }
            break;
        default:
            return 0;
    }
    return 1;
}