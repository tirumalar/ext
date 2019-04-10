/*******************************************************************************
* File Name: Comp_ISENSE_PM.c
* Version 1.20
*
* Description:
*  This file provides the power management source code APIs for the
*  Comparator.
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

#include "Comp_ISENSE.h"

#if(!Comp_ISENSE_CHECK_DEEPSLEEP_SUPPORT)
    static Comp_ISENSE_BACKUP_STRUCT Comp_ISENSE_backup =
    {
        0u, /* enableState */
    };
#endif /* (Comp_ISENSE_CHECK_DEEPSLEEP_SUPPORT) */


/*******************************************************************************
* Function Name: Comp_ISENSE_SaveConfig
********************************************************************************
*
* Summary:
*  Empty function. Included for consistency with other components.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Comp_ISENSE_SaveConfig(void)
{

}


/*******************************************************************************
* Function Name: Comp_ISENSE_RestoreConfig
********************************************************************************
*
* Summary:
*  Empty function. Included for consistency with other components.
*
* Parameters:
*  None
*
* Return:
*  None
*
********************************************************************************/
void Comp_ISENSE_RestoreConfig(void)
{

}


/*******************************************************************************
* Function Name: Comp_ISENSE_Sleep
********************************************************************************
*
* Summary:
*  This is the preferred API to prepare the component for sleep. The Sleep() API 
*  saves the current component state. Call the Comp_Sleep() function before 
*  calling the CySysPmDeepSleep() or the CySysPmHibernate() functions.
*  The "Deep sleep operation" option has an influence on this function
*  implementation.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Comp_ISENSE_Sleep(void)
{
#if(!Comp_ISENSE_CHECK_DEEPSLEEP_SUPPORT)
    if(Comp_ISENSE_CHECK_PWR_MODE_OFF)
    {
        Comp_ISENSE_backup.enableState = 1u;
        Comp_ISENSE_Stop();
    }
    else /* Component is disabled */
    {
        Comp_ISENSE_backup.enableState = 0u;
    }
#endif /* (Comp_ISENSE_CHECK_DEEPSLEEP_SUPPORT) */
}


/*******************************************************************************
* Function Name: Comp_ISENSE_Wakeup
********************************************************************************
*
* Summary:
*  This is the preferred API to restore the component to the state when Sleep() 
*  was called. The Wakeup() function will also re-enable the component. 
*  The "Deep sleep operation" option has an influence on this function
*  implementation.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void Comp_ISENSE_Wakeup(void)
{
#if(!Comp_ISENSE_CHECK_DEEPSLEEP_SUPPORT)
    if(0u != Comp_ISENSE_backup.enableState)
    {
        /* Enable Comp's operation */
        Comp_ISENSE_Enable();
    } /* Do nothing if Comp was disabled before */
#endif /* (Comp_ISENSE_CHECK_DEEPSLEEP_SUPPORT) */
}


/* [] END OF FILE */

