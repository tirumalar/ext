/*******************************************************************************
* File Name: RESET_SWBTN.h  
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

#if !defined(CY_PINS_RESET_SWBTN_H) /* Pins RESET_SWBTN_H */
#define CY_PINS_RESET_SWBTN_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "RESET_SWBTN_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 RESET_SWBTN__PORT == 15 && ((RESET_SWBTN__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    RESET_SWBTN_Write(uint8 value);
void    RESET_SWBTN_SetDriveMode(uint8 mode);
uint8   RESET_SWBTN_ReadDataReg(void);
uint8   RESET_SWBTN_Read(void);
void    RESET_SWBTN_SetInterruptMode(uint16 position, uint16 mode);
uint8   RESET_SWBTN_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the RESET_SWBTN_SetDriveMode() function.
     *  @{
     */
        #define RESET_SWBTN_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define RESET_SWBTN_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define RESET_SWBTN_DM_RES_UP          PIN_DM_RES_UP
        #define RESET_SWBTN_DM_RES_DWN         PIN_DM_RES_DWN
        #define RESET_SWBTN_DM_OD_LO           PIN_DM_OD_LO
        #define RESET_SWBTN_DM_OD_HI           PIN_DM_OD_HI
        #define RESET_SWBTN_DM_STRONG          PIN_DM_STRONG
        #define RESET_SWBTN_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define RESET_SWBTN_MASK               RESET_SWBTN__MASK
#define RESET_SWBTN_SHIFT              RESET_SWBTN__SHIFT
#define RESET_SWBTN_WIDTH              1u

/* Interrupt constants */
#if defined(RESET_SWBTN__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in RESET_SWBTN_SetInterruptMode() function.
     *  @{
     */
        #define RESET_SWBTN_INTR_NONE      (uint16)(0x0000u)
        #define RESET_SWBTN_INTR_RISING    (uint16)(0x0001u)
        #define RESET_SWBTN_INTR_FALLING   (uint16)(0x0002u)
        #define RESET_SWBTN_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define RESET_SWBTN_INTR_MASK      (0x01u) 
#endif /* (RESET_SWBTN__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define RESET_SWBTN_PS                     (* (reg8 *) RESET_SWBTN__PS)
/* Data Register */
#define RESET_SWBTN_DR                     (* (reg8 *) RESET_SWBTN__DR)
/* Port Number */
#define RESET_SWBTN_PRT_NUM                (* (reg8 *) RESET_SWBTN__PRT) 
/* Connect to Analog Globals */                                                  
#define RESET_SWBTN_AG                     (* (reg8 *) RESET_SWBTN__AG)                       
/* Analog MUX bux enable */
#define RESET_SWBTN_AMUX                   (* (reg8 *) RESET_SWBTN__AMUX) 
/* Bidirectional Enable */                                                        
#define RESET_SWBTN_BIE                    (* (reg8 *) RESET_SWBTN__BIE)
/* Bit-mask for Aliased Register Access */
#define RESET_SWBTN_BIT_MASK               (* (reg8 *) RESET_SWBTN__BIT_MASK)
/* Bypass Enable */
#define RESET_SWBTN_BYP                    (* (reg8 *) RESET_SWBTN__BYP)
/* Port wide control signals */                                                   
#define RESET_SWBTN_CTL                    (* (reg8 *) RESET_SWBTN__CTL)
/* Drive Modes */
#define RESET_SWBTN_DM0                    (* (reg8 *) RESET_SWBTN__DM0) 
#define RESET_SWBTN_DM1                    (* (reg8 *) RESET_SWBTN__DM1)
#define RESET_SWBTN_DM2                    (* (reg8 *) RESET_SWBTN__DM2) 
/* Input Buffer Disable Override */
#define RESET_SWBTN_INP_DIS                (* (reg8 *) RESET_SWBTN__INP_DIS)
/* LCD Common or Segment Drive */
#define RESET_SWBTN_LCD_COM_SEG            (* (reg8 *) RESET_SWBTN__LCD_COM_SEG)
/* Enable Segment LCD */
#define RESET_SWBTN_LCD_EN                 (* (reg8 *) RESET_SWBTN__LCD_EN)
/* Slew Rate Control */
#define RESET_SWBTN_SLW                    (* (reg8 *) RESET_SWBTN__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define RESET_SWBTN_PRTDSI__CAPS_SEL       (* (reg8 *) RESET_SWBTN__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define RESET_SWBTN_PRTDSI__DBL_SYNC_IN    (* (reg8 *) RESET_SWBTN__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define RESET_SWBTN_PRTDSI__OE_SEL0        (* (reg8 *) RESET_SWBTN__PRTDSI__OE_SEL0) 
#define RESET_SWBTN_PRTDSI__OE_SEL1        (* (reg8 *) RESET_SWBTN__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define RESET_SWBTN_PRTDSI__OUT_SEL0       (* (reg8 *) RESET_SWBTN__PRTDSI__OUT_SEL0) 
#define RESET_SWBTN_PRTDSI__OUT_SEL1       (* (reg8 *) RESET_SWBTN__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define RESET_SWBTN_PRTDSI__SYNC_OUT       (* (reg8 *) RESET_SWBTN__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(RESET_SWBTN__SIO_CFG)
    #define RESET_SWBTN_SIO_HYST_EN        (* (reg8 *) RESET_SWBTN__SIO_HYST_EN)
    #define RESET_SWBTN_SIO_REG_HIFREQ     (* (reg8 *) RESET_SWBTN__SIO_REG_HIFREQ)
    #define RESET_SWBTN_SIO_CFG            (* (reg8 *) RESET_SWBTN__SIO_CFG)
    #define RESET_SWBTN_SIO_DIFF           (* (reg8 *) RESET_SWBTN__SIO_DIFF)
#endif /* (RESET_SWBTN__SIO_CFG) */

/* Interrupt Registers */
#if defined(RESET_SWBTN__INTSTAT)
    #define RESET_SWBTN_INTSTAT            (* (reg8 *) RESET_SWBTN__INTSTAT)
    #define RESET_SWBTN_SNAP               (* (reg8 *) RESET_SWBTN__SNAP)
    
	#define RESET_SWBTN_0_INTTYPE_REG 		(* (reg8 *) RESET_SWBTN__0__INTTYPE)
#endif /* (RESET_SWBTN__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_RESET_SWBTN_H */


/* [] END OF FILE */
