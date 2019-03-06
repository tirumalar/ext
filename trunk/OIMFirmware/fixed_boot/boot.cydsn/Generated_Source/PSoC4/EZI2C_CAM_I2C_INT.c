/***************************************************************************//**
* \file EZI2C_CAM_I2C_INT.c
* \version 3.20
*
* \brief
*  This file provides the source code to the Interrupt Service Routine for
*  the SCB Component in I2C mode.
*
* Note:
*
********************************************************************************
* \copyright
* Copyright 2013-2016, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#include "EZI2C_CAM_PVT.h"
#include "EZI2C_CAM_I2C_PVT.h"
#include "cyapicallbacks.h"


/*******************************************************************************
* Function Name: EZI2C_CAM_I2C_ISR
****************************************************************************//**
*
*  Handles the Interrupt Service Routine for the SCB I2C mode.
*
*******************************************************************************/
CY_ISR(EZI2C_CAM_I2C_ISR)
{
    uint32 diffCount;
    uint32 endTransfer;

#ifdef EZI2C_CAM_I2C_ISR_ENTRY_CALLBACK
    EZI2C_CAM_I2C_ISR_EntryCallback();
#endif /* EZI2C_CAM_I2C_ISR_ENTRY_CALLBACK */

#if (EZI2C_CAM_I2C_CUSTOM_ADDRESS_HANDLER_CONST)
    uint32 response;

    response = EZI2C_CAM_I2C_ACK_ADDR;
#endif /* (EZI2C_CAM_I2C_CUSTOM_ADDRESS_HANDLER_CONST) */

    endTransfer = 0u; /* Continue active transfer */

    /* Calls customer routine if registered */
    if(NULL != EZI2C_CAM_customIntrHandler)
    {
        EZI2C_CAM_customIntrHandler();
    }

    if(EZI2C_CAM_CHECK_INTR_I2C_EC_MASKED(EZI2C_CAM_INTR_I2C_EC_WAKE_UP))
    {
        /* Mask-off after wakeup */
        EZI2C_CAM_SetI2CExtClkInterruptMode(EZI2C_CAM_NO_INTR_SOURCES);
    }

    /* Master and Slave error tracking:
    * Add the master state check to track only the master errors when the master is active or
    * track slave errors when the slave is active or idle.
    * A special MMS case: in the address phase with misplaced Start: the master sets the LOST_ARB and
    * slave BUS_ERR. The valid event is LOST_ARB comes from the master.
    */
    if(EZI2C_CAM_CHECK_I2C_FSM_MASTER)
    {
        #if(EZI2C_CAM_I2C_MASTER)
        {
            /* INTR_MASTER_I2C_BUS_ERROR:
            * A misplaced Start or Stop condition occurred on the bus: complete the transaction.
            * The interrupt is cleared in I2C_FSM_EXIT_IDLE.
            */
            if(EZI2C_CAM_CHECK_INTR_MASTER_MASKED(EZI2C_CAM_INTR_MASTER_I2C_BUS_ERROR))
            {
                EZI2C_CAM_mstrStatus |= (uint16) (EZI2C_CAM_I2C_MSTAT_ERR_XFER |
                                                         EZI2C_CAM_I2C_MSTAT_ERR_BUS_ERROR);

                endTransfer = EZI2C_CAM_I2C_CMPLT_ANY_TRANSFER;
            }

            /* INTR_MASTER_I2C_ARB_LOST:
            * The MultiMaster lost arbitrage during transaction.
            * A Misplaced Start or Stop condition is treated as lost arbitration when the master drives the SDA.
            * The interrupt source is cleared in I2C_FSM_EXIT_IDLE.
            */
            if(EZI2C_CAM_CHECK_INTR_MASTER_MASKED(EZI2C_CAM_INTR_MASTER_I2C_ARB_LOST))
            {
                EZI2C_CAM_mstrStatus |= (uint16) (EZI2C_CAM_I2C_MSTAT_ERR_XFER |
                                                         EZI2C_CAM_I2C_MSTAT_ERR_ARB_LOST);

                endTransfer = EZI2C_CAM_I2C_CMPLT_ANY_TRANSFER;
            }

            #if(EZI2C_CAM_I2C_MULTI_MASTER_SLAVE)
            {
                /* I2C_MASTER_CMD_M_START_ON_IDLE:
                * MultiMaster-Slave does not generate start, because Slave was addressed.
                * Pass control to slave.
                */
                if(EZI2C_CAM_CHECK_I2C_MASTER_CMD(EZI2C_CAM_I2C_MASTER_CMD_M_START_ON_IDLE))
                {
                    EZI2C_CAM_mstrStatus |= (uint16) (EZI2C_CAM_I2C_MSTAT_ERR_XFER |
                                                             EZI2C_CAM_I2C_MSTAT_ERR_ABORT_XFER);

                    endTransfer = EZI2C_CAM_I2C_CMPLT_ANY_TRANSFER;
                }
            }
            #endif

            /* The error handling common part:
            * Sets a completion flag of the master transaction and passes control to:
            *  - I2C_FSM_EXIT_IDLE - to complete transaction in case of: ARB_LOST or BUS_ERR.
            *  - I2C_FSM_IDLE      - to take chance for the slave to process incoming transaction.
            */
            if(0u != endTransfer)
            {
                /* Set completion flags for master */
                EZI2C_CAM_mstrStatus |= (uint16) EZI2C_CAM_GET_I2C_MSTAT_CMPLT;

                #if(EZI2C_CAM_I2C_MULTI_MASTER_SLAVE)
                {
                    if(EZI2C_CAM_CHECK_I2C_FSM_ADDR)
                    {
                        /* Start generation is set after another master starts accessing Slave.
                        * Clean-up master and turn to slave. Set state to IDLE.
                        */
                        if(EZI2C_CAM_CHECK_I2C_MASTER_CMD(EZI2C_CAM_I2C_MASTER_CMD_M_START_ON_IDLE))
                        {
                            EZI2C_CAM_I2C_MASTER_CLEAR_START;

                            endTransfer = EZI2C_CAM_I2C_CMPLT_ANY_TRANSFER; /* Pass control to Slave */
                        }
                        /* Valid arbitration lost on the address phase happens only when: master LOST_ARB is set and
                        * slave BUS_ERR is cleared. Only in that case set the state to IDLE without SCB IP re-enable.
                        */
                        else if((!EZI2C_CAM_CHECK_INTR_SLAVE_MASKED(EZI2C_CAM_INTR_SLAVE_I2C_BUS_ERROR))
                               && EZI2C_CAM_CHECK_INTR_MASTER_MASKED(EZI2C_CAM_INTR_MASTER_I2C_ARB_LOST))
                        {
                            endTransfer = EZI2C_CAM_I2C_CMPLT_ANY_TRANSFER; /* Pass control to Slave */
                        }
                        else
                        {
                            endTransfer = 0u; /* Causes I2C_FSM_EXIT_IDLE to be set below */
                        }

                        if(0u != endTransfer) /* Clean-up master to proceed with slave */
                        {
                            EZI2C_CAM_CLEAR_TX_FIFO; /* Shifter keeps address, clear it */

                            EZI2C_CAM_DISABLE_MASTER_AUTO_DATA_ACK; /* In case of reading disable autoACK */

                            /* Clean-up master interrupt sources */
                            EZI2C_CAM_ClearMasterInterruptSource(EZI2C_CAM_INTR_MASTER_ALL);

                            /* Disable data processing interrupts: they have to be cleared before */
                            EZI2C_CAM_SetRxInterruptMode(EZI2C_CAM_NO_INTR_SOURCES);
                            EZI2C_CAM_SetTxInterruptMode(EZI2C_CAM_NO_INTR_SOURCES);

                            EZI2C_CAM_state = EZI2C_CAM_I2C_FSM_IDLE;
                        }
                        else
                        {
                            /* Set I2C_FSM_EXIT_IDLE for BUS_ERR and ARB_LOST (that is really bus error) */
                            EZI2C_CAM_state = EZI2C_CAM_I2C_FSM_EXIT_IDLE;
                        }
                    }
                    else
                    {
                        /* Set I2C_FSM_EXIT_IDLE if any other state than address */
                        EZI2C_CAM_state = EZI2C_CAM_I2C_FSM_EXIT_IDLE;
                    }
                }
                #else
                {
                    /* In case of LOST*/
                    EZI2C_CAM_state = EZI2C_CAM_I2C_FSM_EXIT_IDLE;
                }
                #endif
            }
        }
        #endif
    }
    else /* (EZI2C_CAM_CHECK_I2C_FSM_SLAVE) */
    {
        #if(EZI2C_CAM_I2C_SLAVE)
        {
            /* INTR_SLAVE_I2C_BUS_ERROR or EZI2C_CAM_INTR_SLAVE_I2C_ARB_LOST:
            * A Misplaced Start or Stop condition occurred on the bus: set a flag
            * to notify an error condition.
            */
            if(EZI2C_CAM_CHECK_INTR_SLAVE_MASKED(EZI2C_CAM_INTR_SLAVE_I2C_BUS_ERROR |
                                                        EZI2C_CAM_INTR_SLAVE_I2C_ARB_LOST))
            {
                if(EZI2C_CAM_CHECK_I2C_FSM_RD)
                {
                    /* TX direction: master reads from slave */
                    EZI2C_CAM_slStatus &= (uint8) ~EZI2C_CAM_I2C_SSTAT_RD_BUSY;
                    EZI2C_CAM_slStatus |= (uint8) (EZI2C_CAM_I2C_SSTAT_RD_ERR |
                                                          EZI2C_CAM_I2C_SSTAT_RD_CMPLT);
                }
                else
                {
                    /* RX direction: master writes into slave */
                    EZI2C_CAM_slStatus &= (uint8) ~EZI2C_CAM_I2C_SSTAT_WR_BUSY;
                    EZI2C_CAM_slStatus |= (uint8) (EZI2C_CAM_I2C_SSTAT_WR_ERR |
                                                          EZI2C_CAM_I2C_SSTAT_WR_CMPLT);
                }

                EZI2C_CAM_state = EZI2C_CAM_I2C_FSM_EXIT_IDLE;
            }
        }
        #endif
    }

    /* States description:
    * Any Master operation starts from: the ADDR_RD/WR state as the master generates traffic on the bus.
    * Any Slave operation starts from: the IDLE state as the slave always waits for actions from the master.
    */

    /* FSM Master */
    if(EZI2C_CAM_CHECK_I2C_FSM_MASTER)
    {
        #if(EZI2C_CAM_I2C_MASTER)
        {
            /* INTR_MASTER_I2C_STOP:
            * A Stop condition was generated by the master: the end of the transaction.
            * Set completion flags to notify the API.
            */
            if(EZI2C_CAM_CHECK_INTR_MASTER_MASKED(EZI2C_CAM_INTR_MASTER_I2C_STOP))
            {
                EZI2C_CAM_ClearMasterInterruptSource(EZI2C_CAM_INTR_MASTER_I2C_STOP);

                EZI2C_CAM_mstrStatus |= (uint16) EZI2C_CAM_GET_I2C_MSTAT_CMPLT;
                EZI2C_CAM_state       = EZI2C_CAM_I2C_FSM_IDLE;
            }
            else
            {
                if(EZI2C_CAM_CHECK_I2C_FSM_ADDR) /* Address stage */
                {
                    /* INTR_MASTER_I2C_NACK:
                    * The master sent an address but it was NACKed by the slave. Complete transaction.
                    */
                    if(EZI2C_CAM_CHECK_INTR_MASTER_MASKED(EZI2C_CAM_INTR_MASTER_I2C_NACK))
                    {
                        EZI2C_CAM_ClearMasterInterruptSource(EZI2C_CAM_INTR_MASTER_I2C_NACK);

                        EZI2C_CAM_mstrStatus |= (uint16) (EZI2C_CAM_I2C_MSTAT_ERR_XFER |
                                                                 EZI2C_CAM_I2C_MSTAT_ERR_ADDR_NAK);

                        endTransfer = EZI2C_CAM_I2C_CMPLT_ANY_TRANSFER;
                    }
                    /* INTR_TX_UNDERFLOW. The master sent an address:
                    *  - TX direction: the clock is stretched after the ACK phase, because the TX FIFO is
                    *    EMPTY. The TX EMPTY cleans all the TX interrupt sources.
                    *  - RX direction: the 1st byte is received, but there is no ACK permission,
                    *    the clock is stretched after 1 byte is received.
                    */
                    else
                    {
                        if(EZI2C_CAM_CHECK_I2C_FSM_RD) /* Reading */
                        {
                            EZI2C_CAM_state = EZI2C_CAM_I2C_FSM_MSTR_RD_DATA;
                        }
                        else /* Writing */
                        {
                            EZI2C_CAM_state = EZI2C_CAM_I2C_FSM_MSTR_WR_DATA;
                            if(0u != EZI2C_CAM_mstrWrBufSize)
                            {
                                /* Enable INTR.TX_EMPTY if there is data to transmit */
                                EZI2C_CAM_SetTxInterruptMode(EZI2C_CAM_INTR_TX_EMPTY);
                            }
                        }
                    }
                }

                if(EZI2C_CAM_CHECK_I2C_FSM_DATA) /* Data phase */
                {
                    if(EZI2C_CAM_CHECK_I2C_FSM_RD) /* Reading */
                    {
                        /* INTR_RX_FULL:
                        * RX direction: the master received 8 bytes.
                        * Get data from RX FIFO and decide whether to ACK or  NACK the following bytes.
                        */
                        if(EZI2C_CAM_CHECK_INTR_RX_MASKED(EZI2C_CAM_INTR_RX_FULL))
                        {
                            /* Calculate difference */
                            diffCount =  EZI2C_CAM_mstrRdBufSize -
                                        (EZI2C_CAM_mstrRdBufIndex + EZI2C_CAM_GET_RX_FIFO_ENTRIES);

                            /* Proceed transaction or end it when RX FIFO becomes FULL again */
                            if(diffCount > EZI2C_CAM_I2C_FIFO_SIZE)
                            {
                                diffCount = EZI2C_CAM_I2C_FIFO_SIZE;
                            }
                            else
                            {
                                if(0u == diffCount)
                                {
                                    EZI2C_CAM_DISABLE_MASTER_AUTO_DATA_ACK;

                                    diffCount   = EZI2C_CAM_I2C_FIFO_SIZE;
                                    endTransfer = EZI2C_CAM_I2C_CMPLT_ANY_TRANSFER;
                                }
                            }

                            for(; (0u != diffCount); diffCount--)
                            {
                                EZI2C_CAM_mstrRdBufPtr[EZI2C_CAM_mstrRdBufIndex] = (uint8)
                                                                                        EZI2C_CAM_RX_FIFO_RD_REG;
                                EZI2C_CAM_mstrRdBufIndex++;
                            }
                        }
                        /* INTR_RX_NOT_EMPTY:
                        * RX direction: the master received one data byte, ACK or NACK it.
                        * The last byte is stored and NACKed by the master. The NACK and Stop is
                        * generated by one command generate Stop.
                        */
                        else if(EZI2C_CAM_CHECK_INTR_RX_MASKED(EZI2C_CAM_INTR_RX_NOT_EMPTY))
                        {
                            /* Put data in component buffer */
                            EZI2C_CAM_mstrRdBufPtr[EZI2C_CAM_mstrRdBufIndex] = (uint8) EZI2C_CAM_RX_FIFO_RD_REG;
                            EZI2C_CAM_mstrRdBufIndex++;

                            if(EZI2C_CAM_mstrRdBufIndex < EZI2C_CAM_mstrRdBufSize)
                            {
                                EZI2C_CAM_I2C_MASTER_GENERATE_ACK;
                            }
                            else
                            {
                               endTransfer = EZI2C_CAM_I2C_CMPLT_ANY_TRANSFER;
                            }
                        }
                        else
                        {
                            /* Do nothing */
                        }

                        EZI2C_CAM_ClearRxInterruptSource(EZI2C_CAM_INTR_RX_ALL);
                    }
                    else /* Writing */
                    {
                        /* INTR_MASTER_I2C_NACK :
                        * The master writes data to the slave and NACK was received: not all the bytes were
                        * written to the slave from the TX FIFO. Revert the index if there is data in
                        * the TX FIFO and pass control to a complete transfer.
                        */
                        if(EZI2C_CAM_CHECK_INTR_MASTER_MASKED(EZI2C_CAM_INTR_MASTER_I2C_NACK))
                        {
                            EZI2C_CAM_ClearMasterInterruptSource(EZI2C_CAM_INTR_MASTER_I2C_NACK);

                            /* Rollback write buffer index: NACKed byte remains in shifter */
                            EZI2C_CAM_mstrWrBufIndexTmp -= (EZI2C_CAM_GET_TX_FIFO_ENTRIES +
                                                                   EZI2C_CAM_GET_TX_FIFO_SR_VALID);

                            /* Update number of transferred bytes */
                            EZI2C_CAM_mstrWrBufIndex = EZI2C_CAM_mstrWrBufIndexTmp;

                            EZI2C_CAM_mstrStatus |= (uint16) (EZI2C_CAM_I2C_MSTAT_ERR_XFER |
                                                                     EZI2C_CAM_I2C_MSTAT_ERR_SHORT_XFER);

                            EZI2C_CAM_CLEAR_TX_FIFO;

                            endTransfer = EZI2C_CAM_I2C_CMPLT_ANY_TRANSFER;
                        }
                        /* INTR_TX_EMPTY :
                        * TX direction: the TX FIFO is EMPTY, the data from the buffer needs to be put there.
                        * When there is no data in the component buffer, the underflow interrupt is
                        * enabled to catch when all the data has been transferred.
                        */
                        else if(EZI2C_CAM_CHECK_INTR_TX_MASKED(EZI2C_CAM_INTR_TX_EMPTY))
                        {
                            while(EZI2C_CAM_I2C_FIFO_SIZE != EZI2C_CAM_GET_TX_FIFO_ENTRIES)
                            {
                                /* The temporary mstrWrBufIndexTmp is used because slave could NACK the byte and index
                                * roll-back required in this case. The mstrWrBufIndex is updated at the end of transfer.
                                */
                                if(EZI2C_CAM_mstrWrBufIndexTmp < EZI2C_CAM_mstrWrBufSize)
                                {
                                #if(!EZI2C_CAM_CY_SCBIP_V0)
                                   /* Clear INTR_TX.UNDERFLOW before putting the last byte into TX FIFO. This ensures
                                    * a proper trigger at the end of transaction when INTR_TX.UNDERFLOW single trigger
                                    * event. Ticket ID# 156735.
                                    */
                                    if(EZI2C_CAM_mstrWrBufIndexTmp == (EZI2C_CAM_mstrWrBufSize - 1u))
                                    {
                                        EZI2C_CAM_ClearTxInterruptSource(EZI2C_CAM_INTR_TX_UNDERFLOW);
                                        EZI2C_CAM_SetTxInterruptMode(EZI2C_CAM_INTR_TX_UNDERFLOW);
                                    }
                                 #endif /* (!EZI2C_CAM_CY_SCBIP_V0) */

                                    /* Put data into TX FIFO */
                                    EZI2C_CAM_TX_FIFO_WR_REG = (uint32) EZI2C_CAM_mstrWrBufPtr[EZI2C_CAM_mstrWrBufIndexTmp];
                                    EZI2C_CAM_mstrWrBufIndexTmp++;
                                }
                                else
                                {
                                    break; /* No more data to put */
                                }
                            }

                        #if(EZI2C_CAM_CY_SCBIP_V0)
                            if(EZI2C_CAM_mstrWrBufIndexTmp == EZI2C_CAM_mstrWrBufSize)
                            {
                                EZI2C_CAM_SetTxInterruptMode(EZI2C_CAM_INTR_TX_UNDERFLOW);
                            }

                            EZI2C_CAM_ClearTxInterruptSource(EZI2C_CAM_INTR_TX_ALL);
                        #else
                            EZI2C_CAM_ClearTxInterruptSource(EZI2C_CAM_INTR_TX_EMPTY);
                        #endif /* (EZI2C_CAM_CY_SCBIP_V0) */
                        }
                        /* INTR_TX_UNDERFLOW:
                        * TX direction: all data from the TX FIFO was transferred to the slave.
                        * The transaction needs to be completed.
                        */
                        else if(EZI2C_CAM_CHECK_INTR_TX_MASKED(EZI2C_CAM_INTR_TX_UNDERFLOW))
                        {
                            /* Update number of transferred bytes */
                            EZI2C_CAM_mstrWrBufIndex = EZI2C_CAM_mstrWrBufIndexTmp;

                            endTransfer = EZI2C_CAM_I2C_CMPLT_ANY_TRANSFER;
                        }
                        else
                        {
                            /* Do nothing */
                        }
                    }
                }

                if(0u != endTransfer) /* Complete transfer */
                {
                    /* Clean-up master after reading: only in case of NACK */
                    EZI2C_CAM_DISABLE_MASTER_AUTO_DATA_ACK;

                    /* Disable data processing interrupts: they have to be cleared before */
                    EZI2C_CAM_SetRxInterruptMode(EZI2C_CAM_NO_INTR_SOURCES);
                    EZI2C_CAM_SetTxInterruptMode(EZI2C_CAM_NO_INTR_SOURCES);

                    if(EZI2C_CAM_CHECK_I2C_MODE_NO_STOP(EZI2C_CAM_mstrControl))
                    {
                        /* On-going transaction is suspended: the ReStart is generated by the API request */
                        EZI2C_CAM_mstrStatus |= (uint16) (EZI2C_CAM_I2C_MSTAT_XFER_HALT |
                                                                 EZI2C_CAM_GET_I2C_MSTAT_CMPLT);

                        EZI2C_CAM_state = EZI2C_CAM_I2C_FSM_MSTR_HALT;
                    }
                    else
                    {
                        /* Complete transaction: exclude the data processing state and generate Stop.
                        * The completion status will be set after Stop generation.
                        * A special case is read: because NACK and Stop are generated by the command below.
                        * Lost arbitration can occur during NACK generation when
                        * the other master is still reading from the slave.
                        */
                        EZI2C_CAM_I2C_MASTER_GENERATE_STOP;
                    }
                }
            }

        } /* (EZI2C_CAM_I2C_MASTER) */
        #endif

    } /* (EZI2C_CAM_CHECK_I2C_FSM_MASTER) */


    /* FSM Slave */
    else if(EZI2C_CAM_CHECK_I2C_FSM_SLAVE)
    {
        #if(EZI2C_CAM_I2C_SLAVE)
        {
            /* INTR_SLAVE_NACK:
            * The master completes reading the slave: the appropriate flags have to be set.
            * The TX FIFO is cleared after an overflow condition is set.
            */
            if(EZI2C_CAM_CHECK_INTR_SLAVE_MASKED(EZI2C_CAM_INTR_SLAVE_I2C_NACK))
            {
                EZI2C_CAM_ClearSlaveInterruptSource(EZI2C_CAM_INTR_SLAVE_I2C_NACK);

                /* All entries that remain in TX FIFO are: FIFO Size + 1 (SHIFTER) */
                diffCount = (EZI2C_CAM_GET_TX_FIFO_ENTRIES + EZI2C_CAM_GET_TX_FIFO_SR_VALID);

                if(EZI2C_CAM_slOverFlowCount > diffCount) /* Overflow */
                {
                    EZI2C_CAM_slStatus |= (uint8) EZI2C_CAM_I2C_SSTAT_RD_OVFL;
                }
                else /* No Overflow */
                {
                    /* Roll-back temporary index */
                    EZI2C_CAM_slRdBufIndexTmp -= (diffCount - EZI2C_CAM_slOverFlowCount);
                }

                /* Update slave of transferred bytes */
                EZI2C_CAM_slRdBufIndex = EZI2C_CAM_slRdBufIndexTmp;

                /* Clean-up TX FIFO */
                EZI2C_CAM_SetTxInterruptMode(EZI2C_CAM_NO_INTR_SOURCES);
                EZI2C_CAM_slOverFlowCount = 0u;
                EZI2C_CAM_CLEAR_TX_FIFO;

                /* Complete master reading */
                EZI2C_CAM_slStatus &= (uint8) ~EZI2C_CAM_I2C_SSTAT_RD_BUSY;
                EZI2C_CAM_slStatus |= (uint8)  EZI2C_CAM_I2C_SSTAT_RD_CMPLT;
                EZI2C_CAM_state     =  EZI2C_CAM_I2C_FSM_IDLE;
            }


            /* INTR_SLAVE_I2C_WRITE_STOP:
            * The master completes writing to the slave: the appropriate flags have to be set.
            * The RX FIFO contains 1-8 bytes from the previous transaction which needs to be read.
            * There is a possibility that RX FIFO contains an address, it needs to leave it there.
            */
            if(EZI2C_CAM_CHECK_INTR_SLAVE_MASKED(EZI2C_CAM_INTR_SLAVE_I2C_WRITE_STOP))
            {
                EZI2C_CAM_ClearSlaveInterruptSource(EZI2C_CAM_INTR_SLAVE_I2C_WRITE_STOP);

                /* Read bytes from RX FIFO when auto data ACK receive logic is enabled. Otherwise all data bytes
                * were already read from the RX FIFO except for address byte which has to stay here to be handled by
                * I2C_ADDR_MATCH.
                */
                if (0u != (EZI2C_CAM_I2C_CTRL_REG & EZI2C_CAM_I2C_CTRL_S_READY_DATA_ACK))
                {
                    while(0u != EZI2C_CAM_GET_RX_FIFO_ENTRIES)
                    {
                        #if(EZI2C_CAM_CHECK_I2C_ACCEPT_ADDRESS)
                        {
                            if((1u == EZI2C_CAM_GET_RX_FIFO_ENTRIES) &&
                               (EZI2C_CAM_CHECK_INTR_SLAVE_MASKED(EZI2C_CAM_INTR_SLAVE_I2C_ADDR_MATCH)))
                            {
                                break; /* Leave address in RX FIFO */
                            }
                        }
                        #endif

                        /* Put data in component buffer */
                        EZI2C_CAM_slWrBufPtr[EZI2C_CAM_slWrBufIndex] = (uint8) EZI2C_CAM_RX_FIFO_RD_REG;
                        EZI2C_CAM_slWrBufIndex++;
                    }

                    EZI2C_CAM_DISABLE_SLAVE_AUTO_DATA;
                }

                if(EZI2C_CAM_CHECK_INTR_RX(EZI2C_CAM_INTR_RX_OVERFLOW))
                {
                    EZI2C_CAM_slStatus |= (uint8) EZI2C_CAM_I2C_SSTAT_WR_OVFL;
                }

                /* Clears RX interrupt sources triggered on data receiving */
                EZI2C_CAM_SetRxInterruptMode(EZI2C_CAM_NO_INTR_SOURCES);
                EZI2C_CAM_ClearRxInterruptSource(EZI2C_CAM_INTR_RX_ALL);

                /* Complete master writing */
                EZI2C_CAM_slStatus &= (uint8) ~EZI2C_CAM_I2C_SSTAT_WR_BUSY;
                EZI2C_CAM_slStatus |= (uint8)  EZI2C_CAM_I2C_SSTAT_WR_CMPLT;
                EZI2C_CAM_state     =  EZI2C_CAM_I2C_FSM_IDLE;
            }


            /* INTR_SLAVE_I2C_ADDR_MATCH or INTR_SLAVE_I2C_GENERAL:
            * The address match or general call address event starts the slave operation:
            * after leaving the TX or RX direction has to be chosen.
            * The wakeup interrupt must be cleared only after an address match is set.
            */
        #if (EZI2C_CAM_I2C_CUSTOM_ADDRESS_HANDLER_CONST)
            if (EZI2C_CAM_CHECK_INTR_SLAVE_MASKED(EZI2C_CAM_INTR_SLAVE_I2C_ADDR_MATCH |
                                                         EZI2C_CAM_INTR_SLAVE_I2C_GENERAL))
        #else
            if (EZI2C_CAM_CHECK_INTR_SLAVE_MASKED(EZI2C_CAM_INTR_SLAVE_I2C_ADDR_MATCH))
        #endif /* (EZI2C_CAM_I2C_CUSTOM_ADDRESS_HANDLER_CONST) */
            {
                /* Clear externally clocked address match interrupt source when internally clocked is set */
                EZI2C_CAM_ClearI2CExtClkInterruptSource(EZI2C_CAM_INTR_I2C_EC_WAKE_UP);

                #if (EZI2C_CAM_I2C_CUSTOM_ADDRESS_HANDLER)
                {
                    if (NULL != EZI2C_CAM_customAddressHandler)
                    {
                        /* Call custom address handler */
                        response = EZI2C_CAM_customAddressHandler();
                    }
                    else
                    {
                        /* Read address from the RX FIFO. If there is no address underflow triggers but
                        * component does not use that source. */
                        (void) EZI2C_CAM_RX_FIFO_RD_REG;
                        response = EZI2C_CAM_I2C_ACK_ADDR;
                    }

                    /* Clears RX sources after address was received in the RX FIFO */
                    EZI2C_CAM_ClearRxInterruptSource(EZI2C_CAM_INTR_RX_ALL);
                }
                #endif

            #if (EZI2C_CAM_I2C_CUSTOM_ADDRESS_HANDLER_CONST)
                if (response == EZI2C_CAM_I2C_NAK_ADDR)
                {
                #if (!EZI2C_CAM_CY_SCBIP_V0)
                    /* Disable write stop interrupt source as it triggers after address was NACKed. Ticket ID#156094 */
                    EZI2C_CAM_DISABLE_INTR_SLAVE(EZI2C_CAM_INTR_SLAVE_I2C_WRITE_STOP);
                #endif /* (!EZI2C_CAM_CY_SCBIP_V0) */

                    /* Clear address match and stop history */
                    EZI2C_CAM_ClearSlaveInterruptSource(EZI2C_CAM_INTR_SLAVE_ALL);

                    /* ACK the address byte */
                    EZI2C_CAM_I2C_SLAVE_GENERATE_NACK;
                }
                else
            #endif /* (EZI2C_CAM_I2C_CUSTOM_ADDRESS_HANDLER_CONST) */
                {
                    if(EZI2C_CAM_CHECK_I2C_STATUS(EZI2C_CAM_I2C_STATUS_S_READ))
                    /* TX direction: master reads from slave */
                    {
                        EZI2C_CAM_SetTxInterruptMode(EZI2C_CAM_INTR_TX_EMPTY);

                        /* Set temporary index to address buffer clear from API */
                        EZI2C_CAM_slRdBufIndexTmp = EZI2C_CAM_slRdBufIndex;

                        /* Start master reading */
                        EZI2C_CAM_slStatus |= (uint8) EZI2C_CAM_I2C_SSTAT_RD_BUSY;
                        EZI2C_CAM_state     = EZI2C_CAM_I2C_FSM_SL_RD;
                    }
                    else
                    /* RX direction: master writes into slave */
                    {
                        /* Calculate available buffer size */
                        diffCount = (EZI2C_CAM_slWrBufSize - EZI2C_CAM_slWrBufIndex);

                    #if (EZI2C_CAM_CY_SCBIP_V0)
                        if(diffCount < EZI2C_CAM_I2C_FIFO_SIZE)
                        /* Receive data: byte-by-byte */
                        {
                            EZI2C_CAM_SetRxInterruptMode(EZI2C_CAM_INTR_RX_NOT_EMPTY);
                        }
                        else
                        /* Receive data: into RX FIFO */
                        {
                            if(diffCount == EZI2C_CAM_I2C_FIFO_SIZE)
                            {
                                /* NACK when RX FIFO become FULL */
                                EZI2C_CAM_ENABLE_SLAVE_AUTO_DATA;
                            }
                            else
                            {
                                /* Stretch clock when RX FIFO becomes FULL */
                                EZI2C_CAM_ENABLE_SLAVE_AUTO_DATA_ACK;
                                EZI2C_CAM_SetRxInterruptMode(EZI2C_CAM_INTR_RX_FULL);
                            }
                        }

                    #else
                        #if(EZI2C_CAM_CHECK_I2C_ACCEPT_ADDRESS)
                        {
                            /* Enable RX.NOT_EMPTY interrupt source to receive byte by byte.
                            * The byte by byte receive is always chosen for the case when an address is accepted
                            * in RX FIFO. Ticket ID#175559.
                            */
                            EZI2C_CAM_SetRxInterruptMode(EZI2C_CAM_INTR_RX_NOT_EMPTY);
                        }
                        #else
                        {
                            if(diffCount < EZI2C_CAM_I2C_FIFO_SIZE)
                            /* Receive data: byte-by-byte */
                            {
                                EZI2C_CAM_SetRxInterruptMode(EZI2C_CAM_INTR_RX_NOT_EMPTY);
                            }
                            else
                            /* Receive data: into RX FIFO */
                            {
                                if(diffCount == EZI2C_CAM_I2C_FIFO_SIZE)
                                {
                                    /* NACK when RX FIFO become FULL */
                                    EZI2C_CAM_ENABLE_SLAVE_AUTO_DATA;
                                }
                                else
                                {
                                    /* Stretch clock when RX FIFO becomes FULL */
                                    EZI2C_CAM_ENABLE_SLAVE_AUTO_DATA_ACK;
                                    EZI2C_CAM_SetRxInterruptMode(EZI2C_CAM_INTR_RX_FULL);
                                }
                            }
                        }
                        #endif
                    #endif /* (EZI2C_CAM_CY_SCBIP_V0) */

                        /* Start master reading */
                        EZI2C_CAM_slStatus |= (uint8) EZI2C_CAM_I2C_SSTAT_WR_BUSY;
                        EZI2C_CAM_state     = EZI2C_CAM_I2C_FSM_SL_WR;
                    }

                    /* Clear address match and stop history */
                    EZI2C_CAM_ClearSlaveInterruptSource(EZI2C_CAM_INTR_SLAVE_ALL);

                #if (!EZI2C_CAM_CY_SCBIP_V0)
                    /* Enable write stop interrupt source as it triggers after address was NACKed. Ticket ID#156094 */
                    EZI2C_CAM_ENABLE_INTR_SLAVE(EZI2C_CAM_INTR_SLAVE_I2C_WRITE_STOP);
                #endif /* (!EZI2C_CAM_CY_SCBIP_V0) */

                    /* ACK the address byte */
                    EZI2C_CAM_I2C_SLAVE_GENERATE_ACK;
                }
            }

            /* EZI2C_CAM_INTR_RX_FULL:
            * Get data from the RX FIFO and decide whether to ACK or NACK the following bytes
            */
            if(EZI2C_CAM_CHECK_INTR_RX_MASKED(EZI2C_CAM_INTR_RX_FULL))
            {
                /* Calculate available buffer size to take into account that RX FIFO is FULL */
                diffCount =  EZI2C_CAM_slWrBufSize -
                            (EZI2C_CAM_slWrBufIndex + EZI2C_CAM_I2C_FIFO_SIZE);

                if(diffCount > EZI2C_CAM_I2C_FIFO_SIZE) /* Proceed transaction */
                {
                    diffCount   = EZI2C_CAM_I2C_FIFO_SIZE;
                    endTransfer = 0u;  /* Continue active transfer */
                }
                else /* End when FIFO becomes FULL again */
                {
                    endTransfer = EZI2C_CAM_I2C_CMPLT_ANY_TRANSFER;
                }

                for(; (0u != diffCount); diffCount--)
                {
                    /* Put data in component buffer */
                    EZI2C_CAM_slWrBufPtr[EZI2C_CAM_slWrBufIndex] = (uint8) EZI2C_CAM_RX_FIFO_RD_REG;
                    EZI2C_CAM_slWrBufIndex++;
                }

                if(0u != endTransfer) /* End transfer sending NACK */
                {
                    EZI2C_CAM_ENABLE_SLAVE_AUTO_DATA_NACK;

                    /* INTR_RX_FULL triggers earlier than INTR_SLAVE_I2C_STOP:
                    * disable all RX interrupt sources.
                    */
                    EZI2C_CAM_SetRxInterruptMode(EZI2C_CAM_NO_INTR_SOURCES);
                }

                EZI2C_CAM_ClearRxInterruptSource(EZI2C_CAM_INTR_RX_FULL);
            }
            /* EZI2C_CAM_INTR_RX_NOT_EMPTY:
            * The buffer size is less than 8: it requires processing in byte-by-byte mode.
            */
            else if(EZI2C_CAM_CHECK_INTR_RX_MASKED(EZI2C_CAM_INTR_RX_NOT_EMPTY))
            {
                diffCount = EZI2C_CAM_RX_FIFO_RD_REG;

                if(EZI2C_CAM_slWrBufIndex < EZI2C_CAM_slWrBufSize)
                {
                    EZI2C_CAM_I2C_SLAVE_GENERATE_ACK;

                    /* Put data into component buffer */
                    EZI2C_CAM_slWrBufPtr[EZI2C_CAM_slWrBufIndex] = (uint8) diffCount;
                    EZI2C_CAM_slWrBufIndex++;
                }
                else /* Overflow: there is no space in write buffer */
                {
                    EZI2C_CAM_I2C_SLAVE_GENERATE_NACK;

                    EZI2C_CAM_slStatus |= (uint8) EZI2C_CAM_I2C_SSTAT_WR_OVFL;
                }

                EZI2C_CAM_ClearRxInterruptSource(EZI2C_CAM_INTR_RX_NOT_EMPTY);
            }
            else
            {
                /* Does nothing */
            }


            /* EZI2C_CAM_INTR_TX_EMPTY:
            * The master reads the slave: provide data to read or 0xFF in the case of the end of the buffer
            * The overflow condition must be captured, but not set until the end of transaction.
            * There is a possibility of a false overflow due to TX FIFO utilization.
            */
            if(EZI2C_CAM_CHECK_INTR_TX_MASKED(EZI2C_CAM_INTR_TX_EMPTY))
            {
                while(EZI2C_CAM_I2C_FIFO_SIZE != EZI2C_CAM_GET_TX_FIFO_ENTRIES)
                {
                    /* Temporary slRdBufIndexTmp is used because the master can NACK the byte and
                    * index roll-back is required in this case. The slRdBufIndex is updated at the end
                    * of the read transfer.
                    */
                    if(EZI2C_CAM_slRdBufIndexTmp < EZI2C_CAM_slRdBufSize)
                    /* Data from buffer */
                    {
                        EZI2C_CAM_TX_FIFO_WR_REG = (uint32) EZI2C_CAM_slRdBufPtr[EZI2C_CAM_slRdBufIndexTmp];
                        EZI2C_CAM_slRdBufIndexTmp++;
                    }
                    else
                    /* Probably Overflow */
                    {
                        EZI2C_CAM_TX_FIFO_WR_REG = EZI2C_CAM_I2C_SLAVE_OVFL_RETURN;

                        if(EZI2C_CAM_slOverFlowCount <= EZI2C_CAM_I2C_TX_OVERFLOW_COUNT)
                        {
                            /* Get counter in range of overflow. */
                            EZI2C_CAM_slOverFlowCount++;
                        }
                    }
                }

                EZI2C_CAM_ClearTxInterruptSource(EZI2C_CAM_INTR_TX_EMPTY);
            }

        }  /* (EZI2C_CAM_I2C_SLAVE) */
        #endif
    }


    /* FSM EXIT:
    * Slave:  INTR_SLAVE_I2C_BUS_ERROR, INTR_SLAVE_I2C_ARB_LOST
    * Master: INTR_MASTER_I2C_BUS_ERROR, INTR_MASTER_I2C_ARB_LOST.
    */
    else
    {
        EZI2C_CAM_CTRL_REG &= (uint32) ~EZI2C_CAM_CTRL_ENABLED; /* Disable scb IP */

        EZI2C_CAM_state = EZI2C_CAM_I2C_FSM_IDLE;

        EZI2C_CAM_DISABLE_SLAVE_AUTO_DATA;
        EZI2C_CAM_DISABLE_MASTER_AUTO_DATA;

    #if(EZI2C_CAM_CY_SCBIP_V0)
        EZI2C_CAM_SetRxInterruptMode(EZI2C_CAM_NO_INTR_SOURCES);
        EZI2C_CAM_SetTxInterruptMode(EZI2C_CAM_NO_INTR_SOURCES);

        /* Clear interrupt sources as they are not automatically cleared after SCB is disabled */
        EZI2C_CAM_ClearTxInterruptSource(EZI2C_CAM_INTR_RX_ALL);
        EZI2C_CAM_ClearRxInterruptSource(EZI2C_CAM_INTR_TX_ALL);
        EZI2C_CAM_ClearSlaveInterruptSource(EZI2C_CAM_INTR_SLAVE_ALL);
        EZI2C_CAM_ClearMasterInterruptSource(EZI2C_CAM_INTR_MASTER_ALL);
    #endif /* (EZI2C_CAM_CY_SCBIP_V0) */

        EZI2C_CAM_CTRL_REG |= (uint32) EZI2C_CAM_CTRL_ENABLED;  /* Enable scb IP */
    }

#ifdef EZI2C_CAM_I2C_ISR_EXIT_CALLBACK
    EZI2C_CAM_I2C_ISR_ExitCallback();
#endif /* EZI2C_CAM_I2C_ISR_EXIT_CALLBACK */

}


/* [] END OF FILE */
