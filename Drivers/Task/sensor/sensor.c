/*
 * sensor.c
 *
 *  Created on: Mar 7, 2022
 *      Author: andhie
 */

#include "sensor.h"
#include "string.h"
#include "delayDWT.h"

#define SN_GPIO_GetInputPinValue(GPIOx, GPIO_Pin)	(((GPIOx)->IDR & (GPIO_Pin)) == 0 ? 0 : 1)
#define SN_GPIO_SetPinLow(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(((uint32_t)GPIO_Pin) << 16))
#define SN_GPIO_SetPinHigh(GPIOx, GPIO_Pin)			((GPIOx)->BSRR = (uint32_t)(GPIO_Pin))

sensor_var sensorVar ={
		.airSensorFlag = false,
		.finSensorFlag = false,
		.finTmpErrCount = 0,

		.fin_temperature = {0.f,0.f},
		.air_temperature = 0.f,
		.air_rH = 0.f,

			// power sensor
		.phaseCounter = {0U, 0U},
		.phaseFrequency = {0.f, 0.f},
		.PG_Flag = false,

			// waterflow sensor
		.waterFlowCounter = 0U,
		.waterFlowFrequency = 0.f,
		.waterFlow_inLPM = 0.f,
		.waterFlow_const = 0.f,

		 .tankLevel_inPercent = 0,
		 .tankHeight_const	= 0,
		 .waterLevelFrom_cap = 0,
};

static bool sensor_getAirSensorFlag(void){
	return sensorVar.airSensorFlag;
}

static bool sensor_getFinSensorFlag(void){
	return sensorVar.finSensorFlag;
}

static float sensor_getFinTemperature(uint8_t ch){
	return (sensorVar.fin_temperature[ch]);
}

static float sensor_getAirTemperature(void){
	return sensorVar.air_temperature;
}

static float sensor_getAirRH(void){
	return sensorVar.air_rH;
}

static float sensor_getPhaseFrequency(uint8_t ch){
	return (sensorVar.phaseFrequency[ch]);
}

static bool sensor_getPG_status(void){
	return sensorVar.PG_Flag ;//|true;
}

static float sensor_getWaterFlow_Frequency(void){
	return sensorVar.waterFlowFrequency;
}

static float sensor_getWaterFlow_inLPM(void){
	return sensorVar.waterFlow_inLPM;
}

static float sensor_getWaterFlow_const(void){
	return sensorVar.waterFlow_const;
}

static void sensor_setWaterFlow_const(float c){
	sensorVar.waterFlow_const = c;
}

static float sensor_getWaterLevel_inPercent(void){
	return sensorVar.tankLevel_inPercent;
}

static float sensor_getTankHeigth_const(void){
	return sensorVar.tankHeight_const;
}

static void sensor_setTankHeight_const(float c){
	sensorVar.tankHeight_const = c;
}

static float sensor_getTankSensor(void){
	return sensorVar.waterLevelFrom_cap;
}

//============== sensor calculating progress using timer and EXT interrupt

/*
 *  updating counter for each phase
 *  ## need external-interrupt in falling-edge detecting
 */
void sensor_phaseCountingUp(uint8_t ch){
	sensorVar.phaseCounter[ch]++;
}

/*
 *  updating frequency of each phase counter
 *  ## need timer-counter match in 1 second
 */

void sensor_phaseUpdateFrequency(uint8_t ch){
	if( sensorVar.phaseCounter[ch] > 0 )
		sensorVar.phaseFrequency[ch] = (float)sensorVar.phaseCounter[ch];
	else
		sensorVar.phaseFrequency[ch] = 0.f;
	sensorVar.phaseCounter[ch] = 0;
	float tempF1 = fabsf(sensorVar.phaseFrequency[0] - 50.f);
	float tempF2 = fabsf(sensorVar.phaseFrequency[1] - 50.f);

	if( ( tempF1 > 10 ) || ( tempF2 > 10 ) )
		sensorVar.PG_Flag = false;
	else
		sensorVar.PG_Flag = true;
}

/*
 *  updating counter for waterFlow - sensor pulse
 *  ## need external-interrupt in falling-edge detecting
 */
void sensor_waterFlowCountingUp(void){
	sensorVar.waterFlowCounter++;
}

/*
 *  updating frequency of each phase counter
 *  ## need timer-counter match in 1 second
 */

#include "system.h"
void sensor_waterFlowUpdateFrequency(void){
	sensorVar.waterFlowFrequency =  (float)sensorVar.waterFlowCounter ;
	sensorVar.waterFlowCounter = 0;
	sensorVar.waterFlowFrequency = sensorVar.waterFlowFrequency / (735.8f*2.915915f);//( sensorVar.waterFlow_const * 10.f);
	sysVar.memory.totalWaterVolume += sensorVar.waterFlowFrequency;
	sysVar.memory.lastWaterVolume  += sensorVar.waterFlowFrequency;
	sensorVar.waterFlow_inLPM = sensorVar.waterFlowFrequency * 60.f;
}




//=============== sensor calculating progress using time-delay for ultrasonic
uint32_t pulseIn( GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, uint8_t state, uint32_t timeout)
{
  // Cache the port and bit of the pin in order to speed up the
  // pulse width measuring loop and achieve finer resolution.
//	state = state = 0 ? 0 : state;
	uint8_t stateMask = SN_GPIO_GetInputPinValue(GPIOx, GPIO_Pin);
	uint32_t counter = 0;
	// wait for any previous pulse to end
//	while( stateMask == state )
//	{
//		counter++;
//		if( counter > timeout )
//			return 0;
//		stateMask = SN_GPIO_GetInputPinValue(GPIOx, GPIO_Pin);
//		DWT_Delay_us(1);
//	}

	// wait for the pulse to start
	counter=0;
	stateMask = SN_GPIO_GetInputPinValue(GPIOx, GPIO_Pin);
	while( stateMask != state )
	{
		counter++;
		if( counter > timeout )
			return 0;
		stateMask = SN_GPIO_GetInputPinValue(GPIOx, GPIO_Pin);
		DWT_Delay_us(1);
	}
	// wait for the pulse to stop
	counter = 0;
	while( stateMask && state )
	{
		counter++;
		if( counter > timeout )
			return 0;
		stateMask = SN_GPIO_GetInputPinValue(GPIOx, GPIO_Pin);
		DWT_Delay_us(1);
	}
	return counter;
}

static void ultrasonic_measuring(void)
{
	SN_GPIO_SetPinLow(US_TRIGER_GPIO_Port, US_TRIGER_Pin); // Set the trigger pin to low for 2uS
	DWT_Delay_us(2);

	SN_GPIO_SetPinHigh(US_TRIGER_GPIO_Port, US_TRIGER_Pin); // Send a 10uS high to trigger ranging
	DWT_Delay_us(20);

	SN_GPIO_SetPinLow(US_TRIGER_GPIO_Port, US_TRIGER_Pin);
	uint32_t duration = pulseIn(US_ECHO_GPIO_Port, US_ECHO_Pin, 1, 5800000UL);
	sensorVar.waterLevelFrom_cap = (float)duration * 0.034/2.f;
}


//============== sensor calculating progress using counter pulse timeout
static void sensor_waterLevelMeasuring(void)
{
	ultrasonic_measuring();
	float tmpH = sensorVar.tankHeight_const - sensorVar.waterLevelFrom_cap+24.0f;
	if( tmpH < 0.f )
		sensorVar.tankLevel_inPercent = 0;
	else
	{
		sensorVar.tankLevel_inPercent = tmpH /sensorVar.tankHeight_const;
		sensorVar.tankLevel_inPercent*=100.f;
	}
}



// ================= sensor initializing variable
sensor_ctrl sensorCTRL_var = {0};
void sensorDelay(uint32_t time_)
{
	osDelay(time_);
}

void sensorInit(void)
{
	DWT_Delay_Init();
	// initializing fin temperature sensor
	ds18b20_init(&sensorCTRL_var.finTempSensor, DS_GPIO_Port, DS_Pin);
	memset(sensorCTRL_var.finTempAddr[0],0,8);
	memset(sensorCTRL_var.finTempAddr[1],0,8);
	if( sensorCTRL_var.finTempSensor.numberOfDevices > 0 ) {
		if( sensorCTRL_var.finTempSensor.numberOfDevices >= 2 )
			sensorVar.finSensorFlag = true;
		else
			sensorVar.finSensorFlag = false;
		for(uint8_t a = 0; a < sensorCTRL_var.finTempSensor.numberOfDevices; a++)
			ds18b20_searchAddress(&sensorCTRL_var.finTempSensor, sensorCTRL_var.finTempAddr[a], a);
		ds18b20_requestTemperatures(&sensorCTRL_var.finTempSensor);
	}else
		sensorVar.finSensorFlag = false;
	sensorVar.finTmpErrCount = 0;

	// initializing air temperature and humidity sensor
#if RH_SEN_TYPE	== SHTC3_SENSOR
	if ( SHTC3.begin(&sensorCTRL_var.airTempHumSensor, &I2C_CH, 0x70) == SHTC3_OK )
		sensorVar.airSensorFlag = true;
	else
		sensorVar.airSensorFlag = false;
#else
	AHT10.halt = sensorDelay;
	AHT10.getTick = HAL_GetTick;
	if( AHT10.begin(&sensorCTRL_var.airTempHumSensor, &I2C_CH, AIR_RH_ADDR) == AHT10_OK )
		sensorVar.airSensorFlag = true;
	else
		sensorVar.airSensorFlag = false;
#endif
	osDelay(100);

}

static uint8_t sensorTaskRun = 0;
extern osThreadId_t taskSensorHandle;
static void sensorTaskOnStart (void){
	sensorTaskRun = 1;
}

static void sensorTaskOnPause(void){
	vTaskSuspend(taskSensorHandle);
	osDelay(50);
}

static void sensorTaskOnResume(void){
	vTaskResume(taskSensorHandle);
	osDelay(50);
}

/* USER CODE BEGIN Header_SensorTask */
/**
* @brief Function implementing the taskSensor thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_SensorTask */
void SensorTask(void *argument)
{
	while(sensorTaskRun == 0 ){ osDelay(50); }
	sensorInit();
	osDelay(500);
	uint32_t finTempTimeCount = 0;
	float tmp=0.f;
	uint32_t timeSens = 0;
	for(;;)
	{
//		if( HAL_GetTick() - timeSens >= 1000UL )
//		{
//			sensor_waterFlowUpdateFrequency();
//			timeSens = HAL_GetTick();
//		}
		// update fin temperature sensor
		if( sensorVar.finSensorFlag  /*|| ( sensorVar.finTmpErrCount < 3 )*/ )
		{
			if( HAL_GetTick() - finTempTimeCount >= 450UL )
			{
				ds18b20_requestTemperatures(&sensorCTRL_var.finTempSensor);
				for(uint8_t a = 0; a < sensorCTRL_var.finTempSensor.numberOfDevices; a++)
				{
					tmp = ds18b20_getTempCByAddress(&sensorCTRL_var.finTempSensor,sensorCTRL_var.finTempAddr[a],false );
					if( (int)tmp == (int)DEVICE_DISCONNECTED_RAW ) {
						sensorVar.finTmpErrCount++;
						if( sensorVar.finTmpErrCount >= 3 )
						{
							sensorVar.finSensorFlag = false;
							sensorVar.fin_temperature[a] = tmp;
							sensorVar.finTmpErrCount = 0;
							break;
						}

					}else
					{
						sensorVar.fin_temperature[a] = tmp;
						sensorVar.finTmpErrCount = 0;
					};
				}
				finTempTimeCount = HAL_GetTick();
			};
		}else{
			ds18b20_init(&sensorCTRL_var.finTempSensor, DS_GPIO_Port, DS_Pin);
			if( sensorCTRL_var.finTempSensor.numberOfDevices > 0 ) {
				if( sensorCTRL_var.finTempSensor.numberOfDevices >= 2 )
					sensorVar.finSensorFlag = true;
				else
					sensorVar.finSensorFlag = false;
				for(uint8_t a = 0; a < sensorCTRL_var.finTempSensor.numberOfDevices; a++)
					ds18b20_searchAddress(&sensorCTRL_var.finTempSensor, sensorCTRL_var.finTempAddr[a], a);
				ds18b20_requestTemperatures(&sensorCTRL_var.finTempSensor);
				sensorVar.finTmpErrCount = 0;
			}else
				sensorVar.finSensorFlag = false;
			finTempTimeCount = HAL_GetTick();
		}
		osDelay(100);

		// update air temperature and humidity sensor
		if( sensorVar.airSensorFlag ) {
		#if RH_SEN_TYPE	== SHTC3_SENSOR
			if( SHTC3.getEvent(&sensorCTRL_var.airTempHumSensor) == SHTC3_OK )
			{
				sensorVar.air_temperature 	= SHTC3.getTemperature(&sensorCTRL_var.airTempHumSensor);
				sensorVar.air_rH 			= SHTC3.getHumidity(&sensorCTRL_var.airTempHumSensor);
			}
		#else
			if( AHT10.getEvent(&sensorCTRL_var.airTempHumSensor,100) == AHT10_OK  | true ) {
				sensorVar.air_temperature = sensorCTRL_var.airTempHumSensor.temperature;
				sensorVar.air_rH = sensorCTRL_var.airTempHumSensor.humidity;
			}
		#endif
			else
				sensorVar.airSensorFlag = false;
		}else
		{
		#if RH_SEN_TYPE	== SHTC3_SENSOR
			if ( SHTC3.begin(&sensorCTRL_var.airTempHumSensor, &I2C_CH, SHTC3_DEFAULT_ADDR) == SHTC3_OK )
				sensorVar.airSensorFlag = true;
		#else
			if( AHT10.begin(&sensorCTRL_var.airTempHumSensor, &I2C_CH, AIR_RH_ADDR) == AHT10_OK )
				sensorVar.airSensorFlag = true;
		#endif
			else
				sensorVar.airSensorFlag = false;
		};
		osDelay(100);

		// updating water level sensor in tank
		sensor_waterLevelMeasuring();
		osDelay(100);
	}
	sensorTaskRun = 0;
}


// summary of sensor function in structure pointer
sensor_f sensor =
{
		sensorTaskOnStart,
		sensorTaskOnPause,
		sensorTaskOnResume,
		sensor_getAirSensorFlag,
		sensor_getFinSensorFlag,
		sensor_getFinTemperature,
		sensor_getAirTemperature,
		sensor_getAirRH,
		sensor_getPhaseFrequency,
		sensor_getPG_status,
		sensor_getWaterFlow_Frequency,
		sensor_getWaterFlow_inLPM,
		sensor_getWaterFlow_const,
		sensor_setWaterFlow_const,
		sensor_getWaterLevel_inPercent,
		sensor_getTankHeigth_const,
		sensor_setTankHeight_const,
		sensor_setTankHeight_const,
};
