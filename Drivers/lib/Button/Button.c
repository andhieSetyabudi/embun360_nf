/*
 * Button.c
 *
 *  Created on: Feb 24, 2022
 *      Author: andhie
 */

#include "Button.h"
#include "main.h"

void button_create(button_var* var, PORT_IO port_map, uint8_t state, unsigned long duration_)
{
	var->port_map = port_map;
	var->State = state;
	var->lastButtonState = var->lastState =!var->State; //lastState should be inverted from State
	var->duration = duration_;
	var->holdTime = 1000;
	var->DBInterval = 55;
	var->tick = HAL_GetTick;
	button_clearAllISR(var);
}

void button_SetHoldTime(button_var* var, unsigned long time_)
{
	var->holdTime = time_; // Set the hold time in seconds
}

void button_SetDebounceTime(button_var* var, unsigned long time_)
{
	var->DBInterval = time_;
}

void button_Pressed(button_var* var, void (*P)() )
{
	var->F_Pressed = P;
}

void delete_Button_pressed(button_var* var)
{
	var->F_Pressed = NULL;
}

void button_DoublePressed(button_var* var, void (*DP)() )
{
	  var->F_DPressed = DP;
}

//void button_MultiPressed(button_var* var, void (*MUL)() )
//{
//	  var->F_Mult = MUL;
//}

void button_Held(button_var* var, void (*HOLD)() )
{
	var->F_Hold = HOLD;
}

void delete_Button_held(button_var* var)
{
	var->F_Hold = NULL;
}

void button_Released(button_var* var, void(*RELEASED_f)())
{
	var->F_Released = RELEASED_f;
}

void delete_Button_released(button_var* var)
{
	var->F_Released = NULL;
}

void delete_Button_doublePressed(button_var* var)
{
	var->F_DPressed = NULL;
}

void button_clearAllISR(button_var* var)
{
	var->F_Pressed  = NULL;
	var->F_DPressed = NULL;
//	var->F_Mult     = NULL;
	var->F_Hold     = NULL;
	var->F_Released = NULL;
}

uint8_t button_check(button_var* var)
{
  var->output = 0;
  uint8_t button = digitalRead(var->port_map);
  if ( button != var->lastButtonState)
  {
	  var->lastDebounceTime = var->tick();
	  var->lastButtonState = button;
	  var->time = var->tick();
  }


  // Check for Rollover
  unsigned long RO_Time = var->tick(); // current time into RollOver variable
  if (RO_Time < var->time) // is the RollOver variable smaller than ontime?
	var->time = RO_Time; // if yes,  reset ontime to zero

  while ( (var->tick() - var->time) <= var->duration)
  {
	 // button = digitalRead(ButtonPin);     // read the button
	 if (button != var->lastState) // see if the button is not held down.
	 {
	   if (button == !var->State) // button was released
	   {
		 if ((var->tick() - var->lastDebounceTime) >= var->DBInterval) // button debounce, very important
		 {
		   var->output++;                    // increment a counter, but only when the button is pressed then released
		   var->lastDebounceTime = var->tick(); // update the debounce time
		 }
	   }
	   var->lastState = digitalRead(var->port_map); // update the buttons last state with it's new state
	 }
	 button = digitalRead(var->port_map);
  }
  if(button == var->State && button == var->lastButtonState)
	if( (var->HeldTime = (var->tick() - var->time)) > var->holdTime )
		var->output = HELD_btn;

  switch (var->output)
  {
	case WAITING_btn:
	  {
		if( var->lastOut != WAITING_btn )
		{
		  if( var->F_Released != NULL )
			  var->F_Released();
		}
		var->lastOut = var->output = WAITING_btn;
	  }
	  break;

	case PRESSED_btn:
	  if (var->F_Pressed!=NULL)
		  var->F_Pressed();
	  break;

	case DOUBLE_PRESSED_btn:
	  if (var->F_DPressed!=NULL)
		  var->F_DPressed();
	  break;

	/*case MULTI_PRESSED_btn:
	  if (var->F_Mult!=NULL)
		  var->F_Mult();
	  break;
	*/
	case HELD_btn:
	  if (var->F_Hold!=NULL)
		  var->F_Hold();
	  break;
  }
  var->lastOut = var->output;
  return var->output; // return the output count
}

float button_GetHeldTime(button_var* var, float divisor)
{
  if (divisor > 0)
	return var->HeldTime / divisor;
  else
	return -1;
}

static void button_setTick(button_var* var, uint32_t (*tick)())
{
	var->tick = tick;
}

buttonF button = {
		button_create,
		button_SetHoldTime,
		button_SetDebounceTime,

		button_Pressed,
		button_DoublePressed,
		button_Held,
		button_Released,

		delete_Button_held,
		delete_Button_pressed,
		delete_Button_released,
		delete_Button_doublePressed,
		button_clearAllISR,

		button_check,
		button_GetHeldTime,
		button_setTick,
};
