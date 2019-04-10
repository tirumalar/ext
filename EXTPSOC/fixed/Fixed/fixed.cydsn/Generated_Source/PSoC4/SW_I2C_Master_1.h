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
#if !defined(CY_SW_I2C_Master_1_H)
#define CY_SW_I2C_Master_1_H

    #include "cytypes.h"
    #include "cypins.h"
    #include "SW_I2C_Master_1_sda.h"
    #include "SW_I2C_Master_1_scl.h"
      
    /* SDA Pin macros */
    #define SetSDA  (SW_I2C_Master_1_sda_DR |= ( ((uint32) 1 << SW_I2C_Master_1_sda_SHIFT)))
    #define ClrSDA  (SW_I2C_Master_1_sda_DR &= (~((uint32) 1 << SW_I2C_Master_1_sda_SHIFT)))
    #define GetSDA  (SW_I2C_Master_1_sda_PS &  ( ((uint32) 1 << SW_I2C_Master_1_sda_SHIFT)))
    
    /* SCL Pin macros */
    #define SetSCL  (SW_I2C_Master_1_scl_DR |= ( ((uint32) 1 << SW_I2C_Master_1_scl_SHIFT)))
    #define ClrSCL  (SW_I2C_Master_1_scl_DR &= (~((uint32) 1 << SW_I2C_Master_1_scl_SHIFT)))
    #define GetSCL  (SW_I2C_Master_1_scl_PS &  ( ((uint32) 1 << SW_I2C_Master_1_scl_SHIFT)))    

    /* I2C State Machine */
    #define SW_I2C_Master_1_STATE_IDLE         0u
    #define SW_I2C_Master_1_STATE_START        1u
    #define SW_I2C_Master_1_STATE_START_CLK    2u
    #define SW_I2C_Master_1_STATE_ADDRESS      3u
    #define SW_I2C_Master_1_STATE_ADDRESS_CLK  4u
    #define SW_I2C_Master_1_STATE_ACK          7u
    #define SW_I2C_Master_1_STATE_ACK_CLK      8u
    #define SW_I2C_Master_1_STATE_NACK         9u
    #define SW_I2C_Master_1_STATE_NACK_CLK     10u
    #define SW_I2C_Master_1_STATE_DATA         11u
    #define SW_I2C_Master_1_STATE_DATA_CLK     12u    
    #define SW_I2C_Master_1_STATE_STOP         13u    
    #define SW_I2C_Master_1_STATE_STOP_CLK     14u
    #define SW_I2C_Master_1_STATE_NO_STOP      15u
    
    /* API Process returns */
    #define SW_I2C_Master_1_MSTAT_RD_CMPLT         ((uint16) 0x01u)   /* Read complete               */
    #define SW_I2C_Master_1_MSTAT_WR_CMPLT         ((uint16) 0x02u)   /* Write complete              */
    #define SW_I2C_Master_1_MSTAT_XFER_INP         ((uint16) 0x04u)   /* Master transfer in progress */
    #define SW_I2C_Master_1_MSTAT_XFER_HALT        ((uint16) 0x08u)   /* Transfer is halted          */

    #define SW_I2C_Master_1_MSTAT_ERR_MASK         ((uint16) 0x3F0u) /* Mask for all errors                          */
    #define SW_I2C_Master_1_MSTAT_ERR_SHORT_XFER   ((uint16) 0x10u)  /* Master NAKed before end of packet            */
    #define SW_I2C_Master_1_MSTAT_ERR_ADDR_NAK     ((uint16) 0x20u)  /* Slave did not ACK                            */
    #define SW_I2C_Master_1_MSTAT_ERR_ARB_LOST     ((uint16) 0x40u)  /* Master lost arbitration during communication */
    #define SW_I2C_Master_1_MSTAT_ERR_ABORT_XFER   ((uint16) 0x80u)  /* The Slave was addressed before the Start gen */
    #define SW_I2C_Master_1_MSTAT_ERR_BUS_ERROR    ((uint16) 0x100u) /* The misplaced Start or Stop was occurred     */
    #define SW_I2C_Master_1_MSTAT_ERR_XFER         ((uint16) 0x200u) /* Error during transfer                        */
 
    /* Master API returns */
    #define SW_I2C_Master_1_MSTR_NO_ERROR          (0x00u)  /* Function complete without error                      */
    #define SW_I2C_Master_1_MSTR_ERR_ARB_LOST      (0x01u)  /* Master lost arbitration: INTR_MASTER_I2C_ARB_LOST    */
    #define SW_I2C_Master_1_MSTR_ERR_LB_NAK        (0x02u)  /* Last Byte Naked: INTR_MASTER_I2C_NACK                */
    #define SW_I2C_Master_1_MSTR_NOT_READY         (0x04u)  /* Master on the bus or Slave operation is in progress  */
    #define SW_I2C_Master_1_MSTR_BUS_BUSY          (0x08u)  /* Bus is busy, process not started                     */
    #define SW_I2C_Master_1_MSTR_ERR_ABORT_START   (0x10u)  /* Slave was addressed before master begin Start gen    */
    #define SW_I2C_Master_1_MSTR_ERR_BUS_ERR       (0x100u) /* Bus error has: INTR_MASTER_I2C_BUS_ERROR             */
    
    /* "Mode" constants for WriteBuf() or ReadBuf() function */
    #define SW_I2C_Master_1_MODE_COMPLETE_XFER     (0x00u)    /* Full transfer with Start and Stop       */
    #define SW_I2C_Master_1_MODE_REPEAT_START      (0x01u)    /* Begin with a ReStart instead of a Start */
    #define SW_I2C_Master_1_MODE_NO_STOP           (0x02u)    /* Complete the transfer without a Stop    */
    #define SW_I2C_Master_1_WRITE_FRAME            (0x04u)    /* Write frame to be sent */
    #define SW_I2C_Master_1_READ_FRAME             (0x08u)    /* Read frame to be sent */
    #define SW_I2C_Master_1_INITIAL_ADDR_ACK       (0x10u)    /* Initial address to ack */
    
    /* ACK/NACK constants for ReadByte() */
    #define SW_I2C_Master_1_ACK_DATA           (0u)    /* Send ACK to data */
    #define SW_I2C_Master_1_NAK_DATA           (1u)    /* Send NAK to data */
    
    /* High level operation functions */
    uint32 SW_I2C_Master_1_WriteBuf(uint32 slaveAddress, uint8 * wrData, uint32 cnt, uint32 mode);
    uint32 SW_I2C_Master_1_ReadBuf(uint32 slaveAddress, uint8 * rdData, uint32 cnt, uint32 mode);

    /* Manual operation functions */
    uint32 SW_I2C_Master_1_SendStart(uint32 slaveAddress);
    uint32 SW_I2C_Master_1_SendRestart(uint32 slaveAddress);
    uint32 SW_I2C_Master_1_SendStop(void);
    uint32 SW_I2C_Master_1_WriteByte(uint32 theByte);
    uint32 SW_I2C_Master_1_ReadByte(uint32 ackNack);
    
    /* Process Function */
    uint32 SW_I2C_Master_1_Process(void);
    void   SW_I2C_Master_1_ClearStatus(void);
    
#endif /* CY_SW_I2C_Master_1_H */
    
/* [] END OF FILE */
