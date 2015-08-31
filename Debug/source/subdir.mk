################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../source/aio_x.c \
../source/data.c \
../source/dbconnpool.c \
../source/dbcore.c \
../source/ini.c \
../source/log.c \
../source/main.c \
../source/memory.c \
../source/queue.c \
../source/servermanage.c \
../source/socket.c \
../source/thread.c \
../source/threadpool.c 

OBJS += \
./source/aio_x.o \
./source/data.o \
./source/dbconnpool.o \
./source/dbcore.o \
./source/ini.o \
./source/log.o \
./source/main.o \
./source/memory.o \
./source/queue.o \
./source/servermanage.o \
./source/socket.o \
./source/thread.o \
./source/threadpool.o 

C_DEPS += \
./source/aio_x.d \
./source/data.d \
./source/dbconnpool.d \
./source/dbcore.d \
./source/ini.d \
./source/log.d \
./source/main.d \
./source/memory.d \
./source/queue.d \
./source/servermanage.d \
./source/socket.d \
./source/thread.d \
./source/threadpool.d 


# Each subdirectory must supply rules for building sources it contributes
source/%.o: ../source/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


