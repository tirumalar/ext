/*******************************************************************************
* File Name: TRIG_LEFT.c  
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
#include "TRIG_LEFT.h"

static TRIG_LEFT_BACKUP_STRUCT  TRIG_LEFT_backup = {0u, 0u, 0u};


/*******************************************************************************
* Function Name: TRIG_LEFT_Sleep
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
*  \snippet TRIG_LEFT_SUT.c usage_TRIG_LEFT_Sleep_Wakeup
*******************************************************************************/
void TRIG_LEFT_Sleep(void)
{
    #if defined(TRIG_LEFT__PC)
        TRIG_LEFT_backup.pcState = TRIG_LEFT_PC;
    #else
        #if (CY_PSOC4_4200L)
            /* Save the regulator state and put the PHY into suspend mode */
            TRIG_LEFT_backup.usbState = TRIG_LEFT_CR1_REG;
            TRIG_LEFT_USB_POWER_REG |= TRIG_LEFT_USBIO_ENTER_SLEEP;
            TRIG_LEFT_CR1_REG &= TRIG_LEFT_USBIO_CR1_OFF;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(TRIG_LEFT__SIO)
        TRIG_LEFT_backup.sioState = TRIG_LEFT_SIO_REG;
        /* SIO requires unregulated output buffer and single ended input buffer */
        TRIG_LEFT_SIO_REG &= (uint32)(~TRIG_LEFT_SIO_LPM_MASK);
    #endif  
}


/*******************************************************************************
* Function Name: TRIG_LEFT_Wakeup
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
*  Refer to TRIG_LEFT_Sleep() for an example usage.
*******************************************************************************/
void TRIG_LEFT_Wakeup(void)
{
    #if defined(TRIG_LEFT__PC)
        TRIG_LEFT_PC = TRIG_LEFT_backup.pcState;
    #else
        #if (CY_PSOC4_4200L)
            /* Restore the regulator state and come out of suspend mode */
            TRIG_LEFT_USB_POWER_REG &= TRIG_LEFT_USBIO_EXIT_SLEEP_PH1;
            TRIG_LEFT_CR1_REG = TRIG_LEFT_backup.usbState;
            TRIG_LEFT_USB_POWER_REG &= TRIG_LEFT_USBIO_EXIT_SLEEP_PH2;
        #endif
    #endif
    #if defined(CYIPBLOCK_m0s8ioss_VERSION) && defined(TRIG_LEFT__SIO)
        TRIG_LEFT_SIO_REG = TRIG_LEFT_backup.sioState;
    #endif
}


/* [] END OF FILE */
