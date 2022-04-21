################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (9-2020-q2-update)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Drivers/lib/DS18b20/bak/asad/ds18b20.c \
../Drivers/lib/DS18b20/bak/asad/onewire.c 

OBJS += \
./Drivers/lib/DS18b20/bak/asad/ds18b20.o \
./Drivers/lib/DS18b20/bak/asad/onewire.o 

C_DEPS += \
./Drivers/lib/DS18b20/bak/asad/ds18b20.d \
./Drivers/lib/DS18b20/bak/asad/onewire.d 


# Each subdirectory must supply rules for building sources it contributes
Drivers/lib/DS18b20/bak/asad/%.o: ../Drivers/lib/DS18b20/bak/asad/%.c Drivers/lib/DS18b20/bak/asad/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g -DDEBUG -DUSE_HAL_DRIVER -DSTM32F205xx -c -I../Core/Inc -I../Drivers/STM32F2xx_HAL_Driver/Inc -I../Drivers/STM32F2xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F2xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM3 -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/LCD" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/BSP" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/RTC" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/Button" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/DS18b20" -I"D:/Working Area/Widya-imersif/water harvesting/FIrmware/embun_farm/embun_NF_new/Drivers/lib/AHT10" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Drivers-2f-lib-2f-DS18b20-2f-bak-2f-asad

clean-Drivers-2f-lib-2f-DS18b20-2f-bak-2f-asad:
	-$(RM) ./Drivers/lib/DS18b20/bak/asad/ds18b20.d ./Drivers/lib/DS18b20/bak/asad/ds18b20.o ./Drivers/lib/DS18b20/bak/asad/onewire.d ./Drivers/lib/DS18b20/bak/asad/onewire.o

.PHONY: clean-Drivers-2f-lib-2f-DS18b20-2f-bak-2f-asad

