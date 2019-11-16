/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#if !defined(CY_`$INSTANCE_NAME`_H)
#define CY_`$INSTANCE_NAME`_H

    #include "cytypes.h"
    #include "cypins.h"
    #include "`$INSTANCE_NAME`_sda.h"
    #include "`$INSTANCE_NAME`_scl.h"
      
    /* SDA Pin macros */
    #define SetSDA  (`$INSTANCE_NAME`_sda_DR |= ( ((uint32) 1 << `$INSTANCE_NAME`_sda_SHIFT)))
    #define ClrSDA  (`$INSTANCE_NAME`_sda_DR &= (~((uint32) 1 << `$INSTANCE_NAME`_sda_SHIFT)))
    #define GetSDA  (`$INSTANCE_NAME`_sda_PS &  ( ((uint32) 1 << `$INSTANCE_NAME`_sda_SHIFT)))
    
    /* SCL Pin macros */
    #define SetSCL  (`$INSTANCE_NAME`_scl_DR |= ( ((uint32) 1 << `$INSTANCE_NAME`_scl_SHIFT)))
    #define ClrSCL  (`$INSTANCE_NAME`_scl_DR &= (~((uint32) 1 << `$INSTANCE_NAME`_scl_SHIFT)))
    #define GetSCL  (`$INSTANCE_NAME`_scl_PS &  ( ((uint32) 1 << `$INSTANCE_NAME`_scl_SHIFT)))    

    /* I2C State Machine */
    #define `$INSTANCE_NAME`_STATE_IDLE         0u
    #define `$INSTANCE_NAME`_STATE_START        1u
    #define `$INSTANCE_NAME`_STATE_START_CLK    2u
    #define `$INSTANCE_NAME`_STATE_ADDRESS      3u
    #define `$INSTANCE_NAME`_STATE_ADDRESS_CLK  4u
    #define `$INSTANCE_NAME`_STATE_ACK          7u
    #define `$INSTANCE_NAME`_STATE_ACK_CLK      8u
    #define `$INSTANCE_NAME`_STATE_NACK         9u
    #define `$INSTANCE_NAME`_STATE_NACK_CLK     10u
    #define `$INSTANCE_NAME`_STATE_DATA         11u
    #define `$INSTANCE_NAME`_STATE_DATA_CLK     12u    
    #define `$INSTANCE_NAME`_STATE_STOP         13u    
    #define `$INSTANCE_NAME`_STATE_STOP_CLK     14u
    #define `$INSTANCE_NAME`_STATE_NO_STOP      15u
    
    /* API Process returns */
    #define `$INSTANCE_NAME`_MSTAT_RD_CMPLT         ((uint16) 0x01u)   /* Read complete               */
    #define `$INSTANCE_NAME`_MSTAT_WR_CMPLT         ((uint16) 0x02u)   /* Write complete              */
    #define `$INSTANCE_NAME`_MSTAT_XFER_INP         ((uint16) 0x04u)   /* Master transfer in progress */
    #define `$INSTANCE_NAME`_MSTAT_XFER_HALT        ((uint16) 0x08u)   /* Transfer is halted          */

    #define `$INSTANCE_NAME`_MSTAT_ERR_MASK         ((uint16) 0x3F0u) /* Mask for all errors                          */
    #define `$INSTANCE_NAME`_MSTAT_ERR_SHORT_XFER   ((uint16) 0x10u)  /* Master NAKed before end of packet            */
    #define `$INSTANCE_NAME`_MSTAT_ERR_ADDR_NAK     ((uint16) 0x20u)  /* Slave did not ACK                            */
    #define `$INSTANCE_NAME`_MSTAT_ERR_ARB_LOST     ((uint16) 0x40u)  /* Master lost arbitration during communication */
    #define `$INSTANCE_NAME`_MSTAT_ERR_ABORT_XFER   ((uint16) 0x80u)  /* The Slave was addressed before the Start gen */
    #define `$INSTANCE_NAME`_MSTAT_ERR_BUS_ERROR    ((uint16) 0x100u) /* The misplaced Start or Stop was occurred     */
    #define `$INSTANCE_NAME`_MSTAT_ERR_XFER         ((uint16) 0x200u) /* Error during transfer                        */
 
    /* Master API returns */
    #define `$INSTANCE_NAME`_MSTR_NO_ERROR          (0x00u)  /* Function complete without error                      */
    #define `$INSTANCE_NAME`_MSTR_ERR_ARB_LOST      (0x01u)  /* Master lost arbitration: INTR_MASTER_I2C_ARB_LOST    */
    #define `$INSTANCE_NAME`_MSTR_ERR_LB_NAK        (0x02u)  /* Last Byte Naked: INTR_MASTER_I2C_NACK                */
    #define `$INSTANCE_NAME`_MSTR_NOT_READY         (0x04u)  /* Master on the bus or Slave operation is in progress  */
    #define `$INSTANCE_NAME`_MSTR_BUS_BUSY          (0x08u)  /* Bus is busy, process not started                     */
    #define `$INSTANCE_NAME`_MSTR_ERR_ABORT_START   (0x10u)  /* Slave was addressed before master begin Start gen    */
    #define `$INSTANCE_NAME`_MSTR_ERR_BUS_ERR       (0x100u) /* Bus error has: INTR_MASTER_I2C_BUS_ERROR             */
    
    /* "Mode" constants for WriteBuf() or ReadBuf() function */
    #define `$INSTANCE_NAME`_MODE_COMPLETE_XFER     (0x00u)    /* Full transfer with Start and Stop       */
    #define `$INSTANCE_NAME`_MODE_REPEAT_START      (0x01u)    /* Begin with a ReStart instead of a Start */
    #define `$INSTANCE_NAME`_MODE_NO_STOP           (0x02u)    /* Complete the transfer without a Stop    */
    #define `$INSTANCE_NAME`_WRITE_FRAME            (0x04u)    /* Write frame to be sent */
    #define `$INSTANCE_NAME`_READ_FRAME             (0x08u)    /* Read frame to be sent */
    #define `$INSTANCE_NAME`_INITIAL_ADDR_ACK       (0x10u)    /* Initial address to ack */
    
    /* ACK/NACK constants for ReadByte() */
    #define `$INSTANCE_NAME`_ACK_DATA           (0u)    /* Send ACK to data */
    #define `$INSTANCE_NAME`_NAK_DATA           (1u)    /* Send NAK to data */
    
    /* High level operation functions */
    uint32 `$INSTANCE_NAME`_WriteBuf(uint32 slaveAddress, uint8 * wrData, uint32 cnt, uint32 mode);
    uint32 `$INSTANCE_NAME`_ReadBuf(uint32 slaveAddress, uint8 * rdData, uint32 cnt, uint32 mode);

    /* Manual operation functions */
    uint32 `$INSTANCE_NAME`_SendStart(uint32 slaveAddress);
    uint32 `$INSTANCE_NAME`_SendRestart(uint32 slaveAddress);
    uint32 `$INSTANCE_NAME`_SendStop(void);
    uint32 `$INSTANCE_NAME`_WriteByte(uint32 theByte);
    uint32 `$INSTANCE_NAME`_ReadByte(uint32 ackNack);
    
    /* Process Function */
    uint32 `$INSTANCE_NAME`_Process(void);
    void   `$INSTANCE_NAME`_ClearStatus(void);
    
#endif /* CY_`$INSTANCE_NAME`_H */
    
/* [] END OF FILE */
