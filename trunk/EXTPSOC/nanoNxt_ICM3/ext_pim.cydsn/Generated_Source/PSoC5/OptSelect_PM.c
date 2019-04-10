/*******************************************************************************
* File Name: OptSelect_PM.c
* Version 1.80
*
* Description:
*  This file contains the setup, control, and status commands to support 
*  the component operation in the low power mode. 
*
* Note:
*
********************************************************************************
* Copyright 2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "OptSelect.h"

/* Check for removal by optimization */
#if !defined(OptSelect_Sync_ctrl_reg__REMOVED)

static OptSelect_BACKUP_STRUCT  OptSelect_backup = {0u};

    
/*******************************************************************************
* Function Name: OptSelect_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the control register value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void OptSelect_SaveConfig(void) 
{
    OptSelect_backup.controlState = OptSelect_Control;
}


/*******************************************************************************
* Function Name: OptSelect_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the control register value.
*
* Parameters:
*  None
*
* Return:
*  None
*
*
*******************************************************************************/
void OptSelect_RestoreConfig(void) 
{
     OptSelect_Control = OptSelect_backup.controlState;
}


/*******************************************************************************
* Function Name: OptSelect_Sleep
********************************************************************************
*
* Summary:
*  Prepares the component for entering the low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void OptSelect_Sleep(void) 
{
    OptSelect_SaveConfig();
}


/*******************************************************************************
* Function Name: OptSelect_Wakeup
********************************************************************************
*
* Summary:
*  Restores the component after waking up from the low power mode.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void OptSelect_Wakeup(void)  
{
    OptSelect_RestoreConfig();
}

#endif /* End check for removal by optimization */


/* [] END OF FILE */
