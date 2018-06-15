################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/studease.cn/core/win32/stu_errno.c \
../src/studease.cn/core/win32/stu_file.c \
../src/studease.cn/core/win32/stu_os.c \
../src/studease.cn/core/win32/stu_process.c \
../src/studease.cn/core/win32/stu_shmem.c \
../src/studease.cn/core/win32/stu_socket.c \
../src/studease.cn/core/win32/stu_time.c 

OBJS += \
./src/studease.cn/core/win32/stu_errno.o \
./src/studease.cn/core/win32/stu_file.o \
./src/studease.cn/core/win32/stu_os.o \
./src/studease.cn/core/win32/stu_process.o \
./src/studease.cn/core/win32/stu_shmem.o \
./src/studease.cn/core/win32/stu_socket.o \
./src/studease.cn/core/win32/stu_time.o 

C_DEPS += \
./src/studease.cn/core/win32/stu_errno.d \
./src/studease.cn/core/win32/stu_file.d \
./src/studease.cn/core/win32/stu_os.d \
./src/studease.cn/core/win32/stu_process.d \
./src/studease.cn/core/win32/stu_shmem.d \
./src/studease.cn/core/win32/stu_socket.d \
./src/studease.cn/core/win32/stu_time.d 


# Each subdirectory must supply rules for building sources it contributes
src/studease.cn/core/win32/%.o: ../src/studease.cn/core/win32/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/local/ssl/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<" -pthread -lm -march=x86-64
	@echo 'Finished building: $<'
	@echo ' '


