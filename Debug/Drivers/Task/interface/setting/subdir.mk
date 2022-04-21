################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/Task/interface/setting/interfaceAbout.c \
../Drivers/Task/interface/setting/interfaceMenu.c \
../Drivers/Task/interface/setting/interfaceParameter.c \
../Drivers/Task/interface/setting/interfaceTimeDate.c 

OBJS += \
./Drivers/Task/interface/setting/interfaceAbout.o \
./Drivers/Task/interface/setting/interfaceMenu.o \
./Drivers/Task/interface/setting/interfaceParameter.o \
./Drivers/Task/interface/setting/interfaceTimeDate.o 

C_DEPS += \
./Drivers/Task/interface/setting/interfaceAbout.d \
./Drivers/Task/interface/setting/interfaceMenu.d \
./Drivers/Task/interface/setting/interfaceParameter.d \
./Drivers/Task/interface/setting/interfaceTimeDate.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/Task/interface/setting/%.o: ../Drivers/Task/interface/setting/%.c Drivers/Task/interface/setting/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu17 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F205xx -c -I../Core/Inc -I../Drivers/STM32F2xx_HAL_Driver/Inc -I../Drivers/STM32F2xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F2xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/LCD" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/BSP" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/RTC" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/Button" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/DS18b20" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/AHT10" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/Task/interface" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/Display" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/Task/sensor" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/Task/button" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/Task/system" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/eeprom" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/Task/interface/setting" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/SHTC3" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-Task-2f-interface-2f-setting

clean-Drivers-2f-Task-2f-interface-2f-setting:
	-$(RM) ./Drivers/Task/interface/setting/interfaceAbout.d ./Drivers/Task/interface/setting/interfaceAbout.o ./Drivers/Task/interface/setting/interfaceMenu.d ./Drivers/Task/interface/setting/interfaceMenu.o ./Drivers/Task/interface/setting/interfaceParameter.d ./Drivers/Task/interface/setting/interfaceParameter.o ./Drivers/Task/interface/setting/interfaceTimeDate.d ./Drivers/Task/interface/setting/interfaceTimeDate.o

.PHONY: clean-Drivers-2f-Task-2f-interface-2f-setting

