/*******************************************************************************
* File Name: FB.h  
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

#if !defined(CY_PINS_FB_H) /* Pins FB_H */
#define CY_PINS_FB_H

#include "cytypes.h"
#include "cyfitter.h"
#include "FB_aliases.h"


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
} FB_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   FB_Read(void);
void    FB_Write(uint8 value);
uint8   FB_ReadDataReg(void);
#if defined(FB__PC) || (CY_PSOC4_4200L) 
    void    FB_SetDriveMode(uint8 mode);
#endif
void    FB_SetInterruptMode(uint16 position, uint16 mode);
uint8   FB_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void FB_Sleep(void); 
void FB_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(FB__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define FB_DRIVE_MODE_BITS        (3)
    #define FB_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - FB_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the FB_SetDriveMode() function.
         *  @{
         */
        #define FB_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define FB_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define FB_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define FB_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define FB_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define FB_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define FB_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define FB_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define FB_MASK               FB__MASK
#define FB_SHIFT              FB__SHIFT
#define FB_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in FB_SetInterruptMode() function.
     *  @{
     */
        #define FB_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define FB_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define FB_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define FB_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(FB__SIO)
    #define FB_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(FB__PC) && (CY_PSOC4_4200L)
    #define FB_USBIO_ENABLE               ((uint32)0x80000000u)
    #define FB_USBIO_DISABLE              ((uint32)(~FB_USBIO_ENABLE))
    #define FB_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define FB_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define FB_USBIO_ENTER_SLEEP          ((uint32)((1u << FB_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << FB_USBIO_SUSPEND_DEL_SHIFT)))
    #define FB_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << FB_USBIO_SUSPEND_SHIFT)))
    #define FB_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << FB_USBIO_SUSPEND_DEL_SHIFT)))
    #define FB_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(FB__PC)
    /* Port Configuration */
    #define FB_PC                 (* (reg32 *) FB__PC)
#endif
/* Pin State */
#define FB_PS                     (* (reg32 *) FB__PS)
/* Data Register */
#define FB_DR                     (* (reg32 *) FB__DR)
/* Input Buffer Disable Override */
#define FB_INP_DIS                (* (reg32 *) FB__PC2)

/* Interrupt configuration Registers */
#define FB_INTCFG                 (* (reg32 *) FB__INTCFG)
#define FB_INTSTAT                (* (reg32 *) FB__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define FB_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(FB__SIO)
    #define FB_SIO_REG            (* (reg32 *) FB__SIO)
#endif /* (FB__SIO_CFG) */

/* USBIO registers */
#if !defined(FB__PC) && (CY_PSOC4_4200L)
    #define FB_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define FB_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define FB_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define FB_DRIVE_MODE_SHIFT       (0x00u)
#define FB_DRIVE_MODE_MASK        (0x07u << FB_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins FB_H */


/* [] END OF FILE */
