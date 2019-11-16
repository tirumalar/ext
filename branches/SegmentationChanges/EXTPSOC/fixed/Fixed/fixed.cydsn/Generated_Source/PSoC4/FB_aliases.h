/*******************************************************************************
* File Name: FB.h  
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

#if !defined(CY_PINS_FB_ALIASES_H) /* Pins FB_ALIASES_H */
#define CY_PINS_FB_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define FB_0			(FB__0__PC)
#define FB_0_PS		(FB__0__PS)
#define FB_0_PC		(FB__0__PC)
#define FB_0_DR		(FB__0__DR)
#define FB_0_SHIFT	(FB__0__SHIFT)
#define FB_0_INTR	((uint16)((uint16)0x0003u << (FB__0__SHIFT*2u)))

#define FB_INTR_ALL	 ((uint16)(FB_0_INTR))


#endif /* End Pins FB_ALIASES_H */


/* [] END OF FILE */
