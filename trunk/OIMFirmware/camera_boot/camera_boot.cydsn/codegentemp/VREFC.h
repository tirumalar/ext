/*******************************************************************************
* File Name: VREFC.h  
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

#if !defined(CY_PINS_VREFC_H) /* Pins VREFC_H */
#define CY_PINS_VREFC_H

#include "cytypes.h"
#include "cyfitter.h"
#include "VREFC_aliases.h"


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
} VREFC_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   VREFC_Read(void);
void    VREFC_Write(uint8 value);
uint8   VREFC_ReadDataReg(void);
#if defined(VREFC__PC) || (CY_PSOC4_4200L) 
    void    VREFC_SetDriveMode(uint8 mode);
#endif
void    VREFC_SetInterruptMode(uint16 position, uint16 mode);
uint8   VREFC_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void VREFC_Sleep(void); 
void VREFC_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(VREFC__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define VREFC_DRIVE_MODE_BITS        (3)
    #define VREFC_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - VREFC_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the VREFC_SetDriveMode() function.
         *  @{
         */
        #define VREFC_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define VREFC_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define VREFC_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define VREFC_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define VREFC_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define VREFC_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define VREFC_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define VREFC_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define VREFC_MASK               VREFC__MASK
#define VREFC_SHIFT              VREFC__SHIFT
#define VREFC_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in VREFC_SetInterruptMode() function.
     *  @{
     */
        #define VREFC_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define VREFC_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define VREFC_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define VREFC_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(VREFC__SIO)
    #define VREFC_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(VREFC__PC) && (CY_PSOC4_4200L)
    #define VREFC_USBIO_ENABLE               ((uint32)0x80000000u)
    #define VREFC_USBIO_DISABLE              ((uint32)(~VREFC_USBIO_ENABLE))
    #define VREFC_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define VREFC_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define VREFC_USBIO_ENTER_SLEEP          ((uint32)((1u << VREFC_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << VREFC_USBIO_SUSPEND_DEL_SHIFT)))
    #define VREFC_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << VREFC_USBIO_SUSPEND_SHIFT)))
    #define VREFC_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << VREFC_USBIO_SUSPEND_DEL_SHIFT)))
    #define VREFC_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(VREFC__PC)
    /* Port Configuration */
    #define VREFC_PC                 (* (reg32 *) VREFC__PC)
#endif
/* Pin State */
#define VREFC_PS                     (* (reg32 *) VREFC__PS)
/* Data Register */
#define VREFC_DR                     (* (reg32 *) VREFC__DR)
/* Input Buffer Disable Override */
#define VREFC_INP_DIS                (* (reg32 *) VREFC__PC2)

/* Interrupt configuration Registers */
#define VREFC_INTCFG                 (* (reg32 *) VREFC__INTCFG)
#define VREFC_INTSTAT                (* (reg32 *) VREFC__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define VREFC_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(VREFC__SIO)
    #define VREFC_SIO_REG            (* (reg32 *) VREFC__SIO)
#endif /* (VREFC__SIO_CFG) */

/* USBIO registers */
#if !defined(VREFC__PC) && (CY_PSOC4_4200L)
    #define VREFC_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define VREFC_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define VREFC_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define VREFC_DRIVE_MODE_SHIFT       (0x00u)
#define VREFC_DRIVE_MODE_MASK        (0x07u << VREFC_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins VREFC_H */


/* [] END OF FILE */
