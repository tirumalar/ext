/*******************************************************************************
* File Name: Pulse_50us_PM.c
* Version 2.80
*
*  Description:
*     This file provides the power management source code to API for the
*     Timer.
*
*   Note:
*     None
*
*******************************************************************************
* Copyright 2008-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
********************************************************************************/

#include "Pulse_50us.h"

static Pulse_50us_backupStruct Pulse_50us_backup;


/*******************************************************************************
* Function Name: Pulse_50us_SaveConfig
********************************************************************************
*
* Summary:
*     Save the current user configuration
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Pulse_50us_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void Pulse_50us_SaveConfig(void) 
{
    #if (!Pulse_50us_UsingFixedFunction)
        Pulse_50us_backup.TimerUdb = Pulse_50us_ReadCounter();
        Pulse_50us_backup.InterruptMaskValue = Pulse_50us_STATUS_MASK;
        #if (Pulse_50us_UsingHWCaptureCounter)
            Pulse_50us_backup.TimerCaptureCounter = Pulse_50us_ReadCaptureCount();
        #endif /* Back Up capture counter register  */

        #if(!Pulse_50us_UDB_CONTROL_REG_REMOVED)
            Pulse_50us_backup.TimerControlRegister = Pulse_50us_ReadControlRegister();
        #endif /* Backup the enable state of the Timer component */
    #endif /* Backup non retention registers in UDB implementation. All fixed function registers are retention */
}


/*******************************************************************************
* Function Name: Pulse_50us_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the current user configuration.
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Pulse_50us_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void Pulse_50us_RestoreConfig(void) 
{   
    #if (!Pulse_50us_UsingFixedFunction)

        Pulse_50us_WriteCounter(Pulse_50us_backup.TimerUdb);
        Pulse_50us_STATUS_MASK =Pulse_50us_backup.InterruptMaskValue;
        #if (Pulse_50us_UsingHWCaptureCounter)
            Pulse_50us_SetCaptureCount(Pulse_50us_backup.TimerCaptureCounter);
        #endif /* Restore Capture counter register*/

        #if(!Pulse_50us_UDB_CONTROL_REG_REMOVED)
            Pulse_50us_WriteControlRegister(Pulse_50us_backup.TimerControlRegister);
        #endif /* Restore the enable state of the Timer component */
    #endif /* Restore non retention registers in the UDB implementation only */
}


/*******************************************************************************
* Function Name: Pulse_50us_Sleep
********************************************************************************
*
* Summary:
*     Stop and Save the user configuration
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Pulse_50us_backup.TimerEnableState:  Is modified depending on the
*  enable state of the block before entering sleep mode.
*
*******************************************************************************/
void Pulse_50us_Sleep(void) 
{
    #if(!Pulse_50us_UDB_CONTROL_REG_REMOVED)
        /* Save Counter's enable state */
        if(Pulse_50us_CTRL_ENABLE == (Pulse_50us_CONTROL & Pulse_50us_CTRL_ENABLE))
        {
            /* Timer is enabled */
            Pulse_50us_backup.TimerEnableState = 1u;
        }
        else
        {
            /* Timer is disabled */
            Pulse_50us_backup.TimerEnableState = 0u;
        }
    #endif /* Back up enable state from the Timer control register */
    Pulse_50us_Stop();
    Pulse_50us_SaveConfig();
}


/*******************************************************************************
* Function Name: Pulse_50us_Wakeup
********************************************************************************
*
* Summary:
*  Restores and enables the user configuration
*
* Parameters:
*  void
*
* Return:
*  void
*
* Global variables:
*  Pulse_50us_backup.enableState:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void Pulse_50us_Wakeup(void) 
{
    Pulse_50us_RestoreConfig();
    #if(!Pulse_50us_UDB_CONTROL_REG_REMOVED)
        if(Pulse_50us_backup.TimerEnableState == 1u)
        {     /* Enable Timer's operation */
                Pulse_50us_Enable();
        } /* Do nothing if Timer was disabled before */
    #endif /* Remove this code section if Control register is removed */
}


/* [] END OF FILE */
