/**
 ******************************************************************************
 * @file    uart1.c
 * @brief   UART1 interactive console to read/set the internal RTC.
 * @author  aditmekka
 * @version 1.0
 * @date    August 2026
 * @license GNU GPL v3
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

#include "uart1.h"
#include "usart.h"
#include "rtc.h"
#include "get_time.h"
#include "FreeRTOS.h"
#include "stream_buffer.h"
#include <stdio.h>
#include <string.h>

#define UART1_RX_BUF_SIZE   64u
#define UART1_LINE_MAX      48u

static StreamBufferHandle_t uart_rx_stream = NULL;
static uint8_t uart_rx_byte = 0u;

/**
 * @brief  Create the RX stream buffer and arm the first UART1 byte reception.
 */
static void uart_rx_start(void){
    if(uart_rx_stream == NULL){
        uart_rx_stream = xStreamBufferCreate(UART1_RX_BUF_SIZE, 1u);
    }
    HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1u);
}

/**
 * @brief  HAL weak override: a UART1 byte arrived. Push it to the console task
 *         and immediately re-arm reception for the next byte.
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){
    BaseType_t higher_prio = pdFALSE;

    if(huart->Instance != USART1){
        return;
    }

    if(uart_rx_stream != NULL){
        xStreamBufferSendFromISR(uart_rx_stream, &uart_rx_byte, 1u, &higher_prio);
        portYIELD_FROM_ISR(higher_prio);
    }

    HAL_UART_Receive_IT(&huart1, &uart_rx_byte, 1u);
}

static void uart_puts(const char *s){
    HAL_UART_Transmit(&huart1, (const uint8_t *)s, (uint16_t)strlen(s), 100u);
}

static void uart_print_help(void){
    uart_puts("\r\n"
              "UART1 RTC console\r\n"
              "  ?                            show this help\r\n"
              "  time                         show current RTC time\r\n"
              "  settime HH:MM:SS DD/MM/YYYY  set RTC time and date\r\n"
              "\r\n> ");
}

static void uart_print_time(void){
    TimeDate_t t;
    char buf[48];
    int n;

    if(rtc_get_time(&t) != 0u){
        uart_puts("Error: cannot read RTC\r\n");
        return;
    }

    n = snprintf(buf, sizeof(buf), "Time: %02u:%02u:%02u  Date: %02u/%02u/%04u\r\n",
                 (unsigned)t.hours, (unsigned)t.minutes, (unsigned)t.seconds,
                 (unsigned)t.day, (unsigned)t.month, (unsigned)t.year);
    if(n > 0){
        HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)n, 100u);
    }
}

/**
 * @brief  Parse an unsigned integer (1..max_digits digits) from *pp and
 *         advance *pp past it.
 * @retval The parsed value if min <= value <= max, else -1.
 */
static int parse_uint(const char **pp, int max_digits, int min, int max){
    const char *p = *pp;
    int val = 0;
    int digits = 0;

    while(digits < max_digits && *p >= '0' && *p <= '9'){
        val = val * 10 + (*p - '0');
        p++;
        digits++;
    }
    *pp = p;

    if(digits == 0 || val < min || val > max){
        return -1;
    }
    return val;
}

/**
 * @brief  Parse "HH:MM:SS DD/MM/YYYY" into *t. Done manually (no scanf) because
 *         the scanf family is very stack-hungry on this small MCU.
 * @retval 0 on success, 1 on any format or range error.
 */
static int parse_settime(const char *s, TimeDate_t *t){
    int hh, mm, ss, dd, mo, yy;

    while(*s == ' '){
        s++;   /* skip space(s) after the command word */
    }

    hh = parse_uint(&s, 2, 0, 23);
    if(hh < 0 || *s != ':') return 1;
    s++;
    mm = parse_uint(&s, 2, 0, 59);
    if(mm < 0 || *s != ':') return 1;
    s++;
    ss = parse_uint(&s, 2, 0, 59);
    if(ss < 0 || *s != ' ') return 1;
    s++;
    dd = parse_uint(&s, 2, 1, 31);
    if(dd < 0 || *s != '/') return 1;
    s++;
    mo = parse_uint(&s, 2, 1, 12);
    if(mo < 0 || *s != '/') return 1;
    s++;
    yy = parse_uint(&s, 4, 2000, 2099);
    if(yy < 0 || *s != '\0') return 1;

    t->hours   = (uint8_t)hh;
    t->minutes = (uint8_t)mm;
    t->seconds = (uint8_t)ss;
    t->day     = (uint8_t)dd;
    t->month   = (uint8_t)mo;
    t->year    = (uint16_t)yy;
    return 0;
}

static void uart_handle_cmd(const char *line){
    char cmd[12];
    size_t i = 0u;

    /* Copy and upper-case the command word (everything before the first space). */
    while((i + 1u) < sizeof(cmd) && line[i] != '\0' && line[i] != ' '){
        char c = line[i];
        if(c >= 'a' && c <= 'z'){
            c = (char)(c - 'a' + 'A');
        }
        cmd[i] = c;
        i++;
    }
    cmd[i] = '\0';

    if(strcmp(cmd, "?") == 0 || strcmp(cmd, "HELP") == 0){
        uart_print_help();
    }else if(strcmp(cmd, "TIME") == 0){
        uart_print_time();
    }else if(strcmp(cmd, "SETTIME") == 0){
        TimeDate_t t;

        if(parse_settime(line + i, &t) == 0){
            if(rtc_set_time(&t) == 0u){
                uart_puts("OK, RTC set: ");
                uart_print_time();
            }else{
                uart_puts("Error: values out of range\r\n");
            }
        }else{
            uart_puts("Usage: settime HH:MM:SS DD/MM/YYYY\r\n");
        }
    }else{
        uart_puts("Unknown command. Send '?' for help.\r\n");
    }
}

/**
 * @brief  UART1 console loop. Never returns; intended to run as its own task.
 */
void uart_console_run(void){
    char line[UART1_LINE_MAX];
    size_t len = 0u;
    uint8_t ch;

    uart_rx_start();
    uart_print_help();

    for(;;){
        if(xStreamBufferReceive(uart_rx_stream, &ch, 1u, portMAX_DELAY) == 0u){
            continue;
        }

        if(ch == '\r' || ch == '\n'){
            if(len > 0u){
                line[len] = '\0';
                uart_puts("\r\n");
                uart_handle_cmd(line);
                uart_puts("> ");
                len = 0u;
            }
        }else if(ch == 0x08u || ch == 0x7Fu){
            /* Backspace / DEL. */
            if(len > 0u){
                len--;
                uart_puts("\b \b");
            }
        }else{
            if(len < (sizeof(line) - 1u)){
                line[len] = (char)ch;
                len++;
                HAL_UART_Transmit(&huart1, &ch, 1u, 100u);
            }
        }
    }
}
