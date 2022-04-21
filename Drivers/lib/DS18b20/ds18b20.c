/*
 * ds18b20.c
 *
 *  Created on: Mar 1, 2022
 *      Author: andhie
 */

#include "ds18b20.h"
#include "delayDWT.h"

// Model IDs
#define DS18S20MODEL 0x10  // also DS1820
#define DS18B20MODEL 0x28  // also MAX31820
#define DS1822MODEL  0x22
#define DS1825MODEL  0x3B
#define DS28EA00MODEL 0x42

#define DSROM_FAMILY    0
#define DSROM_CRC       7
// OneWire commands
#define STARTCONVO      0x44  // Tells device to take a temperature reading and put it on the scratchpad
#define COPYSCRATCH     0x48  // Copy scratchpad to EEPROM
#define READSCRATCH     0xBE  // Read from scratchpad
#define WRITESCRATCH    0x4E  // Write to scratchpad
#define RECALLSCRATCH   0xB8  // Recall from EEPROM to scratchpad
#define READPOWERSUPPLY 0xB4  // Determine if device needs parasite power
#define ALARMSEARCH     0xEC  // Query bus for devices with an alarm condition

// Scratchpad locations
#define TEMP_LSB        0
#define TEMP_MSB        1
#define HIGH_ALARM_TEMP 2
#define LOW_ALARM_TEMP  3
#define CONFIGURATION   4
#define INTERNAL_BYTE   5
#define COUNT_REMAIN    6
#define COUNT_PER_C     7
#define SCRATCHPAD_CRC  8



bool ds18b20_validFamily(const uint8_t* deviceAddress) {
	switch (deviceAddress[7]) {
		case DS18S20MODEL:
		case DS18B20MODEL:
		case DS1822MODEL:
		case DS1825MODEL:
		case DS28EA00MODEL:
			return true;
		default:
			return false;
		}
}

bool ds18b20_validAddress(const uint8_t* deviceAddress) {
	return (myOneWire_crc8(deviceAddress, 7) == deviceAddress[7]);
}

bool ds18b20_readPowerSupply(ds18b20_t* var, const uint8_t* deviceAddress)
{
	bool parasiteMode = false;
	myOneWire_reset(&var->wire_t);
	if (deviceAddress == NULL)
		myOneWire_skip(&var->wire_t);
	else
		myOneWire_select(&var->wire_t,deviceAddress);

	myOneWire_write(&var->wire_t,READPOWERSUPPLY,0);
	if (myOneWire_read_bit(&var->wire_t) == 0)
		parasiteMode = true;
	myOneWire_reset(&var->wire_t);
	return parasiteMode;
}

float ds18b20_rawToCelsius(int16_t raw) {

	if (raw <= DEVICE_DISCONNECTED_RAW)
		return DEVICE_DISCONNECTED_C;
	// C = RAW/128
	return (float) raw * 0.0078125f;

}

bool ds18b20_isAllZeros(const uint8_t * scratchPad, const size_t length) {
	for (size_t i = 0; i < length; i++) {
		if (scratchPad[i] != 0) {
			return false;
		}
	}

	return true;
}


void ds18b20_init(ds18b20_t* var, GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin){
	myOneWire_begin(&var->wire_t, GPIOx, GPIO_Pin);
	var->numberOfDevices = 0;
	var->globalResolution = 9;
	var->selectedResolution = 0;
	var->selectedPowerMode = 0;
	memset(var->searchAddress,0,8);
	memset(var->selectedAddress,0,8);
	memset(var->selectedScratchpad,0,SIZE_SCRATCHPAD);
	ds18b20_resetSearch(var);
	uint8_t deviceAddress[8];
	var->parasite = false;
	myOneWire_reset_search(&var->wire_t);
	while (myOneWire_search(&var->wire_t, deviceAddress, true)) {

		if (myOneWire_crc8(deviceAddress,7) == deviceAddress[7]) {
			var->numberOfDevices++;

			if (ds18b20_validFamily(deviceAddress)) {
				var->ds18Count++;

				if (!var->parasite && ds18b20_readPowerSupply(var,deviceAddress))
					var->parasite = true;

				uint8_t resolution = ds18b20_getResolution(var);
				if (resolution > var->globalResolution) {
					var->globalResolution = resolution;
				}
			}
		}
	}
	memcpy(var->selectedAddress, deviceAddress, 8);
}

bool ds18b20_select(ds18b20_t* var, uint8_t address[]) {
    if (ds18b20_isConnected(var, address)) {
        memcpy(var->selectedAddress, address, 8);

        if (ds18b20_readScratchpad(var)) {
        	var->selectedResolution = ds18b20_getResolution(var);

            ds18b20_sendCommand(var, MATCH_ROM, READ_POWER_SUPPLY, 0);
            var->selectedPowerMode = myOneWire_read_bit(&var->wire_t);
            return true;
        }
    }
    return false;
}

bool ds18b20_selectNext(ds18b20_t* var) {
	bool ret = false;
    if ( ds18b20_oneWireSearch(var, SEARCH_ROM) ) {
        ret = ds18b20_select(var, var->searchAddress);
    }
    return ret;
}

uint8_t ds18b20_selectNextAlarm(ds18b20_t* var) {
    if (ds18b20_oneWireSearch(var,ALARM_SEARCH)) {
        return ds18b20_select(var, var->searchAddress);
    }
    return 0;
}

void ds18b20_resetSearch(ds18b20_t* var) {
    var->lastDiscrepancy = 0;
    var->lastDevice = 0;
}

float ds18b20_getTempC(ds18b20_t* var) {
	ds18b20_sendCommand(var, MATCH_ROM, CONVERT_T, !var->selectedPowerMode);
	ds18b20_delayForConversion(var,var->selectedResolution, var->selectedPowerMode);
	ds18b20_readScratchpad(var);
    uint8_t lsb = var->selectedScratchpad[TEMP_LSB];
    uint8_t msb = var->selectedScratchpad[TEMP_MSB];

    switch (var->selectedResolution) {
        case 9:
            lsb &= 0xF8;
            break;
        case 10:
            lsb &= 0xFC;
            break;
        case 11:
            lsb &= 0xFE;
            break;
    }

    uint8_t sign = msb & 0x80;
    int16_t temp = (msb << 8) + lsb;

    if (sign) {
        temp = ((temp ^ 0xffff) + 1) * -1;
    }

    return temp / 16.0;
}

int16_t ds18b20_calculateTemperature(const uint8_t* deviceAddress, uint8_t* scratchPad) {

	int16_t fpTemperature = (((int16_t) scratchPad[TEMP_MSB]) << 11)
			| (((int16_t) scratchPad[TEMP_LSB]) << 3);

	/*
	 DS1820 and DS18S20 have a 9-bit temperature register.
	 Resolutions greater than 9-bit can be calculated using the data from
	 the temperature, and COUNT REMAIN and COUNT PER °C registers in the
	 scratchpad.  The resolution of the calculation depends on the model.
	 While the COUNT PER °C register is hard-wired to 16 (10h) in a
	 DS18S20, it changes with temperature in DS1820.
	 After reading the scratchpad, the TEMP_READ value is obtained by
	 truncating the 0.5°C bit (bit 0) from the temperature data. The
	 extended resolution temperature can then be calculated using the
	 following equation:
	                                  COUNT_PER_C - COUNT_REMAIN
	 TEMPERATURE = TEMP_READ - 0.25 + --------------------------
	                                         COUNT_PER_C
	 Hagai Shatz simplified this to integer arithmetic for a 12 bits
	 value for a DS18S20, and James Cameron added legacy DS1820 support.
	 See - http://myarduinotoy.blogspot.co.uk/2013/02/12bit-result-from-ds18s20.html
	 */

	if ((deviceAddress[DSROM_FAMILY] == DS18S20MODEL) && (scratchPad[COUNT_PER_C] != 0)) {
		fpTemperature = ((fpTemperature & 0xfff0) << 3) - 32
				+ (((scratchPad[COUNT_PER_C] - scratchPad[COUNT_REMAIN]) << 7)
						/ scratchPad[COUNT_PER_C]);
	}

	return fpTemperature;
}

void ds18b20_requestTemperatures(ds18b20_t* var) {
	myOneWire_reset(&var->wire_t);
	myOneWire_skip(&var->wire_t);
	myOneWire_write(&var->wire_t,STARTCONVO, var->parasite);
	ds18b20_delayForConversion(var, var->globalResolution, false);
}

float ds18b20_getTempCByAddress(ds18b20_t* var, uint8_t address[], bool newRequest)
{
	if( newRequest )
		ds18b20_requestTemperatures(var);
	uint8_t scratchPad[9];
		if (ds18b20_isConnected_(var, address, scratchPad))
			return ds18b20_rawToCelsius(ds18b20_calculateTemperature(address, scratchPad));
		return DEVICE_DISCONNECTED_RAW;
}

float ds18b20_getTempF(ds18b20_t* var) {
    return ds18b20_getTempC(var) * 1.8 + 32;
}

uint8_t ds18b20_getResolution(ds18b20_t* var) {
    switch (var->selectedScratchpad[CONFIGURATION]) {
        case RES_9_BIT:
            return 9;
        case RES_10_BIT:
            return 10;
        case RES_11_BIT:
            return 11;
        case RES_12_BIT:
            return 12;
    }
    return 0;
}

void ds18b20_setResolution(ds18b20_t* var, uint8_t resolution) {
	resolution = resolution < 9 ? 9 : (resolution > 12 ? 12 : resolution);
    switch (resolution) {
        case 9:
            var->selectedScratchpad[CONFIGURATION] = RES_9_BIT;
            break;
        case 10:
        	var->selectedScratchpad[CONFIGURATION] = RES_10_BIT;
            break;
        case 11:
        	var->selectedScratchpad[CONFIGURATION] = RES_11_BIT;
            break;
        case 12:
        	var->selectedScratchpad[CONFIGURATION] = RES_12_BIT;
            break;
    }

    if (resolution > var->globalResolution) {
    	var->globalResolution = resolution;
    }

    ds18b20_writeScratchpad(var);
}

uint8_t ds18b20_getPowerMode(ds18b20_t* var) {
    return var->selectedPowerMode;
}

uint8_t ds18b20_getFamilyCode(ds18b20_t* var) {
    return var->selectedAddress[0];
}

void ds18b20_getAddress(ds18b20_t* var, uint8_t address[]) {
    memcpy(address, var->selectedAddress, 8);
}

void ds18b20_doConversion(ds18b20_t* var) {
	ds18b20_sendCommand(var, SKIP_ROM, CONVERT_T, !var->globalPowerMode);
	ds18b20_delayForConversion(var, var->globalResolution, var->globalPowerMode);
}

uint8_t ds18b20_getNumberOfDevices(ds18b20_t* var) {
    return var->numberOfDevices;
}

uint8_t ds18b20_hasAlarm(ds18b20_t* var) {
    uint8_t oldResolution = var->selectedResolution;
    ds18b20_setResolution(var, 9);
    float temp = ds18b20_getTempC(var);
    ds18b20_setResolution(var, oldResolution);
    return ((temp <= var->selectedScratchpad[ALARM_LOW]) || (temp >= var->selectedScratchpad[ALARM_HIGH]));
}

void ds18b20_setAlarms(ds18b20_t* var, int8_t alarmLow, int8_t alarmHigh) {
	ds18b20_setAlarmLow(var, alarmLow);
	ds18b20_setAlarmHigh(var, alarmHigh);
	ds18b20_writeScratchpad(var);
}

int8_t ds18b20_getAlarmLow(ds18b20_t* var) {
    return var->selectedScratchpad[ALARM_LOW];
}

void ds18b20_setAlarmLow(ds18b20_t* var, int8_t alarmLow) {
	alarmLow = alarmLow < -55 ? -55 : (alarmLow > 125 ? 125 : alarmLow );
    var->selectedScratchpad[ALARM_LOW] = alarmLow;
    ds18b20_writeScratchpad(var);
}

int8_t ds18b20_getAlarmHigh(ds18b20_t* var) {
    return var->selectedScratchpad[ALARM_HIGH];
}

void ds18b20_setAlarmHigh(ds18b20_t* var, int8_t alarmHigh) {
	alarmHigh = alarmHigh < -55 ? -55 : (alarmHigh > 125 ? 125 : alarmHigh );
	var->selectedScratchpad[ALARM_HIGH] = alarmHigh;
	ds18b20_writeScratchpad(var);
}

void ds18b20_setRegisters(ds18b20_t* var, int8_t lowRegister, int8_t highRegister) {
	ds18b20_setAlarms(var, lowRegister, highRegister);
}

int8_t ds18b20_getLowRegister(ds18b20_t* var) {
    return ds18b20_getAlarmLow(var);
}

void ds18b20_setLowRegister(ds18b20_t* var, int8_t lowRegister) {
	ds18b20_setAlarmLow(var, lowRegister);
}

int8_t ds18b20_getHighRegister(ds18b20_t* var) {
    return ds18b20_getAlarmHigh(var);
}

void ds18b20_setHighRegister(ds18b20_t* var, int8_t highRegister) {
	ds18b20_setAlarmHigh(var, highRegister);
}

uint8_t ds18b20_readScratchpad_(ds18b20_t* var, const uint8_t* deviceAddress,uint8_t* scratchPad) {

	// send the reset command and fail fast
		int b = myOneWire_reset(&var->wire_t);
		if (b == 0)
			return false;

		myOneWire_select(&var->wire_t, deviceAddress);
		myOneWire_write(&var->wire_t, READSCRATCH, 0);

		// Read all registers in a simple loop
		// byte 0: temperature LSB
		// byte 1: temperature MSB
		// byte 2: high alarm temp
		// byte 3: low alarm temp
		// byte 4: DS18S20: store for crc
		//         DS18B20 & DS1822: configuration register
		// byte 5: internal use & crc
		// byte 6: DS18S20: COUNT_REMAIN
		//         DS18B20 & DS1822: store for crc
		// byte 7: DS18S20: COUNT_PER_C
		//         DS18B20 & DS1822: store for crc
		// byte 8: SCRATCHPAD_CRC
		for (uint8_t i = 0; i < 9; i++) {
			scratchPad[i] = myOneWire_read(&var->wire_t);
		}

		b = myOneWire_reset(&var->wire_t);
		return (b == 1);
}

uint8_t ds18b20_readScratchpad(ds18b20_t* var) {

	// send the reset command and fail fast
		int b = myOneWire_reset(&var->wire_t);
		if (b == 0)
			return false;

		myOneWire_select(&var->wire_t, var->selectedAddress);
		myOneWire_write(&var->wire_t, READSCRATCH, 0);

		// Read all registers in a simple loop
		// byte 0: temperature LSB
		// byte 1: temperature MSB
		// byte 2: high alarm temp
		// byte 3: low alarm temp
		// byte 4: DS18S20: store for crc
		//         DS18B20 & DS1822: configuration register
		// byte 5: internal use & crc
		// byte 6: DS18S20: COUNT_REMAIN
		//         DS18B20 & DS1822: store for crc
		// byte 7: DS18S20: COUNT_PER_C
		//         DS18B20 & DS1822: store for crc
		// byte 8: SCRATCHPAD_CRC
		for (uint8_t i = 0; i < 9; i++) {
			var->selectedScratchpad[i] = myOneWire_read(&var->wire_t);
		}

		b = myOneWire_reset(&var->wire_t);
		return (b == 1);
}

void ds18b20_writeScratchpad(ds18b20_t* var) {
	ds18b20_sendCommand(var, MATCH_ROM, WRITE_SCRATCHPAD, 0);
	myOneWire_write(&var->wire_t, var->selectedScratchpad[ALARM_HIGH], 0);
	myOneWire_write(&var->wire_t, var->selectedScratchpad[ALARM_LOW], 0);
	myOneWire_write(&var->wire_t, var->selectedScratchpad[CONFIGURATION], 0);
	ds18b20_sendCommand(var,MATCH_ROM, COPY_SCRATCHPAD, !var->selectedPowerMode);

    if (!var->selectedPowerMode) {
        HAL_Delay(10);
    }
}

uint8_t ds18b20_sendCommand_(ds18b20_t* var, uint8_t romCommand) {
	myOneWire_reset(&var->wire_t);
//    if (!myOneWire_reset(&var->wire_t)||1) {
//        return 0;
//    }

    switch (romCommand) {
        case SEARCH_ROM:
        case SKIP_ROM:
        case ALARM_SEARCH:
        	myOneWire_write(&var->wire_t, romCommand, 0);
            break;
        case MATCH_ROM:
        	myOneWire_select(&var->wire_t, var->selectedAddress);
            break;
        default:
            return 0;
    }

    return 1;
}

uint8_t ds18b20_sendCommand(ds18b20_t* var, uint8_t romCommand, uint8_t functionCommand, uint8_t power) {
    if (!ds18b20_sendCommand_(var,romCommand)) {
        return 0;
    }

    switch (functionCommand) {
        case CONVERT_T:
        case COPY_SCRATCHPAD:
        	myOneWire_write(&var->wire_t, functionCommand, power);
            break;
        case WRITE_SCRATCHPAD:
        case READ_SCRATCHPAD:
        case READ_POWER_SUPPLY:
        	myOneWire_write(&var->wire_t, functionCommand, 0);
            break;
        default:
            return 0;
    }

    return 1;
}

bool ds18b20_oneWireSearch(ds18b20_t* var, uint8_t romCommand) {
    if ( var->lastDevice ){//|| !ds18b20_sendCommand_(var,romCommand)  ) {
    	ds18b20_resetSearch(var);
    	return false;
    }
    ds18b20_sendCommand_(var,romCommand);
    uint8_t lastZero = 0;
    uint8_t direction, byteNumber, bitNumber, currentBit, currentBitComp;

    for (uint8_t bitPosition = 0; bitPosition < 64; bitPosition++) {
        currentBit = myOneWire_read_bit(&var->wire_t);
        currentBitComp = myOneWire_read_bit(&var->wire_t);

        if (currentBit && currentBitComp) {
            var->lastDiscrepancy = 0;
            return false;
        }

        byteNumber = bitPosition / 8;
        bitNumber = bitPosition % 8;

        if (!currentBit && !currentBitComp) {
            if (bitPosition == var->lastDiscrepancy) {
                direction = 1;
            } else if (bitPosition > var->lastDiscrepancy) {
                direction = 0;
                lastZero = bitPosition;
            } else {
            	direction = (var->searchAddress[byteNumber] >> bitNumber) & 0x01;
                if (!direction) {
                    lastZero = bitPosition;
                }
            }
        } else {
            direction = currentBit;
        }

        if( direction )
			var->searchAddress[8] |= (1UL << bitNumber);
		else
			var->searchAddress[8] &= ~(1UL << bitNumber);
        myOneWire_write_bit(&var->wire_t, direction);
    }

    var->lastDiscrepancy = lastZero;

    if (!var->lastDiscrepancy) {
    	var->lastDevice = 1;
    }

    return true;
}

bool ds18b20_searchAddress(ds18b20_t* var, uint8_t* deviceAddress, uint8_t index_) {

	uint8_t depth = 0;
	myOneWire_reset(&var->wire_t);
	while (depth <= index_ && myOneWire_search(&var->wire_t, deviceAddress, true)) {
		if (depth == index_ && ds18b20_validAddress(deviceAddress))
			return true;
		depth++;
	}

	return false;

}


bool ds18b20_isConnected_(ds18b20_t* var, const uint8_t* deviceAddress, uint8_t* scratchPad) {
	bool b = ds18b20_readScratchpad_(var, deviceAddress, scratchPad);
	return b && !ds18b20_isAllZeros(scratchPad,9) && (myOneWire_crc8(scratchPad, 8) == scratchPad[SCRATCHPAD_CRC]);
}

uint8_t ds18b20_isConnected(ds18b20_t* var, uint8_t address[]) {

    if (!ds18b20_sendCommand_(var, SEARCH_ROM)) {
        return 0;
    }
    uint8_t currentBit, currentBitComp, byteNumber, bitNumber;
    for (uint8_t bitPosition = 0; bitPosition < 64; bitPosition++) {
        currentBit = myOneWire_read_bit(&var->wire_t);
        currentBitComp = myOneWire_read_bit(&var->wire_t);

        if (currentBit && currentBitComp) {
            return 0;
        }

        byteNumber = bitPosition / 8;
        bitNumber = bitPosition % 8;
        uint8_t num = (address[byteNumber] >> bitNumber) & 0x01;
        myOneWire_write_bit(&var->wire_t, num);
    }
    return 1;
}

void ds18b20_delayForConversion(ds18b20_t* var, uint8_t resolution, uint8_t powerMode) {
    if (powerMode) {
        while (!myOneWire_read_bit(&var->wire_t));
    } else {
        switch (resolution) {
            case 9:
                HAL_Delay(CONV_TIME_9_BIT);
                break;
            case 10:
            	HAL_Delay(CONV_TIME_10_BIT);
                break;
            case 11:
            	HAL_Delay(CONV_TIME_11_BIT);
                break;
            case 12:
            	HAL_Delay(CONV_TIME_12_BIT);
                break;
        }
    }
}


void A9_wait_for_1(ds18b20_t* var, uint32_t time)
{
	ONEWIRE_INPUT(&var->wire_t);
	DWT_Delay_us(time);
	while(HAL_GPIO_ReadPin(var->wire_t.GPIOx, var->wire_t.GPIO_Pin) == 0);
	ONEWIRE_OUTPUT(&var->wire_t);
}

float myDS_getTempC(ds18b20_t* var)
{
	uint8_t pad_data[] = {0,0,0,0,0,0,0,0,0};//9 Byte
	myOneWire_reset(&var->wire_t);
	myOneWire_write(&var->wire_t, 0xCC, 0);
	myOneWire_write(&var->wire_t, 0x44, 0);//Convert Temperature [44h]
	A9_wait_for_1(var,20);
	myOneWire_reset(&var->wire_t);
	myOneWire_write(&var->wire_t, 0xCC, 0);//Skip ROM [CCh]
	myOneWire_write(&var->wire_t, 0xBE, 0);//Read Scratchpad [BEh]
	for (uint8_t i = 0; i < 9; i++)
	pad_data[i] = myOneWire_read(&var->wire_t);//factor out 1/16 and remember 1/16 != 1/16.0
	uint16_t x = (pad_data[1] << 8) + pad_data[0];
	if ((pad_data[1] >> 7) == 1 ){
	x -= 1; x = ~x;
	return x / -16.0;
	} else return x / 16.0;
}
