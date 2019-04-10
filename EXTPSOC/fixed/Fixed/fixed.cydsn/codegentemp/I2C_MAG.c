/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/

#include "I2C_MAG.h"
#include "CyLib.h"
  
static uint32 bitCounter;
static uint32 byteCounter;
static uint32 packetSize;
static uint32 masterStatus;
static uint32 masterControl;
static uint32 masterState = I2C_MAG_STATE_IDLE;

static uint8 *outBuffer;
static uint8 *inBuffer;
static uint8 theByte;

static uint32 internalAddress;

/*******************************************************************************
* Function Name: I2C_MAG_WriteBuf
********************************************************************************
*
* Summary:
*  Automatically writes an entire buffer of data to a slave device.
*  This function is non-blocking. The Process() API should be called periodically
*  to complete the transmission. If the frame cannot be started, this function
*  returns an error.
*
* Parameters:
*  slaveAddr: 7-bit slave address.
*  wrData:    Pointer to buffer of data to be sent.
*  cnt:       Size of buffer to send.
*  mode:      Transfer mode defines: start or restart condition generation at
*             begin of the transfer and complete the transfer or halt before
*             generating a stop.
*
* Return:
*  - I2C_MAG_I2C_MSTR_NO_ERROR - Function complete without error. 
*    The master started the transfer.
*  - I2C_MAG_I2C_MSTR_BUS_BUSY - Bus is busy. Nothing was sent on
*    the bus. The attempt has to be retried.
*  - I2C_MAG_I2C_MSTR_NOT_READY - Master is not ready for to start
*    transfer. A master still has not completed previous transaction or a 
*    slave operation is in progress (in multi-master-slave configuration).
*    Nothing was sent on the bus. The attempt has to be retried.
*******************************************************************************/
uint32 I2C_MAG_WriteBuf(uint32 slaveAddress, uint8 * wrData, uint32 cnt, uint32 mode)
{
    uint32 ret = I2C_MAG_MSTR_NO_ERROR;
    
    /* Only process the command if the bus is not busy */
    if ((masterStatus & I2C_MAG_MSTAT_XFER_INP) == 0)
    {
        /* Check if SDA and SCL are HIGH */
        if (GetSDA && GetSCL)
        {
            /* Set the control to start a frame based on the mode */
            masterControl = mode | I2C_MAG_WRITE_FRAME;
            
            /* Set the number of bytes to be sent */
            packetSize = cnt;
            
            /* Reset the internal counters */
            bitCounter = 0;
            byteCounter = 0;
                        
            /* Set the pointer to the correct buffer */
            outBuffer = wrData;
            
            /* Store the I2C slave address as 8-bit + read/write bit */
            internalAddress = (slaveAddress << 1);      
            
            /* Clear Master status */
            masterStatus = 0;
        }
        else
        {
            /* Bus is busy, return error */
            ret = I2C_MAG_MSTR_BUS_BUSY;
        }
    }
    else
    {
        /* Transmission in progress, return error */
        ret = I2C_MAG_MSTR_NOT_READY;
    }
    
    return ret;
}

/*******************************************************************************
* Function Name: I2C_MAG_ReadBuf
********************************************************************************
*
* Summary:
*  Automatically reads an entire buffer of data from a slave device.
*  This function is non-blocking. The Process() API should be called periodically
*  to complete the transmission. If the frame cannot be started, this function
*  returns an error.
*
* Parameters:
*  slaveAddr: 7-bit slave address.
*  rdData:    Pointer to buffer where to put data from slave.
*  cnt:       Size of buffer to read.
*  mode:      Transfer mode defines: start or restart condition generation at
*             begin of the transfer and complete the transfer or halt before
*             generating a stop.
*
* Return:
*  - I2C_MAG_I2C_MSTR_NO_ERROR - Function complete without error. 
*    The master started the transfer.
*  - I2C_MAG_I2C_MSTR_BUS_BUSY - Bus is busy. Nothing was sent on
*    the bus. The attempt has to be retried.
*  - I2C_MAG_I2C_MSTR_NOT_READY - Master is not ready for to start
*    transfer. A master still has not completed previous transaction or a 
*    slave operation is in progress (in multi-master-slave configuration).
*    Nothing was sent on the bus. The attempt has to be retried.
*******************************************************************************/
uint32 I2C_MAG_ReadBuf(uint32 slaveAddress, uint8 * rdData, uint32 cnt, uint32 mode)
{
    uint32 ret = I2C_MAG_MSTR_NO_ERROR;
    
    /* Only process the command if the bus is not busy */
    if ((masterStatus & I2C_MAG_MSTAT_XFER_INP) == 0)
    {
        /* Check if SDA and SCL are HIGH */
        if (GetSDA && GetSCL)
        {
            /* Set the control to start a frame based on the mode */
            masterControl = mode | I2C_MAG_READ_FRAME;
            
            /* Set the number of bytes to be sent */
            packetSize = cnt;
            
            /* Reset the internal counters */
            bitCounter = 0;
            byteCounter = 0;
                        
            /* Set the pointer to the correct buffer */
            inBuffer = rdData;
            
            /* Store the I2C slave address as 8-bit + read/write bit */
            internalAddress = (slaveAddress << 1) | 0x01;        
            
            /* Clear Master status */
            masterStatus = 0;
        }
        else
        {
            /* Bus is busy, return error */
            ret = I2C_MAG_MSTR_BUS_BUSY;
        }
    }
    else
    {
        /* Transmission in progress, return error */
        ret = I2C_MAG_MSTR_NOT_READY;
    }
    
    return ret;
}

/*******************************************************************************
* Function Name: I2C_MAG_SendStart
********************************************************************************
*
* Summary:
*  Generates Start condition and sends slave address with read/write bit.
*  This function is blocking and does not return until start condition and
*  address byte are sent and ACK/NACK response is received or errors occurred.
*
* Parameters:
*  slaveAddress: Right justified 8-bit Slave address with R/W LSB bit
*
* Return:
*  - I2C_MAG_I2C_MSTR_NO_ERROR - Function complete without error. 
*    The master started the transfer.
*  - I2C_MAG_I2C_MSTR_BUS_BUSY - Bus is busy. Nothing was sent on
*    the bus. The attempt has to be retried.
*  - I2C_MAG_I2C_MSTR_NOT_READY - Master is not ready for to start
*    transfer. A master still has not completed previous transaction or a 
*    slave operation is in progress (in multi-master-slave configuration).
*    Nothing was sent on the bus. The attempt has to be retried.
*******************************************************************************/
uint32 I2C_MAG_SendStart(uint32 slaveAddress_RW)
{
    uint32 ret;
    
    /* Only process the command if the bus is not busy */
    if ((masterStatus & I2C_MAG_MSTAT_XFER_INP) == 0)
    {
        /* Check if SDA and SCL are HIGH */
        if (GetSDA && GetSCL)
        {
            /* Set the control to start a frame based on the mode */
            masterControl = I2C_MAG_MODE_NO_STOP | I2C_MAG_WRITE_FRAME;
            
            /* Set the number of bytes to be sent */
            packetSize = 0;
            
            /* Reset the internal counters */
            bitCounter = 0;
            byteCounter = 0;
            
            /* Store the I2C slave address as 8-bit + read/write bit */
            internalAddress = slaveAddress_RW;     
            
            /* Clear Master status */
            masterStatus = 0;
        }
        else
        {
            /* Bus is busy, return error */
            ret = I2C_MAG_MSTR_BUS_BUSY;
        }
    }
    else
    {
        /* Transmission in progress, return error */
        ret = I2C_MAG_MSTR_NOT_READY;
    }
    
    return ret;
}

/*******************************************************************************
* Function Name: I2C_MAG_SendStop
********************************************************************************
*
* Summary:
*  Generates Stop condition on the bus.
*  At least one byte has to be read if start or restart condition with read
*  direction was generated before.
*  This function is blocking and does not return until a stop condition
*  is generated or error occurred.
*
* Parameters:
*  None
*
* Return:
*  Error status
*
* Side Effects:
*  A valid Start or ReStart condition shall be generated before calling
*  this function. 
*
*******************************************************************************/
uint32 I2C_MAG_SendStop(void)
{
    ClrSCL;
    ClrSDA;

    masterState = I2C_MAG_STATE_STOP;
    
    return I2C_MAG_MSTR_NO_ERROR;
}

/*******************************************************************************
* Function Name: I2C_MAG_Process
********************************************************************************
*
* Summary:
*  Process one step of the I2C transmission. This API should be called 
*  constantly, so the transmission can proceed. It returns the state of the 
*  transmission.
*
* Return:
*  State of the transmission.
*  
*******************************************************************************/
uint32 I2C_MAG_Process(void)
{    
    /* Run the I2C state machine */
    switch (masterState)
    {
        case I2C_MAG_STATE_IDLE:
            /* Check if a request to send data was received */
            if (masterControl)
            {
                /* Drive I2C lines high */
                SetSDA;
                SetSCL;
                                                
                /* Update the status */
                masterStatus = I2C_MAG_MSTAT_XFER_INP;
                
                /* Move to the next state */
                masterState = I2C_MAG_STATE_START;
            }

            break;
            
        case I2C_MAG_STATE_START:
            /* Check if the I2C bus is busy */
            if ((GetSDA) && (GetSCL))
            {
                /* Start bit. Drive Data Line Low */
                ClrSDA;
                
                /* Move to the next state */
                masterState = I2C_MAG_STATE_START_CLK;
            }
            else
            {
                /* Bus is busy, finish the transmission */
                if (masterControl & I2C_MAG_WRITE_FRAME)
                {
                    masterStatus = I2C_MAG_MSTAT_WR_CMPLT;
                }
                else
                {
                    masterStatus = I2C_MAG_MSTAT_RD_CMPLT;
                }
                
                /* Clear busy bit */
                masterStatus &= ~(I2C_MAG_MSTAT_XFER_INP);
                
                masterStatus |= I2C_MAG_MSTAT_ERR_BUS_ERROR;
                
                /* Clear master control */
                masterControl = 0;
                
                /* Go to IDLE state */
                masterState = I2C_MAG_STATE_IDLE;
            }
            break;
            
        case I2C_MAG_STATE_START_CLK:
            /* Start bit. Drive Clock Line low */
            ClrSCL;
            
            /* Move to the next state */
            masterState = I2C_MAG_STATE_ADDRESS;
            break;
            
        case I2C_MAG_STATE_ADDRESS:
            if (bitCounter < 8)
            {
                /* Process the slave addresses bits */
                if (internalAddress & 0x80)
                {
                    SetSDA;
                }
                else
                {
                    ClrSDA;
                }
                /* Shift the address to prepare next bit */
                internalAddress <<= 1;
                
                /* Clock SCL */
                SetSCL;
                
                /* Increment bit counter */
                bitCounter++;
                
                /* Move the next state */
                masterState = I2C_MAG_STATE_ADDRESS_CLK;
            }
            else
            {
                /* Set SDA high to check if NAK */
                SetSDA;
                
                /* Set Initial address ack bit */
                masterControl |= I2C_MAG_INITIAL_ADDR_ACK;
                
                /* Move to the next state */
                masterState = I2C_MAG_STATE_ACK;
            }
            break;
             
        case I2C_MAG_STATE_ADDRESS_CLK:
            /* Drives clock Low */
            ClrSCL;
            
            /* Go back to the previous address */
            masterState = I2C_MAG_STATE_ADDRESS;
            break;
            
        case I2C_MAG_STATE_ACK:
            /* Set CLK high */
            SetSCL;
            
            /* Check if it is a write command */
            if ((masterControl & I2C_MAG_WRITE_FRAME) ||
                 masterControl & I2C_MAG_INITIAL_ADDR_ACK)
            {                           
                /* Check if slave is stretching the clock */
                if (GetSCL == 0)
                {
                    /* Stay here, slave is stretching the clock */
                }
                else
                {
                    /* Clear initial address ack bit */
                    masterControl &= ~(I2C_MAG_INITIAL_ADDR_ACK);
                    
                    /* Check if slave acked */
                    if (GetSDA)
                    {
                        /* Slave didn't ACK, go to the NACK state */
                        masterState = I2C_MAG_STATE_NACK;
                    }
                    else
                    {
                        /* Slave ACKed, go to the next state */
                        masterState = I2C_MAG_STATE_ACK_CLK;
                    }
                }
            }
            else /* Read command */
            {
                /* Go to the next state */
                masterState = I2C_MAG_STATE_ACK_CLK;
            }
            break;
            
        case I2C_MAG_STATE_ACK_CLK:
            /* Set Clock Low */
            ClrSCL;
            
            /* Check if any data to be transmitted */
            if (byteCounter == packetSize)
            {   
                /* Check if should send a stop */
                if (masterControl & I2C_MAG_MODE_NO_STOP)
                {                    
                    /* Go to NO STOP state */
                    masterState = I2C_MAG_STATE_NO_STOP;
                }
                else
                {
                    /* Set SDA Low */
                    ClrSDA;
                    
                    /* Go to the stop state */
                    masterState = I2C_MAG_STATE_STOP;
                }    
            }
            else
            {              
                /* Reset the bit counter */
                bitCounter = 0;
                
                /* Reset the read byte */
                theByte = 0;
                
                /* Go to the next state */
                masterState = I2C_MAG_STATE_DATA;
            }
            break;
            
        case I2C_MAG_STATE_NACK:
            /* Set SCL and SDA to LOW */
            ClrSCL;     
            ClrSDA;
    
            /* Return error */
            masterStatus |= I2C_MAG_MSTAT_ERR_ADDR_NAK;
            
            /* Go to the next state */
            masterState = I2C_MAG_STATE_STOP;
            break;
            
        case I2C_MAG_STATE_DATA:
            /* Check if it is a write command */
            if (masterControl & I2C_MAG_WRITE_FRAME)
            {           
                /* Check if all bytes were sent */
                if (byteCounter < packetSize)
                {            
                    if (bitCounter < 8)
                    {
                        /* Process the bit from outBuffer */
                        if (outBuffer[byteCounter] & (0x80 >> (bitCounter % 8)))
                        {
                            SetSDA;
                        }
                        else
                        {
                            ClrSDA;
                        }
                        
                        /* Increment bit counter */
                        bitCounter++;
                        
                        /* Set SCL to HIGH*/
                        SetSCL;
                        
                        /* Go to the next state */
                        masterState = I2C_MAG_STATE_DATA_CLK;
                    }
                    else
                    {
                        /* Set SDA high to check if NAK */
                        SetSDA;
                        
                        /* Decrement byte counter */
                        byteCounter++;
                        
                        /* Move to the next state */
                        masterState = I2C_MAG_STATE_ACK;
                    }
                }
                else
                {
                    /* Set SCL and SDA to LOW */
                    ClrSCL;     
                    ClrSDA;         
                    
                    /* Transmission completed */
                    masterState = I2C_MAG_STATE_STOP;
                }
            }
            else /* Process a Read command */
            {
                /* Release the SDA line */
                SetSDA;
                
                /* Check if all bytes were read */
                if (byteCounter < packetSize)
                {            
                    if (bitCounter < 8)
                    {
                        /* Shift the byte to set the current bit */
                        theByte <<= 1;
                        
                        /* Set SCL to High */
                        SetSCL;
                        
                        /* Capture the bit */
                        if (GetSDA)
                        {
                            theByte |= 1;
                        }
                        
                        /* Increment the bit counter */
                        bitCounter++;
                        
                        /* Go to the next state */
                        masterState = I2C_MAG_STATE_DATA_CLK;
                    }
                    else
                    {
                        /* One byte complete, update buffer */
                        inBuffer[byteCounter] = theByte;
                        
                        /* Increment the byte counter */
                        byteCounter++;
                        
                        /* Check if it is the last byte */
                        if (byteCounter == packetSize)
                        {
                            /* Last byte, so NACK it */
                            SetSDA;
                            
                            /* Move to the next state */
                            masterState = I2C_MAG_STATE_ACK;
                        }
                        else
                        {
                            /* Not yet, so ACK it */
                            ClrSDA;
                            
                            /* Move to the next state */
                            masterState = I2C_MAG_STATE_ACK;
                        }
                    }
                }
                else
                {
                }
            }
            break;
            
        case I2C_MAG_STATE_DATA_CLK:
            /* Set SCL to LOW */
            ClrSCL;
            
            /* Go to the next state */
            masterState = I2C_MAG_STATE_DATA;
            break;
            
        case I2C_MAG_STATE_STOP:
            /* Set Clock High */
            SetSCL;
                       
            /* Go to IDLE state */
            masterState = I2C_MAG_STATE_STOP_CLK;
            break;    
            
        case I2C_MAG_STATE_STOP_CLK:
            /* Set SDA High */
            SetSDA;
            
            /* Return completion */
            if (masterControl & I2C_MAG_WRITE_FRAME)
            {
                masterStatus |= I2C_MAG_MSTAT_WR_CMPLT;
            }
            else
            {
                masterStatus |= I2C_MAG_MSTAT_RD_CMPLT;
            }
                
            /* Clear busy bit */
            masterStatus &= ~(I2C_MAG_MSTAT_XFER_INP);
            
            /* Clear the master control */
            masterControl = 0;
            
            /* Go to IDLE state */
            masterState = I2C_MAG_STATE_IDLE;
            break;
            
        case I2C_MAG_STATE_NO_STOP:
            /* Set SDA and SCL High */
            SetSDA;
            SetSCL;
            
            /* Set transmission completed */
            masterStatus = I2C_MAG_MSTAT_XFER_HALT;
            
            /* Clear master control */
            masterControl = 0;
            
            /* Go to IDLE state */
            masterState = I2C_MAG_STATE_IDLE;            
            break;
    }
    
    return masterStatus;
}

/*******************************************************************************
* Function Name: I2C_MAG_ClearStatus
********************************************************************************
*
* Summary:
*  Clear internal status
*
*******************************************************************************/
void   I2C_MAG_ClearStatus(void)
{
    masterStatus = 0;
}

/* [] END OF FILE */
