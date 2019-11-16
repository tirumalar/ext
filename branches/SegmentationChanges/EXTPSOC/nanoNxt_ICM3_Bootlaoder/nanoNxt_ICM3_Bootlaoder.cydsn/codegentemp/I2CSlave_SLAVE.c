/*******************************************************************************
* File Name: I2CSlave_SLAVE.c
* Version 3.30
*
* Description:
*  This file provides the source code of APIs for the I2C component Slave mode.
*
* Note:
*
*******************************************************************************
* Copyright 2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "I2CSlave_PVT.h"

#if(I2CSlave_MODE_SLAVE_ENABLED)

/**********************************
*      System variables
**********************************/

volatile uint8 I2CSlave_slStatus;   /* Slave Status  */

/* Transmit buffer variables */
volatile uint8 * I2CSlave_slRdBufPtr;   /* Pointer to Transmit buffer  */
volatile uint8   I2CSlave_slRdBufSize;  /* Slave Transmit buffer size  */
volatile uint8   I2CSlave_slRdBufIndex; /* Slave Transmit buffer Index */

/* Receive buffer variables */
volatile uint8 * I2CSlave_slWrBufPtr;   /* Pointer to Receive buffer  */
volatile uint8   I2CSlave_slWrBufSize;  /* Slave Receive buffer size  */
volatile uint8   I2CSlave_slWrBufIndex; /* Slave Receive buffer Index */

#if(I2CSlave_SW_ADRR_DECODE)
    volatile uint8 I2CSlave_slAddress;  /* Software address variable */
#endif   /* (I2CSlave_SW_ADRR_DECODE) */


/*******************************************************************************
* Function Name: I2CSlave_SlaveStatus
********************************************************************************
*
* Summary:
*  Returns I2C slave's communication status.
*
* Parameters:
*  None
*
* Return:
*  Current status of I2C slave.
*
* Global variables:
*  I2CSlave_slStatus  - used to store current status of I2C slave.
*
*******************************************************************************/
uint8 I2CSlave_SlaveStatus(void) 
{
    return(I2CSlave_slStatus);
}


/*******************************************************************************
* Function Name: I2CSlave_SlaveClearReadStatus
********************************************************************************
*
* Summary:
*  Clears the read status flags and returns they values.
*  The I2CSlave_SSTAT_RD_BUSY flag is not effected by clear.
*
* Parameters:
*  None
*
* Return:
*  Current read status of I2C slave.
*
* Global variables:
*  I2CSlave_slStatus  - used to store current status of I2C slave.
*
* Reentrant:
*  No
*
*******************************************************************************/
uint8 I2CSlave_SlaveClearReadStatus(void) 
{
    uint8 status;

    /* Mask of transfer complete flag and Error status */
    status = (I2CSlave_slStatus & I2CSlave_SSTAT_RD_MASK);
    I2CSlave_slStatus &= ((uint8) ~I2CSlave_SSTAT_RD_CLEAR);

    return(status);
}


/*******************************************************************************
* Function Name: I2CSlave_SlaveClearWriteStatus
********************************************************************************
*
* Summary:
*  Clears the write status flags and returns they values.
*  The I2CSlave_SSTAT_WR_BUSY flag is not effected by clear.
*
* Parameters:
*  None
*
* Return:
*  Current write status of I2C slave.
*
* Global variables:
*  I2CSlave_slStatus  - used to store current status of I2C slave.
*
* Reentrant:
*  No
*
*******************************************************************************/
uint8 I2CSlave_SlaveClearWriteStatus(void) 
{
    uint8 status;

    /* Mask of transfer complete flag and Error status */
    status = (I2CSlave_slStatus & I2CSlave_SSTAT_WR_MASK);
    I2CSlave_slStatus &= ((uint8) ~I2CSlave_SSTAT_WR_CLEAR);

    return(status);
}


/*******************************************************************************
* Function Name: I2CSlave_SlaveSetAddress
********************************************************************************
*
* Summary:
*  Sets the I2C slave address.
*
* Parameters:
*  address: I2C slave address for the primary device. This value may be any
*  address between 0 and 127.
*
* Return:
*  None
*
* Global variables:
*  I2CSlave_Address  - used to store I2C slave address for the primary
*  device when software address detect feature is used.
*
* Reentrant:
*  No
*
*******************************************************************************/
void I2CSlave_SlaveSetAddress(uint8 address) 
{
    #if(I2CSlave_SW_ADRR_DECODE)
        I2CSlave_slAddress = (address & I2CSlave_SLAVE_ADDR_MASK);
    #else
        I2CSlave_ADDR_REG  = (address & I2CSlave_SLAVE_ADDR_MASK);
    #endif /* (I2CSlave_SW_ADRR_DECODE) */
}


/*******************************************************************************
* Function Name: I2CSlave_SlaveInitReadBuf
********************************************************************************
*
* Summary:
*  Sets the buffer pointer and size of the read buffer. This function also
*  resets the transfer count returned with the I2C_SlaveGetReadBufSize function.
*
* Parameters:
*  readBuf:  Pointer to the data buffer to be read by the master.
*  bufSize:  Size of the read buffer exposed to the I2C master.
*
* Return:
*  None
*
* Global variables:
*  I2CSlave_slRdBufPtr   - used to store pointer to slave read buffer.
*  I2CSlave_slRdBufSize  - used to store salve read buffer size.
*  I2CSlave_slRdBufIndex - used to store current index within slave
*  read buffer.
*
* Side Effects:
*  If this function is called during a bus transaction, data from the previous
*  buffer location and the beginning of current buffer may be transmitted.
*
* Reentrant:
*  No
*
*******************************************************************************/
void I2CSlave_SlaveInitReadBuf(uint8 * rdBuf, uint8 bufSize)
     
{
    /* Check for proper buffer */
    if(NULL != rdBuf)
    {
        I2CSlave_slRdBufPtr   = (volatile uint8 *) rdBuf;    /* Set buffer pointer */
        I2CSlave_slRdBufSize  = bufSize;    /* Set buffer size */
        I2CSlave_slRdBufIndex = 0u;         /* Clears buffer index */
    }
}


/*******************************************************************************
* Function Name: I2CSlave_SlaveInitWriteBuf
********************************************************************************
*
* Summary:
*  Sets the buffer pointer and size of the read buffer. This function also
*  resets the transfer count returned with the I2C_SlaveGetReadBufSize function.
*
* Parameters:
*  writeBuf:  Pointer to the data buffer to be read by the master.
*  bufSize:  Size of the buffer exposed to the I2C master.
*
* Return:
*  None
*
* Global variables:
*  I2CSlave_slWrBufPtr   - used to store pointer to slave write buffer.
*  I2CSlave_slWrBufSize  - used to store salve write buffer size.
*  I2CSlave_slWrBufIndex - used to store current index within slave
*  write buffer.
*
* Side Effects:
*  If this function is called during a bus transaction, data from the previous
*  buffer location and the beginning of current buffer may be transmitted.
*
* Reentrant:
*  No
*
*******************************************************************************/
void I2CSlave_SlaveInitWriteBuf(uint8 * wrBuf, uint8 bufSize)
     
{
    /* Check for proper buffer */
    if(NULL != wrBuf)
    {
        I2CSlave_slWrBufPtr   = (volatile uint8 *) wrBuf;  /* Set buffer pointer */
        I2CSlave_slWrBufSize  = bufSize;   /* Set buffer size */
        I2CSlave_slWrBufIndex = 0u;        /* Clears buffer index */
    }
}


/*******************************************************************************
* Function Name: I2CSlave_SlaveGetReadBufSize
********************************************************************************
*
* Summary:
*  Returns the number of bytes read by the I2C master since an
*  I2C_SlaveInitReadBuf or I2C_SlaveClearReadBuf function was executed.
*  The maximum return value will be the size of the read buffer.
*
* Parameters:
*  None
*
* Return:
*  Bytes read by master.
*
* Global variables:
*  I2CSlave_slRdBufIndex - used to store current index within slave
*  read buffer.
*
*******************************************************************************/
uint8 I2CSlave_SlaveGetReadBufSize(void) 
{
    return(I2CSlave_slRdBufIndex);
}


/*******************************************************************************
* Function Name: I2CSlave_SlaveGetWriteBufSize
********************************************************************************
*
* Summary:
*  Returns the number of bytes written by the I2C master since an
*  I2C_SlaveInitWriteBuf or I2C_SlaveClearWriteBuf function was executed.
*  The maximum return value will be the size of the write buffer.
*
* Parameters:
*  None
*
* Return:
*  Bytes written by master.
*
* Global variables:
*  I2CSlave_slWrBufIndex - used to store current index within slave
*  write buffer.
*
*******************************************************************************/
uint8 I2CSlave_SlaveGetWriteBufSize(void) 
{
    return(I2CSlave_slWrBufIndex);
}


/*******************************************************************************
* Function Name: I2CSlave_SlaveClearReadBuf
********************************************************************************
*
* Summary:
*  Resets the read pointer to the first byte in the read buffer. The next byte
*  read by the master will be the first byte in the read buffer.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  I2CSlave_slRdBufIndex - used to store current index within slave
*  read buffer.
*
* Reentrant:
*  No
*
*******************************************************************************/
void I2CSlave_SlaveClearReadBuf(void) 
{
    I2CSlave_slRdBufIndex = 0u;
}


/*******************************************************************************
* Function Name: I2CSlave_SlaveClearRxBuf
********************************************************************************
*
* Summary:
*  Resets the write pointer to the first byte in the write buffer. The next byte
*  written by the master will be the first byte in the write buffer.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  I2CSlave_slWrBufIndex - used to store current index within slave
*  write buffer.
*
* Reentrant:
*  No
*
*******************************************************************************/
void I2CSlave_SlaveClearWriteBuf(void) 
{
    I2CSlave_slWrBufIndex = 0u;
}

#endif /* (I2CSlave_MODE_SLAVE_ENABLED) */


/* [] END OF FILE */
