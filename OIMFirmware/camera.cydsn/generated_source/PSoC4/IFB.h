/*******************************************************************************
* File Name: IFB.h  
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

#if !defined(CY_PINS_IFB_H) /* Pins IFB_H */
#define CY_PINS_IFB_H

#include "cytypes.h"
#include "cyfitter.h"
#include "IFB_aliases.h"


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
} IFB_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   IFB_Read(void);
void    IFB_Write(uint8 value);
uint8   IFB_ReadDataReg(void);
#if defined(IFB__PC) || (CY_PSOC4_4200L) 
    void    IFB_SetDriveMode(uint8 mode);
#endif
void    IFB_SetInterruptMode(uint16 position, uint16 mode);
uint8   IFB_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void IFB_Sleep(void); 
void IFB_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(IFB__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define IFB_DRIVE_MODE_BITS        (3)
    #define IFB_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - IFB_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the IFB_SetDriveMode() function.
         *  @{
         */
        #define IFB_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define IFB_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define IFB_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define IFB_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define IFB_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define IFB_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define IFB_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define IFB_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define IFB_MASK               IFB__MASK
#define IFB_SHIFT              IFB__SHIFT
#define IFB_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in IFB_SetInterruptMode() function.
     *  @{
     */
        #define IFB_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define IFB_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define IFB_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define IFB_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(IFB__SIO)
    #define IFB_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(IFB__PC) && (CY_PSOC4_4200L)
    #define IFB_USBIO_ENABLE               ((uint32)0x80000000u)
    #define IFB_USBIO_DISABLE              ((uint32)(~IFB_USBIO_ENABLE))
    #define IFB_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define IFB_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define IFB_USBIO_ENTER_SLEEP          ((uint32)((1u << IFB_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << IFB_USBIO_SUSPEND_DEL_SHIFT)))
    #define IFB_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << IFB_USBIO_SUSPEND_SHIFT)))
    #define IFB_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << IFB_USBIO_SUSPEND_DEL_SHIFT)))
    #define IFB_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(IFB__PC)
    /* Port Configuration */
    #define IFB_PC                 (* (reg32 *) IFB__PC)
#endif
/* Pin State */
#define IFB_PS                     (* (reg32 *) IFB__PS)
/* Data Register */
#define IFB_DR                     (* (reg32 *) IFB__DR)
/* Input Buffer Disable Override */
#define IFB_INP_DIS                (* (reg32 *) IFB__PC2)

/* Interrupt configuration Registers */
#define IFB_INTCFG                 (* (reg32 *) IFB__INTCFG)
#define IFB_INTSTAT                (* (reg32 *) IFB__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define IFB_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(IFB__SIO)
    #define IFB_SIO_REG            (* (reg32 *) IFB__SIO)
#endif /* (IFB__SIO_CFG) */

/* USBIO registers */
#if !defined(IFB__PC) && (CY_PSOC4_4200L)
    #define IFB_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define IFB_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define IFB_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define IFB_DRIVE_MODE_SHIFT       (0x00u)
#define IFB_DRIVE_MODE_MASK        (0x07u << IFB_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins IFB_H */


/* [] END OF FILE */
