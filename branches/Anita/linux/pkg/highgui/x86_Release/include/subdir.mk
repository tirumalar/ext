################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
CPP_SRCS += \
../include/grfmt_bmp.cpp 

OBJS += \
./include/grfmt_bmp.o 

CPP_DEPS += \
./include/grfmt_bmp.d 


# Each subdirectory must supply rules for building sources it contributes
include/%.o: ../include/%.cpp
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C++ Compiler'
	g++ -I"/home/mamigo/ws_nanoNXT/linux/pkg/cv/include" -I"/home/mamigo/ws_nanoNXT/linux/pkg/cxcore/include" -I"/home/mamigo/ws_nanoNXT/linux/pkg/highgui/include" -O3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


