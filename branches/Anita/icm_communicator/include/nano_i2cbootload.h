#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <linux/i2c-dev.h> 
#include <cybtldr_utils.h>
#include <cybtldr_api.h>
#include <cybtldr_api2.h>

#ifdef CMX_C1
#define I2C_BUS	"/dev/i2c-1"
#else
#define I2C_BUS	"/dev/i2c-3"
#endif

#define NANO_BOOTLOAD_I2C_ADDR 0x08
#define BOB_COMMAND_ADDRESS 0

void programPSOC(char* fileName,int debugFlag,int delayTime);


