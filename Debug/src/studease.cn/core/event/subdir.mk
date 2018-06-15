################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/studease.cn/core/event/stu_event.c \
../src/studease.cn/core/event/stu_event_epoll.c 

OBJS += \
./src/studease.cn/core/event/stu_event.o \
./src/studease.cn/core/event/stu_event_epoll.o 

C_DEPS += \
./src/studease.cn/core/event/stu_event.d \
./src/studease.cn/core/event/stu_event_epoll.d 


# Each subdirectory must supply rules for building sources it contributes
src/studease.cn/core/event/%.o: ../src/studease.cn/core/event/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/local/ssl/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<" -pthread -lm -march=x86-64
	@echo 'Finished building: $<'
	@echo ' '


