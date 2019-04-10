/*******************************************************************************
* File Name: UART_TIMER_PM.c
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

#include "UART_TIMER.h"

static UART_TIMER_backupStruct UART_TIMER_backup;


/*******************************************************************************
* Function Name: UART_TIMER_SaveConfig
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
*  UART_TIMER_backup:  Variables of this global structure are modified to
*  store the values of non retention configuration registers when Sleep() API is
*  called.
*
*******************************************************************************/
void UART_TIMER_SaveConfig(void) 
{
    #if (!UART_TIMER_UsingFixedFunction)
        UART_TIMER_backup.TimerUdb = UART_TIMER_ReadCounter();
        UART_TIMER_backup.InterruptMaskValue = UART_TIMER_STATUS_MASK;
        #if (UART_TIMER_UsingHWCaptureCounter)
            UART_TIMER_backup.TimerCaptureCounter = UART_TIMER_ReadCaptureCount();
        #endif /* Back Up capture counter register  */

        #if(!UART_TIMER_UDB_CONTROL_REG_REMOVED)
            UART_TIMER_backup.TimerControlRegister = UART_TIMER_ReadControlRegister();
        #endif /* Backup the enable state of the Timer component */
    #endif /* Backup non retention registers in UDB implementation. All fixed function registers are retention */
}


/*******************************************************************************
* Function Name: UART_TIMER_RestoreConfig
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
*  UART_TIMER_backup:  Variables of this global structure are used to
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void UART_TIMER_RestoreConfig(void) 
{   
    #if (!UART_TIMER_UsingFixedFunction)

        UART_TIMER_WriteCounter(UART_TIMER_backup.TimerUdb);
        UART_TIMER_STATUS_MASK =UART_TIMER_backup.InterruptMaskValue;
        #if (UART_TIMER_UsingHWCaptureCounter)
            UART_TIMER_SetCaptureCount(UART_TIMER_backup.TimerCaptureCounter);
        #endif /* Restore Capture counter register*/

        #if(!UART_TIMER_UDB_CONTROL_REG_REMOVED)
            UART_TIMER_WriteControlRegister(UART_TIMER_backup.TimerControlRegister);
        #endif /* Restore the enable state of the Timer component */
    #endif /* Restore non retention registers in the UDB implementation only */
}


/*******************************************************************************
* Function Name: UART_TIMER_Sleep
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
*  UART_TIMER_backup.TimerEnableState:  Is modified depending on the
*  enable state of the block before entering sleep mode.
*
*******************************************************************************/
void UART_TIMER_Sleep(void) 
{
    #if(!UART_TIMER_UDB_CONTROL_REG_REMOVED)
        /* Save Counter's enable state */
        if(UART_TIMER_CTRL_ENABLE == (UART_TIMER_CONTROL & UART_TIMER_CTRL_ENABLE))
        {
            /* Timer is enabled */
            UART_TIMER_backup.TimerEnableState = 1u;
        }
        else
        {
            /* Timer is disabled */
            UART_TIMER_backup.TimerEnableState = 0u;
        }
    #endif /* Back up enable state from the Timer control register */
    UART_TIMER_Stop();
    UART_TIMER_SaveConfig();
}


/*******************************************************************************
* Function Name: UART_TIMER_Wakeup
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
*  UART_TIMER_backup.enableState:  Is used to restore the enable state of
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void UART_TIMER_Wakeup(void) 
{
    UART_TIMER_RestoreConfig();
    #if(!UART_TIMER_UDB_CONTROL_REG_REMOVED)
        if(UART_TIMER_backup.TimerEnableState == 1u)
        {     /* Enable Timer's operation */
                UART_TIMER_Enable();
        } /* Do nothing if Timer was disabled before */
    #endif /* Remove this code section if Control register is removed */
}


/* [] END OF FILE */
