/*******************************************************************************
* File Name: VREFB.h  
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

#if !defined(CY_PINS_VREFB_ALIASES_H) /* Pins VREFB_ALIASES_H */
#define CY_PINS_VREFB_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define VREFB_0			(VREFB__0__PC)
#define VREFB_0_PS		(VREFB__0__PS)
#define VREFB_0_PC		(VREFB__0__PC)
#define VREFB_0_DR		(VREFB__0__DR)
#define VREFB_0_SHIFT	(VREFB__0__SHIFT)
#define VREFB_0_INTR	((uint16)((uint16)0x0003u << (VREFB__0__SHIFT*2u)))

#define VREFB_INTR_ALL	 ((uint16)(VREFB_0_INTR))


#endif /* End Pins VREFB_ALIASES_H */


/* [] END OF FILE */
