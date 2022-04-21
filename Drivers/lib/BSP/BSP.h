/*
 * BSP.h
 *
 *  Created on: Feb 23, 2022
 *      Author: andhie
 */

#ifndef LIB_BSP_BSP_H_
#define LIB_BSP_BSP_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "stdbool.h"
#include "stm32f2xx_hal.h"
#include "stm32f2xx_hal_flash_ex.h"
#include "stm32f2xx_hal_rtc.h"
#include "eeprom.h"

#define lowByte(w) ((uint8_t) ((w) & 0xff))
#define highByte(w) ((uint8_t) ((w) >> 8))

#define bitRead(value, bit) (((value) >> (bit)) & 0x01)
#define bitSet(value, bit) ((value) |= (1UL << (bit)))
#define bitClear(value, bit) ((value) &= ~(1UL << (bit)))
#define bitWrite(value, bit, bitvalue) (bitvalue ? bitSet(value, bit) : bitClear(value, bit))

#define bit(b) (1UL << (b))

char * getUUID(char * buff);

#define MAX_PRECISION	(6)
static const float rounders[MAX_PRECISION + 1] =
{
	0.5,				// 0
	0.05,				// 1
	0.005,				// 2
	0.0005,				// 3
	0.00005,			// 4,
	0.000005,			// 5,
	0.0000005			// 6
};

/* Usage parse the number and turn num <-> ascii*/
#define CHARISNUM(x)                        ((x) >= '0' && (x) <= '9')
#define CHARTONUM(x)                        ((x) - '0')
#define NUMTOCHAR(x)					    ((x)+'0')

char* ftoa(float f, char * buf, int precision);

uint16_t 	__attribute__((optimize("O2"))) twoByteTo16bit(uint8_t *tmp);
void 		__attribute__((optimize("O2"))) uint16toTwoByte(uint8_t *tmp, uint16_t val);
uint32_t 	__attribute__((optimize("O2"))) fourByteToUint32t(uint8_t *tmp);
float 		__attribute__((optimize("O2"))) fourByteToFloat(uint8_t *tmp);
void 		__attribute__((optimize("O2"))) floatToFourByte(uint8_t *tmp,float val);

#define PIN_INPUT			0
#define PIN_INPUTPULLUP		1
#define PIN_OUTPUT			2

#define PORT_LOW	GPIO_PIN_RESET
#define PORT_HIGH	GPIO_PIN_SET

typedef struct __attribute__ ((__packed__)) PORT_IO_{
	  GPIO_TypeDef* port;
	  uint16_t pin;
}PORT_IO;

static void __inline__ pinMode(PORT_IO port_map, uint8_t mode)
{
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = port_map.pin;

	if( mode == PIN_OUTPUT)
	{
		HAL_GPIO_WritePin(port_map.port, port_map.pin, GPIO_PIN_RESET);
		GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
		GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
	} else if( mode == PIN_INPUTPULLUP)
	{
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_PULLUP;
	}else
	{
		GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
		GPIO_InitStruct.Pull = GPIO_NOPULL;
	}
	HAL_GPIO_Init(port_map.port, &GPIO_InitStruct);
}

static void __inline__ digitalWrite(PORT_IO port_map, uint8_t logic)
{
	HAL_GPIO_WritePin(port_map.port, port_map.pin, logic);
}

static uint8_t __inline__ digitalRead(PORT_IO port_map)
{
	return (uint8_t) HAL_GPIO_ReadPin(port_map.port, port_map.pin);
}

static void __inline__ digitalToggle(PORT_IO port_map)
{
	HAL_GPIO_TogglePin(port_map.port, port_map.pin);
}


#define UPPER_LIMIT_FIN_TEMP	100.f

#define LIMIT_MAX_FIN_TEMP	7.0f
#define LIMIT_MIN_FIN_TEMP	2.0f

static void __inline__ normalizeMinimumTemperature(float *temp)
{
	if( *temp > LIMIT_MAX_FIN_TEMP )
		*temp = LIMIT_MIN_FIN_TEMP;
	else if( *temp < LIMIT_MIN_FIN_TEMP )
		*temp = LIMIT_MAX_FIN_TEMP;
}


#define LIMIT_MAX_HUMIDITY 90.f
#define LIMIT_MIN_HUMIDITY 35.f
static void __inline__ normalizeMinimumHumidity(float *temp)
{
	if( *temp > LIMIT_MAX_HUMIDITY )
		*temp = LIMIT_MIN_HUMIDITY;
	else if( *temp < LIMIT_MIN_HUMIDITY )
		*temp = LIMIT_MAX_HUMIDITY;
}

#define DEFAULT_WL_CONST	73.58f	// constant value of waterflow
#define DEFAULT_WT_CONST	200.f	// maximum of water tank ( tandon air )

#define STORE_ADDR		0x08080000

#define   _EE_USE_FLASH_PAGE_OR_SECTOR              (127)
#define   _EE_USE_RAM_BYTE                          (1024)
#define   _EE_VOLTAGE                               FLASH_VOLTAGE_RANGE_3 //  use in some devices

#define   _EE_SIZE              (1024 * 128)
#define   _EE_ADDR_INUSE        (((uint32_t)0x08080000) | (_EE_SIZE*(_EE_USE_FLASH_PAGE_OR_SECTOR - 5)))
#define   _EE_FLASH_BANK        FLASH_BANK_1
#define   _EE_VOLTAGE_RANGE     _EE_VOLTAGE
#define   _EE_PAGE_OR_SECTOR    SECTOR

static inline bool ee_read(uint32_t startVirtualAddress, uint32_t len, uint8_t* data)
{
  if ((startVirtualAddress + len) > _EE_SIZE)
    return false;
  for (uint32_t i = startVirtualAddress; i < len + startVirtualAddress; i++)
  {
    if (data != NULL)
    {
      *data = (*(__IO uint8_t*) (i + 0x08080000));
      data++;
    }
  }
  return true;
}

static inline uint32_t ee_readWord(uint32_t faddr)
{
	return *(uint32_t*)faddr;
}

typedef union
{
  uint32_t dataWord;
  uint8_t dataByte[4];
}parsing;

static inline void ee_reads(uint32_t ReadAddr,uint32_t NumToRead,uint8_t *pBuffer)
{
	uint32_t i;
	parsing buffRead;
//	 bufferEEPROM[EEPROM_BUFFER_SIZE];
	for(i=0;i<NumToRead;i++)
	{
		buffRead.dataWord=ee_readWord(ReadAddr);//读取4个字节.
		pBuffer[ReadAddr] 		= buffRead.dataByte[0];
		pBuffer[ReadAddr+1] 	= buffRead.dataByte[1];
		pBuffer[ReadAddr+2] 	= buffRead.dataByte[2];
		pBuffer[ReadAddr+3] 	= buffRead.dataByte[3];
		ReadAddr+=4;//偏移4个字节.
	}
//	for
}

static inline void ee_write(uint32_t WriteAddr, uint32_t len, uint32_t *data)
{
	HAL_FLASH_Unlock();
		__HAL_FLASH_DATA_CACHE_DISABLE();
		//解锁
//	  FLASH_DataCacheCmd(DISABLE);//FLASH擦除期间,必须禁止数据缓存

		uint32_t addrx=WriteAddr+0x08080000;				//写入的起始地址
		uint32_t endaddr=addrx+len*4;	//写入的结束地址
		FLASH_Erase_Sector(FLASH_SECTOR_8,VOLTAGE_RANGE_3);
			while(WriteAddr<endaddr)//写数据
			{
				if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, 0x08080000+WriteAddr, *data)!=HAL_OK)//写入数据
				{
					break;	//写入异常
				}
				WriteAddr+=4;
				data++;
			}
		__HAL_FLASH_DATA_CACHE_ENABLE();
		HAL_FLASH_Lock();//上锁
}

#define EEPROM_BUFFER_SIZE		1024
#define EEPROM_BUFFER_SECTOR		4
#define EEPROM_BUFFER_SECTOR_COUNT	(EEPROM_BUFFER_SIZE/EEPROM_BUFFER_SECTOR)
static inline void ee_writes(uint32_t WriteAddr, uint32_t len, uint8_t *data)
{
	uint32_t tmpSize = len/4;
	if( len % 4 != 0 )
		tmpSize +=1;

	parsing buffFlush;
	uint32_t flushBuf[tmpSize];
	uint16_t pos = 0;
//	uint32_t loc = 0;
	for( uint16_t oi = 0; oi < tmpSize; oi++)
	{
	  buffFlush.dataByte[0] = data[pos];
	  buffFlush.dataByte[1] = data[pos+1];
	  buffFlush.dataByte[2] = data[pos+2];
	  buffFlush.dataByte[3] = data[pos+3];
	  flushBuf[oi] = buffFlush.dataWord;
	  pos+=4;
//	  loc++;
	}
	ee_write(WriteAddr, tmpSize, flushBuf);
}

//uint8_t bufferEEPROM[EEPROM_BUFFER_SIZE];
//for ( uint16_t pos = 0; pos < len; pos++)
//{
//  bufferEEPROM[pos+addr+5] = data[pos];
//}

//##########################################################################################################

void initEEPROM();
uint16_t readWordFromEEPROM(uint16_t addr);
void writeWordToEEPROM (uint16_t addr,uint16_t data);
#endif /* LIB_BSP_BSP_H_ */
