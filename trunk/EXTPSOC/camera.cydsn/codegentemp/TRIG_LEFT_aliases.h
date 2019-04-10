/*******************************************************************************
* File Name: TRIG_LEFT.h  
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

#if !defined(CY_PINS_TRIG_LEFT_ALIASES_H) /* Pins TRIG_LEFT_ALIASES_H */
#define CY_PINS_TRIG_LEFT_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define TRIG_LEFT_0			(TRIG_LEFT__0__PC)
#define TRIG_LEFT_0_PS		(TRIG_LEFT__0__PS)
#define TRIG_LEFT_0_PC		(TRIG_LEFT__0__PC)
#define TRIG_LEFT_0_DR		(TRIG_LEFT__0__DR)
#define TRIG_LEFT_0_SHIFT	(TRIG_LEFT__0__SHIFT)
#define TRIG_LEFT_0_INTR	((uint16)((uint16)0x0003u << (TRIG_LEFT__0__SHIFT*2u)))

#define TRIG_LEFT_INTR_ALL	 ((uint16)(TRIG_LEFT_0_INTR))


#endif /* End Pins TRIG_LEFT_ALIASES_H */


/* [] END OF FILE */
