/* ------------------------------------------------------------------------------
  File: chr6dm_HMC.h
  Author: CH Robotics
  Version: 1.0
  
  Description: Preprocessor definitions and function declarations for chr6dm_HMC.c
------------------------------------------------------------------------------ */ 

#ifndef	__CHR6DM_HMC
#define __CHR6DM_HMC

// i2c function calls
int32_t i2cBufWrite( uint8_t addr, uint8_t* i2cData, uint8_t length );
int32_t i2cRead( uint8_t addr, uint8_t* i2cData, uint8_t length );

// HMC5843 function calls
int32_t initializeHMC( uint8_t* status_flag );
int32_t getHMCData( uint8_t* I2C1_Buffer_Rx );

#define	I2C_TRANSMITTER 		0
#define	I2C_RECEIVER			1
#define	MAG_SLAVE_ADDRESS7	0x3C

#define	I2C_TX_BUFSIZE			10
#define	I2C_RX_BUFSIZE			10

#define	I2C_ACK_ATTEMPTS		5000
#define	MAX_RETRIES				10000

#define 	HMC_REG0_FAILED 		1
#define 	HMC_REG1_FAILED		2
#define 	HMC_REG2_FAILED		3	

extern uint8_t gHMC_Initialized;

#endif
