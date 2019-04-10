/*******************************************************************************
* File Name: RELAY_A.h  
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

#if !defined(CY_PINS_RELAY_A_H) /* Pins RELAY_A_H */
#define CY_PINS_RELAY_A_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "RELAY_A_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 RELAY_A__PORT == 15 && ((RELAY_A__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    RELAY_A_Write(uint8 value);
void    RELAY_A_SetDriveMode(uint8 mode);
uint8   RELAY_A_ReadDataReg(void);
uint8   RELAY_A_Read(void);
void    RELAY_A_SetInterruptMode(uint16 position, uint16 mode);
uint8   RELAY_A_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the RELAY_A_SetDriveMode() function.
     *  @{
     */
        #define RELAY_A_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define RELAY_A_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define RELAY_A_DM_RES_UP          PIN_DM_RES_UP
        #define RELAY_A_DM_RES_DWN         PIN_DM_RES_DWN
        #define RELAY_A_DM_OD_LO           PIN_DM_OD_LO
        #define RELAY_A_DM_OD_HI           PIN_DM_OD_HI
        #define RELAY_A_DM_STRONG          PIN_DM_STRONG
        #define RELAY_A_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define RELAY_A_MASK               RELAY_A__MASK
#define RELAY_A_SHIFT              RELAY_A__SHIFT
#define RELAY_A_WIDTH              1u

/* Interrupt constants */
#if defined(RELAY_A__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in RELAY_A_SetInterruptMode() function.
     *  @{
     */
        #define RELAY_A_INTR_NONE      (uint16)(0x0000u)
        #define RELAY_A_INTR_RISING    (uint16)(0x0001u)
        #define RELAY_A_INTR_FALLING   (uint16)(0x0002u)
        #define RELAY_A_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define RELAY_A_INTR_MASK      (0x01u) 
#endif /* (RELAY_A__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define RELAY_A_PS                     (* (reg8 *) RELAY_A__PS)
/* Data Register */
#define RELAY_A_DR                     (* (reg8 *) RELAY_A__DR)
/* Port Number */
#define RELAY_A_PRT_NUM                (* (reg8 *) RELAY_A__PRT) 
/* Connect to Analog Globals */                                                  
#define RELAY_A_AG                     (* (reg8 *) RELAY_A__AG)                       
/* Analog MUX bux enable */
#define RELAY_A_AMUX                   (* (reg8 *) RELAY_A__AMUX) 
/* Bidirectional Enable */                                                        
#define RELAY_A_BIE                    (* (reg8 *) RELAY_A__BIE)
/* Bit-mask for Aliased Register Access */
#define RELAY_A_BIT_MASK               (* (reg8 *) RELAY_A__BIT_MASK)
/* Bypass Enable */
#define RELAY_A_BYP                    (* (reg8 *) RELAY_A__BYP)
/* Port wide control signals */                                                   
#define RELAY_A_CTL                    (* (reg8 *) RELAY_A__CTL)
/* Drive Modes */
#define RELAY_A_DM0                    (* (reg8 *) RELAY_A__DM0) 
#define RELAY_A_DM1                    (* (reg8 *) RELAY_A__DM1)
#define RELAY_A_DM2                    (* (reg8 *) RELAY_A__DM2) 
/* Input Buffer Disable Override */
#define RELAY_A_INP_DIS                (* (reg8 *) RELAY_A__INP_DIS)
/* LCD Common or Segment Drive */
#define RELAY_A_LCD_COM_SEG            (* (reg8 *) RELAY_A__LCD_COM_SEG)
/* Enable Segment LCD */
#define RELAY_A_LCD_EN                 (* (reg8 *) RELAY_A__LCD_EN)
/* Slew Rate Control */
#define RELAY_A_SLW                    (* (reg8 *) RELAY_A__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define RELAY_A_PRTDSI__CAPS_SEL       (* (reg8 *) RELAY_A__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define RELAY_A_PRTDSI__DBL_SYNC_IN    (* (reg8 *) RELAY_A__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define RELAY_A_PRTDSI__OE_SEL0        (* (reg8 *) RELAY_A__PRTDSI__OE_SEL0) 
#define RELAY_A_PRTDSI__OE_SEL1        (* (reg8 *) RELAY_A__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define RELAY_A_PRTDSI__OUT_SEL0       (* (reg8 *) RELAY_A__PRTDSI__OUT_SEL0) 
#define RELAY_A_PRTDSI__OUT_SEL1       (* (reg8 *) RELAY_A__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define RELAY_A_PRTDSI__SYNC_OUT       (* (reg8 *) RELAY_A__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(RELAY_A__SIO_CFG)
    #define RELAY_A_SIO_HYST_EN        (* (reg8 *) RELAY_A__SIO_HYST_EN)
    #define RELAY_A_SIO_REG_HIFREQ     (* (reg8 *) RELAY_A__SIO_REG_HIFREQ)
    #define RELAY_A_SIO_CFG            (* (reg8 *) RELAY_A__SIO_CFG)
    #define RELAY_A_SIO_DIFF           (* (reg8 *) RELAY_A__SIO_DIFF)
#endif /* (RELAY_A__SIO_CFG) */

/* Interrupt Registers */
#if defined(RELAY_A__INTSTAT)
    #define RELAY_A_INTSTAT            (* (reg8 *) RELAY_A__INTSTAT)
    #define RELAY_A_SNAP               (* (reg8 *) RELAY_A__SNAP)
    
	#define RELAY_A_0_INTTYPE_REG 		(* (reg8 *) RELAY_A__0__INTTYPE)
#endif /* (RELAY_A__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_RELAY_A_H */


/* [] END OF FILE */
