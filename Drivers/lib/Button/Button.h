/*
 * Button.h
 *
 *  Created on: Feb 24, 2022
 *      Author: andhie
 */

#ifndef LIB_BUTTON_BUTTON_H_
#define LIB_BUTTON_BUTTON_H_

#include "BSP.h"

#define SECONDS 1000000
#define MILLISECONDS 1000
#define MICROSECONDS 1


#define WAITING_btn				0
#define PRESSED_btn				1
#define DOUBLE_PRESSED_btn		2
//#define MULTI_PRESSED_btn		3
#define HELD_btn 		 		100
#define RELEASED_btn			101

typedef struct button_var_t
{
	uint8_t State, lastState;
	uint8_t lastButtonState;
	uint8_t	output, lastOut;

	PORT_IO	 port_map;
	unsigned long holdTime, DBInterval, RO_Time;
	unsigned long time, duration, HeldTime;
	unsigned long lastDebounceTime;

	uint32_t (*tick)();

	void 	(*F_Pressed)();
	void 	(*F_DPressed)();
//	void 	(*F_Mult)();
	void 	(*F_Hold)();
	void    (*F_Released)();
}button_var;

static void button_create(button_var* var, PORT_IO port_map, uint8_t state, unsigned long duration_);
static void button_SetHoldTime(button_var* var, unsigned long time_);
static void button_SetDebounceTime(button_var* var, unsigned long time_);

static void button_Pressed(button_var* var, void (*P)() );
static void button_DoublePressed(button_var* var, void (*DP)() );
//void button_MultiPressed(button_var* var, void (*MUL)() );
static void button_Held(button_var* var, void (*HOLD)() );
static void button_Released(button_var* var, void(*RELEASED_f)());

static void delete_Button_held(button_var* var);
static void delete_Button_pressed(button_var* var);
static void delete_Button_released(button_var* var);
static void delete_Button_doublePressed(button_var* var);
static void button_clearAllISR(button_var* var);

static uint8_t button_check(button_var* var);
static float button_GetHeldTime(button_var* var, float divisor);
static void button_setTick(button_var* var, uint32_t (*tick)());

typedef struct __attribute__ ((__packed__)) button_funct{
	void (*create) 			(button_var* var, PORT_IO port_map, uint8_t state, unsigned long duration_);
	void (*setHoldTime)		(button_var* var, unsigned long time_);
	void (*setDebounceTime)	(button_var* var, unsigned long time_);

	void (*attachPressed)		(button_var* var, void (*P)() );
	void (*attachDoublePressed) (button_var* var, void (*DP)() );
	void (*attachHeld)			(button_var* var, void (*HOLD)() );
	void (*attachReleased)		(button_var* var, void(*RELEASED_f)());

	void (*dettachHeld)			(button_var* var);
	void (*dettachPressed)		(button_var* var);
	void (*dettachReleased)		(button_var* var);
	void (*dettachDoublePressed)(button_var* var);
	void (*clearAllISR)			(button_var* var);

	uint8_t (*check)			(button_var* var);
	float (*getHeldTime)		(button_var* var, float divisor);
	void (*setTick)				(button_var* var, uint32_t (*tick)());
}buttonF;

extern buttonF button;

#endif /* LIB_BUTTON_BUTTON_H_ */
