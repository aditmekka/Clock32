/**
 ******************************************************************************
 * @file    display_draw.c
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

#include "display_draw.h"
#include "app.h"

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

void draw_stop_watch(uint32_t elapsed_ms, _Bool is_running, SwMenuSel_t menu, _Bool is_focused){
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

    u8g2_SetFont(&u8g2, u8g2_font_profont12_tr);
    u8g2_DrawStr(&u8g2, 38, 12, "STOPWATCH");
    u8g2_DrawFrame(&u8g2, 36, 2, 57, 12);

    u8g2_SetFont(&u8g2, u8g2_font_profont15_tr);
    u8g2_DrawStr(&u8g2, 22, 59, "RESET");

    if(!is_running){
        u8g2_DrawStr(&u8g2, 71, 59, "START");
    }else{
        u8g2_DrawStr(&u8g2, 71, 59, "PAUSE");
    }

    u8g2_DrawFrame(&u8g2, 20, 48, 39, 13);
    u8g2_DrawFrame(&u8g2, 69, 48, 39, 13);

    if(is_focused){
        u8g2_SetDrawColor(&u8g2, 2);
        if(menu == SW_RESET){
            u8g2_DrawBox(&u8g2, 20, 48, 39, 13);      /* RESET box */
        }else{
            u8g2_DrawBox(&u8g2, 69, 48, 39, 13);      /* START/PAUSE box */
        }
        u8g2_DrawBox(&u8g2, 36, 2, 57, 12);
        u8g2_SetDrawColor(&u8g2, 1);
    }
}

void draw_timer(uint32_t remaining_ms, TmState_t state, TmField_t field, _Bool editing, _Bool is_focused){
    uint32_t total_seconds = remaining_ms / 1000;
    uint32_t minutes = total_seconds / 60;
    uint32_t seconds = total_seconds % 60;

    u8g2_SetFont(&u8g2, u8g2_font_profont12_tr);
    u8g2_DrawStr(&u8g2, 50, 12, "TIMER");
    u8g2_DrawFrame(&u8g2, 48, 2, 33, 12);

    u8g2_SetFont(&u8g2, u8g2_font_profont29_tr);
    u8g2_DrawStr(&u8g2, 58, 39, ":");
    u8g2_DrawIntZeroPad(&u8g2, 28, 40, minutes, 2);   //MM
    u8g2_DrawIntZeroPad(&u8g2, 71, 40, seconds, 2);   //SS

    u8g2_SetFont(&u8g2, u8g2_font_profont15_tr);
    u8g2_DrawStr(&u8g2, 22, 59, "RESET");
    if(state == TM_RUNNING){
        u8g2_DrawStr(&u8g2, 71, 59, "PAUSE");
    }else{
        u8g2_DrawStr(&u8g2, 71, 59, "START");
    }

    u8g2_DrawFrame(&u8g2, 20, 48, 39, 13);   //RESET BOX
    u8g2_DrawFrame(&u8g2, 69, 48, 39, 13);   //PAUSE/START BOX

    if(is_focused){
        u8g2_SetDrawColor(&u8g2, 2);
        u8g2_DrawBox(&u8g2, 48, 2, 33, 12);//Timer box as "selected" signal
        
        if(state == TM_IDLE){
            if(field == TM_FIELD_MM) u8g2_DrawFrame(&u8g2, 26, 19, 34, 23);   //MM hollow
            else if(field == TM_FIELD_SS) u8g2_DrawFrame(&u8g2, 69, 19, 34, 23);   //SS hollow           
        }
        switch(field){
        case TM_FIELD_MM:
            if(editing) u8g2_DrawBox(&u8g2, 26, 19, 34, 23);   //solid: selected
            break;
        case TM_FIELD_SS:
            if(editing) u8g2_DrawBox(&u8g2, 69, 19, 34, 23);
            break;
        case TM_FIELD_RESET:
            u8g2_DrawBox(&u8g2, 20, 48, 39, 13);
            break;
        case TM_FIELD_START_PAUSE:
            u8g2_DrawBox(&u8g2, 69, 48, 39, 13);
            break;
        default:
            break;
        }
        u8g2_SetDrawColor(&u8g2, 1);
    }

    /* Paused indicator: only shown when paused, direction restored after. */
    if(state == TM_PAUSED){
        u8g2_SetFont(&u8g2, u8g2_font_profont15_tr);
        u8g2_SetFontDirection(&u8g2, 3);
        u8g2_DrawStr(&u8g2, 10, 43, "PAUSED");
        u8g2_SetFontDirection(&u8g2, 0);
    }
}

void draw_face(const AppState_t *snap){
    switch(snap->face){
    case FACE_CLOCK:
        draw_clock_date();
        break;
    case FACE_STOPWATCH:
        draw_stop_watch(snap->sw_elapsed_ms,
                        snap->sw_running,
                        snap->sw_menu,
                        snap->focused);
        break;
    case FACE_TIMER:
        draw_timer(snap->timer_remaining_ms,
                snap->timer_state,
                snap->timer_field,
                snap->timer_editing,
                snap->focused);
        break;
    }

    /* Timer alarm: flash the whole screen (XOR) in sync with the buzzer. */
    if(snap->alarm_active && (((HAL_GetTick() / 200u) % 2u) != 0u)){
        u8g2_SetDrawColor(&u8g2, 2);
        u8g2_DrawBox(&u8g2, 0, 0, 128, 64);
        u8g2_SetDrawColor(&u8g2, 1);
    }
}