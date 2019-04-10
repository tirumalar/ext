/*******************************************************************************
* File Name: POE_OIM_SDn.h  
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

#if !defined(CY_PINS_POE_OIM_SDn_H) /* Pins POE_OIM_SDn_H */
#define CY_PINS_POE_OIM_SDn_H

#include "cytypes.h"
#include "cyfitter.h"
#include "cypins.h"
#include "POE_OIM_SDn_aliases.h"

/* APIs are not generated for P15[7:6] */
#if !(CY_PSOC5A &&\
	 POE_OIM_SDn__PORT == 15 && ((POE_OIM_SDn__MASK & 0xC0) != 0))


/***************************************
*        Function Prototypes             
***************************************/    

/**
* \addtogroup group_general
* @{
*/
void    POE_OIM_SDn_Write(uint8 value);
void    POE_OIM_SDn_SetDriveMode(uint8 mode);
uint8   POE_OIM_SDn_ReadDataReg(void);
uint8   POE_OIM_SDn_Read(void);
void    POE_OIM_SDn_SetInterruptMode(uint16 position, uint16 mode);
uint8   POE_OIM_SDn_ClearInterrupt(void);
/** @} general */

/***************************************
*           API Constants        
***************************************/
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup driveMode Drive mode constants
     * \brief Constants to be passed as "mode" parameter in the POE_OIM_SDn_SetDriveMode() function.
     *  @{
     */
        #define POE_OIM_SDn_DM_ALG_HIZ         PIN_DM_ALG_HIZ
        #define POE_OIM_SDn_DM_DIG_HIZ         PIN_DM_DIG_HIZ
        #define POE_OIM_SDn_DM_RES_UP          PIN_DM_RES_UP
        #define POE_OIM_SDn_DM_RES_DWN         PIN_DM_RES_DWN
        #define POE_OIM_SDn_DM_OD_LO           PIN_DM_OD_LO
        #define POE_OIM_SDn_DM_OD_HI           PIN_DM_OD_HI
        #define POE_OIM_SDn_DM_STRONG          PIN_DM_STRONG
        #define POE_OIM_SDn_DM_RES_UPDWN       PIN_DM_RES_UPDWN
    /** @} driveMode */
/** @} group_constants */
    
/* Digital Port Constants */
#define POE_OIM_SDn_MASK               POE_OIM_SDn__MASK
#define POE_OIM_SDn_SHIFT              POE_OIM_SDn__SHIFT
#define POE_OIM_SDn_WIDTH              1u

/* Interrupt constants */
#if defined(POE_OIM_SDn__INTSTAT)
/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in POE_OIM_SDn_SetInterruptMode() function.
     *  @{
     */
        #define POE_OIM_SDn_INTR_NONE      (uint16)(0x0000u)
        #define POE_OIM_SDn_INTR_RISING    (uint16)(0x0001u)
        #define POE_OIM_SDn_INTR_FALLING   (uint16)(0x0002u)
        #define POE_OIM_SDn_INTR_BOTH      (uint16)(0x0003u) 
    /** @} intrMode */
/** @} group_constants */

    #define POE_OIM_SDn_INTR_MASK      (0x01u) 
#endif /* (POE_OIM_SDn__INTSTAT) */


/***************************************
*             Registers        
***************************************/

/* Main Port Registers */
/* Pin State */
#define POE_OIM_SDn_PS                     (* (reg8 *) POE_OIM_SDn__PS)
/* Data Register */
#define POE_OIM_SDn_DR                     (* (reg8 *) POE_OIM_SDn__DR)
/* Port Number */
#define POE_OIM_SDn_PRT_NUM                (* (reg8 *) POE_OIM_SDn__PRT) 
/* Connect to Analog Globals */                                                  
#define POE_OIM_SDn_AG                     (* (reg8 *) POE_OIM_SDn__AG)                       
/* Analog MUX bux enable */
#define POE_OIM_SDn_AMUX                   (* (reg8 *) POE_OIM_SDn__AMUX) 
/* Bidirectional Enable */                                                        
#define POE_OIM_SDn_BIE                    (* (reg8 *) POE_OIM_SDn__BIE)
/* Bit-mask for Aliased Register Access */
#define POE_OIM_SDn_BIT_MASK               (* (reg8 *) POE_OIM_SDn__BIT_MASK)
/* Bypass Enable */
#define POE_OIM_SDn_BYP                    (* (reg8 *) POE_OIM_SDn__BYP)
/* Port wide control signals */                                                   
#define POE_OIM_SDn_CTL                    (* (reg8 *) POE_OIM_SDn__CTL)
/* Drive Modes */
#define POE_OIM_SDn_DM0                    (* (reg8 *) POE_OIM_SDn__DM0) 
#define POE_OIM_SDn_DM1                    (* (reg8 *) POE_OIM_SDn__DM1)
#define POE_OIM_SDn_DM2                    (* (reg8 *) POE_OIM_SDn__DM2) 
/* Input Buffer Disable Override */
#define POE_OIM_SDn_INP_DIS                (* (reg8 *) POE_OIM_SDn__INP_DIS)
/* LCD Common or Segment Drive */
#define POE_OIM_SDn_LCD_COM_SEG            (* (reg8 *) POE_OIM_SDn__LCD_COM_SEG)
/* Enable Segment LCD */
#define POE_OIM_SDn_LCD_EN                 (* (reg8 *) POE_OIM_SDn__LCD_EN)
/* Slew Rate Control */
#define POE_OIM_SDn_SLW                    (* (reg8 *) POE_OIM_SDn__SLW)

/* DSI Port Registers */
/* Global DSI Select Register */
#define POE_OIM_SDn_PRTDSI__CAPS_SEL       (* (reg8 *) POE_OIM_SDn__PRTDSI__CAPS_SEL) 
/* Double Sync Enable */
#define POE_OIM_SDn_PRTDSI__DBL_SYNC_IN    (* (reg8 *) POE_OIM_SDn__PRTDSI__DBL_SYNC_IN) 
/* Output Enable Select Drive Strength */
#define POE_OIM_SDn_PRTDSI__OE_SEL0        (* (reg8 *) POE_OIM_SDn__PRTDSI__OE_SEL0) 
#define POE_OIM_SDn_PRTDSI__OE_SEL1        (* (reg8 *) POE_OIM_SDn__PRTDSI__OE_SEL1) 
/* Port Pin Output Select Registers */
#define POE_OIM_SDn_PRTDSI__OUT_SEL0       (* (reg8 *) POE_OIM_SDn__PRTDSI__OUT_SEL0) 
#define POE_OIM_SDn_PRTDSI__OUT_SEL1       (* (reg8 *) POE_OIM_SDn__PRTDSI__OUT_SEL1) 
/* Sync Output Enable Registers */
#define POE_OIM_SDn_PRTDSI__SYNC_OUT       (* (reg8 *) POE_OIM_SDn__PRTDSI__SYNC_OUT) 

/* SIO registers */
#if defined(POE_OIM_SDn__SIO_CFG)
    #define POE_OIM_SDn_SIO_HYST_EN        (* (reg8 *) POE_OIM_SDn__SIO_HYST_EN)
    #define POE_OIM_SDn_SIO_REG_HIFREQ     (* (reg8 *) POE_OIM_SDn__SIO_REG_HIFREQ)
    #define POE_OIM_SDn_SIO_CFG            (* (reg8 *) POE_OIM_SDn__SIO_CFG)
    #define POE_OIM_SDn_SIO_DIFF           (* (reg8 *) POE_OIM_SDn__SIO_DIFF)
#endif /* (POE_OIM_SDn__SIO_CFG) */

/* Interrupt Registers */
#if defined(POE_OIM_SDn__INTSTAT)
    #define POE_OIM_SDn_INTSTAT            (* (reg8 *) POE_OIM_SDn__INTSTAT)
    #define POE_OIM_SDn_SNAP               (* (reg8 *) POE_OIM_SDn__SNAP)
    
	#define POE_OIM_SDn_0_INTTYPE_REG 		(* (reg8 *) POE_OIM_SDn__0__INTTYPE)
#endif /* (POE_OIM_SDn__INTSTAT) */

#endif /* CY_PSOC5A... */

#endif /*  CY_PINS_POE_OIM_SDn_H */


/* [] END OF FILE */
