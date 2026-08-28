/**
 ******************************************************************************
 * @file    display_draw.h
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

#ifndef DISPLAY_DRAW_H
#define DISPLAY_DRAW_H

#include "main.h"
#include <stdio.h>
#include "u8g2.h"
#include "u8g2_stm32_hal.h"
#include "get_time.h"

/* Global u8g2 context - defined in display_draw.c. */
extern u8g2_t u8g2;

void display_init(void);

void draw_clock_date(void);
void draw_stop_watch(uint32_t elapsed_ms);

#endif /* DISPLAY_DRAW_H */