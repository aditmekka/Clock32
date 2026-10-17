/**
 ******************************************************************************
 * @file    battery.h
 * @brief   Battery charge measurement (switched divider on ADC1_IN0).
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

#ifndef BATTERY_H
#define BATTERY_H

#include "main.h"
#include <stdbool.h>

/* ADC1_IN0: 12-bit, right aligned, VDDA used as the reference.
 *
 * VDDA is the scale of the whole measurement, so an unknown rail shows up as a
 * percentage error, not an offset: 3300 * 3620 / 3599 = 3319 mV explains a cell
 * measured at 3620 mV reading back as 3599 mV. Divider tolerance scales the
 * reading exactly the same way, so BATTERY_CAL_NUM/DEN below corrects both -
 * measure the cell with a DMM, then set the raw reading and the true value. */
#define BATTERY_ADC_FULL_SCALE      4095u
#define BATTERY_ADC_VREF_MV         3300u

/* Scale calibration against a reference DMM (equal values = no trim). */
#define BATTERY_CAL_NUM             3620u
#define BATTERY_CAL_DEN             3599u

/* Divider between VBAT and the ADC pin: 4k3 over 4k3, so the pin sees half of
 * the cell voltage:  VBAT = V(PA0) * (R_top + R_bottom) / R_bottom. */
#define BATTERY_DIV_NUM             2u
#define BATTERY_DIV_DEN             1u

/* One software trigger converts the whole discontinuous group: the 4 ranks of
 * ADC1_IN0 (4 samples of the same channel) land in the circular DMA buffer.
 * Requires "Number Of Discontinuous Conversions" = 4 in CubeMX. */
#define BATTERY_ADC_SAMPLES         4u

/* Groups converted per reading. They run back to back inside one settling
 * window, so a reading averages BATTERY_ADC_SAMPLES * BATTERY_GROUPS_PER_READ
 * conversions: 1 LSB is 1.6 mV at the cell, and averaging 16 samples keeps the
 * noise of the raw readout down to roughly a single LSB. */
#define BATTERY_GROUPS_PER_READ     4u

/* Divider settling time after VBAT_EN turns the PMOS on (5 ms covers the switch
 * and a filter cap of a few hundred nF), and the worst case for one group of 4
 * conversions (4 * 252 ADC clocks at 12 MHz is ~84 us). Raise the settle time
 * if the divider node carries a large filter capacitor. */
#define BATTERY_SETTLE_MS           5u
#define BATTERY_CONV_TIMEOUT_MS     20u

/* Refresh period of the cached reading (the divider draws ~0.5 mA while on). */
#define BATTERY_SAMPLE_PERIOD_MS    10000u

/* Calibrate the ADC and arm the circular DMA. Call once, before the scheduler. */
void battery_init(void);

/* Switch the divider on, trigger one group, average it, switch it back off.
 * Returns false (and keeps the previous reading) if the group does not arrive. */
_Bool battery_sample(void);

/* Last successful reading. */
uint8_t battery_percentage(void);
uint16_t battery_millivolts(void);

#endif /* BATTERY_H */
