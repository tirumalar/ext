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

#if !defined(CY_EZI2C_PVT_EZI2C_1_H)
#define CY_EZI2C_PVT_EZI2C_1_H

#include "EZI2C_1.h"


/***************************************
*     Vars with External Linkage
***************************************/

extern EZI2C_1_BACKUP_STRUCT  EZI2C_1_backup;

extern volatile uint8 EZI2C_1_curStatus; /* Status byte */
extern volatile uint8 EZI2C_1_curState;  /* Current state of I2C state machine */

/* Pointer to data exposed to I2C Master */
extern volatile uint8 * EZI2C_1_dataPtrS1;

/* Size of buffer1 in bytes */
extern volatile unsigned short EZI2C_1_bufSizeS1; 

/* Offset for read and write operations, set at each write sequence */
extern volatile uint8 EZI2C_1_rwOffsetS1;

/* Points to next value to be read or written */
extern volatile uint8 EZI2C_1_rwIndexS1;

/* Offset where data is read only */
extern volatile unsigned short EZI2C_1_wrProtectS1;

/* If two slave addresses, creat second set of varaibles  */
#if(EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES)

    /* Software address compare 1 */
    extern volatile uint8 EZI2C_1_addrS1;

    /* Software address compare 2 */
    extern volatile uint8 EZI2C_1_addrS2;

    /* Pointer to data exposed to I2C Master */
    extern volatile uint8 * EZI2C_1_dataPtrS2;

    /* Size of buffer2 in bytes */
    extern volatile unsigned short EZI2C_1_bufSizeS2; 

    /* Offset for read and write operations, set at each write sequence */
    extern volatile uint8 EZI2C_1_rwOffsetS2;

    /* Points to next value to be read or written */
    extern volatile uint8 EZI2C_1_rwIndexS2;

    /* Offset where data is read only */
    extern volatile unsigned short EZI2C_1_wrProtectS2;

#endif  /* (EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES) */

#if(EZI2C_1_WAKEUP_ENABLED)
    extern volatile uint8 EZI2C_1_wakeupSource;
#endif /* (EZI2C_1_WAKEUP_ENABLED) */

#endif /* CY_EZI2C_PVT_EZI2C_1_H */


/* [] END OF FILE */
