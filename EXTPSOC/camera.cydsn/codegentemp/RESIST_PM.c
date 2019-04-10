/*******************************************************************************
* File Name: RESIST.c  
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
#include "RESIST.h"

static RESIST_BACKUP_STRUCT  RESIST_backup = {0u, 0u, 0u};


/*******************************************************************************
* Function Name: RESIST_Sleep
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
*  \snippet RESIST_SUT.c usage_RESIST_Sleep_Wakeup
*******************************************************************************/
void RESIST_Sleep(void)
{
    #if defined(RESIST__PC)
        RESIST_backup.pcState = RESIST_PC;
    #else
        #if (CY_PSOC4_4200L)
            /* Save the regulator state and put the PHY into suspend mode */
            RESIST_backup.usbState = RESIST_CR1_REG;
            RESIST_USB_POWER_REG |= RESIST_USBIO_ENTER_SLEEP;
            RESIST_CR1_REG &= RESIST_USBIO_CR1_OFF;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(RESIST__SIO)
        RESIST_backup.sioState = RESIST_SIO_REG;
        /* SIO requires unregulated output buffer and single ended input buffer */
        RESIST_SIO_REG &= (uint32)(~RESIST_SIO_LPM_MASK);
    #endif  
}


/*******************************************************************************
* Function Name: RESIST_Wakeup
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
*  Refer to RESIST_Sleep() for an example usage.
*******************************************************************************/
void RESIST_Wakeup(void)
{
    #if defined(RESIST__PC)
        RESIST_PC = RESIST_backup.pcState;
    #else
        #if (CY_PSOC4_4200L)
            /* Restore the regulator state and come out of suspend mode */
            RESIST_USB_POWER_REG &= RESIST_USBIO_EXIT_SLEEP_PH1;
            RESIST_CR1_REG = RESIST_backup.usbState;
            RESIST_USB_POWER_REG &= RESIST_USBIO_EXIT_SLEEP_PH2;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(RESIST__SIO)
        RESIST_SIO_REG = RESIST_backup.sioState;
    #endif
}


/* [] END OF FILE */
