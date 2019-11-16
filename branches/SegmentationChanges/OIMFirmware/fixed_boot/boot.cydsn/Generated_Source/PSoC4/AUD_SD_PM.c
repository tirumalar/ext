/*******************************************************************************
* File Name: AUD_SD.c  
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
#include "AUD_SD.h"

static AUD_SD_BACKUP_STRUCT  AUD_SD_backup = {0u, 0u, 0u};


/*******************************************************************************
* Function Name: AUD_SD_Sleep
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
*  \snippet AUD_SD_SUT.c usage_AUD_SD_Sleep_Wakeup
*******************************************************************************/
void AUD_SD_Sleep(void)
{
    #if defined(AUD_SD__PC)
        AUD_SD_backup.pcState = AUD_SD_PC;
    #else
        #if (CY_PSOC4_4200L)
            /* Save the regulator state and put the PHY into suspend mode */
            AUD_SD_backup.usbState = AUD_SD_CR1_REG;
            AUD_SD_USB_POWER_REG |= AUD_SD_USBIO_ENTER_SLEEP;
            AUD_SD_CR1_REG &= AUD_SD_USBIO_CR1_OFF;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(AUD_SD__SIO)
        AUD_SD_backup.sioState = AUD_SD_SIO_REG;
        /* SIO requires unregulated output buffer and single ended input buffer */
        AUD_SD_SIO_REG &= (uint32)(~AUD_SD_SIO_LPM_MASK);
    #endif  
}


/*******************************************************************************
* Function Name: AUD_SD_Wakeup
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
*  Refer to AUD_SD_Sleep() for an example usage.
*******************************************************************************/
void AUD_SD_Wakeup(void)
{
    #if defined(AUD_SD__PC)
        AUD_SD_PC = AUD_SD_backup.pcState;
    #else
        #if (CY_PSOC4_4200L)
            /* Restore the regulator state and come out of suspend mode */
            AUD_SD_USB_POWER_REG &= AUD_SD_USBIO_EXIT_SLEEP_PH1;
            AUD_SD_CR1_REG = AUD_SD_backup.usbState;
            AUD_SD_USB_POWER_REG &= AUD_SD_USBIO_EXIT_SLEEP_PH2;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(AUD_SD__SIO)
        AUD_SD_SIO_REG = AUD_SD_backup.sioState;
    #endif
}


/* [] END OF FILE */
