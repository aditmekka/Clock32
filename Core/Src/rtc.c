/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    rtc.c
  * @brief   This file provides code for the configuration
  *          of the RTC instances.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "rtc.h"

/* USER CODE BEGIN 0 */
/* Magic value marking that the RTC epoch was already configured. */
#define RTC_BKP_MAGIC 0x32F2u

/* Days from 1970-01-01 to the epoch base used by this RTC. The counter holds
   whole seconds since 2000-01-01 00:00:00, so one uint32_t carries both the
   time and the date (valid until year 2136). */
#define RTC_EPOCH_1970_DAYS  10957

/**
 * @brief  Days since 1970-01-01 for a Gregorian date (Hinnant algorithm).
 */
static int32_t days_from_civil(int32_t y, uint32_t m, uint32_t d){
    y -= (m <= 2u) ? 1 : 0;
    int32_t era = (y >= 0 ? y : y - 399) / 400;
    uint32_t yoe = (uint32_t)(y - era * 400);                    /* [0, 399] */
    uint32_t doy = (153u * ((m > 2u) ? (m - 3u) : (m + 9u)) + 2u) / 5u + d - 1u;
    uint32_t doe = yoe * 365u + yoe / 4u - yoe / 100u + doy;
    return era * 146097 + (int32_t)doe - 719468;
}

/**
 * @brief  Gregorian date for a day count since 1970-01-01 (Hinnant algorithm).
 */
static void civil_from_days(int32_t z, int32_t *y, uint32_t *m, uint32_t *d){
    z += 719468;
    int32_t era = (z >= 0 ? z : z - 146096) / 146097;
    uint32_t doe = (uint32_t)(z - era * 146097);                 /* [0, 146096] */
    uint32_t yoe = (doe - doe / 1460u + doe / 36524u - doe / 146096u) / 365u;
    int32_t yy = (int32_t)yoe + era * 400;
    uint32_t doy = doe - (365u * yoe + yoe / 4u - yoe / 100u);
    uint32_t mp = (5u * doy + 2u) / 153u;
    uint32_t dd = doy - (153u * mp + 2u) / 5u + 1u;
    uint32_t mm = (mp < 10u) ? (mp + 3u) : (mp - 9u);
    *y = yy + ((mm <= 2u) ? 1 : 0);
    *m = mm;
    *d = dd;
}

/**
 * @brief  Read the RTC counter (epoch seconds), safe against a mid-read
 *         rollover of the high word.
 */
static uint32_t rtc_read_counter(void){
    uint16_t hi1 = (uint16_t)(hrtc.Instance->CNTH & RTC_CNTH_RTC_CNT);
    uint16_t lo  = (uint16_t)(hrtc.Instance->CNTL & RTC_CNTL_RTC_CNT);
    uint16_t hi2 = (uint16_t)(hrtc.Instance->CNTH & RTC_CNTH_RTC_CNT);

    if(hi1 != hi2){
        hi1 = hi2;                  /* counter rolled over while reading */
        lo  = (uint16_t)(hrtc.Instance->CNTL & RTC_CNTL_RTC_CNT);
    }
    return ((uint32_t)hi1 << 16) | lo;
}

/**
 * @brief  Write the RTC counter (epoch seconds).
 */
static void rtc_write_counter(uint32_t value){
    while((hrtc.Instance->CRL & RTC_CRL_RTOFF) == 0u){
    }
    hrtc.Instance->CRL |= RTC_CRL_CNF;                  /* enter config mode */
    hrtc.Instance->CNTH = (uint16_t)((value >> 16) & 0xFFFFu);
    hrtc.Instance->CNTL = (uint16_t)(value & 0xFFFFu);
    hrtc.Instance->CRL &= (uint16_t)~RTC_CRL_CNF;       /* leave config mode */
    while((hrtc.Instance->CRL & RTC_CRL_RTOFF) == 0u){
    }
}
/* USER CODE END 0 */

RTC_HandleTypeDef hrtc;

/* RTC init function */
void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef DateToUpdate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
  hrtc.Init.OutPut = RTC_OUTPUTSOURCE_ALARM;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
  /* The RTC counter is a persistent epoch (seconds since 2000-01-01). If it
     was already configured (magic in BKP_DR1), keep it as-is and return early
     so the default time/date write below does not reset the clock on boot. */
  if (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == RTC_BKP_MAGIC)
  {
    return;   /* already configured - keep the running clock as-is */
  }
  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;

  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
  DateToUpdate.Month = RTC_MONTH_JANUARY;
  DateToUpdate.Date = 0x1;
  DateToUpdate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &DateToUpdate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */
  /* First boot only (an already-configured RTC returns early above): persist
     the magic. The generated SetTime below already set the counter to 0
     (epoch 0 = 2000-01-01 00:00:00). */
  HAL_PWR_EnableBkUpAccess();
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, RTC_BKP_MAGIC);
  /* USER CODE END RTC_Init 2 */

}

void HAL_RTC_MspInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspInit 0 */

  /* USER CODE END RTC_MspInit 0 */
    HAL_PWR_EnableBkUpAccess();
    /* Enable BKP CLK enable for backup registers */
    __HAL_RCC_BKP_CLK_ENABLE();
    /* RTC clock enable */
    __HAL_RCC_RTC_ENABLE();
  /* USER CODE BEGIN RTC_MspInit 1 */

  /* USER CODE END RTC_MspInit 1 */
  }
}

void HAL_RTC_MspDeInit(RTC_HandleTypeDef* rtcHandle)
{

  if(rtcHandle->Instance==RTC)
  {
  /* USER CODE BEGIN RTC_MspDeInit 0 */

  /* USER CODE END RTC_MspDeInit 0 */
    /* Peripheral clock disable */
    __HAL_RCC_RTC_DISABLE();
  /* USER CODE BEGIN RTC_MspDeInit 1 */

  /* USER CODE END RTC_MspDeInit 1 */
  }
}

/* USER CODE BEGIN 1 */

/**
 * @brief  Read the current epoch seconds (since 2000-01-01) from the RTC.
 * @retval Epoch seconds.
 */
uint32_t rtc_get_epoch(void){
    return rtc_read_counter();
}

/**
 * @brief  Read the current time/date from the internal RTC into *time.
 * @param  time Pointer to the TimeDate_t to fill.
 * @retval 0 on success, 1 if time is NULL.
 */
uint8_t rtc_get_time(TimeDate_t *time){
    uint32_t epoch, sod;
    int32_t days, year;
    uint32_t month, day;

    if(time == NULL){
        return 1u;
    }

    epoch = rtc_read_counter();
    sod   = epoch % 86400u;
    days  = (int32_t)(epoch / 86400u) + RTC_EPOCH_1970_DAYS;
    civil_from_days(days, &year, &month, &day);

    time->hours   = (uint8_t)(sod / 3600u);
    time->minutes = (uint8_t)((sod % 3600u) / 60u);
    time->seconds = (uint8_t)(sod % 60u);
    time->day     = (uint8_t)day;
    time->month   = (uint8_t)month;
    time->year    = (uint16_t)year;

    return 0u;
}

/**
 * @brief  Write *time into the internal RTC as an epoch (seconds since
 *         2000-01-01 00:00:00).
 * @param  time Pointer to the TimeDate_t to write.
 * @retval 0 on success, 1 if time is NULL or out of range.
 */
uint8_t rtc_set_time(const TimeDate_t *time){
    int32_t days;
    uint32_t sod, epoch;

    if(time == NULL){
        return 1u;
    }
    if(time->hours > 23u || time->minutes > 59u || time->seconds > 59u){
        return 1u;
    }
    if(time->day < 1u || time->day > 31u || time->month < 1u || time->month > 12u){
        return 1u;
    }
    if(time->year < 2000u || time->year > 2099u){
        return 1u;
    }

    days = days_from_civil((int32_t)time->year, (uint32_t)time->month, (uint32_t)time->day)
           - RTC_EPOCH_1970_DAYS;
    sod   = (uint32_t)time->hours * 3600u + (uint32_t)time->minutes * 60u + (uint32_t)time->seconds;
    epoch = (uint32_t)days * 86400u + sod;

    rtc_write_counter(epoch);

    /* Mark the RTC as configured so MX_RTC_Init keeps it on later boots. */
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, RTC_BKP_MAGIC);

    return 0u;
}

/* USER CODE END 1 */

