################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/configuration.c \
../src/dc_device.c \
../src/dc_device_client_data.c \
../src/dc_device_data.c \
../src/dc_device_driver_func.c \
../src/dc_device_message_func.c \
../src/dc_func.c \
../src/fr_device.c \
../src/fr_device_client_data.c \
../src/fr_device_data.c \
../src/fr_device_driver_func.c \
../src/hardware_server.c \
../src/logger.c \
../src/md5.c \
../src/ppc_device.c \
../src/ppc_device_client_data.c \
../src/ppc_device_data.c \
../src/ppc_device_driver_func.c \
../src/ppc_device_message_func.c \
../src/ppc_func.c \
../src/sc_device.c \
../src/sc_device_client_data.c \
../src/sc_device_data.c \
../src/sc_device_driver_func.c \
../src/sc_device_message_func.c \
../src/sc_func.c \
../src/socket_func.c \
../src/system_func.c \
../src/system_message_func.c \
../src/system_thread.c \
../src/tgs_device.c \
../src/tgs_device_client_data.c \
../src/tgs_device_data.c \
../src/tgs_device_driver_func.c \
../src/tgs_device_message_func.c \
../src/tgs_func.c \
../src/tlv.c \
../src/xml.c 

OBJS += \
./src/configuration.o \
./src/dc_device.o \
./src/dc_device_client_data.o \
./src/dc_device_data.o \
./src/dc_device_driver_func.o \
./src/dc_device_message_func.o \
./src/dc_func.o \
./src/fr_device.o \
./src/fr_device_client_data.o \
./src/fr_device_data.o \
./src/fr_device_driver_func.o \
./src/hardware_server.o \
./src/logger.o \
./src/md5.o \
./src/ppc_device.o \
./src/ppc_device_client_data.o \
./src/ppc_device_data.o \
./src/ppc_device_driver_func.o \
./src/ppc_device_message_func.o \
./src/ppc_func.o \
./src/sc_device.o \
./src/sc_device_client_data.o \
./src/sc_device_data.o \
./src/sc_device_driver_func.o \
./src/sc_device_message_func.o \
./src/sc_func.o \
./src/socket_func.o \
./src/system_func.o \
./src/system_message_func.o \
./src/system_thread.o \
./src/tgs_device.o \
./src/tgs_device_client_data.o \
./src/tgs_device_data.o \
./src/tgs_device_driver_func.o \
./src/tgs_device_message_func.o \
./src/tgs_func.o \
./src/tlv.o \
./src/xml.o 

C_DEPS += \
./src/configuration.d \
./src/dc_device.d \
./src/dc_device_client_data.d \
./src/dc_device_data.d \
./src/dc_device_driver_func.d \
./src/dc_device_message_func.d \
./src/dc_func.d \
./src/fr_device.d \
./src/fr_device_client_data.d \
./src/fr_device_data.d \
./src/fr_device_driver_func.d \
./src/hardware_server.d \
./src/logger.d \
./src/md5.d \
./src/ppc_device.d \
./src/ppc_device_client_data.d \
./src/ppc_device_data.d \
./src/ppc_device_driver_func.d \
./src/ppc_device_message_func.d \
./src/ppc_func.d \
./src/sc_device.d \
./src/sc_device_client_data.d \
./src/sc_device_data.d \
./src/sc_device_driver_func.d \
./src/sc_device_message_func.d \
./src/sc_func.d \
./src/socket_func.d \
./src/system_func.d \
./src/system_message_func.d \
./src/system_thread.d \
./src/tgs_device.d \
./src/tgs_device_client_data.d \
./src/tgs_device_data.d \
./src/tgs_device_driver_func.d \
./src/tgs_device_message_func.d \
./src/tgs_func.d \
./src/tlv.d \
./src/xml.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/libxml2 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


