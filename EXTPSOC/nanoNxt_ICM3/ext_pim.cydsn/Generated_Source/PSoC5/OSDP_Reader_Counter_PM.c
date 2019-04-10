/*******************************************************************************
* File Name: OSDP_Reader_Counter_PM.c  
* Version 3.0
*
*  Description:
*    This file provides the power management source code to API for the
*    Counter.  
*
*   Note:
*     None
*
********************************************************************************
* Copyright 2008-2012, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/

#include "OSDP_Reader_Counter.h"

static OSDP_Reader_Counter_backupStruct OSDP_Reader_Counter_backup;


/*******************************************************************************
* Function Name: OSDP_Reader_Counter_SaveConfig
********************************************************************************
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
*  OSDP_Reader_Counter_backup:  Variables of this global structure are modified to 
*  store the values of non retention configuration registers when Sleep() API is 
*  called.
*
*******************************************************************************/
void OSDP_Reader_Counter_SaveConfig(void) 
{
    #if (!OSDP_Reader_Counter_UsingFixedFunction)

        OSDP_Reader_Counter_backup.CounterUdb = OSDP_Reader_Counter_ReadCounter();

        #if(!OSDP_Reader_Counter_ControlRegRemoved)
            OSDP_Reader_Counter_backup.CounterControlRegister = OSDP_Reader_Counter_ReadControlRegister();
        #endif /* (!OSDP_Reader_Counter_ControlRegRemoved) */

    #endif /* (!OSDP_Reader_Counter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: OSDP_Reader_Counter_RestoreConfig
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
*  OSDP_Reader_Counter_backup:  Variables of this global structure are used to 
*  restore the values of non retention registers on wakeup from sleep mode.
*
*******************************************************************************/
void OSDP_Reader_Counter_RestoreConfig(void) 
{      
    #if (!OSDP_Reader_Counter_UsingFixedFunction)

       OSDP_Reader_Counter_WriteCounter(OSDP_Reader_Counter_backup.CounterUdb);

        #if(!OSDP_Reader_Counter_ControlRegRemoved)
            OSDP_Reader_Counter_WriteControlRegister(OSDP_Reader_Counter_backup.CounterControlRegister);
        #endif /* (!OSDP_Reader_Counter_ControlRegRemoved) */

    #endif /* (!OSDP_Reader_Counter_UsingFixedFunction) */
}


/*******************************************************************************
* Function Name: OSDP_Reader_Counter_Sleep
********************************************************************************
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
*  OSDP_Reader_Counter_backup.enableState:  Is modified depending on the enable 
*  state of the block before entering sleep mode.
*
*******************************************************************************/
void OSDP_Reader_Counter_Sleep(void) 
{
    #if(!OSDP_Reader_Counter_ControlRegRemoved)
        /* Save Counter's enable state */
        if(OSDP_Reader_Counter_CTRL_ENABLE == (OSDP_Reader_Counter_CONTROL & OSDP_Reader_Counter_CTRL_ENABLE))
        {
            /* Counter is enabled */
            OSDP_Reader_Counter_backup.CounterEnableState = 1u;
        }
        else
        {
            /* Counter is disabled */
            OSDP_Reader_Counter_backup.CounterEnableState = 0u;
        }
    #else
        OSDP_Reader_Counter_backup.CounterEnableState = 1u;
        if(OSDP_Reader_Counter_backup.CounterEnableState != 0u)
        {
            OSDP_Reader_Counter_backup.CounterEnableState = 0u;
        }
    #endif /* (!OSDP_Reader_Counter_ControlRegRemoved) */
    
    OSDP_Reader_Counter_Stop();
    OSDP_Reader_Counter_SaveConfig();
}


/*******************************************************************************
* Function Name: OSDP_Reader_Counter_Wakeup
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
*  OSDP_Reader_Counter_backup.enableState:  Is used to restore the enable state of 
*  block on wakeup from sleep mode.
*
*******************************************************************************/
void OSDP_Reader_Counter_Wakeup(void) 
{
    OSDP_Reader_Counter_RestoreConfig();
    #if(!OSDP_Reader_Counter_ControlRegRemoved)
        if(OSDP_Reader_Counter_backup.CounterEnableState == 1u)
        {
            /* Enable Counter's operation */
            OSDP_Reader_Counter_Enable();
        } /* Do nothing if Counter was disabled before */    
    #endif /* (!OSDP_Reader_Counter_ControlRegRemoved) */
    
}


/* [] END OF FILE */
