/*
 * system.c
 *
 *  Created on: Mar 7, 2022
 *      Author: andhie
 */

#include "system.h"
#include "main.h"
#include "string.h"
#include "cmsis_os.h"
#include "BSP.h"
#include "sensor.h"
#include "interface.h"
#include "stm32_rtc.h"
#include "buttonThread.h"

#define SetPinLow(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(((uint32_t)GPIO_Pin) << 16))
#define SetPinHigh(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(GPIO_Pin))
#define GetInputPinValue(GPIOx, GPIO_Pin)	(((GPIOx)->IDR & (GPIO_Pin)) == 0 ? 0 : 1)


#define TURN_OFF(GPIOx, GPIO_Pin)			SetPinLow(GPIOx, GPIO_Pin)
#define TURN_ON(GPIOx, GPIO_Pin)			SetPinHigh(GPIOx, GPIO_Pin)
typedef enum sysStat_seq_t {
		TANK_SEQ = 0,
		FIN_SEQ,
		HPS_SEQ,
		EXP1_SEQ,
		EXP2_SEQ,
}sysStat_seq;

typedef enum outputPin_{
	CT1_p 	= 0,
	CT2_p 	= 1,
	Fan1_p	= 2,
	Fan2_p	= 3,
	Fan3_p	= 4,
	Pump_p	= 5,
}outPin_;


typedef enum ERROR_CODE{
	NO_ERR	= 0,
	ERR_PWR = 1,
	ERR_EXP = 2,
	ERR_FIN = 3,
	ERR_RH  = 4,
	ERR_UN,
}ERR_t;


PORT_IO portOutput[6] = {
		{.port = CT1_GPIO_Port,  .pin = CT1_Pin,  },
		{.port = CT2_GPIO_Port,  .pin = CT2_Pin,  },
		{.port = Fan1_GPIO_Port, .pin = Fan1_Pin, },
		{.port = Fan2_GPIO_Port, .pin = Fan2_Pin, },
		{.port = Fan3_GPIO_Port, .pin = Fan3_Pin, },
		{.port = Pump_GPIO_Port, .pin = Pump_Pin, },
};

PORT_IO portInput[5] = {
		{.port = Pressure1_GPIO_Port, .pin = Pressure1_Pin, },
		{.port = Pressure2_GPIO_Port, .pin = Pressure2_Pin, },
		{.port = Wlevel_L_GPIO_Port,  .pin = Wlevel_L_Pin,  },
		{.port = Wlevel_H_GPIO_Port,  .pin = Wlevel_H_Pin,  },
		{.port = HPS_GPIO_Port, 	  .pin = HPS_Pin,		},
};

systemVar sysVar = {0};
sysMem paramBuffer;

RTC_DateTypeDef date_;
RTC_TimeTypeDef time_;

void systemInputInterrupt_handler( uint16_t GPIO_Pin )
{
	switch(GPIO_Pin)
	{
		case Pressure1_Pin :
			sysVar.sysFlag.expansion1 = GetInputPinValue(portInput[0].port, portInput[0].pin);
			TURN_OFF(portOutput[CT1_p].port, portOutput[CT1_p].pin); // turn off contactor 1 ( compressor 1 )
			sysVar.outputBuffer.contactor1 = 0;
			break;
		case Pressure2_Pin :
			sysVar.sysFlag.expansion2 = GetInputPinValue(portInput[1].port, portInput[1].pin);
			TURN_OFF(portOutput[CT2_p].port, portOutput[CT2_p].pin); // turn off contactor 2 ( compressor 2 )
			sysVar.outputBuffer.contactor2 = 0;
			break;
		case HPS_Pin :
			sysVar.sysFlag.HPS = GetInputPinValue(portInput[4].port, portInput[4].pin);
			TURN_OFF(portOutput[Pump_p].port, portOutput[Pump_p].pin); // turn off contactor 1 ( compressor 1 )
			break;
	};
	if( sysVar.outputBuffer.contactor1 == 0 && sysVar.outputBuffer.contactor2 == 0 )// need to shutdown the fan
		sysVar.outputBuffer.fan = 0;
}

static void system_resetMemory()
{
	sysVar.memory.lastWaterVolume    = 0;
	sysVar.memory.totalWaterVolume   = 0;
	sysVar.memory.minFinTemperature  = LIMIT_MIN_FIN_TEMP;
	sysVar.memory.minHumidity		 = LIMIT_MIN_HUMIDITY;
	sysVar.memory.tankMode			 = floating_sensor;
	sysVar.memory.waterLevelConst	 = DEFAULT_WL_CONST;
	sysVar.memory.waterTankHeight	 = DEFAULT_WT_CONST;
}


static void system_initIO()
{
	// setup output
	for ( uint8_t i = 0; i<6; i++)
		TURN_OFF(portOutput[i].port, portOutput[i].pin);  // because there is inverting buffer ( UN2003 )
	memset(&sysVar,0,sizeof(sysVar));
}


static void system_updateTank()
{
	sysVar.sysFlag.tank = (uint8_t)EMPTY;
	uint8_t state_t[2];
	for(uint8_t u = 0; u<2; u++ )
		state_t [u] = !GetInputPinValue(portInput[2+u].port,portInput[2+u].pin);
	if ( ( state_t[0] == state_t[1] && state_t[0] == 0 )  || state_t[0] == 0 )
		sysVar.sysFlag.tank = (uint8_t) EMPTY;
	else if( state_t[0] == state_t[1] && state_t[0] == 1 )
		sysVar.sysFlag.tank = (uint8_t) FULL;
	else
		sysVar.sysFlag.tank = (uint8_t) FILLING;
}

static void system_updateFin()
{
	sysVar.sysFlag.fin1 = (uint8_t) NORMAL_t;
	sysVar.sysFlag.fin2 = (uint8_t) NORMAL_t;
	// checking fin-1
	float finTmp = sensor.getFinTemperature(0);
	if( finTmp >= sysVar.memory.minFinTemperature && finTmp <= UPPER_LIMIT_FIN_TEMP )
		sysVar.sysFlag.fin1 = (uint8_t)NORMAL_t;
	else
	{
		if(finTmp>UPPER_LIMIT_FIN_TEMP)
			sysVar.sysFlag.fin1 = (uint8_t)UPPER_t;
		else
			sysVar.sysFlag.fin1 = (uint8_t)LOWER_t;
	};
	// checking fin-2
	finTmp = sensor.getFinTemperature(1);
	if( finTmp >= sysVar.memory.minFinTemperature && finTmp <= UPPER_LIMIT_FIN_TEMP )
			sysVar.sysFlag.fin2 = (uint8_t)NORMAL_t;
	else
	{
		if(finTmp>UPPER_LIMIT_FIN_TEMP)
			sysVar.sysFlag.fin2 = (uint8_t)UPPER_t;
		else
			sysVar.sysFlag.fin2 = (uint8_t)LOWER_t;
	};
}

static uint8_t system_checkERROR(void)
{
	// checking for power line
	if( !sensor.getPGStatus() )
		return (uint8_t) ERR_PWR;
	else if (!(sysVar.sysFlag.expansion1 && sysVar.sysFlag.expansion2 ))
		return (uint8_t)ERR_EXP;
	else if( sysVar.sysFlag.fin1 != NORMAL_t || sysVar.sysFlag.fin2 != NORMAL_t )
		return (uint8_t)ERR_FIN;
	else if ( sensor.getAirRH() < sysVar.memory.minHumidity )
		return (uint8_t)ERR_RH;
	else
		return (uint8_t) NO_ERR;
}

static uint8_t system_getError(void)
{
	return sysVar.sysFlag.systemError;
}

const char* system_getErrorMsg(char* buf)
{
	switch(sysVar.sysFlag.systemError)
	{
		case ERR_PWR :
			sprintf(buf,"POWER LINE");
			break;
		case ERR_EXP :
			sprintf(buf, "OVER EXPANSION");
			break;
		case ERR_FIN:
			sprintf(buf, "FIN TEMP.Err");
			break;
		case ERR_RH :
			sprintf(buf, "rH LOWER");
			break;
		default:
			sprintf(buf, "No ERROR");
			break;
	}
	return buf;
}

static void system_updateFlag( uint8_t seq )
{
	switch(seq)
	{
		case TANK_SEQ :
			system_updateTank();
			break;
		case FIN_SEQ:
			system_updateFin();
			break;
		case HPS_SEQ :
			sysVar.sysFlag.HPS = GetInputPinValue(portInput[4].port, portInput[4].pin);
			break;
		case EXP1_SEQ :
			sysVar.sysFlag.expansion1 = GetInputPinValue(portInput[0].port, portInput[0].pin);
			break;
		case EXP2_SEQ :
			sysVar.sysFlag.expansion2 = GetInputPinValue(portInput[1].port, portInput[1].pin);
			break;
		default :
			osDelay(10);
			break;
	};
}

static void  system_updateTotalVolume		(float litre, uint32_t timeSecond)
{
	sysVar.memory.lastWaterVolume    += litre * timeSecond;
	sysVar.memory.totalWaterVolume	 += litre * timeSecond;
}

static uint8_t pumpState = 0;
static void system_updateOutput(uint8_t n)
{
	if( sensor.getPGStatus() )// if power good = OK
	{
		switch(n)
		{
			case 1:		// Contactor 1
				if ( sysVar.outputBuffer.contactor1  )
					TURN_ON(portOutput[CT1_p].port, portOutput[CT1_p].pin);
				else
					TURN_OFF(portOutput[CT1_p].port, portOutput[CT1_p].pin);
				break;
			case 2:		// contactor 2
				if ( sysVar.outputBuffer.contactor2  )
					TURN_ON(portOutput[CT2_p].port, portOutput[CT2_p].pin);
				else
					TURN_OFF(portOutput[CT2_p].port, portOutput[CT2_p].pin);
				break;
			case 3:		// fan
				if( sysVar.outputBuffer.fan != 0 )
				{
					if( ( sysVar.outputBuffer.contactor2 != 0 ) || ( sysVar.outputBuffer.contactor1 != 0 ) )
					{
						TURN_ON(portOutput[Fan1_p].port, portOutput[Fan1_p].pin);
						TURN_ON(portOutput[Fan2_p].port, portOutput[Fan2_p].pin);
						TURN_ON(portOutput[Fan3_p].port, portOutput[Fan3_p].pin);
					}
					else
					{
						TURN_OFF(portOutput[Fan1_p].port, portOutput[Fan1_p].pin);
						TURN_OFF(portOutput[Fan2_p].port, portOutput[Fan2_p].pin);
						TURN_OFF(portOutput[Fan3_p].port, portOutput[Fan3_p].pin);
					}
				}
				else
				{
					TURN_OFF(portOutput[Fan1_p].port, portOutput[Fan1_p].pin);
					TURN_OFF(portOutput[Fan2_p].port, portOutput[Fan2_p].pin);
					TURN_OFF(portOutput[Fan3_p].port, portOutput[Fan3_p].pin);
				}
				break;
			default :
				osDelay(10);
				break;
		}
	}
	else
	{
		// turn all output off, exclude water-pump
		for ( uint8_t i = 0; i<5; i++)
			TURN_OFF(portOutput[i].port, portOutput[i].pin);
		// clear all flag outputBuffer
		memset(&sysVar.outputBuffer,0,sizeof(systemIOBuffer));
	}
	if( sysVar.sysFlag.tank == EMPTY )
		pumpState = 0;
	else if ( sysVar.sysFlag.tank == FULL)
		pumpState = 1;
	if(pumpState)
		TURN_ON(portOutput[Pump_p].port, portOutput[Pump_p].pin);
	else
		TURN_OFF(portOutput[Pump_p].port, portOutput[Pump_p].pin);
	if( sysVar.sysFlag.expansion1 == 0 && sysVar.sysFlag.expansion2 == 0  &&
		sysVar.sysFlag.systemRun == system_RUNNING && sysVar.sysFlag.systemLastRun == sysVar.sysFlag.systemRun)
	{
		sysVar.sysFlag.systemRun = system_STOPPED;
		sysVar.sysFlag.systemLastRun = system_STOPPED;
		interfaceMain.showNotification("System has been\nTerminated !\n\n  ", ico_warning, 5000UL);

	}

}

void system_beeper(uint32_t time_, uint8_t count)
{
	while(count > 0)
	{
		SetPinHigh(top_buzzer_GPIO_Port, top_buzzer_Pin);
		osDelay(time_);
		SetPinLow(top_buzzer_GPIO_Port, top_buzzer_Pin);
		osDelay(time_/2);
		count--;
	}
}

const char* system_getTankStatusStr(uint8_t status, char* buf)
{
	switch (status)
	{
		case EMPTY :
			sprintf(buf,(const char*)"EMPTY");
			break;
		case FILLING :
			sprintf(buf,(const char*)"FILLING");
			break;
		case FULL:
			sprintf(buf,(const char*)"FULL");
			break;
		default :
			sprintf(buf,(const char*)"UNKNOWN");
			break;
	};
	return buf;
}

const char* system_getTankModeStr(tankMode_ mode_, char* buf)
{
	switch(mode_)
	{
		case floating_sensor:
			sprintf(buf,(const char*)"Floating");
			break;
		case us_sensor:
			sprintf(buf,(const char*)"Ultrasonic");
			break;
		default :
			sprintf(buf,(const char*)"UNKNOWN");
			break;
	};
	return buf;
}

const char* system_getStatus(char* buf)
{
	switch (sysVar.sysFlag.systemRun)
	{
		case system_STOPPED :
			sprintf(buf, (const char*)"STOPPED");
			break;
		case system_RUNNING :
			sprintf(buf, (const char*)"RUNNING");
			break;
		default:
			sprintf(buf, (const char*) "ERR-");
			break;
	};
	return buf;
}


/* USER CODE BEGIN Header_taskSystem */
/**
  * @brief  Function implementing the system for input output for machine.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_taskSystem */
static bool needReset = false;
static bool expiredDate = false;

void taskSystem(void *argument)
{
	system_initIO();
	uint32_t size_data = 0;
	// check memory

	eeprom_fillBuffer();
	size_data = sizeof(sysMem);
	eeprom_get_buffer(0,(uint8_t* )&sysVar.memory, size_data);
	uint8_t conf = eeprom_read_buffer(0);
	if( conf == 0xff && eeprom_read_buffer(1)==0xff && eeprom_read_buffer(2)==0xff)
	{
		system_resetMemory();
		size_data = sizeof(sysMem);
		eeprom_put_buffer(0, (uint8_t* )&sysVar.memory, size_data);
		eeprom_flushBuffer();
	}
	else
	{
		size_data = sizeof(sysMem);
		eeprom_get_buffer(0,(uint8_t* )&sysVar.memory, size_data);
	}
//	eeprom_fillBuffer();
//	eeprom_get_buffer(0,(uint8_t* )&sysVar.memory, size_data);
	/*
	rtc_get_memory(0,(uint8_t* )&size_data, sizeof(size_data));
	if( size_data == 0xFFFFFFFF )
	{
		system_resetMemory();
		size_data = sizeof(sysMem);
		rtc_put_memory(0,(uint8_t* )&sysVar.memory, size_data);
	}
	else
	{
		size_data = sizeof(sysMem);
		rtc_get_memory(0,(uint8_t* )&sysVar.memory, size_data);
	}*/
	sensor.setTankHeight_const(sysVar.memory.waterTankHeight);
	sensor.setWaterFlow_const(sysVar.memory.waterLevelConst);
	normalizeMinimumTemperature(&sysVar.memory.minFinTemperature);
	normalizeMinimumHumidity(&sysVar.memory.minHumidity);
	paramBuffer = sysVar.memory;
	uint8_t seq_= 0;
	uint8_t outputState = 0;
	sysVar.sysFlag.systemLastRun = sysVar.sysFlag.systemRun = system_STOPPED;
	uint32_t timeBAK 		= 0;
//	uint32_t timeSYS_R 		= 0;
//	uint32_t sysRTicking 	= 0;
	if( date_.Date == MFG_DATE && date_.Month == MFG_MONTH && date_.Year == MFG_YEAR && date_.WeekDay == MFG_DAY )
	{
		sysVar.memory.totalWaterVolume = 0;
		sysVar.memory.lastWaterVolume  = 0;
	}
	expiredDate = false;
	// if expired
	if( sysVar.memory.month != date_.Month )
	{
		uint8_t bufferDate = sysVar.memory.month % 12;
		bufferDate += 1;
		if( bufferDate >= date_.Month && date_.Date >= 1)
		{
			expiredDate = true;
		}

	}
	needReset = false;
	sysVar.memory.totalWaterVolume = 0;
	for(;;)
	{
		// if expired
		if( expiredDate && sysVar.sysFlag.systemRun == system_RUNNING )
		{
			needReset = true;
			expiredDate = false;
		}
		// if need reset memory
		if( needReset )
		{
			sysVar.memory.totalWaterVolume = 0;
			sysVar.memory.lastWaterVolume  = 0;
			needReset = false;
		}
		// prevent the roll-back of ticking
//		if(osKernelGetTickCount() < timeSYS_R )
//			timeSYS_R = osKernelGetTickCount();
//		if( osKernelGetTickCount() - timeSYS_R >= 100UL )
//		{
			if( sysVar.sysFlag.systemRun == system_RUNNING)
			{
				sysVar.outputBuffer.contactor1 = 1;//sysVar.sysFlag.systemRun;
				sysVar.outputBuffer.contactor2 = 1;//sysVar.sysFlag.systemRun;
				sysVar.outputBuffer.fan		   = 1;
//				sysRTicking++;
			}else{
				sysVar.outputBuffer.contactor1 = 0;//sysVar.sysFlag.systemRun;
				sysVar.outputBuffer.contactor2 = 0;//sysVar.sysFlag.systemRun;
				sysVar.outputBuffer.fan		   = 0;
//				sysRTicking=0;
			}
//			timeSYS_R = osKernelGetTickCount();
//		}

		// update input
		for( seq_ = 0; seq_ < 5; seq_++ )
			system_updateFlag(seq_);

		// check system error
		sysVar.sysFlag.systemError = system_checkERROR();
		if( sysVar.sysFlag.systemError != NO_ERR && sysVar.sysFlag.systemRun == system_RUNNING )	// if there is any error
		{
			// turn off the E-switch
			sysVar.sysFlag.systemRun = system_STOPPED;
			// notification - WARNING !!
			switch( sysVar.sysFlag.systemError )
			{
				case ERR_PWR:
					interfaceMain.showNotification("System has been\nTerminated !\nPower Line ERROR\n ", ico_failed, 15000UL);
					break;
				case ERR_EXP:
					interfaceMain.showNotification("System has been\nTerminated !\nOver Expansion\n ", ico_failed, 15000UL);
					break;
				case ERR_FIN:
					interfaceMain.showNotification("System has been\nTerminated !\nFin Temp. Err\n ", ico_warning, 15000UL);
					break;
				case ERR_RH:
					interfaceMain.showNotification("System has been\nTerminated !\nLower rH \n ", ico_warning, 15000UL);
					break;
				default:
					interfaceMain.showNotification("System has been\nTerminated !\nUnknwon Err\n ", ico_warning, 15000UL);
					break;
			}
		}
//
//		// update limit sensor
//		if( sensor.getFinTemperature(0) <= sysVar.memory.minFinTemperature  || sensor.getFinTemperature(1) <= sysVar.memory.minFinTemperature )
//		{
//			if( sysVar.sysFlag.systemRun == system_RUNNING )
//			{
//				sysVar.sysFlag.systemRun = system_STOPPED;
//				sysVar.sysFlag.systemLastRun = system_STOPPED;
//				interfaceMain.showNotification("System has been\nTerminated !\n\n ", ico_warning, 5000UL);
//			}
//		}

		// back-up to memory ( Emulated EEPROM )
		if( HAL_GetTick() < timeBAK )	// prevent for roll-over the tick-counting
			timeBAK = HAL_GetTick();
		if( HAL_GetTick() - timeBAK >= 5000UL )
		{
			size_data = sizeof(sysMem);
			sysVar.memory.month = date_.Month;
			sysVar.memory.date	= date_.Date;
			uint8_t *a = (uint8_t *)&paramBuffer;
			uint8_t *b = (uint8_t *)&sysVar.memory;
			for(uint8_t ui = 0; ui < sizeof(sysMem); ui++)
			{
				if( *a != *b )
				{
//					rtc_put_memory(0,(uint8_t* )&sysVar.memory, size_data);
					eeprom_put_buffer(0,(uint8_t* )&sysVar.memory, size_data);
					eeprom_flushBuffer();
//					eeprom_fillBuffer();
					paramBuffer = sysVar.memory;
//					rtc_get_memory(0,(uint8_t* )&sysVar.memory, size_data);
					break;
				}
				a++; b++;
			}
			timeBAK = HAL_GetTick();
		}

		system_updateOutput(outputState);
		outputState++;
		outputState = outputState % 4;
		osDelay(100);
	}
}

static inline void system_updateRunning(uint8_t state)
{
//	if( )
}

static inline void systemRuning()
{
	sysVar.sysFlag.systemRun++;
	sysVar.sysFlag.systemRun%=2;

	// checking systemRun, if there is any request or state
	if ( sysVar.sysFlag.systemLastRun != sysVar.sysFlag.systemRun  )
	{
		if( sysVar.sysFlag.systemRun  != system_STOPPED )	// request to starting machine
		{
			sysVar.sysFlag.systemRun = system_RUNNING;
			if( !sensor.getPGStatus() )
			{
				sysVar.sysFlag.systemRun = system_STOPPED;
				interfaceMain.showNotification("POWER SOURCE\n\n is Failed !\n\nPlease Check it ! ", ico_failed, 1500UL);
				memset(&sysVar.outputBuffer,0,sizeof(systemIOBuffer));
			}
			else
			{
				if( sensor.getFinTemperature(0) <= sysVar.memory.minFinTemperature  || sensor.getFinTemperature(1) <= sysVar.memory.minFinTemperature )
				{
					if( sysVar.sysFlag.systemRun == system_RUNNING )
					{
						sysVar.sysFlag.systemRun = system_STOPPED;
						sysVar.sysFlag.systemLastRun = system_STOPPED;
						interfaceMain.showNotification("System has been\nTerminated !\n\n ", ico_warning, 5000UL);
					}
				}
				else
					interfaceMain.showNotification("System starting\n successfully ! ", ico_success, 1500UL);
			}
		}
		else
		{
			interfaceMain.showNotification("System was stopped\n Thank you  ", ico_success, 1500UL);
			// copy system run status to buffer output
			sysVar.outputBuffer.contactor1 = 0;//sysVar.sysFlag.systemRun;
			sysVar.outputBuffer.contactor2 = 0;//sysVar.sysFlag.systemRun;
			sysVar.outputBuffer.fan		   = 0;//sysVar.sysFlag.systemRun;
		}
		sysVar.sysFlag.systemLastRun   = sysVar.sysFlag.systemRun;
	}
	else{
		sysVar.outputBuffer.contactor1 = 0;//sysVar.sysFlag.systemRun;
		sysVar.outputBuffer.contactor2 = 0;//sysVar.sysFlag.systemRun;
		sysVar.outputBuffer.fan		   = 0;//sysVar.sysFlag.systemRun;
	}
}

void system_resetLog()
{
	needReset = true;
}

system_func system_t = {
		system_initIO,
		system_beeper,
		system_getTankStatusStr,
		system_getTankModeStr,
		system_getStatus,
		system_getError,
		system_getErrorMsg,
		systemInputInterrupt_handler,
		system_updateTotalVolume,
		systemRuning,
		system_resetLog,
};
