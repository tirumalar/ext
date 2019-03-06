/*******************************************************************************
* File Name: VFB.h  
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

#if !defined(CY_PINS_VFB_ALIASES_H) /* Pins VFB_ALIASES_H */
#define CY_PINS_VFB_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define VFB_0			(VFB__0__PC)
#define VFB_0_PS		(VFB__0__PS)
#define VFB_0_PC		(VFB__0__PC)
#define VFB_0_DR		(VFB__0__DR)
#define VFB_0_SHIFT	(VFB__0__SHIFT)
#define VFB_0_INTR	((uint16)((uint16)0x0003u << (VFB__0__SHIFT*2u)))

#define VFB_INTR_ALL	 ((uint16)(VFB_0_INTR))


#endif /* End Pins VFB_ALIASES_H */


/* [] END OF FILE */
