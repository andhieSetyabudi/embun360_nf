/*
 * sensor.h
 *
 *  Created on: Mar 7, 2022
 *      Author: andhie
 */
#pragma once
#ifndef TASK_SENSOR_SENSOR_H_
#define TASK_SENSOR_SENSOR_H_

#include "stm32f2xx.h"
#include "cmsis_os.h"
#include "stdbool.h"
#include "stdio.h"
#include "math.h"
#include "AHT10.h"
#include "main.h"
#include "shtc3.h"
#include "ds18b20.h"

#ifndef AIR_RH_ADDR
	#define AIR_RH_ADDR 0x38
#endif

extern I2C_HandleTypeDef hi2c2;
#ifndef I2C_CH
	#define I2C_CH hi2c2
#endif

typedef struct __attribute__ ((__packed__)) sensor_ctrl_t{
	ds18b20_t finTempSensor;
	uint8_t finTempAddr[2][8];
#if RH_SEN_TYPE	== SHTC3_SENSOR
	SHTC3_var airTempHumSensor;
#else
	AHT10_var airTempHumSensor;
#endif

}sensor_ctrl;

typedef struct __attribute__ ((__packed__)) sensor_var_t{

	// sensor error
	bool airSensorFlag,
		 finSensorFlag;
	uint8_t finTmpErrCount;
	float fin_temperature[2];
	float air_temperature,
		  air_rH;

	// power sensor
	uint32_t phaseCounter[2];
	float phaseFrequency[2];
	bool PG_Flag;		// power good

	// waterflow sensor
	uint32_t waterFlowCounter;
	float waterFlowFrequency;
	float waterFlow_inLPM;
	float waterFlow_const;

	// tank sensor based on distance
	float tankLevel_inPercent;
	float tankHeight_const;
	float waterLevelFrom_cap;
}sensor_var;

typedef struct __attribute__ ((__packed__)) sensor_func_t{
	void (*sensorStart)(void);
	void (*sensorPause)(void);
	void (*sensorResume)(void);
	bool (*getAirSensorFlag)		(void);
	bool (*getFinSensorFlag)		(void);
	float (*getFinTemperature) 		(uint8_t ch);
	float (*getAirTemperature)		(void);
	float (*getAirRH) 				(void);
	float (*getPhaseFrequency) 		(uint8_t ch);
	bool (*getPGStatus) 			(void);
	float (*getWaterFlow_inHz)		(void);
	float (*getWaterFlow_inLPM)		(void);
	float (*getWaterFlow_const)		(void);
	void (*setWaterFlow_const)		(float c);
	float (*getWaterLevel_inPercent) (void);
	float (*getTankHeight_const)	(void);
	void (*setTankHeight_const) 	(float c);
	float (*getTankSensor)			(void);
}sensor_f;

extern sensor_f sensor;
extern sensor_var sensorVar;
// external function, will be updated by interrupt NVIC

	// phase detector ( AC line )
	void sensor_phaseCountingUp(uint8_t ch);
	void sensor_phaseUpdateFrequency(uint8_t ch);

	// water flow sensor
	void sensor_waterFlowCountingUp(void);
	void sensor_waterFlowUpdateFrequency(void);

// sensor task -> FREETOS task for sensor
extern void SensorTask(void *argument);

#endif /* TASK_SENSOR_SENSOR_H_ */
