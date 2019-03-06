/*******************************************************************************
* File Name: EXT_RES.h  
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

#if !defined(CY_PINS_EXT_RES_ALIASES_H) /* Pins EXT_RES_ALIASES_H */
#define CY_PINS_EXT_RES_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define EXT_RES_0			(EXT_RES__0__PC)
#define EXT_RES_0_PS		(EXT_RES__0__PS)
#define EXT_RES_0_PC		(EXT_RES__0__PC)
#define EXT_RES_0_DR		(EXT_RES__0__DR)
#define EXT_RES_0_SHIFT	(EXT_RES__0__SHIFT)
#define EXT_RES_0_INTR	((uint16)((uint16)0x0003u << (EXT_RES__0__SHIFT*2u)))

#define EXT_RES_INTR_ALL	 ((uint16)(EXT_RES_0_INTR))


#endif /* End Pins EXT_RES_ALIASES_H */


/* [] END OF FILE */
