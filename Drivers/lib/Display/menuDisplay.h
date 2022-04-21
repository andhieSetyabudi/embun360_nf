/*
 * menuDisplay.h
 *
 *  Created on: Mar 4, 2022
 *      Author: andhie
 */

#ifndef LIB_DISPLAY_MENUDISPLAY_H_
#define LIB_DISPLAY_MENUDISPLAY_H_

#include "stdio.h"
#include "stdbool.h"
#include "stm32f2xx_hal_rtc.h"
#include "MCP23017_LCD12864.h"


#ifndef MENU_MAXCHAR_PERLINE
	#define MENU_MAXCHAR_PERLINE 22
#endif


typedef struct menuDisplay_t {
	uint8_t totalMenu;
	uint8_t position;

	const char **menu;
	unsigned char setTitle;
	const char *cursor;
	char title[MENU_MAXCHAR_PERLINE];
	// date-time for header
	RTC_DateTypeDef *date;
	RTC_TimeTypeDef *time;
} menuDisplay_var;


void menuDisplay_create(menuDisplay_var* var);
void menuDisplay_next(menuDisplay_var *var);
void menuDisplay_previous(menuDisplay_var *gadgetMenu);
void menuDisplay_resetPosition(menuDisplay_var *gadgetMenu);

void menuDisplay_setMenu(menuDisplay_var *var, const char **menu, unsigned char totalMenu);
void menuDisplay_attachDateTime(menuDisplay_var *var,RTC_DateTypeDef *date_, RTC_TimeTypeDef *time_);

uint8_t menuDisplay_getPosition(menuDisplay_var *var);
void menuDisplay_draw(menuDisplay_var *var);
void menuDisplay_setTitle(menuDisplay_var *var, const char *title);

typedef struct __attribute__ ((__packed__)) menuDisplay_tf
{
	void (*init)			(menuDisplay_var* var);
	void (*next)			(menuDisplay_var *var);
	void (*previous)		(menuDisplay_var *gadgetMenu);
	void (*resetPosition)	(menuDisplay_var *gadgetMenu);

	void (*setMenu)			(menuDisplay_var *var, const char **menu, unsigned char totalMenu);
	void (*attachDateTime)	(menuDisplay_var *var,RTC_DateTypeDef *date_, RTC_TimeTypeDef *time_);

	uint8_t (*getPosition)	(menuDisplay_var *var);
	void (*draw)			(menuDisplay_var *var);
	void (*setTitle)		(menuDisplay_var *var, const char *title);
	void (*setCursor)		(menuDisplay_var *var, const char *cursor);
}menuDisplay_f;


extern menuDisplay_f menuDisplay;
#endif /* LIB_DISPLAY_MENUDISPLAY_H_ */
