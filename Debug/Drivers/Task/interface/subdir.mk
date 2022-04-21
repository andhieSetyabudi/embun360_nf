################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Task/interface/interface.c 

OBJS += \
./Drivers/Task/interface/interface.o 

C_DEPS += \
./Drivers/Task/interface/interface.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Task/interface/%.o: ../Drivers/Task/interface/%.c Drivers/Task/interface/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu17 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F205xx -c -I../Core/Inc -I../Drivers/STM32F2xx_HAL_Driver/Inc -I../Drivers/STM32F2xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F2xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/LCD" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/BSP" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/RTC" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/Button" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/DS18b20" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/AHT10" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/Task/interface" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/Display" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/Task/sensor" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/Task/button" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/Task/system" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/eeprom" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/Task/interface/setting" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/SHTC3" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-Task-2f-interface

clean-Drivers-2f-Task-2f-interface:
	-$(RM) ./Drivers/Task/interface/interface.d ./Drivers/Task/interface/interface.o

.PHONY: clean-Drivers-2f-Task-2f-interface

