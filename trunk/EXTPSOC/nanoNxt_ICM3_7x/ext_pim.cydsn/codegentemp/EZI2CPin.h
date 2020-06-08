/*******************************************************************************
* File Name: EZI2CPin.h  
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

#if !defined(CY_PINS_EZI2CPin_H) /* Pins EZI2CPin_H */
#define CY_PINS_EZI2CPin_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "EZI2CPin_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 EZI2CPin__PORT == 15 && ((EZI2CPin__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    EZI2CPin_Write(uint8 value);
void    EZI2CPin_SetDriveMode(uint8 mode);
uint8   EZI2CPin_ReadDataReg(void);
uint8   EZI2CPin_Read(void);
void    EZI2CPin_SetInterruptMode(uint16 position, uint16 mode);
uint8   EZI2CPin_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the EZI2CPin_SetDriveMode() function.
     *  @{
     */
        #define EZI2CPin_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define EZI2CPin_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define EZI2CPin_DM_RES_UP          PIN_DM_RES_UP
        #define EZI2CPin_DM_RES_DWN         PIN_DM_RES_DWN
        #define EZI2CPin_DM_OD_LO           PIN_DM_OD_LO
        #define EZI2CPin_DM_OD_HI           PIN_DM_OD_HI
        #define EZI2CPin_DM_STRONG          PIN_DM_STRONG
        #define EZI2CPin_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define EZI2CPin_MASK               EZI2CPin__MASK
#define EZI2CPin_SHIFT              EZI2CPin__SHIFT
#define EZI2CPin_WIDTH              2u

/* Interrupt constants */
#if defined(EZI2CPin__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in EZI2CPin_SetInterruptMode() function.
     *  @{
     */
        #define EZI2CPin_INTR_NONE      (uint16)(0x0000u)
        #define EZI2CPin_INTR_RISING    (uint16)(0x0001u)
        #define EZI2CPin_INTR_FALLING   (uint16)(0x0002u)
        #define EZI2CPin_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define EZI2CPin_INTR_MASK      (0x01u) 
#endif /* (EZI2CPin__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define EZI2CPin_PS                     (* (reg8 *) EZI2CPin__PS)
/* Data Register */
#define EZI2CPin_DR                     (* (reg8 *) EZI2CPin__DR)
/* Port Number */
#define EZI2CPin_PRT_NUM                (* (reg8 *) EZI2CPin__PRT) 
/* Connect to Analog Globals */                                                  
#define EZI2CPin_AG                     (* (reg8 *) EZI2CPin__AG)                       
/* Analog MUX bux enable */
#define EZI2CPin_AMUX                   (* (reg8 *) EZI2CPin__AMUX) 
/* Bidirectional Enable */                                                        
#define EZI2CPin_BIE                    (* (reg8 *) EZI2CPin__BIE)
/* Bit-mask for Aliased Register Access */
#define EZI2CPin_BIT_MASK               (* (reg8 *) EZI2CPin__BIT_MASK)
/* Bypass Enable */
#define EZI2CPin_BYP                    (* (reg8 *) EZI2CPin__BYP)
/* Port wide control signals */                                                   
#define EZI2CPin_CTL                    (* (reg8 *) EZI2CPin__CTL)
/* Drive Modes */
#define EZI2CPin_DM0                    (* (reg8 *) EZI2CPin__DM0) 
#define EZI2CPin_DM1                    (* (reg8 *) EZI2CPin__DM1)
#define EZI2CPin_DM2                    (* (reg8 *) EZI2CPin__DM2) 
/* Input Buffer Disable Override */
#define EZI2CPin_INP_DIS                (* (reg8 *) EZI2CPin__INP_DIS)
/* LCD Common or Segment Drive */
#define EZI2CPin_LCD_COM_SEG            (* (reg8 *) EZI2CPin__LCD_COM_SEG)
/* Enable Segment LCD */
#define EZI2CPin_LCD_EN                 (* (reg8 *) EZI2CPin__LCD_EN)
/* Slew Rate Control */
#define EZI2CPin_SLW                    (* (reg8 *) EZI2CPin__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define EZI2CPin_PRTDSI__CAPS_SEL       (* (reg8 *) EZI2CPin__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define EZI2CPin_PRTDSI__DBL_SYNC_IN    (* (reg8 *) EZI2CPin__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define EZI2CPin_PRTDSI__OE_SEL0        (* (reg8 *) EZI2CPin__PRTDSI__OE_SEL0) 
#define EZI2CPin_PRTDSI__OE_SEL1        (* (reg8 *) EZI2CPin__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define EZI2CPin_PRTDSI__OUT_SEL0       (* (reg8 *) EZI2CPin__PRTDSI__OUT_SEL0) 
#define EZI2CPin_PRTDSI__OUT_SEL1       (* (reg8 *) EZI2CPin__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define EZI2CPin_PRTDSI__SYNC_OUT       (* (reg8 *) EZI2CPin__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(EZI2CPin__SIO_CFG)
    #define EZI2CPin_SIO_HYST_EN        (* (reg8 *) EZI2CPin__SIO_HYST_EN)
    #define EZI2CPin_SIO_REG_HIFREQ     (* (reg8 *) EZI2CPin__SIO_REG_HIFREQ)
    #define EZI2CPin_SIO_CFG            (* (reg8 *) EZI2CPin__SIO_CFG)
    #define EZI2CPin_SIO_DIFF           (* (reg8 *) EZI2CPin__SIO_DIFF)
#endif /* (EZI2CPin__SIO_CFG) */

/* Interrupt Registers */
#if defined(EZI2CPin__INTSTAT)
    #define EZI2CPin_INTSTAT            (* (reg8 *) EZI2CPin__INTSTAT)
    #define EZI2CPin_SNAP               (* (reg8 *) EZI2CPin__SNAP)
    
	#define EZI2CPin_0_INTTYPE_REG 		(* (reg8 *) EZI2CPin__0__INTTYPE)
	#define EZI2CPin_1_INTTYPE_REG 		(* (reg8 *) EZI2CPin__1__INTTYPE)
#endif /* (EZI2CPin__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_EZI2CPin_H */


/* [] END OF FILE */
