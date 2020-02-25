################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Src/IC_MCU150.c \
../Src/main.c \
../Src/stm32g4xx_hal_msp.c \
../Src/stm32g4xx_it.c \
../Src/syscalls.c \
../Src/system_stm32g4xx.c 

OBJS += \
./Src/IC_MCU150.o \
./Src/main.o \
./Src/stm32g4xx_hal_msp.o \
./Src/stm32g4xx_it.o \
./Src/syscalls.o \
./Src/system_stm32g4xx.o 

C_DEPS += \
./Src/IC_MCU150.d \
./Src/main.d \
./Src/stm32g4xx_hal_msp.d \
./Src/stm32g4xx_it.d \
./Src/syscalls.d \
./Src/system_stm32g4xx.d 


# Each subdirectory must supply rules for building sources it contributes
Src/%.o: ../Src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32G474xx -I"/run/media/gilg/linData/MK/STM/workspace/stm32g474veh/motor_control_foc_stm32/Inc" -I"/run/media/gilg/linData/MK/STM/workspace/stm32g474veh/motor_control_foc_stm32/CanOpen_config" -I"/run/media/gilg/linData/MK/STM/workspace/stm32g474veh/motor_control_foc_stm32/CanOpen_stack" -I"/run/media/gilg/linData/MK/STM/workspace/stm32g474veh/motor_control_foc_stm32/Drivers/STM32G4xx_HAL_Driver/Inc" -I"/run/media/gilg/linData/MK/STM/workspace/stm32g474veh/motor_control_foc_stm32/Drivers/STM32G4xx_HAL_Driver/Inc/Legacy" -I"/run/media/gilg/linData/MK/STM/workspace/stm32g474veh/motor_control_foc_stm32/Drivers/CMSIS/Device/ST/STM32G4xx/Include" -I"/run/media/gilg/linData/MK/STM/workspace/stm32g474veh/motor_control_foc_stm32/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


