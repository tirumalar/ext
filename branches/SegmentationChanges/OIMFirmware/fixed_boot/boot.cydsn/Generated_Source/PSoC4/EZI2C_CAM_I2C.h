/***************************************************************************//**
* \file EZI2C_CAM_I2C.h
* \version 3.20
*
* \brief
*  This file provides constants and parameter values for the SCB Component in
*  the I2C mode.
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

#if !defined(CY_SCB_I2C_EZI2C_CAM_H)
#define CY_SCB_I2C_EZI2C_CAM_H

#include "EZI2C_CAM.h"


/***************************************
*   Initial Parameter Constants
****************************************/

#define EZI2C_CAM_I2C_MODE                   (1u)
#define EZI2C_CAM_I2C_OVS_FACTOR_LOW         (8u)
#define EZI2C_CAM_I2C_OVS_FACTOR_HIGH        (8u)
#define EZI2C_CAM_I2C_MEDIAN_FILTER_ENABLE   (1u)
#define EZI2C_CAM_I2C_SLAVE_ADDRESS          (34u)
#define EZI2C_CAM_I2C_SLAVE_ADDRESS_MASK     (254u)
#define EZI2C_CAM_I2C_ACCEPT_ADDRESS         (0u)
#define EZI2C_CAM_I2C_ACCEPT_GENERAL_CALL    (0u)
#define EZI2C_CAM_I2C_WAKE_ENABLE            (0u)
#define EZI2C_CAM_I2C_DATA_RATE              (1000u)
#define EZI2C_CAM_I2C_DATA_RATE_ACTUAL       (1000u)
#define EZI2C_CAM_I2C_CLOCK_FROM_TERM        (0u)
#define EZI2C_CAM_I2C_EXTERN_INTR_HANDLER    (0u)
#define EZI2C_CAM_I2C_BYTE_MODE_ENABLE       (0u)
#define EZI2C_CAM_I2C_MANUAL_OVS_CONTROL     (1u)


/***************************************
*  Conditional Compilation Parameters
****************************************/

/* I2C sub mode enum */
#define EZI2C_CAM_I2C_MODE_SLAVE              (0x01u)
#define EZI2C_CAM_I2C_MODE_MASTER             (0x02u)
#define EZI2C_CAM_I2C_MODE_MULTI_MASTER       (0x06u)
#define EZI2C_CAM_I2C_MODE_MULTI_MASTER_SLAVE (0x07u)
#define EZI2C_CAM_I2C_MODE_MULTI_MASTER_MASK  (0x04u)

#if (EZI2C_CAM_SCB_MODE_UNCONFIG_CONST_CFG)

    /* Returns true if slave mode is enabled */
    #define EZI2C_CAM_I2C_SLAVE  (0u != (EZI2C_CAM_I2C_MODE_SLAVE & EZI2C_CAM_mode))

    /* Returns true if master mode is enabled */
    #define EZI2C_CAM_I2C_MASTER (0u != (EZI2C_CAM_I2C_MODE_MASTER & EZI2C_CAM_mode))

    /* Returns true if multi-master mode is enabled */
    #define EZI2C_CAM_I2C_MULTI_MASTER \
                            (0u != (EZI2C_CAM_I2C_MODE_MULTI_MASTER_MASK & EZI2C_CAM_mode))

    /* Returns true if mode is multi-master-slave */
    #define EZI2C_CAM_I2C_MULTI_MASTER_SLAVE \
                            (EZI2C_CAM_I2C_MODE_MULTI_MASTER_SLAVE == EZI2C_CAM_mode)

    /* Returns true if address accepts in RX FIFO */
    #define EZI2C_CAM_CHECK_I2C_ACCEPT_ADDRESS   (0u != EZI2C_CAM_acceptAddr)
    #define EZI2C_CAM_CHECK_I2C_GENERAL_CALL \
                (0u != (EZI2C_CAM_I2C_CTRL_REG & EZI2C_CAM_I2C_CTRL_S_GENERAL_IGNORE))

    #define EZI2C_CAM_I2C_WAKE_ENABLE_CONST              (1u)
    #define EZI2C_CAM_I2C_SLAVE_CONST                    (1u)
    #define EZI2C_CAM_I2C_MASTER_CONST                   (1u)
    #define EZI2C_CAM_I2C_MULTI_MASTER_SLAVE_CONST       (1u)
    #define EZI2C_CAM_CHECK_I2C_ACCEPT_ADDRESS_CONST     (1u)
    #define EZI2C_CAM_CHECK_I2C_GENERAL_CALL_CONST       (1u)
    #define EZI2C_CAM_I2C_CUSTOM_ADDRESS_HANDLER_CONST   (1u)

    /* Returns FIFO size */
    #if (EZI2C_CAM_CY_SCBIP_V0 || EZI2C_CAM_CY_SCBIP_V1)
        #define EZI2C_CAM_I2C_FIFO_SIZE      (EZI2C_CAM_FIFO_SIZE)
    #else
        #define EZI2C_CAM_I2C_FIFO_SIZE      (EZI2C_CAM_GET_FIFO_SIZE(EZI2C_CAM_CTRL_REG & \
                                                                                    EZI2C_CAM_CTRL_BYTE_MODE))
    #endif /* (EZI2C_CAM_CY_SCBIP_V0 || EZI2C_CAM_CY_SCBIP_V1) */

#else

    /* Returns true if slave mode is enabled */
    #define EZI2C_CAM_I2C_SLAVE   (0u != (EZI2C_CAM_I2C_MODE_SLAVE & EZI2C_CAM_I2C_MODE))

    /* Returns true if master mode is enabled */
    #define EZI2C_CAM_I2C_MASTER  (0u != (EZI2C_CAM_I2C_MODE_MASTER & EZI2C_CAM_I2C_MODE))

    /* Returns true if multi-master mode is enabled */
    #define EZI2C_CAM_I2C_MULTI_MASTER \
                                    (0u != (EZI2C_CAM_I2C_MODE_MULTI_MASTER_MASK & EZI2C_CAM_I2C_MODE))

    /* Returns true if mode is multi-master-slave */
    #define EZI2C_CAM_I2C_MULTI_MASTER_SLAVE \
                                    (EZI2C_CAM_I2C_MODE_MULTI_MASTER_SLAVE == EZI2C_CAM_I2C_MODE)

    /* Returns true if address accepts in RX FIFO */
    #define EZI2C_CAM_CHECK_I2C_ACCEPT_ADDRESS   (0u != EZI2C_CAM_I2C_ACCEPT_ADDRESS)
    #define EZI2C_CAM_CHECK_I2C_GENERAL_CALL     (0u != EZI2C_CAM_I2C_ACCEPT_GENERAL_CALL)

    /* Returns true if wakeup on address match is enabled */
    #define EZI2C_CAM_I2C_WAKE_ENABLE_CONST  (0u != EZI2C_CAM_I2C_WAKE_ENABLE)

    #define EZI2C_CAM_I2C_SLAVE_CONST    (EZI2C_CAM_I2C_SLAVE)
    #define EZI2C_CAM_I2C_MASTER_CONST   (EZI2C_CAM_I2C_MASTER)
    #define EZI2C_CAM_I2C_MULTI_MASTER_SLAVE_CONST   (EZI2C_CAM_I2C_MULTI_MASTER_SLAVE)
    #define EZI2C_CAM_CHECK_I2C_ACCEPT_ADDRESS_CONST (EZI2C_CAM_CHECK_I2C_ACCEPT_ADDRESS)
    #define EZI2C_CAM_CHECK_I2C_GENERAL_CALL_CONST   (EZI2C_CAM_CHECK_I2C_GENERAL_CALL)
    #define EZI2C_CAM_I2C_CUSTOM_ADDRESS_HANDLER_CONST   (EZI2C_CAM_CHECK_I2C_ACCEPT_ADDRESS_CONST || \
                                                                  EZI2C_CAM_CHECK_I2C_GENERAL_CALL_CONST)

    /* I2C: TX or RX FIFO size */
    #define EZI2C_CAM_I2C_FIFO_SIZE  EZI2C_CAM_GET_FIFO_SIZE(EZI2C_CAM_I2C_BYTE_MODE_ENABLE)

    /* Adjust AF and DF filter settings. Ticket ID#176179 */
    #if ((EZI2C_CAM_I2C_MODE_SLAVE == EZI2C_CAM_I2C_MODE) ||     \
            ((EZI2C_CAM_I2C_MODE_SLAVE != EZI2C_CAM_I2C_MODE) && \
             (EZI2C_CAM_I2C_DATA_RATE_ACTUAL <= EZI2C_CAM_I2C_DATA_RATE_FS_MODE_MAX)))

        #define EZI2C_CAM_I2C_MEDIAN_FILTER_ENABLE_ADJ       (0u)
        #define EZI2C_CAM_I2C_CFG_ANALOG_FITER_ENABLE_ADJ    do{;}while(0)
    #else
        #define EZI2C_CAM_I2C_MEDIAN_FILTER_ENABLE_ADJ       (1u)
        #define EZI2C_CAM_I2C_CFG_ANALOG_FITER_ENABLE_ADJ    EZI2C_CAM_I2C_CFG_ANALOG_FITER_DISABLE
    #endif

    /* Select oversampling factor low and high */
    #define EZI2C_CAM_I2C_OVS_FACTOR_LOW_MIN     ((0u != EZI2C_CAM_I2C_MANUAL_OVS_CONTROL) ? \
                                                            (8u) : (8u))

    #define EZI2C_CAM_I2C_OVS_FACTOR_HIGH_MIN    ((0u != EZI2C_CAM_I2C_MANUAL_OVS_CONTROL) ? \
                                                            (8u) : (8u))

#endif /* (EZI2C_CAM_SCB_MODE_UNCONFIG_CONST_CFG) */

#define EZI2C_CAM_I2C_CUSTOM_ADDRESS_HANDLER (EZI2C_CAM_CHECK_I2C_GENERAL_CALL || \
                                                     EZI2C_CAM_CHECK_I2C_ACCEPT_ADDRESS)


/***************************************
*       Type Definitions
***************************************/

/**
* \addtogroup group_structures
* @{
*/
typedef struct
{
    /** Mode of operation for I2C. The following defines are available choices:
     *  - EZI2C_CAM_I2C_MODE_SLAVE
     *  - EZI2C_CAM_I2C_MODE_MASTER
     *  - EZI2C_CAM_I2C_MODE_MULTI_MASTER
     *  - EZI2C_CAM_I2C_MODE_MULTI_MASTER_SLAVE
    */
    uint32 mode;

    /** Oversampling factor for the low phase of the I2C clock. Ignored for
     *  Slave mode operation.  The oversampling factors need to be chosen in
     *  conjunction with the clock rate in order to generate the desired rate
     *  of I2C operation.
    */
    uint32 oversampleLow;

    /** Oversampling factor for the high phase of the I2C clock. Ignored for
     *  Slave mode operation.
    */
    uint32 oversampleHigh;

    /** This field is left for compatibility and its value is ignored.
     *  Median filter is enabled or disabled depends on the data rate and
     *  operation mode.
    */
    uint32 enableMedianFilter;

    /** 7-bit slave address. Ignored for non-slave modes.
    */
    uint32 slaveAddr;

    /** 8-bit slave address mask. Bit 0 must have a value of 0. Ignored for
     *  non-slave modes.
     *  - Bit value 0 – excludes bit from address comparison.
     *  - Bit value 1 – the bit needs to match with the corresponding bit
     *   of the device address.
    */
    uint32 slaveAddrMask;

    /** When enabled the matching address is received into the RX FIFO:
     *  0 – disable, 1 – enable.
     *
     *  The callback function has to be registered to handle the address
     *  accepted in the RX FIFO. Refer to section Accept matching address
     *  RX FIFO for more information.
    */
    uint32 acceptAddr;

    /** When enabled the matching address generates a wakeup request:
     *  0 – disable, 1 – enable. 1 – enable. Ignored for non-slave modes.
    */
    uint32 enableWake;

    /** When enabled the TX and RX FIFO depth is doubled and equal to
     *  16 bytes: 0 – disable, 1 – enable.
     *
     * Ignored for all devices other than PSoC 4100 BLE / PSoC 4200 BLE /
     * PSoC 4100M / PSoC 4200M / PSoC 4200L / PSoC 4000S / PSoC 4100S /
     * PSoC Analog Coprocessor.
    */
    uint8  enableByteMode;

    /** Data rate in kbps used while the of I2C is in operation. Valid values
      * are between 1 and 1000. Note that this filed must be initialized
      * for correct operation if Unconfigured SCB was utilized with previous
      * version of the component.
    */
    uint16 dataRate;

    /** When enabled the I2C general call address (0x00) will be accepted by
     *  the I2C hardware and trigger an interrupt: 0 – disable, 1 – enable.
     *
     *  The callback function has to be registered to handle a general call
     *  address. Refer to section Accept General Call for more information.
    */
    uint8  acceptGeneralAddr;
} EZI2C_CAM_I2C_INIT_STRUCT;
/** @} structures */

/***************************************
*        Function Prototypes
***************************************/

/**
* \addtogroup group_i2c
* @{
*/
/* Common functions */
#if(EZI2C_CAM_SCB_MODE_UNCONFIG_CONST_CFG)
    void EZI2C_CAM_I2CInit(const EZI2C_CAM_I2C_INIT_STRUCT *config);
#endif /* (EZI2C_CAM_SCB_MODE_UNCONFIG_CONST_CFG) */
/** @} i2c */

/**
* \addtogroup group_interrupt
* @{
*/
#if (EZI2C_CAM_I2C_CUSTOM_ADDRESS_HANDLER_CONST)
    void EZI2C_CAM_SetI2cAddressCustomInterruptHandler(uint32 (*func) (void));
#endif /* (EZI2C_CAM_I2C_CUSTOM_ADDRESS_HANDLER_CONST) */
/** @} interrupt */

/**
* \addtogroup group_i2c
* @{
*/
/* I2C Master functions prototypes */
#if(EZI2C_CAM_I2C_MASTER_CONST)
    /* Read and Clear status functions */
    uint32 EZI2C_CAM_I2CMasterStatus(void);
    uint32 EZI2C_CAM_I2CMasterClearStatus(void);

    /* Interrupt based operation functions */
    uint32 EZI2C_CAM_I2CMasterWriteBuf(uint32 slaveAddress, uint8 * wrData, uint32 cnt, uint32 mode);
    uint32 EZI2C_CAM_I2CMasterReadBuf(uint32 slaveAddress, uint8 * rdData, uint32 cnt, uint32 mode);
    uint32 EZI2C_CAM_I2CMasterGetReadBufSize(void);
    uint32 EZI2C_CAM_I2CMasterGetWriteBufSize(void);
    void   EZI2C_CAM_I2CMasterClearReadBuf(void);
    void   EZI2C_CAM_I2CMasterClearWriteBuf(void);

    /* Manual operation functions */
    uint32 EZI2C_CAM_I2CMasterSendStart(uint32 slaveAddress, uint32 bitRnW);
    uint32 EZI2C_CAM_I2CMasterSendRestart(uint32 slaveAddress, uint32 bitRnW);
    uint32 EZI2C_CAM_I2CMasterSendStop(void);
    uint32 EZI2C_CAM_I2CMasterWriteByte(uint32 theByte);
    uint32 EZI2C_CAM_I2CMasterReadByte(uint32 ackNack);
#endif /* (EZI2C_CAM_I2C_MASTER_CONST) */

/* I2C Slave functions prototypes */
#if(EZI2C_CAM_I2C_SLAVE_CONST)
    /* Read and Clear status functions */
    uint32 EZI2C_CAM_I2CSlaveStatus(void);
    uint32 EZI2C_CAM_I2CSlaveClearReadStatus(void);
    uint32 EZI2C_CAM_I2CSlaveClearWriteStatus(void);

    /* Set Slave address and mask */
    void   EZI2C_CAM_I2CSlaveSetAddress(uint32 address);
    void   EZI2C_CAM_I2CSlaveSetAddressMask(uint32 addressMask);

    /* Interrupt based operation functions */
    void   EZI2C_CAM_I2CSlaveInitReadBuf(uint8 * rdBuf, uint32 bufSize);
    void   EZI2C_CAM_I2CSlaveInitWriteBuf(uint8 * wrBuf, uint32 bufSize);
    uint32 EZI2C_CAM_I2CSlaveGetReadBufSize(void);
    uint32 EZI2C_CAM_I2CSlaveGetWriteBufSize(void);
    void   EZI2C_CAM_I2CSlaveClearReadBuf(void);
    void   EZI2C_CAM_I2CSlaveClearWriteBuf(void);
#endif /* (EZI2C_CAM_I2C_SLAVE_CONST) */
/** @} i2c */

CY_ISR_PROTO(EZI2C_CAM_I2C_ISR);


/***************************************
*            API Constants
***************************************/

/* I2C sub mode enum */
#define EZI2C_CAM_I2C_MODE_SLAVE              (0x01u)
#define EZI2C_CAM_I2C_MODE_MASTER             (0x02u)
#define EZI2C_CAM_I2C_MODE_MULTI_MASTER       (0x06u)
#define EZI2C_CAM_I2C_MODE_MULTI_MASTER_SLAVE (0x07u)
#define EZI2C_CAM_I2C_MODE_MULTI_MASTER_MASK  (0x04u)

/* Master/Slave control constants */
#define EZI2C_CAM_I2C_WRITE_XFER_MODE    (0u)    /* Write    */
#define EZI2C_CAM_I2C_READ_XFER_MODE     (1u)    /* Read     */
#define EZI2C_CAM_I2C_ACK_ADDR           (0u)    /* Send ACK to address */
#define EZI2C_CAM_I2C_NAK_ADDR           (1u)    /* Send NAK to address */
#define EZI2C_CAM_I2C_ACK_DATA           (0u)    /* Send ACK to data */
#define EZI2C_CAM_I2C_NAK_DATA           (1u)    /* Send NAK to data */

/* "Mode" constants for MasterWriteBuf() or MasterReadBuf() function */
#define EZI2C_CAM_I2C_MODE_COMPLETE_XFER     (0x00u)    /* Full transfer with Start and Stop       */
#define EZI2C_CAM_I2C_MODE_REPEAT_START      (0x01u)    /* Begin with a ReStart instead of a Start */
#define EZI2C_CAM_I2C_MODE_NO_STOP           (0x02u)    /* Complete the transfer without a Stop    */

/* Master status */
#define EZI2C_CAM_I2C_MSTAT_CLEAR            ((uint16) 0x00u)   /* Clear (init) status value */

#define EZI2C_CAM_I2C_MSTAT_RD_CMPLT         ((uint16) 0x01u)   /* Read complete               */
#define EZI2C_CAM_I2C_MSTAT_WR_CMPLT         ((uint16) 0x02u)   /* Write complete              */
#define EZI2C_CAM_I2C_MSTAT_XFER_INP         ((uint16) 0x04u)   /* Master transfer in progress */
#define EZI2C_CAM_I2C_MSTAT_XFER_HALT        ((uint16) 0x08u)   /* Transfer is halted          */

#define EZI2C_CAM_I2C_MSTAT_ERR_MASK         ((uint16) 0x3F0u) /* Mask for all errors                          */
#define EZI2C_CAM_I2C_MSTAT_ERR_SHORT_XFER   ((uint16) 0x10u)  /* Master NAKed before end of packet            */
#define EZI2C_CAM_I2C_MSTAT_ERR_ADDR_NAK     ((uint16) 0x20u)  /* Slave did not ACK                            */
#define EZI2C_CAM_I2C_MSTAT_ERR_ARB_LOST     ((uint16) 0x40u)  /* Master lost arbitration during communication */
#define EZI2C_CAM_I2C_MSTAT_ERR_ABORT_XFER   ((uint16) 0x80u)  /* The Slave was addressed before the Start gen */
#define EZI2C_CAM_I2C_MSTAT_ERR_BUS_ERROR    ((uint16) 0x100u) /* The misplaced Start or Stop was occurred     */
#define EZI2C_CAM_I2C_MSTAT_ERR_XFER         ((uint16) 0x200u) /* Error during transfer                        */

/* Master API returns */
#define EZI2C_CAM_I2C_MSTR_NO_ERROR          (0x00u)  /* Function complete without error                       */
#define EZI2C_CAM_I2C_MSTR_ERR_ARB_LOST      (0x01u)  /* Master lost arbitration: INTR_MASTER_I2C_ARB_LOST     */
#define EZI2C_CAM_I2C_MSTR_ERR_LB_NAK        (0x02u)  /* Last Byte Naked: INTR_MASTER_I2C_NACK                 */
#define EZI2C_CAM_I2C_MSTR_NOT_READY         (0x04u)  /* Master on the bus or Slave operation is in progress   */
#define EZI2C_CAM_I2C_MSTR_BUS_BUSY          (0x08u)  /* Bus is busy, process not started                      */
#define EZI2C_CAM_I2C_MSTR_ERR_ABORT_START   (0x10u)  /* Slave was addressed before master begin Start gen     */
#define EZI2C_CAM_I2C_MSTR_ERR_BUS_ERR       (0x100u) /* Bus error has: INTR_MASTER_I2C_BUS_ERROR              */

/* Slave Status Constants */
#define EZI2C_CAM_I2C_SSTAT_RD_CMPLT         (0x01u)    /* Read transfer complete                        */
#define EZI2C_CAM_I2C_SSTAT_RD_BUSY          (0x02u)    /* Read transfer in progress                     */
#define EZI2C_CAM_I2C_SSTAT_RD_OVFL          (0x04u)    /* Read overflow: master reads above buffer size */
#define EZI2C_CAM_I2C_SSTAT_RD_ERR           (0x08u)    /* Read was interrupted by misplaced Start/Stop  */
#define EZI2C_CAM_I2C_SSTAT_RD_MASK          (0x0Fu)    /* Read Status Mask                              */
#define EZI2C_CAM_I2C_SSTAT_RD_NO_ERR        (0x00u)    /* Read no Error                                 */

#define EZI2C_CAM_I2C_SSTAT_WR_CMPLT         (0x10u)    /* Write transfer complete                         */
#define EZI2C_CAM_I2C_SSTAT_WR_BUSY          (0x20u)    /* Write transfer in progress                      */
#define EZI2C_CAM_I2C_SSTAT_WR_OVFL          (0x40u)    /* Write overflow: master writes above buffer size */
#define EZI2C_CAM_I2C_SSTAT_WR_ERR           (0x80u)    /* Write was interrupted by misplaced Start/Stop   */
#define EZI2C_CAM_I2C_SSTAT_WR_MASK          (0xF0u)    /* Write Status Mask                               */
#define EZI2C_CAM_I2C_SSTAT_WR_NO_ERR        (0x00u)    /* Write no Error                                  */

#define EZI2C_CAM_I2C_SSTAT_RD_CLEAR         (0x0Du)    /* Read Status clear: do not clear */
#define EZI2C_CAM_I2C_SSTAT_WR_CLEAR         (0xD0u)    /* Write Status Clear */

/* Internal I2C component constants */
#define EZI2C_CAM_I2C_READ_FLAG              (0x01u)     /* Read flag of the Address */
#define EZI2C_CAM_I2C_SLAVE_OVFL_RETURN      (0xFFu)     /* Return by slave when overflow */

#define EZI2C_CAM_I2C_RESET_ERROR            (0x01u)     /* Flag to re-enable SCB IP */

#define EZI2C_CAM_I2C_TX_OVERFLOW_COUNT      (EZI2C_CAM_I2C_FIFO_SIZE + 2u)


/***************************************
*     Vars with External Linkage
***************************************/

#if(EZI2C_CAM_SCB_MODE_UNCONFIG_CONST_CFG)
    extern const EZI2C_CAM_I2C_INIT_STRUCT EZI2C_CAM_configI2C;
#endif /* (EZI2C_CAM_SCB_MODE_UNCONFIG_CONST_CFG) */


/***************************************
*           FSM states
***************************************/

#define EZI2C_CAM_I2C_FSM_EXIT_IDLE      (0x00u) /* Master and Slave are not active, need to exit to IDLE */

/* Slave mode FSM states */
#define EZI2C_CAM_I2C_FSM_IDLE           (0x10u) /* Idle I2C state                */
#define EZI2C_CAM_I2C_FSM_SLAVE          (0x10u) /* Slave mask for all its states */

#define EZI2C_CAM_I2C_FSM_SL_WR          (0x11u) /* Slave write states */
#define EZI2C_CAM_I2C_FSM_SL_RD          (0x12u) /* Slave read  states */

/* Master mode FSM states */
#define EZI2C_CAM_I2C_FSM_MASTER         (0x20u) /* Master mask for all its states */
#define EZI2C_CAM_I2C_FSM_MSTR_ADDR      (0x08u) /* Master address phase           */
#define EZI2C_CAM_I2C_FSM_MSTR_DATA      (0x04u) /* Master data phase              */
#define EZI2C_CAM_I2C_FSM_MSTR_RD        (0x01u) /* Master read phase              */

#define EZI2C_CAM_I2C_FSM_MSTR_WR_ADDR   (0x28u) /* FSM master transmit address with write          */
#define EZI2C_CAM_I2C_FSM_MSTR_RD_ADDR   (0x29u) /* FSM master transmit address with read           */
#define EZI2C_CAM_I2C_FSM_MSTR_WR_DATA   (0x24u) /* FSM master writes data into the slave           */
#define EZI2C_CAM_I2C_FSM_MSTR_RD_DATA   (0x25u) /* FSM master reads data from the slave            */
#define EZI2C_CAM_I2C_FSM_MSTR_HALT      (0x60u) /* FSM master halt state: wait for Stop or ReStart */

/* Requests to complete any on-going transfer */
#define EZI2C_CAM_I2C_CMPLT_ANY_TRANSFER     (0x01u)

/* Returns true if FSM is in any Master state */
#define EZI2C_CAM_CHECK_I2C_FSM_MASTER   (0u != (EZI2C_CAM_I2C_FSM_MASTER & EZI2C_CAM_state))

/* Returns true if FSM is in any Slave state */
#define EZI2C_CAM_CHECK_I2C_FSM_SLAVE    (0u != (EZI2C_CAM_I2C_FSM_SLAVE  & EZI2C_CAM_state))

/* Returns true if FSM is in Master send address state */
#define EZI2C_CAM_CHECK_I2C_FSM_ADDR (0u != (EZI2C_CAM_I2C_FSM_MSTR_ADDR & EZI2C_CAM_state))

/* Returns true if FSM is in Master send or receive data state */
#define EZI2C_CAM_CHECK_I2C_FSM_DATA (0u != (EZI2C_CAM_I2C_FSM_MSTR_DATA  & EZI2C_CAM_state))

/* Returns true if FSM is in any of read states: applied for Slave and Master */
#define EZI2C_CAM_CHECK_I2C_FSM_RD   (0u != (EZI2C_CAM_I2C_FSM_MSTR_RD  & EZI2C_CAM_state))

/* Returns true if FSM is in IDLE state */
#define EZI2C_CAM_CHECK_I2C_FSM_IDLE (EZI2C_CAM_I2C_FSM_IDLE == EZI2C_CAM_state)

/* Returns true if FSM is HALT state */
#define EZI2C_CAM_CHECK_I2C_FSM_HALT (EZI2C_CAM_I2C_FSM_MSTR_HALT == EZI2C_CAM_state)

/* Set Master read or write completion depends on state */
#define EZI2C_CAM_GET_I2C_MSTAT_CMPLT (EZI2C_CAM_CHECK_I2C_FSM_RD ?           \
                                                    (EZI2C_CAM_I2C_MSTAT_RD_CMPLT) : \
                                                    (EZI2C_CAM_I2C_MSTAT_WR_CMPLT))


/***************************************
*       Macro Definitions
***************************************/

/* Returns TRUE if sourceMask is set in EZI2C_CAM_I2C_MASTER_CMD_REG: used to check if Start was generated */
#define EZI2C_CAM_CHECK_I2C_MASTER_CMD(sourceMask)   (0u != (EZI2C_CAM_I2C_MASTER_CMD_REG & (sourceMask)))

/* Returns TRUE if EZI2C_CAM_MODE_NO_STOP is set in EZI2C_CAM_mstrControl: detects NoStop condition */
#define EZI2C_CAM_CHECK_I2C_MODE_NO_STOP(mode)   (0u != (EZI2C_CAM_I2C_MODE_NO_STOP & (mode)))

/* Returns TRUE if EZI2C_CAM_MODE_REPEAT_START is set: used to detect when generate ReStart condition */
#define EZI2C_CAM_CHECK_I2C_MODE_RESTART(mode)   (0u != (EZI2C_CAM_I2C_MODE_REPEAT_START  & (mode)))

/* Returns TRUE if EZI2C_CAM_state is in one of master states */
#define EZI2C_CAM_CHECK_I2C_MASTER_ACTIVE    (EZI2C_CAM_CHECK_I2C_FSM_MASTER)


/***************************************
*      Common Register Settings
***************************************/

#define EZI2C_CAM_CTRL_I2C       (EZI2C_CAM_CTRL_MODE_I2C)

#define EZI2C_CAM_I2C_CTRL       (EZI2C_CAM_I2C_CTRL_S_GENERAL_IGNORE)

#define EZI2C_CAM_I2C_RX_CTRL    ((EZI2C_CAM_ONE_BYTE_WIDTH - 1u) | \
                                          EZI2C_CAM_RX_CTRL_MSB_FIRST    | \
                                          EZI2C_CAM_RX_CTRL_ENABLED)

#define EZI2C_CAM_I2C_TX_CTRL    ((EZI2C_CAM_ONE_BYTE_WIDTH - 1u) | \
                                         EZI2C_CAM_TX_CTRL_MSB_FIRST     | \
                                         EZI2C_CAM_TX_CTRL_ENABLED)

#define EZI2C_CAM_I2C_INTR_SLAVE_MASK    (EZI2C_CAM_INTR_SLAVE_I2C_ADDR_MATCH | \
                                                 EZI2C_CAM_INTR_SLAVE_I2C_NACK       | \
                                                 EZI2C_CAM_INTR_SLAVE_I2C_WRITE_STOP | \
                                                 EZI2C_CAM_INTR_SLAVE_I2C_BUS_ERROR  | \
                                                 EZI2C_CAM_INTR_SLAVE_I2C_ARB_LOST)

#define EZI2C_CAM_I2C_INTR_MASTER_MASK   (EZI2C_CAM_INTR_MASTER_I2C_ARB_LOST | \
                                                 EZI2C_CAM_INTR_MASTER_I2C_NACK     | \
                                                 EZI2C_CAM_INTR_MASTER_I2C_STOP     | \
                                                 EZI2C_CAM_INTR_MASTER_I2C_BUS_ERROR)

/* Calculates tLOW in uS units */
#define EZI2C_CAM_I2C_TLOW_TIME  ((1000u / EZI2C_CAM_I2C_DATA_RATE) + \
                                            ((0u != (1000u % EZI2C_CAM_I2C_DATA_RATE)) ? (1u) : (0u)))
/* tHIGH = tLOW */
#define EZI2C_CAM_I2C_THIGH_TIME (EZI2C_CAM_I2C_TLOW_TIME)

#define EZI2C_CAM_I2C_SCL_LOW    (0u)
#define EZI2C_CAM_I2C_SCL_HIGH   (1u)

#define EZI2C_CAM_I2C_IGNORE_GENERAL_CALL    ((uint32) (0u == EZI2C_CAM_I2C_ACCEPT_GENERAL_CALL))


/***************************************
*    Initialization Register Settings
***************************************/

#if(EZI2C_CAM_SCB_MODE_I2C_CONST_CFG)

    #if (!EZI2C_CAM_CY_SCBIP_V0)
        #define EZI2C_CAM_I2C_WAKE_ENABLE_ADJ    (EZI2C_CAM_I2C_MULTI_MASTER_SLAVE ? \
                                                            (0u) : (EZI2C_CAM_I2C_WAKE_ENABLE))
    #else
        #define EZI2C_CAM_I2C_WAKE_ENABLE_ADJ    (EZI2C_CAM_I2C_WAKE_ENABLE)
    #endif /* (!EZI2C_CAM_CY_SCBIP_V0) */

    #define EZI2C_CAM_I2C_MODE_MASKED    (EZI2C_CAM_I2C_MODE & \
                                                (EZI2C_CAM_I2C_MODE_SLAVE | EZI2C_CAM_I2C_MODE_MASTER))

    #define EZI2C_CAM_I2C_DEFAULT_CTRL \
                                (EZI2C_CAM_GET_CTRL_BYTE_MODE  (EZI2C_CAM_I2C_BYTE_MODE_ENABLE) | \
                                 EZI2C_CAM_GET_CTRL_ADDR_ACCEPT(EZI2C_CAM_I2C_ACCEPT_ADDRESS)   | \
                                 EZI2C_CAM_GET_CTRL_EC_AM_MODE (EZI2C_CAM_I2C_WAKE_ENABLE_ADJ))

    #define EZI2C_CAM_I2C_DEFAULT_I2C_CTRL \
                    (EZI2C_CAM_GET_I2C_CTRL_HIGH_PHASE_OVS(EZI2C_CAM_I2C_OVS_FACTOR_HIGH_MIN)   | \
                     EZI2C_CAM_GET_I2C_CTRL_LOW_PHASE_OVS (EZI2C_CAM_I2C_OVS_FACTOR_LOW_MIN)    | \
                     EZI2C_CAM_GET_I2C_CTRL_S_GENERAL_IGNORE(EZI2C_CAM_I2C_IGNORE_GENERAL_CALL) | \
                     EZI2C_CAM_GET_I2C_CTRL_SL_MSTR_MODE  (EZI2C_CAM_I2C_MODE_MASKED))

    #define EZI2C_CAM_I2C_DEFAULT_RX_MATCH ((EZI2C_CAM_I2C_SLAVE) ? \
                                (EZI2C_CAM_GET_I2C_8BIT_ADDRESS(EZI2C_CAM_I2C_SLAVE_ADDRESS) | \
                                 EZI2C_CAM_GET_RX_MATCH_MASK   (EZI2C_CAM_I2C_SLAVE_ADDRESS_MASK)) : (0u))

    #define EZI2C_CAM_I2C_DEFAULT_RX_CTRL \
                                (EZI2C_CAM_GET_RX_CTRL_MEDIAN(EZI2C_CAM_I2C_MEDIAN_FILTER_ENABLE_ADJ) | \
                                 EZI2C_CAM_I2C_RX_CTRL)

    #define EZI2C_CAM_I2C_DEFAULT_TX_CTRL  (EZI2C_CAM_I2C_TX_CTRL)

    #define EZI2C_CAM_I2C_DEFAULT_RX_FIFO_CTRL (0u)
    #define EZI2C_CAM_I2C_DEFAULT_TX_FIFO_CTRL (0u)

    /* Interrupt sources */
    #define EZI2C_CAM_I2C_DEFAULT_INTR_I2C_EC_MASK   (EZI2C_CAM_NO_INTR_SOURCES)
    #define EZI2C_CAM_I2C_DEFAULT_INTR_SPI_EC_MASK   (EZI2C_CAM_NO_INTR_SOURCES)
    #define EZI2C_CAM_I2C_DEFAULT_INTR_RX_MASK       (EZI2C_CAM_NO_INTR_SOURCES)
    #define EZI2C_CAM_I2C_DEFAULT_INTR_TX_MASK       (EZI2C_CAM_NO_INTR_SOURCES)

    #define EZI2C_CAM_I2C_DEFAULT_INTR_SLAVE_MASK    ((EZI2C_CAM_I2C_SLAVE) ? \
                (EZI2C_CAM_I2C_INTR_SLAVE_MASK | \
                 EZI2C_CAM_GET_INTR_SLAVE_I2C_GENERAL(EZI2C_CAM_I2C_ACCEPT_GENERAL_CALL)) : (0u))

    #define EZI2C_CAM_I2C_DEFAULT_INTR_MASTER_MASK   ((EZI2C_CAM_I2C_MASTER) ? \
                                                                (EZI2C_CAM_I2C_INTR_MASTER_MASK) : (0u))

#endif /* (EZI2C_CAM_SCB_MODE_I2C_CONST_CFG) */

#endif /* (CY_SCB_I2C_EZI2C_CAM_H) */


/* [] END OF FILE */
