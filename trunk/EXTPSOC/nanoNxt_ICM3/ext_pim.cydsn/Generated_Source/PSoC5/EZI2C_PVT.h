/*******************************************************************************
* File Name: .h
* Version 1.90
*
* Description:
*  This file provides private constants and parameter values for the EZI2C
*  component.
*
* Note:
*
********************************************************************************
* Copyright 2013, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_EZI2C_PVT_EZI2C_H)
#define CY_EZI2C_PVT_EZI2C_H

#include "EZI2C.h"


/***************************************
*     Vars with External Linkage
***************************************/

extern EZI2C_BACKUP_STRUCT  EZI2C_backup;

extern volatile uint8 EZI2C_curStatus; /* Status byte */
extern volatile uint8 EZI2C_curState;  /* Current state of I2C state machine */

/* Pointer to data exposed to I2C Master */
extern volatile uint8 * EZI2C_dataPtrS1;

/* Size of buffer1 in bytes */
extern volatile unsigned short EZI2C_bufSizeS1; 

/* Offset for read and write operations, set at each write sequence */
extern volatile uint8 EZI2C_rwOffsetS1;

/* Points to next value to be read or written */
extern volatile uint8 EZI2C_rwIndexS1;

/* Offset where data is read only */
extern volatile unsigned short EZI2C_wrProtectS1;

/* If two slave addresses, creat second set of varaibles  */
#if(EZI2C_ADDRESSES == EZI2C_TWO_ADDRESSES)

    /* Software address compare 1 */
    extern volatile uint8 EZI2C_addrS1;

    /* Software address compare 2 */
    extern volatile uint8 EZI2C_addrS2;

    /* Pointer to data exposed to I2C Master */
    extern volatile uint8 * EZI2C_dataPtrS2;

    /* Size of buffer2 in bytes */
    extern volatile unsigned short EZI2C_bufSizeS2; 

    /* Offset for read and write operations, set at each write sequence */
    extern volatile uint8 EZI2C_rwOffsetS2;

    /* Points to next value to be read or written */
    extern volatile uint8 EZI2C_rwIndexS2;

    /* Offset where data is read only */
    extern volatile unsigned short EZI2C_wrProtectS2;

#endif  /* (EZI2C_ADDRESSES == EZI2C_TWO_ADDRESSES) */

#if(EZI2C_WAKEUP_ENABLED)
    extern volatile uint8 EZI2C_wakeupSource;
#endif /* (EZI2C_WAKEUP_ENABLED) */

#endif /* CY_EZI2C_PVT_EZI2C_H */


/* [] END OF FILE */
