/*
 * AHT10.h
 *  Created on : Jun 3, 2021
 *      Author : Andhie Setyabudi
 *      e-mail : Andhie.13@gmail.com
 *      Github : andhieSetyabudi
 */

#ifndef AHT10_AHT10_H_
#define AHT10_AHT10_H_

#include "main.h"

#ifdef STM32F4
	#include "stm32f4xx_hal.h"
#elif defined STM32L4
	#include "stm32l4xx_hal.h"
#elif defined STM32F2
	#include "stm32f2xx_hal.h"
#endif


#include "stdio.h"
#include "stddef.h"
#include "stdbool.h"

#define NUM_ARGS_(_1, _2, _3, _4, _5, _6, _7, _8, TOTAL, ...) TOTAL
#define NUM_ARGS(...) NUM_ARGS_(__VA_ARGS__, 6, 5, 4, 3, 2, 1, 0)
#define CONCATE_(X, Y) X##Y
#define CONCATE(MACRO, NUMBER) CONCATE_(MACRO, NUMBER)
#define VA_MACRO(MACRO, ...) CONCATE(MACRO, NUM_ARGS(__VA_ARGS__))(__VA_ARGS__)


#ifndef DEFAULT_AHT10_DELAY
	#define DEFAULT_AHT10_DELAY 500UL
#endif


#define AHTX0_I2CADDR_DEFAULT   0x38        ///< AHT default i2c address
#define AHTX0_CMD_CALIBRATE     0xE1        ///< Calibration command
#define AHTX0_CMD_TRIGGER       0xAC        ///< Trigger reading command
#define AHTX0_CMD_SOFTRESET     0xBA        ///< Soft reset command
#define AHTX0_STATUS_BUSY       0x80        ///< Status bit for busy
#define AHTX0_STATUS_CALIBRATED 0x08        ///< Status bit for calibrated

#define _maxBufferSize          32

#ifndef AHT10_MAX_TRIAL
    #define AHT10_MAX_TRIAL     10
#endif

/* Status Reply */
typedef enum AHT10_status_t
{
	AHT10_OK       = 0x00U,
	AHT10_ERROR    = 0x01U,
	AHT10_BUSY     = 0x02U,
	AHT10_TIMEOUT  = 0x03U
} AHT10_status;



typedef struct __attribute__ ((__packed__)) AHT10_var_t{
	I2C_HandleTypeDef *i2c;
	float temperature,
		  humidity;
	uint8_t address;
	uint8_t error_counter;
}AHT10_var;

// read function ; to request data from main to sensor module
static AHT10_status AHT10_read(AHT10_var* var, uint8_t *buffer, size_t len, uint32_t timeout_);
#define AHT10_read_3(s,t,a,b,c)		AHT10_read(a,b,c,DEFAULT_AHT10_DELAY)
#define AHT10_read_4(s,t,a,b,c,d)	AHT10_read(a,b,c,d)
#define AHT10_read(...)				VA_MACRO(AHT10_read_, void, void,__VA_ARGS__)


// write function ; to write command from main to sensor module
static AHT10_status AHT10_write(AHT10_var* var, const uint8_t *buffer, size_t len, uint32_t timeout_);
#define AHT10_write_3(s,t,a,b,c)	AHT10_write(a,b,c,DEFAULT_AHT10_DELAY)
#define AHT10_write_4(s,t,a,b,c,d)	AHT10_write(a,b,c,d)
#define AHT10_write(...)			VA_MACRO(AHT10_write_, void, void,__VA_ARGS__)

// detecting the device/ module with that address
static AHT10_status AHT10_detect(AHT10_var* var);

// preparing and checking device presence status
static AHT10_status AHT10_begin(AHT10_var* var, I2C_HandleTypeDef *i2c_, uint8_t address);

// taking event status or requesting the value of measurement from modules
static AHT10_status AHT10_getEvent(AHT10_var* var, uint32_t timeout_);

// get the status report from Modules
static uint8_t AHT10_getStatus(AHT10_var* var);

//
//// attach the delay method to the obj-struct
//static void AHT10_attachDelay();

// extract Temperature value from the struct
static float AHT10_getTemperature (AHT10_var* var);

// extract Humidity value from the struct
static float AHT10_getHumidity (AHT10_var* var);

typedef struct __attribute__ ((__packed__)){
	void (*halt)				(uint32_t time_halt);
	uint32_t (*getTick)			(void);

	AHT10_status (*begin)		(AHT10_var* var, I2C_HandleTypeDef *i2c_, uint8_t address);
	AHT10_status (*detect)		(AHT10_var* var);
	AHT10_status (*getEvent)	(AHT10_var* var, uint32_t timeout_);
	float (*getTemperature)		(AHT10_var* var);
	float (*getHumidity)		(AHT10_var* var);
}AHT10_funct;


extern AHT10_funct AHT10;
#endif /* AHT10_AHT10_H_ */
