/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "hmi.h"
#include "usart.h"
#include "display_draw.h"
#include <stdio.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/* USER CODE BEGIN Variables */

/* USER CODE END Variables */
/* Definitions for userInput */
osThreadId_t userInputHandle;
const osThreadAttr_t userInput_attributes = {
  .name = "userInput",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for displayDraw */
osThreadId_t displayDrawHandle;
const osThreadAttr_t displayDraw_attributes = {
  .name = "displayDraw",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */
/* Debug helpers: print encoder events over UART1 (115200 baud). */
static const char *enc_event_str(EncoderEvent_t e)
{
  switch (e)
  {
    case EV_ENC_ROTATED:    return "ROTATED";
    case EV_ENC_PRESS:      return "PRESS";
    case EV_ENC_LONG_PRESS: return "LONG_PRESS";
    default:                return "NONE";
  }
}

static void enc_debug_print(const EncoderEventMsg_t *msg)
{
  char buf[48];
  int n = snprintf(buf, sizeof(buf), "[ENC] %-11s detent=%d\r\n",
                   enc_event_str(msg->event), (int)msg->value);
  if (n > 0)
  {
    HAL_UART_Transmit(&huart1, (uint8_t *)buf, (uint16_t)n, 100u);
  }
}
/* USER CODE END FunctionPrototypes */

void user_input_task(void *argument);
void display_draw_task(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of userInput */
  userInputHandle = osThreadNew(user_input_task, NULL, &userInput_attributes);

  /* creation of displayDraw */
  displayDrawHandle = osThreadNew(display_draw_task, NULL, &displayDraw_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_user_input_task */
/**
  * @brief  Function implementing the userInput thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_user_input_task */
void user_input_task(void *argument)
{
  /* USER CODE BEGIN user_input_task */
  /* Prototyping feedback: a short buzzer "click" on every input, so the
   * encoder rotation/press is audible on the bench. Remove once the real
   * app consumes the events. The beep is non-blocking (PB1 is cleared on a
   * later poll once beep_until has passed). */
  uint32_t beep_until = 0u;

  /* Infinite loop */
  for(;;)
  {
    EncoderEventMsg_t ev = encoder_poll();

    /* Debug print of every encoder event over UART1. */
    if (ev.event != EV_ENC_NONE)
    {
      enc_debug_print(&ev);
    }

    /* Turn the buzzer off when the beep time has elapsed. */
    if ((int32_t)(beep_until - HAL_GetTick()) <= 0)
    {
      HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_RESET);
    }

    switch (ev.event)
    {
      case EV_ENC_ROTATED:
        /* TODO: app handling for rotation (ev.value = detents, + CW / - CCW) */
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
        beep_until = HAL_GetTick() + 30u;
        break;
      case EV_ENC_PRESS:
        /* TODO: app handling for encoder button press (ENTER) */
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
        beep_until = HAL_GetTick() + 80u; /* "confirm" beep */
        break;
      case EV_ENC_LONG_PRESS:
        /* TODO: app handling for encoder button long press */
        HAL_GPIO_WritePin(BUZZER_GPIO_Port, BUZZER_Pin, GPIO_PIN_SET);
        beep_until = HAL_GetTick() + 150u; /* longer beep */
        break;
      default:
        break;
    }

    osDelay(10); /* poll at ~100 Hz */
  }
  /* USER CODE END user_input_task */
}

/* USER CODE BEGIN Header_display_draw_task */
/**
* @brief Function implementing the displayDraw thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_display_draw_task */
void display_draw_task(void *argument)
{
  /* USER CODE BEGIN display_draw_task */
  /* Infinite loop */
  for(;;)
  {
    u8g2_ClearBuffer(&u8g2);
    draw_clock_date();
    u8g2_SendBuffer(&u8g2);
    osDelay(1000); /* update display at 1 Hz */
  }
  /* USER CODE END display_draw_task */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */

/* USER CODE END Application */

