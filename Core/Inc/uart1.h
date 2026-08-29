/**
 ******************************************************************************
 * @file    uart1.h
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

#ifndef UART1_H
#define UART1_H

/**
 * @brief  Run the UART1 console loop (blocks forever). Reads RX bytes from a
 *         FreeRTOS stream buffer and parses line-oriented commands to read or
 *         set the internal RTC. Intended to be called from a dedicated task.
 */
void uart_console_run(void);

#endif /* UART1_H */
