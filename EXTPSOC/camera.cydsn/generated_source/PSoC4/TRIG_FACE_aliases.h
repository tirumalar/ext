/*******************************************************************************
* File Name: TRIG_FACE.h  
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

#if !defined(CY_PINS_TRIG_FACE_ALIASES_H) /* Pins TRIG_FACE_ALIASES_H */
#define CY_PINS_TRIG_FACE_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define TRIG_FACE_0			(TRIG_FACE__0__PC)
#define TRIG_FACE_0_PS		(TRIG_FACE__0__PS)
#define TRIG_FACE_0_PC		(TRIG_FACE__0__PC)
#define TRIG_FACE_0_DR		(TRIG_FACE__0__DR)
#define TRIG_FACE_0_SHIFT	(TRIG_FACE__0__SHIFT)
#define TRIG_FACE_0_INTR	((uint16)((uint16)0x0003u << (TRIG_FACE__0__SHIFT*2u)))

#define TRIG_FACE_INTR_ALL	 ((uint16)(TRIG_FACE_0_INTR))


#endif /* End Pins TRIG_FACE_ALIASES_H */


/* [] END OF FILE */
