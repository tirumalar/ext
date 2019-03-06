/*******************************************************************************
* File Name: TRIG_RIGHT.h  
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

#if !defined(CY_PINS_TRIG_RIGHT_H) /* Pins TRIG_RIGHT_H */
#define CY_PINS_TRIG_RIGHT_H

#include "cytypes.h"
#include "cyfitter.h"
#include "TRIG_RIGHT_aliases.h"


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
} TRIG_RIGHT_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   TRIG_RIGHT_Read(void);
void    TRIG_RIGHT_Write(uint8 value);
uint8   TRIG_RIGHT_ReadDataReg(void);
#if defined(TRIG_RIGHT__PC) || (CY_PSOC4_4200L) 
    void    TRIG_RIGHT_SetDriveMode(uint8 mode);
#endif
void    TRIG_RIGHT_SetInterruptMode(uint16 position, uint16 mode);
uint8   TRIG_RIGHT_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void TRIG_RIGHT_Sleep(void); 
void TRIG_RIGHT_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(TRIG_RIGHT__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define TRIG_RIGHT_DRIVE_MODE_BITS        (3)
    #define TRIG_RIGHT_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - TRIG_RIGHT_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the TRIG_RIGHT_SetDriveMode() function.
         *  @{
         */
        #define TRIG_RIGHT_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define TRIG_RIGHT_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define TRIG_RIGHT_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define TRIG_RIGHT_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define TRIG_RIGHT_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define TRIG_RIGHT_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define TRIG_RIGHT_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define TRIG_RIGHT_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define TRIG_RIGHT_MASK               TRIG_RIGHT__MASK
#define TRIG_RIGHT_SHIFT              TRIG_RIGHT__SHIFT
#define TRIG_RIGHT_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in TRIG_RIGHT_SetInterruptMode() function.
     *  @{
     */
        #define TRIG_RIGHT_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define TRIG_RIGHT_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define TRIG_RIGHT_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define TRIG_RIGHT_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(TRIG_RIGHT__SIO)
    #define TRIG_RIGHT_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(TRIG_RIGHT__PC) && (CY_PSOC4_4200L)
    #define TRIG_RIGHT_USBIO_ENABLE               ((uint32)0x80000000u)
    #define TRIG_RIGHT_USBIO_DISABLE              ((uint32)(~TRIG_RIGHT_USBIO_ENABLE))
    #define TRIG_RIGHT_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define TRIG_RIGHT_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define TRIG_RIGHT_USBIO_ENTER_SLEEP          ((uint32)((1u << TRIG_RIGHT_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << TRIG_RIGHT_USBIO_SUSPEND_DEL_SHIFT)))
    #define TRIG_RIGHT_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << TRIG_RIGHT_USBIO_SUSPEND_SHIFT)))
    #define TRIG_RIGHT_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << TRIG_RIGHT_USBIO_SUSPEND_DEL_SHIFT)))
    #define TRIG_RIGHT_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(TRIG_RIGHT__PC)
    /* Port Configuration */
    #define TRIG_RIGHT_PC                 (* (reg32 *) TRIG_RIGHT__PC)
#endif
/* Pin State */
#define TRIG_RIGHT_PS                     (* (reg32 *) TRIG_RIGHT__PS)
/* Data Register */
#define TRIG_RIGHT_DR                     (* (reg32 *) TRIG_RIGHT__DR)
/* Input Buffer Disable Override */
#define TRIG_RIGHT_INP_DIS                (* (reg32 *) TRIG_RIGHT__PC2)

/* Interrupt configuration Registers */
#define TRIG_RIGHT_INTCFG                 (* (reg32 *) TRIG_RIGHT__INTCFG)
#define TRIG_RIGHT_INTSTAT                (* (reg32 *) TRIG_RIGHT__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define TRIG_RIGHT_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(TRIG_RIGHT__SIO)
    #define TRIG_RIGHT_SIO_REG            (* (reg32 *) TRIG_RIGHT__SIO)
#endif /* (TRIG_RIGHT__SIO_CFG) */

/* USBIO registers */
#if !defined(TRIG_RIGHT__PC) && (CY_PSOC4_4200L)
    #define TRIG_RIGHT_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define TRIG_RIGHT_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define TRIG_RIGHT_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define TRIG_RIGHT_DRIVE_MODE_SHIFT       (0x00u)
#define TRIG_RIGHT_DRIVE_MODE_MASK        (0x07u << TRIG_RIGHT_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins TRIG_RIGHT_H */


/* [] END OF FILE */
