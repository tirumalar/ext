/*******************************************************************************
* File Name: Timer_PWMAUD.h
* Version 2.10
*
* Description:
*  This file provides constants and parameter values for the Timer_PWMAUD
*  component.
*
* Note:
*  None
*
********************************************************************************
* Copyright 2013-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions,
* disclaimers, and limitations in the end user license agreement accompanying
* the software package with which this file was provided.
*******************************************************************************/

#if !defined(CY_TCPWM_Timer_PWMAUD_H)
#define CY_TCPWM_Timer_PWMAUD_H


#include "CyLib.h"
#include "cytypes.h"
#include "cyfitter.h"


/*******************************************************************************
* Internal Type defines
*******************************************************************************/

/* Structure to save state before go to sleep */
typedef struct
{
    uint8  enableState;
} Timer_PWMAUD_BACKUP_STRUCT;


/*******************************************************************************
* Variables
*******************************************************************************/
extern uint8  Timer_PWMAUD_initVar;


/***************************************
*   Conditional Compilation Parameters
****************************************/

#define Timer_PWMAUD_CY_TCPWM_V2                    (CYIPBLOCK_m0s8tcpwm_VERSION == 2u)
#define Timer_PWMAUD_CY_TCPWM_4000                  (CY_PSOC4_4000)

/* TCPWM Configuration */
#define Timer_PWMAUD_CONFIG                         (1lu)

/* Quad Mode */
/* Parameters */
#define Timer_PWMAUD_QUAD_ENCODING_MODES            (0lu)
#define Timer_PWMAUD_QUAD_AUTO_START                (1lu)

/* Signal modes */
#define Timer_PWMAUD_QUAD_INDEX_SIGNAL_MODE         (0lu)
#define Timer_PWMAUD_QUAD_PHIA_SIGNAL_MODE          (3lu)
#define Timer_PWMAUD_QUAD_PHIB_SIGNAL_MODE          (3lu)
#define Timer_PWMAUD_QUAD_STOP_SIGNAL_MODE          (0lu)

/* Signal present */
#define Timer_PWMAUD_QUAD_INDEX_SIGNAL_PRESENT      (0lu)
#define Timer_PWMAUD_QUAD_STOP_SIGNAL_PRESENT       (0lu)

/* Interrupt Mask */
#define Timer_PWMAUD_QUAD_INTERRUPT_MASK            (1lu)

/* Timer/Counter Mode */
/* Parameters */
#define Timer_PWMAUD_TC_RUN_MODE                    (0lu)
#define Timer_PWMAUD_TC_COUNTER_MODE                (0lu)
#define Timer_PWMAUD_TC_COMP_CAP_MODE               (2lu)
#define Timer_PWMAUD_TC_PRESCALER                   (0lu)

/* Signal modes */
#define Timer_PWMAUD_TC_RELOAD_SIGNAL_MODE          (0lu)
#define Timer_PWMAUD_TC_COUNT_SIGNAL_MODE           (3lu)
#define Timer_PWMAUD_TC_START_SIGNAL_MODE           (0lu)
#define Timer_PWMAUD_TC_STOP_SIGNAL_MODE            (0lu)
#define Timer_PWMAUD_TC_CAPTURE_SIGNAL_MODE         (1lu)

/* Signal present */
#define Timer_PWMAUD_TC_RELOAD_SIGNAL_PRESENT       (0lu)
#define Timer_PWMAUD_TC_COUNT_SIGNAL_PRESENT        (0lu)
#define Timer_PWMAUD_TC_START_SIGNAL_PRESENT        (1lu)
#define Timer_PWMAUD_TC_STOP_SIGNAL_PRESENT         (0lu)
#define Timer_PWMAUD_TC_CAPTURE_SIGNAL_PRESENT      (1lu)

/* Interrupt Mask */
#define Timer_PWMAUD_TC_INTERRUPT_MASK              (1lu)

/* PWM Mode */
/* Parameters */
#define Timer_PWMAUD_PWM_KILL_EVENT                 (0lu)
#define Timer_PWMAUD_PWM_STOP_EVENT                 (0lu)
#define Timer_PWMAUD_PWM_MODE                       (4lu)
#define Timer_PWMAUD_PWM_OUT_N_INVERT               (0lu)
#define Timer_PWMAUD_PWM_OUT_INVERT                 (0lu)
#define Timer_PWMAUD_PWM_ALIGN                      (0lu)
#define Timer_PWMAUD_PWM_RUN_MODE                   (0lu)
#define Timer_PWMAUD_PWM_DEAD_TIME_CYCLE            (0lu)
#define Timer_PWMAUD_PWM_PRESCALER                  (0lu)

/* Signal modes */
#define Timer_PWMAUD_PWM_RELOAD_SIGNAL_MODE         (0lu)
#define Timer_PWMAUD_PWM_COUNT_SIGNAL_MODE          (3lu)
#define Timer_PWMAUD_PWM_START_SIGNAL_MODE          (0lu)
#define Timer_PWMAUD_PWM_STOP_SIGNAL_MODE           (0lu)
#define Timer_PWMAUD_PWM_SWITCH_SIGNAL_MODE         (0lu)

/* Signal present */
#define Timer_PWMAUD_PWM_RELOAD_SIGNAL_PRESENT      (0lu)
#define Timer_PWMAUD_PWM_COUNT_SIGNAL_PRESENT       (0lu)
#define Timer_PWMAUD_PWM_START_SIGNAL_PRESENT       (0lu)
#define Timer_PWMAUD_PWM_STOP_SIGNAL_PRESENT        (0lu)
#define Timer_PWMAUD_PWM_SWITCH_SIGNAL_PRESENT      (0lu)

/* Interrupt Mask */
#define Timer_PWMAUD_PWM_INTERRUPT_MASK             (1lu)


/***************************************
*    Initial Parameter Constants
***************************************/

/* Timer/Counter Mode */
#define Timer_PWMAUD_TC_PERIOD_VALUE                (65535lu)
#define Timer_PWMAUD_TC_COMPARE_VALUE               (65535lu)
#define Timer_PWMAUD_TC_COMPARE_BUF_VALUE           (65535lu)
#define Timer_PWMAUD_TC_COMPARE_SWAP                (0lu)

/* PWM Mode */
#define Timer_PWMAUD_PWM_PERIOD_VALUE               (65535lu)
#define Timer_PWMAUD_PWM_PERIOD_BUF_VALUE           (65535lu)
#define Timer_PWMAUD_PWM_PERIOD_SWAP                (0lu)
#define Timer_PWMAUD_PWM_COMPARE_VALUE              (65535lu)
#define Timer_PWMAUD_PWM_COMPARE_BUF_VALUE          (65535lu)
#define Timer_PWMAUD_PWM_COMPARE_SWAP               (0lu)


/***************************************
*    Enumerated Types and Parameters
***************************************/

#define Timer_PWMAUD__LEFT 0
#define Timer_PWMAUD__RIGHT 1
#define Timer_PWMAUD__CENTER 2
#define Timer_PWMAUD__ASYMMETRIC 3

#define Timer_PWMAUD__X1 0
#define Timer_PWMAUD__X2 1
#define Timer_PWMAUD__X4 2

#define Timer_PWMAUD__PWM 4
#define Timer_PWMAUD__PWM_DT 5
#define Timer_PWMAUD__PWM_PR 6

#define Timer_PWMAUD__INVERSE 1
#define Timer_PWMAUD__DIRECT 0

#define Timer_PWMAUD__CAPTURE 2
#define Timer_PWMAUD__COMPARE 0

#define Timer_PWMAUD__TRIG_LEVEL 3
#define Timer_PWMAUD__TRIG_RISING 0
#define Timer_PWMAUD__TRIG_FALLING 1
#define Timer_PWMAUD__TRIG_BOTH 2

#define Timer_PWMAUD__INTR_MASK_TC 1
#define Timer_PWMAUD__INTR_MASK_CC_MATCH 2
#define Timer_PWMAUD__INTR_MASK_NONE 0
#define Timer_PWMAUD__INTR_MASK_TC_CC 3

#define Timer_PWMAUD__UNCONFIG 8
#define Timer_PWMAUD__TIMER 1
#define Timer_PWMAUD__QUAD 3
#define Timer_PWMAUD__PWM_SEL 7

#define Timer_PWMAUD__COUNT_UP 0
#define Timer_PWMAUD__COUNT_DOWN 1
#define Timer_PWMAUD__COUNT_UPDOWN0 2
#define Timer_PWMAUD__COUNT_UPDOWN1 3


/* Prescaler */
#define Timer_PWMAUD_PRESCALE_DIVBY1                ((uint32)(0u << Timer_PWMAUD_PRESCALER_SHIFT))
#define Timer_PWMAUD_PRESCALE_DIVBY2                ((uint32)(1u << Timer_PWMAUD_PRESCALER_SHIFT))
#define Timer_PWMAUD_PRESCALE_DIVBY4                ((uint32)(2u << Timer_PWMAUD_PRESCALER_SHIFT))
#define Timer_PWMAUD_PRESCALE_DIVBY8                ((uint32)(3u << Timer_PWMAUD_PRESCALER_SHIFT))
#define Timer_PWMAUD_PRESCALE_DIVBY16               ((uint32)(4u << Timer_PWMAUD_PRESCALER_SHIFT))
#define Timer_PWMAUD_PRESCALE_DIVBY32               ((uint32)(5u << Timer_PWMAUD_PRESCALER_SHIFT))
#define Timer_PWMAUD_PRESCALE_DIVBY64               ((uint32)(6u << Timer_PWMAUD_PRESCALER_SHIFT))
#define Timer_PWMAUD_PRESCALE_DIVBY128              ((uint32)(7u << Timer_PWMAUD_PRESCALER_SHIFT))

/* TCPWM set modes */
#define Timer_PWMAUD_MODE_TIMER_COMPARE             ((uint32)(Timer_PWMAUD__COMPARE         <<  \
                                                                  Timer_PWMAUD_MODE_SHIFT))
#define Timer_PWMAUD_MODE_TIMER_CAPTURE             ((uint32)(Timer_PWMAUD__CAPTURE         <<  \
                                                                  Timer_PWMAUD_MODE_SHIFT))
#define Timer_PWMAUD_MODE_QUAD                      ((uint32)(Timer_PWMAUD__QUAD            <<  \
                                                                  Timer_PWMAUD_MODE_SHIFT))
#define Timer_PWMAUD_MODE_PWM                       ((uint32)(Timer_PWMAUD__PWM             <<  \
                                                                  Timer_PWMAUD_MODE_SHIFT))
#define Timer_PWMAUD_MODE_PWM_DT                    ((uint32)(Timer_PWMAUD__PWM_DT          <<  \
                                                                  Timer_PWMAUD_MODE_SHIFT))
#define Timer_PWMAUD_MODE_PWM_PR                    ((uint32)(Timer_PWMAUD__PWM_PR          <<  \
                                                                  Timer_PWMAUD_MODE_SHIFT))

/* Quad Modes */
#define Timer_PWMAUD_MODE_X1                        ((uint32)(Timer_PWMAUD__X1              <<  \
                                                                  Timer_PWMAUD_QUAD_MODE_SHIFT))
#define Timer_PWMAUD_MODE_X2                        ((uint32)(Timer_PWMAUD__X2              <<  \
                                                                  Timer_PWMAUD_QUAD_MODE_SHIFT))
#define Timer_PWMAUD_MODE_X4                        ((uint32)(Timer_PWMAUD__X4              <<  \
                                                                  Timer_PWMAUD_QUAD_MODE_SHIFT))

/* Counter modes */
#define Timer_PWMAUD_COUNT_UP                       ((uint32)(Timer_PWMAUD__COUNT_UP        <<  \
                                                                  Timer_PWMAUD_UPDOWN_SHIFT))
#define Timer_PWMAUD_COUNT_DOWN                     ((uint32)(Timer_PWMAUD__COUNT_DOWN      <<  \
                                                                  Timer_PWMAUD_UPDOWN_SHIFT))
#define Timer_PWMAUD_COUNT_UPDOWN0                  ((uint32)(Timer_PWMAUD__COUNT_UPDOWN0   <<  \
                                                                  Timer_PWMAUD_UPDOWN_SHIFT))
#define Timer_PWMAUD_COUNT_UPDOWN1                  ((uint32)(Timer_PWMAUD__COUNT_UPDOWN1   <<  \
                                                                  Timer_PWMAUD_UPDOWN_SHIFT))

/* PWM output invert */
#define Timer_PWMAUD_INVERT_LINE                    ((uint32)(Timer_PWMAUD__INVERSE         <<  \
                                                                  Timer_PWMAUD_INV_OUT_SHIFT))
#define Timer_PWMAUD_INVERT_LINE_N                  ((uint32)(Timer_PWMAUD__INVERSE         <<  \
                                                                  Timer_PWMAUD_INV_COMPL_OUT_SHIFT))

/* Trigger modes */
#define Timer_PWMAUD_TRIG_RISING                    ((uint32)Timer_PWMAUD__TRIG_RISING)
#define Timer_PWMAUD_TRIG_FALLING                   ((uint32)Timer_PWMAUD__TRIG_FALLING)
#define Timer_PWMAUD_TRIG_BOTH                      ((uint32)Timer_PWMAUD__TRIG_BOTH)
#define Timer_PWMAUD_TRIG_LEVEL                     ((uint32)Timer_PWMAUD__TRIG_LEVEL)

/* Interrupt mask */
#define Timer_PWMAUD_INTR_MASK_TC                   ((uint32)Timer_PWMAUD__INTR_MASK_TC)
#define Timer_PWMAUD_INTR_MASK_CC_MATCH             ((uint32)Timer_PWMAUD__INTR_MASK_CC_MATCH)

/* PWM Output Controls */
#define Timer_PWMAUD_CC_MATCH_SET                   (0x00u)
#define Timer_PWMAUD_CC_MATCH_CLEAR                 (0x01u)
#define Timer_PWMAUD_CC_MATCH_INVERT                (0x02u)
#define Timer_PWMAUD_CC_MATCH_NO_CHANGE             (0x03u)
#define Timer_PWMAUD_OVERLOW_SET                    (0x00u)
#define Timer_PWMAUD_OVERLOW_CLEAR                  (0x04u)
#define Timer_PWMAUD_OVERLOW_INVERT                 (0x08u)
#define Timer_PWMAUD_OVERLOW_NO_CHANGE              (0x0Cu)
#define Timer_PWMAUD_UNDERFLOW_SET                  (0x00u)
#define Timer_PWMAUD_UNDERFLOW_CLEAR                (0x10u)
#define Timer_PWMAUD_UNDERFLOW_INVERT               (0x20u)
#define Timer_PWMAUD_UNDERFLOW_NO_CHANGE            (0x30u)

/* PWM Align */
#define Timer_PWMAUD_PWM_MODE_LEFT                  (Timer_PWMAUD_CC_MATCH_CLEAR        |   \
                                                         Timer_PWMAUD_OVERLOW_SET           |   \
                                                         Timer_PWMAUD_UNDERFLOW_NO_CHANGE)
#define Timer_PWMAUD_PWM_MODE_RIGHT                 (Timer_PWMAUD_CC_MATCH_SET          |   \
                                                         Timer_PWMAUD_OVERLOW_NO_CHANGE     |   \
                                                         Timer_PWMAUD_UNDERFLOW_CLEAR)
#define Timer_PWMAUD_PWM_MODE_ASYM                  (Timer_PWMAUD_CC_MATCH_INVERT       |   \
                                                         Timer_PWMAUD_OVERLOW_SET           |   \
                                                         Timer_PWMAUD_UNDERFLOW_CLEAR)

#if (Timer_PWMAUD_CY_TCPWM_V2)
    #if(Timer_PWMAUD_CY_TCPWM_4000)
        #define Timer_PWMAUD_PWM_MODE_CENTER                (Timer_PWMAUD_CC_MATCH_INVERT       |   \
                                                                 Timer_PWMAUD_OVERLOW_NO_CHANGE     |   \
                                                                 Timer_PWMAUD_UNDERFLOW_CLEAR)
    #else
        #define Timer_PWMAUD_PWM_MODE_CENTER                (Timer_PWMAUD_CC_MATCH_INVERT       |   \
                                                                 Timer_PWMAUD_OVERLOW_SET           |   \
                                                                 Timer_PWMAUD_UNDERFLOW_CLEAR)
    #endif /* (Timer_PWMAUD_CY_TCPWM_4000) */
#else
    #define Timer_PWMAUD_PWM_MODE_CENTER                (Timer_PWMAUD_CC_MATCH_INVERT       |   \
                                                             Timer_PWMAUD_OVERLOW_NO_CHANGE     |   \
                                                             Timer_PWMAUD_UNDERFLOW_CLEAR)
#endif /* (Timer_PWMAUD_CY_TCPWM_NEW) */

/* Command operations without condition */
#define Timer_PWMAUD_CMD_CAPTURE                    (0u)
#define Timer_PWMAUD_CMD_RELOAD                     (8u)
#define Timer_PWMAUD_CMD_STOP                       (16u)
#define Timer_PWMAUD_CMD_START                      (24u)

/* Status */
#define Timer_PWMAUD_STATUS_DOWN                    (1u)
#define Timer_PWMAUD_STATUS_RUNNING                 (2u)


/***************************************
*        Function Prototypes
****************************************/

void   Timer_PWMAUD_Init(void);
void   Timer_PWMAUD_Enable(void);
void   Timer_PWMAUD_Start(void);
void   Timer_PWMAUD_Stop(void);

void   Timer_PWMAUD_SetMode(uint32 mode);
void   Timer_PWMAUD_SetCounterMode(uint32 counterMode);
void   Timer_PWMAUD_SetPWMMode(uint32 modeMask);
void   Timer_PWMAUD_SetQDMode(uint32 qdMode);

void   Timer_PWMAUD_SetPrescaler(uint32 prescaler);
void   Timer_PWMAUD_TriggerCommand(uint32 mask, uint32 command);
void   Timer_PWMAUD_SetOneShot(uint32 oneShotEnable);
uint32 Timer_PWMAUD_ReadStatus(void);

void   Timer_PWMAUD_SetPWMSyncKill(uint32 syncKillEnable);
void   Timer_PWMAUD_SetPWMStopOnKill(uint32 stopOnKillEnable);
void   Timer_PWMAUD_SetPWMDeadTime(uint32 deadTime);
void   Timer_PWMAUD_SetPWMInvert(uint32 mask);

void   Timer_PWMAUD_SetInterruptMode(uint32 interruptMask);
uint32 Timer_PWMAUD_GetInterruptSourceMasked(void);
uint32 Timer_PWMAUD_GetInterruptSource(void);
void   Timer_PWMAUD_ClearInterrupt(uint32 interruptMask);
void   Timer_PWMAUD_SetInterrupt(uint32 interruptMask);

void   Timer_PWMAUD_WriteCounter(uint32 count);
uint32 Timer_PWMAUD_ReadCounter(void);

uint32 Timer_PWMAUD_ReadCapture(void);
uint32 Timer_PWMAUD_ReadCaptureBuf(void);

void   Timer_PWMAUD_WritePeriod(uint32 period);
uint32 Timer_PWMAUD_ReadPeriod(void);
void   Timer_PWMAUD_WritePeriodBuf(uint32 periodBuf);
uint32 Timer_PWMAUD_ReadPeriodBuf(void);

void   Timer_PWMAUD_WriteCompare(uint32 compare);
uint32 Timer_PWMAUD_ReadCompare(void);
void   Timer_PWMAUD_WriteCompareBuf(uint32 compareBuf);
uint32 Timer_PWMAUD_ReadCompareBuf(void);

void   Timer_PWMAUD_SetPeriodSwap(uint32 swapEnable);
void   Timer_PWMAUD_SetCompareSwap(uint32 swapEnable);

void   Timer_PWMAUD_SetCaptureMode(uint32 triggerMode);
void   Timer_PWMAUD_SetReloadMode(uint32 triggerMode);
void   Timer_PWMAUD_SetStartMode(uint32 triggerMode);
void   Timer_PWMAUD_SetStopMode(uint32 triggerMode);
void   Timer_PWMAUD_SetCountMode(uint32 triggerMode);

void   Timer_PWMAUD_SaveConfig(void);
void   Timer_PWMAUD_RestoreConfig(void);
void   Timer_PWMAUD_Sleep(void);
void   Timer_PWMAUD_Wakeup(void);


/***************************************
*             Registers
***************************************/

#define Timer_PWMAUD_BLOCK_CONTROL_REG              (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__TCPWM_CTRL )
#define Timer_PWMAUD_BLOCK_CONTROL_PTR              ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__TCPWM_CTRL )
#define Timer_PWMAUD_COMMAND_REG                    (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__TCPWM_CMD )
#define Timer_PWMAUD_COMMAND_PTR                    ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__TCPWM_CMD )
#define Timer_PWMAUD_INTRRUPT_CAUSE_REG             (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__TCPWM_INTR_CAUSE )
#define Timer_PWMAUD_INTRRUPT_CAUSE_PTR             ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__TCPWM_INTR_CAUSE )
#define Timer_PWMAUD_CONTROL_REG                    (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__CTRL )
#define Timer_PWMAUD_CONTROL_PTR                    ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__CTRL )
#define Timer_PWMAUD_STATUS_REG                     (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__STATUS )
#define Timer_PWMAUD_STATUS_PTR                     ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__STATUS )
#define Timer_PWMAUD_COUNTER_REG                    (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__COUNTER )
#define Timer_PWMAUD_COUNTER_PTR                    ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__COUNTER )
#define Timer_PWMAUD_COMP_CAP_REG                   (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__CC )
#define Timer_PWMAUD_COMP_CAP_PTR                   ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__CC )
#define Timer_PWMAUD_COMP_CAP_BUF_REG               (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__CC_BUFF )
#define Timer_PWMAUD_COMP_CAP_BUF_PTR               ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__CC_BUFF )
#define Timer_PWMAUD_PERIOD_REG                     (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__PERIOD )
#define Timer_PWMAUD_PERIOD_PTR                     ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__PERIOD )
#define Timer_PWMAUD_PERIOD_BUF_REG                 (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__PERIOD_BUFF )
#define Timer_PWMAUD_PERIOD_BUF_PTR                 ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__PERIOD_BUFF )
#define Timer_PWMAUD_TRIG_CONTROL0_REG              (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__TR_CTRL0 )
#define Timer_PWMAUD_TRIG_CONTROL0_PTR              ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__TR_CTRL0 )
#define Timer_PWMAUD_TRIG_CONTROL1_REG              (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__TR_CTRL1 )
#define Timer_PWMAUD_TRIG_CONTROL1_PTR              ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__TR_CTRL1 )
#define Timer_PWMAUD_TRIG_CONTROL2_REG              (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__TR_CTRL2 )
#define Timer_PWMAUD_TRIG_CONTROL2_PTR              ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__TR_CTRL2 )
#define Timer_PWMAUD_INTERRUPT_REQ_REG              (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__INTR )
#define Timer_PWMAUD_INTERRUPT_REQ_PTR              ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__INTR )
#define Timer_PWMAUD_INTERRUPT_SET_REG              (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__INTR_SET )
#define Timer_PWMAUD_INTERRUPT_SET_PTR              ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__INTR_SET )
#define Timer_PWMAUD_INTERRUPT_MASK_REG             (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__INTR_MASK )
#define Timer_PWMAUD_INTERRUPT_MASK_PTR             ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__INTR_MASK )
#define Timer_PWMAUD_INTERRUPT_MASKED_REG           (*(reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__INTR_MASKED )
#define Timer_PWMAUD_INTERRUPT_MASKED_PTR           ( (reg32 *) Timer_PWMAUD_cy_m0s8_tcpwm_1__INTR_MASKED )


/***************************************
*       Registers Constants
***************************************/

/* Mask */
#define Timer_PWMAUD_MASK                           ((uint32)Timer_PWMAUD_cy_m0s8_tcpwm_1__TCPWM_CTRL_MASK)

/* Shift constants for control register */
#define Timer_PWMAUD_RELOAD_CC_SHIFT                (0u)
#define Timer_PWMAUD_RELOAD_PERIOD_SHIFT            (1u)
#define Timer_PWMAUD_PWM_SYNC_KILL_SHIFT            (2u)
#define Timer_PWMAUD_PWM_STOP_KILL_SHIFT            (3u)
#define Timer_PWMAUD_PRESCALER_SHIFT                (8u)
#define Timer_PWMAUD_UPDOWN_SHIFT                   (16u)
#define Timer_PWMAUD_ONESHOT_SHIFT                  (18u)
#define Timer_PWMAUD_QUAD_MODE_SHIFT                (20u)
#define Timer_PWMAUD_INV_OUT_SHIFT                  (20u)
#define Timer_PWMAUD_INV_COMPL_OUT_SHIFT            (21u)
#define Timer_PWMAUD_MODE_SHIFT                     (24u)

/* Mask constants for control register */
#define Timer_PWMAUD_RELOAD_CC_MASK                 ((uint32)(Timer_PWMAUD_1BIT_MASK        <<  \
                                                                            Timer_PWMAUD_RELOAD_CC_SHIFT))
#define Timer_PWMAUD_RELOAD_PERIOD_MASK             ((uint32)(Timer_PWMAUD_1BIT_MASK        <<  \
                                                                            Timer_PWMAUD_RELOAD_PERIOD_SHIFT))
#define Timer_PWMAUD_PWM_SYNC_KILL_MASK             ((uint32)(Timer_PWMAUD_1BIT_MASK        <<  \
                                                                            Timer_PWMAUD_PWM_SYNC_KILL_SHIFT))
#define Timer_PWMAUD_PWM_STOP_KILL_MASK             ((uint32)(Timer_PWMAUD_1BIT_MASK        <<  \
                                                                            Timer_PWMAUD_PWM_STOP_KILL_SHIFT))
#define Timer_PWMAUD_PRESCALER_MASK                 ((uint32)(Timer_PWMAUD_8BIT_MASK        <<  \
                                                                            Timer_PWMAUD_PRESCALER_SHIFT))
#define Timer_PWMAUD_UPDOWN_MASK                    ((uint32)(Timer_PWMAUD_2BIT_MASK        <<  \
                                                                            Timer_PWMAUD_UPDOWN_SHIFT))
#define Timer_PWMAUD_ONESHOT_MASK                   ((uint32)(Timer_PWMAUD_1BIT_MASK        <<  \
                                                                            Timer_PWMAUD_ONESHOT_SHIFT))
#define Timer_PWMAUD_QUAD_MODE_MASK                 ((uint32)(Timer_PWMAUD_3BIT_MASK        <<  \
                                                                            Timer_PWMAUD_QUAD_MODE_SHIFT))
#define Timer_PWMAUD_INV_OUT_MASK                   ((uint32)(Timer_PWMAUD_2BIT_MASK        <<  \
                                                                            Timer_PWMAUD_INV_OUT_SHIFT))
#define Timer_PWMAUD_MODE_MASK                      ((uint32)(Timer_PWMAUD_3BIT_MASK        <<  \
                                                                            Timer_PWMAUD_MODE_SHIFT))

/* Shift constants for trigger control register 1 */
#define Timer_PWMAUD_CAPTURE_SHIFT                  (0u)
#define Timer_PWMAUD_COUNT_SHIFT                    (2u)
#define Timer_PWMAUD_RELOAD_SHIFT                   (4u)
#define Timer_PWMAUD_STOP_SHIFT                     (6u)
#define Timer_PWMAUD_START_SHIFT                    (8u)

/* Mask constants for trigger control register 1 */
#define Timer_PWMAUD_CAPTURE_MASK                   ((uint32)(Timer_PWMAUD_2BIT_MASK        <<  \
                                                                  Timer_PWMAUD_CAPTURE_SHIFT))
#define Timer_PWMAUD_COUNT_MASK                     ((uint32)(Timer_PWMAUD_2BIT_MASK        <<  \
                                                                  Timer_PWMAUD_COUNT_SHIFT))
#define Timer_PWMAUD_RELOAD_MASK                    ((uint32)(Timer_PWMAUD_2BIT_MASK        <<  \
                                                                  Timer_PWMAUD_RELOAD_SHIFT))
#define Timer_PWMAUD_STOP_MASK                      ((uint32)(Timer_PWMAUD_2BIT_MASK        <<  \
                                                                  Timer_PWMAUD_STOP_SHIFT))
#define Timer_PWMAUD_START_MASK                     ((uint32)(Timer_PWMAUD_2BIT_MASK        <<  \
                                                                  Timer_PWMAUD_START_SHIFT))

/* MASK */
#define Timer_PWMAUD_1BIT_MASK                      ((uint32)0x01u)
#define Timer_PWMAUD_2BIT_MASK                      ((uint32)0x03u)
#define Timer_PWMAUD_3BIT_MASK                      ((uint32)0x07u)
#define Timer_PWMAUD_6BIT_MASK                      ((uint32)0x3Fu)
#define Timer_PWMAUD_8BIT_MASK                      ((uint32)0xFFu)
#define Timer_PWMAUD_16BIT_MASK                     ((uint32)0xFFFFu)

/* Shift constant for status register */
#define Timer_PWMAUD_RUNNING_STATUS_SHIFT           (30u)


/***************************************
*    Initial Constants
***************************************/

#define Timer_PWMAUD_CTRL_QUAD_BASE_CONFIG                                                          \
        (((uint32)(Timer_PWMAUD_QUAD_ENCODING_MODES     << Timer_PWMAUD_QUAD_MODE_SHIFT))       |\
         ((uint32)(Timer_PWMAUD_CONFIG                  << Timer_PWMAUD_MODE_SHIFT)))

#define Timer_PWMAUD_CTRL_PWM_BASE_CONFIG                                                           \
        (((uint32)(Timer_PWMAUD_PWM_STOP_EVENT          << Timer_PWMAUD_PWM_STOP_KILL_SHIFT))   |\
         ((uint32)(Timer_PWMAUD_PWM_OUT_INVERT          << Timer_PWMAUD_INV_OUT_SHIFT))         |\
         ((uint32)(Timer_PWMAUD_PWM_OUT_N_INVERT        << Timer_PWMAUD_INV_COMPL_OUT_SHIFT))   |\
         ((uint32)(Timer_PWMAUD_PWM_MODE                << Timer_PWMAUD_MODE_SHIFT)))

#define Timer_PWMAUD_CTRL_PWM_RUN_MODE                                                              \
            ((uint32)(Timer_PWMAUD_PWM_RUN_MODE         << Timer_PWMAUD_ONESHOT_SHIFT))
            
#define Timer_PWMAUD_CTRL_PWM_ALIGN                                                                 \
            ((uint32)(Timer_PWMAUD_PWM_ALIGN            << Timer_PWMAUD_UPDOWN_SHIFT))

#define Timer_PWMAUD_CTRL_PWM_KILL_EVENT                                                            \
             ((uint32)(Timer_PWMAUD_PWM_KILL_EVENT      << Timer_PWMAUD_PWM_SYNC_KILL_SHIFT))

#define Timer_PWMAUD_CTRL_PWM_DEAD_TIME_CYCLE                                                       \
            ((uint32)(Timer_PWMAUD_PWM_DEAD_TIME_CYCLE  << Timer_PWMAUD_PRESCALER_SHIFT))

#define Timer_PWMAUD_CTRL_PWM_PRESCALER                                                             \
            ((uint32)(Timer_PWMAUD_PWM_PRESCALER        << Timer_PWMAUD_PRESCALER_SHIFT))

#define Timer_PWMAUD_CTRL_TIMER_BASE_CONFIG                                                         \
        (((uint32)(Timer_PWMAUD_TC_PRESCALER            << Timer_PWMAUD_PRESCALER_SHIFT))       |\
         ((uint32)(Timer_PWMAUD_TC_COUNTER_MODE         << Timer_PWMAUD_UPDOWN_SHIFT))          |\
         ((uint32)(Timer_PWMAUD_TC_RUN_MODE             << Timer_PWMAUD_ONESHOT_SHIFT))         |\
         ((uint32)(Timer_PWMAUD_TC_COMP_CAP_MODE        << Timer_PWMAUD_MODE_SHIFT)))
        
#define Timer_PWMAUD_QUAD_SIGNALS_MODES                                                             \
        (((uint32)(Timer_PWMAUD_QUAD_PHIA_SIGNAL_MODE   << Timer_PWMAUD_COUNT_SHIFT))           |\
         ((uint32)(Timer_PWMAUD_QUAD_INDEX_SIGNAL_MODE  << Timer_PWMAUD_RELOAD_SHIFT))          |\
         ((uint32)(Timer_PWMAUD_QUAD_STOP_SIGNAL_MODE   << Timer_PWMAUD_STOP_SHIFT))            |\
         ((uint32)(Timer_PWMAUD_QUAD_PHIB_SIGNAL_MODE   << Timer_PWMAUD_START_SHIFT)))

#define Timer_PWMAUD_PWM_SIGNALS_MODES                                                              \
        (((uint32)(Timer_PWMAUD_PWM_SWITCH_SIGNAL_MODE  << Timer_PWMAUD_CAPTURE_SHIFT))         |\
         ((uint32)(Timer_PWMAUD_PWM_COUNT_SIGNAL_MODE   << Timer_PWMAUD_COUNT_SHIFT))           |\
         ((uint32)(Timer_PWMAUD_PWM_RELOAD_SIGNAL_MODE  << Timer_PWMAUD_RELOAD_SHIFT))          |\
         ((uint32)(Timer_PWMAUD_PWM_STOP_SIGNAL_MODE    << Timer_PWMAUD_STOP_SHIFT))            |\
         ((uint32)(Timer_PWMAUD_PWM_START_SIGNAL_MODE   << Timer_PWMAUD_START_SHIFT)))

#define Timer_PWMAUD_TIMER_SIGNALS_MODES                                                            \
        (((uint32)(Timer_PWMAUD_TC_CAPTURE_SIGNAL_MODE  << Timer_PWMAUD_CAPTURE_SHIFT))         |\
         ((uint32)(Timer_PWMAUD_TC_COUNT_SIGNAL_MODE    << Timer_PWMAUD_COUNT_SHIFT))           |\
         ((uint32)(Timer_PWMAUD_TC_RELOAD_SIGNAL_MODE   << Timer_PWMAUD_RELOAD_SHIFT))          |\
         ((uint32)(Timer_PWMAUD_TC_STOP_SIGNAL_MODE     << Timer_PWMAUD_STOP_SHIFT))            |\
         ((uint32)(Timer_PWMAUD_TC_START_SIGNAL_MODE    << Timer_PWMAUD_START_SHIFT)))
        
#define Timer_PWMAUD_TIMER_UPDOWN_CNT_USED                                                          \
                ((Timer_PWMAUD__COUNT_UPDOWN0 == Timer_PWMAUD_TC_COUNTER_MODE)                  ||\
                 (Timer_PWMAUD__COUNT_UPDOWN1 == Timer_PWMAUD_TC_COUNTER_MODE))

#define Timer_PWMAUD_PWM_UPDOWN_CNT_USED                                                            \
                ((Timer_PWMAUD__CENTER == Timer_PWMAUD_PWM_ALIGN)                               ||\
                 (Timer_PWMAUD__ASYMMETRIC == Timer_PWMAUD_PWM_ALIGN))               
        
#define Timer_PWMAUD_PWM_PR_INIT_VALUE              (1u)
#define Timer_PWMAUD_QUAD_PERIOD_INIT_VALUE         (0x8000u)



#endif /* End CY_TCPWM_Timer_PWMAUD_H */

/* [] END OF FILE */
