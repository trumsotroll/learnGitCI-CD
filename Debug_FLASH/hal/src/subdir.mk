################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../hal/src/gpio_hal.c 

OBJS += \
./hal/src/gpio_hal.o 

C_DEPS += \
./hal/src/gpio_hal.d 


# Each subdirectory must supply rules for building sources it contributes
hal/src/%.o: ../hal/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@hal/src/gpio_hal.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


