/*******************************************************************************
* File Name: communication_api.h  
* Version 1.0
*
* Description:
* This is the header file for the communication module created by the author.  
* It contains function prototypes and constants for the users convenience. 
********************************************************************************/

//#include "I2C.h"
//#include "CyLib.h"

#define uint8 unsigned char
typedef     unsigned short uint16;


#include "cybtldr_utils.h"

/* Slave address for I2C used for bootloading.Replace this constant value with your
*  bootloader slave address project */
#define SLAVE_ADDR 4

/* Function declarations */
int OpenConnection(void);
int CloseConnection(void);
int ReadData(unsigned char* rdData, int byteCnt);
int WriteData(unsigned char* wrData, int byteCnt);



//[] END OF FILE

