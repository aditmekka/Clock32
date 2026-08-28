/**
 ******************************************************************************
 * @file    display_draw.h
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

#ifndef DISPLAY_DRAW_H
#define DISPLAY_DRAW_H

#include "main.h"
#include <stdio.h>
#include <stdbool.h>
#include "u8g2.h"
#include "u8g2_stm32_hal.h"
#include "get_time.h"
#include "app.h"

/* Global u8g2 context - defined in display_draw.c. */
extern u8g2_t u8g2;

void display_init(void);

void draw_face(const AppState_t *snap);

#endif /* DISPLAY_DRAW_H */