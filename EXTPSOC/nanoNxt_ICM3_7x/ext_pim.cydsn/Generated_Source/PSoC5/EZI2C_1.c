/*******************************************************************************
* File Name: EZI2C_1.c
* Version 1.90
*
* Description:
*  This file contains the setup, control and status commands for the EZI2C
*  component.  Actual protocol and operation code resides in the interrupt
*  service routine file.
*
********************************************************************************
* Copyright 2008-2013, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "EZI2C_1_PVT.h"

/**********************************
*      System variables
**********************************/

uint8 EZI2C_1_initVar = 0u; /* Defines if component was initialized */

volatile uint8 EZI2C_1_curStatus; /* Status byte */
volatile uint8 EZI2C_1_curState;  /* Current state of I2C state machine */

/* Pointer to data exposed to I2C Master */
volatile uint8 * EZI2C_1_dataPtrS1;

/* Size of buffer1 in bytes */
volatile unsigned short EZI2C_1_bufSizeS1; 

/* Offset for read and write operations, set at each write sequence */
volatile uint8 EZI2C_1_rwOffsetS1;

/* Points to next value to be read or written */
volatile uint8 EZI2C_1_rwIndexS1;

/* Offset where data is read only */
volatile unsigned short EZI2C_1_wrProtectS1;

/* If two slave addresses, creat second set of varaibles  */
#if(EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES)

    /* Software address compare 1 */
    volatile uint8 EZI2C_1_addrS1;

    /* Software address compare 2 */
    volatile uint8 EZI2C_1_addrS2;

    /* Pointer to data exposed to I2C Master */
    volatile uint8 * EZI2C_1_dataPtrS2;

    /* Size of buffer2 in bytes */
    volatile unsigned short EZI2C_1_bufSizeS2; 

    /* Offset for read and write operations, set at each write sequence */
    volatile uint8 EZI2C_1_rwOffsetS2;

    /* Points to next value to be read or written */
    volatile uint8 EZI2C_1_rwIndexS2;

    /* Offset where data is read only */
    volatile unsigned short EZI2C_1_wrProtectS2;

#endif  /* (EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES) */


/*******************************************************************************
* Function Name: EZI2C_1_Init
********************************************************************************
*
* Summary:
*  Initializes/restores default EZI2C configuration provided with customizer.
*  Usually called in EZI2C_1_Start().
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  EZI2C_1_addrS1 - the new slave address for the first slave device is
*   saved.
*
*  EZI2C_1_addrS2 - the new slave address for the second slave device
*   is saved, if EzI2C component is configured for two slave addresses.
*
* Reentrant:
*  No
*
* Side Effects:
*  All changes applied by API to the component's configuration will be reset.
*
*******************************************************************************/
void EZI2C_1_Init(void) 
{
    /* Clear Status register */
    EZI2C_1_CSR_REG  = 0x00u;

    /* Enable I2C block's slave operation.
    *  These revisions require slave to be enabled for registers to be
    *  written.
    */
    EZI2C_1_CFG_REG |= EZI2C_1_CFG_EN_SLAVE;

    /* 8 LSB bits of the 10-bit are written with the divide factor */
    EZI2C_1_CLKDIV1_REG = LO8(EZI2C_1_DIVIDE_FACTOR);

    /* 2 MSB bits of the 10-bit are written with the divide factor */
    EZI2C_1_CLKDIV2_REG = HI8(EZI2C_1_DIVIDE_FACTOR);

    /* Define clock rate */
    if(EZI2C_1_BUS_SPEED <= EZI2C_1_BUS_SPEED_50KHZ)
    {   /* 50 kHz - 32 samples/bit */
        EZI2C_1_CFG_REG |= EZI2C_1_CFG_CLK_RATE;
    }
    else
    {   /* 100kHz or 400kHz - 16 samples/bit */
        EZI2C_1_CFG_REG &= ((uint8) ~EZI2C_1_CFG_CLK_RATE);
    }

    /* if I2C block is used as wake up source */
    #if(1u == EZI2C_1_ENABLE_WAKEUP)

        /* Configure I2C address match to act as wake-up source */
       EZI2C_1_XCFG_REG |= EZI2C_1_XCFG_I2C_ON;

        /* Process sio_select and pselect */
        #if(EZI2C_1_ADDRESSES == EZI2C_1_ONE_ADDRESS)
            if((uint8) EZI2C_1__ANY != (uint8) EZI2C_1_BUS_PORT)
            {
                /* SCL and SDA lines get their inputs from SIO block */
                EZI2C_1_CFG_REG |= EZI2C_1_CFG_PSELECT;

                if((uint8) EZI2C_1__I2C0 == (uint8) EZI2C_1_BUS_PORT)
                {
                    /* SCL and SDA lines get their inputs from SIO1 */
                    EZI2C_1_CFG_REG &= ((uint8) ~EZI2C_1_CFG_SIO_SELECT);
                }
                else /* SIO2 */
                {
                    /* SCL and SDA lines get their inputs from SIO2 */
                    EZI2C_1_CFG_REG |= EZI2C_1_CFG_SIO_SELECT;
                }
            }
            else    /* GPIO is used */
            {
                /* SCL and SDA lines get their inputs from GPIO module. */
                EZI2C_1_CFG_REG &= ((uint8) ~EZI2C_1_CFG_PSELECT);
            }
        #endif  /* (EZI2C_1_ADDRESSES == EZI2C_1_ONE_ADDRESS) */

    #endif /* (1u == EZI2C_1_ENABLE_WAKEUP) */

    
    #if(EZI2C_1_ADDRESSES == EZI2C_1_ONE_ADDRESS)

        /* Set default slave address */
        EZI2C_1_ADDR_REG  = EZI2C_1_DEFAULT_ADDR1;

        /* Turn on hardware address detection */
        EZI2C_1_XCFG_REG  |= EZI2C_1_XCFG_HDWR_ADDR_EN;

    #else   /* Two devices */

        /* Set default slave addresses */
        EZI2C_1_addrS1  = EZI2C_1_DEFAULT_ADDR1;
        EZI2C_1_addrS2  = EZI2C_1_DEFAULT_ADDR2;

    #endif  /* End of (EZI2C_1_ADDRESSES == EZI2C_1_ONE_ADDRESS) */

    /* Reset offsets and pointers */
    EZI2C_1_dataPtrS1 = (volatile uint8 *)0u;
    EZI2C_1_rwOffsetS1 = 0u;
    EZI2C_1_rwIndexS1 = 0u;
    EZI2C_1_wrProtectS1 = 0u;
    EZI2C_1_bufSizeS1 = 0u;

    /* Reset offsets and pointers for 2nd slave address */
    #if(EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES)
        EZI2C_1_dataPtrS2 = (volatile uint8 *)0u;
        EZI2C_1_rwOffsetS2 = 0u;
        EZI2C_1_rwIndexS2 = 0u;
        EZI2C_1_wrProtectS2 = 0u;
        EZI2C_1_bufSizeS2 = 0u;
    #endif  /* End of (EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES) */

    /* Enable the I2C block clock */
    EZI2C_1_XCFG_REG  |= EZI2C_1_XCFG_CLK_EN;
}


/*******************************************************************************
* Function Name: EZI2C_1_Enable
********************************************************************************
*
* Summary:
*  Enables the I2C block operation, sets interrupt priority, sets
*  interrupt vector, clears pending interrupts and enables interrupts. Clears
*  status variables and reset state machine variable.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  EZI2C_1_curStatus - this global variable are cleared, it stores the
*  current component status.
*
* EZI2C_1_curState - global variable are cleared, it stores the current
*  state of the state machine.
*
* Reentrant:
*  No
*
*******************************************************************************/
void EZI2C_1_Enable(void) 
{
    uint8 interruptState;

    /* Enter critical section */
    interruptState = CyEnterCriticalSection();

    /* Enable I2C block in Active mode template */
    EZI2C_1_PM_ACT_CFG_REG |= EZI2C_1_ACT_PWR_EN;

    /* Enable I2C block in Alternate Active (Standby) mode template */
    EZI2C_1_PM_STBY_CFG_REG |= EZI2C_1_STBY_PWR_EN;

    /* Exit critical section */
    CyExitCriticalSection(interruptState);

    /* Set the interrupt priority */
    CyIntSetPriority(EZI2C_1_ISR_NUMBER, EZI2C_1_ISR_PRIORITY);

    /* Set the interrupt vector */
    (void) CyIntSetVector(EZI2C_1_ISR_NUMBER, &EZI2C_1_ISR);

    /* Clear any pending interrupt */
    (void) CyIntClearPending(EZI2C_1_ISR_NUMBER);

    /* Reset State Machine to IDLE */
    EZI2C_1_curState = EZI2C_1_SM_IDLE;

    /* Clear Status variable */
    EZI2C_1_curStatus = 0x00u;

    /* Enable the interrupt */
    EZI2C_1_EnableInt();
}


/*******************************************************************************
* Function Name: EZI2C_1_Start
********************************************************************************
*
* Summary:
*  Starts the component and enables the interupt. If this function is called at
*  first (or EZI2C_1_initVar was cleared, then EZI2C_1_Init()
*  function is called and all offsets and pointers are reset. Anyway, the
*  state machine state is set to IDLE, status variable is cleared and the
*  interrupt is enabled.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  EZI2C_1_initVar - is used to indicate initial configuration
*  of this component.  The variable is initialized to zero and set to 1
*  the first time EZI2C_1_Start() is called. This allows for component
*  initialization without re-initialization in all subsequent calls
*  to the EZI2C_1_Start() routine.
*
*  EZI2C_1_dataPtrS1 global variable, which stores pointer to the
*  data exposed to an I2C master for the first slave address is reset if
*  EZI2C_1_initVar is set 0 by EZI2C_1_initVar function call.
*
*  EZI2C_1_rwOffsetS1 - global variable, which stores offset for read
*  and write operations, is set at each write sequence of the first slave
*  address is reset if EZI2C_1_initVar is 0, by
*  EZI2C_1_initVar function call.
*
*  EZI2C_1_rwIndexS1 - global variable, which stores pointer to the
*  next value to be read or written for the first slave address is reset if
*  EZI2C_1_initVar is 0, by EZI2C_1_initVar function call.
*
* EZI2C_1_wrProtectS1 - global variable, which stores offset where data
*  is read only for the first slave address is reset if
*  EZI2C_1_initVar is 0, by EZI2C_1_initVar function call.
*
* EZI2C_1_bufSizeS1 - global variable, which stores size of data array
*  exposed to an I2C master for the first slave address is reset if
*  EZI2C_1_initVar is 0, by EZI2C_1_initVar function call.
*
*  EZI2C_1_dataPtrS2 - global variable, which stores pointer to the
*  data exposed to an I2C master for the second slave address is reset if
*  EZI2C_1_initVar is 0, by EZI2C_1_initVar function call.
*
*  EZI2C_1_rwOffsetS2 - global variable, which stores offset for read
*  and write operations, is set at each write sequence of the second slave
*  device is reset if EZI2C_1_initVar is 0, by EZI2C_1_initVar
*  function call.
*
*  EZI2C_1_rwIndexS2 - global variable, which stores pointer to the
*  next value to be read or written for the second slave address is reset if
*  EZI2C_1_initVar is 0, by EZI2C_1_initVar function call.
*
* EZI2C_1_wrProtectS2 - global variable, which stores offset where data
*  is read only for the second slave address is reset if
*  EZI2C_1_initVar is 0, by EZI2C_1_initVar function call.
*
* EZI2C_1_bufSizeS2 - global variable, which stores size of data array
*  exposed to an I2C master for the second slave address is reset if
*  EZI2C_1_initVar is 0, by EZI2C_1_initVar function call.
*
* Side Effects:
*  This component automatically enables its interrupt. If I2C is enabled
*  without the interrupt enabled, it could lock up the I2C bus.
*
* Reentrant:
*  No
*
*******************************************************************************/
void EZI2C_1_Start(void) 
{
    if(0u == EZI2C_1_initVar)
    {
        EZI2C_1_Init(); /* Initialize component's parameters */
        EZI2C_1_initVar = 1u; /* Component initialized */
    }
    
    EZI2C_1_Enable();
}


/*******************************************************************************
* Function Name: EZI2C_1_Stop
********************************************************************************
*
* Summary:
*  Disable the I2C block's slave operation and the corresponding interrupt.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void EZI2C_1_Stop(void) 
{
    uint8 interruptState;

    /* Disable Interrupt */
    EZI2C_1_DisableInt();

    /* Store resgisters which are held in reset when Slave is disabled */
    EZI2C_1_backup.adr = EZI2C_1_ADDR_REG;
    EZI2C_1_backup.clkDiv1  = EZI2C_1_CLKDIV1_REG;
    EZI2C_1_backup.clkDiv2  = EZI2C_1_CLKDIV2_REG;

    /* Reset fixed-function block */
    EZI2C_1_CFG_REG &= ((uint8) ~EZI2C_1_CFG_EN_SLAVE);
    EZI2C_1_CFG_REG |= EZI2C_1_CFG_EN_SLAVE;

    /* Restore registers */
    EZI2C_1_ADDR_REG = EZI2C_1_backup.adr;
    EZI2C_1_CLKDIV1_REG = EZI2C_1_backup.clkDiv1;
    EZI2C_1_CLKDIV2_REG = EZI2C_1_backup.clkDiv2;

    interruptState = CyEnterCriticalSection();

    /* Disable I2C block in Active mode template */
    EZI2C_1_PM_ACT_CFG_REG &= ((uint8) ~EZI2C_1_ACT_PWR_EN);

    /* Disable I2C block in Alternate Active (Standby) mode template */
    EZI2C_1_PM_STBY_CFG_REG &= ((uint8) ~EZI2C_1_STBY_PWR_EN);

    CyExitCriticalSection(interruptState);

    /* Reset State Machine to IDLE */
    EZI2C_1_curState = EZI2C_1_SM_IDLE;
}


/*******************************************************************************
* Function Name: EZI2C_1_EnableInt
********************************************************************************
*
* Summary:
*  Enables the interrupt service routine for the component.  This is normally
*  handled with the start command.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void EZI2C_1_EnableInt(void) 
{
    /* Enable interrupt */
    (void) CyIntEnable(EZI2C_1_ISR_NUMBER);
}


/*******************************************************************************
* Function Name: EZI2C_1_DisableInt
********************************************************************************
*
* Summary:
*  Disable I2C interrupts. Normally this function is not required since the
*  Stop function disables the interrupt. If the I2C interrupt is disabled while
*  the I2C master is still running, it may cause the I2C bus to lock up.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Side Effects:
*  If the I2C interrupt is disabled and the master is addressing the current
*  slave, the bus will be locked until the interrupt is re-enabled.
*
*******************************************************************************/
void EZI2C_1_DisableInt(void) 
{
    /* Disable interrupt */
    (void) CyIntDisable(EZI2C_1_ISR_NUMBER);
}


/*******************************************************************************
* Function Name: EZI2C_1_SetAddress1
********************************************************************************
*
* Summary:
*  This function sets the main address of this I2C slave device. This value may
*  be any value between 0 and 127.
*
* Parameters:
*  address:  The 7-bit slave address between 0 and 127.
*
* Return:
*  None
*
* Global variables:
*  EZI2C_1_addrS1 - the new slave address for the first slave device is
*  saved in it, if the component is configured to act as two slave devices.
*
* Reentrant:
*  No, if two addresses are used.
*
*******************************************************************************/
void EZI2C_1_SetAddress1(uint8 address) 
{
    #if(EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES)

        /* Address is stored in variable */
        EZI2C_1_addrS1  = address & EZI2C_1_SADDR_MASK;

    #else

        /* Address is stored in hardware */
        EZI2C_1_ADDR_REG = address & EZI2C_1_SADDR_MASK;

    #endif /* (EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES) */
}


/*******************************************************************************
* Function Name: EZI2C_1_GetAddress1
********************************************************************************
*
* Summary:
*  Returns address of the first slave device.
*
* Parameters:
*  None
*
* Return:
*  If the component is configured to has two slave addresses than primary
*  address is returned, otherwise address from the the address register is
*  returned.
*
* Global variables:
*  EZI2C_1_addrS1 - if component is configured to has two slave
*  addresses than primary address is saved here, otherwise address is written to
*  the register.
*
* Reentrant:
*  No
*
*******************************************************************************/
uint8 EZI2C_1_GetAddress1(void) 
{
    /* Get 1st slave address */
    #if(EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES)

        /* Return address from variable */
        return(EZI2C_1_addrS1);

    #else

        /* Return address from hardware */
        return(EZI2C_1_ADDR_REG);

    #endif /* (EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES) */
}


/*******************************************************************************
* Function Name: EZI2C_1_GetActivity
********************************************************************************
*
* Summary:
*  This function returns a nonzero value if the I2C read or write cycle
*  occurred since the last time this function was called.  The activity
*  flag resets to zero at the end of this function call.
*  The Read and Write busy flags are cleared when read, but the "BUSY"
*  flag is only cleared by an I2C Stop.
*
* Parameters:
*  None
*
* Return:
*  A non-zero value is returned if activity is detected:
*   EZI2C_1_STATUS_READ1   Set if Read sequence is detected for first
*                                   address. Cleared when status read.
*
*   EZI2C_1_STATUS_WRITE1  Set if Write sequence is detected for first
*                                   address. Cleared when status read.
*
*   EZI2C_1_STATUS_READ2   Set if Read sequence is detected for second
*                                   address (if enabled). Cleared when status
*                                   read.
*
*   EZI2C_1_STATUS_WRITE2  Set if Write sequence is detected for second
*                                   address (if enabled). Cleared when status
*                                   read.
*
*   EZI2C_1_STATUS_BUSY    Set if Start detected, cleared when stop
*                                   detected.
*
*   EZI2C_1_STATUS_ERR     Set when I2C hardware detected, cleared
*                                   when status read.
*
* Global variables:
*  EZI2C_1_curStatus - global variable, which stores the current
*  component status.
*
* Reentrant:
*  No
*
*******************************************************************************/
uint8 EZI2C_1_GetActivity(void) 
{
    uint8 tmpStatus;

    tmpStatus = EZI2C_1_curStatus;

    /* Clear status, but no Busy one */
    EZI2C_1_curStatus &= EZI2C_1_STATUS_BUSY;

    return(tmpStatus);
}


/*******************************************************************************
* Function Name: EZI2C_1_SetBuffer1
********************************************************************************
*
* Summary:
*  This function sets the buffer, size of the buffer, and the R/W boundry
*  for the memory buffer.
*
* Parameters:
*  size:  Size of the buffer in bytes.
*
*  rwBoundry: Sets how many bytes are writable in the beginning of the buffer.
*  This value must be less than or equal to the buffer size.
*
*  dataPtr:  Pointer to the data buffer.
*
* Return:
*  None
*
* Global variables:
*  EZI2C_1_dataPtrS1 - stores pointer to the data exposed to an I2C
*  master for the first slave address is modified with the the new pointer to
*  data, passed by function parameter.
*
*  EZI2C_1_rwOffsetS1 - stores offset for read and write operations, is
*  modified at each write sequence of the first slave address is reset.
*
*  EZI2C_1_rwIndexS1 - stores pointer to the next value to be read or
*  written for the first slave address is set to 0.
*
* Reentrant:
*  No
*
* Side Effects:
*  It is recommended to disable component interrupt before calling this function
*  and enable it afterwards for the proper component operation.
*
*******************************************************************************/
void EZI2C_1_SetBuffer1(unsigned short bufSize, unsigned short rwBoundry, volatile uint8 * dataPtr) 
{
    /* Check for proper buffer */
    if(NULL != dataPtr)
    {
        EZI2C_1_dataPtrS1   = dataPtr;  /* Set buffer pointer */
        EZI2C_1_bufSizeS1   = bufSize;
        EZI2C_1_wrProtectS1 = rwBoundry;
    }
    EZI2C_1_rwOffsetS1  = 0u;  /* Clears buffer offset */
    EZI2C_1_rwIndexS1   = 0u;  /* Clears buffer index */
}


#if (EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES)
    /*******************************************************************************
    * Function Name: EZI2C_1_SetBuffer2
    ********************************************************************************
    *
    * Summary:
    *  This function sets the buffer pointer, size and read/write area for the
    *  second slave data. This is the data that is exposed to the I2C Master
    *  for the second I2C address. This function is only provided if two I2C
    *  addresses have been selected in the user parameters.
    *
    * Parameters:
    *  bufSize:  Size of the buffer exposed to the I2C master.
    *
    *  rwBoundry: Sets how many bytes are readable and writable by the the I2C
    *  master. This value must be less than or equal to the buffer size. Data
    *  located at offset rwBoundry and above are read only.
    *
    *  dataPtr:  This is a pointer to the data array or structure that is used
    *  for the I2C data buffer.
    *
    * Return:
    *  None
    *
    * Global variables:
    *  EZI2C_1_dataPtrS2 - stores pointer to the data exposed to an I2C
    *  master for the second slave address is modified with the the new pointer to
    *  data, passed by function parameter.
    *
    *  EZI2C_1_rwOffsetS2 - stores offset for read and write operations,
    *  is modified at each write sequence of the second slave address is reset.
    *
    *  EZI2C_1_rwIndexS2 - stores pointer to the next value to be read or
    *  written for the second slave address is set to 0.
    *
    * Reentrant:
    *  No
    *
    * Side Effects:
    *  It is recommended to disable component interrupt before calling this
    *  function and enable it afterwards for the proper component operation.
    *
    *******************************************************************************/
    void EZI2C_1_SetBuffer2(unsigned short bufSize, unsigned short rwBoundry, volatile uint8 * dataPtr) 
    {
        /* Check for proper buffer */
        if(NULL != dataPtr)
        {
            EZI2C_1_dataPtrS2   = dataPtr;  /* Set buffer pointer */
            EZI2C_1_bufSizeS2   = bufSize;
            EZI2C_1_wrProtectS2 = rwBoundry;
        }
        EZI2C_1_rwOffsetS2  = 0u;  /* Clears buffer offset */
        EZI2C_1_rwIndexS2   = 0u;  /* Clears buffer index */
    }


    /*******************************************************************************
    * Function Name: EZI2C_1_SetAddress2
    ********************************************************************************
    *
    * Summary:
    *  Sets the I2C slave address for the second device. This value may be any
    *  value between 0 and 127. This function is only provided if two I2C
    *  addresses have been selected in the user parameters.
    *
    * Parameters:
    *  address:  The 7-bit slave address between 0 and 127.
    *
    * Return:
    *  None
    *
    * Global variables:
    *  EZI2C_1_addrS2 - the secondary slave address is modified.
    *
    * Reentrant:
    *  No
    *
    *******************************************************************************/
    void EZI2C_1_SetAddress2(uint8 address) 
    {
        /* Set slave address */
        EZI2C_1_addrS2  = address & EZI2C_1_SADDR_MASK;
    }


    /*******************************************************************************
    * Function Name: EZI2C_1_GetAddress2
    ********************************************************************************
    *
    * Summary:
    *  Returns the I2C slave address for the second device. This function is only
    *  provided if two I2C addresses have been selected in the user parameters.
    *
    * Parameters:
    *  EZI2C_1_addrS2 - global variable, which stores the second I2C
    *   address.
    *
    * Return:
    *  The secondary I2C slave address.
    *
    * Global variables:
    *  EZI2C_1_addrS2 - the secondary slave address is used.
    *
    * Reentrant:
    *  No
    *
    *******************************************************************************/
    uint8 EZI2C_1_GetAddress2(void) 
    {
        return(EZI2C_1_addrS2);
    }

#endif  /* (EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES) */


/* [] END OF FILE */
