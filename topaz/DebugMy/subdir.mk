################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../dart.c \
../dart_conf.c \
../dart_func.c \
../logger.c \
../uart.c 

OBJS += \
./dart.o \
./dart_conf.o \
./dart_func.o \
./logger.o \
./uart.o 

C_DEPS += \
./dart.d \
./dart_conf.d \
./dart_func.d \
./logger.d \
./uart.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/glib-2.0 -I/usr/include/libxml2 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


