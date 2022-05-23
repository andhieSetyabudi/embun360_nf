/*
 * myOneWire.h
 *
 *  Created on: Mar 1, 2022
 *      Author: andhie
 */

#ifndef LIB_DS18B20_MYONEWIRE_H_
#define LIB_DS18B20_MYONEWIRE_H_

#include "BSP.h"
#include "stdio.h"
#include "stdbool.h"
#include "string.h"

/* Pin settings */
#define TM_GPIO_SetPinLow(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(((uint32_t)GPIO_Pin) << 16))
#define TM_GPIO_SetPinHigh(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(GPIO_Pin))
#define TM_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)	(((GPIOx)->IDR & (GPIO_Pin)) == 0 ? 0 : 1)

#define ONEWIRE_LOW(structure)						TM_GPIO_SetPinLow((structure)->GPIOx, (structure)->GPIO_Pin)
#define ONEWIRE_HIGH(structure)						TM_GPIO_SetPinHigh((structure)->GPIOx, (structure)->GPIO_Pin)
#define ONEWIRE_INPUT(structure)					TM_GPIO_SetPinAsInput((structure)->GPIOx, (structure)->GPIO_Pin)
#define ONEWIRE_OUTPUT(structure)					TM_GPIO_SetPinAsOutput((structure)->GPIOx, (structure)->GPIO_Pin)
#define ONEWIRE_GET_INPUT(structure)				TM_GPIO_GetInputPinValue(structure->GPIOx, (structure)->GPIO_Pin)

/* OneWire commands */
#define ONEWIRE_CMD_RSCRATCHPAD			0xBE
#define ONEWIRE_CMD_WSCRATCHPAD			0x4E
#define ONEWIRE_CMD_CPYSCRATCHPAD		0x48
#define ONEWIRE_CMD_RECEEPROM			0xB8
#define ONEWIRE_CMD_RPWRSUPPLY			0xB4
#define ONEWIRE_CMD_SEARCHROM			0xF0
#define ONEWIRE_CMD_READROM				0x33
#define ONEWIRE_CMD_MATCHROM			0x55
#define ONEWIRE_CMD_SKIPROM				0xCC

typedef struct OneWire_t_{
	PORT_IO pinDesc;
	GPIO_TypeDef* GPIOx;           /*!< GPIOx port to be used for I/O functions */
	uint16_t GPIO_Pin;
	uint8_t LastDiscrepancy;       /*!< Search private */
	uint8_t LastFamilyDiscrepancy; /*!< Search private */
	bool LastDeviceFlag;        /*!< Search private */
	uint8_t ROM_NO[8];             /*!< 8-bytes address of last search device */
} OneWire_t;


void  myOneWire_begin(OneWire_t* var, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin);
uint8_t myOneWire_reset(OneWire_t* var);
void myOneWire_write_bit(OneWire_t* var, uint8_t v);
uint8_t myOneWire_read_bit(OneWire_t* var);
void myOneWire_write(OneWire_t* var, uint8_t v, uint8_t power /* = 0 */);
void myOneWire_write_bytes(OneWire_t* var, const uint8_t *buf, uint16_t count, bool power /* = 0 */);
uint8_t myOneWire_read(OneWire_t* var);
void myOneWire_read_bytes(OneWire_t* var, uint8_t *buf, uint16_t count);
void myOneWire_select(OneWire_t* var, const uint8_t rom[8]);
void myOneWire_skip(OneWire_t* var);
void myOneWire_depower(OneWire_t* var);
void myOneWire_reset_search(OneWire_t* var);
void myOneWire_target_search(OneWire_t* var, uint8_t family_code);
bool myOneWire_search(OneWire_t* var, uint8_t *newAddr, bool search_mode /* = true */);
uint8_t myOneWire_crc8(const uint8_t *addr, uint8_t len);




#endif /* LIB_DS18B20_MYONEWIRE_H_ */
