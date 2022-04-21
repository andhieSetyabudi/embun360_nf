/*
 * interfaceAbout.c
 *
 *  Created on: Mar 8, 2022
 *      Author: andhie
 */


#include "interface.h"
#include "buttonThread.h"
#include "interfaceAbout.h"
#include "menuDisplay.h"
//
#define total_about 4
menuDisplay_var aboutMenu_;
const char* aboutLang[total_about];
char UUID_Device[18];

static uint8_t interfaceAbout_onExit = 0;
void interfaceAbout_onPressed()
{
	interfaceAbout_onExit = 1;
}

void interfaceAbout_onNext()
{
	aboutMenu_.position = aboutMenu_.position + 3;
	aboutMenu_.position = aboutMenu_.position > total_about ? 0 : aboutMenu_.position;
}

void interfaceAbout_onPrevious()
{
	aboutMenu_.position = aboutMenu_.position - 3;
	aboutMenu_.position = aboutMenu_.position > total_about ? total_about-1 : aboutMenu_.position;
}

void interfaceAbout_init()
{
	menuDisplay.init(&aboutMenu_);
	menuDisplay.setMenu(&aboutMenu_, (const char **)aboutLang, total_about);
	menuDisplay.attachDateTime(&aboutMenu_,interfaceVar.date, interfaceVar.time);
	menuDisplay.setTitle(&aboutMenu_,"Info");
	btnTask.attachPressed(BTN_OK,		interfaceAbout_onPressed );
	btnTask.attachPressed(BTN_SETTING,	interfaceAbout_onPressed );
	btnTask.attachPressed(BTN_UP, 		interfaceAbout_onPrevious);
	btnTask.attachPressed(BTN_DOWN,	 	interfaceAbout_onNext);
	interfaceAbout_onExit = 0;
}


void interfaceAbout_deInit()
{
	btnTask.clearISR(BTN_SETTING);
	btnTask.dettachPressed(BTN_OK);
	btnTask.clearISR(BTN_UP);
	btnTask.clearISR(BTN_DOWN);
	interfaceAbout_onExit = 0;
}

void interfaceAbout_loop()
{

	char aboutList[total_about][24];
	sprintf(aboutList[0], (const char*)"Model : %s", MODEL);
	sprintf(aboutList[1], (const char*)"Firm   : %s", VERSION);
	sprintf(aboutList[2], (const char*)"SN      : %s", getUUID(UUID_Device));
	sprintf(aboutList[3], (const char*)"WIDYA IMERSIF %s", YEAR_MADE);
	for( uint8_t i = 0; i<total_about; i++)
		aboutLang[i]=aboutList[i];
	interfaceAbout_init();
	while(1)
	{
		if( interfaceVar.interface_onNotifState == 0 )
			menuDisplay.draw(&aboutMenu_);
		else
			interface_drawNotification();
		osDelay(100);
		if( interfaceAbout_onExit != 0 )
			break;
	};
	interfaceAbout_deInit();

}
