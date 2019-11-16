/*******************************************************************************
* File Name: PWM.h  
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

#if !defined(CY_PINS_PWM_ALIASES_H) /* Pins PWM_ALIASES_H */
#define CY_PINS_PWM_ALIASES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"


/***************************************
*              Constants        
***************************************/
#define PWM_0			(PWM__0__PC)
#define PWM_0_PS		(PWM__0__PS)
#define PWM_0_PC		(PWM__0__PC)
#define PWM_0_DR		(PWM__0__DR)
#define PWM_0_SHIFT	(PWM__0__SHIFT)
#define PWM_0_INTR	((uint16)((uint16)0x0003u << (PWM__0__SHIFT*2u)))

#define PWM_1			(PWM__1__PC)
#define PWM_1_PS		(PWM__1__PS)
#define PWM_1_PC		(PWM__1__PC)
#define PWM_1_DR		(PWM__1__DR)
#define PWM_1_SHIFT	(PWM__1__SHIFT)
#define PWM_1_INTR	((uint16)((uint16)0x0003u << (PWM__1__SHIFT*2u)))

#define PWM_2			(PWM__2__PC)
#define PWM_2_PS		(PWM__2__PS)
#define PWM_2_PC		(PWM__2__PC)
#define PWM_2_DR		(PWM__2__DR)
#define PWM_2_SHIFT	(PWM__2__SHIFT)
#define PWM_2_INTR	((uint16)((uint16)0x0003u << (PWM__2__SHIFT*2u)))

#define PWM_3			(PWM__3__PC)
#define PWM_3_PS		(PWM__3__PS)
#define PWM_3_PC		(PWM__3__PC)
#define PWM_3_DR		(PWM__3__DR)
#define PWM_3_SHIFT	(PWM__3__SHIFT)
#define PWM_3_INTR	((uint16)((uint16)0x0003u << (PWM__3__SHIFT*2u)))

#define PWM_4			(PWM__4__PC)
#define PWM_4_PS		(PWM__4__PS)
#define PWM_4_PC		(PWM__4__PC)
#define PWM_4_DR		(PWM__4__DR)
#define PWM_4_SHIFT	(PWM__4__SHIFT)
#define PWM_4_INTR	((uint16)((uint16)0x0003u << (PWM__4__SHIFT*2u)))

#define PWM_5			(PWM__5__PC)
#define PWM_5_PS		(PWM__5__PS)
#define PWM_5_PC		(PWM__5__PC)
#define PWM_5_DR		(PWM__5__DR)
#define PWM_5_SHIFT	(PWM__5__SHIFT)
#define PWM_5_INTR	((uint16)((uint16)0x0003u << (PWM__5__SHIFT*2u)))

#define PWM_6			(PWM__6__PC)
#define PWM_6_PS		(PWM__6__PS)
#define PWM_6_PC		(PWM__6__PC)
#define PWM_6_DR		(PWM__6__DR)
#define PWM_6_SHIFT	(PWM__6__SHIFT)
#define PWM_6_INTR	((uint16)((uint16)0x0003u << (PWM__6__SHIFT*2u)))

#define PWM_7			(PWM__7__PC)
#define PWM_7_PS		(PWM__7__PS)
#define PWM_7_PC		(PWM__7__PC)
#define PWM_7_DR		(PWM__7__DR)
#define PWM_7_SHIFT	(PWM__7__SHIFT)
#define PWM_7_INTR	((uint16)((uint16)0x0003u << (PWM__7__SHIFT*2u)))

#define PWM_INTR_ALL	 ((uint16)(PWM_0_INTR| PWM_1_INTR| PWM_2_INTR| PWM_3_INTR| PWM_4_INTR| PWM_5_INTR| PWM_6_INTR| PWM_7_INTR))


#endif /* End Pins PWM_ALIASES_H */


/* [] END OF FILE */
