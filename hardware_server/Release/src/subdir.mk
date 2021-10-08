################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/dc_device_thread.c \
../src/hardware_server.c \
../src/logger.c \
../src/socket_func.c \
../src/system_func.c \
../src/system_thread.c \
../src/tlv.c \
../src/uart.c \
../src/xml.c 

OBJS += \
./src/dc_device_thread.o \
./src/hardware_server.o \
./src/logger.o \
./src/socket_func.o \
./src/system_func.o \
./src/system_thread.o \
./src/tlv.o \
./src/uart.o \
./src/xml.o 

C_DEPS += \
./src/dc_device_thread.d \
./src/hardware_server.d \
./src/logger.d \
./src/socket_func.d \
./src/system_func.d \
./src/system_thread.d \
./src/tlv.d \
./src/uart.d \
./src/xml.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


