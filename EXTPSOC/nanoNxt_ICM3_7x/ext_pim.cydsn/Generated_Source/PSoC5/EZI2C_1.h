/*******************************************************************************
* File Name: EZI2C_1.h
* Version 1.90
*
* Description:
*  This is the header file for the EzI2C user module.  It contains function
*  prototypes and constants for the users convenience.
*
********************************************************************************
* Copyright 2008-2013, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_EZI2C_EZI2C_1_H)
#define CY_EZI2C_EZI2C_1_H

#include "cyfitter.h"
#include "cytypes.h"
#include "CyLib.h"

/* Check if required defines such as CY_PSOC5LP are available in cy_boot */
#if !defined (CY_PSOC5LP)
    #error Component EZI2C_v1_90 requires cy_boot v3.0 or later
#endif /* (CY_PSOC5LP) */


/***************************************
*   Conditional Compilation Parameters
***************************************/

#define EZI2C_1_ADDRESSES         (1u)
#define EZI2C_1_ONE_ADDRESS       (0x01u)
#define EZI2C_1_TWO_ADDRESSES     (0x02u)
#define EZI2C_1_ENABLE_WAKEUP     (0u)

/* Wakeup enabled */
#define EZI2C_1_WAKEUP_ENABLED     (0u != EZI2C_1_ENABLE_WAKEUP)


/* Enumerated type*/ 
#define EZI2C_1__ANY 0
#define EZI2C_1__I2C0 1
#define EZI2C_1__I2C1 2


/***************************************
*       Type Definitions
***************************************/

/* Structure to save registers before go to sleep */
typedef struct 
{
    uint8   enableState;

    uint8   xcfg;
    uint8   adr;
    uint8   cfg;
       
    uint8   clkDiv1;
    uint8   clkDiv2;
        
}   EZI2C_1_BACKUP_STRUCT;


/***************************************
*        Function Prototypes
***************************************/

void    EZI2C_1_Start(void) ;
void    EZI2C_1_Stop(void) ;
void    EZI2C_1_EnableInt(void) ;
void    EZI2C_1_DisableInt(void) ;

void    EZI2C_1_SetAddress1(uint8 address) ;
uint8   EZI2C_1_GetAddress1(void) ;
void    EZI2C_1_SetBuffer1(unsigned short bufSize, unsigned short rwBoundry, volatile uint8 * dataPtr) ;
uint8   EZI2C_1_GetActivity(void) ;
void    EZI2C_1_Init(void) ;
void    EZI2C_1_Enable(void) ;
void    EZI2C_1_Sleep(void) ;
void    EZI2C_1_Wakeup(void) ;
void    EZI2C_1_SaveConfig(void) ;
void    EZI2C_1_RestoreConfig(void) ;


#if(EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES)

    void    EZI2C_1_SetAddress2(uint8 address ) ;
    uint8   EZI2C_1_GetAddress2(void) ;
    void    EZI2C_1_SetBuffer2(unsigned short bufSize, unsigned short rwBoundry, volatile uint8 * dataPtr) ;

#endif  /* (EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES) */


/* EZI2C interrupt handler */
CY_ISR_PROTO(EZI2C_1_ISR);
#if(EZI2C_1_WAKEUP_ENABLED)
    CY_ISR_PROTO(EZI2C_1_WAKEUP_ISR);
#endif /* (EZI2C_1_WAKEUP_ENABLED) */

/***************************************
*     Vars with External Linkage
***************************************/

extern uint8 EZI2C_1_initVar;


/***************************************
*   Initial Parameter Constants
***************************************/

#define EZI2C_1_DEFAULT_ADDR1      (8u)
#define EZI2C_1_DEFAULT_ADDR2      (9u)

#define EZI2C_1_BUS_SPEED          (100u)
#define EZI2C_1_SUBADDR_WIDTH      (0u)
#define EZI2C_1_SUBADDR_8BIT       (0x00u) /* 8-bit sub-address width */
#define EZI2C_1_SUBADDR_16BIT      (0x01u) /* 16-bit sub-address width */
#define EZI2C_1_BUS_PORT           (0u)


/***************************************
*              API Constants
***************************************/

/* Status bit definition */

/* A read addr 1 operation occured since last status check */
#define EZI2C_1_STATUS_READ1       (0x01u)

/* A Write addr 1 opereation occured since last status check */
#define EZI2C_1_STATUS_WRITE1      (0x02u)

/* A read addr 2 operation occured since last status check */
#define EZI2C_1_STATUS_READ2       (0x04u)

/* A Write addr 2 opereation occured since last status check */
#define EZI2C_1_STATUS_WRITE2      (0x08u)

/* A start has occured, but a Stop has not been detected */
#define EZI2C_1_STATUS_BUSY        (0x10u)

/* Addr 1 read busy */
#define EZI2C_1_STATUS_RD1BUSY     (0x11u)

/* Addr 1 write busy */
#define EZI2C_1_STATUS_WR1BUSY     (0x12u)

/* Addr 2 read busy */
#define EZI2C_1_STATUS_RD2BUSY     (0x14u)

/* Addr 2 write busy */
#define EZI2C_1_STATUS_WR2BUSY     (0x18u)

/* Mask for status bits. */
#define EZI2C_1_STATUS_MASK        (0x1Fu)

/* An Error occured since last read */
#define EZI2C_1_STATUS_ERR         (0x80u)

/* Dummy data to be sent to master */
#define EZI2C_1_DUMMY_DATA         (0xFFu)

/* The I2C Master bits in I2C cinfiguration register */
#define EZI2C_1_I2C_MASTER_MASK    (0xDDu)

/* Component's enable/disable state */
#define EZI2C_1_ENABLED            (0x01u)
#define EZI2C_1_DISABLED           (0x00u)

#define EZI2C_1_BUS_SPEED_50KHZ      (50u)

/* Bus speed grater 50kHz requires 16 oversample rate */
#if (EZI2C_1_BUS_SPEED <= EZI2C_1_BUS_SPEED_50KHZ)

	#define EZI2C_1_OVER_SAMPLE_RATE       (32u)

#else

	#define EZI2C_1_OVER_SAMPLE_RATE       (16u)

#endif  /* End (EZI2C_1_BUS_SPEED <= EZI2C_1_BUS_SPEED_50KHZ) */

/* Divide factor calculation */
#define EZI2C_1_DIVIDE_FACTOR_WITH_FRACT_BYTE  \
                (((uint32) BCLK__BUS_CLK__KHZ << 8u) / ((uint32)EZI2C_1_BUS_SPEED * \
                EZI2C_1_OVER_SAMPLE_RATE))
                
#define EZI2C_1_DIVIDE_FACTOR  (((uint32) EZI2C_1_DIVIDE_FACTOR_WITH_FRACT_BYTE) >> 8u)


/* Returns 1 if corresponding bit is set, otherwise 0 */
#define EZI2C_1_IS_BIT_SET(value, mask) (((mask) == ((value) & (mask))) ? 0x01u : 0x00u)

#define EZI2C_1_ADDRESS_SHIFT      (1u)
#define EZI2C_1_ADDRESS_LSB_SHIFT  (8u)


/***************************************
*              Registers
***************************************/

/* I2C Extended Configuration Register */
#define EZI2C_1_XCFG_REG       (* (reg8 *) EZI2C_1_I2C_Prim__XCFG )
#define EZI2C_1_XCFG_PTR       (  (reg8 *) EZI2C_1_I2C_Prim__XCFG )

/* I2C Slave Adddress Register */
#define EZI2C_1_ADDR_REG       (* (reg8 *) EZI2C_1_I2C_Prim__ADR )
#define EZI2C_1_ADDR_PTR       (  (reg8 *) EZI2C_1_I2C_Prim__ADR )

/* I2C Configuration Register */
#define EZI2C_1_CFG_REG        (* (reg8 *) EZI2C_1_I2C_Prim__CFG )
#define EZI2C_1_CFG_PTR        (  (reg8 *) EZI2C_1_I2C_Prim__CFG )

/* I2C Control and Status Register */
#define EZI2C_1_CSR_REG        (* (reg8 *) EZI2C_1_I2C_Prim__CSR )
#define EZI2C_1_CSR_PTR        (  (reg8 *) EZI2C_1_I2C_Prim__CSR )

/* I2C Data Register */
#define EZI2C_1_DATA_REG       (* (reg8 *) EZI2C_1_I2C_Prim__D )
#define EZI2C_1_DATA_PTR       (  (reg8 *) EZI2C_1_I2C_Prim__D )

/*  8 LSB bits of the 10-bit Clock Divider */
#define EZI2C_1_CLKDIV1_REG        (* (reg8 *) EZI2C_1_I2C_Prim__CLK_DIV1 )
#define EZI2C_1_CLKDIV1_PTR        (  (reg8 *) EZI2C_1_I2C_Prim__CLK_DIV1 )

/* 2 MSB bits of the 10-bit Clock Divider */
#define EZI2C_1_CLKDIV2_REG        (* (reg8 *) EZI2C_1_I2C_Prim__CLK_DIV2 )
#define EZI2C_1_CLKDIV2_PTR        (  (reg8 *) EZI2C_1_I2C_Prim__CLK_DIV2 )

/* Power System Control Register 1 */
#define EZI2C_1_PWRSYS_CR1_REG     (* (reg8 *) CYREG_PWRSYS_CR1 )
#define EZI2C_1_PWRSYS_CR1_PTR     (  (reg8 *) CYREG_PWRSYS_CR1 )

/* I2C operation in Active Mode */
#define EZI2C_1_PM_ACT_CFG_REG     (* (reg8 *) EZI2C_1_I2C_Prim__PM_ACT_CFG )
#define EZI2C_1_PM_ACT_CFG_PTR     (  (reg8 *) EZI2C_1_I2C_Prim__PM_ACT_CFG )

/* I2C operation in Alternate Active (Standby) Mode */
#define EZI2C_1_PM_STBY_CFG_REG    (* (reg8 *) EZI2C_1_I2C_Prim__PM_STBY_CFG )
#define EZI2C_1_PM_STBY_CFG_PTR    (  (reg8 *) EZI2C_1_I2C_Prim__PM_STBY_CFG )


/***************************************
*       Register Constants
***************************************/

/* XCFG I2C Extended Configuration Register */
#define EZI2C_1_XCFG_CLK_EN            (0x80u)
#define EZI2C_1_XCFG_HDWR_ADDR_EN      (0x01u)

/* Force nack */
#define EZI2C_1_XCFG_FORCE_NACK        (0x10u)

/* Ready to sleep */
#define EZI2C_1_XCFG_SLEEP_READY       (0x20u)

/* Should be set before entering sleep mode */
#define EZI2C_1_XCFG_I2C_ON            (0x40u)

/* Enables the I2C regulator backup */
#define EZI2C_1_PWRSYS_CR1_I2C_BACKUP  (0x04u)

/* Data I2C Slave Data Register */
#define EZI2C_1_SADDR_MASK        (0x7Fu)
#define EZI2C_1_DATA_MASK         (0xFFu)
#define EZI2C_1_READ_FLAG         (0x01u)

/* CFG I2C Configuration Register */

/* Pin Select for SCL/SDA lines */
#define EZI2C_1_CFG_SIO_SELECT    (0x80u)

/* Pin Select */
#define EZI2C_1_CFG_PSELECT       (0x40u)

/* Bus Error Interrupt Enable */
#define EZI2C_1_CFG_BUS_ERR_IE    (0x20u)

/* Enable Interrupt on STOP condition */
#define EZI2C_1_CFG_STOP_IE       (0x10u)

/* Enable Interrupt on STOP condition */
#define EZI2C_1_CFG_STOP_ERR_IE   (0x10u)

/* Clock rate mask. 1 for 50K, 0 for 100K and 400K */
#define EZI2C_1_CFG_CLK_RATE      (0x04u)


/* Enable Slave operation */
#define EZI2C_1_CFG_EN_SLAVE      (0x01u)

/* CSR I2C Control and Status Register */

/* Active high when bus error has occured */
#define EZI2C_1_CSR_BUS_ERROR     (0x80u)

/* Set to 1 if lost arbitration in host mode */
#define EZI2C_1_CSR_LOST_ARB      (0x40u)

/* Set to 1 if Stop has been detected */
#define EZI2C_1_CSR_STOP_STATUS   (0x20u)

/* ACK response */
#define EZI2C_1_CSR_ACK           (0x10u)

/* NAK response */
#define EZI2C_1_CSR_NAK           (0x00u)

/* Set in firmware 0 = status bit, 1 Address is slave */
#define EZI2C_1_CSR_ADDRESS       (0x08u)

/* Set in firmware 1 = transmit, 0 = receive. */
#define EZI2C_1_CSR_TRANSMIT      (0x04u)

/* Last received bit. */
#define EZI2C_1_CSR_LRB           (0x02u)

 /* Last received bit was an ACK */
#define EZI2C_1_CSR_LRB_ACK       (0x00u)

/* Last received bit was an NAK */
#define EZI2C_1_CSR_LRB_NAK       (0x02u)

/* Informs that last byte has been sent. */
#define EZI2C_1_CSR_BYTE_COMPLETE (0x01u)

/* CLK_DIV I2C Clock Divide Factor Register */

/* Status bit, Set at Start and cleared at Stop condition */
#define EZI2C_1_CLK_DIV_MSK       (0x07u)

/* Divide input clock by  1 */
#define EZI2C_1_CLK_DIV_1         (0x00u)

/* Divide input clock by  2 */
#define EZI2C_1_CLK_DIV_2         (0x01u)

/* Divide input clock by  4 */
#define EZI2C_1_CLK_DIV_4         (0x02u)

/* Divide input clock by  8 */
#define EZI2C_1_CLK_DIV_8         (0x03u)

/* Divide input clock by 16 */
#define EZI2C_1_CLK_DIV_16        (0x04u)

/* Divide input clock by 32 */
#define EZI2C_1_CLK_DIV_32        (0x05u)

/* Divide input clock by 64 */
#define EZI2C_1_CLK_DIV_64        (0x06u)

/* Active Power Mode CFG Register - power enable mask */
#define EZI2C_1_ACT_PWR_EN    EZI2C_1_I2C_Prim__PM_ACT_MSK

/* Alternate Active (Standby) Power Mode CFG Register - power enable mask */
#define EZI2C_1_STBY_PWR_EN    EZI2C_1_I2C_Prim__PM_STBY_MSK

/* Number of the EZI2C_1_isr interrupt. */
#define EZI2C_1_ISR_NUMBER    EZI2C_1_isr__INTC_NUMBER

/* Priority of the EZI2C_1_isr interrupt. */
#define EZI2C_1_ISR_PRIORITY  EZI2C_1_isr__INTC_PRIOR_NUM

/* I2C state machine constants */

/* Wait for Start */
#define  EZI2C_1_SM_IDLE              (0x00u)

/* Default address states */

/* Wait for sub-address */
#define  EZI2C_1_SM_DEV1_WR_ADDR      (0x01u)

/* Wait for sub-address MSB */
#define  EZI2C_1_SM_DEV1_WR_ADDR_MSB  (0x01u)

/* Wait for sub-address LSB */
#define  EZI2C_1_SM_DEV1_WR_ADDR_LSB  (0x02u)

/* Get data from Master */
#define  EZI2C_1_SM_DEV1_WR_DATA      (0x04u)

/* Send data to Master */
#define  EZI2C_1_SM_DEV1_RD_DATA      (0x08u)

/* Second address states */

/* Wait for sub-address */
#define  EZI2C_1_SM_DEV2_WR_ADDR      (0x11u)

/* Wait for sub-address MSB */
#define  EZI2C_1_SM_DEV2_WR_ADDR_MSB  (0x11u)

/* Wait for sub-address LSB */
#define  EZI2C_1_SM_DEV2_WR_ADDR_LSB  (0x12u)

/* Get data from Master */
#define  EZI2C_1_SM_DEV2_WR_DATA      (0x14u)

/* Send data to Master */
#define  EZI2C_1_SM_DEV2_RD_DATA      (0x18u)


/***************************************
*       Obsolete
***************************************/

/* Following definitions are OBSOLETE and must not be used */

#if(EZI2C_1_ADDRESSES == EZI2C_1_ONE_ADDRESS)

    void    EZI2C_1_SlaveSetSleepMode(void) ;
    void    EZI2C_1_SlaveSetWakeMode(void) ;

#endif /* (EZI2C_1_ADDRESSES == EZI2C_1_ONE_ADDRESS)*/

#define EZI2C_1_State          EZI2C_1_curState
#define EZI2C_1_Status         EZI2C_1_curStatus
#define EZI2C_1_DataPtr        EZI2C_1_dataPtrS1

#define EZI2C_1_RwOffset1      EZI2C_1_rwOffsetS1
#define EZI2C_1_RwIndex1       EZI2C_1_rwIndexS1
#define EZI2C_1_WrProtect1     EZI2C_1_wrProtectS1
#define EZI2C_1_BufSize1       EZI2C_1_bufSizeS1

#if(EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES)

    #define EZI2C_1_DataPtr2   EZI2C_1_dataPtrS2
    #define EZI2C_1_Address1   EZI2C_1_addrS1
    #define EZI2C_1_Address2   EZI2C_1_addrS2

    #define EZI2C_1_RwOffset2  EZI2C_1_rwOffsetS2
    #define EZI2C_1_RwIndex2   EZI2C_1_rwIndexS2
    #define EZI2C_1_WrProtect2 EZI2C_1_wrProtectS2
    #define EZI2C_1_BufSize2   EZI2C_1_bufSizeS2

#endif /* EZI2C_1_ADDRESSES == EZI2C_1_TWO_ADDRESSES */


#endif /* CY_EZI2C_EZI2C_1_H */


/* [] END OF FILE */
