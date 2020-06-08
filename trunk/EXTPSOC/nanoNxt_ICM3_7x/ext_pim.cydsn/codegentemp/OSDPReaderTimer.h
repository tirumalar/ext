/*******************************************************************************
* File Name: OSDPReaderTimer.h
* Version 2.80
*
*  Description:
*     Contains the function prototypes and constants available to the timer
*     user module.
*
*   Note:
*     None
*
********************************************************************************
* Copyright 2008-2017, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
********************************************************************************/

#if !defined(CY_TIMER_OSDPReaderTimer_H)
#define CY_TIMER_OSDPReaderTimer_H

#include "cytypes.h"
#include "cyfitter.h"
#include "CyLib.h" /* For CyEnterCriticalSection() and CyExitCriticalSection() functions */

extern uint8 OSDPReaderTimer_initVar;

/* Check to see if required defines such as CY_PSOC5LP are available */
/* They are defined starting with cy_boot v3.0 */
#if !defined (CY_PSOC5LP)
    #error Component Timer_v2_80 requires cy_boot v3.0 or later
#endif /* (CY_ PSOC5LP) */


/**************************************
*           Parameter Defaults
**************************************/

#define OSDPReaderTimer_Resolution                 16u
#define OSDPReaderTimer_UsingFixedFunction         0u
#define OSDPReaderTimer_UsingHWCaptureCounter      0u
#define OSDPReaderTimer_SoftwareCaptureMode        0u
#define OSDPReaderTimer_SoftwareTriggerMode        0u
#define OSDPReaderTimer_UsingHWEnable              0u
#define OSDPReaderTimer_EnableTriggerMode          0u
#define OSDPReaderTimer_InterruptOnCaptureCount    0u
#define OSDPReaderTimer_RunModeUsed                0u
#define OSDPReaderTimer_ControlRegRemoved          0u

#if defined(OSDPReaderTimer_TimerUDB_sCTRLReg_SyncCtl_ctrlreg__CONTROL_REG)
    #define OSDPReaderTimer_UDB_CONTROL_REG_REMOVED            (0u)
#elif  (OSDPReaderTimer_UsingFixedFunction)
    #define OSDPReaderTimer_UDB_CONTROL_REG_REMOVED            (0u)
#else 
    #define OSDPReaderTimer_UDB_CONTROL_REG_REMOVED            (1u)
#endif /* End OSDPReaderTimer_TimerUDB_sCTRLReg_SyncCtl_ctrlreg__CONTROL_REG */


/***************************************
*       Type defines
***************************************/


/**************************************************************************
 * Sleep Wakeup Backup structure for Timer Component
 *************************************************************************/
typedef struct
{
    uint8 TimerEnableState;
    #if(!OSDPReaderTimer_UsingFixedFunction)

        uint16 TimerUdb;
        uint8 InterruptMaskValue;
        #if (OSDPReaderTimer_UsingHWCaptureCounter)
            uint8 TimerCaptureCounter;
        #endif /* variable declarations for backing up non retention registers in CY_UDB_V1 */

        #if (!OSDPReaderTimer_UDB_CONTROL_REG_REMOVED)
            uint8 TimerControlRegister;
        #endif /* variable declaration for backing up enable state of the Timer */
    #endif /* define backup variables only for UDB implementation. Fixed function registers are all retention */

}OSDPReaderTimer_backupStruct;


/***************************************
*       Function Prototypes
***************************************/

void    OSDPReaderTimer_Start(void) ;
void    OSDPReaderTimer_Stop(void) ;

void    OSDPReaderTimer_SetInterruptMode(uint8 interruptMode) ;
uint8   OSDPReaderTimer_ReadStatusRegister(void) ;
/* Deprecated function. Do not use this in future. Retained for backward compatibility */
#define OSDPReaderTimer_GetInterruptSource() OSDPReaderTimer_ReadStatusRegister()

#if(!OSDPReaderTimer_UDB_CONTROL_REG_REMOVED)
    uint8   OSDPReaderTimer_ReadControlRegister(void) ;
    void    OSDPReaderTimer_WriteControlRegister(uint8 control) ;
#endif /* (!OSDPReaderTimer_UDB_CONTROL_REG_REMOVED) */

uint16  OSDPReaderTimer_ReadPeriod(void) ;
void    OSDPReaderTimer_WritePeriod(uint16 period) ;
uint16  OSDPReaderTimer_ReadCounter(void) ;
void    OSDPReaderTimer_WriteCounter(uint16 counter) ;
uint16  OSDPReaderTimer_ReadCapture(void) ;
void    OSDPReaderTimer_SoftwareCapture(void) ;

#if(!OSDPReaderTimer_UsingFixedFunction) /* UDB Prototypes */
    #if (OSDPReaderTimer_SoftwareCaptureMode)
        void    OSDPReaderTimer_SetCaptureMode(uint8 captureMode) ;
    #endif /* (!OSDPReaderTimer_UsingFixedFunction) */

    #if (OSDPReaderTimer_SoftwareTriggerMode)
        void    OSDPReaderTimer_SetTriggerMode(uint8 triggerMode) ;
    #endif /* (OSDPReaderTimer_SoftwareTriggerMode) */

    #if (OSDPReaderTimer_EnableTriggerMode)
        void    OSDPReaderTimer_EnableTrigger(void) ;
        void    OSDPReaderTimer_DisableTrigger(void) ;
    #endif /* (OSDPReaderTimer_EnableTriggerMode) */


    #if(OSDPReaderTimer_InterruptOnCaptureCount)
        void    OSDPReaderTimer_SetInterruptCount(uint8 interruptCount) ;
    #endif /* (OSDPReaderTimer_InterruptOnCaptureCount) */

    #if (OSDPReaderTimer_UsingHWCaptureCounter)
        void    OSDPReaderTimer_SetCaptureCount(uint8 captureCount) ;
        uint8   OSDPReaderTimer_ReadCaptureCount(void) ;
    #endif /* (OSDPReaderTimer_UsingHWCaptureCounter) */

    void OSDPReaderTimer_ClearFIFO(void) ;
#endif /* UDB Prototypes */

/* Sleep Retention APIs */
void OSDPReaderTimer_Init(void)          ;
void OSDPReaderTimer_Enable(void)        ;
void OSDPReaderTimer_SaveConfig(void)    ;
void OSDPReaderTimer_RestoreConfig(void) ;
void OSDPReaderTimer_Sleep(void)         ;
void OSDPReaderTimer_Wakeup(void)        ;


/***************************************
*   Enumerated Types and Parameters
***************************************/

/* Enumerated Type B_Timer__CaptureModes, Used in Capture Mode */
#define OSDPReaderTimer__B_TIMER__CM_NONE 0
#define OSDPReaderTimer__B_TIMER__CM_RISINGEDGE 1
#define OSDPReaderTimer__B_TIMER__CM_FALLINGEDGE 2
#define OSDPReaderTimer__B_TIMER__CM_EITHEREDGE 3
#define OSDPReaderTimer__B_TIMER__CM_SOFTWARE 4



/* Enumerated Type B_Timer__TriggerModes, Used in Trigger Mode */
#define OSDPReaderTimer__B_TIMER__TM_NONE 0x00u
#define OSDPReaderTimer__B_TIMER__TM_RISINGEDGE 0x04u
#define OSDPReaderTimer__B_TIMER__TM_FALLINGEDGE 0x08u
#define OSDPReaderTimer__B_TIMER__TM_EITHEREDGE 0x0Cu
#define OSDPReaderTimer__B_TIMER__TM_SOFTWARE 0x10u


/***************************************
*    Initialial Parameter Constants
***************************************/

#define OSDPReaderTimer_INIT_PERIOD             3599u
#define OSDPReaderTimer_INIT_CAPTURE_MODE       ((uint8)((uint8)0u << OSDPReaderTimer_CTRL_CAP_MODE_SHIFT))
#define OSDPReaderTimer_INIT_TRIGGER_MODE       ((uint8)((uint8)0u << OSDPReaderTimer_CTRL_TRIG_MODE_SHIFT))
#if (OSDPReaderTimer_UsingFixedFunction)
    #define OSDPReaderTimer_INIT_INTERRUPT_MODE (((uint8)((uint8)1u << OSDPReaderTimer_STATUS_TC_INT_MASK_SHIFT)) | \
                                                  ((uint8)((uint8)0 << OSDPReaderTimer_STATUS_CAPTURE_INT_MASK_SHIFT)))
#else
    #define OSDPReaderTimer_INIT_INTERRUPT_MODE (((uint8)((uint8)1u << OSDPReaderTimer_STATUS_TC_INT_MASK_SHIFT)) | \
                                                 ((uint8)((uint8)0 << OSDPReaderTimer_STATUS_CAPTURE_INT_MASK_SHIFT)) | \
                                                 ((uint8)((uint8)0 << OSDPReaderTimer_STATUS_FIFOFULL_INT_MASK_SHIFT)))
#endif /* (OSDPReaderTimer_UsingFixedFunction) */
#define OSDPReaderTimer_INIT_CAPTURE_COUNT      (2u)
#define OSDPReaderTimer_INIT_INT_CAPTURE_COUNT  ((uint8)((uint8)(1u - 1u) << OSDPReaderTimer_CTRL_INTCNT_SHIFT))


/***************************************
*           Registers
***************************************/

#if (OSDPReaderTimer_UsingFixedFunction) /* Implementation Specific Registers and Register Constants */


    /***************************************
    *    Fixed Function Registers
    ***************************************/

    #define OSDPReaderTimer_STATUS         (*(reg8 *) OSDPReaderTimer_TimerHW__SR0 )
    /* In Fixed Function Block Status and Mask are the same register */
    #define OSDPReaderTimer_STATUS_MASK    (*(reg8 *) OSDPReaderTimer_TimerHW__SR0 )
    #define OSDPReaderTimer_CONTROL        (*(reg8 *) OSDPReaderTimer_TimerHW__CFG0)
    #define OSDPReaderTimer_CONTROL2       (*(reg8 *) OSDPReaderTimer_TimerHW__CFG1)
    #define OSDPReaderTimer_CONTROL2_PTR   ( (reg8 *) OSDPReaderTimer_TimerHW__CFG1)
    #define OSDPReaderTimer_RT1            (*(reg8 *) OSDPReaderTimer_TimerHW__RT1)
    #define OSDPReaderTimer_RT1_PTR        ( (reg8 *) OSDPReaderTimer_TimerHW__RT1)

    #if (CY_PSOC3 || CY_PSOC5LP)
        #define OSDPReaderTimer_CONTROL3       (*(reg8 *) OSDPReaderTimer_TimerHW__CFG2)
        #define OSDPReaderTimer_CONTROL3_PTR   ( (reg8 *) OSDPReaderTimer_TimerHW__CFG2)
    #endif /* (CY_PSOC3 || CY_PSOC5LP) */
    #define OSDPReaderTimer_GLOBAL_ENABLE  (*(reg8 *) OSDPReaderTimer_TimerHW__PM_ACT_CFG)
    #define OSDPReaderTimer_GLOBAL_STBY_ENABLE  (*(reg8 *) OSDPReaderTimer_TimerHW__PM_STBY_CFG)

    #define OSDPReaderTimer_CAPTURE_LSB         (* (reg16 *) OSDPReaderTimer_TimerHW__CAP0 )
    #define OSDPReaderTimer_CAPTURE_LSB_PTR       ((reg16 *) OSDPReaderTimer_TimerHW__CAP0 )
    #define OSDPReaderTimer_PERIOD_LSB          (* (reg16 *) OSDPReaderTimer_TimerHW__PER0 )
    #define OSDPReaderTimer_PERIOD_LSB_PTR        ((reg16 *) OSDPReaderTimer_TimerHW__PER0 )
    #define OSDPReaderTimer_COUNTER_LSB         (* (reg16 *) OSDPReaderTimer_TimerHW__CNT_CMP0 )
    #define OSDPReaderTimer_COUNTER_LSB_PTR       ((reg16 *) OSDPReaderTimer_TimerHW__CNT_CMP0 )


    /***************************************
    *    Register Constants
    ***************************************/

    /* Fixed Function Block Chosen */
    #define OSDPReaderTimer_BLOCK_EN_MASK                     OSDPReaderTimer_TimerHW__PM_ACT_MSK
    #define OSDPReaderTimer_BLOCK_STBY_EN_MASK                OSDPReaderTimer_TimerHW__PM_STBY_MSK

    /* Control Register Bit Locations */
    /* Interrupt Count - Not valid for Fixed Function Block */
    #define OSDPReaderTimer_CTRL_INTCNT_SHIFT                  0x00u
    /* Trigger Polarity - Not valid for Fixed Function Block */
    #define OSDPReaderTimer_CTRL_TRIG_MODE_SHIFT               0x00u
    /* Trigger Enable - Not valid for Fixed Function Block */
    #define OSDPReaderTimer_CTRL_TRIG_EN_SHIFT                 0x00u
    /* Capture Polarity - Not valid for Fixed Function Block */
    #define OSDPReaderTimer_CTRL_CAP_MODE_SHIFT                0x00u
    /* Timer Enable - As defined in Register Map, part of TMRX_CFG0 register */
    #define OSDPReaderTimer_CTRL_ENABLE_SHIFT                  0x00u

    /* Control Register Bit Masks */
    #define OSDPReaderTimer_CTRL_ENABLE                        ((uint8)((uint8)0x01u << OSDPReaderTimer_CTRL_ENABLE_SHIFT))

    /* Control2 Register Bit Masks */
    /* As defined in Register Map, Part of the TMRX_CFG1 register */
    #define OSDPReaderTimer_CTRL2_IRQ_SEL_SHIFT                 0x00u
    #define OSDPReaderTimer_CTRL2_IRQ_SEL                      ((uint8)((uint8)0x01u << OSDPReaderTimer_CTRL2_IRQ_SEL_SHIFT))

    #if (CY_PSOC5A)
        /* Use CFG1 Mode bits to set run mode */
        /* As defined by Verilog Implementation */
        #define OSDPReaderTimer_CTRL_MODE_SHIFT                 0x01u
        #define OSDPReaderTimer_CTRL_MODE_MASK                 ((uint8)((uint8)0x07u << OSDPReaderTimer_CTRL_MODE_SHIFT))
    #endif /* (CY_PSOC5A) */
    #if (CY_PSOC3 || CY_PSOC5LP)
        /* Control3 Register Bit Locations */
        #define OSDPReaderTimer_CTRL_RCOD_SHIFT        0x02u
        #define OSDPReaderTimer_CTRL_ENBL_SHIFT        0x00u
        #define OSDPReaderTimer_CTRL_MODE_SHIFT        0x00u

        /* Control3 Register Bit Masks */
        #define OSDPReaderTimer_CTRL_RCOD_MASK  ((uint8)((uint8)0x03u << OSDPReaderTimer_CTRL_RCOD_SHIFT)) /* ROD and COD bit masks */
        #define OSDPReaderTimer_CTRL_ENBL_MASK  ((uint8)((uint8)0x80u << OSDPReaderTimer_CTRL_ENBL_SHIFT)) /* HW_EN bit mask */
        #define OSDPReaderTimer_CTRL_MODE_MASK  ((uint8)((uint8)0x03u << OSDPReaderTimer_CTRL_MODE_SHIFT)) /* Run mode bit mask */

        #define OSDPReaderTimer_CTRL_RCOD       ((uint8)((uint8)0x03u << OSDPReaderTimer_CTRL_RCOD_SHIFT))
        #define OSDPReaderTimer_CTRL_ENBL       ((uint8)((uint8)0x80u << OSDPReaderTimer_CTRL_ENBL_SHIFT))
    #endif /* (CY_PSOC3 || CY_PSOC5LP) */

    /*RT1 Synch Constants: Applicable for PSoC3 and PSoC5LP */
    #define OSDPReaderTimer_RT1_SHIFT                       0x04u
    /* Sync TC and CMP bit masks */
    #define OSDPReaderTimer_RT1_MASK                        ((uint8)((uint8)0x03u << OSDPReaderTimer_RT1_SHIFT))
    #define OSDPReaderTimer_SYNC                            ((uint8)((uint8)0x03u << OSDPReaderTimer_RT1_SHIFT))
    #define OSDPReaderTimer_SYNCDSI_SHIFT                   0x00u
    /* Sync all DSI inputs with Mask  */
    #define OSDPReaderTimer_SYNCDSI_MASK                    ((uint8)((uint8)0x0Fu << OSDPReaderTimer_SYNCDSI_SHIFT))
    /* Sync all DSI inputs */
    #define OSDPReaderTimer_SYNCDSI_EN                      ((uint8)((uint8)0x0Fu << OSDPReaderTimer_SYNCDSI_SHIFT))

    #define OSDPReaderTimer_CTRL_MODE_PULSEWIDTH            ((uint8)((uint8)0x01u << OSDPReaderTimer_CTRL_MODE_SHIFT))
    #define OSDPReaderTimer_CTRL_MODE_PERIOD                ((uint8)((uint8)0x02u << OSDPReaderTimer_CTRL_MODE_SHIFT))
    #define OSDPReaderTimer_CTRL_MODE_CONTINUOUS            ((uint8)((uint8)0x00u << OSDPReaderTimer_CTRL_MODE_SHIFT))

    /* Status Register Bit Locations */
    /* As defined in Register Map, part of TMRX_SR0 register */
    #define OSDPReaderTimer_STATUS_TC_SHIFT                 0x07u
    /* As defined in Register Map, part of TMRX_SR0 register, Shared with Compare Status */
    #define OSDPReaderTimer_STATUS_CAPTURE_SHIFT            0x06u
    /* As defined in Register Map, part of TMRX_SR0 register */
    #define OSDPReaderTimer_STATUS_TC_INT_MASK_SHIFT        (OSDPReaderTimer_STATUS_TC_SHIFT - 0x04u)
    /* As defined in Register Map, part of TMRX_SR0 register, Shared with Compare Status */
    #define OSDPReaderTimer_STATUS_CAPTURE_INT_MASK_SHIFT   (OSDPReaderTimer_STATUS_CAPTURE_SHIFT - 0x04u)

    /* Status Register Bit Masks */
    #define OSDPReaderTimer_STATUS_TC                       ((uint8)((uint8)0x01u << OSDPReaderTimer_STATUS_TC_SHIFT))
    #define OSDPReaderTimer_STATUS_CAPTURE                  ((uint8)((uint8)0x01u << OSDPReaderTimer_STATUS_CAPTURE_SHIFT))
    /* Interrupt Enable Bit-Mask for interrupt on TC */
    #define OSDPReaderTimer_STATUS_TC_INT_MASK              ((uint8)((uint8)0x01u << OSDPReaderTimer_STATUS_TC_INT_MASK_SHIFT))
    /* Interrupt Enable Bit-Mask for interrupt on Capture */
    #define OSDPReaderTimer_STATUS_CAPTURE_INT_MASK         ((uint8)((uint8)0x01u << OSDPReaderTimer_STATUS_CAPTURE_INT_MASK_SHIFT))

#else   /* UDB Registers and Register Constants */


    /***************************************
    *           UDB Registers
    ***************************************/

    #define OSDPReaderTimer_STATUS              (* (reg8 *) OSDPReaderTimer_TimerUDB_rstSts_stsreg__STATUS_REG )
    #define OSDPReaderTimer_STATUS_MASK         (* (reg8 *) OSDPReaderTimer_TimerUDB_rstSts_stsreg__MASK_REG)
    #define OSDPReaderTimer_STATUS_AUX_CTRL     (* (reg8 *) OSDPReaderTimer_TimerUDB_rstSts_stsreg__STATUS_AUX_CTL_REG)
    #define OSDPReaderTimer_CONTROL             (* (reg8 *) OSDPReaderTimer_TimerUDB_sCTRLReg_SyncCtl_ctrlreg__CONTROL_REG )
    
    #if(OSDPReaderTimer_Resolution <= 8u) /* 8-bit Timer */
        #define OSDPReaderTimer_CAPTURE_LSB         (* (reg8 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__F0_REG )
        #define OSDPReaderTimer_CAPTURE_LSB_PTR       ((reg8 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__F0_REG )
        #define OSDPReaderTimer_PERIOD_LSB          (* (reg8 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__D0_REG )
        #define OSDPReaderTimer_PERIOD_LSB_PTR        ((reg8 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__D0_REG )
        #define OSDPReaderTimer_COUNTER_LSB         (* (reg8 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__A0_REG )
        #define OSDPReaderTimer_COUNTER_LSB_PTR       ((reg8 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__A0_REG )
    #elif(OSDPReaderTimer_Resolution <= 16u) /* 8-bit Timer */
        #if(CY_PSOC3) /* 8-bit addres space */
            #define OSDPReaderTimer_CAPTURE_LSB         (* (reg16 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__F0_REG )
            #define OSDPReaderTimer_CAPTURE_LSB_PTR       ((reg16 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__F0_REG )
            #define OSDPReaderTimer_PERIOD_LSB          (* (reg16 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__D0_REG )
            #define OSDPReaderTimer_PERIOD_LSB_PTR        ((reg16 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__D0_REG )
            #define OSDPReaderTimer_COUNTER_LSB         (* (reg16 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__A0_REG )
            #define OSDPReaderTimer_COUNTER_LSB_PTR       ((reg16 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__A0_REG )
        #else /* 16-bit address space */
            #define OSDPReaderTimer_CAPTURE_LSB         (* (reg16 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__16BIT_F0_REG )
            #define OSDPReaderTimer_CAPTURE_LSB_PTR       ((reg16 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__16BIT_F0_REG )
            #define OSDPReaderTimer_PERIOD_LSB          (* (reg16 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__16BIT_D0_REG )
            #define OSDPReaderTimer_PERIOD_LSB_PTR        ((reg16 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__16BIT_D0_REG )
            #define OSDPReaderTimer_COUNTER_LSB         (* (reg16 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__16BIT_A0_REG )
            #define OSDPReaderTimer_COUNTER_LSB_PTR       ((reg16 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__16BIT_A0_REG )
        #endif /* CY_PSOC3 */
    #elif(OSDPReaderTimer_Resolution <= 24u)/* 24-bit Timer */
        #define OSDPReaderTimer_CAPTURE_LSB         (* (reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__F0_REG )
        #define OSDPReaderTimer_CAPTURE_LSB_PTR       ((reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__F0_REG )
        #define OSDPReaderTimer_PERIOD_LSB          (* (reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__D0_REG )
        #define OSDPReaderTimer_PERIOD_LSB_PTR        ((reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__D0_REG )
        #define OSDPReaderTimer_COUNTER_LSB         (* (reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__A0_REG )
        #define OSDPReaderTimer_COUNTER_LSB_PTR       ((reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__A0_REG )
    #else /* 32-bit Timer */
        #if(CY_PSOC3 || CY_PSOC5) /* 8-bit address space */
            #define OSDPReaderTimer_CAPTURE_LSB         (* (reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__F0_REG )
            #define OSDPReaderTimer_CAPTURE_LSB_PTR       ((reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__F0_REG )
            #define OSDPReaderTimer_PERIOD_LSB          (* (reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__D0_REG )
            #define OSDPReaderTimer_PERIOD_LSB_PTR        ((reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__D0_REG )
            #define OSDPReaderTimer_COUNTER_LSB         (* (reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__A0_REG )
            #define OSDPReaderTimer_COUNTER_LSB_PTR       ((reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__A0_REG )
        #else /* 32-bit address space */
            #define OSDPReaderTimer_CAPTURE_LSB         (* (reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__32BIT_F0_REG )
            #define OSDPReaderTimer_CAPTURE_LSB_PTR       ((reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__32BIT_F0_REG )
            #define OSDPReaderTimer_PERIOD_LSB          (* (reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__32BIT_D0_REG )
            #define OSDPReaderTimer_PERIOD_LSB_PTR        ((reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__32BIT_D0_REG )
            #define OSDPReaderTimer_COUNTER_LSB         (* (reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__32BIT_A0_REG )
            #define OSDPReaderTimer_COUNTER_LSB_PTR       ((reg32 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__32BIT_A0_REG )
        #endif /* CY_PSOC3 || CY_PSOC5 */ 
    #endif

    #define OSDPReaderTimer_COUNTER_LSB_PTR_8BIT       ((reg8 *) OSDPReaderTimer_TimerUDB_sT16_timerdp_u0__A0_REG )
    
    #if (OSDPReaderTimer_UsingHWCaptureCounter)
        #define OSDPReaderTimer_CAP_COUNT              (*(reg8 *) OSDPReaderTimer_TimerUDB_sCapCount_counter__PERIOD_REG )
        #define OSDPReaderTimer_CAP_COUNT_PTR          ( (reg8 *) OSDPReaderTimer_TimerUDB_sCapCount_counter__PERIOD_REG )
        #define OSDPReaderTimer_CAPTURE_COUNT_CTRL     (*(reg8 *) OSDPReaderTimer_TimerUDB_sCapCount_counter__CONTROL_AUX_CTL_REG )
        #define OSDPReaderTimer_CAPTURE_COUNT_CTRL_PTR ( (reg8 *) OSDPReaderTimer_TimerUDB_sCapCount_counter__CONTROL_AUX_CTL_REG )
    #endif /* (OSDPReaderTimer_UsingHWCaptureCounter) */


    /***************************************
    *       Register Constants
    ***************************************/

    /* Control Register Bit Locations */
    #define OSDPReaderTimer_CTRL_INTCNT_SHIFT              0x00u       /* As defined by Verilog Implementation */
    #define OSDPReaderTimer_CTRL_TRIG_MODE_SHIFT           0x02u       /* As defined by Verilog Implementation */
    #define OSDPReaderTimer_CTRL_TRIG_EN_SHIFT             0x04u       /* As defined by Verilog Implementation */
    #define OSDPReaderTimer_CTRL_CAP_MODE_SHIFT            0x05u       /* As defined by Verilog Implementation */
    #define OSDPReaderTimer_CTRL_ENABLE_SHIFT              0x07u       /* As defined by Verilog Implementation */

    /* Control Register Bit Masks */
    #define OSDPReaderTimer_CTRL_INTCNT_MASK               ((uint8)((uint8)0x03u << OSDPReaderTimer_CTRL_INTCNT_SHIFT))
    #define OSDPReaderTimer_CTRL_TRIG_MODE_MASK            ((uint8)((uint8)0x03u << OSDPReaderTimer_CTRL_TRIG_MODE_SHIFT))
    #define OSDPReaderTimer_CTRL_TRIG_EN                   ((uint8)((uint8)0x01u << OSDPReaderTimer_CTRL_TRIG_EN_SHIFT))
    #define OSDPReaderTimer_CTRL_CAP_MODE_MASK             ((uint8)((uint8)0x03u << OSDPReaderTimer_CTRL_CAP_MODE_SHIFT))
    #define OSDPReaderTimer_CTRL_ENABLE                    ((uint8)((uint8)0x01u << OSDPReaderTimer_CTRL_ENABLE_SHIFT))

    /* Bit Counter (7-bit) Control Register Bit Definitions */
    /* As defined by the Register map for the AUX Control Register */
    #define OSDPReaderTimer_CNTR_ENABLE                    0x20u

    /* Status Register Bit Locations */
    #define OSDPReaderTimer_STATUS_TC_SHIFT                0x00u  /* As defined by Verilog Implementation */
    #define OSDPReaderTimer_STATUS_CAPTURE_SHIFT           0x01u  /* As defined by Verilog Implementation */
    #define OSDPReaderTimer_STATUS_TC_INT_MASK_SHIFT       OSDPReaderTimer_STATUS_TC_SHIFT
    #define OSDPReaderTimer_STATUS_CAPTURE_INT_MASK_SHIFT  OSDPReaderTimer_STATUS_CAPTURE_SHIFT
    #define OSDPReaderTimer_STATUS_FIFOFULL_SHIFT          0x02u  /* As defined by Verilog Implementation */
    #define OSDPReaderTimer_STATUS_FIFONEMP_SHIFT          0x03u  /* As defined by Verilog Implementation */
    #define OSDPReaderTimer_STATUS_FIFOFULL_INT_MASK_SHIFT OSDPReaderTimer_STATUS_FIFOFULL_SHIFT

    /* Status Register Bit Masks */
    /* Sticky TC Event Bit-Mask */
    #define OSDPReaderTimer_STATUS_TC                      ((uint8)((uint8)0x01u << OSDPReaderTimer_STATUS_TC_SHIFT))
    /* Sticky Capture Event Bit-Mask */
    #define OSDPReaderTimer_STATUS_CAPTURE                 ((uint8)((uint8)0x01u << OSDPReaderTimer_STATUS_CAPTURE_SHIFT))
    /* Interrupt Enable Bit-Mask */
    #define OSDPReaderTimer_STATUS_TC_INT_MASK             ((uint8)((uint8)0x01u << OSDPReaderTimer_STATUS_TC_SHIFT))
    /* Interrupt Enable Bit-Mask */
    #define OSDPReaderTimer_STATUS_CAPTURE_INT_MASK        ((uint8)((uint8)0x01u << OSDPReaderTimer_STATUS_CAPTURE_SHIFT))
    /* NOT-Sticky FIFO Full Bit-Mask */
    #define OSDPReaderTimer_STATUS_FIFOFULL                ((uint8)((uint8)0x01u << OSDPReaderTimer_STATUS_FIFOFULL_SHIFT))
    /* NOT-Sticky FIFO Not Empty Bit-Mask */
    #define OSDPReaderTimer_STATUS_FIFONEMP                ((uint8)((uint8)0x01u << OSDPReaderTimer_STATUS_FIFONEMP_SHIFT))
    /* Interrupt Enable Bit-Mask */
    #define OSDPReaderTimer_STATUS_FIFOFULL_INT_MASK       ((uint8)((uint8)0x01u << OSDPReaderTimer_STATUS_FIFOFULL_SHIFT))

    #define OSDPReaderTimer_STATUS_ACTL_INT_EN             0x10u   /* As defined for the ACTL Register */

    /* Datapath Auxillary Control Register definitions */
    #define OSDPReaderTimer_AUX_CTRL_FIFO0_CLR             0x01u   /* As defined by Register map */
    #define OSDPReaderTimer_AUX_CTRL_FIFO1_CLR             0x02u   /* As defined by Register map */
    #define OSDPReaderTimer_AUX_CTRL_FIFO0_LVL             0x04u   /* As defined by Register map */
    #define OSDPReaderTimer_AUX_CTRL_FIFO1_LVL             0x08u   /* As defined by Register map */
    #define OSDPReaderTimer_STATUS_ACTL_INT_EN_MASK        0x10u   /* As defined for the ACTL Register */

#endif /* Implementation Specific Registers and Register Constants */

#endif  /* CY_TIMER_OSDPReaderTimer_H */


/* [] END OF FILE */
