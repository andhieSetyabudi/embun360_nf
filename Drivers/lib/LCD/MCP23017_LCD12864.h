/*
 *  MCP23017_LCD12864.h
 *
 *  Created on: Feb 21, 2022
 *      Author: andhie setyabudi
 *      mail  : andhie.13@gmail.com
 * https://github.com/andhieSetyabudi
 */
#pragma once
#ifndef LIB_LCD_MCP23017_LCD12864_H_
#define LIB_LCD_MCP23017_LCD12864_H_

#include "stdbool.h"
#include "stdio.h"
#include "stm32f2xx.h"
#include "stm32f2xx_hal.h"

#include "delayDWT.h"
//#include "u8x8_font.h"
extern const uint8_t Font5x8[1125];
extern const uint8_t Font5x7[485];

#define IODIR_A		0x00 		///< Controls the direction of the data I/O for port A.
#define IODIR_B		0x01			///< Controls the direction of the data I/O for port B.
#define IPOL_A		0x02			///< Configures the polarity on the corresponding GPIO_ port bits for port A.
#define IPOL_B		0x03			///< Configures the polarity on the corresponding GPIO_ port bits for port B.
#define GPINTEN_A	0x04			///< Controls the interrupt-on-change for each pin of port A.
#define GPINTEN_B	0x05			///< Controls the interrupt-on-change for each pin of port B.
#define DEFVAL_A	0x06			///< Controls the default comparaison value for interrupt-on-change for port A.
#define DEFVAL_B	0x07			///< Controls the default comparaison value for interrupt-on-change for port B.
#define INTCON_A	0x08			///< Controls how the associated pin value is compared for the interrupt-on-change for port A.
#define INTCON_B	0x09			///< Controls how the associated pin value is compared for the interrupt-on-change for port B.
#define IOCON		0x0A			///< Controls the device.
#define GPPU_A		0x0C			///< Controls the pull-up resistors for the port A pins.
#define GPPU_B		0x0D			///< Controls the pull-up resistors for the port B pins.
#define INTF_A		0x0E			///< Reflects the interrupt condition on the port A pins.
#define INTF_B		0x0F			///< Reflects the interrupt condition on the port B pins.
#define INTCAP_A	0x10			///< Captures the port A value at the time the interrupt occured.
#define INTCAP_B	0x11			///< Captures the port B value at the time the interrupt occured.
#define GPIO_A		0x12			///< Reflects the value on the port A.
#define GPIO_B		0x13			///< Reflects the value on the port B.
#define OLAT_A		0x14			///< Provides access to the port A output latches.
#define OLAT_B		0x15			///< Provides access to the port B output latches.
//}MCP23_Register;

typedef struct __attribute__ ((__packed__)) lcd_var_
{
	I2C_HandleTypeDef *i2c_;
	uint8_t BL;
	uint16_t pinIO;
	uint8_t addr_;
	uint8_t flag_;
}lcd_var;

typedef struct __attribute__ ((__packed__)) lcd_ctrl_t{
	uint8_t count_char;

	uint8_t font_length_sysinfo;
	uint8_t font_characters_space;
	uint8_t font_space;
	uint8_t font_x;
	uint8_t font_y;

	uint8_t font_size_x;
	uint8_t font_size_y;

	bool inverting_text;
	bool inverting_space;
	uint8_t char_utf_8_1;
	uint8_t char_utf_8_2;

	uint16_t char_count_utf_8;

	uint8_t* font_data;
	uint8_t frame_buffer[64][16];
}lcd_ctrl;

extern lcd_var lcd_;
extern lcd_ctrl ctrl_lcd;

static void lcd_create(I2C_HandleTypeDef *i2c__, uint8_t addr);


static void lcd_writeRegister(uint8_t reg, uint8_t value);
static void lcd_writeRegisterPort(uint8_t reg, uint8_t portA, uint8_t portB);
static void lcd_transfer(uint8_t type_, uint8_t data);

static void lcd_onParallelMode(void);
static void lcd_fillBuffer(bool fill_type);
static void lcd_clearBuffer(void);
static void lcd_clear(bool type_clear, bool fill_type);

static void lcd_setBL_on(void) ;
static void lcd_setBL_off(void);
static void lcd_sendBuffer(bool type_flush);

//====== lcd base
static uint16_t lcdBase_utf8(uint8_t char_utf_8_data);
static void lcdBase_write(uint8_t character);
static size_t lcdBase_write_str(const uint8_t *buffer, size_t size);
static void lcdBase_write_char(uint16_t character);
static void lcdBase_setFont(const uint8_t* data);
static void lcdBase_invertingText(bool _inverting_text, bool _inverting_space);
static void lcdBase_drawText(uint8_t x, uint8_t y, char* data);
static void lcdBase_drawPixel(uint8_t x, uint8_t y, uint8_t pixel);
static void lcdBase_drawHLine(uint8_t x, uint8_t y, uint8_t end_line, uint8_t line_width, uint8_t pixel);
static void lcdBase_drawVLine(uint8_t x, uint8_t y, uint8_t end_line, uint8_t line_width, uint8_t pixel);
static void lcdBase_drawBox(uint8_t x, uint8_t y, uint8_t end_x, uint8_t end_y, uint8_t border_width, uint8_t pixel);
static void lcdBase_drawFillBox(uint8_t x, uint8_t y, uint8_t end_x, uint8_t end_y, uint8_t pixel);
static void lcdBase_drawCircle(uint8_t x, uint8_t y, uint8_t radius, uint8_t pixel);
static void lcdBase_drawFillCircle(uint8_t x, uint8_t y, uint8_t radius, uint8_t pixel);

static void lcdBase_drawBitmap(uint8_t x, uint8_t y, uint8_t size_x, uint8_t size_y, const uint8_t *bitmap_buffer, uint8_t pixel);



typedef struct __attribute__ ((__packed__))  lcdDef
{
	// lcd control
	void (*init) (I2C_HandleTypeDef *i2c__, uint8_t addr);
	void (*clearBuffer)	(void);
	void (*clear) (bool type_clear, bool fill_type);
	void (*BL_on)(void);
	void (*BL_off)(void);
	void (*sendBuffer) (bool type_flush);

	// lcd base function
	void (*setFont) (const uint8_t* data);

	uint16_t (*utf8) (uint8_t char_utf_8_data);
	void (*write)	(uint8_t character);
	void (*write_char)	(uint16_t character);
	size_t (*write_str)(const uint8_t *buffer, size_t size);

	void (*invertingText) (bool _inverting_text, bool _inverting_space);
	void (*text) (uint8_t x, uint8_t y, char* data);

	void (*drawPixel)	(uint8_t x, uint8_t y, uint8_t pixel);
	void (*drawHLine)	(uint8_t x, uint8_t y, uint8_t end_line, uint8_t line_width, uint8_t pixel);
	void (*drawVLine)	(uint8_t x, uint8_t y, uint8_t end_line, uint8_t line_width, uint8_t pixel);
	void (*drawBox) 	(uint8_t x, uint8_t y, uint8_t end_x, uint8_t end_y, uint8_t border_width, uint8_t pixel);
	void (*drawFillBox)	(uint8_t x, uint8_t y, uint8_t end_x, uint8_t end_y, uint8_t pixel);
	void (*drawCircle)	(uint8_t x, uint8_t y, uint8_t radius, uint8_t pixel);
	void (*drawFillCircle) (uint8_t x, uint8_t y, uint8_t radius, uint8_t pixel);
	void (*drawBitmap)	(uint8_t x, uint8_t y, uint8_t size_x, uint8_t size_y, const uint8_t *bitmap_buffer, uint8_t pixel);
	size_t (*getWitdthStr)	(const uint8_t* buffer, size_t size_);
}lcd_def;


extern lcd_def lcd;
#endif /* LIB_LCD_MCP23017_LCD12864_H_ */
