################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/config.c \
../src/driver_state.c \
../src/logger.c \
../src/pdex.c \
../src/pdex_frames.c \
../src/pdex_func.c \
../src/read_thread.c \
../src/uart.c \
../src/write_thread.c 

OBJS += \
./src/config.o \
./src/driver_state.o \
./src/logger.o \
./src/pdex.o \
./src/pdex_frames.o \
./src/pdex_func.o \
./src/read_thread.o \
./src/uart.o \
./src/write_thread.o 

C_DEPS += \
./src/config.d \
./src/driver_state.d \
./src/logger.d \
./src/pdex.d \
./src/pdex_frames.d \
./src/pdex_func.d \
./src/read_thread.d \
./src/uart.d \
./src/write_thread.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c src/subdir.mk
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/glib-2.0 -I/usr/include/libxml2 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ -O0 -g3 -Wall -c -fmessage-length=0 -fPIC -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


