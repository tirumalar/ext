/*******************************************************************************
* File Name: OLD_SDA.h  
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

#if !defined(CY_PINS_OLD_SDA_ALIASES_H) /* Pins OLD_SDA_ALIASES_H */
#define CY_PINS_OLD_SDA_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define OLD_SDA_0			(OLD_SDA__0__PC)
#define OLD_SDA_0_PS		(OLD_SDA__0__PS)
#define OLD_SDA_0_PC		(OLD_SDA__0__PC)
#define OLD_SDA_0_DR		(OLD_SDA__0__DR)
#define OLD_SDA_0_SHIFT	(OLD_SDA__0__SHIFT)
#define OLD_SDA_0_INTR	((uint16)((uint16)0x0003u << (OLD_SDA__0__SHIFT*2u)))

#define OLD_SDA_INTR_ALL	 ((uint16)(OLD_SDA_0_INTR))


#endif /* End Pins OLD_SDA_ALIASES_H */


/* [] END OF FILE */
