/*******************************************************************************
* File Name: OSDPReaderTimer_PM.c
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

#include "OSDPReaderTimer.h"

static OSDPReaderTimer_backupStruct OSDPReaderTimer_backup;


/*******************************************************************************
* Function Name: OSDPReaderTimer_SaveConfig
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
*  OSDPReaderTimer_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void OSDPReaderTimer_SaveConfig(void) 
{
    #if (!OSDPReaderTimer_UsingFixedFunction)
        OSDPReaderTimer_backup.TimerUdb = OSDPReaderTimer_ReadCounter();
        OSDPReaderTimer_backup.InterruptMaskValue = OSDPReaderTimer_STATUS_MASK;
        #if (OSDPReaderTimer_UsingHWCaptureCounter)
            OSDPReaderTimer_backup.TimerCaptureCounter = OSDPReaderTimer_ReadCaptureCount();
        #endif /* Back Up capture counter register  */

        #if(!OSDPReaderTimer_UDB_CONTROL_REG_REMOVED)
            OSDPReaderTimer_backup.TimerControlRegister = OSDPReaderTimer_ReadControlRegister();
        #endif /* Backup the enable state of the Timer component */
    #endif /* Backup non retention registers in UDB implementation. All fixed function registers are retention */
}


/*******************************************************************************
* Function Name: OSDPReaderTimer_RestoreConfig
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
*  OSDPReaderTimer_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void OSDPReaderTimer_RestoreConfig(void) 
{   
    #if (!OSDPReaderTimer_UsingFixedFunction)

        OSDPReaderTimer_WriteCounter(OSDPReaderTimer_backup.TimerUdb);
        OSDPReaderTimer_STATUS_MASK =OSDPReaderTimer_backup.InterruptMaskValue;
        #if (OSDPReaderTimer_UsingHWCaptureCounter)
            OSDPReaderTimer_SetCaptureCount(OSDPReaderTimer_backup.TimerCaptureCounter);
        #endif /* Restore Capture counter register*/

        #if(!OSDPReaderTimer_UDB_CONTROL_REG_REMOVED)
            OSDPReaderTimer_WriteControlRegister(OSDPReaderTimer_backup.TimerControlRegister);
        #endif /* Restore the enable state of the Timer component */
    #endif /* Restore non retention registers in the UDB implementation only */
}


/*******************************************************************************
* Function Name: OSDPReaderTimer_Sleep
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
*  OSDPReaderTimer_backup.TimerEnableState:  Is modified depending on the
*  enable state of the block before entering sleep mode.
*
*******************************************************************************/
void OSDPReaderTimer_Sleep(void) 
{
    #if(!OSDPReaderTimer_UDB_CONTROL_REG_REMOVED)
        /* Save Counter's enable state */
        if(OSDPReaderTimer_CTRL_ENABLE == (OSDPReaderTimer_CONTROL & OSDPReaderTimer_CTRL_ENABLE))
        {
            /* Timer is enabled */
            OSDPReaderTimer_backup.TimerEnableState = 1u;
        }
        else
        {
            /* Timer is disabled */
            OSDPReaderTimer_backup.TimerEnableState = 0u;
        }
    #endif /* Back up enable state from the Timer control register */
    OSDPReaderTimer_Stop();
    OSDPReaderTimer_SaveConfig();
}


/*******************************************************************************
* Function Name: OSDPReaderTimer_Wakeup
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
*  OSDPReaderTimer_backup.enableState:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void OSDPReaderTimer_Wakeup(void) 
{
    OSDPReaderTimer_RestoreConfig();
    #if(!OSDPReaderTimer_UDB_CONTROL_REG_REMOVED)
        if(OSDPReaderTimer_backup.TimerEnableState == 1u)
        {     /* Enable Timer's operation */
                OSDPReaderTimer_Enable();
        } /* Do nothing if Timer was disabled before */
    #endif /* Remove this code section if Control register is removed */
}


/* [] END OF FILE */
