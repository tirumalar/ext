/*******************************************************************************
* File Name: LEDR_OUT.h  
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

#if !defined(CY_PINS_LEDR_OUT_H) /* Pins LEDR_OUT_H */
#define CY_PINS_LEDR_OUT_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "LEDR_OUT_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 LEDR_OUT__PORT == 15 && ((LEDR_OUT__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    LEDR_OUT_Write(uint8 value);
void    LEDR_OUT_SetDriveMode(uint8 mode);
uint8   LEDR_OUT_ReadDataReg(void);
uint8   LEDR_OUT_Read(void);
void    LEDR_OUT_SetInterruptMode(uint16 position, uint16 mode);
uint8   LEDR_OUT_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the LEDR_OUT_SetDriveMode() function.
     *  @{
     */
        #define LEDR_OUT_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define LEDR_OUT_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define LEDR_OUT_DM_RES_UP          PIN_DM_RES_UP
        #define LEDR_OUT_DM_RES_DWN         PIN_DM_RES_DWN
        #define LEDR_OUT_DM_OD_LO           PIN_DM_OD_LO
        #define LEDR_OUT_DM_OD_HI           PIN_DM_OD_HI
        #define LEDR_OUT_DM_STRONG          PIN_DM_STRONG
        #define LEDR_OUT_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define LEDR_OUT_MASK               LEDR_OUT__MASK
#define LEDR_OUT_SHIFT              LEDR_OUT__SHIFT
#define LEDR_OUT_WIDTH              1u

/* Interrupt constants */
#if defined(LEDR_OUT__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in LEDR_OUT_SetInterruptMode() function.
     *  @{
     */
        #define LEDR_OUT_INTR_NONE      (uint16)(0x0000u)
        #define LEDR_OUT_INTR_RISING    (uint16)(0x0001u)
        #define LEDR_OUT_INTR_FALLING   (uint16)(0x0002u)
        #define LEDR_OUT_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define LEDR_OUT_INTR_MASK      (0x01u) 
#endif /* (LEDR_OUT__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define LEDR_OUT_PS                     (* (reg8 *) LEDR_OUT__PS)
/* Data Register */
#define LEDR_OUT_DR                     (* (reg8 *) LEDR_OUT__DR)
/* Port Number */
#define LEDR_OUT_PRT_NUM                (* (reg8 *) LEDR_OUT__PRT) 
/* Connect to Analog Globals */                                                  
#define LEDR_OUT_AG                     (* (reg8 *) LEDR_OUT__AG)                       
/* Analog MUX bux enable */
#define LEDR_OUT_AMUX                   (* (reg8 *) LEDR_OUT__AMUX) 
/* Bidirectional Enable */                                                        
#define LEDR_OUT_BIE                    (* (reg8 *) LEDR_OUT__BIE)
/* Bit-mask for Aliased Register Access */
#define LEDR_OUT_BIT_MASK               (* (reg8 *) LEDR_OUT__BIT_MASK)
/* Bypass Enable */
#define LEDR_OUT_BYP                    (* (reg8 *) LEDR_OUT__BYP)
/* Port wide control signals */                                                   
#define LEDR_OUT_CTL                    (* (reg8 *) LEDR_OUT__CTL)
/* Drive Modes */
#define LEDR_OUT_DM0                    (* (reg8 *) LEDR_OUT__DM0) 
#define LEDR_OUT_DM1                    (* (reg8 *) LEDR_OUT__DM1)
#define LEDR_OUT_DM2                    (* (reg8 *) LEDR_OUT__DM2) 
/* Input Buffer Disable Override */
#define LEDR_OUT_INP_DIS                (* (reg8 *) LEDR_OUT__INP_DIS)
/* LCD Common or Segment Drive */
#define LEDR_OUT_LCD_COM_SEG            (* (reg8 *) LEDR_OUT__LCD_COM_SEG)
/* Enable Segment LCD */
#define LEDR_OUT_LCD_EN                 (* (reg8 *) LEDR_OUT__LCD_EN)
/* Slew Rate Control */
#define LEDR_OUT_SLW                    (* (reg8 *) LEDR_OUT__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define LEDR_OUT_PRTDSI__CAPS_SEL       (* (reg8 *) LEDR_OUT__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define LEDR_OUT_PRTDSI__DBL_SYNC_IN    (* (reg8 *) LEDR_OUT__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define LEDR_OUT_PRTDSI__OE_SEL0        (* (reg8 *) LEDR_OUT__PRTDSI__OE_SEL0) 
#define LEDR_OUT_PRTDSI__OE_SEL1        (* (reg8 *) LEDR_OUT__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define LEDR_OUT_PRTDSI__OUT_SEL0       (* (reg8 *) LEDR_OUT__PRTDSI__OUT_SEL0) 
#define LEDR_OUT_PRTDSI__OUT_SEL1       (* (reg8 *) LEDR_OUT__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define LEDR_OUT_PRTDSI__SYNC_OUT       (* (reg8 *) LEDR_OUT__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(LEDR_OUT__SIO_CFG)
    #define LEDR_OUT_SIO_HYST_EN        (* (reg8 *) LEDR_OUT__SIO_HYST_EN)
    #define LEDR_OUT_SIO_REG_HIFREQ     (* (reg8 *) LEDR_OUT__SIO_REG_HIFREQ)
    #define LEDR_OUT_SIO_CFG            (* (reg8 *) LEDR_OUT__SIO_CFG)
    #define LEDR_OUT_SIO_DIFF           (* (reg8 *) LEDR_OUT__SIO_DIFF)
#endif /* (LEDR_OUT__SIO_CFG) */

/* Interrupt Registers */
#if defined(LEDR_OUT__INTSTAT)
    #define LEDR_OUT_INTSTAT            (* (reg8 *) LEDR_OUT__INTSTAT)
    #define LEDR_OUT_SNAP               (* (reg8 *) LEDR_OUT__SNAP)
    
	#define LEDR_OUT_0_INTTYPE_REG 		(* (reg8 *) LEDR_OUT__0__INTTYPE)
#endif /* (LEDR_OUT__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_LEDR_OUT_H */


/* [] END OF FILE */
