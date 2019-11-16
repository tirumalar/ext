/*******************************************************************************
* File Name: VREFB.h  
* Version 2.20
*
* Description:
*  This file contains Pin function prototypes and register defines
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_PINS_VREFB_H) /* Pins VREFB_H */
#define CY_PINS_VREFB_H

#include "cytypes.h"
#include "cyfitter.h"
#include "VREFB_aliases.h"


/***************************************
*     Data Struct Definitions
***************************************/

/**
* \addtogroup group_structures
* @{
*/
    
/* Structure for sleep mode support */
typedef struct
{
    uint32 pcState; /**< State of the port control register */
    uint32 sioState; /**< State of the SIO configuration */
    uint32 usbState; /**< State of the USBIO regulator */
} VREFB_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   VREFB_Read(void);
void    VREFB_Write(uint8 value);
uint8   VREFB_ReadDataReg(void);
#if defined(VREFB__PC) || (CY_PSOC4_4200L) 
    void    VREFB_SetDriveMode(uint8 mode);
#endif
void    VREFB_SetInterruptMode(uint16 position, uint16 mode);
uint8   VREFB_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void VREFB_Sleep(void); 
void VREFB_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(VREFB__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define VREFB_DRIVE_MODE_BITS        (3)
    #define VREFB_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - VREFB_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the VREFB_SetDriveMode() function.
         *  @{
         */
        #define VREFB_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define VREFB_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define VREFB_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define VREFB_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define VREFB_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define VREFB_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define VREFB_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define VREFB_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define VREFB_MASK               VREFB__MASK
#define VREFB_SHIFT              VREFB__SHIFT
#define VREFB_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in VREFB_SetInterruptMode() function.
     *  @{
     */
        #define VREFB_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define VREFB_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define VREFB_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define VREFB_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(VREFB__SIO)
    #define VREFB_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(VREFB__PC) && (CY_PSOC4_4200L)
    #define VREFB_USBIO_ENABLE               ((uint32)0x80000000u)
    #define VREFB_USBIO_DISABLE              ((uint32)(~VREFB_USBIO_ENABLE))
    #define VREFB_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define VREFB_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define VREFB_USBIO_ENTER_SLEEP          ((uint32)((1u << VREFB_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << VREFB_USBIO_SUSPEND_DEL_SHIFT)))
    #define VREFB_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << VREFB_USBIO_SUSPEND_SHIFT)))
    #define VREFB_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << VREFB_USBIO_SUSPEND_DEL_SHIFT)))
    #define VREFB_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(VREFB__PC)
    /* Port Configuration */
    #define VREFB_PC                 (* (reg32 *) VREFB__PC)
#endif
/* Pin State */
#define VREFB_PS                     (* (reg32 *) VREFB__PS)
/* Data Register */
#define VREFB_DR                     (* (reg32 *) VREFB__DR)
/* Input Buffer Disable Override */
#define VREFB_INP_DIS                (* (reg32 *) VREFB__PC2)

/* Interrupt configuration Registers */
#define VREFB_INTCFG                 (* (reg32 *) VREFB__INTCFG)
#define VREFB_INTSTAT                (* (reg32 *) VREFB__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define VREFB_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(VREFB__SIO)
    #define VREFB_SIO_REG            (* (reg32 *) VREFB__SIO)
#endif /* (VREFB__SIO_CFG) */

/* USBIO registers */
#if !defined(VREFB__PC) && (CY_PSOC4_4200L)
    #define VREFB_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define VREFB_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define VREFB_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define VREFB_DRIVE_MODE_SHIFT       (0x00u)
#define VREFB_DRIVE_MODE_MASK        (0x07u << VREFB_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins VREFB_H */


/* [] END OF FILE */
