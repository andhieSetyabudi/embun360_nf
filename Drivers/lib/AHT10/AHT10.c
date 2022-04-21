/*
 * AHT10.c
 *  Created on : Jun 3, 2021
 *      Author : Andhie Setyabudi
 *      e-mail : Andhie.13@gmail.com
 *      Github : andhieSetyabudi
 */


#include "AHT10.h"
#include "stdlib.h"
#include "string.h"

static void AHT10_delay(uint32_t time_)
{
	if(AHT10.halt != NULL)
		AHT10.halt(time_);
	else
		HAL_Delay(time_);
}

static uint32_t AHT10_getTick(void)
{
	if(AHT10.getTick != NULL)
		return AHT10.getTick();
	else
		return HAL_GetTick();
}

static AHT10_status AHT10_read(AHT10_var* var, uint8_t *buffer, size_t len, uint32_t timeout_)
{
    if (len > _maxBufferSize)
        return AHT10_ERROR;
//    HAL_I2C
    return HAL_I2C_Master_Receive(var->i2c, var->address, buffer, len, timeout_);
}

static AHT10_status AHT10_write(AHT10_var* var, const uint8_t *buffer, size_t len, uint32_t timeout_)
{
	 if ( len > _maxBufferSize )
	        return AHT10_ERROR;
	 return HAL_I2C_Master_Transmit(var->i2c, var->address, buffer, len, timeout_);
}

static AHT10_status AHT10_detect(AHT10_var* var)
{
	if( var->i2c == NULL )
		return AHT10_ERROR;
	uint8_t ret =  HAL_I2C_IsDeviceReady(var->i2c, var->address, 5, 100);
	return (AHT10_status)ret;
}

static AHT10_status AHT10_begin(AHT10_var* var, I2C_HandleTypeDef *i2c_, uint8_t address)
{
	var->i2c 			= i2c_;
	var->address 		= address<<1;
	var->temperature 	= 0;
	var->humidity 		= 0;
	uint8_t ret = AHT10_detect(var);
	if ( ret != AHT10_OK )
		return ret;
	uint8_t *cmd = malloc(3 * sizeof(*cmd));
	if ( cmd != NULL )
		memset(cmd,0,3);
	cmd[0] = AHTX0_CMD_SOFTRESET;
	ret = AHT10_write(var,cmd,1);
	if( ret != AHT10_OK )
		return ret;
	AHT10_delay(20);	// time for module to warming up

	cmd[0] = AHTX0_CMD_CALIBRATE;
	cmd[1] = 0x08;
	cmd[2] = 0x00;
	ret = AHT10_write(var, cmd, 3);
	if ( ret != AHT10_OK )
		return ret;
	uint32_t ts = AHT10_getTick();
	while (AHT10_getStatus(var) & AHTX0_STATUS_BUSY)
	{
		AHT10_delay(5);
		if( ts - AHT10_getTick() >= 5000 )
		{
			free(cmd);
			return AHT10_TIMEOUT;
		}
	};
	free(cmd);
	// load last status to clearing buffer
	if (!(AHT10_getStatus(var) & AHTX0_STATUS_CALIBRATED))
		return AHT10_ERROR;
	else
		return AHT10_OK;

}

static AHT10_status AHT10_getEvent(AHT10_var* var, uint32_t timeout_)
{
	uint8_t ret;
	if (var->error_counter >= AHT10_MAX_TRIAL)   // didn't get reply yet from AHT10
	{
		// need to restart module
		uint8_t address_ = var->address>>1;
		I2C_HandleTypeDef *i2c_ = var->i2c;
		ret = AHT10_begin(var, i2c_, address_);
		if ( ret != AHT10_OK )
			return ret;
		var->error_counter = 0;
	}
//	uint8_t cmd_TRIGGER[3] = {AHTX0_CMD_TRIGGER, 0x33, 0};
	// read the data and store it!
	uint8_t *cmd_ptr = malloc(3 * sizeof(*cmd_ptr));
	if (cmd_ptr != NULL)
	{
		cmd_ptr[0] = AHTX0_CMD_TRIGGER;
		cmd_ptr[1] = 0x33;
		cmd_ptr[2] = 0;
		ret = AHT10_write(var, cmd_ptr, 3);
		if (ret != AHT10_OK )
		{
			var->error_counter++;
			return ret;
		}
	}
	else
		return AHT10_ERROR;
	free(cmd_ptr);
	unsigned long t = AHT10_getTick();
	while (AHT10_getStatus(var) & AHTX0_STATUS_BUSY)
	{
		AHT10_delay(10);
		uint32_t rollOver = AHT10_getTick();
		if (rollOver < t)
			t = rollOver;
		if (AHT10_getTick() - t >= timeout_)
		{
			var->error_counter++;
			return AHT10_TIMEOUT;
		};
	}

	uint8_t *data = malloc(6 * sizeof(*data));
	if (data != NULL)
	{
		// clear the data
		memset(data,0,6);
	}

	ret = AHT10_read(var, data, 6, timeout_);
	if (ret != AHT10_OK )
	{
		var->error_counter++;
		free(data);
		return ret;
	}

	uint32_t h = data[1];
	h <<= 8;
	h |= data[2];
	h <<= 4;
	h |= data[3] >> 4;
	var->humidity = ((float)h * 100) / 0x100000;

	uint32_t tdata = data[3] & 0x0F;
	tdata <<= 8;
	tdata |= data[4];
	tdata <<= 8;
	tdata |= data[5];
	var->temperature = ((float)tdata * 200 / 0x100000) - 50;
	free(data);
	return AHT10_OK;
}

static uint8_t AHT10_getStatus(AHT10_var* var)
{
	uint8_t ret;
	if ( AHT10_read(var, &ret, 1) != AHT10_OK )
		return 0xFF;
	else
		return ret;
}


// extract Temperature value from the struct
static float AHT10_getTemperature (AHT10_var* var)
{
	return var->temperature;
}

// extract Humidity value from the struct
static float AHT10_getHumidity (AHT10_var* var)
{
	return var->humidity;
}


AHT10_funct AHT10 =
{
	.halt 			= NULL,
	.getTick		= NULL,
	.begin			= AHT10_begin,
	.detect			= AHT10_detect,
	.getEvent		= AHT10_getEvent,
	.getHumidity	= AHT10_getHumidity,
	.getTemperature	= AHT10_getTemperature,
};
