/*******************************************************************************
* File Name: I_SENSE.h  
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

#if !defined(CY_PINS_I_SENSE_ALIASES_H) /* Pins I_SENSE_ALIASES_H */
#define CY_PINS_I_SENSE_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define I_SENSE_0			(I_SENSE__0__PC)
#define I_SENSE_0_PS		(I_SENSE__0__PS)
#define I_SENSE_0_PC		(I_SENSE__0__PC)
#define I_SENSE_0_DR		(I_SENSE__0__DR)
#define I_SENSE_0_SHIFT	(I_SENSE__0__SHIFT)
#define I_SENSE_0_INTR	((uint16)((uint16)0x0003u << (I_SENSE__0__SHIFT*2u)))

#define I_SENSE_INTR_ALL	 ((uint16)(I_SENSE_0_INTR))


#endif /* End Pins I_SENSE_ALIASES_H */


/* [] END OF FILE */
