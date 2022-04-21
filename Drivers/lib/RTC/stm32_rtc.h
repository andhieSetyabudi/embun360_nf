/*
 * stm32_rtc.h
 *
 *  Created on: Feb 24, 2022
 *      Author: andhie
 */

#ifndef LIB_RTC_STM32_RTC_H_
#define LIB_RTC_STM32_RTC_H_

#include "stdio.h"
#include "stm32_rtc.h"
#include "stm32f2xx.h"
#include "stm32f2xx_hal.h"

#include "stdbool.h"

#define RTC_MAGIC_KEY			0x0A0B00


extern RTC_HandleTypeDef hrtc;

static inline int weekDayOf(int year, int month, int day)
{
    int wday = 0;
    wday = (day + ((153 * (month + 12 * ((14 - month) / 12) - 3) + 2) / 5)
               + (365 * (year + 4800 - ((14 - month) / 12)))
               + ((year + 4800 - ((14 - month) / 12)) / 4)
               - ((year + 4800 - ((14 - month) / 12)) / 100)
               + ((year + 4800 - ((14 - month) / 12)) / 400)
               - 32045)
        % 7;
    return wday;
}

static __inline__ uint8_t getMaxDayOf(uint8_t month, uint8_t years)
{
    // Check for 31 Days
    if (month == 1 || month == 3 || month == 5
        || month == 7 || month == 8 || month == 10
        || month == 12) {
        return 31;
    }

    // Check for 30 Days
    else if (month == 4 || month == 6
             || month == 9 || month == 11) {
        return 30;
    }

    // Check for 28/29 Days
    else if (month == 2) {
        if( years%4==0 )
            return 29;
        else
            return 28;
    }
    else
        return 0;
}

bool __attribute__((optimize("O2"))) rtc_init();
void __attribute__((optimize("O2"))) rtc_getDateTime(RTC_DateTypeDef* date, RTC_TimeTypeDef* time);
void  __attribute__((optimize("O2"))) rtc_setDateTime(RTC_DateTypeDef *date, RTC_TimeTypeDef *time);


// using RTC backup register as memory-backup
#define MAX_REG			RTC_BKP_DR18
#define MAX_LENGTH		MAX_REG*4

void __attribute__((optimize("O2"))) rtc_get_memory(uint32_t addr, uint8_t* data, uint8_t len);
void __attribute__((optimize("O2"))) rtc_put_memory(uint32_t addr, uint8_t* data, uint8_t len);

#endif /* LIB_RTC_STM32_RTC_H_ */
