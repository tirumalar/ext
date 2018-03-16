################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../backend-loader.cpp \
../blob.cpp \
../connection-parameters.cpp \
../connection-pool.cpp \
../error.cpp \
../into-type.cpp \
../once-temp-type.cpp \
../prepare-temp-type.cpp \
../procedure.cpp \
../ref-counted-prepare-info.cpp \
../ref-counted-statement.cpp \
../row.cpp \
../rowid.cpp \
../session.cpp \
../soci-simple.cpp \
../statement.cpp \
../transaction.cpp \
../use-type.cpp \
../values.cpp 

OBJS += \
./backend-loader.o \
./blob.o \
./connection-parameters.o \
./connection-pool.o \
./error.o \
./into-type.o \
./once-temp-type.o \
./prepare-temp-type.o \
./procedure.o \
./ref-counted-prepare-info.o \
./ref-counted-statement.o \
./row.o \
./rowid.o \
./session.o \
./soci-simple.o \
./statement.o \
./transaction.o \
./use-type.o \
./values.o 

CPP_DEPS += \
./backend-loader.d \
./blob.d \
./connection-parameters.d \
./connection-pool.d \
./error.d \
./into-type.d \
./once-temp-type.d \
./prepare-temp-type.d \
./procedure.d \
./ref-counted-prepare-info.d \
./ref-counted-statement.d \
./row.d \
./rowid.d \
./session.d \
./soci-simple.d \
./statement.d \
./transaction.d \
./use-type.d \
./values.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -DSOCI_USE_BOOST -I"/home/eyelock/HBox/boost_1_53_0" -I"/home/eyelock/HBox/ws_HHH/linux/pkg/soci-3.2.1/soci" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


