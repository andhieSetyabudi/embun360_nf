/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f2xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stm32f2xx_hal_rtc.h"

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */
void onStandBy(void);

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define Pump_Pin GPIO_PIN_5
#define Pump_GPIO_Port GPIOA
#define CT1_Pin GPIO_PIN_6
#define CT1_GPIO_Port GPIOA
#define CT2_Pin GPIO_PIN_7
#define CT2_GPIO_Port GPIOA
#define Fan2_Pin GPIO_PIN_5
#define Fan2_GPIO_Port GPIOC
#define Fan1_Pin GPIO_PIN_0
#define Fan1_GPIO_Port GPIOB
#define Fan3_Pin GPIO_PIN_1
#define Fan3_GPIO_Port GPIOB
#define btm_buzzer_Pin GPIO_PIN_12
#define btm_buzzer_GPIO_Port GPIOB
#define phase2_Pin GPIO_PIN_13
#define phase2_GPIO_Port GPIOB
#define phase2_EXTI_IRQn EXTI15_10_IRQn
#define phase1_Pin GPIO_PIN_14
#define phase1_GPIO_Port GPIOB
#define phase1_EXTI_IRQn EXTI15_10_IRQn
#define HPS_Pin GPIO_PIN_6
#define HPS_GPIO_Port GPIOC
#define HPS_EXTI_IRQn EXTI9_5_IRQn
#define Wlevel_H_Pin GPIO_PIN_7
#define Wlevel_H_GPIO_Port GPIOC
#define Wlevel_L_Pin GPIO_PIN_8
#define Wlevel_L_GPIO_Port GPIOC
#define Pressure2_Pin GPIO_PIN_9
#define Pressure2_GPIO_Port GPIOC
#define Pressure2_EXTI_IRQn EXTI9_5_IRQn
#define Pressure1_Pin GPIO_PIN_8
#define Pressure1_GPIO_Port GPIOA
#define Pressure1_EXTI_IRQn EXTI9_5_IRQn
#define US_TRIGER_Pin GPIO_PIN_9
#define US_TRIGER_GPIO_Port GPIOA
#define US_ECHO_Pin GPIO_PIN_10
#define US_ECHO_GPIO_Port GPIOA
#define flowMeter_Pin GPIO_PIN_15
#define flowMeter_GPIO_Port GPIOA
#define flowMeter_EXTI_IRQn EXTI15_10_IRQn
#define DS_Pin GPIO_PIN_10
#define DS_GPIO_Port GPIOC
#define PWR_BTN_Pin GPIO_PIN_11
#define PWR_BTN_GPIO_Port GPIOC
#define PWR_BTN_EXTI_IRQn EXTI15_10_IRQn
#define STG_BTN_Pin GPIO_PIN_12
#define STG_BTN_GPIO_Port GPIOC
#define STG_BTN_EXTI_IRQn EXTI15_10_IRQn
#define OK_BTN_Pin GPIO_PIN_2
#define OK_BTN_GPIO_Port GPIOD
#define OK_BTN_EXTI_IRQn EXTI2_IRQn
#define UP_BTN_Pin GPIO_PIN_3
#define UP_BTN_GPIO_Port GPIOB
#define UP_BTN_EXTI_IRQn EXTI3_IRQn
#define DOWN_BTN_Pin GPIO_PIN_4
#define DOWN_BTN_GPIO_Port GPIOB
#define DOWN_BTN_EXTI_IRQn EXTI4_IRQn
#define top_buzzer_Pin GPIO_PIN_5
#define top_buzzer_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

#define VERSION		"v1.1.0"
#define MODEL		"Embun-360NF"
#define COMPANY		"PT. Widya Imersif Teknologi"
#define YEAR_MADE	"@ 2022"


#define MFG_DATE	25
#define MFG_MONTH	RTC_MONTH_FEBRUARY
#define MFG_DAY		RTC_WEEKDAY_FRIDAY
#define MFG_YEAR	22 // 2022




/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
