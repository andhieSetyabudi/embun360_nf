/**
  ******************************************************************************
  * @file    EEPROM/EEPROM_Emulation/src/eeprom.c
  * @author  MCD Application Team
  * @brief   This file provides all the EEPROM emulation firmware functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/** @addtogroup EEPROM_Emulation
  * @{
  */

/* Includes ------------------------------------------------------------------*/
#include "eeprom.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
#define FLASH_USER_START_ADDR   ADDR_FLASH_SECTOR_11   /* Start @ of user Flash area */
#define FLASH_USER_END_ADDR     ADDR_FLASH_SECTOR_11  +  GetSectorSize(ADDR_FLASH_SECTOR_11) - 1 /* End @ of user Flash area : sector start address + sector size - 1 */

#define EEPROM_BUFFER_SIZE		1024
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint32_t FirstSector = 0, NbOfSectors = 0;
uint32_t Address = 0, SECTORError = 0;
__IO uint32_t data32 = 0 , MemoryProgramStatus = 0;

static uint8_t /*__attribute__ ((section(".ram")))*/ bufferEEPROM[EEPROM_BUFFER_SIZE];

/*Variable used for Erase procedure*/
static FLASH_EraseInitTypeDef EraseInitStruct;

/* Global variable used to store variable value in read sequence */
uint16_t DataVar = 0;

/* Virtual address defined by the user: 0xFFFF value is prohibited */
uint16_t VirtAddVarTab[NB_OF_VAR];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
//static uint16_t eepromReadVar(const uint32_t  pos, uint16_t* Data);


/**
  * @brief  Function reads a byte from the eeprom buffer
  * @param  pos : address to read
  * @retval byte : data read from eeprom
  */
uint8_t eeprom_read_buffer(const uint32_t pos)
{
  return bufferEEPROM[pos];
}

/**
  * @brief  Function writes a byte to the eeprom buffer
  * @param  pos : address to write
  * @param  value : value to write
  * @retval none
  */
void eeprom_write_buffer(uint32_t pos, uint8_t value)
{
	bufferEEPROM[pos] = value;
}

void eeprom_put_buffer(uint32_t pos, uint8_t *pData, uint32_t size_)
{
	for(uint32_t count = 0; count<size_; count++)
	{
		bufferEEPROM[pos+count] = pData[count];
	}
}


void eeprom_get_buffer(uint32_t pos, uint8_t *pData, uint32_t size_)
{
	eeprom_fillBuffer();
	for(uint32_t count = 0; count<size_; count++)
	{
		pData[count] = bufferEEPROM[pos+count];
	}
}

/**
  * @brief  This function copies the data from flash into the buffer
  * @param  none
  * @retval none
  */
void eeprom_fillBuffer(void)
{
  memcpy(bufferEEPROM, (uint8_t *)(FLASH_USER_START_ADDR), EEPROM_BUFFER_SIZE);
}
/**
  * @brief  This function writes the buffer content into the flash
  * @param  none
  * @retval none
  */
void eeprom_flushBuffer(void)
{
	EraseInitStruct.TypeErase = TYPEERASE_SECTORS;
	EraseInitStruct.Sector = GetSector(FLASH_USER_START_ADDR);
	EraseInitStruct.NbSectors = 1;
	EraseInitStruct.VoltageRange = VOLTAGE_RANGE;
	uint32_t SectorError = 0;
	HAL_StatusTypeDef flash_ok = HAL_ERROR;
	// Unlock EEPROM before erase and program
	while (flash_ok != HAL_OK) {
		flash_ok = HAL_FLASH_Unlock();
	}
	flash_ok = HAL_ERROR;
	while (flash_ok != HAL_OK) {
		flash_ok =  HAL_FLASHEx_Erase(&EraseInitStruct, &SectorError);//FLASH_EraseSector(GetSector(FLASH_USER_START_ADDR),VOLTAGE_RANGE);
	}

	flash_ok = HAL_ERROR;
	uint32_t addr_a = 0;
	if (flash_ok != HAL_OK) {
		while(addr_a < EEPROM_BUFFER_SIZE )
		{
			flash_ok = HAL_FLASH_Program(TYPEPROGRAM_BYTE,FLASH_USER_START_ADDR+addr_a, (uint32_t)bufferEEPROM[addr_a]);
			addr_a++;
		}
	}
	flash_ok = HAL_ERROR;
	while (flash_ok != HAL_OK) {
		flash_ok = HAL_FLASH_Lock();
	}

}




/**
  * @brief  Gets the sector of a given address
  * @param  None
  * @retval The sector of a given address
  */
uint32_t GetSector(uint32_t Address)
{
  uint32_t sector = 0;

  if((Address < ADDR_FLASH_SECTOR_1) && (Address >= ADDR_FLASH_SECTOR_0))
  {
    sector = FLASH_SECTOR_0;
  }
  else if((Address < ADDR_FLASH_SECTOR_2) && (Address >= ADDR_FLASH_SECTOR_1))
  {
    sector = FLASH_SECTOR_1;
  }
  else if((Address < ADDR_FLASH_SECTOR_3) && (Address >= ADDR_FLASH_SECTOR_2))
  {
    sector = FLASH_SECTOR_2;
  }
  else if((Address < ADDR_FLASH_SECTOR_4) && (Address >= ADDR_FLASH_SECTOR_3))
  {
    sector = FLASH_SECTOR_3;
  }
  else if((Address < ADDR_FLASH_SECTOR_5) && (Address >= ADDR_FLASH_SECTOR_4))
  {
    sector = FLASH_SECTOR_4;
  }
  else if((Address < ADDR_FLASH_SECTOR_6) && (Address >= ADDR_FLASH_SECTOR_5))
  {
    sector = FLASH_SECTOR_5;
  }
  else if((Address < ADDR_FLASH_SECTOR_7) && (Address >= ADDR_FLASH_SECTOR_6))
  {
    sector = FLASH_SECTOR_6;
  }
  else if((Address < ADDR_FLASH_SECTOR_8) && (Address >= ADDR_FLASH_SECTOR_7))
  {
    sector = FLASH_SECTOR_7;
  }
  else if((Address < ADDR_FLASH_SECTOR_9) && (Address >= ADDR_FLASH_SECTOR_8))
  {
    sector = FLASH_SECTOR_8;
  }
  else if((Address < ADDR_FLASH_SECTOR_10) && (Address >= ADDR_FLASH_SECTOR_9))
  {
    sector = FLASH_SECTOR_9;
  }
  else if((Address < ADDR_FLASH_SECTOR_11) && (Address >= ADDR_FLASH_SECTOR_10))
  {
    sector = FLASH_SECTOR_10;
  }
  else /* (Address < FLASH_END_ADDR) && (Address >= ADDR_FLASH_SECTOR_11) */
  {
    sector = FLASH_SECTOR_11;
  }

  return sector;
}

/**
  * @brief  Gets sector Size
  * @param  None
  * @retval The size of a given sector
  */
uint32_t GetSectorSize(uint32_t Sector)
{
  uint32_t sectorsize = 0x00;
  if((Sector == FLASH_SECTOR_0) || (Sector == FLASH_SECTOR_1) ||\
     (Sector == FLASH_SECTOR_2) || (Sector == FLASH_SECTOR_3))
  {
    sectorsize = 16 * 1024;
  }
  else if((Sector == FLASH_SECTOR_4))
  {
    sectorsize = 64 * 1024;
  }
  else
  {
    sectorsize = 128 * 1024;
  }
  return sectorsize;
}


/**
  * @brief  Returns the last stored variable data, if found, which correspond to
  *   the passed virtual address
  * @param  VirtAddress: Variable virtual address
  * @param  Data: Global variable contains the read variable value
  * @retval Success or error status:
  *           - 0: if variable was found
  *           - 1: if the variable was not found
  *           - NO_VALID_PAGE: if no valid page was found.
  */
uint16_t eepromReadVar(const uint32_t  pos, uint16_t* Data)
{
	 __IO uint16_t data = 0;
	  if (pos <= (FLASH_USER_END_ADDR - FLASH_USER_START_ADDR)) {
	    /* with actual EEPROM, pos is a relative address */
	    data = *(__IO uint16_t *)(FLASH_USER_START_ADDR + pos);
	    *Data = (uint16_t)data;
	  }
	  return (uint16_t)data;
}



/**
  * @}
  */

/******************* (C) COPYRIGHT 2011 STMicroelectronics *****END OF FILE****/
