/*******************************************************************************
* File Name: STEP_COUNT.h
* Version 2.10
*
* Description:
*  This file provides constants and parameter values for the STEP_COUNT
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

#if !defined(CY_TCPWM_STEP_COUNT_H)
#define CY_TCPWM_STEP_COUNT_H


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
} STEP_COUNT_BACKUP_STRUCT;


/*******************************************************************************
* Variables
*******************************************************************************/
extern uint8  STEP_COUNT_initVar;


/***************************************
*   Conditional Compilation Parameters
****************************************/

#define STEP_COUNT_CY_TCPWM_V2                    (CYIPBLOCK_m0s8tcpwm_VERSION == 2u)
#define STEP_COUNT_CY_TCPWM_4000                  (CY_PSOC4_4000)

/* TCPWM Configuration */
#define STEP_COUNT_CONFIG                         (1lu)

/* Quad Mode */
/* Parameters */
#define STEP_COUNT_QUAD_ENCODING_MODES            (0lu)
#define STEP_COUNT_QUAD_AUTO_START                (1lu)

/* Signal modes */
#define STEP_COUNT_QUAD_INDEX_SIGNAL_MODE         (0lu)
#define STEP_COUNT_QUAD_PHIA_SIGNAL_MODE          (3lu)
#define STEP_COUNT_QUAD_PHIB_SIGNAL_MODE          (3lu)
#define STEP_COUNT_QUAD_STOP_SIGNAL_MODE          (0lu)

/* Signal present */
#define STEP_COUNT_QUAD_INDEX_SIGNAL_PRESENT      (0lu)
#define STEP_COUNT_QUAD_STOP_SIGNAL_PRESENT       (0lu)

/* Interrupt Mask */
#define STEP_COUNT_QUAD_INTERRUPT_MASK            (1lu)

/* Timer/Counter Mode */
/* Parameters */
#define STEP_COUNT_TC_RUN_MODE                    (0lu)
#define STEP_COUNT_TC_COUNTER_MODE                (0lu)
#define STEP_COUNT_TC_COMP_CAP_MODE               (2lu)
#define STEP_COUNT_TC_PRESCALER                   (0lu)

/* Signal modes */
#define STEP_COUNT_TC_RELOAD_SIGNAL_MODE          (0lu)
#define STEP_COUNT_TC_COUNT_SIGNAL_MODE           (2lu)
#define STEP_COUNT_TC_START_SIGNAL_MODE           (0lu)
#define STEP_COUNT_TC_STOP_SIGNAL_MODE            (0lu)
#define STEP_COUNT_TC_CAPTURE_SIGNAL_MODE         (0lu)

/* Signal present */
#define STEP_COUNT_TC_RELOAD_SIGNAL_PRESENT       (0lu)
#define STEP_COUNT_TC_COUNT_SIGNAL_PRESENT        (1lu)
#define STEP_COUNT_TC_START_SIGNAL_PRESENT        (0lu)
#define STEP_COUNT_TC_STOP_SIGNAL_PRESENT         (0lu)
#define STEP_COUNT_TC_CAPTURE_SIGNAL_PRESENT      (0lu)

/* Interrupt Mask */
#define STEP_COUNT_TC_INTERRUPT_MASK              (0lu)

/* PWM Mode */
/* Parameters */
#define STEP_COUNT_PWM_KILL_EVENT                 (0lu)
#define STEP_COUNT_PWM_STOP_EVENT                 (0lu)
#define STEP_COUNT_PWM_MODE                       (4lu)
#define STEP_COUNT_PWM_OUT_N_INVERT               (0lu)
#define STEP_COUNT_PWM_OUT_INVERT                 (0lu)
#define STEP_COUNT_PWM_ALIGN                      (0lu)
#define STEP_COUNT_PWM_RUN_MODE                   (0lu)
#define STEP_COUNT_PWM_DEAD_TIME_CYCLE            (0lu)
#define STEP_COUNT_PWM_PRESCALER                  (0lu)

/* Signal modes */
#define STEP_COUNT_PWM_RELOAD_SIGNAL_MODE         (0lu)
#define STEP_COUNT_PWM_COUNT_SIGNAL_MODE          (3lu)
#define STEP_COUNT_PWM_START_SIGNAL_MODE          (0lu)
#define STEP_COUNT_PWM_STOP_SIGNAL_MODE           (0lu)
#define STEP_COUNT_PWM_SWITCH_SIGNAL_MODE         (0lu)

/* Signal present */
#define STEP_COUNT_PWM_RELOAD_SIGNAL_PRESENT      (0lu)
#define STEP_COUNT_PWM_COUNT_SIGNAL_PRESENT       (0lu)
#define STEP_COUNT_PWM_START_SIGNAL_PRESENT       (0lu)
#define STEP_COUNT_PWM_STOP_SIGNAL_PRESENT        (0lu)
#define STEP_COUNT_PWM_SWITCH_SIGNAL_PRESENT      (0lu)

/* Interrupt Mask */
#define STEP_COUNT_PWM_INTERRUPT_MASK             (1lu)


/***************************************
*    Initial Parameter Constants
***************************************/

/* Timer/Counter Mode */
#define STEP_COUNT_TC_PERIOD_VALUE                (65535lu)
#define STEP_COUNT_TC_COMPARE_VALUE               (65535lu)
#define STEP_COUNT_TC_COMPARE_BUF_VALUE           (65535lu)
#define STEP_COUNT_TC_COMPARE_SWAP                (0lu)

/* PWM Mode */
#define STEP_COUNT_PWM_PERIOD_VALUE               (65535lu)
#define STEP_COUNT_PWM_PERIOD_BUF_VALUE           (65535lu)
#define STEP_COUNT_PWM_PERIOD_SWAP                (0lu)
#define STEP_COUNT_PWM_COMPARE_VALUE              (65535lu)
#define STEP_COUNT_PWM_COMPARE_BUF_VALUE          (65535lu)
#define STEP_COUNT_PWM_COMPARE_SWAP               (0lu)


/***************************************
*    Enumerated Types and Parameters
***************************************/

#define STEP_COUNT__LEFT 0
#define STEP_COUNT__RIGHT 1
#define STEP_COUNT__CENTER 2
#define STEP_COUNT__ASYMMETRIC 3

#define STEP_COUNT__X1 0
#define STEP_COUNT__X2 1
#define STEP_COUNT__X4 2

#define STEP_COUNT__PWM 4
#define STEP_COUNT__PWM_DT 5
#define STEP_COUNT__PWM_PR 6

#define STEP_COUNT__INVERSE 1
#define STEP_COUNT__DIRECT 0

#define STEP_COUNT__CAPTURE 2
#define STEP_COUNT__COMPARE 0

#define STEP_COUNT__TRIG_LEVEL 3
#define STEP_COUNT__TRIG_RISING 0
#define STEP_COUNT__TRIG_FALLING 1
#define STEP_COUNT__TRIG_BOTH 2

#define STEP_COUNT__INTR_MASK_TC 1
#define STEP_COUNT__INTR_MASK_CC_MATCH 2
#define STEP_COUNT__INTR_MASK_NONE 0
#define STEP_COUNT__INTR_MASK_TC_CC 3

#define STEP_COUNT__UNCONFIG 8
#define STEP_COUNT__TIMER 1
#define STEP_COUNT__QUAD 3
#define STEP_COUNT__PWM_SEL 7

#define STEP_COUNT__COUNT_UP 0
#define STEP_COUNT__COUNT_DOWN 1
#define STEP_COUNT__COUNT_UPDOWN0 2
#define STEP_COUNT__COUNT_UPDOWN1 3


/* Prescaler */
#define STEP_COUNT_PRESCALE_DIVBY1                ((uint32)(0u << STEP_COUNT_PRESCALER_SHIFT))
#define STEP_COUNT_PRESCALE_DIVBY2                ((uint32)(1u << STEP_COUNT_PRESCALER_SHIFT))
#define STEP_COUNT_PRESCALE_DIVBY4                ((uint32)(2u << STEP_COUNT_PRESCALER_SHIFT))
#define STEP_COUNT_PRESCALE_DIVBY8                ((uint32)(3u << STEP_COUNT_PRESCALER_SHIFT))
#define STEP_COUNT_PRESCALE_DIVBY16               ((uint32)(4u << STEP_COUNT_PRESCALER_SHIFT))
#define STEP_COUNT_PRESCALE_DIVBY32               ((uint32)(5u << STEP_COUNT_PRESCALER_SHIFT))
#define STEP_COUNT_PRESCALE_DIVBY64               ((uint32)(6u << STEP_COUNT_PRESCALER_SHIFT))
#define STEP_COUNT_PRESCALE_DIVBY128              ((uint32)(7u << STEP_COUNT_PRESCALER_SHIFT))

/* TCPWM set modes */
#define STEP_COUNT_MODE_TIMER_COMPARE             ((uint32)(STEP_COUNT__COMPARE         <<  \
                                                                  STEP_COUNT_MODE_SHIFT))
#define STEP_COUNT_MODE_TIMER_CAPTURE             ((uint32)(STEP_COUNT__CAPTURE         <<  \
                                                                  STEP_COUNT_MODE_SHIFT))
#define STEP_COUNT_MODE_QUAD                      ((uint32)(STEP_COUNT__QUAD            <<  \
                                                                  STEP_COUNT_MODE_SHIFT))
#define STEP_COUNT_MODE_PWM                       ((uint32)(STEP_COUNT__PWM             <<  \
                                                                  STEP_COUNT_MODE_SHIFT))
#define STEP_COUNT_MODE_PWM_DT                    ((uint32)(STEP_COUNT__PWM_DT          <<  \
                                                                  STEP_COUNT_MODE_SHIFT))
#define STEP_COUNT_MODE_PWM_PR                    ((uint32)(STEP_COUNT__PWM_PR          <<  \
                                                                  STEP_COUNT_MODE_SHIFT))

/* Quad Modes */
#define STEP_COUNT_MODE_X1                        ((uint32)(STEP_COUNT__X1              <<  \
                                                                  STEP_COUNT_QUAD_MODE_SHIFT))
#define STEP_COUNT_MODE_X2                        ((uint32)(STEP_COUNT__X2              <<  \
                                                                  STEP_COUNT_QUAD_MODE_SHIFT))
#define STEP_COUNT_MODE_X4                        ((uint32)(STEP_COUNT__X4              <<  \
                                                                  STEP_COUNT_QUAD_MODE_SHIFT))

/* Counter modes */
#define STEP_COUNT_COUNT_UP                       ((uint32)(STEP_COUNT__COUNT_UP        <<  \
                                                                  STEP_COUNT_UPDOWN_SHIFT))
#define STEP_COUNT_COUNT_DOWN                     ((uint32)(STEP_COUNT__COUNT_DOWN      <<  \
                                                                  STEP_COUNT_UPDOWN_SHIFT))
#define STEP_COUNT_COUNT_UPDOWN0                  ((uint32)(STEP_COUNT__COUNT_UPDOWN0   <<  \
                                                                  STEP_COUNT_UPDOWN_SHIFT))
#define STEP_COUNT_COUNT_UPDOWN1                  ((uint32)(STEP_COUNT__COUNT_UPDOWN1   <<  \
                                                                  STEP_COUNT_UPDOWN_SHIFT))

/* PWM output invert */
#define STEP_COUNT_INVERT_LINE                    ((uint32)(STEP_COUNT__INVERSE         <<  \
                                                                  STEP_COUNT_INV_OUT_SHIFT))
#define STEP_COUNT_INVERT_LINE_N                  ((uint32)(STEP_COUNT__INVERSE         <<  \
                                                                  STEP_COUNT_INV_COMPL_OUT_SHIFT))

/* Trigger modes */
#define STEP_COUNT_TRIG_RISING                    ((uint32)STEP_COUNT__TRIG_RISING)
#define STEP_COUNT_TRIG_FALLING                   ((uint32)STEP_COUNT__TRIG_FALLING)
#define STEP_COUNT_TRIG_BOTH                      ((uint32)STEP_COUNT__TRIG_BOTH)
#define STEP_COUNT_TRIG_LEVEL                     ((uint32)STEP_COUNT__TRIG_LEVEL)

/* Interrupt mask */
#define STEP_COUNT_INTR_MASK_TC                   ((uint32)STEP_COUNT__INTR_MASK_TC)
#define STEP_COUNT_INTR_MASK_CC_MATCH             ((uint32)STEP_COUNT__INTR_MASK_CC_MATCH)

/* PWM Output Controls */
#define STEP_COUNT_CC_MATCH_SET                   (0x00u)
#define STEP_COUNT_CC_MATCH_CLEAR                 (0x01u)
#define STEP_COUNT_CC_MATCH_INVERT                (0x02u)
#define STEP_COUNT_CC_MATCH_NO_CHANGE             (0x03u)
#define STEP_COUNT_OVERLOW_SET                    (0x00u)
#define STEP_COUNT_OVERLOW_CLEAR                  (0x04u)
#define STEP_COUNT_OVERLOW_INVERT                 (0x08u)
#define STEP_COUNT_OVERLOW_NO_CHANGE              (0x0Cu)
#define STEP_COUNT_UNDERFLOW_SET                  (0x00u)
#define STEP_COUNT_UNDERFLOW_CLEAR                (0x10u)
#define STEP_COUNT_UNDERFLOW_INVERT               (0x20u)
#define STEP_COUNT_UNDERFLOW_NO_CHANGE            (0x30u)

/* PWM Align */
#define STEP_COUNT_PWM_MODE_LEFT                  (STEP_COUNT_CC_MATCH_CLEAR        |   \
                                                         STEP_COUNT_OVERLOW_SET           |   \
                                                         STEP_COUNT_UNDERFLOW_NO_CHANGE)
#define STEP_COUNT_PWM_MODE_RIGHT                 (STEP_COUNT_CC_MATCH_SET          |   \
                                                         STEP_COUNT_OVERLOW_NO_CHANGE     |   \
                                                         STEP_COUNT_UNDERFLOW_CLEAR)
#define STEP_COUNT_PWM_MODE_ASYM                  (STEP_COUNT_CC_MATCH_INVERT       |   \
                                                         STEP_COUNT_OVERLOW_SET           |   \
                                                         STEP_COUNT_UNDERFLOW_CLEAR)

#if (STEP_COUNT_CY_TCPWM_V2)
    #if(STEP_COUNT_CY_TCPWM_4000)
        #define STEP_COUNT_PWM_MODE_CENTER                (STEP_COUNT_CC_MATCH_INVERT       |   \
                                                                 STEP_COUNT_OVERLOW_NO_CHANGE     |   \
                                                                 STEP_COUNT_UNDERFLOW_CLEAR)
    #else
        #define STEP_COUNT_PWM_MODE_CENTER                (STEP_COUNT_CC_MATCH_INVERT       |   \
                                                                 STEP_COUNT_OVERLOW_SET           |   \
                                                                 STEP_COUNT_UNDERFLOW_CLEAR)
    #endif /* (STEP_COUNT_CY_TCPWM_4000) */
#else
    #define STEP_COUNT_PWM_MODE_CENTER                (STEP_COUNT_CC_MATCH_INVERT       |   \
                                                             STEP_COUNT_OVERLOW_NO_CHANGE     |   \
                                                             STEP_COUNT_UNDERFLOW_CLEAR)
#endif /* (STEP_COUNT_CY_TCPWM_NEW) */

/* Command operations without condition */
#define STEP_COUNT_CMD_CAPTURE                    (0u)
#define STEP_COUNT_CMD_RELOAD                     (8u)
#define STEP_COUNT_CMD_STOP                       (16u)
#define STEP_COUNT_CMD_START                      (24u)

/* Status */
#define STEP_COUNT_STATUS_DOWN                    (1u)
#define STEP_COUNT_STATUS_RUNNING                 (2u)


/***************************************
*        Function Prototypes
****************************************/

void   STEP_COUNT_Init(void);
void   STEP_COUNT_Enable(void);
void   STEP_COUNT_Start(void);
void   STEP_COUNT_Stop(void);

void   STEP_COUNT_SetMode(uint32 mode);
void   STEP_COUNT_SetCounterMode(uint32 counterMode);
void   STEP_COUNT_SetPWMMode(uint32 modeMask);
void   STEP_COUNT_SetQDMode(uint32 qdMode);

void   STEP_COUNT_SetPrescaler(uint32 prescaler);
void   STEP_COUNT_TriggerCommand(uint32 mask, uint32 command);
void   STEP_COUNT_SetOneShot(uint32 oneShotEnable);
uint32 STEP_COUNT_ReadStatus(void);

void   STEP_COUNT_SetPWMSyncKill(uint32 syncKillEnable);
void   STEP_COUNT_SetPWMStopOnKill(uint32 stopOnKillEnable);
void   STEP_COUNT_SetPWMDeadTime(uint32 deadTime);
void   STEP_COUNT_SetPWMInvert(uint32 mask);

void   STEP_COUNT_SetInterruptMode(uint32 interruptMask);
uint32 STEP_COUNT_GetInterruptSourceMasked(void);
uint32 STEP_COUNT_GetInterruptSource(void);
void   STEP_COUNT_ClearInterrupt(uint32 interruptMask);
void   STEP_COUNT_SetInterrupt(uint32 interruptMask);

void   STEP_COUNT_WriteCounter(uint32 count);
uint32 STEP_COUNT_ReadCounter(void);

uint32 STEP_COUNT_ReadCapture(void);
uint32 STEP_COUNT_ReadCaptureBuf(void);

void   STEP_COUNT_WritePeriod(uint32 period);
uint32 STEP_COUNT_ReadPeriod(void);
void   STEP_COUNT_WritePeriodBuf(uint32 periodBuf);
uint32 STEP_COUNT_ReadPeriodBuf(void);

void   STEP_COUNT_WriteCompare(uint32 compare);
uint32 STEP_COUNT_ReadCompare(void);
void   STEP_COUNT_WriteCompareBuf(uint32 compareBuf);
uint32 STEP_COUNT_ReadCompareBuf(void);

void   STEP_COUNT_SetPeriodSwap(uint32 swapEnable);
void   STEP_COUNT_SetCompareSwap(uint32 swapEnable);

void   STEP_COUNT_SetCaptureMode(uint32 triggerMode);
void   STEP_COUNT_SetReloadMode(uint32 triggerMode);
void   STEP_COUNT_SetStartMode(uint32 triggerMode);
void   STEP_COUNT_SetStopMode(uint32 triggerMode);
void   STEP_COUNT_SetCountMode(uint32 triggerMode);

void   STEP_COUNT_SaveConfig(void);
void   STEP_COUNT_RestoreConfig(void);
void   STEP_COUNT_Sleep(void);
void   STEP_COUNT_Wakeup(void);


/***************************************
*             Registers
***************************************/

#define STEP_COUNT_BLOCK_CONTROL_REG              (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__TCPWM_CTRL )
#define STEP_COUNT_BLOCK_CONTROL_PTR              ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__TCPWM_CTRL )
#define STEP_COUNT_COMMAND_REG                    (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__TCPWM_CMD )
#define STEP_COUNT_COMMAND_PTR                    ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__TCPWM_CMD )
#define STEP_COUNT_INTRRUPT_CAUSE_REG             (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__TCPWM_INTR_CAUSE )
#define STEP_COUNT_INTRRUPT_CAUSE_PTR             ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__TCPWM_INTR_CAUSE )
#define STEP_COUNT_CONTROL_REG                    (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__CTRL )
#define STEP_COUNT_CONTROL_PTR                    ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__CTRL )
#define STEP_COUNT_STATUS_REG                     (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__STATUS )
#define STEP_COUNT_STATUS_PTR                     ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__STATUS )
#define STEP_COUNT_COUNTER_REG                    (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__COUNTER )
#define STEP_COUNT_COUNTER_PTR                    ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__COUNTER )
#define STEP_COUNT_COMP_CAP_REG                   (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__CC )
#define STEP_COUNT_COMP_CAP_PTR                   ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__CC )
#define STEP_COUNT_COMP_CAP_BUF_REG               (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__CC_BUFF )
#define STEP_COUNT_COMP_CAP_BUF_PTR               ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__CC_BUFF )
#define STEP_COUNT_PERIOD_REG                     (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__PERIOD )
#define STEP_COUNT_PERIOD_PTR                     ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__PERIOD )
#define STEP_COUNT_PERIOD_BUF_REG                 (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__PERIOD_BUFF )
#define STEP_COUNT_PERIOD_BUF_PTR                 ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__PERIOD_BUFF )
#define STEP_COUNT_TRIG_CONTROL0_REG              (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__TR_CTRL0 )
#define STEP_COUNT_TRIG_CONTROL0_PTR              ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__TR_CTRL0 )
#define STEP_COUNT_TRIG_CONTROL1_REG              (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__TR_CTRL1 )
#define STEP_COUNT_TRIG_CONTROL1_PTR              ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__TR_CTRL1 )
#define STEP_COUNT_TRIG_CONTROL2_REG              (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__TR_CTRL2 )
#define STEP_COUNT_TRIG_CONTROL2_PTR              ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__TR_CTRL2 )
#define STEP_COUNT_INTERRUPT_REQ_REG              (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__INTR )
#define STEP_COUNT_INTERRUPT_REQ_PTR              ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__INTR )
#define STEP_COUNT_INTERRUPT_SET_REG              (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__INTR_SET )
#define STEP_COUNT_INTERRUPT_SET_PTR              ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__INTR_SET )
#define STEP_COUNT_INTERRUPT_MASK_REG             (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__INTR_MASK )
#define STEP_COUNT_INTERRUPT_MASK_PTR             ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__INTR_MASK )
#define STEP_COUNT_INTERRUPT_MASKED_REG           (*(reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__INTR_MASKED )
#define STEP_COUNT_INTERRUPT_MASKED_PTR           ( (reg32 *) STEP_COUNT_cy_m0s8_tcpwm_1__INTR_MASKED )


/***************************************
*       Registers Constants
***************************************/

/* Mask */
#define STEP_COUNT_MASK                           ((uint32)STEP_COUNT_cy_m0s8_tcpwm_1__TCPWM_CTRL_MASK)

/* Shift constants for control register */
#define STEP_COUNT_RELOAD_CC_SHIFT                (0u)
#define STEP_COUNT_RELOAD_PERIOD_SHIFT            (1u)
#define STEP_COUNT_PWM_SYNC_KILL_SHIFT            (2u)
#define STEP_COUNT_PWM_STOP_KILL_SHIFT            (3u)
#define STEP_COUNT_PRESCALER_SHIFT                (8u)
#define STEP_COUNT_UPDOWN_SHIFT                   (16u)
#define STEP_COUNT_ONESHOT_SHIFT                  (18u)
#define STEP_COUNT_QUAD_MODE_SHIFT                (20u)
#define STEP_COUNT_INV_OUT_SHIFT                  (20u)
#define STEP_COUNT_INV_COMPL_OUT_SHIFT            (21u)
#define STEP_COUNT_MODE_SHIFT                     (24u)

/* Mask constants for control register */
#define STEP_COUNT_RELOAD_CC_MASK                 ((uint32)(STEP_COUNT_1BIT_MASK        <<  \
                                                                            STEP_COUNT_RELOAD_CC_SHIFT))
#define STEP_COUNT_RELOAD_PERIOD_MASK             ((uint32)(STEP_COUNT_1BIT_MASK        <<  \
                                                                            STEP_COUNT_RELOAD_PERIOD_SHIFT))
#define STEP_COUNT_PWM_SYNC_KILL_MASK             ((uint32)(STEP_COUNT_1BIT_MASK        <<  \
                                                                            STEP_COUNT_PWM_SYNC_KILL_SHIFT))
#define STEP_COUNT_PWM_STOP_KILL_MASK             ((uint32)(STEP_COUNT_1BIT_MASK        <<  \
                                                                            STEP_COUNT_PWM_STOP_KILL_SHIFT))
#define STEP_COUNT_PRESCALER_MASK                 ((uint32)(STEP_COUNT_8BIT_MASK        <<  \
                                                                            STEP_COUNT_PRESCALER_SHIFT))
#define STEP_COUNT_UPDOWN_MASK                    ((uint32)(STEP_COUNT_2BIT_MASK        <<  \
                                                                            STEP_COUNT_UPDOWN_SHIFT))
#define STEP_COUNT_ONESHOT_MASK                   ((uint32)(STEP_COUNT_1BIT_MASK        <<  \
                                                                            STEP_COUNT_ONESHOT_SHIFT))
#define STEP_COUNT_QUAD_MODE_MASK                 ((uint32)(STEP_COUNT_3BIT_MASK        <<  \
                                                                            STEP_COUNT_QUAD_MODE_SHIFT))
#define STEP_COUNT_INV_OUT_MASK                   ((uint32)(STEP_COUNT_2BIT_MASK        <<  \
                                                                            STEP_COUNT_INV_OUT_SHIFT))
#define STEP_COUNT_MODE_MASK                      ((uint32)(STEP_COUNT_3BIT_MASK        <<  \
                                                                            STEP_COUNT_MODE_SHIFT))

/* Shift constants for trigger control register 1 */
#define STEP_COUNT_CAPTURE_SHIFT                  (0u)
#define STEP_COUNT_COUNT_SHIFT                    (2u)
#define STEP_COUNT_RELOAD_SHIFT                   (4u)
#define STEP_COUNT_STOP_SHIFT                     (6u)
#define STEP_COUNT_START_SHIFT                    (8u)

/* Mask constants for trigger control register 1 */
#define STEP_COUNT_CAPTURE_MASK                   ((uint32)(STEP_COUNT_2BIT_MASK        <<  \
                                                                  STEP_COUNT_CAPTURE_SHIFT))
#define STEP_COUNT_COUNT_MASK                     ((uint32)(STEP_COUNT_2BIT_MASK        <<  \
                                                                  STEP_COUNT_COUNT_SHIFT))
#define STEP_COUNT_RELOAD_MASK                    ((uint32)(STEP_COUNT_2BIT_MASK        <<  \
                                                                  STEP_COUNT_RELOAD_SHIFT))
#define STEP_COUNT_STOP_MASK                      ((uint32)(STEP_COUNT_2BIT_MASK        <<  \
                                                                  STEP_COUNT_STOP_SHIFT))
#define STEP_COUNT_START_MASK                     ((uint32)(STEP_COUNT_2BIT_MASK        <<  \
                                                                  STEP_COUNT_START_SHIFT))

/* MASK */
#define STEP_COUNT_1BIT_MASK                      ((uint32)0x01u)
#define STEP_COUNT_2BIT_MASK                      ((uint32)0x03u)
#define STEP_COUNT_3BIT_MASK                      ((uint32)0x07u)
#define STEP_COUNT_6BIT_MASK                      ((uint32)0x3Fu)
#define STEP_COUNT_8BIT_MASK                      ((uint32)0xFFu)
#define STEP_COUNT_16BIT_MASK                     ((uint32)0xFFFFu)

/* Shift constant for status register */
#define STEP_COUNT_RUNNING_STATUS_SHIFT           (30u)


/***************************************
*    Initial Constants
***************************************/

#define STEP_COUNT_CTRL_QUAD_BASE_CONFIG                                                          \
        (((uint32)(STEP_COUNT_QUAD_ENCODING_MODES     << STEP_COUNT_QUAD_MODE_SHIFT))       |\
         ((uint32)(STEP_COUNT_CONFIG                  << STEP_COUNT_MODE_SHIFT)))

#define STEP_COUNT_CTRL_PWM_BASE_CONFIG                                                           \
        (((uint32)(STEP_COUNT_PWM_STOP_EVENT          << STEP_COUNT_PWM_STOP_KILL_SHIFT))   |\
         ((uint32)(STEP_COUNT_PWM_OUT_INVERT          << STEP_COUNT_INV_OUT_SHIFT))         |\
         ((uint32)(STEP_COUNT_PWM_OUT_N_INVERT        << STEP_COUNT_INV_COMPL_OUT_SHIFT))   |\
         ((uint32)(STEP_COUNT_PWM_MODE                << STEP_COUNT_MODE_SHIFT)))

#define STEP_COUNT_CTRL_PWM_RUN_MODE                                                              \
            ((uint32)(STEP_COUNT_PWM_RUN_MODE         << STEP_COUNT_ONESHOT_SHIFT))
            
#define STEP_COUNT_CTRL_PWM_ALIGN                                                                 \
            ((uint32)(STEP_COUNT_PWM_ALIGN            << STEP_COUNT_UPDOWN_SHIFT))

#define STEP_COUNT_CTRL_PWM_KILL_EVENT                                                            \
             ((uint32)(STEP_COUNT_PWM_KILL_EVENT      << STEP_COUNT_PWM_SYNC_KILL_SHIFT))

#define STEP_COUNT_CTRL_PWM_DEAD_TIME_CYCLE                                                       \
            ((uint32)(STEP_COUNT_PWM_DEAD_TIME_CYCLE  << STEP_COUNT_PRESCALER_SHIFT))

#define STEP_COUNT_CTRL_PWM_PRESCALER                                                             \
            ((uint32)(STEP_COUNT_PWM_PRESCALER        << STEP_COUNT_PRESCALER_SHIFT))

#define STEP_COUNT_CTRL_TIMER_BASE_CONFIG                                                         \
        (((uint32)(STEP_COUNT_TC_PRESCALER            << STEP_COUNT_PRESCALER_SHIFT))       |\
         ((uint32)(STEP_COUNT_TC_COUNTER_MODE         << STEP_COUNT_UPDOWN_SHIFT))          |\
         ((uint32)(STEP_COUNT_TC_RUN_MODE             << STEP_COUNT_ONESHOT_SHIFT))         |\
         ((uint32)(STEP_COUNT_TC_COMP_CAP_MODE        << STEP_COUNT_MODE_SHIFT)))
        
#define STEP_COUNT_QUAD_SIGNALS_MODES                                                             \
        (((uint32)(STEP_COUNT_QUAD_PHIA_SIGNAL_MODE   << STEP_COUNT_COUNT_SHIFT))           |\
         ((uint32)(STEP_COUNT_QUAD_INDEX_SIGNAL_MODE  << STEP_COUNT_RELOAD_SHIFT))          |\
         ((uint32)(STEP_COUNT_QUAD_STOP_SIGNAL_MODE   << STEP_COUNT_STOP_SHIFT))            |\
         ((uint32)(STEP_COUNT_QUAD_PHIB_SIGNAL_MODE   << STEP_COUNT_START_SHIFT)))

#define STEP_COUNT_PWM_SIGNALS_MODES                                                              \
        (((uint32)(STEP_COUNT_PWM_SWITCH_SIGNAL_MODE  << STEP_COUNT_CAPTURE_SHIFT))         |\
         ((uint32)(STEP_COUNT_PWM_COUNT_SIGNAL_MODE   << STEP_COUNT_COUNT_SHIFT))           |\
         ((uint32)(STEP_COUNT_PWM_RELOAD_SIGNAL_MODE  << STEP_COUNT_RELOAD_SHIFT))          |\
         ((uint32)(STEP_COUNT_PWM_STOP_SIGNAL_MODE    << STEP_COUNT_STOP_SHIFT))            |\
         ((uint32)(STEP_COUNT_PWM_START_SIGNAL_MODE   << STEP_COUNT_START_SHIFT)))

#define STEP_COUNT_TIMER_SIGNALS_MODES                                                            \
        (((uint32)(STEP_COUNT_TC_CAPTURE_SIGNAL_MODE  << STEP_COUNT_CAPTURE_SHIFT))         |\
         ((uint32)(STEP_COUNT_TC_COUNT_SIGNAL_MODE    << STEP_COUNT_COUNT_SHIFT))           |\
         ((uint32)(STEP_COUNT_TC_RELOAD_SIGNAL_MODE   << STEP_COUNT_RELOAD_SHIFT))          |\
         ((uint32)(STEP_COUNT_TC_STOP_SIGNAL_MODE     << STEP_COUNT_STOP_SHIFT))            |\
         ((uint32)(STEP_COUNT_TC_START_SIGNAL_MODE    << STEP_COUNT_START_SHIFT)))
        
#define STEP_COUNT_TIMER_UPDOWN_CNT_USED                                                          \
                ((STEP_COUNT__COUNT_UPDOWN0 == STEP_COUNT_TC_COUNTER_MODE)                  ||\
                 (STEP_COUNT__COUNT_UPDOWN1 == STEP_COUNT_TC_COUNTER_MODE))

#define STEP_COUNT_PWM_UPDOWN_CNT_USED                                                            \
                ((STEP_COUNT__CENTER == STEP_COUNT_PWM_ALIGN)                               ||\
                 (STEP_COUNT__ASYMMETRIC == STEP_COUNT_PWM_ALIGN))               
        
#define STEP_COUNT_PWM_PR_INIT_VALUE              (1u)
#define STEP_COUNT_QUAD_PERIOD_INIT_VALUE         (0x8000u)



#endif /* End CY_TCPWM_STEP_COUNT_H */

/* [] END OF FILE */
