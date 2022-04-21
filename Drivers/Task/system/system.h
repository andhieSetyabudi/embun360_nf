/*
 * system.h
 *
 *  Created on: Mar 7, 2022
 *      Author: andhie
 */
#pragma once
#ifndef TASK_SYSTEM_SYSTEM_H_
#define TASK_SYSTEM_SYSTEM_H_

#include "stdio.h"
#include "math.h"
#include "string.h"
#include "stdbool.h"
#include "buttonThread.h"

typedef enum
{
    EMPTY = 0,
    FILLING,
    FULL,
}tankStatus;

typedef enum
{
    floating_sensor = 0,
    us_sensor,
}tankMode_;

typedef enum
{
	system_STOPPED = 0,
	system_RUNNING = 1,
}systemStatus;

typedef enum {
	NORMAL_t = 0,
	LOWER_t	 = 1,
	UPPER_t  = 2,
}finStatus;


typedef struct systemFlag_t
{
	uint8_t fin1;
	uint8_t fin2;
	uint8_t tank;
	uint8_t HPS;
	uint8_t expansion1,
			expansion2;
	uint8_t systemRun;
	uint8_t systemLastRun;
	uint8_t systemError;
}systemFlag;

typedef struct systemIO_buffer_t
{
	uint8_t contactor1,
			contactor2,
			fan;
}systemIOBuffer;

typedef struct sysMem_t
{
	float minHumidity,
		  minFinTemperature,
		  lastWaterVolume,
		  totalWaterVolume;
	uint8_t tankMode;
	float waterLevelConst;
	float waterTankHeight;
	uint8_t month;
	uint8_t date;
}sysMem;

typedef struct systemVar_t
{
    float tankHeight;
    sysMem memory;
    systemFlag sysFlag;
    systemIOBuffer outputBuffer;
}systemVar;

typedef struct system_func_t{
	void (*init) ();
	void (*beeper) (uint32_t time_, uint8_t count);
	const char* (*getTankStatusStr) (uint8_t status, char* buf);
	const char* (*getTankModeStr)	(tankMode_ mode_, char* buf);
	const char* (*getStatus)		(char* buf);
	uint8_t     (*getError)		    (void);
	const char* (*getErrorCode)		(char* buf);
	void (*inputInterrupt_handler)	( uint16_t GPIO_Pin );
	void (*updateTotalVolume)		(float litre, uint32_t timeSecond);
	void (*onRun)();
	void (*resetLog)();
}system_func;


void systemInputInterrupt_handler(uint16_t GPIO_Pin);



extern systemVar sysVar;
extern system_func system_t;

extern void taskSystem(void *argument);




#endif /* TASK_SYSTEM_SYSTEM_H_ */
