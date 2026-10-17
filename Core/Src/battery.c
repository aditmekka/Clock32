/**
 ******************************************************************************
 * @file    battery.c
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

/*
 * Measurement path
 * ----------------
 * The 4k3/4k3 divider would draw ~0.5 mA from the cell permanently, so it hangs
 * on the drain of a PMOS high-side switch. A small NMOS on VBAT_EN pulls that
 * PMOS gate low, so VBAT_EN = 1 connects the divider and VBAT_EN = 0 leaves the
 * cell isolated.
 *
 * ADC1 runs in discontinuous mode over a 4-rank regular group, all four ranks on
 * ADC1_IN0, with 239.5 cycle sampling to suit the divider impedance. One
 * software trigger therefore converts the channel 4 times, and the results are
 * copied by circular DMA into bat_dma_buf: oversampling for free, at the cost of
 * one settling delay per reading.
 */

#include "battery.h"
#include "adc.h"
#include "cmsis_os2.h"

/* Circular DMA target: a trigger refills all BATTERY_ADC_SAMPLES slots. */
static uint16_t bat_dma_buf[BATTERY_ADC_SAMPLES];

/* Set from the DMA transfer complete interrupt once a whole group is buffered. */
static volatile _Bool bat_dma_done;

/* Calibration and DMA arming succeeded. */
static _Bool bat_hw_ok;

/* Last good reading, published to the display and the UART console. */
static volatile uint16_t bat_mv;
static volatile uint8_t  bat_pct;

/* Remaining charge of a rested 1S Li-ion cell under light load, in descending
 * millivolts; linearly interpolated between the points. */
static const uint16_t bat_curve_mv[] = {
    4200u, 4150u, 4110u, 4080u, 4020u, 3980u, 3950u, 3910u, 3870u, 3830u, 3790u, 3750u,
    3710u, 3670u, 3630u, 3590u, 3550u, 3500u, 3450u, 3400u, 3300u, 3200u, 3000u
};
static const uint8_t bat_curve_pct[] = {
    100u, 95u, 90u, 85u, 80u, 75u, 70u, 65u, 60u, 55u, 50u, 45u,
    40u, 35u, 30u, 25u, 20u, 15u, 10u, 7u, 3u, 1u, 0u
};

#define BATTERY_CURVE_LEN   (sizeof(bat_curve_mv) / sizeof(bat_curve_mv[0]))

/**
 * @brief  HAL weak override: the circular DMA copied a complete group, so the
 *         four raw samples are in bat_dma_buf.
 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc){
    if(hadc->Instance == ADC1){
        bat_dma_done = true;
    }
}

/**
 * @brief  Mean of all sampled conversions, scaled by the reference, the divider
 *         and the calibration trim, in mV.
 */
static uint16_t battery_sum_to_mv(uint32_t sum){
    /* mV = sum * VREF * (R_top + R_bottom) / R_bottom / (samples * full scale) */
    uint32_t num = sum * BATTERY_ADC_VREF_MV * BATTERY_DIV_NUM;
    uint32_t den = BATTERY_ADC_SAMPLES * BATTERY_GROUPS_PER_READ
                 * BATTERY_ADC_FULL_SCALE * BATTERY_DIV_DEN;
    uint32_t mv  = (num + (den / 2u)) / den;   /* rounded, not truncated */

    /* Second rounding, on the result: keeps the 32-bit products small. */
    return (uint16_t)(((mv * BATTERY_CAL_NUM) + (BATTERY_CAL_DEN / 2u))
                      / BATTERY_CAL_DEN);
}

/**
 * @brief  Cell voltage to remaining charge, interpolated on the curve above.
 */
static uint8_t battery_mv_to_pct(uint16_t mv){
    int i;

    if(mv >= bat_curve_mv[0]){
        return bat_curve_pct[0];
    }
    if(mv <= bat_curve_mv[BATTERY_CURVE_LEN - 1u]){
        return 0u;
    }

    for(i = 0; i < (int)(BATTERY_CURVE_LEN - 1u); i++){
        if(mv >= bat_curve_mv[i + 1]){
            uint16_t span = bat_curve_mv[i] - bat_curve_mv[i + 1];
            uint8_t  drop = bat_curve_pct[i] - bat_curve_pct[i + 1];

            return (uint8_t)(bat_curve_pct[i] -
                             (((bat_curve_mv[i] - mv) * drop) / span));
        }
    }

    return 0u;
}

void battery_init(void){
    /* Leave the cell isolated until the first reading. */
    HAL_GPIO_WritePin(VBAT_EN_GPIO_Port, VBAT_EN_Pin, GPIO_PIN_RESET);

    /* The self calibration stops and restarts the ADC, so it has to happen
     * before the DMA is armed. A failure is not fatal: the clock keeps running,
     * only the battery reading stays unavailable. */
    if(HAL_ADCEx_Calibration_Start(&hadc1) != HAL_OK){
        return;
    }

    /* Arm the circular DMA once; every later trigger refills the whole buffer,
     * so the transfer does not have to be restarted for each reading. The
     * conversion started here runs with the divider off and is discarded. */
    bat_dma_done = false;
    if(HAL_ADC_Start_DMA(&hadc1, (uint32_t *)bat_dma_buf, BATTERY_ADC_SAMPLES) != HAL_OK){
        return;
    }

    bat_hw_ok = true;
}

_Bool battery_sample(void){
    uint32_t sum = 0u;
    uint32_t start;
    uint8_t  g;
    uint8_t  i;

    if(!bat_hw_ok){
        return false;
    }

    /* Connect the divider and let it settle before the first sample-and-hold. */
    HAL_GPIO_WritePin(VBAT_EN_GPIO_Port, VBAT_EN_Pin, GPIO_PIN_SET);
    osDelay(BATTERY_SETTLE_MS);

    /* Several groups in a row, all inside the same settling window. Each group
     * is one software trigger converting the whole discontinuous group. */
    for(g = 0u; g < BATTERY_GROUPS_PER_READ; g++){
        bat_dma_done = false;
        start = HAL_GetTick();

        if(HAL_ADC_Start(&hadc1) != HAL_OK){
            HAL_GPIO_WritePin(VBAT_EN_GPIO_Port, VBAT_EN_Pin, GPIO_PIN_RESET);
            return false;
        }

        while(!bat_dma_done){
            if((HAL_GetTick() - start) > BATTERY_CONV_TIMEOUT_MS){
                HAL_GPIO_WritePin(VBAT_EN_GPIO_Port, VBAT_EN_Pin, GPIO_PIN_RESET);

                /* Re-arm so that a lost transfer does not disable sampling for
                 * good. Usually a sign of a group size mismatch, see battery.h. */
                (void)HAL_ADC_Stop_DMA(&hadc1);
                bat_dma_done = false;
                (void)HAL_ADC_Start_DMA(&hadc1, (uint32_t *)bat_dma_buf,
                                        BATTERY_ADC_SAMPLES);
                return false;
            }
            osDelay(1);
        }

        for(i = 0u; i < BATTERY_ADC_SAMPLES; i++){
            sum += bat_dma_buf[i];
        }
    }

    /* Done measuring: disconnect the divider again. */
    HAL_GPIO_WritePin(VBAT_EN_GPIO_Port, VBAT_EN_Pin, GPIO_PIN_RESET);

    bat_mv  = battery_sum_to_mv(sum);
    bat_pct = battery_mv_to_pct(bat_mv);

    return true;
}

uint8_t battery_percentage(void){
    return bat_pct;
}

uint16_t battery_millivolts(void){
    return bat_mv;
}
