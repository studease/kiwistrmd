################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/kiwistrmd.com/core/ksd_conf.c \
../src/kiwistrmd.com/core/ksd_cycle.c \
../src/kiwistrmd.com/core/ksd_process.c \
../src/kiwistrmd.com/core/ksd_request.c 

OBJS += \
./src/kiwistrmd.com/core/ksd_conf.o \
./src/kiwistrmd.com/core/ksd_cycle.o \
./src/kiwistrmd.com/core/ksd_process.o \
./src/kiwistrmd.com/core/ksd_request.o 

C_DEPS += \
./src/kiwistrmd.com/core/ksd_conf.d \
./src/kiwistrmd.com/core/ksd_cycle.d \
./src/kiwistrmd.com/core/ksd_process.d \
./src/kiwistrmd.com/core/ksd_request.d 


# Each subdirectory must supply rules for building sources it contributes
src/kiwistrmd.com/core/%.o: ../src/kiwistrmd.com/core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/local/ssl/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<" -pthread -lm -march=x86-64
	@echo 'Finished building: $<'
	@echo ' '


