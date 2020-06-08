/*******************************************************************************
* File Name: REED_2.h  
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

#if !defined(CY_PINS_REED_2_H) /* Pins REED_2_H */
#define CY_PINS_REED_2_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "REED_2_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 REED_2__PORT == 15 && ((REED_2__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    REED_2_Write(uint8 value);
void    REED_2_SetDriveMode(uint8 mode);
uint8   REED_2_ReadDataReg(void);
uint8   REED_2_Read(void);
void    REED_2_SetInterruptMode(uint16 position, uint16 mode);
uint8   REED_2_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the REED_2_SetDriveMode() function.
     *  @{
     */
        #define REED_2_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define REED_2_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define REED_2_DM_RES_UP          PIN_DM_RES_UP
        #define REED_2_DM_RES_DWN         PIN_DM_RES_DWN
        #define REED_2_DM_OD_LO           PIN_DM_OD_LO
        #define REED_2_DM_OD_HI           PIN_DM_OD_HI
        #define REED_2_DM_STRONG          PIN_DM_STRONG
        #define REED_2_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define REED_2_MASK               REED_2__MASK
#define REED_2_SHIFT              REED_2__SHIFT
#define REED_2_WIDTH              1u

/* Interrupt constants */
#if defined(REED_2__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in REED_2_SetInterruptMode() function.
     *  @{
     */
        #define REED_2_INTR_NONE      (uint16)(0x0000u)
        #define REED_2_INTR_RISING    (uint16)(0x0001u)
        #define REED_2_INTR_FALLING   (uint16)(0x0002u)
        #define REED_2_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define REED_2_INTR_MASK      (0x01u) 
#endif /* (REED_2__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define REED_2_PS                     (* (reg8 *) REED_2__PS)
/* Data Register */
#define REED_2_DR                     (* (reg8 *) REED_2__DR)
/* Port Number */
#define REED_2_PRT_NUM                (* (reg8 *) REED_2__PRT) 
/* Connect to Analog Globals */                                                  
#define REED_2_AG                     (* (reg8 *) REED_2__AG)                       
/* Analog MUX bux enable */
#define REED_2_AMUX                   (* (reg8 *) REED_2__AMUX) 
/* Bidirectional Enable */                                                        
#define REED_2_BIE                    (* (reg8 *) REED_2__BIE)
/* Bit-mask for Aliased Register Access */
#define REED_2_BIT_MASK               (* (reg8 *) REED_2__BIT_MASK)
/* Bypass Enable */
#define REED_2_BYP                    (* (reg8 *) REED_2__BYP)
/* Port wide control signals */                                                   
#define REED_2_CTL                    (* (reg8 *) REED_2__CTL)
/* Drive Modes */
#define REED_2_DM0                    (* (reg8 *) REED_2__DM0) 
#define REED_2_DM1                    (* (reg8 *) REED_2__DM1)
#define REED_2_DM2                    (* (reg8 *) REED_2__DM2) 
/* Input Buffer Disable Override */
#define REED_2_INP_DIS                (* (reg8 *) REED_2__INP_DIS)
/* LCD Common or Segment Drive */
#define REED_2_LCD_COM_SEG            (* (reg8 *) REED_2__LCD_COM_SEG)
/* Enable Segment LCD */
#define REED_2_LCD_EN                 (* (reg8 *) REED_2__LCD_EN)
/* Slew Rate Control */
#define REED_2_SLW                    (* (reg8 *) REED_2__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define REED_2_PRTDSI__CAPS_SEL       (* (reg8 *) REED_2__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define REED_2_PRTDSI__DBL_SYNC_IN    (* (reg8 *) REED_2__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define REED_2_PRTDSI__OE_SEL0        (* (reg8 *) REED_2__PRTDSI__OE_SEL0) 
#define REED_2_PRTDSI__OE_SEL1        (* (reg8 *) REED_2__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define REED_2_PRTDSI__OUT_SEL0       (* (reg8 *) REED_2__PRTDSI__OUT_SEL0) 
#define REED_2_PRTDSI__OUT_SEL1       (* (reg8 *) REED_2__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define REED_2_PRTDSI__SYNC_OUT       (* (reg8 *) REED_2__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(REED_2__SIO_CFG)
    #define REED_2_SIO_HYST_EN        (* (reg8 *) REED_2__SIO_HYST_EN)
    #define REED_2_SIO_REG_HIFREQ     (* (reg8 *) REED_2__SIO_REG_HIFREQ)
    #define REED_2_SIO_CFG            (* (reg8 *) REED_2__SIO_CFG)
    #define REED_2_SIO_DIFF           (* (reg8 *) REED_2__SIO_DIFF)
#endif /* (REED_2__SIO_CFG) */

/* Interrupt Registers */
#if defined(REED_2__INTSTAT)
    #define REED_2_INTSTAT            (* (reg8 *) REED_2__INTSTAT)
    #define REED_2_SNAP               (* (reg8 *) REED_2__SNAP)
    
	#define REED_2_0_INTTYPE_REG 		(* (reg8 *) REED_2__0__INTTYPE)
#endif /* (REED_2__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_REED_2_H */


/* [] END OF FILE */
