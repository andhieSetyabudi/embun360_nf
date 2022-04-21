/*
 * shtc3.c
 *
 *  Created on: Apr 1, 2022
 *      Author: andhie
 */

#include "shtc3.h"
#include "stdio.h"
#include "stm32f2xx_hal_i2c.h"
//#include "delayDWT.h"

/* defining register */
#define SHTC3_READ_ID             0xEFC8 // command: read ID register
#define SHTC3_SOFT_RESET          0x805D // soft reset
#define SHTC3_SLEEP               0xB098 // sleep
#define SHTC3_WAKEUP              0x3517 // wakeup
#define SHTC3_MEAS_T_RH_POLLING   0x7866 // meas. read T first, clock stretching disabled
#define SHTC3_MEAS_T_RH_CLOCKSTR  0x7CA2 // meas. read T first, clock stretching enabled
#define SHTC3_MEAS_RH_T_POLLING   0x58E0 // meas. read RH first, clock stretching disabled
#define SHTC3_MEAS_RH_T_CLOCKSTR  0x5C24 // meas. read RH first, clock stretching enabled

#define SHTC3_maxBufferSize          32



//------------------------------------------------------------------------------
static SHTC3_st SHTC3_CheckCrc(uint8_t data[], uint8_t nbrOfBytes,
                              uint8_t checksum){
  uint8_t bit;        // bit mask
  uint8_t crc = 0xFF; // calculated checksum
  uint8_t byteCtr;    // byte counter

  // calculates 8-Bit checksum with given polynomial
  for(byteCtr = 0; byteCtr < nbrOfBytes; byteCtr++) {
    crc ^= (data[byteCtr]);
    for(bit = 8; bit > 0; --bit) {
      if(crc & 0x80) {
        crc = (crc << 1) ^ 0x131 ;
      } else {
        crc = (crc << 1);
      }
    }
  }

  // verify checksum
  if(crc != checksum) {
    return CHECKSUM_ERROR;
  } else {
    return SHTC3_OK;
  }
}

//------------------------------------------------------------------------------
static float SHTC3_CalcTemperature(uint16_t rawValue){
  // calculate temperature [°C]
  // T = -45 + 175 * rawValue / 2^16
  return 175 * (float)rawValue / 65536.0f - 45.0f;
}

//------------------------------------------------------------------------------
static float SHTC3_CalcHumidity(uint16_t rawValue){
  // calculate relative humidity [%RH]
  // RH = rawValue / 2^16 * 100
  return 100 * (float)rawValue / 65536.0f;
}



static SHTC3_st SHTC3_sendCMD(SHTC3_var* var, uint16_t cmd, uint32_t timeout_)
{
	uint8_t buff[2];
	buff[0] = cmd >> 8;
	buff[1] = cmd & 0xff;
	 return HAL_I2C_Master_Transmit(var->i2c, var->address, buff, 2, timeout_);
}

static SHTC3_st SHTC3_read(SHTC3_var* var, uint8_t *buffer, size_t len, uint32_t timeout_)
{
    if (len > SHTC3_maxBufferSize)
        return SHTC3_ERROR;
    return HAL_I2C_Master_Receive(var->i2c, var->address, buffer, len, timeout_);
}

static SHTC3_st SHTC3_write(SHTC3_var* var, const uint8_t *buffer, size_t len, uint32_t timeout_)
{
	 if ( len > SHTC3_maxBufferSize )
	        return SHTC3_ERROR;
	 return HAL_I2C_Master_Transmit(var->i2c, var->address, buffer, len, timeout_);
}

static SHTC3_st SHTC3_detect(SHTC3_var* var)
{
	if( var->i2c == NULL )
		return SHTC3_ERROR;
	uint8_t ret =  HAL_I2C_IsDeviceReady(var->i2c, var->address, 5, 100);
	return (SHTC3_st)ret;
}

static SHTC3_st SHTC3_wakeup(SHTC3_var* var)
{
	if( var->i2c == NULL )
		return SHTC3_ERROR;
	uint8_t ret = SHTC3_sendCMD(var, SHTC3_WAKEUP, 100 );
	return (SHTC3_st)ret;
}

static SHTC3_st SHTC3_sleep(SHTC3_var* var)
{
	if( var->i2c == NULL )
		return SHTC3_ERROR;
	uint16_t data_reg = SHTC3_SLEEP;
	uint8_t ret = SHTC3_sendCMD(var, SHTC3_SLEEP, 100 );
	return (SHTC3_st)ret;
}

static SHTC3_st SHTC3_begin(SHTC3_var* var, I2C_HandleTypeDef *i2c_, uint8_t address)
{
	var->i2c 			= i2c_;
	var->address 		= address<<1;
	var->tmpRAW 		= 0;
	var->humRAW 		= 0;
	uint8_t ret = SHTC3_detect(var);
	if ( ret != SHTC3_OK )
		return ret;
//	if( SHTC3_checkID(var)!= SHTC3_OK )
//		return SHTC3_ERROR;
	else
		return SHTC3_OK;
}

static SHTC3_st SHTC3_checkID(SHTC3_var* var)
{
	uint8_t ret = SHTC3_detect(var);
	if ( ret != SHTC3_OK )
		return ret;
	if( SHTC3_wakeup(var)!= SHTC3_OK )
		return SHTC3_ERROR;
	if( SHTC3_sendCMD(var,SHTC3_READ_ID,100) != SHTC3_OK )
		return SHTC3_ERROR;
	uint8_t buf[3];
	if( SHTC3_read(var, buf, 3, 100) != SHTC3_OK )
		return SHTC3_ERROR;
	if( SHTC3_sleep(var) != SHTC3_OK )
		return SHTC3_ERROR;
	if( SHTC3_CheckCrc(buf,2,buf[3])  != SHTC3_OK )
		return SHTC3_ERROR;
	else
		var->device_id = (((uint16_t)buf[0] << 8) | ((uint16_t)buf[1]));
	return SHTC3_OK;
}

static SHTC3_st SHTC3_getEvent(SHTC3_var* var)
{
	uint8_t ret = SHTC3_detect(var);
	if ( ret != SHTC3_OK )
		return ret;
	if( SHTC3_wakeup(var)!= SHTC3_OK )
		return SHTC3_ERROR;
	if( SHTC3_sendCMD(var,0x5C24,100) != SHTC3_OK)
		return SHTC3_ERROR;
	uint8_t pData[6];
	if( SHTC3_read(var, pData, 6, 100) != SHTC3_OK )
		return SHTC3_ERROR;
	// Update values
		var->humRAW = ((uint16_t)pData[0] << 8) | (uint16_t)pData[1] ;
		var->tmpRAW = ((uint16_t)pData[3] << 8) | (uint16_t)pData[4] ;
	if( SHTC3_sleep(var) != SHTC3_OK )
		return SHTC3_ERROR;
	return SHTC3_OK;
	/*
	if( SHTC3_CheckCrc(pData,2,pData[3])  != SHTC3_OK )
		return SHTC3_ERROR;
	USBSerial.print("CRC1 : PASS \r\n");
	if( SHTC3_CheckCrc(pData+3,2,pData[5])  != SHTC3_OK )
			return SHTC3_ERROR;
	USBSerial.print("CRC2 : PASS \r\n");
	return SHTC3_OK;
	*/
}

// extract Temperature value from the struct
static float SHTC3_getTemperature (SHTC3_var* var)
{
	return SHTC3_CalcTemperature(var->tmpRAW);
}

// extract Humidity value from the struct
static float SHTC3_getHumidity (SHTC3_var* var)
{
	return SHTC3_CalcHumidity(var->humRAW);
}


SHTC3_funct SHTC3 =
{
	.begin			= SHTC3_begin,
	.detect			= SHTC3_detect,
	.getEvent		= SHTC3_getEvent,
	.getHumidity	= SHTC3_getHumidity,
	.getTemperature	= SHTC3_getTemperature,
};
