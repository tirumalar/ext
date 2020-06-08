/*******************************************************************************
* File Name: EZI2Cs.h
* Version 2.0
*
* Description:
*  This is the header file for the EzI2C user module.  It contains function
*  prototypes and constants for the users convenience.
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_EZI2C_EZI2Cs_H)
#define CY_EZI2C_EZI2Cs_H

#include "cyfitter.h"
#include "cytypes.h"
#include "CyLib.h"

/* Check if required defines such as CY_PSOC5LP are available in cy_boot */
#if !defined (CY_PSOC5LP)
    #error Component EZI2C_v2_0 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */

/***************************************
*   Initial Parameter Constants
***************************************/

#define EZI2Cs_DATA_RATE      (400u)
#define EZI2Cs_BUS_PORT       (0u)
#define EZI2Cs_ENABLE_WAKEUP  (0u)
#define EZI2Cs_SUBADDR_WIDTH  (1u)
#define EZI2Cs_ADDRESSES      (1u)
#define EZI2Cs_DEFAULT_ADDR1  (56u)
#define EZI2Cs_DEFAULT_ADDR2  (9u)


/***************************************
*   Conditional Compilation Parameters
***************************************/

#define EZI2Cs_WAKEUP_ENABLED (0u != EZI2Cs_ENABLE_WAKEUP)

/* Number of addresses enum. */
#define EZI2Cs_ONE_ADDRESS    (1u)
#define EZI2Cs_TWO_ADDRESSES  (2u)

/* Sub-address size enum. */
#define EZI2Cs_ANY   (0u)
#define EZI2Cs_I2C0  (1u)
#define EZI2Cs_I2C1  (2u)

/* Sub-address size enum. */
#define EZI2Cs_SUBADDR_8BIT   (0u)
#define EZI2Cs_SUBADDR_16BIT  (1u)


/***************************************
*       Type Definitions
***************************************/

/* Structure to store configuration before enter Sleep */
typedef struct
{
    uint8   enableState;
    uint8   xcfg;
    uint8   adr;
    uint8   cfg;
    uint8   clkDiv1;
    uint8   clkDiv2;

} EZI2Cs_BACKUP_STRUCT;


/***************************************
*        Function Prototypes
***************************************/

void    EZI2Cs_Init(void)     ;
void    EZI2Cs_Enable(void)   ;
void    EZI2Cs_Start(void)    ;
void    EZI2Cs_Stop(void)     ;

#define EZI2Cs_EnableInt()        CyIntEnable      (EZI2Cs_ISR_NUMBER)
#define EZI2Cs_DisableInt()       CyIntDisable     (EZI2Cs_ISR_NUMBER)
#define EZI2Cs_ClearPendingInt()  CyIntClearPending(EZI2Cs_ISR_NUMBER)
#define EZI2Cs_SetPendingInt()    CyIntSetPending  (EZI2Cs_ISR_NUMBER)

void    EZI2Cs_Sleep(void)            ;
void    EZI2Cs_Wakeup(void)           ;
void    EZI2Cs_SaveConfig(void)       ;
void    EZI2Cs_RestoreConfig(void)    ;

uint8   EZI2Cs_GetActivity(void)      ;

void    EZI2Cs_SetBuffer1(uint16 bufSize, uint16 rwBoundary, volatile uint8 * dataPtr) ;
void    EZI2Cs_SetAddress1(uint8 address) ;
uint8   EZI2Cs_GetAddress1(void)          ;

#if (EZI2Cs_ADDRESSES == EZI2Cs_TWO_ADDRESSES)
    void    EZI2Cs_SetBuffer2(uint16 bufSize, uint16 rwBoundary, volatile uint8 * dataPtr) ;
    void    EZI2Cs_SetAddress2(uint8 address )    ;
    uint8   EZI2Cs_GetAddress2(void)              ;
#endif /* (EZI2Cs_ADDRESSES == EZI2Cs_TWO_ADDRESSES) */

/* EZI2C interrupt handler */
CY_ISR_PROTO(EZI2Cs_ISR);
#if (EZI2Cs_WAKEUP_ENABLED)
    CY_ISR_PROTO(EZI2Cs_WAKEUP_ISR);
#endif /* (EZI2Cs_WAKEUP_ENABLED) */


/***************************************
*     Vars with External Linkage
***************************************/

extern uint8 EZI2Cs_initVar;


/***************************************
*              API Constants
***************************************/

/* Status bit definition */
#define EZI2Cs_STATUS_READ1   (0x01u) /* A read addr 1 operation occurred since last status check */
#define EZI2Cs_STATUS_WRITE1  (0x02u) /* A Write addr 1 operation occurred since last status check */
#define EZI2Cs_STATUS_READ2   (0x04u) /* A read addr 2 operation occurred since last status check */
#define EZI2Cs_STATUS_WRITE2  (0x08u) /* A Write addr 2 operation occurred since last status check */
#define EZI2Cs_STATUS_BUSY    (0x10u) /* A start has occurred, but a Stop has not been detected */
#define EZI2Cs_STATUS_RD1BUSY (0x11u) /* Addr 1 read busy  */
#define EZI2Cs_STATUS_WR1BUSY (0x12u) /* Addr 1 write busy */
#define EZI2Cs_STATUS_RD2BUSY (0x14u) /* Addr 2 read busy  */
#define EZI2Cs_STATUS_WR2BUSY (0x18u) /* Addr 2 write busy */
#define EZI2Cs_STATUS_MASK    (0x1Fu) /* Mask for status bits */
#define EZI2Cs_STATUS_ERR     (0x80u) /* An Error occurred since last read */

/* Data send to master in case of read overflow */
#define EZI2Cs_DUMMY_DATA         (0xFFu)

/* Address shift */
#define EZI2Cs_ADDRESS_SHIFT      (1u)
#define EZI2Cs_ADDRESS_LSB_SHIFT  (8u)

/* Component state enum. */
#define EZI2Cs_ENABLED            (0x01u)
#define EZI2Cs_DISABLED           (0x00u)

/* Return 1 if corresponding bit is set, otherwise 0 */
#define EZI2Cs_IS_BIT_SET(value, mask) (((mask) == ((value) & (mask))) ? (1u) : (0u))


/***************************************
*              Registers
***************************************/

/* I2C Extended Configuration Register */
#define EZI2Cs_XCFG_REG       (*(reg8 *) EZI2Cs_I2C_Prim__XCFG)
#define EZI2Cs_XCFG_PTR       ( (reg8 *) EZI2Cs_I2C_Prim__XCFG)

/* I2C Slave Address Register */
#define EZI2Cs_ADDR_REG       (*(reg8 *) EZI2Cs_I2C_Prim__ADR)
#define EZI2Cs_ADDR_PTR       ( (reg8 *) EZI2Cs_I2C_Prim__ADR)

/* I2C Configuration Register */
#define EZI2Cs_CFG_REG        (*(reg8 *) EZI2Cs_I2C_Prim__CFG)
#define EZI2Cs_CFG_PTR        ( (reg8 *) EZI2Cs_I2C_Prim__CFG)

/* I2C Control and Status Register */
#define EZI2Cs_CSR_REG        (*(reg8 *) EZI2Cs_I2C_Prim__CSR)
#define EZI2Cs_CSR_PTR        ( (reg8 *) EZI2Cs_I2C_Prim__CSR)

/* I2C Data Register */
#define EZI2Cs_DATA_REG       (*(reg8 *) EZI2Cs_I2C_Prim__D)
#define EZI2Cs_DATA_PTR       ( (reg8 *) EZI2Cs_I2C_Prim__D)

/*  8 LSB bits of the 10-bit Clock Divider */
#define EZI2Cs_CLKDIV1_REG    (*(reg8 *) EZI2Cs_I2C_Prim__CLK_DIV1)
#define EZI2Cs_CLKDIV1_PTR    ( (reg8 *) EZI2Cs_I2C_Prim__CLK_DIV1)

/* 2 MSB bits of the 10-bit Clock Divider */
#define EZI2Cs_CLKDIV2_REG    (*(reg8 *) EZI2Cs_I2C_Prim__CLK_DIV2)
#define EZI2Cs_CLKDIV2_PTR    ( (reg8 *) EZI2Cs_I2C_Prim__CLK_DIV2)

/* Power System Control Register 1 */
#define EZI2Cs_PWRSYS_CR1_REG (*(reg8 *) CYREG_PWRSYS_CR1)
#define EZI2Cs_PWRSYS_CR1_PTR ( (reg8 *) CYREG_PWRSYS_CR1)

/* I2C operation in Active Mode */
#define EZI2Cs_PM_ACT_CFG_REG (*(reg8 *) EZI2Cs_I2C_Prim__PM_ACT_CFG)
#define EZI2Cs_PM_ACT_CFG_PTR ( (reg8 *) EZI2Cs_I2C_Prim__PM_ACT_CFG)
#define EZI2Cs_ACT_PWR_EN     ( (uint8)  EZI2Cs_I2C_Prim__PM_ACT_MSK)

/* I2C operation in Alternate Active (Standby) Mode */
#define EZI2Cs_PM_STBY_CFG_REG    (*(reg8 *) EZI2Cs_I2C_Prim__PM_STBY_CFG)
#define EZI2Cs_PM_STBY_CFG_PTR    ( (reg8 *) EZI2Cs_I2C_Prim__PM_STBY_CFG)
#define EZI2Cs_STBY_PWR_EN        ( (uint8)  EZI2Cs_I2C_Prim__PM_STBY_MSK)


/***************************************
*       Register Constants
***************************************/

/* I2C backup regulator */
#define EZI2Cs_PWRSYS_CR1_I2C_BACKUP  (0x04u)

/* Interrupt number and priority */
#define EZI2Cs_ISR_NUMBER         (EZI2Cs_isr__INTC_NUMBER)
#define EZI2Cs_ISR_PRIORITY       (EZI2Cs_isr__INTC_PRIOR_NUM)

/* Block reset constants */
#define EZI2Cs_CLEAR_REG          (0x00u)
#define EZI2Cs_BLOCK_RESET_DELAY  (2u)

/* XCFG I2C Extended Configuration Register */
#define EZI2Cs_XCFG_CLK_EN        (0x80u) /* Clock enable */
#define EZI2Cs_XCFG_I2C_ON        (0x40u) /* Set before entering sleep mode */
#define EZI2Cs_XCFG_SLEEP_READY   (0x20u) /* Ready to sleep */
#define EZI2Cs_XCFG_FORCE_NACK    (0x10u) /* Force nack */
#define EZI2Cs_XCFG_HDWR_ADDR_EN  (0x01u) /* Hardware address comparison */

/* Data I2C Slave Data Register */
#define EZI2Cs_SADDR_MASK         (0x7Fu)
#define EZI2Cs_DATA_MASK          (0xFFu)
#define EZI2Cs_READ_FLAG          (0x01u)

/* CFG I2C Configuration Register */
#define EZI2Cs_CFG_SIO_SELECT    (0x80u) /* Pin Select for SCL/SDA lines */
#define EZI2Cs_CFG_PSELECT       (0x40u) /* Pin Select */
#define EZI2Cs_CFG_BUS_ERR_IE    (0x20u) /* Bus Error Interrupt Enable */
#define EZI2Cs_CFG_STOP_ERR_IE   (0x10u) /* Enable Interrupt on STOP condition */
#define EZI2Cs_CFG_STOP_IE       (0x10u) /* Enable Interrupt on STOP condition */
#define EZI2Cs_CFG_CLK_RATE      (0x04u) /* Clock rate mask. 1 for 50K, 0 for 100K and 400K */
#define EZI2Cs_CFG_EN_SLAVE      (0x01u) /* Enable Slave operation */

/* CSR I2C Control and Status Register */
#define EZI2Cs_CSR_BUS_ERROR     (0x80u) /* Active high when bus error has occurred */
#define EZI2Cs_CSR_LOST_ARB      (0x40u) /* Set to 1 if lost arbitration in host mode */
#define EZI2Cs_CSR_STOP_STATUS   (0x20u) /* Set to 1 if Stop has been detected */
#define EZI2Cs_CSR_ACK           (0x10u) /* ACK response */
#define EZI2Cs_CSR_NAK           (0x00u) /* NAK response */
#define EZI2Cs_CSR_LRB_ACK       (0x00u) /* Last received bit was an ACK */
#define EZI2Cs_CSR_ADDRESS       (0x08u) /* Set in firmware 0 = status bit, 1 Address is slave */
#define EZI2Cs_CSR_TRANSMIT      (0x04u) /* Set in firmware 1 = transmit, 0 = receive */
#define EZI2Cs_CSR_LRB           (0x02u) /* Last received bit */
#define EZI2Cs_CSR_LRB_NAK       (0x02u) /* Last received bit was an NAK */
#define EZI2Cs_CSR_BYTE_COMPLETE (0x01u) /* Informs that last byte has been sent */

/* I2C state machine constants */
#define  EZI2Cs_SM_IDLE              (0x00u) /* Wait for Start */

/* Primary slave address states */
#define  EZI2Cs_SM_DEV1_WR_ADDR      (0x01u) /* Wait for sub-address */
#define  EZI2Cs_SM_DEV1_WR_ADDR_MSB  (0x01u) /* Wait for sub-address MSB */
#define  EZI2Cs_SM_DEV1_WR_ADDR_LSB  (0x02u) /* Wait for sub-address LSB */
#define  EZI2Cs_SM_DEV1_WR_DATA      (0x04u) /* Get data from Master */
#define  EZI2Cs_SM_DEV1_RD_DATA      (0x08u) /* Send data to Master */

/* Secondary slave address states */
#define  EZI2Cs_SM_DEV2_WR_ADDR      (0x11u) /* Wait for sub-address */
#define  EZI2Cs_SM_DEV2_WR_ADDR_MSB  (0x11u) /* Wait for sub-address MSB */
#define  EZI2Cs_SM_DEV2_WR_ADDR_LSB  (0x12u) /* Wait for sub-address LSB */
#define  EZI2Cs_SM_DEV2_WR_DATA      (0x14u) /* Get data from Master */
#define  EZI2Cs_SM_DEV2_RD_DATA      (0x18u) /* Send data to Master */


/***************************************
*    Initialization Register Settings
***************************************/

/* Oversampling rate for data date 50kpbs and less is 32, 16 for others */
#define EZI2Cs_OVS_32_LIM      (50u)
#define EZI2Cs_OVERSAMPLE_RATE ((EZI2Cs_DATA_RATE <= EZI2Cs_OVS_32_LIM) ? (32u) : (16u))

/* Return default bits depends on configuration */
#define EZI2Cs_DEFAULT_HDWR_ADDR  ((EZI2Cs_ONE_ADDRESS == EZI2Cs_ADDRESSES) ? (1u) : (0u))

#define EZI2Cs_DEFAULT_PSELECT    ((EZI2Cs_ANY  != EZI2Cs_BUS_PORT) ? (1u) : (0u))
#define EZI2Cs_DEFAULT_SIO_SELECT ((EZI2Cs_I2C1 == EZI2Cs_BUS_PORT) ? (1u) : (0u))
#define EZI2Cs_DEFUALT_CLK_RATE   ((EZI2Cs_DATA_RATE <= EZI2Cs_OVS_32_LIM) ? (1u) : (0u))

/* Return bits within registers position */
#define EZI2Cs_GET_XCFG_HDWR_ADDR_EN(hwAddr) ((0u != (hwAddr)) ? (EZI2Cs_XCFG_HDWR_ADDR_EN) : (0u))
#define EZI2Cs_GET_XCFG_I2C_ON(wakeup)       ((0u != (wakeup)) ? (EZI2Cs_XCFG_I2C_ON) : (0u))

#define EZI2Cs_GET_CFG_PSELECT(pSel)      ((0u != (pSel)) ? (EZI2Cs_CFG_PSELECT) : (0u))
#define EZI2Cs_GET_CFG_SIO_SELECT(sioSel) ((0u != (sioSel)) ? (EZI2Cs_CFG_SIO_SELECT) : (0u))
#define EZI2Cs_GET_CFG_CLK_RATE(clkRate)  ((0u != (clkRate)) ? (EZI2Cs_CFG_CLK_RATE) : (0u))

/* Initial registers settings */
#define EZI2Cs_DEFAULT_CFG    (EZI2Cs_GET_CFG_SIO_SELECT (EZI2Cs_DEFAULT_SIO_SELECT) | \
                                         EZI2Cs_GET_CFG_PSELECT(EZI2Cs_DEFAULT_PSELECT)        | \
                                         EZI2Cs_GET_CFG_CLK_RATE(EZI2Cs_DEFUALT_CLK_RATE)      | \
                                         EZI2Cs_CFG_EN_SLAVE)

#define EZI2Cs_DEFAULT_XCFG   (EZI2Cs_GET_XCFG_HDWR_ADDR_EN(EZI2Cs_DEFAULT_HDWR_ADDR) | \
                                         EZI2Cs_GET_XCFG_I2C_ON(EZI2Cs_ENABLE_WAKEUP)           | \
                                         EZI2Cs_XCFG_CLK_EN)

#define EZI2Cs_DEFAULT_ADDR   (EZI2Cs_DEFAULT_ADDR1)

/* Divide factor calculation */
#define EZI2Cs_DIVIDE_FACTOR_WITH_FRACT_BYTE \
    (((uint32) BCLK__BUS_CLK__KHZ << 8u) / ((uint32) EZI2Cs_DATA_RATE * EZI2Cs_OVERSAMPLE_RATE))

#define EZI2Cs_DEFAULT_DIVIDE_FACTOR  (((uint32) EZI2Cs_DIVIDE_FACTOR_WITH_FRACT_BYTE) >> 8u)

#define EZI2Cs_DEFAULT_CLKDIV1    LO8(EZI2Cs_DEFAULT_DIVIDE_FACTOR)
#define EZI2Cs_DEFAULT_CLKDIV2    HI8(EZI2Cs_DEFAULT_DIVIDE_FACTOR)


/***************************************
* The following code is DEPRECATED and
* should not be used in new projects.
***************************************/

#define EZI2Cs_BUS_SPEED_50KHZ    (50u)
#define EZI2Cs_BUS_SPEED          EZI2Cs_DATA_RATE
#define EZI2Cs_OVER_SAMPLE_RATE   EZI2Cs_OVERSAMPLE_RATE

#define EZI2Cs_I2C_MASTER_MASK    (0xDDu)

#define EZI2Cs__ANY    EZI2Cs_ANY
#define EZI2Cs__I2C0   EZI2Cs_I2C0
#define EZI2Cs__I2C1   EZI2Cs_I2C1

#define EZI2Cs_DIVIDE_FACTOR  EZI2Cs_DEFAULT_DIVIDE_FACTOR

#if (EZI2Cs_ONE_ADDRESS == EZI2Cs_ADDRESSES)
    void EZI2Cs_SlaveSetSleepMode(void)   ;
    void EZI2Cs_SlaveSetWakeMode(void)    ;
#endif /* (EZI2Cs_ONE_ADDRESS == EZI2Cs_ADDRESSES) */

#define EZI2Cs_State          EZI2Cs_curState
#define EZI2Cs_Status         EZI2Cs_curStatus
#define EZI2Cs_DataPtr        EZI2Cs_dataPtrS1

#define EZI2Cs_RwOffset1      EZI2Cs_rwOffsetS1
#define EZI2Cs_RwIndex1       EZI2Cs_rwIndexS1
#define EZI2Cs_WrProtect1     EZI2Cs_wrProtectS1
#define EZI2Cs_BufSize1       EZI2Cs_bufSizeS1

#if (EZI2Cs_TWO_ADDRESSES == EZI2Cs_ADDRESSES)
    #define EZI2Cs_DataPtr2   EZI2Cs_dataPtrS2
    #define EZI2Cs_Address1   EZI2Cs_addrS1
    #define EZI2Cs_Address2   EZI2Cs_addrS2

    #define EZI2Cs_RwOffset2  EZI2Cs_rwOffsetS2
    #define EZI2Cs_RwIndex2   EZI2Cs_rwIndexS2
    #define EZI2Cs_WrProtect2 EZI2Cs_wrProtectS2
    #define EZI2Cs_BufSize2   EZI2Cs_bufSizeS2
#endif /* (EZI2Cs_TWO_ADDRESSES == EZI2Cs_ADDRESSES) */

#endif /* CY_EZI2C_EZI2Cs_H */


/* [] END OF FILE */
