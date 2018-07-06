################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../blob.cpp \
../common.cpp \
../factory.cpp \
../row-id.cpp \
../session.cpp \
../standard-into-type.cpp \
../standard-use-type.cpp \
../statement.cpp \
../vector-into-type.cpp \
../vector-use-type.cpp 

OBJS += \
./blob.o \
./common.o \
./factory.o \
./row-id.o \
./session.o \
./standard-into-type.o \
./standard-use-type.o \
./statement.o \
./vector-into-type.o \
./vector-use-type.o 

CPP_DEPS += \
./blob.d \
./common.d \
./factory.d \
./row-id.d \
./session.d \
./standard-into-type.d \
./standard-use-type.d \
./statement.d \
./vector-into-type.d \
./vector-use-type.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DSOCI_USE_BOOST -I"/home/eyelock/HBox/ws_HHH/linux/pkg/sqlite3" -I"/home/eyelock/HBox/ws_HHH/linux/pkg/soci-3.2.1/soci" -I"/home/eyelock/HBox/ws_HHH/linux/pkg/soci-3.2.1/soci/sqlite3" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


