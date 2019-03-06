/*******************************************************************************
* File Name: AUDIO_DAC_PM.c
* Version 1.10
*
* Description:
*  This file provides Low power mode APIs for IDAC_P4 component.
*
********************************************************************************
* Copyright 2013-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "AUDIO_DAC.h"


static AUDIO_DAC_BACKUP_STRUCT AUDIO_DAC_backup;


/*******************************************************************************
* Function Name: AUDIO_DAC_SaveConfig
********************************************************************************
*
* Summary:
*  Saves component state before sleep
* Parameters:
*  None
*
* Return:
*  None
*
* Global Variables:
*  None
*
*******************************************************************************/
void AUDIO_DAC_SaveConfig(void)
{
    /* All registers are retention - nothing to save */
}


/*******************************************************************************
* Function Name: AUDIO_DAC_Sleep
********************************************************************************
*
* Summary:
*  Calls _SaveConfig() function
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void AUDIO_DAC_Sleep(void)
{
        if(0u != (AUDIO_DAC_IDAC_CONTROL_REG & ((uint32)AUDIO_DAC_IDAC_MODE_MASK <<
        AUDIO_DAC_IDAC_MODE_POSITION)))
        {
            AUDIO_DAC_backup.enableState = 1u;
        }
        else
        {
            AUDIO_DAC_backup.enableState = 0u;
        }

    AUDIO_DAC_Stop();
    AUDIO_DAC_SaveConfig();
}


/*******************************************************************************
* Function Name: AUDIO_DAC_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores component state after wakeup
* Parameters:
*  None
*
* Return:
*  None
*
* Global Variables:
*  None
*
*******************************************************************************/
void AUDIO_DAC_RestoreConfig(void)
{
    /* All registers are retention - nothing to save */
}


/*******************************************************************************
* Function Name: AUDIO_DAC_Wakeup
********************************************************************************
*
* Summary:
*  Calls _RestoreConfig() function
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void AUDIO_DAC_Wakeup(void)
{
    /* Restore IDAC register settings */
    AUDIO_DAC_RestoreConfig();
    if(AUDIO_DAC_backup.enableState == 1u)
    {
        /* Enable operation */
        AUDIO_DAC_Enable();
    } /* Do nothing if the component was disabled before */

}


/* [] END OF FILE */
