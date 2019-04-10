/*******************************************************************************
* File Name: Tamper_Timer_PM.c
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

#include "Tamper_Timer.h"

static Tamper_Timer_backupStruct Tamper_Timer_backup;


/*******************************************************************************
* Function Name: Tamper_Timer_SaveConfig
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
*  Tamper_Timer_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void Tamper_Timer_SaveConfig(void) 
{
    #if (!Tamper_Timer_UsingFixedFunction)
        Tamper_Timer_backup.TimerUdb = Tamper_Timer_ReadCounter();
        Tamper_Timer_backup.InterruptMaskValue = Tamper_Timer_STATUS_MASK;
        #if (Tamper_Timer_UsingHWCaptureCounter)
            Tamper_Timer_backup.TimerCaptureCounter = Tamper_Timer_ReadCaptureCount();
        #endif /* Back Up capture counter register  */

        #if(!Tamper_Timer_UDB_CONTROL_REG_REMOVED)
            Tamper_Timer_backup.TimerControlRegister = Tamper_Timer_ReadControlRegister();
        #endif /* Backup the enable state of the Timer component */
    #endif /* Backup non retention registers in UDB implementation. All fixed function registers are retention */
}


/*******************************************************************************
* Function Name: Tamper_Timer_RestoreConfig
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
*  Tamper_Timer_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void Tamper_Timer_RestoreConfig(void) 
{   
    #if (!Tamper_Timer_UsingFixedFunction)

        Tamper_Timer_WriteCounter(Tamper_Timer_backup.TimerUdb);
        Tamper_Timer_STATUS_MASK =Tamper_Timer_backup.InterruptMaskValue;
        #if (Tamper_Timer_UsingHWCaptureCounter)
            Tamper_Timer_SetCaptureCount(Tamper_Timer_backup.TimerCaptureCounter);
        #endif /* Restore Capture counter register*/

        #if(!Tamper_Timer_UDB_CONTROL_REG_REMOVED)
            Tamper_Timer_WriteControlRegister(Tamper_Timer_backup.TimerControlRegister);
        #endif /* Restore the enable state of the Timer component */
    #endif /* Restore non retention registers in the UDB implementation only */
}


/*******************************************************************************
* Function Name: Tamper_Timer_Sleep
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
*  Tamper_Timer_backup.TimerEnableState:  Is modified depending on the
*  enable state of the block before entering sleep mode.
*
*******************************************************************************/
void Tamper_Timer_Sleep(void) 
{
    #if(!Tamper_Timer_UDB_CONTROL_REG_REMOVED)
        /* Save Counter's enable state */
        if(Tamper_Timer_CTRL_ENABLE == (Tamper_Timer_CONTROL & Tamper_Timer_CTRL_ENABLE))
        {
            /* Timer is enabled */
            Tamper_Timer_backup.TimerEnableState = 1u;
        }
        else
        {
            /* Timer is disabled */
            Tamper_Timer_backup.TimerEnableState = 0u;
        }
    #endif /* Back up enable state from the Timer control register */
    Tamper_Timer_Stop();
    Tamper_Timer_SaveConfig();
}


/*******************************************************************************
* Function Name: Tamper_Timer_Wakeup
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
*  Tamper_Timer_backup.enableState:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void Tamper_Timer_Wakeup(void) 
{
    Tamper_Timer_RestoreConfig();
    #if(!Tamper_Timer_UDB_CONTROL_REG_REMOVED)
        if(Tamper_Timer_backup.TimerEnableState == 1u)
        {     /* Enable Timer's operation */
                Tamper_Timer_Enable();
        } /* Do nothing if Timer was disabled before */
    #endif /* Remove this code section if Control register is removed */
}


/* [] END OF FILE */
