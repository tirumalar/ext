/*******************************************************************************
* File Name: AUD_OUT.h  
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

#if !defined(CY_PINS_AUD_OUT_ALIASES_H) /* Pins AUD_OUT_ALIASES_H */
#define CY_PINS_AUD_OUT_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define AUD_OUT_0			(AUD_OUT__0__PC)
#define AUD_OUT_0_PS		(AUD_OUT__0__PS)
#define AUD_OUT_0_PC		(AUD_OUT__0__PC)
#define AUD_OUT_0_DR		(AUD_OUT__0__DR)
#define AUD_OUT_0_SHIFT	(AUD_OUT__0__SHIFT)
#define AUD_OUT_0_INTR	((uint16)((uint16)0x0003u << (AUD_OUT__0__SHIFT*2u)))

#define AUD_OUT_INTR_ALL	 ((uint16)(AUD_OUT_0_INTR))


#endif /* End Pins AUD_OUT_ALIASES_H */


/* [] END OF FILE */
