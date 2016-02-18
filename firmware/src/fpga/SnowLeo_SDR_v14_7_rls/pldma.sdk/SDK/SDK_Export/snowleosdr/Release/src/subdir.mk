################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/si5351.c \
../src/snowleo_hw.c \
../src/snowleo_i2c_ops.c \
../src/snowleo_spi_ops.c 

OBJS += \
./src/si5351.o \
./src/snowleo_hw.o \
./src/snowleo_i2c_ops.o \
./src/snowleo_spi_ops.o 

C_DEPS += \
./src/si5351.d \
./src/snowleo_hw.d \
./src/snowleo_i2c_ops.d \
./src/snowleo_spi_ops.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: ARM Linux gcc compiler'
	arm-xilinx-linux-gnueabi-gcc -Wall -O2 -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


