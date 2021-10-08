################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/logger.c \
../src/port_conf.c \
../src/port_func.c \
../src/port_router.c \
../src/port_router_data.c 

OBJS += \
./src/logger.o \
./src/port_conf.o \
./src/port_func.o \
./src/port_router.o \
./src/port_router_data.o 

C_DEPS += \
./src/logger.d \
./src/port_conf.d \
./src/port_func.d \
./src/port_router.d \
./src/port_router_data.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/glib-2.0 -I/usr/include/hidapi -I/usr/include/libxml2 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


