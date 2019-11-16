/*******************************************************************************
* File Name: MAX_I.h  
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

#if !defined(CY_PINS_MAX_I_H) /* Pins MAX_I_H */
#define CY_PINS_MAX_I_H

#include "cytypes.h"
#include "cyfitter.h"
#include "MAX_I_aliases.h"


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
} MAX_I_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   MAX_I_Read(void);
void    MAX_I_Write(uint8 value);
uint8   MAX_I_ReadDataReg(void);
#if defined(MAX_I__PC) || (CY_PSOC4_4200L) 
    void    MAX_I_SetDriveMode(uint8 mode);
#endif
void    MAX_I_SetInterruptMode(uint16 position, uint16 mode);
uint8   MAX_I_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void MAX_I_Sleep(void); 
void MAX_I_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(MAX_I__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define MAX_I_DRIVE_MODE_BITS        (3)
    #define MAX_I_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - MAX_I_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the MAX_I_SetDriveMode() function.
         *  @{
         */
        #define MAX_I_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define MAX_I_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define MAX_I_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define MAX_I_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define MAX_I_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define MAX_I_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define MAX_I_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define MAX_I_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define MAX_I_MASK               MAX_I__MASK
#define MAX_I_SHIFT              MAX_I__SHIFT
#define MAX_I_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in MAX_I_SetInterruptMode() function.
     *  @{
     */
        #define MAX_I_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define MAX_I_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define MAX_I_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define MAX_I_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(MAX_I__SIO)
    #define MAX_I_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(MAX_I__PC) && (CY_PSOC4_4200L)
    #define MAX_I_USBIO_ENABLE               ((uint32)0x80000000u)
    #define MAX_I_USBIO_DISABLE              ((uint32)(~MAX_I_USBIO_ENABLE))
    #define MAX_I_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define MAX_I_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define MAX_I_USBIO_ENTER_SLEEP          ((uint32)((1u << MAX_I_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << MAX_I_USBIO_SUSPEND_DEL_SHIFT)))
    #define MAX_I_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << MAX_I_USBIO_SUSPEND_SHIFT)))
    #define MAX_I_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << MAX_I_USBIO_SUSPEND_DEL_SHIFT)))
    #define MAX_I_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(MAX_I__PC)
    /* Port Configuration */
    #define MAX_I_PC                 (* (reg32 *) MAX_I__PC)
#endif
/* Pin State */
#define MAX_I_PS                     (* (reg32 *) MAX_I__PS)
/* Data Register */
#define MAX_I_DR                     (* (reg32 *) MAX_I__DR)
/* Input Buffer Disable Override */
#define MAX_I_INP_DIS                (* (reg32 *) MAX_I__PC2)

/* Interrupt configuration Registers */
#define MAX_I_INTCFG                 (* (reg32 *) MAX_I__INTCFG)
#define MAX_I_INTSTAT                (* (reg32 *) MAX_I__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define MAX_I_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(MAX_I__SIO)
    #define MAX_I_SIO_REG            (* (reg32 *) MAX_I__SIO)
#endif /* (MAX_I__SIO_CFG) */

/* USBIO registers */
#if !defined(MAX_I__PC) && (CY_PSOC4_4200L)
    #define MAX_I_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define MAX_I_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define MAX_I_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define MAX_I_DRIVE_MODE_SHIFT       (0x00u)
#define MAX_I_DRIVE_MODE_MASK        (0x07u << MAX_I_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins MAX_I_H */


/* [] END OF FILE */
