/*
 * mainDisplay.c
 *
 *  Created on: Mar 4, 2022
 *      Author: andhie
 */


#include "MCP23017_LCD12864.h"
#include "mainDisplay.h"
#include "string.h"

void mainDisplay_drawHeader(mainDisplay_var *var)
{
	char buff[20];
	lcd.setFont(Font5x7);
	if( var->date == NULL)
		sprintf(buff,"1-1-2000");
	else
		sprintf(buff,"%02d-%02d-%02d", var->date->Date, var->date->Month, var->date->Year+2000);
	lcd.text(1,1, buff);
	if( var->time == NULL )
		sprintf(buff,"00:00");
	else
		sprintf(buff,"%02d:%02d", var->time->Hours, var->time->Minutes);
	uint8_t pos_x = 128 - lcd.getWitdthStr((const uint8_t*)buff, strlen(buff));
	lcd.text(pos_x-1,1, buff);
	lcd.drawHLine(0,10,128,1,1);
}

void mainDisplay_drawContent(mainDisplay_var * var)
{
	for (size_t i = 0; i < 4; i++) {
		for (size_t j = 0; j < 2; j++) {
			//gadgetDisplay->tableChanged[i][j] = 0;
		}
	}
}

void mainDisplay_draw(mainDisplay_var * var)
{
	lcd.clearBuffer();
	mainDisplay_drawHeader(var);
	mainDisplay_drawContent(var);
	lcd.sendBuffer(false);
}
