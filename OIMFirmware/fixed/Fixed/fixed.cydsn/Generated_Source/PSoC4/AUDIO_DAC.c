/*******************************************************************************
* File Name: AUDIO_DAC.c
* Version 1.10
*
* Description:
*  This file provides the source code of APIs for the IDAC_P4 component.
*
*******************************************************************************
* Copyright 2013-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "AUDIO_DAC.h"

uint32 AUDIO_DAC_initVar = 0u;


/*******************************************************************************
* Function Name: AUDIO_DAC_Init
********************************************************************************
*
* Summary:
*  Initializes IDAC registers with initial values provided from customizer.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  None
*
*******************************************************************************/
void AUDIO_DAC_Init(void)
{
    uint32 regVal;
    uint8 enableInterrupts;

    /* Set initial configuration */
    enableInterrupts = CyEnterCriticalSection();
    
    #if(AUDIO_DAC_MODE_SOURCE == AUDIO_DAC_IDAC_POLARITY)
        regVal  = AUDIO_DAC_CSD_TRIM1_REG & ~(AUDIO_DAC_CSD_IDAC_TRIM1_MASK);
        regVal |=  (AUDIO_DAC_SFLASH_TRIM1_REG & AUDIO_DAC_CSD_IDAC_TRIM1_MASK);
        AUDIO_DAC_CSD_TRIM1_REG = regVal;
    #else
        regVal  = AUDIO_DAC_CSD_TRIM2_REG & ~(AUDIO_DAC_CSD_IDAC_TRIM2_MASK);
        regVal |=  (AUDIO_DAC_SFLASH_TRIM2_REG & AUDIO_DAC_CSD_IDAC_TRIM2_MASK);
        AUDIO_DAC_CSD_TRIM2_REG = regVal;
    #endif /* (AUDIO_DAC_MODE_SOURCE == AUDIO_DAC_IDAC_POLARITY) */

    /* clear previous values */
    AUDIO_DAC_IDAC_CONTROL_REG &= ((uint32)~((uint32)AUDIO_DAC_IDAC_VALUE_MASK <<
        AUDIO_DAC_IDAC_VALUE_POSITION)) | ((uint32)~((uint32)AUDIO_DAC_IDAC_MODE_MASK <<
        AUDIO_DAC_IDAC_MODE_POSITION))  | ((uint32)~((uint32)AUDIO_DAC_IDAC_RANGE_MASK  <<
        AUDIO_DAC_IDAC_RANGE_POSITION));

    AUDIO_DAC_IDAC_POLARITY_CONTROL_REG &= (~(uint32)((uint32)AUDIO_DAC_IDAC_POLARITY_MASK <<
        AUDIO_DAC_IDAC_POLARITY_POSITION));

    /* set new configuration */
    AUDIO_DAC_IDAC_CONTROL_REG |= (((uint32)AUDIO_DAC_IDAC_INIT_VALUE <<
        AUDIO_DAC_IDAC_VALUE_POSITION) | ((uint32)AUDIO_DAC_IDAC_RANGE <<
        AUDIO_DAC_IDAC_RANGE_POSITION));

    AUDIO_DAC_IDAC_POLARITY_CONTROL_REG |= ((uint32)AUDIO_DAC_IDAC_POLARITY <<
                                                           AUDIO_DAC_IDAC_POLARITY_POSITION);

    CyExitCriticalSection(enableInterrupts);

}


/*******************************************************************************
* Function Name: AUDIO_DAC_Enable
********************************************************************************
*
* Summary:
*  Enables IDAC operations.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  None
*
*******************************************************************************/
void AUDIO_DAC_Enable(void)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    /* Enable the IDAC */
    AUDIO_DAC_IDAC_CONTROL_REG |= ((uint32)AUDIO_DAC_IDAC_EN_MODE <<
                                                  AUDIO_DAC_IDAC_MODE_POSITION);
    AUDIO_DAC_IDAC_POLARITY_CONTROL_REG |= ((uint32)AUDIO_DAC_IDAC_CSD_EN <<
                                                           AUDIO_DAC_IDAC_CSD_EN_POSITION);
    CyExitCriticalSection(enableInterrupts);

}


/*******************************************************************************
* Function Name: AUDIO_DAC_Start
********************************************************************************
*
* Summary:
*  Starts the IDAC hardware.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  AUDIO_DAC_initVar
*
*******************************************************************************/
void AUDIO_DAC_Start(void)
{
    if(0u == AUDIO_DAC_initVar)
    {
        AUDIO_DAC_Init();
        AUDIO_DAC_initVar = 1u;
    }

    AUDIO_DAC_Enable();

}


/*******************************************************************************
* Function Name: AUDIO_DAC_Stop
********************************************************************************
*
* Summary:
*  Stops the IDAC hardware.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  None
*
*******************************************************************************/
void AUDIO_DAC_Stop(void)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    /* Disable the IDAC */
    AUDIO_DAC_IDAC_CONTROL_REG &= ((uint32)~((uint32)AUDIO_DAC_IDAC_MODE_MASK <<
        AUDIO_DAC_IDAC_MODE_POSITION));
    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: AUDIO_DAC_SetValue
********************************************************************************
*
* Summary:
*  Sets the IDAC value.
*
* Parameters:
*  uint32 value
*
* Return:
*  None
*
* Global variables:
*  None
*
*******************************************************************************/
void AUDIO_DAC_SetValue(uint32 value)
{
    uint8 enableInterrupts;
    uint32 newRegisterValue;

    enableInterrupts = CyEnterCriticalSection();

    #if(AUDIO_DAC_IDAC_VALUE_POSITION != 0u)
        newRegisterValue = ((AUDIO_DAC_IDAC_CONTROL_REG & (~(uint32)((uint32)AUDIO_DAC_IDAC_VALUE_MASK <<
        AUDIO_DAC_IDAC_VALUE_POSITION))) | (value << AUDIO_DAC_IDAC_VALUE_POSITION));
    #else
        newRegisterValue = ((AUDIO_DAC_IDAC_CONTROL_REG & (~(uint32)AUDIO_DAC_IDAC_VALUE_MASK)) | value);
    #endif /* AUDIO_DAC_IDAC_VALUE_POSITION != 0u */

    AUDIO_DAC_IDAC_CONTROL_REG = newRegisterValue;

    CyExitCriticalSection(enableInterrupts);
}

/* [] END OF FILE */
