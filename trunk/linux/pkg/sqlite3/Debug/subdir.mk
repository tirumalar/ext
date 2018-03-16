################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../sqlite3.c 

OBJS += \
./sqlite3.o 

C_DEPS += \
./sqlite3.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -DSQLITE_HAS_CODEC -DSQLITE_OS_UNIX=1 -D_HAVE_SQLITE_CONFIG_H -DBUILD_sqlite -DSQLITE_THREADSAFE=1 -DSQLITE_OMIT_LOAD_EXTENSION=1 -DSQLITE_TEMP_STORE=2 -I"/home/eyelock/HBox/openssl/include" -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


