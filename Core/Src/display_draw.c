/**
 ******************************************************************************
 * @file    display_draw.c
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

#include "display_draw.h"

/* Global u8g2 drawing context (extern in display_draw.h). */
u8g2_t u8g2;

void u8g2_DrawInt(u8g2_t *u8g2, uint8_t x, uint8_t y, int num) {
    char buffer[12];
    sprintf(buffer, "%d", num);
    u8g2_DrawStr(u8g2, x, y, buffer);
}

void u8g2_DrawIntZeroPad(u8g2_t *u8g2, uint8_t x, uint8_t y, int num, uint8_t digits) {
    char buffer[12];

    sprintf(buffer, "%0*d", digits, num);
    u8g2_DrawStr(u8g2, x, y, buffer);
}

void display_init(void){
    u8g2_Setup_sh1106_i2c_128x64_noname_f(&u8g2, U8G2_R0, u8x8_byte_stm32_hw_i2c, u8x8_gpio_and_delay_stm32);
    u8g2_SetI2CAddress(&u8g2, 0x3C);
    u8g2_InitDisplay(&u8g2);
    u8g2_SetPowerSave(&u8g2, 0);
    u8g2_ClearBuffer(&u8g2);
    u8g2_SendBuffer(&u8g2);
}

void draw_clock_date(void){
    TimeDate_t current_time = get_time();

    u8g2_SetBitmapMode(&u8g2, 1);
    u8g2_SetFontMode(&u8g2, 1);

    u8g2_SetFont(&u8g2, u8g2_font_profont17_tr);

    u8g2_DrawIntZeroPad(&u8g2, 20, 52, current_time.day, 2); //dd
    u8g2_DrawStr(&u8g2, 38, 52, "/");
    u8g2_DrawIntZeroPad(&u8g2, 47, 52, current_time.month, 2); //mm
    u8g2_DrawStr(&u8g2, 65, 52, "/");
    u8g2_DrawIntZeroPad(&u8g2, 74, 52, current_time.year, 4); //yyyy

    u8g2_DrawIntZeroPad(&u8g2, 97, 31, current_time.seconds, 2); //ss

    u8g2_SetFont(&u8g2, u8g2_font_profont29_tr);
    u8g2_DrawIntZeroPad(&u8g2, 15, 31, current_time.hours, 2); //HH
    u8g2_DrawIntZeroPad(&u8g2, 62, 31, current_time.minutes, 2); //MM
    if(current_time.seconds % 2 == 0){
        u8g2_DrawStr(&u8g2, 47, 31, ":");
    }else{
        u8g2_DrawStr(&u8g2, 47, 31, " ");
    }

    u8g2_SetDrawColor(&u8g2, 2);
    u8g2_DrawBox(&u8g2, 13, 10, 103, 23);
}

void draw_stop_watch(uint32_t elapsed_ms){
    uint32_t total_seconds = elapsed_ms / 1000;

    uint32_t hours   = total_seconds / 3600;
    uint32_t minutes = (total_seconds / 60) % 60;
    uint32_t seconds = total_seconds % 60;

    u8g2_SetFont(&u8g2, u8g2_font_profont29_tr);

    u8g2_DrawIntZeroPad(&u8g2, 8, 42, hours, 2);
    u8g2_DrawStr(&u8g2, 39, 42, ":");

    u8g2_DrawIntZeroPad(&u8g2, 53, 42, minutes, 2);
    u8g2_DrawStr(&u8g2, 84, 42, ":");

    u8g2_SetFont(&u8g2, u8g2_font_profont22_tr);
    u8g2_DrawIntZeroPad(&u8g2, 98, 42, seconds, 2);
}