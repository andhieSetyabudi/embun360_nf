/*
 * splashScreen.c
 *
 *  Created on: Mar 4, 2022
 *      Author: andhie
 */

#include "splashScreen.h"

uint8_t* bmpHex = NULL;
uint8_t bmpWidth  = 0;
uint8_t bmpHeight = 0;

static void (*splScr_delay)(uint32_t time_) = NULL;
void splScr_attachHalt(void (*v)(uint32_t time_))
{
	splScr_delay = v;
}

void splScr_halt(uint32_t time_)
{
	if(splScr_delay != NULL)
		splScr_delay(time_);
	else
		HAL_Delay(time_);
}


void splScr_init(const uint8_t* bmp, uint8_t width, uint8_t height, const uint8_t* font)
{
	bmpHex = (uint8_t*)bmp;
	bmpWidth = width;
	bmpHeight = height;
	lcd.setFont(font);
}

void splScr_displaySplash (splash_align align, const char* str1, const char* str2,  uint32_t timeout_)
{
	if( bmpHex == NULL )
		return;
	uint8_t pos_x, pos_y;
	uint8_t pos_x_t = 0, pos_y_t = 0;
	switch (align){
		case LEFT_ :
						pos_x = 5;
						pos_y = 0;
						break;
		case RIGHT_:
						pos_y = 5;
						pos_x = 128 - bmpWidth;
						break;
		case CENTER_ :
		default :
						pos_y = 5;
						pos_x = 128 - bmpWidth;
						pos_x /= 2;
						break;

	};

	pos_x_t = 128 - lcd.getWitdthStr(str2, strlen(str2));
	pos_y_t = 64 - ctrl_lcd.font_size_y;
	pos_y = 64 - bmpHeight;
	pos_y /= 2;
	pos_y -= ctrl_lcd.font_size_y;
	lcd.clearBuffer();
	lcd.drawBitmap(pos_x,pos_y,bmpWidth,bmpHeight,bmpHex,1);
	lcd.text(0,pos_y_t, str1);
	lcd.text(pos_x_t,pos_y_t, str2);
	lcd.sendBuffer(true);
	splScr_halt(timeout_);
}

splashScreen_ft splashScreen ={
		splScr_init,
		splScr_attachHalt,
		splScr_displaySplash,
};
