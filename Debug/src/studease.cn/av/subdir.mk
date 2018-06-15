################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/studease.cn/av/stu_av.c 

OBJS += \
./src/studease.cn/av/stu_av.o 

C_DEPS += \
./src/studease.cn/av/stu_av.d 


# Each subdirectory must supply rules for building sources it contributes
src/studease.cn/av/%.o: ../src/studease.cn/av/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/local/ssl/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<" -pthread -lm -march=x86-64
	@echo 'Finished building: $<'
	@echo ' '


