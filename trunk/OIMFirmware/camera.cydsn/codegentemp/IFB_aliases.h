/*******************************************************************************
* File Name: IFB.h  
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

#if !defined(CY_PINS_IFB_ALIASES_H) /* Pins IFB_ALIASES_H */
#define CY_PINS_IFB_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define IFB_0			(IFB__0__PC)
#define IFB_0_PS		(IFB__0__PS)
#define IFB_0_PC		(IFB__0__PC)
#define IFB_0_DR		(IFB__0__DR)
#define IFB_0_SHIFT	(IFB__0__SHIFT)
#define IFB_0_INTR	((uint16)((uint16)0x0003u << (IFB__0__SHIFT*2u)))

#define IFB_INTR_ALL	 ((uint16)(IFB_0_INTR))


#endif /* End Pins IFB_ALIASES_H */


/* [] END OF FILE */
