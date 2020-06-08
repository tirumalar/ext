/*******************************************************************************
* File Name: EZI2CPin.h  
* Version 2.20
*
* Description:
*  This file contains the Alias definitions for Per-Pin APIs in cypins.h. 
*  Information on using these APIs can be found in the System Reference Guide.
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_EZI2CPin_ALIASES_H) /* Pins EZI2CPin_ALIASES_H */
#define CY_PINS_EZI2CPin_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"


/***************************************
*              Constants        
***************************************/
#define EZI2CPin_0			(EZI2CPin__0__PC)
#define EZI2CPin_0_INTR	((uint16)((uint16)0x0001u << EZI2CPin__0__SHIFT))

#define EZI2CPin_1			(EZI2CPin__1__PC)
#define EZI2CPin_1_INTR	((uint16)((uint16)0x0001u << EZI2CPin__1__SHIFT))

#define EZI2CPin_INTR_ALL	 ((uint16)(EZI2CPin_0_INTR| EZI2CPin_1_INTR))
#define EZI2CPin_scl			(EZI2CPin__scl__PC)
#define EZI2CPin_scl_INTR	((uint16)((uint16)0x0001u << EZI2CPin__0__SHIFT))

#define EZI2CPin_sda			(EZI2CPin__sda__PC)
#define EZI2CPin_sda_INTR	((uint16)((uint16)0x0001u << EZI2CPin__1__SHIFT))

#endif /* End Pins EZI2CPin_ALIASES_H */


/* [] END OF FILE */
