/*******************************************************************************
* File Name: MAX_I.h  
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

#if !defined(CY_PINS_MAX_I_ALIASES_H) /* Pins MAX_I_ALIASES_H */
#define CY_PINS_MAX_I_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define MAX_I_0			(MAX_I__0__PC)
#define MAX_I_0_PS		(MAX_I__0__PS)
#define MAX_I_0_PC		(MAX_I__0__PC)
#define MAX_I_0_DR		(MAX_I__0__DR)
#define MAX_I_0_SHIFT	(MAX_I__0__SHIFT)
#define MAX_I_0_INTR	((uint16)((uint16)0x0003u << (MAX_I__0__SHIFT*2u)))

#define MAX_I_INTR_ALL	 ((uint16)(MAX_I_0_INTR))


#endif /* End Pins MAX_I_ALIASES_H */


/* [] END OF FILE */
