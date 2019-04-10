/*******************************************************************************
* File Name: CAM_SPARE.h  
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

#if !defined(CY_PINS_CAM_SPARE_ALIASES_H) /* Pins CAM_SPARE_ALIASES_H */
#define CY_PINS_CAM_SPARE_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define CAM_SPARE_0			(CAM_SPARE__0__PC)
#define CAM_SPARE_0_PS		(CAM_SPARE__0__PS)
#define CAM_SPARE_0_PC		(CAM_SPARE__0__PC)
#define CAM_SPARE_0_DR		(CAM_SPARE__0__DR)
#define CAM_SPARE_0_SHIFT	(CAM_SPARE__0__SHIFT)
#define CAM_SPARE_0_INTR	((uint16)((uint16)0x0003u << (CAM_SPARE__0__SHIFT*2u)))

#define CAM_SPARE_INTR_ALL	 ((uint16)(CAM_SPARE_0_INTR))


#endif /* End Pins CAM_SPARE_ALIASES_H */


/* [] END OF FILE */
