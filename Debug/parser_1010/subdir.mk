################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../parser_1010/parser_1010.c 

OBJS += \
./parser_1010/parser_1010.o 

C_DEPS += \
./parser_1010/parser_1010.d 


# Each subdirectory must supply rules for building sources it contributes
parser_1010/%.o: ../parser_1010/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: MCU GCC Compiler'
	@echo $(PWD)
	arm-none-eabi-gcc -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16 '-D__weak=__attribute__((weak))' '-D__packed="__attribute__((__packed__))"' -DUSE_HAL_DRIVER -DSTM32G474xx -I"/home/gilg/workspace/STM32G474/stm32_g4veh_test/Inc" -I"/home/gilg/workspace/STM32G474/stm32_g4veh_test/Drivers/STM32G4xx_HAL_Driver/Inc" -I"/home/gilg/workspace/STM32G474/stm32_g4veh_test/Drivers/STM32G4xx_HAL_Driver/Inc/Legacy" -I"/home/gilg/workspace/STM32G474/stm32_g4veh_test/Drivers/CMSIS/Device/ST/STM32G4xx/Include" -I"/home/gilg/workspace/STM32G474/stm32_g4veh_test/Drivers/CMSIS/Include"  -Og -g3 -Wall -fmessage-length=0 -ffunction-sections -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


