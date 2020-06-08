/*******************************************************************************
* File Name: EZI2C_PM.c
* Version 1.90
*
* Description:
*  This file contains the API for the proper switching to/from the low power
*  modes.
*
********************************************************************************
* Copyright 2008-2013, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "EZI2C_PVT.h"

       
EZI2C_BACKUP_STRUCT  EZI2C_backup =
{
    /* enable state - disabled */
    EZI2C_DISABLED,

    /* xcfg: wakeup disabled, enabled hardware addr detection */
    EZI2C_XCFG_HDWR_ADDR_EN,

    /* addr: default address (0x04) */
    EZI2C_DEFAULT_ADDR1,

    /* cfg: default bus speed - 100kHz, so write 0 (16 samples/bit) */
    0x00u,

    /* clkDiv1 */
    LO8(BCLK__BUS_CLK__KHZ / EZI2C_BUS_SPEED),

    /* clkDiv2 */
    HI8(BCLK__BUS_CLK__KHZ / EZI2C_BUS_SPEED)
};

#if(EZI2C_WAKEUP_ENABLED)
    volatile uint8 EZI2C_wakeupSource;
#endif /* (EZI2C_WAKEUP_ENABLED) */
    

/*******************************************************************************
* Function Name: EZI2C_SaveConfig
********************************************************************************
*
* Summary:
*  Saves the current user configuration of the EZI2C component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  EZI2C_backup - the non retention registers are saved to.
*
* Reentrant:
*  No
*
*******************************************************************************/
void EZI2C_SaveConfig(void) 
{

    #if(EZI2C_WAKEUP_ENABLED)
        uint8 interruptState;
    #endif /* (EZI2C_WAKEUP_ENABLED) */

    EZI2C_backup.xcfg = EZI2C_XCFG_REG;
    EZI2C_backup.adr  = EZI2C_ADDR_REG;
    EZI2C_backup.cfg  = EZI2C_CFG_REG;
    EZI2C_backup.clkDiv1  = EZI2C_CLKDIV1_REG;
    EZI2C_backup.clkDiv2  = EZI2C_CLKDIV2_REG;

    #if(EZI2C_WAKEUP_ENABLED)
        /* Enable the I2C backup regulator  */
        interruptState = CyEnterCriticalSection();
        EZI2C_PWRSYS_CR1_REG |= EZI2C_PWRSYS_CR1_I2C_BACKUP;
        CyExitCriticalSection(interruptState);

        /* Set force nack before putting the device to power off mode.
        *  It is cleared on wake up.
        */
        EZI2C_XCFG_REG |= EZI2C_XCFG_FORCE_NACK;
        while(0u == (EZI2C_XCFG_REG & EZI2C_XCFG_SLEEP_READY))
        {
            /* Waits for ongoing transaction to be completed. */
        }
        
         /* Setup wakeup interrupt */
        EZI2C_DisableInt();
        (void) CyIntSetVector(EZI2C_ISR_NUMBER, &EZI2C_WAKEUP_ISR);
        EZI2C_wakeupSource = 0u; /* Clear wakeup event */
        EZI2C_EnableInt();

        /* Leave interrupt enabled to wake up */
            
    #endif /* (EZI2C_WAKEUP_ENABLED) */
}


/*******************************************************************************
* Function Name: EZI2C_RestoreConfig
********************************************************************************
*
* Summary:
*  Restores the previously saved by EZI2C_SaveConfig() or 
*  EZI2C_Sleep() configuration of the EZI2C component.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  EZI2C_backup - the non retention registers are restored from.
*
* Reentrant:
*  No
*
* Side Effects:
*  Calling this function before EZI2C_SaveConfig() or
*  EZI2C_Sleep() will lead to unpredictable results.
*
*******************************************************************************/
void EZI2C_RestoreConfig(void) 
{
    uint8 enableInterrupts;

    /* Enabled if was in Sleep */
    if(0u != (EZI2C_PWRSYS_CR1_I2C_BACKUP & EZI2C_PWRSYS_CR1_REG))    
    {
        /* Disable back-up regulator */
        enableInterrupts = CyEnterCriticalSection();
        /* Disable the I2C regulator backup */
        EZI2C_PWRSYS_CR1_REG &= ((uint8) ~EZI2C_PWRSYS_CR1_I2C_BACKUP);
        CyExitCriticalSection(enableInterrupts);
        
        /* Re-enable Master */
        EZI2C_CFG_REG = EZI2C_backup.cfg;
    }
    else /* The I2C_REG_BACKUP was cleaned by PM API: it means Hibernate or wake-up not set */
    {
        #if(EZI2C_WAKEUP_ENABLED)
           /* Disable power to I2C block before register restore */
            enableInterrupts = CyEnterCriticalSection();
            EZI2C_PM_ACT_CFG_REG  &= ((uint8) ~EZI2C_ACT_PWR_EN);
            EZI2C_PM_STBY_CFG_REG &= ((uint8) ~EZI2C_STBY_PWR_EN);
            CyExitCriticalSection(enableInterrupts);

            /* Enable component after restore complete */
            EZI2C_backup.enableState = EZI2C_ENABLED;
        #endif /* (EZI2C_WAKEUP_ENABLED) */

        /* Restore component registers: Hibernate disable power */
        EZI2C_CFG_REG = EZI2C_backup.cfg;    
        EZI2C_XCFG_REG = EZI2C_backup.xcfg;
        EZI2C_ADDR_REG = EZI2C_backup.adr;
        EZI2C_CLKDIV1_REG = EZI2C_backup.clkDiv1;
        EZI2C_CLKDIV2_REG = EZI2C_backup.clkDiv2;
    }

    #if(EZI2C_WAKEUP_ENABLED)
        /* Trigger I2C interrupt if wakeup interrupt was triggered before */
        EZI2C_DisableInt();
        (void) CyIntSetVector(EZI2C_ISR_NUMBER, &EZI2C_ISR);
        if(0u != EZI2C_wakeupSource)
        {
            (void) CyIntSetPending(EZI2C_ISR_NUMBER); /* Generate interrupt to release I2C bus */
        }
        EZI2C_EnableInt();
    #endif /* (EZI2C_WAKEUP_ENABLED) */
}


/*******************************************************************************
* Function Name: EZI2C_Sleep
********************************************************************************
*
* Summary:
*  Saves component enable state and configuration. Stops component operation.
*  Should be called just prior to entering sleep. If "Enable wakeup from the
*  Sleep mode" is properly configured and enabled, this function should not be
*  called.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  EZI2C_backup - the non retention registers are saved to. Changed
*  by EZI2C_SaveConfig() function call.
*
* Reentrant:
*  No
*
*******************************************************************************/
void EZI2C_Sleep(void) 
{
    
    #if(EZI2C_WAKEUP_ENABLED)
        /* The I2C block should be always enabled if used as wakeup source */
        EZI2C_backup.enableState = EZI2C_DISABLED;
    #else
        
        EZI2C_backup.enableState = EZI2C_IS_BIT_SET(EZI2C_PM_ACT_CFG_REG, EZI2C_ACT_PWR_EN);

        if(EZI2C_IS_BIT_SET(EZI2C_PM_ACT_CFG_REG, EZI2C_ACT_PWR_EN))
        {
            
            EZI2C_Stop();
        }
    #endif /* (EZI2C_WAKEUP_ENABLED) */

    /* Save registers configuration */
    EZI2C_SaveConfig();
}


/*******************************************************************************
* Function Name: EZI2C_Wakeup
********************************************************************************
*
* Summary:
*  Restores component enable state and configuration. Should be called
*  just after awaking from sleep.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  EZI2C_backup - the non retention registers are restored from.
*
* Reentrant:
*  No
*
* Side Effects:
*  Calling this function before EZI2C_SaveConfig() or
*  EZI2C_Sleep() will lead to unpredictable results.
*
*******************************************************************************/
void EZI2C_Wakeup(void) 
{
    /* Restore registers values */
    EZI2C_RestoreConfig();
    
    if(EZI2C_DISABLED != EZI2C_backup.enableState)
    {
        /* Enable component's operation */
        EZI2C_Enable();

    } /* Do nothing if component's block was disabled before */
}


/***************************************
*       Obsolete
***************************************/

/* Following APIs are OBSOLETE and must not be used */

#if (EZI2C_ADDRESSES == EZI2C_ONE_ADDRESS)
    /*******************************************************************************
    * Function Name: EZI2C_SlaveSetSleepMode
    ********************************************************************************
    *
    * Summary:
    *  Disables the run time I2C regulator and enables the sleep Slave I2C.
    *  Should be called just prior to entering sleep. This function is only
    *  provided if a single I2C address is used.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  None
    *
    * Side Effects:
    *  The I2C interrupt will be disabled if Wake up from Sleep mode option is
    *  enabled.
    *
    *******************************************************************************/
    void EZI2C_SlaveSetSleepMode(void) 
    {
        #if(EZI2C_WAKEUP_ENABLED)
            uint8 interruptState;

            /* Enable the I2C backup regulator  */
            interruptState = CyEnterCriticalSection();
            EZI2C_PWRSYS_CR1_REG |= EZI2C_PWRSYS_CR1_I2C_BACKUP;
            CyExitCriticalSection(interruptState);

            /* Set force nack before putting the device to power off mode.
            *  It is cleared on wake up.
            */
            EZI2C_XCFG_REG |= EZI2C_XCFG_FORCE_NACK;
            while(0u == (EZI2C_XCFG_REG & EZI2C_XCFG_SLEEP_READY))
            {
                /* Waits for ongoing transaction to be completed. */
            }
            
             /* Setup wakeup interrupt */
            EZI2C_DisableInt();
            (void) CyIntSetVector(EZI2C_ISR_NUMBER, &EZI2C_WAKEUP_ISR);
            EZI2C_wakeupSource = 0u; /* Clear wakeup event */
            EZI2C_EnableInt();

            /* Leave interrupt enabled to wake up */
            
        #endif /* (EZI2C_WAKEUP_ENABLED) */
    }


    /*******************************************************************************
    * Function Name: EZI2C_SlaveSetWakeMode
    ********************************************************************************
    *
    * Summary:
    *  Disables the sleep EzI2C slave and re-enables the run time I2C.  Should be
    *  called just after awaking from sleep.  Must preserve address to continue.
    *  This function is only provided if a single I2C address is used.
    *
    * Parameters:
    *  None
    *
    * Return:
    *  None
    *
    * Side Effects:
    *  The I2C interrupt will be enabled if Wake up from Sleep mode option is
    *  enabled.
    *
    *******************************************************************************/
    void EZI2C_SlaveSetWakeMode(void) 
    {
        #if(EZI2C_WAKEUP_ENABLED)
            uint8 interruptState;

            interruptState = CyEnterCriticalSection();
            /* Disable the I2C regulator backup */
            EZI2C_PWRSYS_CR1_REG &= ((uint8) ~EZI2C_PWRSYS_CR1_I2C_BACKUP);
            CyExitCriticalSection(interruptState);

            /* Trigger I2C interrupt if wakeup interrupt was triggered before */
            EZI2C_DisableInt();
            (void) CyIntSetVector(EZI2C_ISR_NUMBER, &EZI2C_ISR);
            if(0u != EZI2C_wakeupSource)
            {
                (void) CyIntSetPending(EZI2C_ISR_NUMBER); /* Generate interrupt to release I2C bus */
            }
            EZI2C_EnableInt();

        #endif /* (EZI2C_WAKEUP_ENABLED) */
    }

#endif /* (EZI2C_ADDRESSES == EZI2C_ONE_ADDRESS) */

/* [] END OF FILE */
