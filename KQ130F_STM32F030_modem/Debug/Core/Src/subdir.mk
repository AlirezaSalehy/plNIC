################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Src/freertos.c \
../Core/Src/main.c \
../Core/Src/stm32f0xx_hal_msp.c \
../Core/Src/stm32f0xx_hal_timebase_tim.c \
../Core/Src/stm32f0xx_it.c \
../Core/Src/syscalls.c \
../Core/Src/sysmem.c \
../Core/Src/system_stm32f0xx.c 

CPP_SRCS += \
../Core/Src/AppMain.cpp \
../Core/Src/main.cpp 

C_DEPS += \
./Core/Src/freertos.d \
./Core/Src/main.d \
./Core/Src/stm32f0xx_hal_msp.d \
./Core/Src/stm32f0xx_hal_timebase_tim.d \
./Core/Src/stm32f0xx_it.d \
./Core/Src/syscalls.d \
./Core/Src/sysmem.d \
./Core/Src/system_stm32f0xx.d 

OBJS += \
./Core/Src/AppMain.o \
./Core/Src/freertos.o \
./Core/Src/main.o \
./Core/Src/stm32f0xx_hal_msp.o \
./Core/Src/stm32f0xx_hal_timebase_tim.o \
./Core/Src/stm32f0xx_it.o \
./Core/Src/syscalls.o \
./Core/Src/sysmem.o \
./Core/Src/system_stm32f0xx.o 

CPP_DEPS += \
./Core/Src/AppMain.d \
./Core/Src/main.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.cpp Core/Src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m0 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I"C:/Users/Lidoma/Desktop/plNIC/KQ130F_STM32F030_modem/lib/Inc" -I../Core/Inc -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/STM32F0xx_HAL_Driver/Inc -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Middlewares/Third_Party/FreeRTOS/Source/include -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/CMSIS/Device/ST/STM32F0xx/Include -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/CMSIS/Include -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
Core/Src/%.o Core/Src/%.su: ../Core/Src/%.c Core/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I"C:/Users/Lidoma/Desktop/plNIC/KQ130F_STM32F030_modem/lib/Inc" -I../Core/Inc -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/STM32F0xx_HAL_Driver/Inc -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Middlewares/Third_Party/FreeRTOS/Source/include -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/CMSIS/Device/ST/STM32F0xx/Include -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/CMSIS/Include -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Core-2f-Src

clean-Core-2f-Src:
	-$(RM) ./Core/Src/AppMain.d ./Core/Src/AppMain.o ./Core/Src/AppMain.su ./Core/Src/freertos.d ./Core/Src/freertos.o ./Core/Src/freertos.su ./Core/Src/main.d ./Core/Src/main.o ./Core/Src/main.su ./Core/Src/stm32f0xx_hal_msp.d ./Core/Src/stm32f0xx_hal_msp.o ./Core/Src/stm32f0xx_hal_msp.su ./Core/Src/stm32f0xx_hal_timebase_tim.d ./Core/Src/stm32f0xx_hal_timebase_tim.o ./Core/Src/stm32f0xx_hal_timebase_tim.su ./Core/Src/stm32f0xx_it.d ./Core/Src/stm32f0xx_it.o ./Core/Src/stm32f0xx_it.su ./Core/Src/syscalls.d ./Core/Src/syscalls.o ./Core/Src/syscalls.su ./Core/Src/sysmem.d ./Core/Src/sysmem.o ./Core/Src/sysmem.su ./Core/Src/system_stm32f0xx.d ./Core/Src/system_stm32f0xx.o ./Core/Src/system_stm32f0xx.su

.PHONY: clean-Core-2f-Src

