/*
 * interface.c
 *
 *  Created on: Mar 7, 2022
 *      Author: andhie
 */
#include "string.h"
#include "MCP23017_LCD12864.h"
#include "interface.h"
#include <sensor.h>
#include <system.h>
#include <buttonThread.h>
#include <interfaceMenu.h>
#include "splashScreen.h"

#define getValid(var)		(( var != NULL ) ? var : 0.f )

static char buff[24];
char strBuf[14];
static char val1[10], val2[10];

static uint8_t interface_onInit = 0;

extern I2C_HandleTypeDef hi2c1;
extern osThreadId_t taskInterfaceHandle;
mainInterface_var interfaceVar= {0};

void interfaceDelay (uint32_t time_)
{
	osDelay(time_);
}

void interface_previousPage();
void interface_nextPage();
void interface_onShutdown();
void interface_onMenu();
// linker each pointer ( variable )

void interface_linkingTimeDate(RTC_DateTypeDef *date, RTC_TimeTypeDef *time)
{
	interfaceVar.date = date;
	interfaceVar.time = time;
}

void interface_linkingMeasurement(float *airTemp, 	float* airRH, 		  float *finTemp1,   float *finTemp2,
								  float *waterFlow, float *currentVolume, float *totalVolume, float *tankPercent,
								  float *tankUS)
{
	interfaceVar.airTemp  		= airTemp;
	interfaceVar.airRH	  		= airRH;
	interfaceVar.finTemp1 		= finTemp1;
	interfaceVar.finTemp2 		= finTemp2;
	interfaceVar.waterFlow		= waterFlow;
	interfaceVar.currentVolume	= currentVolume;
	interfaceVar.totalVolume 	= totalVolume;
	interfaceVar.tankInPercent 	= tankPercent;
	interfaceVar.tankUS			= tankUS;
}

void interface_linkingStatusFlag( uint8_t *tankMode, uint8_t *tankFillingStatus, uint8_t *systemStatus )
{
	interfaceVar.tankMode 			= tankMode;
	interfaceVar.tankFillingStatus 	= tankFillingStatus;
	interfaceVar.systemStatus 		= systemStatus;
}

void interface_linkingSystemSet	(float *minTemperature, float *minHumidity )
{
	interfaceVar.minTemperature = minTemperature;
	interfaceVar.minHumidity = minHumidity;
}

uint8_t interface_isReady(void)
{
	return interfaceVar.ready;
}

void interface_initButton()
{
	interfaceVar.onExit		= 0;
	btnTask.attachPressed(BTN_UP, interface_nextPage);
	btnTask.attachPressed(BTN_DOWN, interface_previousPage);
	btnTask.attachPressed(BTN_SETTING, interface_onMenu);
//	btnTask.attachLongPressed(BTN_OK, system_t.onRun);
}

void interface_deInitButton(void)
{
	btnTask.clearISR(BTN_SETTING);
	btnTask.dettachPressed(BTN_OK);
	btnTask.clearISR(BTN_UP);
	btnTask.clearISR(BTN_DOWN);
	interfaceVar.onExit		= 0;
}

void interface_init(void)
{
	interfaceVar.ready 		= 0x00;
	interfaceVar.pageCursor = 0;
	interfaceVar.onExit		= 0;
	lcd.init(&hi2c1, 0x20);
	lcd.clear(1, 1);
	splashScreen.attachDelay(interfaceDelay);
	splashScreen.init(splashScreenICO, splashWidth, splashHeight, Font5x7);
	splashScreen.display (CENTER_, "Initialize", VERSION, 3500);
	interfaceVar.ready = 0x01;
	interface_onInit = 1;
	interfaceVar.shutingDown_state = 0;
}

void interface_nextPage()
{
	interfaceVar.pageCursor++;
	interfaceVar.pageCursor = interfaceVar.pageCursor < totalMainPage ? interfaceVar.pageCursor : 0;
}

void interface_previousPage()
{
	interfaceVar.pageCursor += totalMainPage;
	interfaceVar.pageCursor--;
	interfaceVar.pageCursor = interfaceVar.pageCursor % totalMainPage;
}

void interface_onShutdown()
{
	osThreadSuspend(taskInterfaceHandle);
	interfaceVar.shutingDown_state = 1;
	vTaskSuspend(taskInterfaceHandle);
	if( interfaceVar.onShutingdown )
		interfaceVar.onShutingdown();
	splashScreen.attachDelay(HAL_Delay);
	lcd.clear(1,0);
	lcd.clearBuffer();
	splashScreen.init(splashScreenICO, splashWidth, splashHeight, Font5x7);
	splashScreen.display (CENTER_, "Shutting down", " ", 1000);
	splashScreen.display (CENTER_, "Shutting down", " ", 5500);
	HAL_NVIC_SystemReset();
}

void interface_onMenu()
{
	interfaceVar.onExit	= 1;
}

static uint32_t mainIfaceBlinking = 0;
static uint8_t mainIfaceBlinked = 0;
void interface_drawHeader(void)
{
	lcd.setFont(Font5x7);
	if( interfaceVar.date == NULL)
		sprintf(buff,"1-1-2000");
	else
		sprintf(buff,"%02d-%02d-%02d", interfaceVar.date->Date, interfaceVar.date->Month, interfaceVar.date->Year+2000);
	lcd.text(1,1, buff);
	if( interfaceVar.time == NULL )
		sprintf(buff,"00%c00",mainIfaceBlinked!=0?':':' ');
	else
		sprintf(buff,"%02d%c%02d", interfaceVar.time->Hours, mainIfaceBlinked!=0?':':' ', interfaceVar.time->Minutes);
	uint8_t pos_x = 128 - lcd.getWitdthStr((const uint8_t*)buff, strlen(buff));
	lcd.text(pos_x-1,1, buff);
	lcd.drawHLine(0,10,128,1,1);
	if( osKernelGetTickCount() - mainIfaceBlinking >= 500UL )
	{
		mainIfaceBlinked = ~mainIfaceBlinked;
		mainIfaceBlinking = osKernelGetTickCount();
	}
}

/*
 * Will displaying result and status
 *  - System status : running or stopped
 *  - Tank status :
 *  				a. on ultasonic sensor mode : tank level in percent
 *  				b. on floating sensor : empty, filling or full
 *  - current volume
 *  - total volume ( month )
 */
void interface_drawPage1(void)
{

	lcd.setFont(Font5x8);
	float tmp, tmp2;
	uint8_t tmp_u;
//	// system status
//	if( interfaceVar.systemStatus )
//		tmp_u = *interfaceVar.systemStatus;
//	else
//		tmp_u = 0;
	system_t.getStatus(strBuf);
	sprintf(buff,(const char*) " Status : %8s ",strBuf);
	lcd.text(1,15, buff);
	// tank status
	if( interfaceVar.tankMode )
		tmp_u = *interfaceVar.tankMode;
	else
		tmp_u = 0;
	if( tmp_u == floating_sensor )
	{
		if( interfaceVar.tankFillingStatus )
			tmp_u = *interfaceVar.tankFillingStatus;
		else
			tmp_u = 0;
		sprintf(buff, " Tank    : %s ",system_t.getTankStatusStr(tmp_u, strBuf));
		lcd.text(1,28, buff);
	}
	else
	{
		if( interfaceVar.tankInPercent )
			tmp = *interfaceVar.tankInPercent;
		else
			tmp = 0.f;
		tmp = tmp>100.f ? 100.f:tmp;
		sprintf(buff, " Tank    : ");
		lcd.text(1,28, buff);
		lcd.drawBox(41, 28, 54, 8, 1, 1);
		uint8_t fil_ = (uint8_t)(50.f*tmp/100.f);
		lcd.drawFillBox(43,30,fil_ ,4,1);
		sprintf(buff, (const char*) "%4s%%",ftoa(tmp, val1, tmp>10 ? 1 : 2 ));
		uint8_t xpos = 127 - lcd.getWitdthStr((const uint8_t*)buff, strlen(buff));
		lcd.text(xpos,28, buff);
	}

	// current water volume
	if( interfaceVar.currentVolume )
		tmp = *interfaceVar.currentVolume;
	else
		tmp = 0;
	sprintf(buff, "Currently: %7sL",  ftoa(tmp, val1, 3));
	lcd.text(1,41, buff);

	// total volume
	if( interfaceVar.totalVolume )
		tmp = *interfaceVar.totalVolume;
	else
		tmp = 0;
	const char* unit[3] = {"L", "KL", "ML"};
	uint8_t id_ = 0;
	for( id_ = 2; id_ > 0; id_--)
	{
		tmp2 = tmp / pow(1000.f,id_);
		if ( tmp2 >= 10.f )  // if more than
			break;
	}
	if( tmp2 < 0.15f && tmp > 0  && id_ == 0 )
		tmp2 = tmp;
	sprintf(buff, "Total (M) : %6s %s/M",  ftoa(tmp2, val1, id_ > 0 ? 2 : 1),unit[id_]);
	lcd.text(1,54, buff);
}

/*
 * Will displaying sensor parameter
 * - System status : running or stopped
 * - air temperature and humidity
 * - fin temperature 1 and 2
 * - water flow in liter per minutes
 *
 */
void interface_drawPage2(void)
{
	lcd.setFont(Font5x8);
	float tmp, tmp2;
	uint8_t tmp_u;
	// system status
//	if( interfaceVar.systemStatus )
//		tmp_u = *interfaceVar.systemStatus;
//	else
//		tmp_u = 0;
	system_t.getStatus(strBuf);
	sprintf(buff, " Status : %8s ",strBuf);
	lcd.text(1,15, buff);
	// air temperature and humidity
	if( interfaceVar.airRH )
		tmp = *interfaceVar.airRH;
	else
		tmp = 0;
	if( interfaceVar.airTemp )
			tmp2 = *interfaceVar.airTemp;
		else
			tmp2 = 0;
	sprintf(buff,(const char*) " Air       : %4s%cC",  ftoa(tmp2, val1, 2), 0xB0);
	lcd.text(1,28, buff);
	sprintf(buff,(const char*) "%4s%%",  ftoa(tmp, val2, 2));
	uint8_t xpos = 127 - lcd.getWitdthStr((const uint8_t*)buff, strlen(buff));
	lcd.text(xpos,28, buff);
	// fin temperature
	if( interfaceVar.finTemp1 )
		tmp = *interfaceVar.finTemp1;
	else
		tmp = 0;
	sprintf(buff, " Fin       : %4s%cC",  ftoa(tmp, val1, 2), 0xB0);
	lcd.text(1,41, buff);
	if( interfaceVar.finTemp2 )
		tmp = *interfaceVar.finTemp2;
	else
		tmp = 0;
	sprintf(buff, "%4s%cC",  ftoa(tmp, val1, 2), 0xB0);
	xpos = 127 - lcd.getWitdthStr((const uint8_t*)buff, strlen(buff));
	lcd.text(xpos,41, buff);

	// water flow
	if( interfaceVar.waterFlow )
		tmp = *interfaceVar.waterFlow;
	else
		tmp = 0;
	sprintf(buff, " Flow     : %4sLPM",  ftoa(tmp, val1, 2));
	lcd.text(1,54, buff);

}

void interface_showNotification	(const char * message, uint8_t ICO, uint32_t timeout_)
{
	if( interfaceVar.interface_onNotifState != 1 )
	{
		interfaceVar.interface_onNotifState = 1;
		interfaceVar.interface_notifInterrupted = 0;
	}
	interfaceVar.interface_notifMessage = message;
	interfaceVar.interface_notifIco=ICO;
	interfaceVar.notifTimeout = timeout_;
	lcd.drawFillBox(7,3,114,59,0);
	lcd.drawBox(7,3,114,59,1,1);
	unsigned char y_text = 10;
	switch ( interfaceVar.interface_notifIco )
	{
		case ico_warning :
			lcd.drawBitmap(56, 8, 16, 16, warningIcon, 1);
			y_text = 25;
			break;
		case ico_success :
			lcd.drawBitmap(52, 6, 24, 24,successIcon, 1);
			y_text = 31;
			break;
		case ico_failed :
			lcd.drawBitmap(52, 6, 24, 24, failedIcon, 1);
			y_text = 31;
			break;
		default:
			break;
	}
	char *str[4], *pointer;
	char buff[strlen(interfaceVar.interface_notifMessage) + 1];
	strcpy(buff, interfaceVar.interface_notifMessage);
	pointer = buff;
	for (unsigned char i = 0; i < 4 && str[i] != NULL; i++) {
			str[i] = strtok_r(pointer, "\n", &pointer);
		if (str[i] != NULL) {
			uint8_t d = 128 - lcd.getWitdthStr((const uint8_t*)str[i],strlen(str[i]));
			d /= 2;
			lcd.text(d, i * 10 + y_text, str[i]);
		}
	};
	lcd.sendBuffer(1);
	osDelay(100);
}


//static uint32_t notifMillis = 0;
void interface_drawNotification()
{
	if ( interfaceVar.interface_onNotifState == 1)
	{
		lcd.drawFillBox(7,3,114,59,0);
		lcd.drawBox(7,3,114,59,1,1);
		unsigned char y_text = 10;
		switch ( interfaceVar.interface_notifIco )
		{
			case ico_warning :
				lcd.drawBitmap(56, 8, 16, 16, warningIcon, 1);
				y_text = 25;
				break;
			case ico_success :
				lcd.drawBitmap(52, 6, 24, 24,successIcon, 1);
				y_text = 31;
				break;
			case ico_failed :
				lcd.drawBitmap(52, 6, 24, 24, failedIcon, 1);
				y_text = 31;
				break;
			default:
				break;
		}
		char *str[4], *pointer;
		char buff[strlen(interfaceVar.interface_notifMessage) + 1];
		strcpy(buff, interfaceVar.interface_notifMessage);
		pointer = buff;
		for (unsigned char i = 0; i < 4 && str[i] != NULL; i++) {
				str[i] = strtok_r(pointer, "\n", &pointer);
			if (str[i] != NULL) {
				uint8_t d = 128 - lcd.getWitdthStr((const uint8_t*)str[i],strlen(str[i]));
				d /= 2;
				lcd.text(d, i * 10 + y_text, str[i]);
			}
		};
		uint16_t waitTime = interfaceVar.notifTimeout / 100;
		uint16_t count = 0;
		system_t.beeper(10, 1);
		while (interfaceVar.interface_notifInterrupted == 0 )
		{
			lcd.sendBuffer(1);
//			osDelay(100);
			system_t.beeper(100, 1);
			count++;
			if( count >= waitTime )
				break;
		}
		system_t.beeper(500, 1);
	}
	interfaceVar.interface_notifInterrupted = 1;
	interfaceVar.interface_onNotifState=0;
	interfaceVar.notifTimeout = 0;
	interfaceVar.interface_notifIco = 0;
	lcd.clearBuffer();
	lcd.init(&hi2c1, 0x20);
}


void interface_buttonBeep()
{
	interfaceVar.interface_notifInterrupted = 1;
	system_t.beeper(100, 1);
}


void InterfaceTask(void *argument)
{
	system_t.beeper(100, 1);
	while(interface_onInit == 0 ){ osDelay(50);};

	while( btnTask.getLastStatus(BTN_PWR) == HELD_btn) {osDelay(50);};
	interfaceVar.ready = 1;
	btnTask.attachLongPressed(BTN_PWR, interface_onShutdown);
	system_t.beeper(100, 1);
//	btnTask.attachLongPressed(BTN_OK, interface_drawNotification );
	btnTask.attachAnyPressed(interface_buttonBeep);
	for(;;)
	{
		// displaying main display
		interface_initButton();
		while(btnTask.getLastStatus(BTN_OK)!=WAITING_btn)
		{
			osDelay(100);
		};

		while( interfaceVar.onExit == 0)
		{
			if( btnTask.getLastStatus(BTN_OK)!= WAITING_btn )	// any action on button
			{
				char bufStat[16];
				char msg[40];
//				system_getStatus(bufStat);
				if( btnTask.getLastStatus(BTN_OK) == HELD_btn)
				{
					uint32_t pressCount = 0;
					if( !sysVar.sysFlag.systemRun )
						interfaceMain.showNotification("Starting the system\n Please wait\n\n ", ico_warning, 1000UL);
					else
						interfaceMain.showNotification("Terminating the system\n Please wait\n\n ", ico_warning, 1000UL);
					osDelay(500);
					uint8_t errChk = system_t.getError();
					while(btnTask.getLastStatus(BTN_OK)!=WAITING_btn)
					{
						pressCount++;
						if(pressCount>50)
							break;
						osDelay(50);
					};
					if( sysVar.sysFlag.systemRun )
					{
						interfaceMain.showNotification("System has been\n Terminating \n by User\n", ico_success, 5000UL);
						sysVar.sysFlag.systemRun = system_STOPPED;
					}
					else
					{
						if( errChk != 0 )
						{
							sprintf(msg,"System can't start\n\n%s\n\n", system_t.getErrorCode(bufStat));
							interfaceVar.interface_onNotifState = 0;
							interfaceMain.showNotification(msg, ico_failed, 60000UL);
							sysVar.sysFlag.systemRun = system_STOPPED;
						}else
						{
							interfaceMain.showNotification("System has been\n Started \n by User\n", ico_success, 5000UL);
							sysVar.sysFlag.systemRun = system_RUNNING;
						}
					}
					while(btnTask.getLastStatus(BTN_OK)!=WAITING_btn)
					{
						osDelay(100);
					};
				}
				else
				{
					if( sysVar.sysFlag.systemRun == system_STOPPED )
						interfaceMain.showNotification("Long press for \n \"Starting\" \n \n", 4, 1000UL);
					else
						interfaceMain.showNotification("Long press for \n \"Terminating\" \n \n", 4, 1000UL);
				}

			}




			if( interfaceVar.interface_onNotifState == 0 )
			{
				lcd.clearBuffer();
				interface_drawHeader();
				if( interfaceVar.pageCursor == 0 )
					interface_drawPage1();
				else
					interface_drawPage2();
				lcd.sendBuffer(1);
			}
			else
				interface_drawNotification();
			osDelay(100);
		}
		interface_deInitButton();

		// displaying menu display
		interfaceMenu_loop();
	}
}

interfaceMain_f interfaceMain =
{
	interface_init,
	interface_isReady,
	interface_linkingTimeDate,
	interface_linkingMeasurement,
	interface_linkingStatusFlag,
	interface_linkingSystemSet,
	interface_showNotification,
};
