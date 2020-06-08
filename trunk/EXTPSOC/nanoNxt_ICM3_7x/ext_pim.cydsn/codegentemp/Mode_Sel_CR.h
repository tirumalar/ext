/*******************************************************************************
* File Name: Mode_Sel_CR.h  
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

#if !defined(CY_PINS_Mode_Sel_CR_H) /* Pins Mode_Sel_CR_H */
#define CY_PINS_Mode_Sel_CR_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "Mode_Sel_CR_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 Mode_Sel_CR__PORT == 15 && ((Mode_Sel_CR__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    Mode_Sel_CR_Write(uint8 value);
void    Mode_Sel_CR_SetDriveMode(uint8 mode);
uint8   Mode_Sel_CR_ReadDataReg(void);
uint8   Mode_Sel_CR_Read(void);
void    Mode_Sel_CR_SetInterruptMode(uint16 position, uint16 mode);
uint8   Mode_Sel_CR_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the Mode_Sel_CR_SetDriveMode() function.
     *  @{
     */
        #define Mode_Sel_CR_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define Mode_Sel_CR_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define Mode_Sel_CR_DM_RES_UP          PIN_DM_RES_UP
        #define Mode_Sel_CR_DM_RES_DWN         PIN_DM_RES_DWN
        #define Mode_Sel_CR_DM_OD_LO           PIN_DM_OD_LO
        #define Mode_Sel_CR_DM_OD_HI           PIN_DM_OD_HI
        #define Mode_Sel_CR_DM_STRONG          PIN_DM_STRONG
        #define Mode_Sel_CR_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define Mode_Sel_CR_MASK               Mode_Sel_CR__MASK
#define Mode_Sel_CR_SHIFT              Mode_Sel_CR__SHIFT
#define Mode_Sel_CR_WIDTH              1u

/* Interrupt constants */
#if defined(Mode_Sel_CR__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in Mode_Sel_CR_SetInterruptMode() function.
     *  @{
     */
        #define Mode_Sel_CR_INTR_NONE      (uint16)(0x0000u)
        #define Mode_Sel_CR_INTR_RISING    (uint16)(0x0001u)
        #define Mode_Sel_CR_INTR_FALLING   (uint16)(0x0002u)
        #define Mode_Sel_CR_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define Mode_Sel_CR_INTR_MASK      (0x01u) 
#endif /* (Mode_Sel_CR__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define Mode_Sel_CR_PS                     (* (reg8 *) Mode_Sel_CR__PS)
/* Data Register */
#define Mode_Sel_CR_DR                     (* (reg8 *) Mode_Sel_CR__DR)
/* Port Number */
#define Mode_Sel_CR_PRT_NUM                (* (reg8 *) Mode_Sel_CR__PRT) 
/* Connect to Analog Globals */                                                  
#define Mode_Sel_CR_AG                     (* (reg8 *) Mode_Sel_CR__AG)                       
/* Analog MUX bux enable */
#define Mode_Sel_CR_AMUX                   (* (reg8 *) Mode_Sel_CR__AMUX) 
/* Bidirectional Enable */                                                        
#define Mode_Sel_CR_BIE                    (* (reg8 *) Mode_Sel_CR__BIE)
/* Bit-mask for Aliased Register Access */
#define Mode_Sel_CR_BIT_MASK               (* (reg8 *) Mode_Sel_CR__BIT_MASK)
/* Bypass Enable */
#define Mode_Sel_CR_BYP                    (* (reg8 *) Mode_Sel_CR__BYP)
/* Port wide control signals */                                                   
#define Mode_Sel_CR_CTL                    (* (reg8 *) Mode_Sel_CR__CTL)
/* Drive Modes */
#define Mode_Sel_CR_DM0                    (* (reg8 *) Mode_Sel_CR__DM0) 
#define Mode_Sel_CR_DM1                    (* (reg8 *) Mode_Sel_CR__DM1)
#define Mode_Sel_CR_DM2                    (* (reg8 *) Mode_Sel_CR__DM2) 
/* Input Buffer Disable Override */
#define Mode_Sel_CR_INP_DIS                (* (reg8 *) Mode_Sel_CR__INP_DIS)
/* LCD Common or Segment Drive */
#define Mode_Sel_CR_LCD_COM_SEG            (* (reg8 *) Mode_Sel_CR__LCD_COM_SEG)
/* Enable Segment LCD */
#define Mode_Sel_CR_LCD_EN                 (* (reg8 *) Mode_Sel_CR__LCD_EN)
/* Slew Rate Control */
#define Mode_Sel_CR_SLW                    (* (reg8 *) Mode_Sel_CR__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define Mode_Sel_CR_PRTDSI__CAPS_SEL       (* (reg8 *) Mode_Sel_CR__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define Mode_Sel_CR_PRTDSI__DBL_SYNC_IN    (* (reg8 *) Mode_Sel_CR__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define Mode_Sel_CR_PRTDSI__OE_SEL0        (* (reg8 *) Mode_Sel_CR__PRTDSI__OE_SEL0) 
#define Mode_Sel_CR_PRTDSI__OE_SEL1        (* (reg8 *) Mode_Sel_CR__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define Mode_Sel_CR_PRTDSI__OUT_SEL0       (* (reg8 *) Mode_Sel_CR__PRTDSI__OUT_SEL0) 
#define Mode_Sel_CR_PRTDSI__OUT_SEL1       (* (reg8 *) Mode_Sel_CR__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define Mode_Sel_CR_PRTDSI__SYNC_OUT       (* (reg8 *) Mode_Sel_CR__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(Mode_Sel_CR__SIO_CFG)
    #define Mode_Sel_CR_SIO_HYST_EN        (* (reg8 *) Mode_Sel_CR__SIO_HYST_EN)
    #define Mode_Sel_CR_SIO_REG_HIFREQ     (* (reg8 *) Mode_Sel_CR__SIO_REG_HIFREQ)
    #define Mode_Sel_CR_SIO_CFG            (* (reg8 *) Mode_Sel_CR__SIO_CFG)
    #define Mode_Sel_CR_SIO_DIFF           (* (reg8 *) Mode_Sel_CR__SIO_DIFF)
#endif /* (Mode_Sel_CR__SIO_CFG) */

/* Interrupt Registers */
#if defined(Mode_Sel_CR__INTSTAT)
    #define Mode_Sel_CR_INTSTAT            (* (reg8 *) Mode_Sel_CR__INTSTAT)
    #define Mode_Sel_CR_SNAP               (* (reg8 *) Mode_Sel_CR__SNAP)
    
	#define Mode_Sel_CR_0_INTTYPE_REG 		(* (reg8 *) Mode_Sel_CR__0__INTTYPE)
#endif /* (Mode_Sel_CR__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_Mode_Sel_CR_H */


/* [] END OF FILE */
