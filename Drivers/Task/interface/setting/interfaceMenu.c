/*
 * interfaceSetting.c
 *
 *  Created on: Mar 8, 2022
 *      Author: andhie
 */


#include <interfaceMenu.h>
#include "interfaceAbout.h"
#include "interfaceParameter.h"
#include "interfaceTimeDate.h"
#include "interface.h"
#include "buttonThread.h"
#include "menuDisplay.h"
#include "menuDisplay.h"
#include "system.h"

#define total_menu 5
menuDisplay_var mainMenu_;
const char *MenuLang[total_menu] =
		{
			"Parameters",
			"Time & Date",
			"Info",
			"Reset Log",
			"Exit",
		};

extern mainInterface_var interfaceVar;
uint8_t interfaceMenu_position = 255;
void interfaceMenu_onPrevious()
{
	menuDisplay.previous(&mainMenu_);
}

void interfaceMenu_onNext()
{
	menuDisplay.next(&mainMenu_);
}

void interfaceMenu_onOK()
{
	interfaceMenu_position = menuDisplay.getPosition(&mainMenu_);
}



void interfaceMenu_initButton()
{
	btnTask.attachPressed(BTN_UP, 	interfaceMenu_onPrevious);
	btnTask.attachPressed(BTN_DOWN,	interfaceMenu_onNext );
	btnTask.attachPressed(BTN_OK,	interfaceMenu_onOK );
	interfaceMenu_position = 255;
}

void interfaceMenu_init()
{
	menuDisplay.init(&mainMenu_);
	mainMenu_.setTitle = 0;
	menuDisplay.setMenu(&mainMenu_, MenuLang, total_menu);
	menuDisplay.attachDateTime(&mainMenu_,interfaceVar.date, interfaceVar.time);
	menuDisplay.setCursor(&mainMenu_, ">");
	interfaceMenu_initButton();
}

void interfaceMenu_deInit()
{
	btnTask.clearISR(BTN_SETTING);
	btnTask.dettachPressed(BTN_OK);
	btnTask.clearISR(BTN_UP);
	btnTask.clearISR(BTN_DOWN);
}

void interfaceMenu_loop()
{
	interfaceMenu_position = 255;
	uint8_t quitLoop = 0;
	interfaceMenu_init();
	while(1)
	{
		if( interfaceMenu_position != 255 )
		{
			interfaceMenu_deInit();
			switch(interfaceMenu_position)
			{
				case 0 : // parameters
						interfaceParameter_loop();
						break;
				case 1 : // time & dates
						interfaceTimeDate_loop();
						break;
				case 2 : // information
						interfaceAbout_loop();
						break;
				case 3 : // reset log
					    system_t.resetLog();
					    quitLoop = 1;
						break;
				case 4 : // exit
					quitLoop = 1;
					break;
			}
			interfaceMenu_initButton();
			interfaceMenu_position = 255;
		}
		if( quitLoop )
			break;
		if( interfaceVar.interface_onNotifState == 0 )
			menuDisplay.draw(&mainMenu_);
		else
			interface_drawNotification();
		osDelay(100);
	}
	interfaceMenu_deInit();
}
