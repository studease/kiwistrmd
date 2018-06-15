################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/studease.cn/websocket/stu_websocket.c \
../src/studease.cn/websocket/stu_websocket_filter.c \
../src/studease.cn/websocket/stu_websocket_frame.c \
../src/studease.cn/websocket/stu_websocket_phase.c \
../src/studease.cn/websocket/stu_websocket_request.c 

OBJS += \
./src/studease.cn/websocket/stu_websocket.o \
./src/studease.cn/websocket/stu_websocket_filter.o \
./src/studease.cn/websocket/stu_websocket_frame.o \
./src/studease.cn/websocket/stu_websocket_phase.o \
./src/studease.cn/websocket/stu_websocket_request.o 

C_DEPS += \
./src/studease.cn/websocket/stu_websocket.d \
./src/studease.cn/websocket/stu_websocket_filter.d \
./src/studease.cn/websocket/stu_websocket_frame.d \
./src/studease.cn/websocket/stu_websocket_phase.d \
./src/studease.cn/websocket/stu_websocket_request.d 


# Each subdirectory must supply rules for building sources it contributes
src/studease.cn/websocket/%.o: ../src/studease.cn/websocket/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/local/ssl/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<" -pthread -lm -march=x86-64
	@echo 'Finished building: $<'
	@echo ' '


