/* ------------------------------------------------------------------------------
  File: chr6dm_HMC.c
  Author: CH Robotics
  Version: 1.0
  
  Description: Functions for interacting with HMC5843 magnetic sensor
------------------------------------------------------------------------------ */ 

#include "stm32f10x.h"
#include "chr6dm_HMC.h"

uint8_t gHMC_Initialized = 0;

/*******************************************************************************
* Function Name  : initializeHMC
* Input          : None
* Output         : uint8_t* status_flag
* Return         : 1 if success, 0 if fail
* Description    : Sets the configuration registers of the HMC.  The registers
						 are set in an order that guarantees that the next register
						 accessed during a read will be 0x03 (i.e. getHMCData can be called
						 after initializeHMC is called.
						 
						 If the function succesfully initializes the HMC, then
						 the return value is 1.  If failed, then the function returns 0,
						 and status_flag is filled with one of the following error codes:
						 
						 HMC_REG0_FAILED => Couldn't set register 0
						 HMC_REG1_FAILED => Couldn't set register 1
						 HMC_REG2_FAILED => Couldn't set register 2
						 
*******************************************************************************/
int32_t initializeHMC( uint8_t* status_flag )
{
	 
	 uint8_t I2C1_Buffer_Tx[I2C_TX_BUFSIZE];
	 uint8_t I2C1_Buffer_Rx[I2C_RX_BUFSIZE];
	 
	 // Set mag. update frequency to 50 Hz
	 I2C1_Buffer_Tx[0] = 0x00;
	 I2C1_Buffer_Tx[1] = 0x18;		// 50 Hz
//	 I2C1_Buffer_Tx[1] = 0x04;	// 1 Hz
//	 I2C1_Buffer_Tx[1] = 0x08;	// 2 Hz
//	 I2C1_Buffer_Tx[1] = 0x0C;	// 5 Hz
//	 I2C1_Buffer_Tx[1] = 0x10;	// 10 Hz
	 
	 if( !i2cBufWrite( MAG_SLAVE_ADDRESS7, I2C1_Buffer_Tx, 2 ) )
	 {
		  *status_flag = HMC_REG0_FAILED;
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 // Read back value of register
	 I2C1_Buffer_Tx[0] = 0x00;
	 if( !i2cBufWrite( MAG_SLAVE_ADDRESS7, I2C1_Buffer_Tx, 1 ) )
	 {
		  *status_flag = HMC_REG0_FAILED;
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 i2cRead( MAG_SLAVE_ADDRESS7, I2C1_Buffer_Rx, 1 );
	 
	 if( I2C1_Buffer_Rx[0] != I2C1_Buffer_Tx[1] )
	 {
		  *status_flag = HMC_REG0_FAILED;
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 // Set gain setting on mag. sensor
	 I2C1_Buffer_Tx[0] = 0x01;
//	 I2C1_Buffer_Tx[1] = 0x04;		// +/- 3.2 Gauss
//	 I2C1_Buffer_Tx[1] = 0x03;		// +/- 2 Gauss
	 I2C1_Buffer_Tx[1] = 0x02;		// +/- 1.5 Gauss
	 
	 if( !i2cBufWrite( MAG_SLAVE_ADDRESS7, I2C1_Buffer_Tx, 2 ) )
	 {
		  *status_flag = HMC_REG2_FAILED;
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 // Read back value of register
	 I2C1_Buffer_Tx[0] = 0x01;
	 if( !i2cBufWrite( MAG_SLAVE_ADDRESS7, I2C1_Buffer_Tx, 1 ) )
	 {
		  *status_flag = HMC_REG2_FAILED;
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 i2cRead( MAG_SLAVE_ADDRESS7, I2C1_Buffer_Rx, 1 );
	 
	 if( I2C1_Buffer_Rx[0] != I2C1_Buffer_Tx[1] )
	 {
		  *status_flag = HMC_REG2_FAILED;
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 
	 // Set magnetometer to run in continuous mode
	 I2C1_Buffer_Tx[0] = 0x02;
	 I2C1_Buffer_Tx[1] = 0x00;
	 
	 if( !i2cBufWrite( MAG_SLAVE_ADDRESS7, I2C1_Buffer_Tx, 2 ) )
	 {
		  *status_flag = HMC_REG2_FAILED;
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 // Read back value of register
	 I2C1_Buffer_Tx[0] = 0x02;
	 if( !i2cBufWrite( MAG_SLAVE_ADDRESS7, I2C1_Buffer_Tx, 1 ) )
	 {
		  *status_flag = HMC_REG2_FAILED;
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 i2cRead( MAG_SLAVE_ADDRESS7, I2C1_Buffer_Rx, 1 );
	 
	 if( I2C1_Buffer_Rx[0] != I2C1_Buffer_Tx[1] )
	 {
		  *status_flag = HMC_REG2_FAILED;
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 gHMC_Initialized = 1;
	 
	 return 1;
}

/*******************************************************************************
* Function Name  : getHMCData
* Input          : None
* Output         : uint8_t* i2cBuf
* Return         : 1 if success, 0 if fail
* Description    : Fills i2cBuf with 7 bytes of data - the first 6 bytes are
						 the magnetic readings from the x, y, and z axes (respectively)
						 of the HMC5843, while the last byte is the HMC status register.
						 This function call assumes that the internal register address
						 pointer of the HMC is set to register 0x03.
*******************************************************************************/
int32_t getHMCData( uint8_t* i2cBuf )
{
	 int32_t returnval;
	 char statusBuffer[1];
	 
	 
	 // Get data from device
	 returnval = i2cRead( MAG_SLAVE_ADDRESS7, i2cBuf, 7 );

	 if( !returnval )
	 {
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 else
	 {
		  return 1;
	 }
}


/*******************************************************************************
* Function Name  : i2cBufWrite
* Input          : uint8_t addr, uint8_t* i2cData, uint8_t length
* Output         : None
* Return         : 1 if success, 0 if fail
* Description    : Writes 'length' bytes from i2cData[] to the i2c address 'addr'
						 This is a blocking function.
*******************************************************************************/
int32_t i2cBufWrite( uint8_t addr, uint8_t* i2cData, uint8_t length )
{
	 int32_t index;
	 	 
	 // Send START condition
	 I2C_GenerateSTART(I2C1, ENABLE);

	 // Wait for START condition to transmit
	 if( !i2cWaitForEvent(I2C_EVENT_MASTER_MODE_SELECT) )
	 {
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 // Send slave address
	 I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Transmitter);

	 // Wait for ACK
	 if( !i2cWaitForEvent(I2C_EVENT_MASTER_TRANSMITTER_MODE_SELECTED) )
	 {
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }

	 // Start transmitting data
	 for( index = 0; index < length; index++ )
	 {
		  // Send byte
		  I2C_SendData(I2C1, i2cData[index]);

		  // Wait for ACK
		  if( !i2cWaitForEvent(I2C_EVENT_MASTER_BYTE_TRANSMITTED) )
		  {
				I2C_GenerateSTOP(I2C1, ENABLE);
				return 0;
		  }
	 }
	 
	 /* Send STOP condition */
	 I2C_GenerateSTOP(I2C1, ENABLE);
	 
	 return 1;
}


/*******************************************************************************
* Function Name  : i2cRead
* Input          : uint8_t addr, uint8_t* i2cData, uint8_t length
* Output         : None
* Return         : 1 if success, 0 if fail
* Description    : Writes 'length' bytes from i2cData[] to the i2c address 'addr'
						 This is a blocking function.
*******************************************************************************/
int32_t i2cRead( uint8_t addr, uint8_t* i2cData, uint8_t bytesToRead )
{
	 uint8_t* pBuffer = i2cData;
	 int32_t retries;
	 
	 // Send START condition
	 I2C_GenerateSTART(I2C1, ENABLE);

	 // Wait for START condition to transmit
	 if( !i2cWaitForEvent(I2C_EVENT_MASTER_MODE_SELECT) )
	 {
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 // Send slave address
	 I2C_Send7bitAddress(I2C1, addr, I2C_Direction_Receiver);

	 // Wait for ACK
	 if( !i2cWaitForEvent(I2C_EVENT_MASTER_RECEIVER_MODE_SELECTED) )
	 {
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 // Start receiving data
	 retries = 0;
	 while(bytesToRead  && (retries < MAX_RETRIES))  
	 {
		  if(bytesToRead == 1)
		  {
				// Disable Acknowledgement
				I2C_AcknowledgeConfig(I2C1, DISABLE);

				// Send STOP Condition 
				I2C_GenerateSTOP(I2C1, ENABLE);
		  }

		  /* Test on EV7 and clear it */
		  if(I2C_CheckEvent(I2C1, I2C_EVENT_MASTER_BYTE_RECEIVED))  
		  {      
				// Read a byte from the EEPROM
				*pBuffer = I2C_ReceiveData(I2C1);

				/* Point to the next location where the byte read will be saved */
				pBuffer++; 

				/* Decrement the read bytes counter */
				bytesToRead--;
				
				retries = 0;
		  }
		  
		  retries++;
	 }
	 
	 if( retries >= MAX_RETRIES )
	 {
		  I2C_GenerateSTOP(I2C1, ENABLE);
		  return 0;
	 }
	 
	 
	 /* Enable Acknowledgement to be ready for another reception */
	 I2C_AcknowledgeConfig(I2C1, ENABLE);
	 
	 return 1;
}


/*******************************************************************************
* Function Name  : i2cWaitForEvent
* Input          : uint32_t event
* Output         : None
* Return         : 1 if success, 0 if fail
* Description    : Wrapper function for i2c functionality.  Checks for the specified
						 event I2C_ACK_ATTEMPTS times, and then errors out if the event
						 hasn't occured.
*******************************************************************************/
int i2cWaitForEvent( uint32_t event )
{
	 int32_t attempts;
	 
	 attempts = 0;
	 while(!I2C_CheckEvent(I2C1, event))
	 {
		  attempts++;
		  if( attempts > I2C_ACK_ATTEMPTS )
		  {
				return 0;
		  }		  
	 }
	 
	 return 1;
}
