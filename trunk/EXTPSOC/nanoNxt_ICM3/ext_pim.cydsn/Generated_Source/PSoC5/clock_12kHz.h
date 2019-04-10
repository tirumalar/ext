/*******************************************************************************
* File Name: clock_12kHz.h
* Version 2.20
*
*  Description:
*   Provides the function and constant definitions for the clock component.
*
*  Note:
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_CLOCK_clock_12kHz_H)
#define CY_CLOCK_clock_12kHz_H

#include <cytypes.h>
#include <cyfitter.h>


/***************************************
* Conditional Compilation Parameters
***************************************/

/* Check to see if required defines such as CY_PSOC5LP are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5LP)
    #error Component cy_clock_v2_20 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */


/***************************************
*        Function Prototypes
***************************************/

void clock_12kHz_Start(void) ;
void clock_12kHz_Stop(void) ;

#if(CY_PSOC3 || CY_PSOC5LP)
void clock_12kHz_StopBlock(void) ;
#endif /* (CY_PSOC3 || CY_PSOC5LP) */

void clock_12kHz_StandbyPower(uint8 state) ;
void clock_12kHz_SetDividerRegister(unsigned short clkDivider, uint8 restart) 
                                ;
unsigned short clock_12kHz_GetDividerRegister(void) ;
void clock_12kHz_SetModeRegister(uint8 modeBitMask) ;
void clock_12kHz_ClearModeRegister(uint8 modeBitMask) ;
uint8 clock_12kHz_GetModeRegister(void) ;
void clock_12kHz_SetSourceRegister(uint8 clkSource) ;
uint8 clock_12kHz_GetSourceRegister(void) ;
#if defined(clock_12kHz__CFG3)
void clock_12kHz_SetPhaseRegister(uint8 clkPhase) ;
uint8 clock_12kHz_GetPhaseRegister(void) ;
#endif /* defined(clock_12kHz__CFG3) */

#define clock_12kHz_Enable()                       clock_12kHz_Start()
#define clock_12kHz_Disable()                      clock_12kHz_Stop()
#define clock_12kHz_SetDivider(clkDivider)         clock_12kHz_SetDividerRegister(clkDivider, 1u)
#define clock_12kHz_SetDividerValue(clkDivider)    clock_12kHz_SetDividerRegister((clkDivider) - 1u, 1u)
#define clock_12kHz_SetMode(clkMode)               clock_12kHz_SetModeRegister(clkMode)
#define clock_12kHz_SetSource(clkSource)           clock_12kHz_SetSourceRegister(clkSource)
#if defined(clock_12kHz__CFG3)
#define clock_12kHz_SetPhase(clkPhase)             clock_12kHz_SetPhaseRegister(clkPhase)
#define clock_12kHz_SetPhaseValue(clkPhase)        clock_12kHz_SetPhaseRegister((clkPhase) + 1u)
#endif /* defined(clock_12kHz__CFG3) */


/***************************************
*             Registers
***************************************/

/* Register to enable or disable the clock */
#define clock_12kHz_CLKEN              (* (reg8 *) clock_12kHz__PM_ACT_CFG)
#define clock_12kHz_CLKEN_PTR          ((reg8 *) clock_12kHz__PM_ACT_CFG)

/* Register to enable or disable the clock */
#define clock_12kHz_CLKSTBY            (* (reg8 *) clock_12kHz__PM_STBY_CFG)
#define clock_12kHz_CLKSTBY_PTR        ((reg8 *) clock_12kHz__PM_STBY_CFG)

/* Clock LSB divider configuration register. */
#define clock_12kHz_DIV_LSB            (* (reg8 *) clock_12kHz__CFG0)
#define clock_12kHz_DIV_LSB_PTR        ((reg8 *) clock_12kHz__CFG0)
#define clock_12kHz_DIV_PTR            ((reg16 *) clock_12kHz__CFG0)

/* Clock MSB divider configuration register. */
#define clock_12kHz_DIV_MSB            (* (reg8 *) clock_12kHz__CFG1)
#define clock_12kHz_DIV_MSB_PTR        ((reg8 *) clock_12kHz__CFG1)

/* Mode and source configuration register */
#define clock_12kHz_MOD_SRC            (* (reg8 *) clock_12kHz__CFG2)
#define clock_12kHz_MOD_SRC_PTR        ((reg8 *) clock_12kHz__CFG2)

#if defined(clock_12kHz__CFG3)
/* Analog clock phase configuration register */
#define clock_12kHz_PHASE              (* (reg8 *) clock_12kHz__CFG3)
#define clock_12kHz_PHASE_PTR          ((reg8 *) clock_12kHz__CFG3)
#endif /* defined(clock_12kHz__CFG3) */


/**************************************
*       Register Constants
**************************************/

/* Power manager register masks */
#define clock_12kHz_CLKEN_MASK         clock_12kHz__PM_ACT_MSK
#define clock_12kHz_CLKSTBY_MASK       clock_12kHz__PM_STBY_MSK

/* CFG2 field masks */
#define clock_12kHz_SRC_SEL_MSK        clock_12kHz__CFG2_SRC_SEL_MASK
#define clock_12kHz_MODE_MASK          (~(clock_12kHz_SRC_SEL_MSK))

#if defined(clock_12kHz__CFG3)
/* CFG3 phase mask */
#define clock_12kHz_PHASE_MASK         clock_12kHz__CFG3_PHASE_DLY_MASK
#endif /* defined(clock_12kHz__CFG3) */

#endif /* CY_CLOCK_clock_12kHz_H */


/* [] END OF FILE */
