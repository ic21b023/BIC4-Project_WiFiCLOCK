################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (10.3-2021.10)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Own_Libraries/SerialProtocol/serialprotocol.c 

OBJS += \
./Own_Libraries/SerialProtocol/serialprotocol.o 

C_DEPS += \
./Own_Libraries/SerialProtocol/serialprotocol.d 


# Each subdirectory must supply rules for building sources it contributes
Own_Libraries/SerialProtocol/%.o Own_Libraries/SerialProtocol/%.su: ../Own_Libraries/SerialProtocol/%.c Own_Libraries/SerialProtocol/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32L432xx -c -I../Core/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc -I../Drivers/STM32L4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32L4xx/Include -I../Drivers/CMSIS/Include -I../Middlewares/Third_Party/FreeRTOS/Source/include -I../Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS_V2 -I../Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F -I"C:/Users/reite/STM32CubeIDE/workspace_1.9.0/BIC4-Project_WiFiCLOCK/Own_Libraries" -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Own_Libraries-2f-SerialProtocol

clean-Own_Libraries-2f-SerialProtocol:
	-$(RM) ./Own_Libraries/SerialProtocol/serialprotocol.d ./Own_Libraries/SerialProtocol/serialprotocol.o ./Own_Libraries/SerialProtocol/serialprotocol.su

.PHONY: clean-Own_Libraries-2f-SerialProtocol

