/*
 * mainDisplay.h
 *
 *  Created on: Mar 4, 2022
 *      Author: andhie
 */

#ifndef LIB_DISPLAY_MAINDISPLAY_H_
#define LIB_DISPLAY_MAINDISPLAY_H_
#include "stdbool.h"

typedef struct mainDisplay_s
{
	uint8_t* warningIco;
	char*	 warningMsg;

	// date-time for header
	RTC_DateTypeDef *date;
	RTC_TimeTypeDef *time;

	float *rh, *airTemp,
		  *finTemp1, *finTemp2,
		  *tankLevel;

	char* table[4][2];
	bool tableChanged[4][2];
	void (*halt)(uint32_t timeout);
}mainDisplay_var;


#endif /* LIB_DISPLAY_MAINDISPLAY_H_ */
