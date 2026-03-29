################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../driver/src/GPIO_driver.c \
../driver/src/Systick_driver.c \
../driver/src/Systick_hal.c \
../driver/src/UART_driver.c 

OBJS += \
./driver/src/GPIO_driver.o \
./driver/src/Systick_driver.o \
./driver/src/Systick_hal.o \
./driver/src/UART_driver.o 

C_DEPS += \
./driver/src/GPIO_driver.d \
./driver/src/Systick_driver.d \
./driver/src/Systick_hal.d \
./driver/src/UART_driver.d 


# Each subdirectory must supply rules for building sources it contributes
driver/src/%.o: ../driver/src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Standard S32DS C Compiler'
	arm-none-eabi-gcc "@driver/src/GPIO_driver.args" -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


