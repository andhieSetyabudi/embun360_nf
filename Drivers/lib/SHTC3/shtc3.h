/*
 * shtc3.h
 *
 *  Created on: Apr 1, 2022
 *      Author: andhie
 */

#ifndef LIB_SHTC3_SHTC3_H_
#define LIB_SHTC3_SHTC3_H_
#include "stm32f2xx.h"
#include "cmsis_os2.h"

#define SHTC3_DEFAULT_ADDR		0x70

typedef enum{
	SHTC3_OK       = 0x00, // no error
	SHTC3_ERROR      = 0x01, // no acknowledgment error
	CHECKSUM_ERROR = 0x02 // checksum mismatch error
}SHTC3_st;




typedef struct SHTC3_var_t{
	I2C_HandleTypeDef *i2c;
	uint16_t tmpRAW, humRAW;
	float temperature,
		  humidity;
	uint8_t address;
	uint8_t error_counter;
	uint16_t device_id;
}SHTC3_var;

// preparing and checking device presence status
static SHTC3_st SHTC3_begin(SHTC3_var* var, I2C_HandleTypeDef *i2c_, uint8_t address);

// detecting the device/ module with that address
static SHTC3_st SHTC3_detect(SHTC3_var* var);

// load device ID
static SHTC3_st SHTC3_checkID(SHTC3_var* var);

// taking event status or requesting the value of measurement from modules
static SHTC3_st SHTC3_getEvent(SHTC3_var* var);

// get the status report from Modules
static SHTC3_st SHTC3_getStatus(SHTC3_var* var);


// attach the delay method to the obj-struct
static void SHTC3_attachDelay();

// extract Temperature value from the struct
static float SHTC3_getTemperature (SHTC3_var* var);

// extract Humidity value from the struct
static float SHTC3_getHumidity (SHTC3_var* var);



static SHTC3_st SHTC3_StartWriteAccess(void);
static SHTC3_st SHTC3_StartReadAccess(void);
static void SHTC3_StopAccess(void);
static SHTC3_st SHTC3_Read2BytesAndCrc(uint16_t *data);
//static SHTC3_et SHTC3_WriteCommand(etCommands cmd);
static SHTC3_st SHTC3_CheckCrc(uint8_t data[], uint8_t nbrOfBytes,
                              uint8_t checksum);
static float SHTC3_CalcTemperature(uint16_t rawValue);
static float SHTC3_CalcHumidity(uint16_t rawValue);



typedef struct __attribute__ ((__packed__)){
	SHTC3_st (*begin)			(SHTC3_var* var, I2C_HandleTypeDef *i2c_, uint8_t address);
	SHTC3_st (*detect)			(SHTC3_var* var);
	SHTC3_st (*getEvent)		(SHTC3_var* var);
	float (*getTemperature)		(SHTC3_var* var);
	float (*getHumidity)		(SHTC3_var* var);
}SHTC3_funct;


extern SHTC3_funct SHTC3;

#endif /* LIB_SHTC3_SHTC3_H_ */
