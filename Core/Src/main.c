/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "string.h"

#include "splashScreen.h"
#include "menuDisplay.h"
#include "stm32_rtc.h"
#include "MCP23017_LCD12864.h"
#include "BSP.h"

// task scheduler
#include "sensor.h"
#include "buttonThread.h"
#include "interface.h"
#include "system.h"
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
I2C_HandleTypeDef hi2c1;
I2C_HandleTypeDef hi2c2;

IWDG_HandleTypeDef hiwdg;

RTC_HandleTypeDef hrtc;

TIM_HandleTypeDef htim14;

/* Definitions for bspTask */
osThreadId_t bspTaskHandle;
const osThreadAttr_t bspTask_attributes = {
  .name = "bspTask",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityBelowNormal,
};
/* Definitions for button_task */
osThreadId_t button_taskHandle;
const osThreadAttr_t button_task_attributes = {
  .name = "button_task",
  .stack_size = 2048 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for taskSensor */
osThreadId_t taskSensorHandle;
const osThreadAttr_t taskSensor_attributes = {
  .name = "taskSensor",
  .stack_size = 1024 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* Definitions for taskInterface */
osThreadId_t taskInterfaceHandle;
const osThreadAttr_t taskInterface_attributes = {
  .name = "taskInterface",
  .stack_size = 4096 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for systemTask */
osThreadId_t systemTaskHandle;
const osThreadAttr_t systemTask_attributes = {
  .name = "systemTask",
  .stack_size = 512 * 4,
  .priority = (osPriority_t) osPriorityLow,
};
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_RTC_Init(void);
static void MX_I2C1_Init(void);
static void MX_I2C2_Init(void);
static void MX_TIM14_Init(void);
static void MX_IWDG_Init(void);
void StartDefaultTask(void *argument);
extern void taskButton(void *argument);
extern void SensorTask(void *argument);
extern void InterfaceTask(void *argument);
extern void taskSystem(void *argument);

/* USER CODE BEGIN PFP */
uint8_t resetCounter = 0; // for lcd reset

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	if(GPIO_Pin == GPIO_PIN_12) // INT Source is pin A9
	{
//		clicked_count++; // Toggle LED
		sensor_phaseCountingUp(0);
	};
	if( GPIO_Pin == phase1_Pin )
		sensor_phaseCountingUp(0);
	if( GPIO_Pin == phase2_Pin )
		sensor_phaseCountingUp(1);
	if( GPIO_Pin == flowMeter_Pin )
		sensor_waterFlowCountingUp();

	// for button interrupt
	switch(GPIO_Pin)
	{
		case PWR_BTN_Pin :
			btnTask.setPressed(BTN_PWR);
			break;
		case STG_BTN_Pin :
			btnTask.setPressed(BTN_SETTING);
			break;
		case OK_BTN_Pin :
			btnTask.setPressed(BTN_OK);
			break;
		case UP_BTN_Pin :
			btnTask.setPressed(BTN_UP);
			break;
		case DOWN_BTN_Pin :
			btnTask.setPressed(BTN_DOWN);
			break;
	}
	system_t.inputInterrupt_handler(GPIO_Pin);
}


void onStandBy(void)
{
	HAL_IWDG_Refresh(&hiwdg);
	char bText[10];
	uint8_t btState=0;
	PORT_IO tmpBtn = {
			.port = PWR_BTN_GPIO_Port,
			.pin = PWR_BTN_Pin,
	};
	button_var tmpBTN;
	button.create(&tmpBTN, tmpBtn, 0, 350UL);
	button.setHoldTime(&tmpBTN, 1500UL);
	lcd.init(&hi2c1, 0x20);
	lcd.clear(1,0);
	lcd.clearBuffer();
	splashScreen.init(splashScreenICO, splashWidth, splashHeight, Font5x7);
	splashScreen.display (CENTER_, "Stand-by", VERSION, 500);
	btState= button.check(&tmpBTN);
	uint32_t count = 0;
	uint8_t flip_ = 1;
	uint32_t timeUp = 0;
	uint8_t blinked=0;
	while( 1 )
	{
		if( HAL_GetTick() - timeUp >= 750 )
		{
			rtc_getDateTime(&date_, &time_);
			blinked =~ blinked;
			timeUp = HAL_GetTick();
		}
//		lcd.init(&hi2c1, 0x20);
		sprintf(bText, "%02d%c%02d%c%02d",   time_.Hours, blinked!=0?':':' ',time_.Minutes, blinked!=0?':':' ', time_.Seconds);
		if( flip_ )
			splashScreen.display(CENTER_, "Stand-by", bText, 75);
		else
			splashScreen.display(CENTER_, " ", bText, 75);
		btState= button.check(&tmpBTN);
		if( btState == HELD_btn )
			break;
		count++;
		if( count % 2 == 0 )
		{
			count=0;
			flip_ = flip_ > 0 ? 0 : 1;
		}
		HAL_IWDG_Refresh(&hiwdg);
	};
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_RTC_Init();
  MX_I2C1_Init();
  MX_I2C2_Init();
  MX_TIM14_Init();
  MX_IWDG_Init();
  /* USER CODE BEGIN 2 */
  system_t.init();
  onStandBy();
//  rtc_getDateTime(&date_, &time_);
//  initEEPROM();

  /* USER CODE END 2 */

  /* Init scheduler */
  osKernelInitialize();

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
  /* creation of bspTask */
  bspTaskHandle = osThreadNew(StartDefaultTask, NULL, &bspTask_attributes);

  /* creation of button_task */
  button_taskHandle = osThreadNew(taskButton, NULL, &button_task_attributes);

  /* creation of taskSensor */
  taskSensorHandle = osThreadNew(SensorTask, NULL, &taskSensor_attributes);

  /* creation of taskInterface */
  taskInterfaceHandle = osThreadNew(InterfaceTask, NULL, &taskInterface_attributes);

  /* creation of systemTask */
  systemTaskHandle = osThreadNew(taskSystem, NULL, &systemTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 13;
  RCC_OscInitStruct.PLL.PLLN = 195;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_RTC;
  PeriphClkInitStruct.RTCClockSelection = RCC_RTCCLKSOURCE_LSI;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief I2C2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C2_Init(void)
{

  /* USER CODE BEGIN I2C2_Init 0 */

  /* USER CODE END I2C2_Init 0 */

  /* USER CODE BEGIN I2C2_Init 1 */

  /* USER CODE END I2C2_Init 1 */
  hi2c2.Instance = I2C2;
  hi2c2.Init.ClockSpeed = 100000;
  hi2c2.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c2.Init.OwnAddress1 = 0;
  hi2c2.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c2.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c2.Init.OwnAddress2 = 0;
  hi2c2.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c2.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C2_Init 2 */

  /* USER CODE END I2C2_Init 2 */

}

/**
  * @brief IWDG Initialization Function
  * @param None
  * @retval None
  */
static void MX_IWDG_Init(void)
{

  /* USER CODE BEGIN IWDG_Init 0 */

  /* USER CODE END IWDG_Init 0 */

  /* USER CODE BEGIN IWDG_Init 1 */

  /* USER CODE END IWDG_Init 1 */
  hiwdg.Instance = IWDG;
  hiwdg.Init.Prescaler = IWDG_PRESCALER_64;
  hiwdg.Init.Reload = 4095;
  if (HAL_IWDG_Init(&hiwdg) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN IWDG_Init 2 */

  /* USER CODE END IWDG_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */
  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */
  	if( rtc_init() == true )
  		return;
  	else
  	{
  		sTime.Hours = 0x0;
		sTime.Minutes = 0x0;
		sTime.Seconds = 0x0;
		sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
		sTime.StoreOperation = RTC_STOREOPERATION_RESET;

		sDate.WeekDay = MFG_DAY;
		sDate.Month	  = MFG_MONTH;
		sDate.Date 	  = MFG_DATE;
		sDate.Year 	  = MFG_YEAR;

		rtc_setDateTime(&sDate, &sTime);
  	}
  	return;

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 0x0;
  sTime.Minutes = 0x0;
  sTime.Seconds = 0x0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_MONDAY;
  sDate.Month = RTC_MONTH_JANUARY;
  sDate.Date = 0x1;
  sDate.Year = 0x0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BCD) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

}

/**
  * @brief TIM14 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM14_Init(void)
{

  /* USER CODE BEGIN TIM14_Init 0 */

  /* USER CODE END TIM14_Init 0 */

  /* USER CODE BEGIN TIM14_Init 1 */

  /* USER CODE END TIM14_Init 1 */
  htim14.Instance = TIM14;
  htim14.Init.Prescaler = 49999;
  htim14.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim14.Init.Period = 1000;
  htim14.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim14.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
  if (HAL_TIM_Base_Init(&htim14) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM14_Init 2 */
  HAL_TIM_Base_Start_IT(&htim14);
  /* USER CODE END TIM14_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, Pump_Pin|CT1_Pin|CT2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_4|Fan2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, Fan1_Pin|Fan3_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, btm_buzzer_Pin|top_buzzer_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(US_TRIGER_GPIO_Port, US_TRIGER_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : Pump_Pin CT1_Pin CT2_Pin */
  GPIO_InitStruct.Pin = Pump_Pin|CT1_Pin|CT2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PC4 Fan2_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_4|Fan2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : Fan1_Pin Fan3_Pin */
  GPIO_InitStruct.Pin = Fan1_Pin|Fan3_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : btm_buzzer_Pin top_buzzer_Pin */
  GPIO_InitStruct.Pin = btm_buzzer_Pin|top_buzzer_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : phase2_Pin phase1_Pin UP_BTN_Pin DOWN_BTN_Pin */
  GPIO_InitStruct.Pin = phase2_Pin|phase1_Pin|UP_BTN_Pin|DOWN_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : HPS_Pin Pressure2_Pin */
  GPIO_InitStruct.Pin = HPS_Pin|Pressure2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : Wlevel_H_Pin Wlevel_L_Pin DS_Pin */
  GPIO_InitStruct.Pin = Wlevel_H_Pin|Wlevel_L_Pin|DS_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : Pressure1_Pin */
  GPIO_InitStruct.Pin = Pressure1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(Pressure1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : US_TRIGER_Pin */
  GPIO_InitStruct.Pin = US_TRIGER_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_MEDIUM;
  HAL_GPIO_Init(US_TRIGER_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : US_ECHO_Pin */
  GPIO_InitStruct.Pin = US_ECHO_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(US_ECHO_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : flowMeter_Pin */
  GPIO_InitStruct.Pin = flowMeter_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(flowMeter_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PWR_BTN_Pin STG_BTN_Pin */
  GPIO_InitStruct.Pin = PWR_BTN_Pin|STG_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pin : OK_BTN_Pin */
  GPIO_InitStruct.Pin = OK_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(OK_BTN_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI2_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI2_IRQn);

  HAL_NVIC_SetPriority(EXTI3_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI3_IRQn);

  HAL_NVIC_SetPriority(EXTI4_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI4_IRQn);

  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 5, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the bspTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
  /* USER CODE BEGIN 5 */
  /* Infinite loop */
//	 initMenu();

//	 float totalVolume = 100;
//	 float tankLevelPercent = 0;
//	 uint8_t tankMode_ = 0;
//	 uint8_t systemStatus_ = 0;
//	 float minTemperature = 25;
//	 float minHumidity = 30;

	   	interfaceMain.attachTimeDate(&date_, &time_);
	 	interfaceMain.attachMeasurement(&sensorVar.air_temperature, &sensorVar.air_rH,
	 								    &sensorVar.fin_temperature[0], &sensorVar.fin_temperature[1],
	 								    &sensorVar.waterFlow_inLPM, &sysVar.memory.totalWaterVolume ,&sysVar.memory.lastWaterVolume, &sensorVar.tankLevel_inPercent,
										&sensorVar.waterLevelFrom_cap);
	 	interfaceMain.attachSystemFlag	(&sysVar.memory.tankMode, &sysVar.sysFlag.tank, &sysVar.sysFlag.systemRun );
	 	interfaceMain.attachSystemSet(&sysVar.memory.minFinTemperature, &sysVar.memory.minHumidity);
	 HAL_IWDG_Refresh(&hiwdg);
	 interfaceMain.init();
	 sensor.sensorStart();
	 while(interfaceMain.isReady() == 0 ) {osDelay(50);};
	 btnTask.begin();

	 uint8_t co = 0;
	 bool flipFlop = false;
  for(;;)
  {
//	  osDelay(500);
//		  HAL_GPIO_WritePin(GPIO_A, GPIO_PIN_5, GPIO_PIN_SET);
//	osDelay(500);
//		  HAL_GPIO_WritePin(GPIO_A, GPIO_PIN_5, GPIO_PIN_RESET);

	  co++;
	  if( co% 10 == 0 )
	  {
		  HAL_IWDG_Refresh(&hiwdg);
		  flipFlop = !flipFlop;
		  co = 1;
	  }

	  HAL_IWDG_Refresh(&hiwdg);
//	  tankLevelPercent+=0.1;
//	  totalVolume+=0.5;
	  rtc_getDateTime(&date_, &time_);
	  osDelay(500);
  }
  /* USER CODE END 5 */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM1 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */
	if( htim->Instance==TIM14)
	{
		sensor_phaseUpdateFrequency(0);
		sensor_phaseUpdateFrequency(1);
		sensor_waterFlowUpdateFrequency();
		resetCounter++;
		if( resetCounter >= 10 )
		{

//			lcd.init(&hi2c1, 0x20);
//			interfaceMain.showNotification("POWER SOURCE\n\n is Failed !\n\nPlease Check it ! ", ico_failed, 3000);
			resetCounter = 0;
		}
//		HAL_IWDG_Refresh(&hiwdg);
	}
  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM1) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */
  /* USER CODE END Callback 1 */
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
	  system_t.beeper(10000, 1);
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

