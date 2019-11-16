/*******************************************************************************
* File Name: PWM_M_PM.c
* Version 3.30
*
* Description:
*  This file provides the power management source code to API for the
*  PWM.
*
* Note:
*
********************************************************************************
* Copyright 2008-2014, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "PWM_M.h"

static PWM_M_backupStruct PWM_M_backup;


/*******************************************************************************
* Function Name: PWM_M_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_M_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void PWM_M_SaveConfig(void) 
{

    #if(!PWM_M_UsingFixedFunction)
        #if(!PWM_M_PWMModeIsCenterAligned)
            PWM_M_backup.PWMPeriod = PWM_M_ReadPeriod();
        #endif /* (!PWM_M_PWMModeIsCenterAligned) */
        PWM_M_backup.PWMUdb = PWM_M_ReadCounter();
        #if (PWM_M_UseStatus)
            PWM_M_backup.InterruptMaskValue = PWM_M_STATUS_MASK;
        #endif /* (PWM_M_UseStatus) */

        #if(PWM_M_DeadBandMode == PWM_M__B_PWM__DBM_256_CLOCKS || \
            PWM_M_DeadBandMode == PWM_M__B_PWM__DBM_2_4_CLOCKS)
            PWM_M_backup.PWMdeadBandValue = PWM_M_ReadDeadTime();
        #endif /*  deadband count is either 2-4 clocks or 256 clocks */

        #if(PWM_M_KillModeMinTime)
             PWM_M_backup.PWMKillCounterPeriod = PWM_M_ReadKillTime();
        #endif /* (PWM_M_KillModeMinTime) */

        #if(PWM_M_UseControl)
            PWM_M_backup.PWMControlRegister = PWM_M_ReadControlRegister();
        #endif /* (PWM_M_UseControl) */
    #endif  /* (!PWM_M_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: PWM_M_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration of the component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_M_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_M_RestoreConfig(void) 
{
        #if(!PWM_M_UsingFixedFunction)
            #if(!PWM_M_PWMModeIsCenterAligned)
                PWM_M_WritePeriod(PWM_M_backup.PWMPeriod);
            #endif /* (!PWM_M_PWMModeIsCenterAligned) */

            PWM_M_WriteCounter(PWM_M_backup.PWMUdb);

            #if (PWM_M_UseStatus)
                PWM_M_STATUS_MASK = PWM_M_backup.InterruptMaskValue;
            #endif /* (PWM_M_UseStatus) */

            #if(PWM_M_DeadBandMode == PWM_M__B_PWM__DBM_256_CLOCKS || \
                PWM_M_DeadBandMode == PWM_M__B_PWM__DBM_2_4_CLOCKS)
                PWM_M_WriteDeadTime(PWM_M_backup.PWMdeadBandValue);
            #endif /* deadband count is either 2-4 clocks or 256 clocks */

            #if(PWM_M_KillModeMinTime)
                PWM_M_WriteKillTime(PWM_M_backup.PWMKillCounterPeriod);
            #endif /* (PWM_M_KillModeMinTime) */

            #if(PWM_M_UseControl)
                PWM_M_WriteControlRegister(PWM_M_backup.PWMControlRegister);
            #endif /* (PWM_M_UseControl) */
        #endif  /* (!PWM_M_UsingFixedFunction) */
    }


/*******************************************************************************
* Function Name: PWM_M_Sleep
********************************************************************************
*
* Summary:
*  Disables block's operation and saves the user configuration. Should be called
*  just prior to entering sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_M_backup.PWMEnableState:  Is modified depending on the enable
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void PWM_M_Sleep(void) 
{
    #if(PWM_M_UseControl)
        if(PWM_M_CTRL_ENABLE == (PWM_M_CONTROL & PWM_M_CTRL_ENABLE))
        {
            /*Component is enabled */
            PWM_M_backup.PWMEnableState = 1u;
        }
        else
        {
            /* Component is disabled */
            PWM_M_backup.PWMEnableState = 0u;
        }
    #endif /* (PWM_M_UseControl) */

    /* Stop component */
    PWM_M_Stop();

    /* Save registers configuration */
    PWM_M_SaveConfig();
}


/*******************************************************************************
* Function Name: PWM_M_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration. Should be called just after
*  awaking from sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_M_backup.pwmEnable:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void PWM_M_Wakeup(void) 
{
     /* Restore registers values */
    PWM_M_RestoreConfig();

    if(PWM_M_backup.PWMEnableState != 0u)
    {
        /* Enable component's operation */
        PWM_M_Enable();
    } /* Do nothing if component's block was disabled before */

}


/* [] END OF FILE */
