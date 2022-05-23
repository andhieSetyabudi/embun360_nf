/*
 * myOneWire.c
 *
 *  Created on: Mar 1, 2022
 *      Author: andhie
 */

#include "myOneWire.h"
#include "delayDWT.h"

void TM_GPIO_SetPinAsInput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	uint8_t i;
	/* Go through all pins */
	for (i = 0x00; i < 0x10; i++) {
		/* Pin is set */
		if (GPIO_Pin & (1 << i)) {
			/* Set 00 bits combination for input */
			GPIOx->MODER &= ~(0x03 << (2 * i));
		}
	}
}

void TM_GPIO_SetPinAsOutput(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin) {
	uint8_t i;
	/* Go through all pins */
	for (i = 0x00; i < 0x10; i++) {
		/* Pin is set */
		if (GPIO_Pin & (1 << i)) {
			/* Set 01 bits combination for output */
			GPIOx->MODER = (GPIOx->MODER & ~(0x03 << (2 * i))) | (0x01 << (2 * i));
		}
	}
}


void LH_signal(OneWire_t* var, uint32_t L_time, uint32_t H_time) {
	HAL_GPIO_WritePin(var->pinDesc.port, var->pinDesc.pin, GPIO_PIN_RESET);
	DWT_Delay_us(L_time);//From pullup_HIGH to GND_LOW:---___
	HAL_GPIO_WritePin(var->pinDesc.port, var->pinDesc.pin, GPIO_PIN_SET);
	DWT_Delay_us(H_time);//From GND_LOW to pullup_HIGH:___---
}


void  myOneWire_begin(OneWire_t* var, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin)
{
	var->pinDesc.pin = GPIO_Pin;
	var->pinDesc.port = GPIOx;
	var->GPIOx = GPIOx;
	var->GPIO_Pin = GPIO_Pin;
	ONEWIRE_INPUT(var);
//	pinMode(var->pinDesc, PIN_INPUT);
	myOneWire_reset_search(var);
}

uint8_t myOneWire_reset(OneWire_t* var)
{
	uint8_t r;
	uint8_t retries = 125;

//	pinMode(var->pinDesc, PIN_INPUT);
	ONEWIRE_INPUT(var);
	// wait until the wire is high... just in case
	do {
		if (--retries == 0) return 0;
		DWT_Delay_us(2);
	} //while ( !digitalRead(var->pinDesc));
	while(!ONEWIRE_GET_INPUT(var));

	ONEWIRE_LOW(var);
	ONEWIRE_OUTPUT(var);
	DWT_Delay_us(480);

	/* Release line and wait for 70us */
	ONEWIRE_INPUT(var);
	DWT_Delay_us(70);

	/* Check bit value */
	r = !TM_GPIO_GetInputPinValue(var->GPIOx, var->GPIO_Pin);

	/* Delay for 410 us */
	DWT_Delay_us(410);
	return r;
}

void myOneWire_write_bit(OneWire_t* var, uint8_t v)
{

	if (v) {
		/* Set line low */
		ONEWIRE_LOW(var);
		ONEWIRE_OUTPUT(var);
		DWT_Delay_us(10);

		/* Bit high */
		ONEWIRE_HIGH(var);

		/* Wait for 55 us and release the line */
		DWT_Delay_us(55);
	} else {
		/* Set line low */
		ONEWIRE_LOW(var);
		ONEWIRE_OUTPUT(var);
		DWT_Delay_us(65);

		/* Bit high */
		ONEWIRE_HIGH(var);

		/* Wait for 5 us and release the line */
		DWT_Delay_us(5);
	}
}

uint8_t myOneWire_read_bit(OneWire_t* var)
{
	uint8_t r;
	ONEWIRE_OUTPUT(var);
	ONEWIRE_LOW(var);
	DWT_Delay_us(3);
	ONEWIRE_INPUT(var);	// let pin float, pull up will raise
	DWT_Delay_us(10);
	r = TM_GPIO_GetInputPinValue(var->GPIOx, var->GPIO_Pin);
	DWT_Delay_us(55);
	return r;

//	pinMode(var->pinDesc, PIN_OUTPUT);
//	digitalWrite(var->pinDesc, PORT_LOW);
//	DWT_Delay_us(3);
//	pinMode(var->pinDesc, PIN_INPUT);
//	DWT_Delay_us(10);
//	uint8_t r = (digitalRead(var->pinDesc) ? 1 : 0);
//	DWT_Delay_us(53);
//	return r;
}

void myOneWire_write(OneWire_t* var, uint8_t v, uint8_t power /* = 0 */) {


	uint8_t bitMask;

	for (bitMask = 0x01; bitMask; bitMask <<= 1) {
		myOneWire_write_bit(var, (bitMask & v)?1:0);
	}
	if ( !power) {
		ONEWIRE_INPUT(var);
		ONEWIRE_LOW(var);
	}
}

void myOneWire_write_bytes(OneWire_t* var, const uint8_t *buf, uint16_t count, bool power /* = 0 */) {
	for (uint16_t i = 0 ; i < count ; i++)
		myOneWire_write(var,buf[i], 0);
	if (!power) {
		ONEWIRE_INPUT(var);
		ONEWIRE_LOW(var);
	}
}

uint8_t myOneWire_read(OneWire_t* var) {
	uint8_t bitMask;
	uint8_t r = 0;

	for (bitMask = 0x01; bitMask; bitMask <<= 1) {
		if ( myOneWire_read_bit(var))
				r |= bitMask;
	}
	return r;
}

void myOneWire_read_bytes(OneWire_t* var, uint8_t *buf, uint16_t count) {
  for (uint16_t i = 0 ; i < count ; i++)
    buf[i] = myOneWire_read(var);
}

void myOneWire_select(OneWire_t* var, const uint8_t rom[8])
{
    uint8_t i;
    myOneWire_write(var, 0x55, 0);           // Choose ROM

    for (i = 0; i < 8; i++)
    	myOneWire_write(var, rom[i],0);
}

void myOneWire_skip(OneWire_t* var)
{
	myOneWire_write(var, 0xCC, 0);           // Skip ROM
}

void myOneWire_depower(OneWire_t* var)
{
	ONEWIRE_INPUT(var);
}

void myOneWire_reset_search(OneWire_t* var)
{
  // reset the search state
  var->LastDiscrepancy = 0;
  var->LastDeviceFlag = false;
  var->LastFamilyDiscrepancy = 0;
  for(int i = 7; ; i--) {
	  var->ROM_NO[i] = 0;
    if ( i == 0) break;
  }
}

void myOneWire_target_search(OneWire_t* var, uint8_t family_code)
{
   // set the search state to find SearchFamily type devices
   var->ROM_NO[0] = family_code;
   for (uint8_t i = 1; i < 8; i++)
	   var->ROM_NO[i] = 0;
   var->LastDiscrepancy = 64;
   var->LastFamilyDiscrepancy = 0;
   var->LastDeviceFlag = false;
}

bool myOneWire_search(OneWire_t* var, uint8_t *newAddr, bool search_mode /* = true */)
{
   uint8_t id_bit_number;
   uint8_t last_zero, rom_byte_number;
   bool    search_result;
   uint8_t id_bit, cmp_id_bit;

   unsigned char rom_byte_mask, search_direction;

   // initialize for search
   id_bit_number = 1;
   last_zero = 0;
   rom_byte_number = 0;
   rom_byte_mask = 1;
   search_result = false;

   // if the last call was not the last one
   if (!var->LastDeviceFlag) {
      // 1-Wire reset
      if (!myOneWire_reset(var)) {
         // reset the search
			var->LastDiscrepancy = 0;
			var->LastDeviceFlag = false;
			var->LastFamilyDiscrepancy = 0;
			return false;
      }

      // issue the search command
      if (search_mode == true) {
    	  myOneWire_write(var,0xF0,0);   // NORMAL SEARCH
      } else {
    	  myOneWire_write(var,0xEC,0);   // CONDITIONAL SEARCH
      }

      // loop to do the search
      do
      {
         // read a bit and its complement
         id_bit = myOneWire_read_bit(var);
         cmp_id_bit = myOneWire_read_bit(var);

         // check for no devices on 1-wire
         if ((id_bit == 1) && (cmp_id_bit == 1)) {
            break;
         } else {
            // all devices coupled have 0 or 1
            if (id_bit != cmp_id_bit) {
               search_direction = id_bit;  // bit write value for search
            } else {
               // if this discrepancy if before the Last Discrepancy
               // on a previous next then pick the same as last time
               if (id_bit_number < var->LastDiscrepancy) {
                  search_direction = ((var->ROM_NO[rom_byte_number] & rom_byte_mask) > 0);
               } else {
                  // if equal to last pick 1, if not then pick 0
                  search_direction = (id_bit_number == var->LastDiscrepancy);
               }
               // if 0 was picked then record its position in LastZero
               if (search_direction == 0) {
                  last_zero = id_bit_number;

                  // check for Last discrepancy in family
                  if (last_zero < 9)
                	  var->LastFamilyDiscrepancy = last_zero;
               }
            }

            // set or clear the bit in the ROM byte rom_byte_number
            // with mask rom_byte_mask
            if (search_direction == 1)
            	var->ROM_NO[rom_byte_number] |= rom_byte_mask;
            else
            	var->ROM_NO[rom_byte_number] &= ~rom_byte_mask;

            // serial number search direction write bit
            myOneWire_write_bit(var,search_direction);

            // increment the byte counter id_bit_number
            // and shift the mask rom_byte_mask
            id_bit_number++;
            rom_byte_mask <<= 1;

            // if the mask is 0 then go to new SerialNum byte rom_byte_number and reset mask
            if (rom_byte_mask == 0) {
                rom_byte_number++;
                rom_byte_mask = 1;
            }
         }
      }
      while(rom_byte_number < 8);  // loop until through all ROM bytes 0-7

      // if the search was successful then
      if (!(id_bit_number < 65)) {
         // search successful so set LastDiscrepancy,LastDeviceFlag,search_result
    	  var->LastDiscrepancy = last_zero;

         // check for last device
         if (var->LastDiscrepancy == 0) {
        	 var->LastDeviceFlag = true;
         }
         search_result = true;
      }
   }

   // if no device found then reset counters so next 'search' will be like a first
   if (!search_result || !var->ROM_NO[0]) {
	   var->LastDiscrepancy = 0;
	   var->LastDeviceFlag = false;
	   var->LastFamilyDiscrepancy = 0;
      search_result = false;
   } else {
	   memcpy(newAddr, var->ROM_NO, 8);
//      for (int i = 0; i < 8; i++)
//    	  newAddr[i] = var->ROM_NO[i];
   }
   return search_result;
  }


uint8_t myOneWire_crc8(const uint8_t *addr, uint8_t len)
{
	uint8_t crc = 0;

	while (len--) {
		uint8_t inbyte = *addr++;
		for (uint8_t i = 8; i; i--) {
			uint8_t mix = (crc ^ inbyte) & 0x01;
			crc >>= 1;
			if (mix) crc ^= 0x8C;
			inbyte >>= 1;
		}
	}
	return crc;
}
