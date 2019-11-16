/*******************************************************************************
* File Name: CR_DE.h  
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

#if !defined(CY_PINS_CR_DE_H) /* Pins CR_DE_H */
#define CY_PINS_CR_DE_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "CR_DE_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 CR_DE__PORT == 15 && ((CR_DE__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    CR_DE_Write(uint8 value);
void    CR_DE_SetDriveMode(uint8 mode);
uint8   CR_DE_ReadDataReg(void);
uint8   CR_DE_Read(void);
void    CR_DE_SetInterruptMode(uint16 position, uint16 mode);
uint8   CR_DE_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the CR_DE_SetDriveMode() function.
     *  @{
     */
        #define CR_DE_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define CR_DE_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define CR_DE_DM_RES_UP          PIN_DM_RES_UP
        #define CR_DE_DM_RES_DWN         PIN_DM_RES_DWN
        #define CR_DE_DM_OD_LO           PIN_DM_OD_LO
        #define CR_DE_DM_OD_HI           PIN_DM_OD_HI
        #define CR_DE_DM_STRONG          PIN_DM_STRONG
        #define CR_DE_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define CR_DE_MASK               CR_DE__MASK
#define CR_DE_SHIFT              CR_DE__SHIFT
#define CR_DE_WIDTH              1u

/* Interrupt constants */
#if defined(CR_DE__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in CR_DE_SetInterruptMode() function.
     *  @{
     */
        #define CR_DE_INTR_NONE      (uint16)(0x0000u)
        #define CR_DE_INTR_RISING    (uint16)(0x0001u)
        #define CR_DE_INTR_FALLING   (uint16)(0x0002u)
        #define CR_DE_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define CR_DE_INTR_MASK      (0x01u) 
#endif /* (CR_DE__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define CR_DE_PS                     (* (reg8 *) CR_DE__PS)
/* Data Register */
#define CR_DE_DR                     (* (reg8 *) CR_DE__DR)
/* Port Number */
#define CR_DE_PRT_NUM                (* (reg8 *) CR_DE__PRT) 
/* Connect to Analog Globals */                                                  
#define CR_DE_AG                     (* (reg8 *) CR_DE__AG)                       
/* Analog MUX bux enable */
#define CR_DE_AMUX                   (* (reg8 *) CR_DE__AMUX) 
/* Bidirectional Enable */                                                        
#define CR_DE_BIE                    (* (reg8 *) CR_DE__BIE)
/* Bit-mask for Aliased Register Access */
#define CR_DE_BIT_MASK               (* (reg8 *) CR_DE__BIT_MASK)
/* Bypass Enable */
#define CR_DE_BYP                    (* (reg8 *) CR_DE__BYP)
/* Port wide control signals */                                                   
#define CR_DE_CTL                    (* (reg8 *) CR_DE__CTL)
/* Drive Modes */
#define CR_DE_DM0                    (* (reg8 *) CR_DE__DM0) 
#define CR_DE_DM1                    (* (reg8 *) CR_DE__DM1)
#define CR_DE_DM2                    (* (reg8 *) CR_DE__DM2) 
/* Input Buffer Disable Override */
#define CR_DE_INP_DIS                (* (reg8 *) CR_DE__INP_DIS)
/* LCD Common or Segment Drive */
#define CR_DE_LCD_COM_SEG            (* (reg8 *) CR_DE__LCD_COM_SEG)
/* Enable Segment LCD */
#define CR_DE_LCD_EN                 (* (reg8 *) CR_DE__LCD_EN)
/* Slew Rate Control */
#define CR_DE_SLW                    (* (reg8 *) CR_DE__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define CR_DE_PRTDSI__CAPS_SEL       (* (reg8 *) CR_DE__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define CR_DE_PRTDSI__DBL_SYNC_IN    (* (reg8 *) CR_DE__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define CR_DE_PRTDSI__OE_SEL0        (* (reg8 *) CR_DE__PRTDSI__OE_SEL0) 
#define CR_DE_PRTDSI__OE_SEL1        (* (reg8 *) CR_DE__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define CR_DE_PRTDSI__OUT_SEL0       (* (reg8 *) CR_DE__PRTDSI__OUT_SEL0) 
#define CR_DE_PRTDSI__OUT_SEL1       (* (reg8 *) CR_DE__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define CR_DE_PRTDSI__SYNC_OUT       (* (reg8 *) CR_DE__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(CR_DE__SIO_CFG)
    #define CR_DE_SIO_HYST_EN        (* (reg8 *) CR_DE__SIO_HYST_EN)
    #define CR_DE_SIO_REG_HIFREQ     (* (reg8 *) CR_DE__SIO_REG_HIFREQ)
    #define CR_DE_SIO_CFG            (* (reg8 *) CR_DE__SIO_CFG)
    #define CR_DE_SIO_DIFF           (* (reg8 *) CR_DE__SIO_DIFF)
#endif /* (CR_DE__SIO_CFG) */

/* Interrupt Registers */
#if defined(CR_DE__INTSTAT)
    #define CR_DE_INTSTAT            (* (reg8 *) CR_DE__INTSTAT)
    #define CR_DE_SNAP               (* (reg8 *) CR_DE__SNAP)
    
	#define CR_DE_0_INTTYPE_REG 		(* (reg8 *) CR_DE__0__INTTYPE)
#endif /* (CR_DE__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_CR_DE_H */


/* [] END OF FILE */
