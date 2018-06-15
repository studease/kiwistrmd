################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/studease.cn/av/format/stu_flv.c \
../src/studease.cn/av/format/stu_mp4.c 

OBJS += \
./src/studease.cn/av/format/stu_flv.o \
./src/studease.cn/av/format/stu_mp4.o 

C_DEPS += \
./src/studease.cn/av/format/stu_flv.d \
./src/studease.cn/av/format/stu_mp4.d 


# Each subdirectory must supply rules for building sources it contributes
src/studease.cn/av/format/%.o: ../src/studease.cn/av/format/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/local/ssl/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<" -pthread -lm -march=x86-64
	@echo 'Finished building: $<'
	@echo ' '


