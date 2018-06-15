################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/studease.cn/core/stu_alloc.c \
../src/studease.cn/core/stu_base64.c \
../src/studease.cn/core/stu_buf.c \
../src/studease.cn/core/stu_connection.c \
../src/studease.cn/core/stu_files.c \
../src/studease.cn/core/stu_flash.c \
../src/studease.cn/core/stu_hash.c \
../src/studease.cn/core/stu_hmac.c \
../src/studease.cn/core/stu_json.c \
../src/studease.cn/core/stu_list.c \
../src/studease.cn/core/stu_log.c \
../src/studease.cn/core/stu_mq.c \
../src/studease.cn/core/stu_palloc.c \
../src/studease.cn/core/stu_rbtree.c \
../src/studease.cn/core/stu_string.c \
../src/studease.cn/core/stu_thread.c \
../src/studease.cn/core/stu_timer.c \
../src/studease.cn/core/stu_times.c \
../src/studease.cn/core/stu_upstream.c 

OBJS += \
./src/studease.cn/core/stu_alloc.o \
./src/studease.cn/core/stu_base64.o \
./src/studease.cn/core/stu_buf.o \
./src/studease.cn/core/stu_connection.o \
./src/studease.cn/core/stu_files.o \
./src/studease.cn/core/stu_flash.o \
./src/studease.cn/core/stu_hash.o \
./src/studease.cn/core/stu_hmac.o \
./src/studease.cn/core/stu_json.o \
./src/studease.cn/core/stu_list.o \
./src/studease.cn/core/stu_log.o \
./src/studease.cn/core/stu_mq.o \
./src/studease.cn/core/stu_palloc.o \
./src/studease.cn/core/stu_rbtree.o \
./src/studease.cn/core/stu_string.o \
./src/studease.cn/core/stu_thread.o \
./src/studease.cn/core/stu_timer.o \
./src/studease.cn/core/stu_times.o \
./src/studease.cn/core/stu_upstream.o 

C_DEPS += \
./src/studease.cn/core/stu_alloc.d \
./src/studease.cn/core/stu_base64.d \
./src/studease.cn/core/stu_buf.d \
./src/studease.cn/core/stu_connection.d \
./src/studease.cn/core/stu_files.d \
./src/studease.cn/core/stu_flash.d \
./src/studease.cn/core/stu_hash.d \
./src/studease.cn/core/stu_hmac.d \
./src/studease.cn/core/stu_json.d \
./src/studease.cn/core/stu_list.d \
./src/studease.cn/core/stu_log.d \
./src/studease.cn/core/stu_mq.d \
./src/studease.cn/core/stu_palloc.d \
./src/studease.cn/core/stu_rbtree.d \
./src/studease.cn/core/stu_string.d \
./src/studease.cn/core/stu_thread.d \
./src/studease.cn/core/stu_timer.d \
./src/studease.cn/core/stu_times.d \
./src/studease.cn/core/stu_upstream.d 


# Each subdirectory must supply rules for building sources it contributes
src/studease.cn/core/%.o: ../src/studease.cn/core/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/local/ssl/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<" -pthread -lm -march=x86-64
	@echo 'Finished building: $<'
	@echo ' '


