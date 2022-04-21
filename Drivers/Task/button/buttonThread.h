/*
 * buttonThread.h
 *
 *  Created on: Mar 7, 2022
 *      Author: andhie
 */
#pragma once
#ifndef TASK_BUTTON_BUTTONTHREAD_H_
#define TASK_BUTTON_BUTTONTHREAD_H_
#include "cmsis_os.h"
#include "Button.h"
#include "BSP.h"

typedef enum buttonNav_t
{
	BTN_PWR       	= 0x00U,
	BTN_SETTING    	= 0x01U,
	BTN_OK     		= 0x02U,
	BTN_UP  		= 0x03U,
	BTN_DOWN 		= 0x04U
} buttonNav_;



typedef struct __attribute__ ((__packed__)) btnTask_fc{
	void (*setPressed) 			(uint8_t num);
	void (*attachReleased)  	(uint8_t butNum, void (*P)() );
	void (*attachPressed)		(uint8_t butNum, void (*P)() );
	void (*attachDoublePressed) (uint8_t butNum, void (*P)() );
	void (*attachLongPressed)	(uint8_t butNum, void (*P)() );
	void (*dettachPressed)		(uint8_t butNum);
	void (*dettachReleased)		(uint8_t butNum);
	void (*dettachDoublePressed)(uint8_t butNum);
	void (*dettachLongPressed)	(uint8_t butNum);
	void (*clearISR)			(uint8_t butNum);

	void (*attachAnyPressed)	(void (*P)() );
	void (*dettachAnyPressed)();
	uint8_t (*getLastStatus) (uint8_t butNum);
	void (*begin)();
}btnTask_f;

extern btnTask_f btnTask;

extern void taskButton(void *argument);

#endif /* TASK_BUTTON_BUTTONTHREAD_H_ */
