/*******************************************************************************
* File Name: PS_D1.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
* Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_PS_D1_H) /* Pins PS_D1_H */
#define CY_PINS_PS_D1_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "PS_D1_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 PS_D1__PORT == 15 && ((PS_D1__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    PS_D1_Write(uint8 value);
void    PS_D1_SetDriveMode(uint8 mode);
uint8   PS_D1_ReadDataReg(void);
uint8   PS_D1_Read(void);
void    PS_D1_SetInterruptMode(uint16 position, uint16 mode);
uint8   PS_D1_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the PS_D1_SetDriveMode() function.
     *  @{
     */
        #define PS_D1_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define PS_D1_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define PS_D1_DM_RES_UP          PIN_DM_RES_UP
        #define PS_D1_DM_RES_DWN         PIN_DM_RES_DWN
        #define PS_D1_DM_OD_LO           PIN_DM_OD_LO
        #define PS_D1_DM_OD_HI           PIN_DM_OD_HI
        #define PS_D1_DM_STRONG          PIN_DM_STRONG
        #define PS_D1_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define PS_D1_MASK               PS_D1__MASK
#define PS_D1_SHIFT              PS_D1__SHIFT
#define PS_D1_WIDTH              1u

/* Interrupt constants */
#if defined(PS_D1__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in PS_D1_SetInterruptMode() function.
     *  @{
     */
        #define PS_D1_INTR_NONE      (uint16)(0x0000u)
        #define PS_D1_INTR_RISING    (uint16)(0x0001u)
        #define PS_D1_INTR_FALLING   (uint16)(0x0002u)
        #define PS_D1_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define PS_D1_INTR_MASK      (0x01u) 
#endif /* (PS_D1__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define PS_D1_PS                     (* (reg8 *) PS_D1__PS)
/* Data Register */
#define PS_D1_DR                     (* (reg8 *) PS_D1__DR)
/* Port Number */
#define PS_D1_PRT_NUM                (* (reg8 *) PS_D1__PRT) 
/* Connect to Analog Globals */                                                  
#define PS_D1_AG                     (* (reg8 *) PS_D1__AG)                       
/* Analog MUX bux enable */
#define PS_D1_AMUX                   (* (reg8 *) PS_D1__AMUX) 
/* Bidirectional Enable */                                                        
#define PS_D1_BIE                    (* (reg8 *) PS_D1__BIE)
/* Bit-mask for Aliased Register Access */
#define PS_D1_BIT_MASK               (* (reg8 *) PS_D1__BIT_MASK)
/* Bypass Enable */
#define PS_D1_BYP                    (* (reg8 *) PS_D1__BYP)
/* Port wide control signals */                                                   
#define PS_D1_CTL                    (* (reg8 *) PS_D1__CTL)
/* Drive Modes */
#define PS_D1_DM0                    (* (reg8 *) PS_D1__DM0) 
#define PS_D1_DM1                    (* (reg8 *) PS_D1__DM1)
#define PS_D1_DM2                    (* (reg8 *) PS_D1__DM2) 
/* Input Buffer Disable Override */
#define PS_D1_INP_DIS                (* (reg8 *) PS_D1__INP_DIS)
/* LCD Common or Segment Drive */
#define PS_D1_LCD_COM_SEG            (* (reg8 *) PS_D1__LCD_COM_SEG)
/* Enable Segment LCD */
#define PS_D1_LCD_EN                 (* (reg8 *) PS_D1__LCD_EN)
/* Slew Rate Control */
#define PS_D1_SLW                    (* (reg8 *) PS_D1__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define PS_D1_PRTDSI__CAPS_SEL       (* (reg8 *) PS_D1__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define PS_D1_PRTDSI__DBL_SYNC_IN    (* (reg8 *) PS_D1__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define PS_D1_PRTDSI__OE_SEL0        (* (reg8 *) PS_D1__PRTDSI__OE_SEL0) 
#define PS_D1_PRTDSI__OE_SEL1        (* (reg8 *) PS_D1__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define PS_D1_PRTDSI__OUT_SEL0       (* (reg8 *) PS_D1__PRTDSI__OUT_SEL0) 
#define PS_D1_PRTDSI__OUT_SEL1       (* (reg8 *) PS_D1__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define PS_D1_PRTDSI__SYNC_OUT       (* (reg8 *) PS_D1__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(PS_D1__SIO_CFG)
    #define PS_D1_SIO_HYST_EN        (* (reg8 *) PS_D1__SIO_HYST_EN)
    #define PS_D1_SIO_REG_HIFREQ     (* (reg8 *) PS_D1__SIO_REG_HIFREQ)
    #define PS_D1_SIO_CFG            (* (reg8 *) PS_D1__SIO_CFG)
    #define PS_D1_SIO_DIFF           (* (reg8 *) PS_D1__SIO_DIFF)
#endif /* (PS_D1__SIO_CFG) */

/* Interrupt Registers */
#if defined(PS_D1__INTSTAT)
    #define PS_D1_INTSTAT            (* (reg8 *) PS_D1__INTSTAT)
    #define PS_D1_SNAP               (* (reg8 *) PS_D1__SNAP)
    
	#define PS_D1_0_INTTYPE_REG 		(* (reg8 *) PS_D1__0__INTTYPE)
#endif /* (PS_D1__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_PS_D1_H */


/* [] END OF FILE */
