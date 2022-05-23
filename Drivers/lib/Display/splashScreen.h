/*
 * splashScreen.h
 *
 *  Created on: Mar 4, 2022
 *      Author: andhie
 */

#ifndef LIB_DISPLAY_SPLASHSCREEN_H_
#define LIB_DISPLAY_SPLASHSCREEN_H_

#include "MCP23017_LCD12864.h"

extern const uint8_t splashWidth;
extern const uint8_t splashHeight;
extern const uint8_t splashScreenICO[];

extern const uint8_t warningIco_width;
extern const uint8_t warningIco_heigth;
extern const uint8_t warningIcon[];

extern const uint8_t successIcon_width;
extern const uint8_t successIcon_heigth;
extern const uint8_t failedIcon[];

extern const uint8_t failedIcon_width;
extern const uint8_t failedIcon_heigth;
extern const uint8_t successIcon[];

typedef enum splash_align_t{
	LEFT_  = 0,
	CENTER_= 1,
	RIGHT_ = 2,
}splash_align;

//void (*splScr_delay)(uint32_t time_);
static void splScr_attachHalt(void (*v)(uint32_t time_));
static void splScr_halt(uint32_t time_);
static void splScr_init(const uint8_t* bmp, uint8_t width, uint8_t height, const uint8_t* font);
static void splScr_displaySplash (splash_align align, const char* str1, const char* str2, uint32_t timeout_ );

typedef struct __attribute__ ((__packed__))  splashScreen_f
{
	void (*init)(const uint8_t* bmp, uint8_t width, uint8_t height, const uint8_t* font);
	void (*attachDelay) (void (*v)(uint32_t time_));
	void (*display)	(splash_align align, const char* str1, const char* str2, uint32_t timeout_ );
} splashScreen_ft;

extern splashScreen_ft splashScreen;

#endif /* LIB_DISPLAY_SPLASHSCREEN_H_ */
