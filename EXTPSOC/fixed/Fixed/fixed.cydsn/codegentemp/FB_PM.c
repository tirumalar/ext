/*******************************************************************************
* File Name: FB.c  
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
#include "FB.h"

static FB_BACKUP_STRUCT  FB_backup = {0u, 0u, 0u};


/*******************************************************************************
* Function Name: FB_Sleep
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
*  \snippet FB_SUT.c usage_FB_Sleep_Wakeup
*******************************************************************************/
void FB_Sleep(void)
{
    #if defined(FB__PC)
        FB_backup.pcState = FB_PC;
    #else
        #if (CY_PSOC4_4200L)
            /* Save the regulator state and put the PHY into suspend mode */
            FB_backup.usbState = FB_CR1_REG;
            FB_USB_POWER_REG |= FB_USBIO_ENTER_SLEEP;
            FB_CR1_REG &= FB_USBIO_CR1_OFF;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(FB__SIO)
        FB_backup.sioState = FB_SIO_REG;
        /* SIO requires unregulated output buffer and single ended input buffer */
        FB_SIO_REG &= (uint32)(~FB_SIO_LPM_MASK);
    #endif  
}


/*******************************************************************************
* Function Name: FB_Wakeup
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
*  Refer to FB_Sleep() for an example usage.
*******************************************************************************/
void FB_Wakeup(void)
{
    #if defined(FB__PC)
        FB_PC = FB_backup.pcState;
    #else
        #if (CY_PSOC4_4200L)
            /* Restore the regulator state and come out of suspend mode */
            FB_USB_POWER_REG &= FB_USBIO_EXIT_SLEEP_PH1;
            FB_CR1_REG = FB_backup.usbState;
            FB_USB_POWER_REG &= FB_USBIO_EXIT_SLEEP_PH2;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(FB__SIO)
        FB_SIO_REG = FB_backup.sioState;
    #endif
}


/* [] END OF FILE */
