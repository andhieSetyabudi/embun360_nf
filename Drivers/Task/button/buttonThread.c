/*
 * buttonThread.c
 *
 *  Created on: Mar 7, 2022
 *      Author: andhie
 */

#include "main.h"
#include "buttonThread.h"

static button_var BTN[5]         = {0};
static PORT_IO BTN_PORT_IO[5]    = {0};
static void (*any_Pressed)() 	 = NULL;

static uint8_t buttonTask_start = 0;
static void initButton_pin(void)
{
// defined pin
	// Power Button
	BTN_PORT_IO[BTN_PWR].port = PWR_BTN_GPIO_Port;
	BTN_PORT_IO[BTN_PWR].pin  = PWR_BTN_Pin;
	// setting button
	BTN_PORT_IO[BTN_SETTING].port = STG_BTN_GPIO_Port;
	BTN_PORT_IO[BTN_SETTING].pin  = STG_BTN_Pin;
	// ok button
	BTN_PORT_IO[BTN_OK].port = OK_BTN_GPIO_Port;
	BTN_PORT_IO[BTN_OK].pin  = OK_BTN_Pin;
	// up button
	BTN_PORT_IO[BTN_UP].port = UP_BTN_GPIO_Port;
	BTN_PORT_IO[BTN_UP].pin  = UP_BTN_Pin;
	// down button
	BTN_PORT_IO[BTN_DOWN].port = DOWN_BTN_GPIO_Port;
	BTN_PORT_IO[BTN_DOWN].pin  = DOWN_BTN_Pin;

	for( uint8_t i = 0; i < 5; i++)
	{
		button.create(&BTN[i], BTN_PORT_IO[i], 0, 350UL);
		button.setTick(&BTN[i], osKernelGetTickCount);
	}

}

static void buttonSetPressed(uint8_t butNum)
{
	if (butNum > 5)
		return;
	if( BTN[butNum].lastOut == WAITING_btn )
	{
		BTN[butNum].lastOut = PRESSED_btn;
		if(any_Pressed )
			any_Pressed();
	}

}

static void buttonAttachReleased(uint8_t butNum, void (*P)() )
{
	if (butNum > 5)
		return;
	button.attachReleased(&BTN[butNum],P);
}

static void buttonAttachPressed(uint8_t butNum, void (*P)() )
{
	if (butNum > 5)
		return;
	button.attachPressed(&BTN[butNum],P);
}

static void buttonAttachDoublePressed(uint8_t butNum, void (*P)() )
{
	if (butNum > 5)
		return;
	button.attachDoublePressed(&BTN[butNum],P);
}

static void buttonAttachLongPressed(uint8_t butNum, void (*P)() )
{
	if (butNum > 5)
		return;
	button.attachHeld(&BTN[butNum],P);
}

static void buttonDettachPressed(uint8_t butNum)
{
	if (butNum > 5)
		return;
	button.dettachPressed(&BTN[butNum]);
}

static void buttonDettachReleased(uint8_t butNum)
{
	if (butNum > 5)
		return;
	button.dettachReleased(&BTN[butNum]);
}

static void buttonDettachDoublePressed(uint8_t butNum)
{
	if (butNum > 5)
		return;
	button.dettachDoublePressed(&BTN[butNum]);
}

static void buttonDettachLongPressed(uint8_t butNum)
{
	if (butNum > 5)
		return;
	button.dettachHeld(&BTN[butNum]);
}

static void buttonClearISR(uint8_t butNum)
{
	if (butNum > 5)
		return;
	button.clearAllISR(&BTN[butNum]);
}

static void buttonAttachAnyPressed(void (*P)() )
{
	any_Pressed = P;
}

static void buttonDettachAnyPressed()
{
	any_Pressed = NULL;
}

static uint8_t buttonGetLastStatus(uint8_t butNum)
{
	return BTN[butNum].lastOut;
}


static void buttonStartTask()
{
	buttonTask_start = 1;
}
// button task on FREERTOS
void taskButton(void *argument)
{
	buttonTask_start = 0;
	initButton_pin();
	while(buttonTask_start == 0 ) {osDelay(50);};
	uint8_t lastAnyButtonState = WAITING_btn;
	for(;;)
	{
		lastAnyButtonState = WAITING_btn;
		for( uint8_t i = 0; i < 5; i++)
		{
			lastAnyButtonState = button.check(&BTN[i]);
			if( lastAnyButtonState != WAITING_btn)// && BTN[i].lastOut == WAITING_btn )
			{
				if(any_Pressed )
					any_Pressed();
			}
			lastAnyButtonState = WAITING_btn;
		}
		osDelay(5);
	}
}


btnTask_f btnTask =
{
		buttonSetPressed,
		buttonAttachReleased,
		buttonAttachPressed,
		buttonAttachDoublePressed,
		buttonAttachLongPressed,
		buttonDettachPressed,
		buttonDettachReleased,
		buttonDettachDoublePressed,
		buttonDettachLongPressed,
		buttonClearISR,
		buttonAttachAnyPressed,
		buttonDettachAnyPressed,
		buttonGetLastStatus,
		buttonStartTask,
};
