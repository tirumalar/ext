/*******************************************************************************
* File Name: Timer_Tick_PM.c
* Version 2.10
*
* Description:
*  This file contains the setup, control, and status commands to support
*  the component operations in the low power mode.
*
* Note:
*  None
*
********************************************************************************
* Copyright 2013-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "Timer_Tick.h"

static Timer_Tick_BACKUP_STRUCT Timer_Tick_backup;


/*******************************************************************************
* Function Name: Timer_Tick_SaveConfig
********************************************************************************
*
* Summary:
*  All configuration registers are retention. Nothing to save here.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Timer_Tick_SaveConfig(void)
{

}


/*******************************************************************************
* Function Name: Timer_Tick_Sleep
********************************************************************************
*
* Summary:
*  Stops the component operation and saves the user configuration.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Timer_Tick_Sleep(void)
{
    if(0u != (Timer_Tick_BLOCK_CONTROL_REG & Timer_Tick_MASK))
    {
        Timer_Tick_backup.enableState = 1u;
    }
    else
    {
        Timer_Tick_backup.enableState = 0u;
    }

    Timer_Tick_Stop();
    Timer_Tick_SaveConfig();
}


/*******************************************************************************
* Function Name: Timer_Tick_RestoreConfig
********************************************************************************
*
* Summary:
*  All configuration registers are retention. Nothing to restore here.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Timer_Tick_RestoreConfig(void)
{

}


/*******************************************************************************
* Function Name: Timer_Tick_Wakeup
********************************************************************************
*
* Summary:
*  Restores the user configuration and restores the enable state.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Timer_Tick_Wakeup(void)
{
    Timer_Tick_RestoreConfig();

    if(0u != Timer_Tick_backup.enableState)
    {
        Timer_Tick_Enable();
    }
}


/* [] END OF FILE */
