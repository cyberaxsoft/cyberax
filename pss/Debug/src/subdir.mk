################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/hardware_serv.c \
../src/logger.c \
../src/pss.c \
../src/pss_client_data.c \
../src/pss_client_thread.c \
../src/pss_command_line.c \
../src/pss_data.c \
../src/pss_dc_device.c \
../src/pss_func.c \
../src/pss_parse.c \
../src/pss_port_thread.c \
../src/pss_ppc_device.c \
../src/pss_sc_device.c \
../src/pss_tgs_device.c \
../src/pss_tlv.c \
../src/pss_xml.c 

OBJS += \
./src/hardware_serv.o \
./src/logger.o \
./src/pss.o \
./src/pss_client_data.o \
./src/pss_client_thread.o \
./src/pss_command_line.o \
./src/pss_data.o \
./src/pss_dc_device.o \
./src/pss_func.o \
./src/pss_parse.o \
./src/pss_port_thread.o \
./src/pss_ppc_device.o \
./src/pss_sc_device.o \
./src/pss_tgs_device.o \
./src/pss_tlv.o \
./src/pss_xml.o 

C_DEPS += \
./src/hardware_serv.d \
./src/logger.d \
./src/pss.d \
./src/pss_client_data.d \
./src/pss_client_thread.d \
./src/pss_command_line.d \
./src/pss_data.d \
./src/pss_dc_device.d \
./src/pss_func.d \
./src/pss_parse.d \
./src/pss_port_thread.d \
./src/pss_ppc_device.d \
./src/pss_sc_device.d \
./src/pss_tgs_device.d \
./src/pss_tlv.d \
./src/pss_xml.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/libxml2 -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


