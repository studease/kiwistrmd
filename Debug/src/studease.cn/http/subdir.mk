################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/studease.cn/http/stu_http.c \
../src/studease.cn/http/stu_http_filter.c \
../src/studease.cn/http/stu_http_header.c \
../src/studease.cn/http/stu_http_parse.c \
../src/studease.cn/http/stu_http_phase.c \
../src/studease.cn/http/stu_http_request.c \
../src/studease.cn/http/stu_http_status.c \
../src/studease.cn/http/stu_http_upstream.c 

OBJS += \
./src/studease.cn/http/stu_http.o \
./src/studease.cn/http/stu_http_filter.o \
./src/studease.cn/http/stu_http_header.o \
./src/studease.cn/http/stu_http_parse.o \
./src/studease.cn/http/stu_http_phase.o \
./src/studease.cn/http/stu_http_request.o \
./src/studease.cn/http/stu_http_status.o \
./src/studease.cn/http/stu_http_upstream.o 

C_DEPS += \
./src/studease.cn/http/stu_http.d \
./src/studease.cn/http/stu_http_filter.d \
./src/studease.cn/http/stu_http_header.d \
./src/studease.cn/http/stu_http_parse.d \
./src/studease.cn/http/stu_http_phase.d \
./src/studease.cn/http/stu_http_request.d \
./src/studease.cn/http/stu_http_status.d \
./src/studease.cn/http/stu_http_upstream.d 


# Each subdirectory must supply rules for building sources it contributes
src/studease.cn/http/%.o: ../src/studease.cn/http/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/local/ssl/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<" -pthread -lm -march=x86-64
	@echo 'Finished building: $<'
	@echo ' '


