/*******************************************************************************
* File Name: EXT_RES.h  
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

#if !defined(CY_PINS_EXT_RES_H) /* Pins EXT_RES_H */
#define CY_PINS_EXT_RES_H

#include "cytypes.h"
#include "cyfitter.h"
#include "EXT_RES_aliases.h"


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
} EXT_RES_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   EXT_RES_Read(void);
void    EXT_RES_Write(uint8 value);
uint8   EXT_RES_ReadDataReg(void);
#if defined(EXT_RES__PC) || (CY_PSOC4_4200L) 
    void    EXT_RES_SetDriveMode(uint8 mode);
#endif
void    EXT_RES_SetInterruptMode(uint16 position, uint16 mode);
uint8   EXT_RES_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void EXT_RES_Sleep(void); 
void EXT_RES_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(EXT_RES__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define EXT_RES_DRIVE_MODE_BITS        (3)
    #define EXT_RES_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - EXT_RES_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the EXT_RES_SetDriveMode() function.
         *  @{
         */
        #define EXT_RES_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define EXT_RES_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define EXT_RES_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define EXT_RES_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define EXT_RES_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define EXT_RES_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define EXT_RES_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define EXT_RES_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define EXT_RES_MASK               EXT_RES__MASK
#define EXT_RES_SHIFT              EXT_RES__SHIFT
#define EXT_RES_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in EXT_RES_SetInterruptMode() function.
     *  @{
     */
        #define EXT_RES_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define EXT_RES_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define EXT_RES_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define EXT_RES_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(EXT_RES__SIO)
    #define EXT_RES_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(EXT_RES__PC) && (CY_PSOC4_4200L)
    #define EXT_RES_USBIO_ENABLE               ((uint32)0x80000000u)
    #define EXT_RES_USBIO_DISABLE              ((uint32)(~EXT_RES_USBIO_ENABLE))
    #define EXT_RES_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define EXT_RES_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define EXT_RES_USBIO_ENTER_SLEEP          ((uint32)((1u << EXT_RES_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << EXT_RES_USBIO_SUSPEND_DEL_SHIFT)))
    #define EXT_RES_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << EXT_RES_USBIO_SUSPEND_SHIFT)))
    #define EXT_RES_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << EXT_RES_USBIO_SUSPEND_DEL_SHIFT)))
    #define EXT_RES_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(EXT_RES__PC)
    /* Port Configuration */
    #define EXT_RES_PC                 (* (reg32 *) EXT_RES__PC)
#endif
/* Pin State */
#define EXT_RES_PS                     (* (reg32 *) EXT_RES__PS)
/* Data Register */
#define EXT_RES_DR                     (* (reg32 *) EXT_RES__DR)
/* Input Buffer Disable Override */
#define EXT_RES_INP_DIS                (* (reg32 *) EXT_RES__PC2)

/* Interrupt configuration Registers */
#define EXT_RES_INTCFG                 (* (reg32 *) EXT_RES__INTCFG)
#define EXT_RES_INTSTAT                (* (reg32 *) EXT_RES__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define EXT_RES_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(EXT_RES__SIO)
    #define EXT_RES_SIO_REG            (* (reg32 *) EXT_RES__SIO)
#endif /* (EXT_RES__SIO_CFG) */

/* USBIO registers */
#if !defined(EXT_RES__PC) && (CY_PSOC4_4200L)
    #define EXT_RES_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define EXT_RES_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define EXT_RES_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define EXT_RES_DRIVE_MODE_SHIFT       (0x00u)
#define EXT_RES_DRIVE_MODE_MASK        (0x07u << EXT_RES_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins EXT_RES_H */


/* [] END OF FILE */
