/*******************************************************************************
* File Name: OLD_SCL.c  
* Version 2.20
*
* Description:
*  This file contains APIs to set up the Pins component for low power modes.
*
* Note:
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "cytypes.h"
#include "OLD_SCL.h"

static OLD_SCL_BACKUP_STRUCT  OLD_SCL_backup = {0u, 0u, 0u};


/*******************************************************************************
* Function Name: OLD_SCL_Sleep
****************************************************************************//**
*
* \brief Stores the pin configuration and prepares the pin for entering chip 
*  deep-sleep/hibernate modes. This function applies only to SIO and USBIO pins.
*  It should not be called for GPIO or GPIO_OVT pins.
*
* <b>Note</b> This function is available in PSoC 4 only.
*
* \return 
*  None 
*  
* \sideeffect
*  For SIO pins, this function configures the pin input threshold to CMOS and
*  drive level to Vddio. This is needed for SIO pins when in device 
*  deep-sleep/hibernate modes.
*
* \funcusage
*  \snippet OLD_SCL_SUT.c usage_OLD_SCL_Sleep_Wakeup
*******************************************************************************/
void OLD_SCL_Sleep(void)
{
    #if defined(OLD_SCL__PC)
        OLD_SCL_backup.pcState = OLD_SCL_PC;
    #else
        #if (CY_PSOC4_4200L)
            /* Save the regulator state and put the PHY into suspend mode */
            OLD_SCL_backup.usbState = OLD_SCL_CR1_REG;
            OLD_SCL_USB_POWER_REG |= OLD_SCL_USBIO_ENTER_SLEEP;
            OLD_SCL_CR1_REG &= OLD_SCL_USBIO_CR1_OFF;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(OLD_SCL__SIO)
        OLD_SCL_backup.sioState = OLD_SCL_SIO_REG;
        /* SIO requires unregulated output buffer and single ended input buffer */
        OLD_SCL_SIO_REG &= (uint32)(~OLD_SCL_SIO_LPM_MASK);
    #endif  
}


/*******************************************************************************
* Function Name: OLD_SCL_Wakeup
****************************************************************************//**
*
* \brief Restores the pin configuration that was saved during Pin_Sleep(). This 
* function applies only to SIO and USBIO pins. It should not be called for
* GPIO or GPIO_OVT pins.
*
* For USBIO pins, the wakeup is only triggered for falling edge interrupts.
*
* <b>Note</b> This function is available in PSoC 4 only.
*
* \return 
*  None
*  
* \funcusage
*  Refer to OLD_SCL_Sleep() for an example usage.
*******************************************************************************/
void OLD_SCL_Wakeup(void)
{
    #if defined(OLD_SCL__PC)
        OLD_SCL_PC = OLD_SCL_backup.pcState;
    #else
        #if (CY_PSOC4_4200L)
            /* Restore the regulator state and come out of suspend mode */
            OLD_SCL_USB_POWER_REG &= OLD_SCL_USBIO_EXIT_SLEEP_PH1;
            OLD_SCL_CR1_REG = OLD_SCL_backup.usbState;
            OLD_SCL_USB_POWER_REG &= OLD_SCL_USBIO_EXIT_SLEEP_PH2;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(OLD_SCL__SIO)
        OLD_SCL_SIO_REG = OLD_SCL_backup.sioState;
    #endif
}


/* [] END OF FILE */
