/*
 * stm32_rtc.c
 *
 *  Created on: Feb 24, 2022
 *      Author: andhie
 */


#include "stm32_rtc.h"

bool rtc_init()
{
	RTC_TimeTypeDef sTime = {0};
	RTC_DateTypeDef sDate = {0};
	uint32_t rtc_BAK = HAL_RTCEx_BKUPRead(&hrtc, RTC_BKP_DR19);
	if( rtc_BAK == RTC_MAGIC_KEY )
		return true;
	if (HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
	{
	  Error_Handler();
	}
	if (HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
	{
	  Error_Handler();
	}
	HAL_PWREx_EnableBkUpReg();
	if (!(sTime.Hours == 0 && sTime.Minutes == 0 && sTime.Seconds == 0) &&
		(sTime.Hours < 24 && sTime.Minutes < 60 && sTime.Seconds < 60))
		return true;

	return false;
}

void rtc_getDateTime(RTC_DateTypeDef* date, RTC_TimeTypeDef* time) {
  for (int i = 0; i < 2 && (HAL_RTC_GetTime(&hrtc, time, RTC_FORMAT_BIN) != HAL_OK); i++) osDelay(1);
  for (int i = 0; i < 2 && (HAL_RTC_GetDate(&hrtc, date, RTC_FORMAT_BIN) != HAL_OK); i++) osDelay(1);
}

void rtc_setDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time) {
  date->WeekDay = weekDayOf(date->Year, date->Month, date->Date);
  for (int i = 0; i < 2 && (HAL_RTC_SetTime(&hrtc, time, RTC_FORMAT_BIN) != HAL_OK); i++) osDelay(1);
  for (int i = 0; i < 2 && (HAL_RTC_SetDate(&hrtc, date, RTC_FORMAT_BIN) != HAL_OK); i++) osDelay(1);
  HAL_RTCEx_BKUPWrite(&hrtc, RTC_BKP_DR19, RTC_MAGIC_KEY);
}

typedef union
{
  uint32_t dataWord;
  uint8_t dataByte[4];
}parse;

void __attribute__((optimize("O2"))) rtc_get_memory(uint32_t addr, uint8_t* data, uint8_t len)
{
	if( len > MAX_LENGTH )
		len = MAX_LENGTH;
	if ( (len+addr) > MAX_LENGTH )
		return;
	parse buffRead;
	uint16_t pos = 0;
	for( uint16_t oi = 0; oi < len; oi++)
	{
	  buffRead.dataWord = HAL_RTCEx_BKUPRead(&hrtc, (uint32_t)oi+addr);
	  data[pos] 	= buffRead.dataByte[0];
	  data[pos+1] 	= buffRead.dataByte[1];
	  data[pos+2] 	= buffRead.dataByte[2];
	  data[pos+3] 	= buffRead.dataByte[3];
	  pos+=4;
	}
}

void __attribute__((optimize("O2"))) rtc_put_memory(uint32_t addr, uint8_t* data, uint8_t len)
{
	if( len > MAX_LENGTH )
		len = MAX_LENGTH;
	if ( (len+addr) > MAX_LENGTH )
		return;
	parse buffFlush;
	uint8_t pos=0;
	for( uint16_t oi = 0; oi < len; oi++)
	{
	  buffFlush.dataByte[0] = data[pos];
	  buffFlush.dataByte[1] = data[pos+1];
	  buffFlush.dataByte[2] = data[pos+2];
	  buffFlush.dataByte[3] = data[pos+3];
	  pos+=4;
	  HAL_RTCEx_BKUPWrite(&hrtc,(uint32_t) oi+addr, buffFlush.dataWord);
	}
}
