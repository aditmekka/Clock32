/**
 ******************************************************************************
 * @file    hmi.h
 * @brief   Header file for hmi.c
 * @brief   This file contains the function prototypes for hmi.c
 *          and the necessary includes for the HMI module.
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