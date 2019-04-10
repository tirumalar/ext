/*******************************************************************************
* File Name: OLD_SCL.h  
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

#if !defined(CY_PINS_OLD_SCL_H) /* Pins OLD_SCL_H */
#define CY_PINS_OLD_SCL_H

#include "cytypes.h"
#include "cyfitter.h"
#include "OLD_SCL_aliases.h"


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
} OLD_SCL_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   OLD_SCL_Read(void);
void    OLD_SCL_Write(uint8 value);
uint8   OLD_SCL_ReadDataReg(void);
#if defined(OLD_SCL__PC) || (CY_PSOC4_4200L) 
    void    OLD_SCL_SetDriveMode(uint8 mode);
#endif
void    OLD_SCL_SetInterruptMode(uint16 position, uint16 mode);
uint8   OLD_SCL_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void OLD_SCL_Sleep(void); 
void OLD_SCL_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(OLD_SCL__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define OLD_SCL_DRIVE_MODE_BITS        (3)
    #define OLD_SCL_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - OLD_SCL_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the OLD_SCL_SetDriveMode() function.
         *  @{
         */
        #define OLD_SCL_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define OLD_SCL_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define OLD_SCL_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define OLD_SCL_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define OLD_SCL_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define OLD_SCL_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define OLD_SCL_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define OLD_SCL_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define OLD_SCL_MASK               OLD_SCL__MASK
#define OLD_SCL_SHIFT              OLD_SCL__SHIFT
#define OLD_SCL_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in OLD_SCL_SetInterruptMode() function.
     *  @{
     */
        #define OLD_SCL_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define OLD_SCL_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define OLD_SCL_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define OLD_SCL_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(OLD_SCL__SIO)
    #define OLD_SCL_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(OLD_SCL__PC) && (CY_PSOC4_4200L)
    #define OLD_SCL_USBIO_ENABLE               ((uint32)0x80000000u)
    #define OLD_SCL_USBIO_DISABLE              ((uint32)(~OLD_SCL_USBIO_ENABLE))
    #define OLD_SCL_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define OLD_SCL_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define OLD_SCL_USBIO_ENTER_SLEEP          ((uint32)((1u << OLD_SCL_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << OLD_SCL_USBIO_SUSPEND_DEL_SHIFT)))
    #define OLD_SCL_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << OLD_SCL_USBIO_SUSPEND_SHIFT)))
    #define OLD_SCL_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << OLD_SCL_USBIO_SUSPEND_DEL_SHIFT)))
    #define OLD_SCL_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(OLD_SCL__PC)
    /* Port Configuration */
    #define OLD_SCL_PC                 (* (reg32 *) OLD_SCL__PC)
#endif
/* Pin State */
#define OLD_SCL_PS                     (* (reg32 *) OLD_SCL__PS)
/* Data Register */
#define OLD_SCL_DR                     (* (reg32 *) OLD_SCL__DR)
/* Input Buffer Disable Override */
#define OLD_SCL_INP_DIS                (* (reg32 *) OLD_SCL__PC2)

/* Interrupt configuration Registers */
#define OLD_SCL_INTCFG                 (* (reg32 *) OLD_SCL__INTCFG)
#define OLD_SCL_INTSTAT                (* (reg32 *) OLD_SCL__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define OLD_SCL_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(OLD_SCL__SIO)
    #define OLD_SCL_SIO_REG            (* (reg32 *) OLD_SCL__SIO)
#endif /* (OLD_SCL__SIO_CFG) */

/* USBIO registers */
#if !defined(OLD_SCL__PC) && (CY_PSOC4_4200L)
    #define OLD_SCL_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define OLD_SCL_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define OLD_SCL_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define OLD_SCL_DRIVE_MODE_SHIFT       (0x00u)
#define OLD_SCL_DRIVE_MODE_MASK        (0x07u << OLD_SCL_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins OLD_SCL_H */


/* [] END OF FILE */
