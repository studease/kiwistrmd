################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/studease.cn/rtmp/stu_rtmp.c \
../src/studease.cn/rtmp/stu_rtmp_amf.c \
../src/studease.cn/rtmp/stu_rtmp_application.c \
../src/studease.cn/rtmp/stu_rtmp_chunk.c \
../src/studease.cn/rtmp/stu_rtmp_filter.c \
../src/studease.cn/rtmp/stu_rtmp_handshaker.c \
../src/studease.cn/rtmp/stu_rtmp_instance.c \
../src/studease.cn/rtmp/stu_rtmp_message.c \
../src/studease.cn/rtmp/stu_rtmp_netconnection.c \
../src/studease.cn/rtmp/stu_rtmp_netstream.c \
../src/studease.cn/rtmp/stu_rtmp_parse.c \
../src/studease.cn/rtmp/stu_rtmp_phase.c \
../src/studease.cn/rtmp/stu_rtmp_phase_flv.c \
../src/studease.cn/rtmp/stu_rtmp_phase_fmp4.c \
../src/studease.cn/rtmp/stu_rtmp_request.c \
../src/studease.cn/rtmp/stu_rtmp_responder.c \
../src/studease.cn/rtmp/stu_rtmp_stream.c \
../src/studease.cn/rtmp/stu_rtmp_upstream.c 

OBJS += \
./src/studease.cn/rtmp/stu_rtmp.o \
./src/studease.cn/rtmp/stu_rtmp_amf.o \
./src/studease.cn/rtmp/stu_rtmp_application.o \
./src/studease.cn/rtmp/stu_rtmp_chunk.o \
./src/studease.cn/rtmp/stu_rtmp_filter.o \
./src/studease.cn/rtmp/stu_rtmp_handshaker.o \
./src/studease.cn/rtmp/stu_rtmp_instance.o \
./src/studease.cn/rtmp/stu_rtmp_message.o \
./src/studease.cn/rtmp/stu_rtmp_netconnection.o \
./src/studease.cn/rtmp/stu_rtmp_netstream.o \
./src/studease.cn/rtmp/stu_rtmp_parse.o \
./src/studease.cn/rtmp/stu_rtmp_phase.o \
./src/studease.cn/rtmp/stu_rtmp_phase_flv.o \
./src/studease.cn/rtmp/stu_rtmp_phase_fmp4.o \
./src/studease.cn/rtmp/stu_rtmp_request.o \
./src/studease.cn/rtmp/stu_rtmp_responder.o \
./src/studease.cn/rtmp/stu_rtmp_stream.o \
./src/studease.cn/rtmp/stu_rtmp_upstream.o 

C_DEPS += \
./src/studease.cn/rtmp/stu_rtmp.d \
./src/studease.cn/rtmp/stu_rtmp_amf.d \
./src/studease.cn/rtmp/stu_rtmp_application.d \
./src/studease.cn/rtmp/stu_rtmp_chunk.d \
./src/studease.cn/rtmp/stu_rtmp_filter.d \
./src/studease.cn/rtmp/stu_rtmp_handshaker.d \
./src/studease.cn/rtmp/stu_rtmp_instance.d \
./src/studease.cn/rtmp/stu_rtmp_message.d \
./src/studease.cn/rtmp/stu_rtmp_netconnection.d \
./src/studease.cn/rtmp/stu_rtmp_netstream.d \
./src/studease.cn/rtmp/stu_rtmp_parse.d \
./src/studease.cn/rtmp/stu_rtmp_phase.d \
./src/studease.cn/rtmp/stu_rtmp_phase_flv.d \
./src/studease.cn/rtmp/stu_rtmp_phase_fmp4.d \
./src/studease.cn/rtmp/stu_rtmp_request.d \
./src/studease.cn/rtmp/stu_rtmp_responder.d \
./src/studease.cn/rtmp/stu_rtmp_stream.d \
./src/studease.cn/rtmp/stu_rtmp_upstream.d 


# Each subdirectory must supply rules for building sources it contributes
src/studease.cn/rtmp/%.o: ../src/studease.cn/rtmp/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -I/usr/local/ssl/include -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -o "$@" "$<" -pthread -lm -march=x86-64
	@echo 'Finished building: $<'
	@echo ' '


