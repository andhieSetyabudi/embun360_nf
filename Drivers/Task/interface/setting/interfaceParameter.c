/*
 * interfaceParameter.c
 *
 *  Created on: Mar 9, 2022
 *      Author: andhie
 */


#include "interfaceParameter.h"
#include "interface.h"
#include "menuDisplay.h"
#include "buttonThread.h"
#include "string.h"
#include "system.h"
#include "sensor.h"


#define total_menu_parameter 5
static menuDisplay_var parameterMenu;
static char paramListTmp[total_menu_parameter][30];
const char *paramList[total_menu_parameter];

extern mainInterface_var interfaceVar;
static uint8_t interfaceParameter_position = 255;
static uint8_t interfaceParameter_quitSetup = 0;
static void interfaceParameter_updateList();
void interfaceParameter_onPrevious()
{
	switch(interfaceParameter_position)
	{
		case 0 : // minimum fin temperature
			*interfaceVar.minTemperature+=0.05;
			normalizeMinimumTemperature(interfaceVar.minTemperature);
			break;
		case 1: // minimum humidity
			*interfaceVar.minHumidity +=0.5;
			normalizeMinimumHumidity(interfaceVar.minHumidity);
			break;
		case 255:
				menuDisplay.previous(&parameterMenu);
				break;
	}
}

void interfaceParameter_onNext()
{
	float tmp = 0;
	switch(interfaceParameter_position)
	{
		case 0 : // minimum fin temperature
				tmp = *interfaceVar.minTemperature-0.05;
				normalizeMinimumTemperature(&tmp);
				*interfaceVar.minTemperature=tmp;
				break;
		case 1: // minimum humidity
				tmp =*interfaceVar.minHumidity -0.5;
				normalizeMinimumHumidity(&tmp);
				*interfaceVar.minHumidity = tmp;
				break;
		case 255:
			menuDisplay.next(&parameterMenu);
			break;
	}

}

void interfaceParameter_onOK()
{
	interfaceParameter_position = menuDisplay.getPosition(&parameterMenu);
}

void interfaceParameter_onAccept()
{
	interfaceParameter_quitSetup = 1;
}

void interfaceParameter_buttonInit()
{
	btnTask.attachPressed(BTN_UP, 	interfaceParameter_onPrevious);
	btnTask.attachPressed(BTN_DOWN,	interfaceParameter_onNext );
	btnTask.attachPressed(BTN_OK,	interfaceParameter_onOK );
	interfaceParameter_position = 255;
	interfaceParameter_updateList();
}

void interfaceParameter_init()
{
	menuDisplay.init(&parameterMenu);
	menuDisplay.setMenu(&parameterMenu, paramList, total_menu_parameter);
	menuDisplay.attachDateTime(&parameterMenu,interfaceVar.date, interfaceVar.time);
	menuDisplay.setTitle(&parameterMenu, "Parameter");
	menuDisplay.setCursor(&parameterMenu, ">");
	interfaceParameter_quitSetup = 0;
	interfaceParameter_buttonInit();
}


void interfaceParameter_deInit()
{
	btnTask.dettachPressed(BTN_OK);
}

static void interfaceParameter_updateList()
{
	for(uint8_t i = 0; i<total_menu_parameter; i++)
		paramList[i]=paramListTmp[i];
}

static void interfaceParameter_blinkingValue(uint8_t loc)
{
	static uint8_t blinked = 0;
	static uint32_t timeBlinking = 0;
	if( osKernelGetTickCount() - timeBlinking >= 500UL )
	{
		char bufVal[10];
		switch(loc)
		{
			case 0: // minimum temperature
				if(blinked)
					sprintf(paramListTmp[0], (const char*) "Min. Fin Temp : %4s",ftoa(*interfaceVar.minTemperature, bufVal,2));
				else
					sprintf(paramListTmp[0], (const char*) "Min. Fin Temp : ");
				break;
			case 1: // minimum humidity
				if(blinked)
					sprintf(paramListTmp[1], (const char*) "Min. Humidity : %4s",ftoa(*interfaceVar.minHumidity, bufVal,1));
				else
					sprintf(paramListTmp[1], (const char*) "Min. Humidity : ");
				break;
		}
		interfaceParameter_updateList();
		blinked = ~blinked;
		timeBlinking = osKernelGetTickCount();
	}
}

void interfaceParameter_loop()
{
	char bufVal[12];
	sprintf(paramListTmp[0], (const char*) "Min. Fin Temp : %.4s",ftoa(*interfaceVar.minTemperature, bufVal,2));
	sprintf(paramListTmp[1], (const char*) "Min. Humidity : %.4s",ftoa(*interfaceVar.minHumidity, bufVal,1));
	sprintf(paramListTmp[2], (const char*) "Tank mode : %s", system_t.getTankModeStr(*interfaceVar.tankMode,bufVal));
	sprintf(paramListTmp[3], (const char*) "Tank height : %.4s", ftoa(sysVar.memory.waterTankHeight, bufVal,2));
	sprintf(paramListTmp[4], (const char*) "Back");
	interfaceParameter_init();
	while(1)
	{
		if( interfaceParameter_position != 255 ){
			interfaceParameter_deInit();
			if( interfaceParameter_position == 0 )	// minimum temperature
			{
				btnTask.attachPressed(BTN_OK,	interfaceParameter_onAccept );
				while(1)
				{
					interfaceParameter_blinkingValue(0);
					if( interfaceParameter_quitSetup != 0)
						break;
					if( interfaceVar.interface_onNotifState == 0 )
						menuDisplay.draw(&parameterMenu);
					else
						interface_drawNotification();
					osDelay(100);
				};
				interfaceParameter_quitSetup = 0;
				sprintf(paramListTmp[0], (const char*) "Min. Fin Temp : %04s",ftoa(*interfaceVar.minTemperature, bufVal,2));
			}
			else if ( interfaceParameter_position == 1) // minimum Humidity
			{
				btnTask.attachPressed(BTN_OK,	interfaceParameter_onAccept );
				while(1)
				{
					interfaceParameter_blinkingValue(1);
					if( interfaceParameter_quitSetup != 0)
						break;
					if( interfaceVar.interface_onNotifState == 0 )
						menuDisplay.draw(&parameterMenu);
					else
						interface_drawNotification();
					osDelay(100);
				};
				interfaceParameter_quitSetup = 0;
				sprintf(paramListTmp[1], (const char*) "Min. Humidity : %.4s",ftoa(*interfaceVar.minHumidity, bufVal,1));
			}
			else if( interfaceParameter_position == 2 )	// change tank sensor mode
			{
				*interfaceVar.tankMode+=1;
				if(*interfaceVar.tankMode>1)
					*interfaceVar.tankMode = 0;
//				*interfaceVar.tankMode = *interfaceVar.tankMode%2;
				sprintf(paramListTmp[2], (const char*) "Tank mode : %8s", system_t.getTankModeStr(*interfaceVar.tankMode,bufVal));
			}
			else if ( interfaceParameter_position == 3 )
			{
				sysVar.memory.waterTankHeight = *interfaceVar.tankUS;
				sensor.setTankHeight_const(sysVar.memory.waterTankHeight);
				sprintf(paramListTmp[3], (const char*) "Tank height : %.4s", ftoa(sysVar.memory.waterTankHeight, bufVal,2));
			}

			if( interfaceParameter_position == total_menu_parameter-1 )
				break;
			interfaceParameter_position = 255;
			interfaceParameter_buttonInit();
		}
		if( interfaceVar.interface_onNotifState == 0 )
			menuDisplay.draw(&parameterMenu);
		else
			interface_drawNotification();
		osDelay(100);
	};
	interfaceParameter_position = 255;
	interfaceParameter_deInit();
	btnTask.clearISR(BTN_UP);
	btnTask.clearISR(BTN_DOWN);
}

