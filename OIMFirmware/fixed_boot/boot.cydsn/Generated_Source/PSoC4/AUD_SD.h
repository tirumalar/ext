/*******************************************************************************
* File Name: AUD_SD.h  
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

#if !defined(CY_PINS_AUD_SD_H) /* Pins AUD_SD_H */
#define CY_PINS_AUD_SD_H

#include "cytypes.h"
#include "cyfitter.h"
#include "AUD_SD_aliases.h"


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
} AUD_SD_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   AUD_SD_Read(void);
void    AUD_SD_Write(uint8 value);
uint8   AUD_SD_ReadDataReg(void);
#if defined(AUD_SD__PC) || (CY_PSOC4_4200L) 
    void    AUD_SD_SetDriveMode(uint8 mode);
#endif
void    AUD_SD_SetInterruptMode(uint16 position, uint16 mode);
uint8   AUD_SD_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void AUD_SD_Sleep(void); 
void AUD_SD_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(AUD_SD__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define AUD_SD_DRIVE_MODE_BITS        (3)
    #define AUD_SD_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - AUD_SD_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the AUD_SD_SetDriveMode() function.
         *  @{
         */
        #define AUD_SD_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define AUD_SD_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define AUD_SD_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define AUD_SD_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define AUD_SD_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define AUD_SD_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define AUD_SD_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define AUD_SD_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define AUD_SD_MASK               AUD_SD__MASK
#define AUD_SD_SHIFT              AUD_SD__SHIFT
#define AUD_SD_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in AUD_SD_SetInterruptMode() function.
     *  @{
     */
        #define AUD_SD_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define AUD_SD_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define AUD_SD_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define AUD_SD_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(AUD_SD__SIO)
    #define AUD_SD_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(AUD_SD__PC) && (CY_PSOC4_4200L)
    #define AUD_SD_USBIO_ENABLE               ((uint32)0x80000000u)
    #define AUD_SD_USBIO_DISABLE              ((uint32)(~AUD_SD_USBIO_ENABLE))
    #define AUD_SD_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define AUD_SD_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define AUD_SD_USBIO_ENTER_SLEEP          ((uint32)((1u << AUD_SD_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << AUD_SD_USBIO_SUSPEND_DEL_SHIFT)))
    #define AUD_SD_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << AUD_SD_USBIO_SUSPEND_SHIFT)))
    #define AUD_SD_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << AUD_SD_USBIO_SUSPEND_DEL_SHIFT)))
    #define AUD_SD_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(AUD_SD__PC)
    /* Port Configuration */
    #define AUD_SD_PC                 (* (reg32 *) AUD_SD__PC)
#endif
/* Pin State */
#define AUD_SD_PS                     (* (reg32 *) AUD_SD__PS)
/* Data Register */
#define AUD_SD_DR                     (* (reg32 *) AUD_SD__DR)
/* Input Buffer Disable Override */
#define AUD_SD_INP_DIS                (* (reg32 *) AUD_SD__PC2)

/* Interrupt configuration Registers */
#define AUD_SD_INTCFG                 (* (reg32 *) AUD_SD__INTCFG)
#define AUD_SD_INTSTAT                (* (reg32 *) AUD_SD__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define AUD_SD_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(AUD_SD__SIO)
    #define AUD_SD_SIO_REG            (* (reg32 *) AUD_SD__SIO)
#endif /* (AUD_SD__SIO_CFG) */

/* USBIO registers */
#if !defined(AUD_SD__PC) && (CY_PSOC4_4200L)
    #define AUD_SD_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define AUD_SD_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define AUD_SD_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define AUD_SD_DRIVE_MODE_SHIFT       (0x00u)
#define AUD_SD_DRIVE_MODE_MASK        (0x07u << AUD_SD_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins AUD_SD_H */


/* [] END OF FILE */
