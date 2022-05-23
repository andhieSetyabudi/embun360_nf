/*
 * interfaceTimeDate.c
 *
 *  Created on: Mar 9, 2022
 *      Author: andhie
 */

#include "interface.h"
#include "menuDisplay.h"
#include "buttonThread.h"
#include "stm32_rtc.h"
#include "string.h"

#define total_timedate_menu	3
menuDisplay_var timeDateMenu;
const char *timeDateList[total_timedate_menu];
static char timeDateListTmp[total_timedate_menu][30];

static RTC_DateTypeDef date_M;
static RTC_TimeTypeDef time_M;

static uint8_t interfaceTimeDate_position = 255;

static uint8_t interfaceTimeDate_cursor[2] = {0,0};

static void interfaceTimeDate_updateList()
{
	for(uint8_t i = 0; i<total_timedate_menu; i++)
		timeDateList[i]=timeDateListTmp[i];
}


static void interfaceTimeDate_onMenuNext()
{
	menuDisplay.next(&timeDateMenu);
}

static void interfaceTimeDate_onMenuPrevious()
{
	menuDisplay.previous(&timeDateMenu);
}

static void interfaceTimeDate_onMenuOK()
{
	interfaceTimeDate_position = menuDisplay.getPosition(&timeDateMenu);
}

static void interfaceTimeDate_onNextCursor()
{
	interfaceTimeDate_cursor[1]++;
}

static void interfaceTimeDate_onUp()
{
	if( interfaceTimeDate_cursor[0] == 0 ) {		// time
		switch ( interfaceTimeDate_cursor[1])
		{
			case 0 : // hour
				time_M.Hours++;
				if(time_M.Hours>=24)
					time_M.Hours =0;
				break;
			case 1 : // Minutes
				time_M.Minutes++;
				if(time_M.Minutes>=60)
					time_M.Minutes = 0;
				break;
		};

	}else{	// date
		uint8_t max_day = 0;
		switch ( interfaceTimeDate_cursor[1])
		{
			case 0 : // years
				date_M.Year++;
				if(date_M.Year>=99)
					date_M.Year =0;
				break;
			case 1 : // Minutes
				date_M.Month++;
				if(date_M.Month > 12)
					date_M.Month = RTC_MONTH_JANUARY;
				break;
			case 2 : // Day
				max_day = getMaxDayOf(date_M.Month, date_M.Year);
				date_M.Date++;
				if(date_M.Date > max_day)
					date_M.Date = 1;
				break;
		};
	}
}

static void interfaceTimeDate_onDown()
{
	if( interfaceTimeDate_cursor[0] == 0 ) {		// time
		switch ( interfaceTimeDate_cursor[1])
		{
			case 0 : // hour
				time_M.Hours--;
				if(time_M.Hours>= 24)
					time_M.Hours= 23;
				break;
			case 1 : // Minutes
				time_M.Minutes--;
				if(time_M.Minutes>=60)
					time_M.Minutes = 59;
				break;
		};

	}else{	// date
		uint8_t max_day = 0;
		switch ( interfaceTimeDate_cursor[1])
		{
			case 0 : // years
				date_M.Year--;
				if(date_M.Year >= 99)
					date_M.Year = 99;
				break;
			case 1 : // Minutes
				date_M.Month--;
				if(date_M.Month>=12)
					date_M.Month = RTC_MONTH_DECEMBER;
				break;
			case 2 : // Day
				max_day = getMaxDayOf(date_M.Month, date_M.Year);
				date_M.Date--;
				if(date_M.Date > max_day)
					date_M.Date = max_day;
				break;
		};
	}
}

static void interfaceTimeDate_init()
{
	menuDisplay.init(&timeDateMenu);
	menuDisplay.setMenu(&timeDateMenu, timeDateList, total_timedate_menu);
	menuDisplay.attachDateTime(&timeDateMenu,interfaceVar.date, interfaceVar.time);
	menuDisplay.setTitle(&timeDateMenu, "Time & Date");
	menuDisplay.setCursor(&timeDateMenu, ">");
//	interfaceParameter_quitSetup = 0;
//	interfaceParameter_buttonInit();
	interfaceTimeDate_position = 255;
}

static void interfaceTimeDate_initButton()
{
	btnTask.attachPressed(BTN_UP,	interfaceTimeDate_onMenuPrevious );
	btnTask.attachPressed(BTN_DOWN,	interfaceTimeDate_onMenuNext );
	btnTask.attachPressed(BTN_OK,	interfaceTimeDate_onMenuOK );
}

static void interfaceTimeDate_deInitButton()
{
	btnTask.clearISR(BTN_UP);
	btnTask.clearISR(BTN_DOWN);
	btnTask.dettachPressed(BTN_OK);
	btnTask.clearISR(BTN_SETTING);
}

static void interfaceTimeDate_blinkingLine(uint8_t loc, uint8_t td)
{
	switch(loc)
	{
		case 0: // time
			switch ( td )
			{
				case 0 : // hours
					lcd.drawHLine(48,35,13,1,1);
					break;
				case 1 : // minutes
					lcd.drawHLine(68,35,13,1,1);
					break;
			};
			break;
		case 1: // date
			switch ( td )
			{
				case 0 : // years
					lcd.drawHLine(48,48,25,1,1);
					break;
				case 1 : // month
					lcd.drawHLine(83,48,13,1,1);
					break;
				case 2 : // date
					lcd.drawHLine(106,48,13,1,1);
					break;
			};

			break;
	};
	lcd.sendBuffer(true);
	interfaceTimeDate_updateList();
}


void interfaceTimeDate_loop()
{
	date_M = *interfaceVar.date;
	time_M = *interfaceVar.time;
	sprintf(timeDateListTmp[0], (const char*)"Time  :  %02d : %02d", time_M.Hours, time_M.Minutes);
	sprintf(timeDateListTmp[1], (const char*)"Date :  %02d - %02d - %02d", date_M.Year+2000, date_M.Month, date_M.Date);
	sprintf(timeDateListTmp[2], (const char*)"Back");
	interfaceTimeDate_init();
	interfaceTimeDate_updateList();
	interfaceTimeDate_initButton();
	memset(interfaceTimeDate_cursor,0,2);
	while(1)
	{
		if( interfaceTimeDate_position != 255 )
		{
			interfaceTimeDate_deInitButton();
			if( interfaceTimeDate_position == 2 )
				break;
			else
			{
				interfaceTimeDate_cursor[0] = interfaceTimeDate_position;
				interfaceTimeDate_cursor[1] = 0;
				btnTask.attachPressed(BTN_OK,	interfaceTimeDate_onNextCursor);
				btnTask.attachPressed(BTN_DOWN,interfaceTimeDate_onDown );
				btnTask.attachPressed(BTN_UP,interfaceTimeDate_onUp );

				while(1)
				{
					if( ( ( interfaceTimeDate_cursor[0] == 0 && interfaceTimeDate_cursor[1] >= 2 ) ) ||
						( ( interfaceTimeDate_cursor[0] == 1 && interfaceTimeDate_cursor[1] >= 3 ) ) )
						break;
					sprintf(timeDateListTmp[0], (const char*)"Time  :  %02d : %02d", time_M.Hours, time_M.Minutes);
					sprintf(timeDateListTmp[1], (const char*)"Date :  %02d - %02d - %02d", date_M.Year+2000, date_M.Month, date_M.Date);
					if( interfaceVar.interface_onNotifState == 0 )
					{
						menuDisplay.draw(&timeDateMenu);
						interfaceTimeDate_blinkingLine(interfaceTimeDate_cursor[0], interfaceTimeDate_cursor[1]);
					}
					else
						interface_drawNotification();
					osDelay(100);
				}
				btnTask.clearISR(BTN_UP);
				btnTask.clearISR(BTN_DOWN);
				btnTask.clearISR(BTN_OK);
				rtc_setDateTime(&date_M, &time_M);
			}
			interfaceTimeDate_initButton();
			interfaceTimeDate_position = 255;
		}
		if( interfaceVar.interface_onNotifState == 0 )
			menuDisplay.draw(&timeDateMenu);
		else
			interface_drawNotification();
		osDelay(100);
	}
	interfaceTimeDate_deInitButton();
	interfaceTimeDate_position = 255;
}

