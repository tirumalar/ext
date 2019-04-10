/*******************************************************************************
* File Name: .h
* Version 2.0
*
* Description:
*  This file provides private constants and parameter values for the EZI2C
*  component.
*
********************************************************************************
* Copyright 2013-2015, Cypress Semiconductor Corporation. All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_EZI2C_PVT_EZI2Cs_H)
#define CY_EZI2C_PVT_EZI2Cs_H

#include "EZI2Cs.h"


/***************************************
*     Vars with External Linkage
***************************************/

extern EZI2Cs_BACKUP_STRUCT  EZI2Cs_backup;

/* Status and state variables */
extern volatile uint8 EZI2Cs_curStatus;
extern volatile uint8 EZI2Cs_curState;

/* Primary slave address variables */
extern volatile uint8* EZI2Cs_dataPtrS1;
extern volatile uint16 EZI2Cs_bufSizeS1;
extern volatile uint16 EZI2Cs_wrProtectS1;
extern volatile uint16 EZI2Cs_rwOffsetS1;
extern volatile uint16 EZI2Cs_rwIndexS1;

/* Secondary slave address variables */
#if (EZI2Cs_ADDRESSES == EZI2Cs_TWO_ADDRESSES)
    extern volatile uint8  EZI2Cs_addrS1;
    extern volatile uint8  EZI2Cs_addrS2;
    extern volatile uint8* EZI2Cs_dataPtrS2;
    extern volatile uint16 EZI2Cs_bufSizeS2;
    extern volatile uint16 EZI2Cs_wrProtectS2;
    extern volatile uint16 EZI2Cs_rwOffsetS2;
    extern volatile uint16 EZI2Cs_rwIndexS2;
#endif  /* (EZI2Cs_ADDRESSES == EZI2Cs_TWO_ADDRESSES) */

#if (EZI2Cs_WAKEUP_ENABLED)
    extern volatile uint8 EZI2Cs_wakeupSource;
#endif /* (EZI2Cs_WAKEUP_ENABLED) */

#endif /* CY_EZI2C_PVT_EZI2Cs_H */


/* [] END OF FILE */
