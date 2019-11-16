/*******************************************************************************
* File Name: CAM_SPARE.h  
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

#if !defined(CY_PINS_CAM_SPARE_H) /* Pins CAM_SPARE_H */
#define CY_PINS_CAM_SPARE_H

#include "cytypes.h"
#include "cyfitter.h"
#include "CAM_SPARE_aliases.h"


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
} CAM_SPARE_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   CAM_SPARE_Read(void);
void    CAM_SPARE_Write(uint8 value);
uint8   CAM_SPARE_ReadDataReg(void);
#if defined(CAM_SPARE__PC) || (CY_PSOC4_4200L) 
    void    CAM_SPARE_SetDriveMode(uint8 mode);
#endif
void    CAM_SPARE_SetInterruptMode(uint16 position, uint16 mode);
uint8   CAM_SPARE_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void CAM_SPARE_Sleep(void); 
void CAM_SPARE_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(CAM_SPARE__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define CAM_SPARE_DRIVE_MODE_BITS        (3)
    #define CAM_SPARE_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - CAM_SPARE_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the CAM_SPARE_SetDriveMode() function.
         *  @{
         */
        #define CAM_SPARE_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define CAM_SPARE_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define CAM_SPARE_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define CAM_SPARE_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define CAM_SPARE_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define CAM_SPARE_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define CAM_SPARE_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define CAM_SPARE_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define CAM_SPARE_MASK               CAM_SPARE__MASK
#define CAM_SPARE_SHIFT              CAM_SPARE__SHIFT
#define CAM_SPARE_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in CAM_SPARE_SetInterruptMode() function.
     *  @{
     */
        #define CAM_SPARE_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define CAM_SPARE_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define CAM_SPARE_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define CAM_SPARE_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(CAM_SPARE__SIO)
    #define CAM_SPARE_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(CAM_SPARE__PC) && (CY_PSOC4_4200L)
    #define CAM_SPARE_USBIO_ENABLE               ((uint32)0x80000000u)
    #define CAM_SPARE_USBIO_DISABLE              ((uint32)(~CAM_SPARE_USBIO_ENABLE))
    #define CAM_SPARE_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define CAM_SPARE_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define CAM_SPARE_USBIO_ENTER_SLEEP          ((uint32)((1u << CAM_SPARE_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << CAM_SPARE_USBIO_SUSPEND_DEL_SHIFT)))
    #define CAM_SPARE_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << CAM_SPARE_USBIO_SUSPEND_SHIFT)))
    #define CAM_SPARE_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << CAM_SPARE_USBIO_SUSPEND_DEL_SHIFT)))
    #define CAM_SPARE_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(CAM_SPARE__PC)
    /* Port Configuration */
    #define CAM_SPARE_PC                 (* (reg32 *) CAM_SPARE__PC)
#endif
/* Pin State */
#define CAM_SPARE_PS                     (* (reg32 *) CAM_SPARE__PS)
/* Data Register */
#define CAM_SPARE_DR                     (* (reg32 *) CAM_SPARE__DR)
/* Input Buffer Disable Override */
#define CAM_SPARE_INP_DIS                (* (reg32 *) CAM_SPARE__PC2)

/* Interrupt configuration Registers */
#define CAM_SPARE_INTCFG                 (* (reg32 *) CAM_SPARE__INTCFG)
#define CAM_SPARE_INTSTAT                (* (reg32 *) CAM_SPARE__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define CAM_SPARE_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(CAM_SPARE__SIO)
    #define CAM_SPARE_SIO_REG            (* (reg32 *) CAM_SPARE__SIO)
#endif /* (CAM_SPARE__SIO_CFG) */

/* USBIO registers */
#if !defined(CAM_SPARE__PC) && (CY_PSOC4_4200L)
    #define CAM_SPARE_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define CAM_SPARE_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define CAM_SPARE_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define CAM_SPARE_DRIVE_MODE_SHIFT       (0x00u)
#define CAM_SPARE_DRIVE_MODE_MASK        (0x07u << CAM_SPARE_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins CAM_SPARE_H */


/* [] END OF FILE */
