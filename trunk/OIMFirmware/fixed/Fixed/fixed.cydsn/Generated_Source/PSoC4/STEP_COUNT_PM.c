/*******************************************************************************
* File Name: STEP_COUNT_PM.c
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

#include "STEP_COUNT.h"

static STEP_COUNT_BACKUP_STRUCT STEP_COUNT_backup;


/*******************************************************************************
* Function Name: STEP_COUNT_SaveConfig
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
void STEP_COUNT_SaveConfig(void)
{

}


/*******************************************************************************
* Function Name: STEP_COUNT_Sleep
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
void STEP_COUNT_Sleep(void)
{
    if(0u != (STEP_COUNT_BLOCK_CONTROL_REG & STEP_COUNT_MASK))
    {
        STEP_COUNT_backup.enableState = 1u;
    }
    else
    {
        STEP_COUNT_backup.enableState = 0u;
    }

    STEP_COUNT_Stop();
    STEP_COUNT_SaveConfig();
}


/*******************************************************************************
* Function Name: STEP_COUNT_RestoreConfig
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
void STEP_COUNT_RestoreConfig(void)
{

}


/*******************************************************************************
* Function Name: STEP_COUNT_Wakeup
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
void STEP_COUNT_Wakeup(void)
{
    STEP_COUNT_RestoreConfig();

    if(0u != STEP_COUNT_backup.enableState)
    {
        STEP_COUNT_Enable();
    }
}


/* [] END OF FILE */
