/*******************************************************************************
* File Name: VREFA.h  
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

#if !defined(CY_PINS_VREFA_ALIASES_H) /* Pins VREFA_ALIASES_H */
#define CY_PINS_VREFA_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define VREFA_0			(VREFA__0__PC)
#define VREFA_0_PS		(VREFA__0__PS)
#define VREFA_0_PC		(VREFA__0__PC)
#define VREFA_0_DR		(VREFA__0__DR)
#define VREFA_0_SHIFT	(VREFA__0__SHIFT)
#define VREFA_0_INTR	((uint16)((uint16)0x0003u << (VREFA__0__SHIFT*2u)))

#define VREFA_INTR_ALL	 ((uint16)(VREFA_0_INTR))


#endif /* End Pins VREFA_ALIASES_H */


/* [] END OF FILE */
