/*
 * menuDisplay.c
 *
 *  Created on: Mar 4, 2022
 *      Author: andhie
 */


#include "MCP23017_LCD12864.h"
#include "menuDisplay.h"
#include "string.h"

void menuDisplay_create(menuDisplay_var* var)
{
	var->menu = NULL;
	var->totalMenu = 0;
	var->position = 0;
	var->setTitle = 0;
	var->date = NULL;
	var->time = NULL;
	var->cursor = NULL;
}

void menuDisplay_next(menuDisplay_var *var)
{
	var->position++;
	var->position = var->position % var->totalMenu;
}

void menuDisplay_previous(menuDisplay_var *var) {
	var->position += var->totalMenu;
	var->position--;
	var->position = var->position % var->totalMenu;

}

void menuDisplay_resetPosition(menuDisplay_var *var) {
	var->position = 0;
}

uint8_t menuDisplay_getPosition(menuDisplay_var *var) {
	return var->position;
}

void menuDisplay_setMenu(menuDisplay_var *var, const char **menu, unsigned char totalMenu) {
	var->menu = menu;
	var->totalMenu = totalMenu;
}

void menuDisplay_attachDateTime(menuDisplay_var *var,RTC_DateTypeDef *date_, RTC_TimeTypeDef *time_)
{
	var->date = date_;
	var->time = time_;
}

void menuDisplay_setTitle(menuDisplay_var *var, const char *title){
	var->setTitle = 1;
	strcpy(var->title, title);
}
void menuDisplay_setCursor(menuDisplay_var *var, const char* cursor){
	var->cursor = cursor;
}

static uint32_t menuTimeBlinking = 0;
static uint8_t menuBlinked = 0;
void menuDisplay_drawHeader(menuDisplay_var *var)
{
	char buff[20];
	lcd.setFont(Font5x7);
	if( var->date == NULL)
		sprintf(buff,"1-1-2000");
	else
		sprintf(buff,"%02d-%02d-%02d", var->date->Date, var->date->Month, var->date->Year+2000);
	lcd.text(1,1, buff);
	if( var->time == NULL )
		sprintf(buff,"00%c00",menuBlinked!=0?':':' ');
	else
		sprintf(buff,"%02d%c%02d", var->time->Hours, menuBlinked!=0?':':' ', var->time->Minutes);
	uint8_t pos_x = 128 - lcd.getWitdthStr((const uint8_t*)buff, strlen(buff));
	lcd.text(pos_x-1,1, buff);
	lcd.drawHLine(0,10,128,1,1);
	if( HAL_GetTick() - menuTimeBlinking >= 500UL )
	{
		menuBlinked = ~menuBlinked;
		menuTimeBlinking = HAL_GetTick();
	}
}

void menuDisplay_drawMenu(menuDisplay_var *var) {
	int x = 0, y = 15;
	unsigned char totalMenuShowed ;
	if( var->setTitle != 0 )
	{
		totalMenuShowed = var->totalMenu < 3 ? var->totalMenu : 3;
	}
	else
	{
		totalMenuShowed = var->totalMenu < 4 ? var->totalMenu : 4;
	}
      unsigned char menuOffset = var->position < var->totalMenu ? 0 : var->position - totalMenuShowed - 1;
	  if (var->position < menuOffset) {
		menuOffset = var->position;
	  } else if(menuOffset + totalMenuShowed - 1 < var->position) {
		menuOffset = var->position - totalMenuShowed + 1;
	  }

	if(var->setTitle== 1){
		lcd.text(x + 2, y, var->title);
		uint8_t len_line = lcd.getWitdthStr((uint8_t*)var->title, strlen(var->title));
		lcd.drawHLine(x+1,y+9,len_line+2,1,1);
		y+=12;
	}
	for (size_t i = 0; i < totalMenuShowed; i++) {
		if( var->cursor )
		{
			if (var->position - menuOffset == i)
			{
				lcd.text( x + 4, y + (12 * i), var->cursor);
			}
		}

	    lcd.text( x + 13, y + (12 * i),(char* ) var->menu[i + menuOffset]);
	  }
}

void menuDisplay_draw(menuDisplay_var *var)
{
	lcd.clearBuffer();
	menuDisplay_drawHeader(var);
	menuDisplay_drawMenu(var);
	lcd.sendBuffer(true);
}


menuDisplay_f menuDisplay =
{
	menuDisplay_create,
	menuDisplay_next,
	menuDisplay_previous,
	menuDisplay_resetPosition,
	menuDisplay_setMenu,
	menuDisplay_attachDateTime,
	menuDisplay_getPosition,
	menuDisplay_draw,
	menuDisplay_setTitle,
	menuDisplay_setCursor,
};
