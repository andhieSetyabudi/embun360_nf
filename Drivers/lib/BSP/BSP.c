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


