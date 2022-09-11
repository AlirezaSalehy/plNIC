################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/application.c \
../Src/comm.c \
../Src/dongle.c \
../Src/fwupdate.c \
../Src/iap.c \
../Src/main.c \
../Src/misc.c \
../Src/stk_p.c \
../Src/stm32f10x_adc.c \
../Src/stm32f10x_bkp.c \
../Src/stm32f10x_can.c \
../Src/stm32f10x_cec.c \
../Src/stm32f10x_crc.c \
../Src/stm32f10x_dac.c \
../Src/stm32f10x_dbgmcu.c \
../Src/stm32f10x_dma.c \
../Src/stm32f10x_exti.c \
../Src/stm32f10x_flash.c \
../Src/stm32f10x_fsmc.c \
../Src/stm32f10x_gpio.c \
../Src/stm32f10x_i2c.c \
../Src/stm32f10x_it.c \
../Src/stm32f10x_iwdg.c \
../Src/stm32f10x_plm.c \
../Src/stm32f10x_pwr.c \
../Src/stm32f10x_rcc.c \
../Src/stm32f10x_rtc.c \
../Src/stm32f10x_sdio.c \
../Src/stm32f10x_spi.c \
../Src/stm32f10x_tim.c \
../Src/stm32f10x_usart.c \
../Src/stm32f10x_wwdg.c \
../Src/syscalls.c \
../Src/sysmem.c \
../Src/system_stm32f10x.c \
../Src/usb.c \
../Src/user.c 

O_SRCS += \
../Src/AES128.o \
../Src/stk_l.o \
../Src/stk_n.o 

OBJS += \
./Src/application.o \
./Src/comm.o \
./Src/dongle.o \
./Src/fwupdate.o \
./Src/iap.o \
./Src/main.o \
./Src/misc.o \
./Src/stk_p.o \
./Src/stm32f10x_adc.o \
./Src/stm32f10x_bkp.o \
./Src/stm32f10x_can.o \
./Src/stm32f10x_cec.o \
./Src/stm32f10x_crc.o \
./Src/stm32f10x_dac.o \
./Src/stm32f10x_dbgmcu.o \
./Src/stm32f10x_dma.o \
./Src/stm32f10x_exti.o \
./Src/stm32f10x_flash.o \
./Src/stm32f10x_fsmc.o \
./Src/stm32f10x_gpio.o \
./Src/stm32f10x_i2c.o \
./Src/stm32f10x_it.o \
./Src/stm32f10x_iwdg.o \
./Src/stm32f10x_plm.o \
./Src/stm32f10x_pwr.o \
./Src/stm32f10x_rcc.o \
./Src/stm32f10x_rtc.o \
./Src/stm32f10x_sdio.o \
./Src/stm32f10x_spi.o \
./Src/stm32f10x_tim.o \
./Src/stm32f10x_usart.o \
./Src/stm32f10x_wwdg.o \
./Src/syscalls.o \
./Src/sysmem.o \
./Src/system_stm32f10x.o \
./Src/usb.o \
./Src/user.o 

C_DEPS += \
./Src/application.d \
./Src/comm.d \
./Src/dongle.d \
./Src/fwupdate.d \
./Src/iap.d \
./Src/main.d \
./Src/misc.d \
./Src/stk_p.d \
./Src/stm32f10x_adc.d \
./Src/stm32f10x_bkp.d \
./Src/stm32f10x_can.d \
./Src/stm32f10x_cec.d \
./Src/stm32f10x_crc.d \
./Src/stm32f10x_dac.d \
./Src/stm32f10x_dbgmcu.d \
./Src/stm32f10x_dma.d \
./Src/stm32f10x_exti.d \
./Src/stm32f10x_flash.d \
./Src/stm32f10x_fsmc.d \
./Src/stm32f10x_gpio.d \
./Src/stm32f10x_i2c.d \
./Src/stm32f10x_it.d \
./Src/stm32f10x_iwdg.d \
./Src/stm32f10x_plm.d \
./Src/stm32f10x_pwr.d \
./Src/stm32f10x_rcc.d \
./Src/stm32f10x_rtc.d \
./Src/stm32f10x_sdio.d \
./Src/stm32f10x_spi.d \
./Src/stm32f10x_tim.d \
./Src/stm32f10x_usart.d \
./Src/stm32f10x_wwdg.d \
./Src/syscalls.d \
./Src/sysmem.d \
./Src/system_stm32f10x.d \
./Src/usb.d \
./Src/user.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o Src/%.su: ../Src/%.c Src/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m3 -std=gnu11 -g3 -DDEBUG -DSTM32 -DSTM32F1 -DSTM32F103C8Tx -c -I../Inc -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfloat-abi=soft -mthumb -o "$@"

clean: clean-Src

clean-Src:
	-$(RM) ./Src/application.d ./Src/application.o ./Src/application.su ./Src/comm.d ./Src/comm.o ./Src/comm.su ./Src/dongle.d ./Src/dongle.o ./Src/dongle.su ./Src/fwupdate.d ./Src/fwupdate.o ./Src/fwupdate.su ./Src/iap.d ./Src/iap.o ./Src/iap.su ./Src/main.d ./Src/main.o ./Src/main.su ./Src/misc.d ./Src/misc.o ./Src/misc.su ./Src/stk_p.d ./Src/stk_p.o ./Src/stk_p.su ./Src/stm32f10x_adc.d ./Src/stm32f10x_adc.o ./Src/stm32f10x_adc.su ./Src/stm32f10x_bkp.d ./Src/stm32f10x_bkp.o ./Src/stm32f10x_bkp.su ./Src/stm32f10x_can.d ./Src/stm32f10x_can.o ./Src/stm32f10x_can.su ./Src/stm32f10x_cec.d ./Src/stm32f10x_cec.o ./Src/stm32f10x_cec.su ./Src/stm32f10x_crc.d ./Src/stm32f10x_crc.o ./Src/stm32f10x_crc.su ./Src/stm32f10x_dac.d ./Src/stm32f10x_dac.o ./Src/stm32f10x_dac.su ./Src/stm32f10x_dbgmcu.d ./Src/stm32f10x_dbgmcu.o ./Src/stm32f10x_dbgmcu.su ./Src/stm32f10x_dma.d ./Src/stm32f10x_dma.o ./Src/stm32f10x_dma.su ./Src/stm32f10x_exti.d ./Src/stm32f10x_exti.o ./Src/stm32f10x_exti.su ./Src/stm32f10x_flash.d ./Src/stm32f10x_flash.o ./Src/stm32f10x_flash.su ./Src/stm32f10x_fsmc.d ./Src/stm32f10x_fsmc.o ./Src/stm32f10x_fsmc.su ./Src/stm32f10x_gpio.d ./Src/stm32f10x_gpio.o ./Src/stm32f10x_gpio.su ./Src/stm32f10x_i2c.d ./Src/stm32f10x_i2c.o ./Src/stm32f10x_i2c.su ./Src/stm32f10x_it.d ./Src/stm32f10x_it.o ./Src/stm32f10x_it.su ./Src/stm32f10x_iwdg.d ./Src/stm32f10x_iwdg.o ./Src/stm32f10x_iwdg.su ./Src/stm32f10x_plm.d ./Src/stm32f10x_plm.o ./Src/stm32f10x_plm.su ./Src/stm32f10x_pwr.d ./Src/stm32f10x_pwr.o ./Src/stm32f10x_pwr.su ./Src/stm32f10x_rcc.d ./Src/stm32f10x_rcc.o ./Src/stm32f10x_rcc.su ./Src/stm32f10x_rtc.d ./Src/stm32f10x_rtc.o ./Src/stm32f10x_rtc.su ./Src/stm32f10x_sdio.d ./Src/stm32f10x_sdio.o ./Src/stm32f10x_sdio.su ./Src/stm32f10x_spi.d ./Src/stm32f10x_spi.o ./Src/stm32f10x_spi.su ./Src/stm32f10x_tim.d ./Src/stm32f10x_tim.o ./Src/stm32f10x_tim.su ./Src/stm32f10x_usart.d ./Src/stm32f10x_usart.o ./Src/stm32f10x_usart.su ./Src/stm32f10x_wwdg.d ./Src/stm32f10x_wwdg.o ./Src/stm32f10x_wwdg.su ./Src/syscalls.d ./Src/syscalls.o ./Src/syscalls.su ./Src/sysmem.d ./Src/sysmem.o ./Src/sysmem.su ./Src/system_stm32f10x.d ./Src/system_stm32f10x.o ./Src/system_stm32f10x.su ./Src/usb.d ./Src/usb.o ./Src/usb.su ./Src/user.d ./Src/user.o ./Src/user.su

.PHONY: clean-Src

