/*******************************************************************************
* File Name: VREFC.h  
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

#if !defined(CY_PINS_VREFC_ALIASES_H) /* Pins VREFC_ALIASES_H */
#define CY_PINS_VREFC_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define VREFC_0			(VREFC__0__PC)
#define VREFC_0_PS		(VREFC__0__PS)
#define VREFC_0_PC		(VREFC__0__PC)
#define VREFC_0_DR		(VREFC__0__DR)
#define VREFC_0_SHIFT	(VREFC__0__SHIFT)
#define VREFC_0_INTR	((uint16)((uint16)0x0003u << (VREFC__0__SHIFT*2u)))

#define VREFC_INTR_ALL	 ((uint16)(VREFC_0_INTR))


#endif /* End Pins VREFC_ALIASES_H */


/* [] END OF FILE */
