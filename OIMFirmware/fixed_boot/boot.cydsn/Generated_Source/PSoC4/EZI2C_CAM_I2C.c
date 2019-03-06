/***************************************************************************//**
* \file EZI2C_CAM_I2C.c
* \version 3.20
*
* \brief
*  This file provides the source code to the API for the SCB Component in
*  I2C mode.
*
* Note:
*
*******************************************************************************
* \copyright
* Copyright 2013-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "EZI2C_CAM_PVT.h"
#include "EZI2C_CAM_I2C_PVT.h"


/***************************************
*      I2C Private Vars
***************************************/

volatile uint8 EZI2C_CAM_state;  /* Current state of I2C FSM */

#if(EZI2C_CAM_SCB_MODE_UNCONFIG_CONST_CFG)

    /***************************************
    *  Configuration Structure Initialization
    ***************************************/

    /* Constant configuration of I2C */
    const EZI2C_CAM_I2C_INIT_STRUCT EZI2C_CAM_configI2C =
    {
        EZI2C_CAM_I2C_MODE,
        EZI2C_CAM_I2C_OVS_FACTOR_LOW,
        EZI2C_CAM_I2C_OVS_FACTOR_HIGH,
        EZI2C_CAM_I2C_MEDIAN_FILTER_ENABLE,
        EZI2C_CAM_I2C_SLAVE_ADDRESS,
        EZI2C_CAM_I2C_SLAVE_ADDRESS_MASK,
        EZI2C_CAM_I2C_ACCEPT_ADDRESS,
        EZI2C_CAM_I2C_WAKE_ENABLE,
        EZI2C_CAM_I2C_BYTE_MODE_ENABLE,
        EZI2C_CAM_I2C_DATA_RATE,
        EZI2C_CAM_I2C_ACCEPT_GENERAL_CALL,
    };

    /*******************************************************************************
    * Function Name: EZI2C_CAM_I2CInit
    ****************************************************************************//**
    *
    *
    *  Configures the EZI2C_CAM for I2C operation.
    *
    *  This function is intended specifically to be used when the EZI2C_CAM 
    *  configuration is set to “Unconfigured EZI2C_CAM” in the customizer. 
    *  After initializing the EZI2C_CAM in I2C mode using this function, 
    *  the component can be enabled using the EZI2C_CAM_Start() or 
    * EZI2C_CAM_Enable() function.
    *  This function uses a pointer to a structure that provides the configuration 
    *  settings. This structure contains the same information that would otherwise 
    *  be provided by the customizer settings.
    *
    *  \param config: pointer to a structure that contains the following list of 
    *   fields. These fields match the selections available in the customizer. 
    *   Refer to the customizer for further description of the settings.
    *
    *******************************************************************************/
    void EZI2C_CAM_I2CInit(const EZI2C_CAM_I2C_INIT_STRUCT *config)
    {
        uint32 medianFilter;
        uint32 locEnableWake;

        if(NULL == config)
        {
            CYASSERT(0u != 0u); /* Halt execution due to bad function parameter */
        }
        else
        {
            /* Configure pins */
            EZI2C_CAM_SetPins(EZI2C_CAM_SCB_MODE_I2C, EZI2C_CAM_DUMMY_PARAM,
                                     EZI2C_CAM_DUMMY_PARAM);

            /* Store internal configuration */
            EZI2C_CAM_scbMode       = (uint8) EZI2C_CAM_SCB_MODE_I2C;
            EZI2C_CAM_scbEnableWake = (uint8) config->enableWake;
            EZI2C_CAM_scbEnableIntr = (uint8) EZI2C_CAM_SCB_IRQ_INTERNAL;

            EZI2C_CAM_mode          = (uint8) config->mode;
            EZI2C_CAM_acceptAddr    = (uint8) config->acceptAddr;

        #if (EZI2C_CAM_CY_SCBIP_V0)
            /* Adjust SDA filter settings. Ticket ID#150521 */
            EZI2C_CAM_SET_I2C_CFG_SDA_FILT_TRIM(EZI2C_CAM_EC_AM_I2C_CFG_SDA_FILT_TRIM);
        #endif /* (EZI2C_CAM_CY_SCBIP_V0) */

            /* Adjust AF and DF filter settings. Ticket ID#176179 */
            if (((EZI2C_CAM_I2C_MODE_SLAVE != config->mode) &&
                 (config->dataRate <= EZI2C_CAM_I2C_DATA_RATE_FS_MODE_MAX)) ||
                 (EZI2C_CAM_I2C_MODE_SLAVE == config->mode))
            {
                /* AF = 1, DF = 0 */
                EZI2C_CAM_I2C_CFG_ANALOG_FITER_ENABLE;
                medianFilter = EZI2C_CAM_DIGITAL_FILTER_DISABLE;
            }
            else
            {
                /* AF = 0, DF = 1 */
                EZI2C_CAM_I2C_CFG_ANALOG_FITER_DISABLE;
                medianFilter = EZI2C_CAM_DIGITAL_FILTER_ENABLE;
            }

        #if (!EZI2C_CAM_CY_SCBIP_V0)
            locEnableWake = (EZI2C_CAM_I2C_MULTI_MASTER_SLAVE) ? (0u) : (config->enableWake);
        #else
            locEnableWake = config->enableWake;
        #endif /* (!EZI2C_CAM_CY_SCBIP_V0) */

            /* Configure I2C interface */
            EZI2C_CAM_CTRL_REG     = EZI2C_CAM_GET_CTRL_BYTE_MODE  (config->enableByteMode) |
                                            EZI2C_CAM_GET_CTRL_ADDR_ACCEPT(config->acceptAddr)     |
                                            EZI2C_CAM_GET_CTRL_EC_AM_MODE (locEnableWake);

            EZI2C_CAM_I2C_CTRL_REG = EZI2C_CAM_GET_I2C_CTRL_HIGH_PHASE_OVS(config->oversampleHigh) |
                    EZI2C_CAM_GET_I2C_CTRL_LOW_PHASE_OVS (config->oversampleLow)                          |
                    EZI2C_CAM_GET_I2C_CTRL_S_GENERAL_IGNORE((uint32)(0u == config->acceptGeneralAddr))    |
                    EZI2C_CAM_GET_I2C_CTRL_SL_MSTR_MODE  (config->mode);

            /* Configure RX direction */
            EZI2C_CAM_RX_CTRL_REG      = EZI2C_CAM_GET_RX_CTRL_MEDIAN(medianFilter) |
                                                EZI2C_CAM_I2C_RX_CTRL;
            EZI2C_CAM_RX_FIFO_CTRL_REG = EZI2C_CAM_CLEAR_REG;

            /* Set default address and mask */
            EZI2C_CAM_RX_MATCH_REG    = ((EZI2C_CAM_I2C_SLAVE) ?
                                                (EZI2C_CAM_GET_I2C_8BIT_ADDRESS(config->slaveAddr) |
                                                 EZI2C_CAM_GET_RX_MATCH_MASK(config->slaveAddrMask)) :
                                                (EZI2C_CAM_CLEAR_REG));


            /* Configure TX direction */
            EZI2C_CAM_TX_CTRL_REG      = EZI2C_CAM_I2C_TX_CTRL;
            EZI2C_CAM_TX_FIFO_CTRL_REG = EZI2C_CAM_CLEAR_REG;

            /* Configure interrupt with I2C handler but do not enable it */
            CyIntDisable    (EZI2C_CAM_ISR_NUMBER);
            CyIntSetPriority(EZI2C_CAM_ISR_NUMBER, EZI2C_CAM_ISR_PRIORITY);
            (void) CyIntSetVector(EZI2C_CAM_ISR_NUMBER, &EZI2C_CAM_I2C_ISR);

            /* Configure interrupt sources */
        #if(!EZI2C_CAM_CY_SCBIP_V1)
            EZI2C_CAM_INTR_SPI_EC_MASK_REG = EZI2C_CAM_NO_INTR_SOURCES;
        #endif /* (!EZI2C_CAM_CY_SCBIP_V1) */

            EZI2C_CAM_INTR_I2C_EC_MASK_REG = EZI2C_CAM_NO_INTR_SOURCES;
            EZI2C_CAM_INTR_RX_MASK_REG     = EZI2C_CAM_NO_INTR_SOURCES;
            EZI2C_CAM_INTR_TX_MASK_REG     = EZI2C_CAM_NO_INTR_SOURCES;

            EZI2C_CAM_INTR_SLAVE_MASK_REG  = ((EZI2C_CAM_I2C_SLAVE) ?
                            (EZI2C_CAM_GET_INTR_SLAVE_I2C_GENERAL(config->acceptGeneralAddr) |
                             EZI2C_CAM_I2C_INTR_SLAVE_MASK) : (EZI2C_CAM_CLEAR_REG));

            EZI2C_CAM_INTR_MASTER_MASK_REG = ((EZI2C_CAM_I2C_MASTER) ?
                                                     (EZI2C_CAM_I2C_INTR_MASTER_MASK) :
                                                     (EZI2C_CAM_CLEAR_REG));

            /* Configure global variables */
            EZI2C_CAM_state = EZI2C_CAM_I2C_FSM_IDLE;

            /* Internal slave variables */
            EZI2C_CAM_slStatus        = 0u;
            EZI2C_CAM_slRdBufIndex    = 0u;
            EZI2C_CAM_slWrBufIndex    = 0u;
            EZI2C_CAM_slOverFlowCount = 0u;

            /* Internal master variables */
            EZI2C_CAM_mstrStatus     = 0u;
            EZI2C_CAM_mstrRdBufIndex = 0u;
            EZI2C_CAM_mstrWrBufIndex = 0u;
        }
    }

#else

    /*******************************************************************************
    * Function Name: EZI2C_CAM_I2CInit
    ****************************************************************************//**
    *
    *  Configures the SCB for the I2C operation.
    *
    *******************************************************************************/
    void EZI2C_CAM_I2CInit(void)
    {
    #if(EZI2C_CAM_CY_SCBIP_V0)
        /* Adjust SDA filter settings. Ticket ID#150521 */
        EZI2C_CAM_SET_I2C_CFG_SDA_FILT_TRIM(EZI2C_CAM_EC_AM_I2C_CFG_SDA_FILT_TRIM);
    #endif /* (EZI2C_CAM_CY_SCBIP_V0) */

        /* Adjust AF and DF filter settings. Ticket ID#176179 */
        EZI2C_CAM_I2C_CFG_ANALOG_FITER_ENABLE_ADJ;

        /* Configure I2C interface */
        EZI2C_CAM_CTRL_REG     = EZI2C_CAM_I2C_DEFAULT_CTRL;
        EZI2C_CAM_I2C_CTRL_REG = EZI2C_CAM_I2C_DEFAULT_I2C_CTRL;

        /* Configure RX direction */
        EZI2C_CAM_RX_CTRL_REG      = EZI2C_CAM_I2C_DEFAULT_RX_CTRL;
        EZI2C_CAM_RX_FIFO_CTRL_REG = EZI2C_CAM_I2C_DEFAULT_RX_FIFO_CTRL;

        /* Set default address and mask */
        EZI2C_CAM_RX_MATCH_REG     = EZI2C_CAM_I2C_DEFAULT_RX_MATCH;

        /* Configure TX direction */
        EZI2C_CAM_TX_CTRL_REG      = EZI2C_CAM_I2C_DEFAULT_TX_CTRL;
        EZI2C_CAM_TX_FIFO_CTRL_REG = EZI2C_CAM_I2C_DEFAULT_TX_FIFO_CTRL;

        /* Configure interrupt with I2C handler but do not enable it */
        CyIntDisable    (EZI2C_CAM_ISR_NUMBER);
        CyIntSetPriority(EZI2C_CAM_ISR_NUMBER, EZI2C_CAM_ISR_PRIORITY);
    #if(!EZI2C_CAM_I2C_EXTERN_INTR_HANDLER)
        (void) CyIntSetVector(EZI2C_CAM_ISR_NUMBER, &EZI2C_CAM_I2C_ISR);
    #endif /* (EZI2C_CAM_I2C_EXTERN_INTR_HANDLER) */

        /* Configure interrupt sources */
    #if(!EZI2C_CAM_CY_SCBIP_V1)
        EZI2C_CAM_INTR_SPI_EC_MASK_REG = EZI2C_CAM_I2C_DEFAULT_INTR_SPI_EC_MASK;
    #endif /* (!EZI2C_CAM_CY_SCBIP_V1) */

        EZI2C_CAM_INTR_I2C_EC_MASK_REG = EZI2C_CAM_I2C_DEFAULT_INTR_I2C_EC_MASK;
        EZI2C_CAM_INTR_SLAVE_MASK_REG  = EZI2C_CAM_I2C_DEFAULT_INTR_SLAVE_MASK;
        EZI2C_CAM_INTR_MASTER_MASK_REG = EZI2C_CAM_I2C_DEFAULT_INTR_MASTER_MASK;
        EZI2C_CAM_INTR_RX_MASK_REG     = EZI2C_CAM_I2C_DEFAULT_INTR_RX_MASK;
        EZI2C_CAM_INTR_TX_MASK_REG     = EZI2C_CAM_I2C_DEFAULT_INTR_TX_MASK;

        /* Configure global variables */
        EZI2C_CAM_state = EZI2C_CAM_I2C_FSM_IDLE;

    #if(EZI2C_CAM_I2C_SLAVE)
        /* Internal slave variable */
        EZI2C_CAM_slStatus        = 0u;
        EZI2C_CAM_slRdBufIndex    = 0u;
        EZI2C_CAM_slWrBufIndex    = 0u;
        EZI2C_CAM_slOverFlowCount = 0u;
    #endif /* (EZI2C_CAM_I2C_SLAVE) */

    #if(EZI2C_CAM_I2C_MASTER)
    /* Internal master variable */
        EZI2C_CAM_mstrStatus     = 0u;
        EZI2C_CAM_mstrRdBufIndex = 0u;
        EZI2C_CAM_mstrWrBufIndex = 0u;
    #endif /* (EZI2C_CAM_I2C_MASTER) */
    }
#endif /* (EZI2C_CAM_SCB_MODE_UNCONFIG_CONST_CFG) */


/*******************************************************************************
* Function Name: EZI2C_CAM_I2CStop
****************************************************************************//**
*
*  Resets the I2C FSM into the default state.
*
*******************************************************************************/
void EZI2C_CAM_I2CStop(void)
{
    EZI2C_CAM_state = EZI2C_CAM_I2C_FSM_IDLE;
}


#if(EZI2C_CAM_I2C_WAKE_ENABLE_CONST)
    /*******************************************************************************
    * Function Name: EZI2C_CAM_I2CSaveConfig
    ****************************************************************************//**
    *
    *  Enables EZI2C_CAM_INTR_I2C_EC_WAKE_UP interrupt source. This interrupt
    *  triggers on address match and wakes up device.
    *
    *******************************************************************************/
    void EZI2C_CAM_I2CSaveConfig(void)
    {
    #if (!EZI2C_CAM_CY_SCBIP_V0)
        #if (EZI2C_CAM_I2C_MULTI_MASTER_SLAVE_CONST && EZI2C_CAM_I2C_WAKE_ENABLE_CONST)
            /* Enable externally clocked address match if it was not enabled before.
            * This applicable only for Multi-Master-Slave. Ticket ID#192742 */
            if (0u == (EZI2C_CAM_CTRL_REG & EZI2C_CAM_CTRL_EC_AM_MODE))
            {
                /* Enable external address match logic */
                EZI2C_CAM_Stop();
                EZI2C_CAM_CTRL_REG |= EZI2C_CAM_CTRL_EC_AM_MODE;
                EZI2C_CAM_Enable();
            }
        #endif /* (EZI2C_CAM_I2C_MULTI_MASTER_SLAVE_CONST) */

        #if (EZI2C_CAM_SCB_CLK_INTERNAL)
            /* Disable clock to internal address match logic. Ticket ID#187931 */
            EZI2C_CAM_SCBCLK_Stop();
        #endif /* (EZI2C_CAM_SCB_CLK_INTERNAL) */
    #endif /* (!EZI2C_CAM_CY_SCBIP_V0) */

        EZI2C_CAM_SetI2CExtClkInterruptMode(EZI2C_CAM_INTR_I2C_EC_WAKE_UP);
    }


    /*******************************************************************************
    * Function Name: EZI2C_CAM_I2CRestoreConfig
    ****************************************************************************//**
    *
    *  Disables EZI2C_CAM_INTR_I2C_EC_WAKE_UP interrupt source. This interrupt
    *  triggers on address match and wakes up device.
    *
    *******************************************************************************/
    void EZI2C_CAM_I2CRestoreConfig(void)
    {
        /* Disable wakeup interrupt on address match */
        EZI2C_CAM_SetI2CExtClkInterruptMode(EZI2C_CAM_NO_INTR_SOURCES);

    #if (!EZI2C_CAM_CY_SCBIP_V0)
        #if (EZI2C_CAM_SCB_CLK_INTERNAL)
            /* Enable clock to internal address match logic. Ticket ID#187931 */
            EZI2C_CAM_SCBCLK_Start();
        #endif /* (EZI2C_CAM_SCB_CLK_INTERNAL) */
    #endif /* (!EZI2C_CAM_CY_SCBIP_V0) */
    }
#endif /* (EZI2C_CAM_I2C_WAKE_ENABLE_CONST) */


/* [] END OF FILE */
