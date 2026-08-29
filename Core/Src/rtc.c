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
/* Magic value marking that the RTC calendar was already configured. */
#define RTC_BKP_MAGIC 0x32F2u

static uint8_t bcd2bin(uint8_t bcd){
    return (uint8_t)(((bcd >> 4) & 0x0Fu) * 10u + (bcd & 0x0Fu));
}

static uint8_t bin2bcd(uint8_t bin){
    return (uint8_t)(((bin / 10u) << 4) | (bin % 10u));
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
  uint8_t rtc_already_set = 0u;
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
  /* The F1 RTC keeps only a seconds counter in hardware; the calendar date
     lives in RAM (hrtc.DateToUpdate). Mirror it into the backup registers so
     it survives a reset while the backup domain stays powered. */
  rtc_already_set = (HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR1) == RTC_BKP_MAGIC) ? 1u : 0u;
  if (rtc_already_set != 0u)
  {
    hrtc.DateToUpdate.Year    = (uint8_t)HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR2);
    hrtc.DateToUpdate.Month   = (uint8_t)HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR3);
    hrtc.DateToUpdate.Date    = (uint8_t)HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR4);
    hrtc.DateToUpdate.WeekDay = (uint8_t)HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR5);
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
  if (rtc_already_set == 0u)
  {
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, RTC_BKP_MAGIC);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, 0x00u);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, 0x01u);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, 0x01u);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, RTC_WEEKDAY_MONDAY);
  }
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
 * @brief  Read the current time/date from the internal RTC into *time.
 * @param  time Pointer to the TimeDate_t to fill.
 * @retval 0 on success, 1 if time is NULL or a HAL call failed.
 */
uint8_t rtc_get_time(TimeDate_t *time){
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

    if(time == NULL){
        return 1u;
    }

    if(HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK){
        return 1u;
    }
    if(HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK){
        return 1u;
    }

    time->hours   = bcd2bin(sTime.Hours);
    time->minutes = bcd2bin(sTime.Minutes);
    time->seconds = bcd2bin(sTime.Seconds);
    time->day     = bcd2bin(sDate.Date);
    time->month   = bcd2bin(sDate.Month);
    time->year    = (uint16_t)(2000u + bcd2bin(sDate.Year));

    return 0u;
}

/**
 * @brief  Write *time into the internal RTC (24-hour format) and persist the
 *         date in the backup registers so it survives a reset.
 * @param  time Pointer to the TimeDate_t to write.
 * @retval 0 on success, 1 if time is NULL, out of range, or a HAL call failed.
 */
uint8_t rtc_set_time(const TimeDate_t *time){
    RTC_TimeTypeDef sTime = {0};
    RTC_DateTypeDef sDate = {0};

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

    sTime.Hours   = bin2bcd(time->hours);
    sTime.Minutes = bin2bcd(time->minutes);
    sTime.Seconds = bin2bcd(time->seconds);
    if(HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK){
        return 1u;
    }

    /* WeekDay is recomputed by HAL_RTC_SetDate, so any value is fine here. */
    sDate.WeekDay = RTC_WEEKDAY_MONDAY;
    sDate.Month   = bin2bcd(time->month);
    sDate.Date    = bin2bcd(time->day);
    sDate.Year    = bin2bcd((uint8_t)(time->year - 2000u));
    if(HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK){
        return 1u;
    }

    /* Save the date in the backup registers (the F1 has no on-chip date
       register, so this is what keeps the calendar across a reset). */
    HAL_PWR_EnableBkUpAccess();
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR1, RTC_BKP_MAGIC);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR2, (uint32_t)(time->year - 2000u));
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR3, (uint32_t)time->month);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR4, (uint32_t)time->day);
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR5, (uint32_t)hrtc.DateToUpdate.WeekDay);

    return 0u;
}

/* USER CODE END 1 */

