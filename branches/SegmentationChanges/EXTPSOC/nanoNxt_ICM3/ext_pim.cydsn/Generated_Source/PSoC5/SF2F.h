/*******************************************************************************
* File Name: SF2F.h  
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

#if !defined(CY_PINS_SF2F_H) /* Pins SF2F_H */
#define CY_PINS_SF2F_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "SF2F_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 SF2F__PORT == 15 && ((SF2F__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    SF2F_Write(uint8 value);
void    SF2F_SetDriveMode(uint8 mode);
uint8   SF2F_ReadDataReg(void);
uint8   SF2F_Read(void);
void    SF2F_SetInterruptMode(uint16 position, uint16 mode);
uint8   SF2F_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the SF2F_SetDriveMode() function.
     *  @{
     */
        #define SF2F_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define SF2F_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define SF2F_DM_RES_UP          PIN_DM_RES_UP
        #define SF2F_DM_RES_DWN         PIN_DM_RES_DWN
        #define SF2F_DM_OD_LO           PIN_DM_OD_LO
        #define SF2F_DM_OD_HI           PIN_DM_OD_HI
        #define SF2F_DM_STRONG          PIN_DM_STRONG
        #define SF2F_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define SF2F_MASK               SF2F__MASK
#define SF2F_SHIFT              SF2F__SHIFT
#define SF2F_WIDTH              1u

/* Interrupt constants */
#if defined(SF2F__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in SF2F_SetInterruptMode() function.
     *  @{
     */
        #define SF2F_INTR_NONE      (uint16)(0x0000u)
        #define SF2F_INTR_RISING    (uint16)(0x0001u)
        #define SF2F_INTR_FALLING   (uint16)(0x0002u)
        #define SF2F_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define SF2F_INTR_MASK      (0x01u) 
#endif /* (SF2F__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define SF2F_PS                     (* (reg8 *) SF2F__PS)
/* Data Register */
#define SF2F_DR                     (* (reg8 *) SF2F__DR)
/* Port Number */
#define SF2F_PRT_NUM                (* (reg8 *) SF2F__PRT) 
/* Connect to Analog Globals */                                                  
#define SF2F_AG                     (* (reg8 *) SF2F__AG)                       
/* Analog MUX bux enable */
#define SF2F_AMUX                   (* (reg8 *) SF2F__AMUX) 
/* Bidirectional Enable */                                                        
#define SF2F_BIE                    (* (reg8 *) SF2F__BIE)
/* Bit-mask for Aliased Register Access */
#define SF2F_BIT_MASK               (* (reg8 *) SF2F__BIT_MASK)
/* Bypass Enable */
#define SF2F_BYP                    (* (reg8 *) SF2F__BYP)
/* Port wide control signals */                                                   
#define SF2F_CTL                    (* (reg8 *) SF2F__CTL)
/* Drive Modes */
#define SF2F_DM0                    (* (reg8 *) SF2F__DM0) 
#define SF2F_DM1                    (* (reg8 *) SF2F__DM1)
#define SF2F_DM2                    (* (reg8 *) SF2F__DM2) 
/* Input Buffer Disable Override */
#define SF2F_INP_DIS                (* (reg8 *) SF2F__INP_DIS)
/* LCD Common or Segment Drive */
#define SF2F_LCD_COM_SEG            (* (reg8 *) SF2F__LCD_COM_SEG)
/* Enable Segment LCD */
#define SF2F_LCD_EN                 (* (reg8 *) SF2F__LCD_EN)
/* Slew Rate Control */
#define SF2F_SLW                    (* (reg8 *) SF2F__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define SF2F_PRTDSI__CAPS_SEL       (* (reg8 *) SF2F__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define SF2F_PRTDSI__DBL_SYNC_IN    (* (reg8 *) SF2F__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define SF2F_PRTDSI__OE_SEL0        (* (reg8 *) SF2F__PRTDSI__OE_SEL0) 
#define SF2F_PRTDSI__OE_SEL1        (* (reg8 *) SF2F__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define SF2F_PRTDSI__OUT_SEL0       (* (reg8 *) SF2F__PRTDSI__OUT_SEL0) 
#define SF2F_PRTDSI__OUT_SEL1       (* (reg8 *) SF2F__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define SF2F_PRTDSI__SYNC_OUT       (* (reg8 *) SF2F__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(SF2F__SIO_CFG)
    #define SF2F_SIO_HYST_EN        (* (reg8 *) SF2F__SIO_HYST_EN)
    #define SF2F_SIO_REG_HIFREQ     (* (reg8 *) SF2F__SIO_REG_HIFREQ)
    #define SF2F_SIO_CFG            (* (reg8 *) SF2F__SIO_CFG)
    #define SF2F_SIO_DIFF           (* (reg8 *) SF2F__SIO_DIFF)
#endif /* (SF2F__SIO_CFG) */

/* Interrupt Registers */
#if defined(SF2F__INTSTAT)
    #define SF2F_INTSTAT            (* (reg8 *) SF2F__INTSTAT)
    #define SF2F_SNAP               (* (reg8 *) SF2F__SNAP)
    
	#define SF2F_0_INTTYPE_REG 		(* (reg8 *) SF2F__0__INTTYPE)
#endif /* (SF2F__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_SF2F_H */


/* [] END OF FILE */
