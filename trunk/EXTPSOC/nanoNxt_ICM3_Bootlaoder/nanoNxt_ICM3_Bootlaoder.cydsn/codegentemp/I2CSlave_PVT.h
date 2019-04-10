/*******************************************************************************
* File Name: .h
* Version 3.30
*
* Description:
*  This file provides private constants and parameter values for the I2C
*  component.
*
* Note:
*
********************************************************************************
* Copyright 2012, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_I2C_PVT_I2CSlave_H)
#define CY_I2C_PVT_I2CSlave_H

#include "I2CSlave.h"

#define I2CSlave_TIMEOUT_ENABLED_INC    (0u)
#if(0u != I2CSlave_TIMEOUT_ENABLED_INC)
    #include "I2CSlave_TMOUT.h"
#endif /* (0u != I2CSlave_TIMEOUT_ENABLED_INC) */


/**********************************
*   Variables with external linkage
**********************************/

extern I2CSlave_BACKUP_STRUCT I2CSlave_backup;

extern volatile uint8 I2CSlave_state;   /* Current state of I2C FSM */

/* Master variables */
#if(I2CSlave_MODE_MASTER_ENABLED)
    extern volatile uint8 I2CSlave_mstrStatus;   /* Master Status byte  */
    extern volatile uint8 I2CSlave_mstrControl;  /* Master Control byte */

    /* Transmit buffer variables */
    extern volatile uint8 * I2CSlave_mstrRdBufPtr;   /* Pointer to Master Read buffer */
    extern volatile uint8   I2CSlave_mstrRdBufSize;  /* Master Read buffer size       */
    extern volatile uint8   I2CSlave_mstrRdBufIndex; /* Master Read buffer Index      */

    /* Receive buffer variables */
    extern volatile uint8 * I2CSlave_mstrWrBufPtr;   /* Pointer to Master Write buffer */
    extern volatile uint8   I2CSlave_mstrWrBufSize;  /* Master Write buffer size       */
    extern volatile uint8   I2CSlave_mstrWrBufIndex; /* Master Write buffer Index      */

#endif /* (I2CSlave_MODE_MASTER_ENABLED) */

/* Slave variables */
#if(I2CSlave_MODE_SLAVE_ENABLED)
    extern volatile uint8 I2CSlave_slStatus;         /* Slave Status  */

    /* Transmit buffer variables */
    extern volatile uint8 * I2CSlave_slRdBufPtr;     /* Pointer to Transmit buffer  */
    extern volatile uint8   I2CSlave_slRdBufSize;    /* Slave Transmit buffer size  */
    extern volatile uint8   I2CSlave_slRdBufIndex;   /* Slave Transmit buffer Index */

    /* Receive buffer variables */
    extern volatile uint8 * I2CSlave_slWrBufPtr;     /* Pointer to Receive buffer  */
    extern volatile uint8   I2CSlave_slWrBufSize;    /* Slave Receive buffer size  */
    extern volatile uint8   I2CSlave_slWrBufIndex;   /* Slave Receive buffer Index */

    #if(I2CSlave_SW_ADRR_DECODE)
        extern volatile uint8 I2CSlave_slAddress;     /* Software address variable */
    #endif   /* (I2CSlave_SW_ADRR_DECODE) */

#endif /* (I2CSlave_MODE_SLAVE_ENABLED) */

#if((I2CSlave_FF_IMPLEMENTED) && (I2CSlave_WAKEUP_ENABLED))
    extern volatile uint8 I2CSlave_wakeupSource;
#endif /* ((I2CSlave_FF_IMPLEMENTED) && (I2CSlave_WAKEUP_ENABLED)) */


#endif /* CY_I2C_PVT_I2CSlave_H */


/* [] END OF FILE */
