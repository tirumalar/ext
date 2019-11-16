/*******************************************************************************
* File Name: AUD_GAIN1.h  
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

#if !defined(CY_PINS_AUD_GAIN1_H) /* Pins AUD_GAIN1_H */
#define CY_PINS_AUD_GAIN1_H

#include "cytypes.h"
#include "cyfitter.h"
#include "AUD_GAIN1_aliases.h"


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
} AUD_GAIN1_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   AUD_GAIN1_Read(void);
void    AUD_GAIN1_Write(uint8 value);
uint8   AUD_GAIN1_ReadDataReg(void);
#if defined(AUD_GAIN1__PC) || (CY_PSOC4_4200L) 
    void    AUD_GAIN1_SetDriveMode(uint8 mode);
#endif
void    AUD_GAIN1_SetInterruptMode(uint16 position, uint16 mode);
uint8   AUD_GAIN1_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void AUD_GAIN1_Sleep(void); 
void AUD_GAIN1_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(AUD_GAIN1__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define AUD_GAIN1_DRIVE_MODE_BITS        (3)
    #define AUD_GAIN1_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - AUD_GAIN1_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the AUD_GAIN1_SetDriveMode() function.
         *  @{
         */
        #define AUD_GAIN1_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define AUD_GAIN1_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define AUD_GAIN1_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define AUD_GAIN1_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define AUD_GAIN1_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define AUD_GAIN1_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define AUD_GAIN1_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define AUD_GAIN1_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define AUD_GAIN1_MASK               AUD_GAIN1__MASK
#define AUD_GAIN1_SHIFT              AUD_GAIN1__SHIFT
#define AUD_GAIN1_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in AUD_GAIN1_SetInterruptMode() function.
     *  @{
     */
        #define AUD_GAIN1_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define AUD_GAIN1_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define AUD_GAIN1_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define AUD_GAIN1_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(AUD_GAIN1__SIO)
    #define AUD_GAIN1_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(AUD_GAIN1__PC) && (CY_PSOC4_4200L)
    #define AUD_GAIN1_USBIO_ENABLE               ((uint32)0x80000000u)
    #define AUD_GAIN1_USBIO_DISABLE              ((uint32)(~AUD_GAIN1_USBIO_ENABLE))
    #define AUD_GAIN1_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define AUD_GAIN1_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define AUD_GAIN1_USBIO_ENTER_SLEEP          ((uint32)((1u << AUD_GAIN1_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << AUD_GAIN1_USBIO_SUSPEND_DEL_SHIFT)))
    #define AUD_GAIN1_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << AUD_GAIN1_USBIO_SUSPEND_SHIFT)))
    #define AUD_GAIN1_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << AUD_GAIN1_USBIO_SUSPEND_DEL_SHIFT)))
    #define AUD_GAIN1_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(AUD_GAIN1__PC)
    /* Port Configuration */
    #define AUD_GAIN1_PC                 (* (reg32 *) AUD_GAIN1__PC)
#endif
/* Pin State */
#define AUD_GAIN1_PS                     (* (reg32 *) AUD_GAIN1__PS)
/* Data Register */
#define AUD_GAIN1_DR                     (* (reg32 *) AUD_GAIN1__DR)
/* Input Buffer Disable Override */
#define AUD_GAIN1_INP_DIS                (* (reg32 *) AUD_GAIN1__PC2)

/* Interrupt configuration Registers */
#define AUD_GAIN1_INTCFG                 (* (reg32 *) AUD_GAIN1__INTCFG)
#define AUD_GAIN1_INTSTAT                (* (reg32 *) AUD_GAIN1__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define AUD_GAIN1_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(AUD_GAIN1__SIO)
    #define AUD_GAIN1_SIO_REG            (* (reg32 *) AUD_GAIN1__SIO)
#endif /* (AUD_GAIN1__SIO_CFG) */

/* USBIO registers */
#if !defined(AUD_GAIN1__PC) && (CY_PSOC4_4200L)
    #define AUD_GAIN1_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define AUD_GAIN1_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define AUD_GAIN1_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define AUD_GAIN1_DRIVE_MODE_SHIFT       (0x00u)
#define AUD_GAIN1_DRIVE_MODE_MASK        (0x07u << AUD_GAIN1_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins AUD_GAIN1_H */


/* [] END OF FILE */
