/*
 * ds18b20.h
 *
 *  Created on: Mar 1, 2022
 *      Author: andhie
 */

#ifndef LIB_DS18B20_DS18B20_H_
#define LIB_DS18B20_DS18B20_H_

#include "myOneWire.h"
#include "inttypes.h"
#include "string.h"


#define SEARCH_ROM 0xF0
#define READ_ROM 0x33
#define MATCH_ROM 0x55
#define SKIP_ROM 0xCC
#define ALARM_SEARCH 0xEC
#define CONVERT_T 0x44
#define WRITE_SCRATCHPAD 0x4E
#define READ_SCRATCHPAD 0xBE
#define COPY_SCRATCHPAD 0x48
#define RECALL 0xB8
#define READ_POWER_SUPPLY 0xB4
#define MODEL_DS1820 0x10
#define MODEL_DS18S20 0x10
#define MODEL_DS1822 0x22
#define MODEL_DS18B20 0x28
#define SIZE_SCRATCHPAD 9
#define TEMP_LSB 0
#define TEMP_MSB 1
#define ALARM_HIGH 2
#define ALARM_LOW 3
#define CONFIGURATION 4
#define CRC8 8
#define RES_9_BIT 0x1F
#define RES_10_BIT 0x3F
#define RES_11_BIT 0x5F
#define RES_12_BIT 0x7F
#define CONV_TIME_9_BIT 94
#define CONV_TIME_10_BIT 188
#define CONV_TIME_11_BIT 375
#define CONV_TIME_12_BIT 750


// Error Codes
#define DEVICE_DISCONNECTED_C -127
#define DEVICE_DISCONNECTED_F -196.6
#define DEVICE_DISCONNECTED_RAW -7040

typedef struct __attribute__ ((__packed__)) ds18b20_tvar{
	uint8_t globalResolution;
	uint8_t globalPowerMode;
	uint8_t numberOfDevices;
	uint8_t selectedAddress[8];
	uint8_t selectedScratchpad[SIZE_SCRATCHPAD];
	uint8_t selectedResolution;
	uint8_t selectedPowerMode;
	uint8_t searchAddress[8];
	uint8_t lastDiscrepancy;
	uint8_t lastDevice;
	OneWire_t wire_t;
	uint8_t ds18Count;	// additional
	bool parasite;		// additional
}ds18b20_t;




void ds18b20_init(ds18b20_t* var, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);

bool ds18b20_select(ds18b20_t* var, uint8_t address[]);
bool ds18b20_selectNext(ds18b20_t* var);

uint8_t ds18b20_selectNextAlarm(ds18b20_t* var);

void ds18b20_resetSearch(ds18b20_t* var);

void ds18b20_requestTemperatures(ds18b20_t* var);
float ds18b20_getTempC(ds18b20_t* var);
float ds18b20_getTempCByAddress(ds18b20_t* var, uint8_t address[], bool newRequest);

float ds18b20_getTempF(ds18b20_t* var);

uint8_t ds18b20_getResolution(ds18b20_t* var);

void ds18b20_setResolution(ds18b20_t* var, uint8_t resolution);

uint8_t ds18b20_getPowerMode(ds18b20_t* var);

uint8_t ds18b20_getFamilyCode(ds18b20_t* var);

void ds18b20_getAddress(ds18b20_t* var, uint8_t address[]);

void ds18b20_doConversion(ds18b20_t* var);
uint8_t ds18b20_getNumberOfDevices(ds18b20_t* var);

uint8_t ds18b20_hasAlarm(ds18b20_t* var);
void ds18b20_setAlarms(ds18b20_t* var, int8_t alarmLow, int8_t alarmHigh);

int8_t ds18b20_getAlarmLow(ds18b20_t* var);

void ds18b20_setAlarmLow(ds18b20_t* var, int8_t alarmLow);

int8_t ds18b20_getAlarmHigh(ds18b20_t* var);
void ds18b20_setAlarmHigh(ds18b20_t* var, int8_t alarmHigh);

void ds18b20_setRegisters(ds18b20_t* var, int8_t lowRegister, int8_t highRegister);

int8_t ds18b20_getLowRegister(ds18b20_t* var);

void ds18b20_setLowRegister(ds18b20_t* var, int8_t lowRegister);

int8_t ds18b20_getHighRegister(ds18b20_t* var);

void ds18b20_setHighRegister(ds18b20_t* var, int8_t highRegister);

uint8_t ds18b20_readScratchpad_(ds18b20_t* var, const uint8_t* deviceAddress,uint8_t* scratchPad);
uint8_t ds18b20_readScratchpad(ds18b20_t* var);

void ds18b20_writeScratchpad(ds18b20_t* var);
uint8_t ds18b20_sendCommand_(ds18b20_t* var, uint8_t romCommand);
uint8_t ds18b20_sendCommand(ds18b20_t* var, uint8_t romCommand, uint8_t functionCommand, uint8_t power);
bool ds18b20_oneWireSearch(ds18b20_t* var, uint8_t romCommand);
bool ds18b20_searchAddress(ds18b20_t* var, uint8_t* deviceAddress, uint8_t index_);
bool ds18b20_isConnected_(ds18b20_t* var, const uint8_t* deviceAddress, uint8_t* scratchPad);
uint8_t ds18b20_isConnected(ds18b20_t* var, uint8_t address[]);
void ds18b20_delayForConversion(ds18b20_t* var, uint8_t resolution, uint8_t powerMode);


float myDS_getTempC(ds18b20_t* var);


#endif /* LIB_DS18B20_DS18B20_H_ */
