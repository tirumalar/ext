/*******************************************************************************
* File Name: VREFA.h  
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

#if !defined(CY_PINS_VREFA_H) /* Pins VREFA_H */
#define CY_PINS_VREFA_H

#include "cytypes.h"
#include "cyfitter.h"
#include "VREFA_aliases.h"


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
} VREFA_BACKUP_STRUCT;

/** @} structures */


/***************************************
*        Function Prototypes             
***************************************/
/**
* \addtogroup group_general
* @{
*/
uint8   VREFA_Read(void);
void    VREFA_Write(uint8 value);
uint8   VREFA_ReadDataReg(void);
#if defined(VREFA__PC) || (CY_PSOC4_4200L) 
    void    VREFA_SetDriveMode(uint8 mode);
#endif
void    VREFA_SetInterruptMode(uint16 position, uint16 mode);
uint8   VREFA_ClearInterrupt(void);
/** @} general */

/**
* \addtogroup group_power
* @{
*/
void VREFA_Sleep(void); 
void VREFA_Wakeup(void);
/** @} power */


/***************************************
*           API Constants        
***************************************/
#if defined(VREFA__PC) || (CY_PSOC4_4200L) 
    /* Drive Modes */
    #define VREFA_DRIVE_MODE_BITS        (3)
    #define VREFA_DRIVE_MODE_IND_MASK    (0xFFFFFFFFu >> (32 - VREFA_DRIVE_MODE_BITS))

    /**
    * \addtogroup group_constants
    * @{
    */
        /** \addtogroup driveMode Drive mode constants
         * \brief Constants to be passed as "mode" parameter in the VREFA_SetDriveMode() function.
         *  @{
         */
        #define VREFA_DM_ALG_HIZ         (0x00u) /**< \brief High Impedance Analog   */
        #define VREFA_DM_DIG_HIZ         (0x01u) /**< \brief High Impedance Digital  */
        #define VREFA_DM_RES_UP          (0x02u) /**< \brief Resistive Pull Up       */
        #define VREFA_DM_RES_DWN         (0x03u) /**< \brief Resistive Pull Down     */
        #define VREFA_DM_OD_LO           (0x04u) /**< \brief Open Drain, Drives Low  */
        #define VREFA_DM_OD_HI           (0x05u) /**< \brief Open Drain, Drives High */
        #define VREFA_DM_STRONG          (0x06u) /**< \brief Strong Drive            */
        #define VREFA_DM_RES_UPDWN       (0x07u) /**< \brief Resistive Pull Up/Down  */
        /** @} driveMode */
    /** @} group_constants */
#endif

/* Digital Port Constants */
#define VREFA_MASK               VREFA__MASK
#define VREFA_SHIFT              VREFA__SHIFT
#define VREFA_WIDTH              1u

/**
* \addtogroup group_constants
* @{
*/
    /** \addtogroup intrMode Interrupt constants
     * \brief Constants to be passed as "mode" parameter in VREFA_SetInterruptMode() function.
     *  @{
     */
        #define VREFA_INTR_NONE      ((uint16)(0x0000u)) /**< \brief Disabled             */
        #define VREFA_INTR_RISING    ((uint16)(0x5555u)) /**< \brief Rising edge trigger  */
        #define VREFA_INTR_FALLING   ((uint16)(0xaaaau)) /**< \brief Falling edge trigger */
        #define VREFA_INTR_BOTH      ((uint16)(0xffffu)) /**< \brief Both edge trigger    */
    /** @} intrMode */
/** @} group_constants */

/* SIO LPM definition */
#if defined(VREFA__SIO)
    #define VREFA_SIO_LPM_MASK       (0x03u)
#endif

/* USBIO definitions */
#if !defined(VREFA__PC) && (CY_PSOC4_4200L)
    #define VREFA_USBIO_ENABLE               ((uint32)0x80000000u)
    #define VREFA_USBIO_DISABLE              ((uint32)(~VREFA_USBIO_ENABLE))
    #define VREFA_USBIO_SUSPEND_SHIFT        CYFLD_USBDEVv2_USB_SUSPEND__OFFSET
    #define VREFA_USBIO_SUSPEND_DEL_SHIFT    CYFLD_USBDEVv2_USB_SUSPEND_DEL__OFFSET
    #define VREFA_USBIO_ENTER_SLEEP          ((uint32)((1u << VREFA_USBIO_SUSPEND_SHIFT) \
                                                        | (1u << VREFA_USBIO_SUSPEND_DEL_SHIFT)))
    #define VREFA_USBIO_EXIT_SLEEP_PH1       ((uint32)~((uint32)(1u << VREFA_USBIO_SUSPEND_SHIFT)))
    #define VREFA_USBIO_EXIT_SLEEP_PH2       ((uint32)~((uint32)(1u << VREFA_USBIO_SUSPEND_DEL_SHIFT)))
    #define VREFA_USBIO_CR1_OFF              ((uint32)0xfffffffeu)
#endif


/***************************************
*             Registers        
***************************************/
/* Main Port Registers */
#if defined(VREFA__PC)
    /* Port Configuration */
    #define VREFA_PC                 (* (reg32 *) VREFA__PC)
#endif
/* Pin State */
#define VREFA_PS                     (* (reg32 *) VREFA__PS)
/* Data Register */
#define VREFA_DR                     (* (reg32 *) VREFA__DR)
/* Input Buffer Disable Override */
#define VREFA_INP_DIS                (* (reg32 *) VREFA__PC2)

/* Interrupt configuration Registers */
#define VREFA_INTCFG                 (* (reg32 *) VREFA__INTCFG)
#define VREFA_INTSTAT                (* (reg32 *) VREFA__INTSTAT)

/* "Interrupt cause" register for Combined Port Interrupt (AllPortInt) in GSRef component */
#if defined (CYREG_GPIO_INTR_CAUSE)
    #define VREFA_INTR_CAUSE         (* (reg32 *) CYREG_GPIO_INTR_CAUSE)
#endif

/* SIO register */
#if defined(VREFA__SIO)
    #define VREFA_SIO_REG            (* (reg32 *) VREFA__SIO)
#endif /* (VREFA__SIO_CFG) */

/* USBIO registers */
#if !defined(VREFA__PC) && (CY_PSOC4_4200L)
    #define VREFA_USB_POWER_REG       (* (reg32 *) CYREG_USBDEVv2_USB_POWER_CTRL)
    #define VREFA_CR1_REG             (* (reg32 *) CYREG_USBDEVv2_CR1)
    #define VREFA_USBIO_CTRL_REG      (* (reg32 *) CYREG_USBDEVv2_USB_USBIO_CTRL)
#endif    
    
    
/***************************************
* The following code is DEPRECATED and 
* must not be used in new designs.
***************************************/
/**
* \addtogroup group_deprecated
* @{
*/
#define VREFA_DRIVE_MODE_SHIFT       (0x00u)
#define VREFA_DRIVE_MODE_MASK        (0x07u << VREFA_DRIVE_MODE_SHIFT)
/** @} deprecated */

#endif /* End Pins VREFA_H */


/* [] END OF FILE */
