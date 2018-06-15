################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/studease.cn/av/codec/stu_aac.c \
../src/studease.cn/av/codec/stu_avc.c \
../src/studease.cn/av/codec/stu_codec.c 

OBJS += \
./src/studease.cn/av/codec/stu_aac.o \
./src/studease.cn/av/codec/stu_avc.o \
./src/studease.cn/av/codec/stu_codec.o 

C_DEPS += \
./src/studease.cn/av/codec/stu_aac.d \
./src/studease.cn/av/codec/stu_avc.d \
./src/studease.cn/av/codec/stu_codec.d 


# Each subdirectory must supply rules for building sources it contributes
src/studease.cn/av/codec/%.o: ../src/studease.cn/av/codec/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/local/ssl/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<" -pthread -lm -march=x86-64
	@echo 'Finished building: $<'
	@echo ' '


