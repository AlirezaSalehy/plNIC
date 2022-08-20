################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../lib/Src/rsa.c 

CPP_SRCS += \
../lib/Src/Encryption.cpp 

C_DEPS += \
./lib/Src/rsa.d 

OBJS += \
./lib/Src/Encryption.o \
./lib/Src/rsa.o 

CPP_DEPS += \
./lib/Src/Encryption.d 


# Each subdirectory must supply rules for building sources it contributes
lib/Src/%.o lib/Src/%.su: ../lib/Src/%.cpp lib/Src/subdir.mk
	arm-none-eabi-g++ "$<" -mcpu=cortex-m0 -std=gnu++14 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I"C:/Users/Lidoma/Desktop/plNIC/KQ130F_STM32F030_modem/lib/Inc" -I../Core/Inc -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/STM32F0xx_HAL_Driver/Inc -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Middlewares/Third_Party/FreeRTOS/Source/include -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/CMSIS/Device/ST/STM32F0xx/Include -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/CMSIS/Include -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -fno-exceptions -fno-rtti -fno-use-cxa-atexit -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"
lib/Src/%.o lib/Src/%.su: ../lib/Src/%.c lib/Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m0 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F030x8 -c -I"C:/Users/Lidoma/Desktop/plNIC/KQ130F_STM32F030_modem/lib/Inc" -I../Core/Inc -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/STM32F0xx_HAL_Driver/Inc -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Middlewares/Third_Party/FreeRTOS/Source/include -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/CMSIS/Device/ST/STM32F0xx/Include -I/home/lidoma/STM32Cube/Repository/STM32Cube_FW_F0_V1.11.3/Drivers/CMSIS/Include -I../Drivers/STM32F0xx_HAL_Driver/Inc -I../Drivers/STM32F0xx_HAL_Driver/Inc/Legacy -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM0 -I../Drivers/CMSIS/Device/ST/STM32F0xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-lib-2f-Src

clean-lib-2f-Src:
	-$(RM) ./lib/Src/Encryption.d ./lib/Src/Encryption.o ./lib/Src/Encryption.su ./lib/Src/rsa.d ./lib/Src/rsa.o ./lib/Src/rsa.su

.PHONY: clean-lib-2f-Src

