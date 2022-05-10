/*
 * interface.h
 *
 *  Created on: Mar 7, 2022
 *      Author: andhie
 */
#pragma once
#ifndef TASK_INTERFACE_INTERFACE_H_
#define TASK_INTERFACE_INTERFACE_H_

#include "main.h"
#include "cmsis_os.h"
#include "stm32f2xx.h"

#define totalMainPage		2

enum ico_num{
	ico_warning = 0,
	ico_success = 1,
	ico_failed = 2,
};

typedef struct interfaceMain_var_t{
	uint8_t ready;
	uint8_t pageCursor;
	uint8_t onExit;
	// date-time for header
	RTC_DateTypeDef *date;
	RTC_TimeTypeDef *time;

	float *minTemperature;
	float *minHumidity;

	float *airTemp, * airRH;
	float *finTemp1, *finTemp2;
	float *waterFlow, *totalVolume;
	float *currentVolume;

	float *tankInPercent;
	float *tankUS;
	uint8_t *tankMode;
	uint8_t *tankFillingStatus;
	uint8_t *systemStatus;

	uint8_t icoN;
	uint8_t notifState;
	uint32_t notifTimeout;

	uint8_t shutingDown_state;


	void (*onShutingdown)();
	uint8_t interface_notifIco;
	uint8_t interface_onNotifState;
	uint8_t interface_notifInterrupted;
	const char *interface_notifMessage;
}mainInterface_var;


typedef struct interfaceMain_func{
	void (*init)	(void);
	uint8_t (*isReady) ();
	// linker of each variables
	void (*attachTimeDate) 		(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);
	void (*attachMeasurement) 	(float *airTemp, 	float* airRH, 		  float *finTemp1,   float *finTemp2,
			  	  	  	  	  	  float *waterFlow, float *currentVolume, float *totalVolume, float *tankPercent,
								  float *tankUS);
	void (*attachSystemFlag)	( uint8_t *tankMode, uint8_t *tankFillingStatus, uint8_t *systemStatus );

	void (*attachSystemSet)		(float *minTemperature, float *minHumidity );
	void (*showNotification)	(const char * message, uint8_t ICO, uint32_t timeout_);
}interfaceMain_f;

extern interfaceMain_f interfaceMain;
extern mainInterface_var interfaceVar;

void interface_drawHeader(void);
extern void InterfaceTask(void *argument);

#endif /* TASK_INTERFACE_INTERFACE_H_ */
