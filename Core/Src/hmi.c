/**
 ******************************************************************************
 * @file    hmi.c
 * @brief   implementation file for hmi.h
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

#include "hmi.h"

/* Time-based debounce for the encoder push button (ms). */
#define ENC_BTN_DEBOUNCE_MS     20u
/* Hold time before the encoder button is reported as a long press (ms). */
#define ENC_BTN_LONG_PRESS_MS   1000u

/**
 * @brief  Poll the rotary encoder and return the next event, if any.
 *
 * The encoder is connected to TIM3 in encoder mode (PA6/PA7), and its push
 * button to ENC_SW (PB0, active-low with internal pull-up).
 *
 * @retval EncoderEventMsg_t, with .event one of:
 *           - EV_ENC_ROTATED     .value = detents (+ = CW, - = CCW)
 *           - EV_ENC_PRESS       .value = 0 (debounced short press)
 *           - EV_ENC_LONG_PRESS  .value = 0 (held >= ENC_BTN_LONG_PRESS_MS)
 *           - EV_ENC_NONE        .value = 0 (nothing happened)
 */
EncoderEventMsg_t encoder_poll(void)
{
    EncoderEventMsg_t event_msg = {.event = EV_ENC_NONE, .value = 0};

    /* ------------------- Rotation -> EV_ENC_ROTATED ------------------- */
    static int16_t last_cnt = 0;

    int16_t current_cnt = (int16_t)TIM3->CNT;
    int16_t delta = current_cnt - last_cnt;

    if (delta != 0) {
        int8_t detents = delta / ENC_COUNTS_PER_DETENT;
        int16_t consumed = detents * ENC_COUNTS_PER_DETENT;

        last_cnt += consumed;

        if (detents != 0) {
            event_msg.event = EV_ENC_ROTATED;
            event_msg.value = detents;
            return event_msg;
        }
    }

    /* --------------- Encoder button -> PRESS / LONG_PRESS -------------- */
    static uint8_t btn_level    = 1u;  /* debounced level: 1 = released */
    static uint8_t btn_prev     = 1u;  /* previous debounced level      */
    static uint32_t change_tick = 0u;  /* last accepted level change    */
    static uint32_t press_tick  = 0u;  /* when the current press began  */
    static uint8_t long_sent    = 0u;  /* long press already reported   */

    uint32_t now = HAL_GetTick();
    uint8_t raw = (HAL_GPIO_ReadPin(ENC_SW_GPIO_Port, ENC_SW_Pin) == GPIO_PIN_RESET)
                      ? 0u : 1u;

    /* Debounce: only accept a level change once it has been stable. */
    if (raw != btn_level) {
        if ((now - change_tick) >= ENC_BTN_DEBOUNCE_MS) {
            btn_level = raw;
            change_tick = now;
            if (btn_level == 0u) {          /* just pressed */
                press_tick = now;
                long_sent = 0u;
            }
        }
    } else {
        change_tick = now;
    }

    /* Report the release edge (a short press) as EV_ENC_PRESS. */
    if (btn_level != btn_prev) {
        btn_prev = btn_level;
        if ((btn_level == 1u) && !long_sent) {
            event_msg.event = EV_ENC_PRESS;
        }
    }

    /* Report a long press while the button is still held. */
    if ((btn_level == 0u) && !long_sent) {
        if ((now - press_tick) >= ENC_BTN_LONG_PRESS_MS) {
            long_sent = 1u;
            event_msg.event = EV_ENC_LONG_PRESS;
        }
    }

    return event_msg;
}