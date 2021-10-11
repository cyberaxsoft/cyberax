################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/config.c \
../src/db.c \
../src/disp.c \
../src/logon.c \
../src/main_window.c \
../src/message_dlg.c \
../src/pos.c \
../src/tgs.c 

OBJS += \
./src/config.o \
./src/db.o \
./src/disp.o \
./src/logon.o \
./src/main_window.o \
./src/message_dlg.o \
./src/pos.o \
./src/tgs.o 

C_DEPS += \
./src/config.d \
./src/db.d \
./src/disp.d \
./src/logon.d \
./src/main_window.d \
./src/message_dlg.d \
./src/pos.d \
./src/tgs.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/include/gtk-2.0 -I/usr/include/harfbuzz/ -I/usr/include/postgresql/ -I/usr/include/atk-1.0 -I/usr/include/gdk-pixbuf-2.0/ -I/usr/include/gtk-2.0 -I/usr/include/pango-1.0 -I/usr/include/alsa/ -I/usr/include/cairo -I/usr/include/glib-2.0 -I/usr/lib/x86_64-linux-gnu/gtk-2.0/include/ -I/usr/include/libxml2 -I/usr/lib/x86_64-linux-gnu/glib-2.0/include/ -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


