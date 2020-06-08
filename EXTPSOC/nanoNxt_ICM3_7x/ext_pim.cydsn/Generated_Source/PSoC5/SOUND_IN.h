/*******************************************************************************
* File Name: SOUND_IN.h  
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

#if !defined(CY_PINS_SOUND_IN_H) /* Pins SOUND_IN_H */
#define CY_PINS_SOUND_IN_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "SOUND_IN_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 SOUND_IN__PORT == 15 && ((SOUND_IN__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    SOUND_IN_Write(uint8 value);
void    SOUND_IN_SetDriveMode(uint8 mode);
uint8   SOUND_IN_ReadDataReg(void);
uint8   SOUND_IN_Read(void);
void    SOUND_IN_SetInterruptMode(uint16 position, uint16 mode);
uint8   SOUND_IN_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the SOUND_IN_SetDriveMode() function.
     *  @{
     */
        #define SOUND_IN_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define SOUND_IN_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define SOUND_IN_DM_RES_UP          PIN_DM_RES_UP
        #define SOUND_IN_DM_RES_DWN         PIN_DM_RES_DWN
        #define SOUND_IN_DM_OD_LO           PIN_DM_OD_LO
        #define SOUND_IN_DM_OD_HI           PIN_DM_OD_HI
        #define SOUND_IN_DM_STRONG          PIN_DM_STRONG
        #define SOUND_IN_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define SOUND_IN_MASK               SOUND_IN__MASK
#define SOUND_IN_SHIFT              SOUND_IN__SHIFT
#define SOUND_IN_WIDTH              1u

/* Interrupt constants */
#if defined(SOUND_IN__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in SOUND_IN_SetInterruptMode() function.
     *  @{
     */
        #define SOUND_IN_INTR_NONE      (uint16)(0x0000u)
        #define SOUND_IN_INTR_RISING    (uint16)(0x0001u)
        #define SOUND_IN_INTR_FALLING   (uint16)(0x0002u)
        #define SOUND_IN_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define SOUND_IN_INTR_MASK      (0x01u) 
#endif /* (SOUND_IN__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define SOUND_IN_PS                     (* (reg8 *) SOUND_IN__PS)
/* Data Register */
#define SOUND_IN_DR                     (* (reg8 *) SOUND_IN__DR)
/* Port Number */
#define SOUND_IN_PRT_NUM                (* (reg8 *) SOUND_IN__PRT) 
/* Connect to Analog Globals */                                                  
#define SOUND_IN_AG                     (* (reg8 *) SOUND_IN__AG)                       
/* Analog MUX bux enable */
#define SOUND_IN_AMUX                   (* (reg8 *) SOUND_IN__AMUX) 
/* Bidirectional Enable */                                                        
#define SOUND_IN_BIE                    (* (reg8 *) SOUND_IN__BIE)
/* Bit-mask for Aliased Register Access */
#define SOUND_IN_BIT_MASK               (* (reg8 *) SOUND_IN__BIT_MASK)
/* Bypass Enable */
#define SOUND_IN_BYP                    (* (reg8 *) SOUND_IN__BYP)
/* Port wide control signals */                                                   
#define SOUND_IN_CTL                    (* (reg8 *) SOUND_IN__CTL)
/* Drive Modes */
#define SOUND_IN_DM0                    (* (reg8 *) SOUND_IN__DM0) 
#define SOUND_IN_DM1                    (* (reg8 *) SOUND_IN__DM1)
#define SOUND_IN_DM2                    (* (reg8 *) SOUND_IN__DM2) 
/* Input Buffer Disable Override */
#define SOUND_IN_INP_DIS                (* (reg8 *) SOUND_IN__INP_DIS)
/* LCD Common or Segment Drive */
#define SOUND_IN_LCD_COM_SEG            (* (reg8 *) SOUND_IN__LCD_COM_SEG)
/* Enable Segment LCD */
#define SOUND_IN_LCD_EN                 (* (reg8 *) SOUND_IN__LCD_EN)
/* Slew Rate Control */
#define SOUND_IN_SLW                    (* (reg8 *) SOUND_IN__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define SOUND_IN_PRTDSI__CAPS_SEL       (* (reg8 *) SOUND_IN__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define SOUND_IN_PRTDSI__DBL_SYNC_IN    (* (reg8 *) SOUND_IN__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define SOUND_IN_PRTDSI__OE_SEL0        (* (reg8 *) SOUND_IN__PRTDSI__OE_SEL0) 
#define SOUND_IN_PRTDSI__OE_SEL1        (* (reg8 *) SOUND_IN__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define SOUND_IN_PRTDSI__OUT_SEL0       (* (reg8 *) SOUND_IN__PRTDSI__OUT_SEL0) 
#define SOUND_IN_PRTDSI__OUT_SEL1       (* (reg8 *) SOUND_IN__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define SOUND_IN_PRTDSI__SYNC_OUT       (* (reg8 *) SOUND_IN__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(SOUND_IN__SIO_CFG)
    #define SOUND_IN_SIO_HYST_EN        (* (reg8 *) SOUND_IN__SIO_HYST_EN)
    #define SOUND_IN_SIO_REG_HIFREQ     (* (reg8 *) SOUND_IN__SIO_REG_HIFREQ)
    #define SOUND_IN_SIO_CFG            (* (reg8 *) SOUND_IN__SIO_CFG)
    #define SOUND_IN_SIO_DIFF           (* (reg8 *) SOUND_IN__SIO_DIFF)
#endif /* (SOUND_IN__SIO_CFG) */

/* Interrupt Registers */
#if defined(SOUND_IN__INTSTAT)
    #define SOUND_IN_INTSTAT            (* (reg8 *) SOUND_IN__INTSTAT)
    #define SOUND_IN_SNAP               (* (reg8 *) SOUND_IN__SNAP)
    
	#define SOUND_IN_0_INTTYPE_REG 		(* (reg8 *) SOUND_IN__0__INTTYPE)
#endif /* (SOUND_IN__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_SOUND_IN_H */


/* [] END OF FILE */
