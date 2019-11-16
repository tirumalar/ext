/*******************************************************************************
* File Name: TIMER_PM.c
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

#include "TIMER.h"

static TIMER_BACKUP_STRUCT TIMER_backup;


/*******************************************************************************
* Function Name: TIMER_SaveConfig
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
void TIMER_SaveConfig(void)
{

}


/*******************************************************************************
* Function Name: TIMER_Sleep
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
void TIMER_Sleep(void)
{
    if(0u != (TIMER_BLOCK_CONTROL_REG & TIMER_MASK))
    {
        TIMER_backup.enableState = 1u;
    }
    else
    {
        TIMER_backup.enableState = 0u;
    }

    TIMER_Stop();
    TIMER_SaveConfig();
}


/*******************************************************************************
* Function Name: TIMER_RestoreConfig
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
void TIMER_RestoreConfig(void)
{

}


/*******************************************************************************
* Function Name: TIMER_Wakeup
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
void TIMER_Wakeup(void)
{
    TIMER_RestoreConfig();

    if(0u != TIMER_backup.enableState)
    {
        TIMER_Enable();
    }
}


/* [] END OF FILE */
