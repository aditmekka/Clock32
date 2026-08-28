/**
 ******************************************************************************
 * @file    hmi.h
 * @brief   Header file for hmi.c
 * @brief   This file contains the function prototypes for hmi.c
 *          and the necessary includes for the HMI module.
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

#ifndef HMI_H
#define HMI_H

#include "main.h"

#define ENC_COUNTS_PER_DETENT 2

/* Rotary encoder input events.
 * A single rotary encoder (with integrated push button) provides all HMI
 * input on the PCB:
 *   - rotate CW            -> EV_ENC_ROTATED, value > 0 (detent count)
 *   - rotate CCW           -> EV_ENC_ROTATED, value < 0 (detent count)
 *   - button press         -> EV_ENC_PRESS
 *   - button hold          -> EV_ENC_LONG_PRESS
 */
typedef enum {
    EV_ENC_NONE,
    EV_ENC_ROTATED,
    EV_ENC_PRESS,
    EV_ENC_LONG_PRESS
} EncoderEvent_t;

typedef struct {
    EncoderEvent_t event;
    int8_t value;
} EncoderEventMsg_t;

EncoderEventMsg_t encoder_poll(void);

#endif /* HMI_H */