/*
 * BSP.c
 *
 *  Created on: Feb 24, 2022
 *      Author: andhie
 */


#include "BSP.h"
#include "stm32f2xx_hal_flash.h"

char * getUUID(char * buff)
{
	union {
	  uint32_t w[3];
	  uint8_t b[12];
	} uid;
	uid.w[0] = HAL_GetUIDw0();
	uid.w[1] = HAL_GetUIDw1();
	uid.w[2] = HAL_GetUIDw2();
	sprintf(buff, "%02X%02X%02X%02X%02X%02X", uid.b[11], uid.b[10] + uid.b[2], uid.b[9], uid.b[8] + uid.b[0], uid.b[7], uid.b[6]);
	return buff;
}

char* ftoa(float f, char * buf, int precision)
{
	char * ptr = buf;
	char * p = ptr;
	char * p1;
	char c;
	long intPart;
	if (precision > MAX_PRECISION)		// check precision bounds
		precision = MAX_PRECISION;
	if (f < 0) {		// sign stuff
		f = -f;
		*ptr++ = '-';
	}
	if (precision < 0) {  // negative precision == automatic precision guess
		if (f < 1.0) precision = 4;
		else if (f < 10.0) precision = 3;
		else if (f < 100.0) precision = 2;
		else if (f < 1000.0) precision = 1;
		else precision = 0;
	}
	if (precision)  	// round value according the precision
		f += rounders[precision];
	// integer part...
	intPart = f;
	f -= intPart;
	if (!intPart)
		*ptr++ = '0';
	else
	{
		p = ptr;	// save start pointer
		while (intPart) { // convert (reverse order)
			*p++ = '0' + intPart % 10;
			intPart /= 10;
		}
		p1 = p;  // save end pos
		while (p > ptr)	{ // reverse result
			c = *--p;
			*p = *ptr;
			*ptr++ = c;
		}
		ptr = p1;	// restore end pos
	}
	if (precision) {	// decimal part
		*ptr++ = '.';	// place decimal point
		while (precision--)	 { // convert
			f *= 10.0;
			c = f;
			*ptr++ = '0' + c;
			f -= c;
		}
	}
	*ptr = 0;	// terminating zero
	return buf;
}


uint16_t twoByteTo16bit(uint8_t *tmp)
{
	uint16_t i;
	i = tmp[0];
	i = ((i << 8) & 0xff00) | tmp[1];
	return i;
}

void uint16toTwoByte(uint8_t *tmp, uint16_t val)
{
	tmp[0] = (val >> 8) & 0xff;
	tmp[1] = val & 0xff;
}

uint32_t fourByteToUint32t(uint8_t *tmp)
{
	uint32_t i;
	i = tmp[0];
	i = (i << 8) | tmp[1];
	i = (i << 8) | tmp[2];
	i = (i << 8) | tmp[3];
	return(i);
}

float fourByteToFloat(uint8_t *tmp)
{
	float f;
	uint32_t i;
	i = tmp[0];
	i = (i << 8) | tmp[1];
	i = (i << 8) | tmp[2];
	i = (i << 8) | tmp[3];
	f = *(float *)&i;
	return(f);
}

void floatToFourByte(uint8_t *tmp,float val)
{
	uint32_t i;
	i = *((uint32_t *)&val);
	tmp[0] = (i >> 24) & 0xff;
	tmp[1] = (i >> 16) & 0xff;
	tmp[2] = (i >> 8) & 0xff;
	tmp[3] = i & 0xff;
}

void initEEPROM()
{
	EE_Init();
//	FLASH_
}



#define VIRT_ADDR			0x08080000

#define HW_BANK				0x01

uint8_t __attribute__ ((section(".ram"))) bufferEEPROM[EEPROM_BUFFER_SIZE];

//typedef union
//{
//  uint32_t dataWord;
//  uint8_t dataByte[4];
//}parsing;

bool eeprom_buffer_fill(void)
{
  uint16_t pos = 0;
  for( uint16_t oi = 0; oi < EEPROM_BUFFER_SECTOR_COUNT; oi++)
  {
      parsing buffRead;
      EE_ReadVariable(oi+VIRT_ADDR, &buffRead.dataWord);
      bufferEEPROM[pos] 	= buffRead.dataByte[0];
      bufferEEPROM[pos+1] 	= buffRead.dataByte[1];
      bufferEEPROM[pos+2] 	= buffRead.dataByte[2];
      bufferEEPROM[pos+3] 	= buffRead.dataByte[3];
      pos+=4;
  }
  return true;
}

bool eeprom_buffer_flush(void)
{
  parsing buffFlush;
  uint16_t pos = 0;
  for( uint16_t oi = 0; oi < EEPROM_BUFFER_SECTOR_COUNT; oi++)
  {
      buffFlush.dataByte[0] = bufferEEPROM[pos];
      buffFlush.dataByte[1] = bufferEEPROM[pos+1];
      buffFlush.dataByte[2] = bufferEEPROM[pos+2];
      buffFlush.dataByte[3] = bufferEEPROM[pos+3];
      pos+=4;
      EE_WriteVariable(oi+VIRT_ADDR, buffFlush.dataWord);
  }
  return true;
}

void eeprom_buffer_read(uint16_t addr, uint8_t *data, uint16_t size)
{
  for ( uint16_t pos = 0; pos < size; pos++)
  {
      data[pos] = bufferEEPROM[pos+addr+5];
  }
}

void eeprom_buffer_write(uint16_t addr, uint8_t *data, uint16_t size)
{
  for ( uint16_t pos = 0; pos < size; pos++)
  {
      bufferEEPROM[pos+addr+5] = data[pos];
  }
}


uint16_t readWordFromEEPROM(uint16_t addr)
{
	uint16_t buf;
//	FLASH_FlushCaches()
//	EE_ReadVariable(addr, &buf);
//	return buf;
    return (*(__IO uint16_t *)addr);

}

void writeWordToEEPROM (uint16_t addr,uint16_t data)
{
	// !! word is 4byte
//	uint32_t targetAddr = STORE_ADDR + addr;
	HAL_StatusTypeDef flash_ok = HAL_ERROR;
	// Unlock EEPROM before erase and program
	while (flash_ok != HAL_OK) {
		flash_ok = HAL_FLASH_Unlock();
//		HAL_FLASH
//				HAL_FLASHEx_DATAEEPROM_Unlock();
	}
	flash_ok = HAL_ERROR;
	while (flash_ok != HAL_OK) {
		flash_ok = EE_WriteVariable(addr, data);

//				HAL_FLASHEx_DATAEEPROM_Erase(targetAddr);
	}
//	flash_ok = HAL_ERROR;
//	while (flash_ok != HAL_OK) {
//		flash_ok = HAL_FLASHEx_DATAEEPROM_Program(FLASH_TYPEPROGRAM_WORD,targetAddr,data);
//
//	}
	flash_ok = HAL_ERROR;
	while (flash_ok != HAL_OK) {
		flash_ok = HAL_FLASH_Lock();
	}
}
