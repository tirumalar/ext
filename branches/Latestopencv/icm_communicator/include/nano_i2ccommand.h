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


#ifdef CMX_C1
#define I2C_BUS	"/dev/i2c-1"
#else
#define I2C_BUS	"/dev/i2c-3"
#endif

#define NANO_BOB_I2C_ADDR 0x38
#define NANO_BOOTLOAD_I2C_ADDR 0x08
#define READER_DATA_SIZE	125 // 61 // 125

static char _i2c_bus[] = I2C_BUS;

void getSWVersion(char* version);
void getHWVersion(char* version);
void getMTBHWVersion(char* version);
void getMTBSWVersion(char* version);
void setBootload();
int clearReaderMemory();
int checkReaderReply();
int sendReaderData(unsigned char *data, int datalen, int lastpack);
int startBootloadProcess();
void dumpI2C();
void setI2C(int offset, int value);
void setI2CArray(int offset, char *datafile);
