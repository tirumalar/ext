/*******************************************************************************
* File Name: PWM_DAC.h
* Version 2.10
*
* Description:
*  This file provides constants and parameter values for the PWM_DAC
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

#if !defined(CY_TCPWM_PWM_DAC_H)
#define CY_TCPWM_PWM_DAC_H


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
} PWM_DAC_BACKUP_STRUCT;


/*******************************************************************************
* Variables
*******************************************************************************/
extern uint8  PWM_DAC_initVar;


/***************************************
*   Conditional Compilation Parameters
****************************************/

#define PWM_DAC_CY_TCPWM_V2                    (CYIPBLOCK_m0s8tcpwm_VERSION == 2u)
#define PWM_DAC_CY_TCPWM_4000                  (CY_PSOC4_4000)

/* TCPWM Configuration */
#define PWM_DAC_CONFIG                         (7lu)

/* Quad Mode */
/* Parameters */
#define PWM_DAC_QUAD_ENCODING_MODES            (0lu)
#define PWM_DAC_QUAD_AUTO_START                (1lu)

/* Signal modes */
#define PWM_DAC_QUAD_INDEX_SIGNAL_MODE         (0lu)
#define PWM_DAC_QUAD_PHIA_SIGNAL_MODE          (3lu)
#define PWM_DAC_QUAD_PHIB_SIGNAL_MODE          (3lu)
#define PWM_DAC_QUAD_STOP_SIGNAL_MODE          (0lu)

/* Signal present */
#define PWM_DAC_QUAD_INDEX_SIGNAL_PRESENT      (0lu)
#define PWM_DAC_QUAD_STOP_SIGNAL_PRESENT       (0lu)

/* Interrupt Mask */
#define PWM_DAC_QUAD_INTERRUPT_MASK            (1lu)

/* Timer/Counter Mode */
/* Parameters */
#define PWM_DAC_TC_RUN_MODE                    (0lu)
#define PWM_DAC_TC_COUNTER_MODE                (0lu)
#define PWM_DAC_TC_COMP_CAP_MODE               (2lu)
#define PWM_DAC_TC_PRESCALER                   (0lu)

/* Signal modes */
#define PWM_DAC_TC_RELOAD_SIGNAL_MODE          (0lu)
#define PWM_DAC_TC_COUNT_SIGNAL_MODE           (3lu)
#define PWM_DAC_TC_START_SIGNAL_MODE           (0lu)
#define PWM_DAC_TC_STOP_SIGNAL_MODE            (0lu)
#define PWM_DAC_TC_CAPTURE_SIGNAL_MODE         (0lu)

/* Signal present */
#define PWM_DAC_TC_RELOAD_SIGNAL_PRESENT       (0lu)
#define PWM_DAC_TC_COUNT_SIGNAL_PRESENT        (0lu)
#define PWM_DAC_TC_START_SIGNAL_PRESENT        (0lu)
#define PWM_DAC_TC_STOP_SIGNAL_PRESENT         (0lu)
#define PWM_DAC_TC_CAPTURE_SIGNAL_PRESENT      (0lu)

/* Interrupt Mask */
#define PWM_DAC_TC_INTERRUPT_MASK              (1lu)

/* PWM Mode */
/* Parameters */
#define PWM_DAC_PWM_KILL_EVENT                 (0lu)
#define PWM_DAC_PWM_STOP_EVENT                 (0lu)
#define PWM_DAC_PWM_MODE                       (4lu)
#define PWM_DAC_PWM_OUT_N_INVERT               (0lu)
#define PWM_DAC_PWM_OUT_INVERT                 (0lu)
#define PWM_DAC_PWM_ALIGN                      (0lu)
#define PWM_DAC_PWM_RUN_MODE                   (0lu)
#define PWM_DAC_PWM_DEAD_TIME_CYCLE            (0lu)
#define PWM_DAC_PWM_PRESCALER                  (0lu)

/* Signal modes */
#define PWM_DAC_PWM_RELOAD_SIGNAL_MODE         (0lu)
#define PWM_DAC_PWM_COUNT_SIGNAL_MODE          (3lu)
#define PWM_DAC_PWM_START_SIGNAL_MODE          (0lu)
#define PWM_DAC_PWM_STOP_SIGNAL_MODE           (0lu)
#define PWM_DAC_PWM_SWITCH_SIGNAL_MODE         (0lu)

/* Signal present */
#define PWM_DAC_PWM_RELOAD_SIGNAL_PRESENT      (0lu)
#define PWM_DAC_PWM_COUNT_SIGNAL_PRESENT       (0lu)
#define PWM_DAC_PWM_START_SIGNAL_PRESENT       (0lu)
#define PWM_DAC_PWM_STOP_SIGNAL_PRESENT        (0lu)
#define PWM_DAC_PWM_SWITCH_SIGNAL_PRESENT      (0lu)

/* Interrupt Mask */
#define PWM_DAC_PWM_INTERRUPT_MASK             (1lu)


/***************************************
*    Initial Parameter Constants
***************************************/

/* Timer/Counter Mode */
#define PWM_DAC_TC_PERIOD_VALUE                (65535lu)
#define PWM_DAC_TC_COMPARE_VALUE               (65535lu)
#define PWM_DAC_TC_COMPARE_BUF_VALUE           (65535lu)
#define PWM_DAC_TC_COMPARE_SWAP                (0lu)

/* PWM Mode */
#define PWM_DAC_PWM_PERIOD_VALUE               (600lu)
#define PWM_DAC_PWM_PERIOD_BUF_VALUE           (65535lu)
#define PWM_DAC_PWM_PERIOD_SWAP                (0lu)
#define PWM_DAC_PWM_COMPARE_VALUE              (5lu)
#define PWM_DAC_PWM_COMPARE_BUF_VALUE          (65535lu)
#define PWM_DAC_PWM_COMPARE_SWAP               (0lu)


/***************************************
*    Enumerated Types and Parameters
***************************************/

#define PWM_DAC__LEFT 0
#define PWM_DAC__RIGHT 1
#define PWM_DAC__CENTER 2
#define PWM_DAC__ASYMMETRIC 3

#define PWM_DAC__X1 0
#define PWM_DAC__X2 1
#define PWM_DAC__X4 2

#define PWM_DAC__PWM 4
#define PWM_DAC__PWM_DT 5
#define PWM_DAC__PWM_PR 6

#define PWM_DAC__INVERSE 1
#define PWM_DAC__DIRECT 0

#define PWM_DAC__CAPTURE 2
#define PWM_DAC__COMPARE 0

#define PWM_DAC__TRIG_LEVEL 3
#define PWM_DAC__TRIG_RISING 0
#define PWM_DAC__TRIG_FALLING 1
#define PWM_DAC__TRIG_BOTH 2

#define PWM_DAC__INTR_MASK_TC 1
#define PWM_DAC__INTR_MASK_CC_MATCH 2
#define PWM_DAC__INTR_MASK_NONE 0
#define PWM_DAC__INTR_MASK_TC_CC 3

#define PWM_DAC__UNCONFIG 8
#define PWM_DAC__TIMER 1
#define PWM_DAC__QUAD 3
#define PWM_DAC__PWM_SEL 7

#define PWM_DAC__COUNT_UP 0
#define PWM_DAC__COUNT_DOWN 1
#define PWM_DAC__COUNT_UPDOWN0 2
#define PWM_DAC__COUNT_UPDOWN1 3


/* Prescaler */
#define PWM_DAC_PRESCALE_DIVBY1                ((uint32)(0u << PWM_DAC_PRESCALER_SHIFT))
#define PWM_DAC_PRESCALE_DIVBY2                ((uint32)(1u << PWM_DAC_PRESCALER_SHIFT))
#define PWM_DAC_PRESCALE_DIVBY4                ((uint32)(2u << PWM_DAC_PRESCALER_SHIFT))
#define PWM_DAC_PRESCALE_DIVBY8                ((uint32)(3u << PWM_DAC_PRESCALER_SHIFT))
#define PWM_DAC_PRESCALE_DIVBY16               ((uint32)(4u << PWM_DAC_PRESCALER_SHIFT))
#define PWM_DAC_PRESCALE_DIVBY32               ((uint32)(5u << PWM_DAC_PRESCALER_SHIFT))
#define PWM_DAC_PRESCALE_DIVBY64               ((uint32)(6u << PWM_DAC_PRESCALER_SHIFT))
#define PWM_DAC_PRESCALE_DIVBY128              ((uint32)(7u << PWM_DAC_PRESCALER_SHIFT))

/* TCPWM set modes */
#define PWM_DAC_MODE_TIMER_COMPARE             ((uint32)(PWM_DAC__COMPARE         <<  \
                                                                  PWM_DAC_MODE_SHIFT))
#define PWM_DAC_MODE_TIMER_CAPTURE             ((uint32)(PWM_DAC__CAPTURE         <<  \
                                                                  PWM_DAC_MODE_SHIFT))
#define PWM_DAC_MODE_QUAD                      ((uint32)(PWM_DAC__QUAD            <<  \
                                                                  PWM_DAC_MODE_SHIFT))
#define PWM_DAC_MODE_PWM                       ((uint32)(PWM_DAC__PWM             <<  \
                                                                  PWM_DAC_MODE_SHIFT))
#define PWM_DAC_MODE_PWM_DT                    ((uint32)(PWM_DAC__PWM_DT          <<  \
                                                                  PWM_DAC_MODE_SHIFT))
#define PWM_DAC_MODE_PWM_PR                    ((uint32)(PWM_DAC__PWM_PR          <<  \
                                                                  PWM_DAC_MODE_SHIFT))

/* Quad Modes */
#define PWM_DAC_MODE_X1                        ((uint32)(PWM_DAC__X1              <<  \
                                                                  PWM_DAC_QUAD_MODE_SHIFT))
#define PWM_DAC_MODE_X2                        ((uint32)(PWM_DAC__X2              <<  \
                                                                  PWM_DAC_QUAD_MODE_SHIFT))
#define PWM_DAC_MODE_X4                        ((uint32)(PWM_DAC__X4              <<  \
                                                                  PWM_DAC_QUAD_MODE_SHIFT))

/* Counter modes */
#define PWM_DAC_COUNT_UP                       ((uint32)(PWM_DAC__COUNT_UP        <<  \
                                                                  PWM_DAC_UPDOWN_SHIFT))
#define PWM_DAC_COUNT_DOWN                     ((uint32)(PWM_DAC__COUNT_DOWN      <<  \
                                                                  PWM_DAC_UPDOWN_SHIFT))
#define PWM_DAC_COUNT_UPDOWN0                  ((uint32)(PWM_DAC__COUNT_UPDOWN0   <<  \
                                                                  PWM_DAC_UPDOWN_SHIFT))
#define PWM_DAC_COUNT_UPDOWN1                  ((uint32)(PWM_DAC__COUNT_UPDOWN1   <<  \
                                                                  PWM_DAC_UPDOWN_SHIFT))

/* PWM output invert */
#define PWM_DAC_INVERT_LINE                    ((uint32)(PWM_DAC__INVERSE         <<  \
                                                                  PWM_DAC_INV_OUT_SHIFT))
#define PWM_DAC_INVERT_LINE_N                  ((uint32)(PWM_DAC__INVERSE         <<  \
                                                                  PWM_DAC_INV_COMPL_OUT_SHIFT))

/* Trigger modes */
#define PWM_DAC_TRIG_RISING                    ((uint32)PWM_DAC__TRIG_RISING)
#define PWM_DAC_TRIG_FALLING                   ((uint32)PWM_DAC__TRIG_FALLING)
#define PWM_DAC_TRIG_BOTH                      ((uint32)PWM_DAC__TRIG_BOTH)
#define PWM_DAC_TRIG_LEVEL                     ((uint32)PWM_DAC__TRIG_LEVEL)

/* Interrupt mask */
#define PWM_DAC_INTR_MASK_TC                   ((uint32)PWM_DAC__INTR_MASK_TC)
#define PWM_DAC_INTR_MASK_CC_MATCH             ((uint32)PWM_DAC__INTR_MASK_CC_MATCH)

/* PWM Output Controls */
#define PWM_DAC_CC_MATCH_SET                   (0x00u)
#define PWM_DAC_CC_MATCH_CLEAR                 (0x01u)
#define PWM_DAC_CC_MATCH_INVERT                (0x02u)
#define PWM_DAC_CC_MATCH_NO_CHANGE             (0x03u)
#define PWM_DAC_OVERLOW_SET                    (0x00u)
#define PWM_DAC_OVERLOW_CLEAR                  (0x04u)
#define PWM_DAC_OVERLOW_INVERT                 (0x08u)
#define PWM_DAC_OVERLOW_NO_CHANGE              (0x0Cu)
#define PWM_DAC_UNDERFLOW_SET                  (0x00u)
#define PWM_DAC_UNDERFLOW_CLEAR                (0x10u)
#define PWM_DAC_UNDERFLOW_INVERT               (0x20u)
#define PWM_DAC_UNDERFLOW_NO_CHANGE            (0x30u)

/* PWM Align */
#define PWM_DAC_PWM_MODE_LEFT                  (PWM_DAC_CC_MATCH_CLEAR        |   \
                                                         PWM_DAC_OVERLOW_SET           |   \
                                                         PWM_DAC_UNDERFLOW_NO_CHANGE)
#define PWM_DAC_PWM_MODE_RIGHT                 (PWM_DAC_CC_MATCH_SET          |   \
                                                         PWM_DAC_OVERLOW_NO_CHANGE     |   \
                                                         PWM_DAC_UNDERFLOW_CLEAR)
#define PWM_DAC_PWM_MODE_ASYM                  (PWM_DAC_CC_MATCH_INVERT       |   \
                                                         PWM_DAC_OVERLOW_SET           |   \
                                                         PWM_DAC_UNDERFLOW_CLEAR)

#if (PWM_DAC_CY_TCPWM_V2)
    #if(PWM_DAC_CY_TCPWM_4000)
        #define PWM_DAC_PWM_MODE_CENTER                (PWM_DAC_CC_MATCH_INVERT       |   \
                                                                 PWM_DAC_OVERLOW_NO_CHANGE     |   \
                                                                 PWM_DAC_UNDERFLOW_CLEAR)
    #else
        #define PWM_DAC_PWM_MODE_CENTER                (PWM_DAC_CC_MATCH_INVERT       |   \
                                                                 PWM_DAC_OVERLOW_SET           |   \
                                                                 PWM_DAC_UNDERFLOW_CLEAR)
    #endif /* (PWM_DAC_CY_TCPWM_4000) */
#else
    #define PWM_DAC_PWM_MODE_CENTER                (PWM_DAC_CC_MATCH_INVERT       |   \
                                                             PWM_DAC_OVERLOW_NO_CHANGE     |   \
                                                             PWM_DAC_UNDERFLOW_CLEAR)
#endif /* (PWM_DAC_CY_TCPWM_NEW) */

/* Command operations without condition */
#define PWM_DAC_CMD_CAPTURE                    (0u)
#define PWM_DAC_CMD_RELOAD                     (8u)
#define PWM_DAC_CMD_STOP                       (16u)
#define PWM_DAC_CMD_START                      (24u)

/* Status */
#define PWM_DAC_STATUS_DOWN                    (1u)
#define PWM_DAC_STATUS_RUNNING                 (2u)


/***************************************
*        Function Prototypes
****************************************/

void   PWM_DAC_Init(void);
void   PWM_DAC_Enable(void);
void   PWM_DAC_Start(void);
void   PWM_DAC_Stop(void);

void   PWM_DAC_SetMode(uint32 mode);
void   PWM_DAC_SetCounterMode(uint32 counterMode);
void   PWM_DAC_SetPWMMode(uint32 modeMask);
void   PWM_DAC_SetQDMode(uint32 qdMode);

void   PWM_DAC_SetPrescaler(uint32 prescaler);
void   PWM_DAC_TriggerCommand(uint32 mask, uint32 command);
void   PWM_DAC_SetOneShot(uint32 oneShotEnable);
uint32 PWM_DAC_ReadStatus(void);

void   PWM_DAC_SetPWMSyncKill(uint32 syncKillEnable);
void   PWM_DAC_SetPWMStopOnKill(uint32 stopOnKillEnable);
void   PWM_DAC_SetPWMDeadTime(uint32 deadTime);
void   PWM_DAC_SetPWMInvert(uint32 mask);

void   PWM_DAC_SetInterruptMode(uint32 interruptMask);
uint32 PWM_DAC_GetInterruptSourceMasked(void);
uint32 PWM_DAC_GetInterruptSource(void);
void   PWM_DAC_ClearInterrupt(uint32 interruptMask);
void   PWM_DAC_SetInterrupt(uint32 interruptMask);

void   PWM_DAC_WriteCounter(uint32 count);
uint32 PWM_DAC_ReadCounter(void);

uint32 PWM_DAC_ReadCapture(void);
uint32 PWM_DAC_ReadCaptureBuf(void);

void   PWM_DAC_WritePeriod(uint32 period);
uint32 PWM_DAC_ReadPeriod(void);
void   PWM_DAC_WritePeriodBuf(uint32 periodBuf);
uint32 PWM_DAC_ReadPeriodBuf(void);

void   PWM_DAC_WriteCompare(uint32 compare);
uint32 PWM_DAC_ReadCompare(void);
void   PWM_DAC_WriteCompareBuf(uint32 compareBuf);
uint32 PWM_DAC_ReadCompareBuf(void);

void   PWM_DAC_SetPeriodSwap(uint32 swapEnable);
void   PWM_DAC_SetCompareSwap(uint32 swapEnable);

void   PWM_DAC_SetCaptureMode(uint32 triggerMode);
void   PWM_DAC_SetReloadMode(uint32 triggerMode);
void   PWM_DAC_SetStartMode(uint32 triggerMode);
void   PWM_DAC_SetStopMode(uint32 triggerMode);
void   PWM_DAC_SetCountMode(uint32 triggerMode);

void   PWM_DAC_SaveConfig(void);
void   PWM_DAC_RestoreConfig(void);
void   PWM_DAC_Sleep(void);
void   PWM_DAC_Wakeup(void);


/***************************************
*             Registers
***************************************/

#define PWM_DAC_BLOCK_CONTROL_REG              (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__TCPWM_CTRL )
#define PWM_DAC_BLOCK_CONTROL_PTR              ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__TCPWM_CTRL )
#define PWM_DAC_COMMAND_REG                    (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__TCPWM_CMD )
#define PWM_DAC_COMMAND_PTR                    ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__TCPWM_CMD )
#define PWM_DAC_INTRRUPT_CAUSE_REG             (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__TCPWM_INTR_CAUSE )
#define PWM_DAC_INTRRUPT_CAUSE_PTR             ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__TCPWM_INTR_CAUSE )
#define PWM_DAC_CONTROL_REG                    (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__CTRL )
#define PWM_DAC_CONTROL_PTR                    ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__CTRL )
#define PWM_DAC_STATUS_REG                     (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__STATUS )
#define PWM_DAC_STATUS_PTR                     ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__STATUS )
#define PWM_DAC_COUNTER_REG                    (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__COUNTER )
#define PWM_DAC_COUNTER_PTR                    ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__COUNTER )
#define PWM_DAC_COMP_CAP_REG                   (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__CC )
#define PWM_DAC_COMP_CAP_PTR                   ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__CC )
#define PWM_DAC_COMP_CAP_BUF_REG               (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__CC_BUFF )
#define PWM_DAC_COMP_CAP_BUF_PTR               ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__CC_BUFF )
#define PWM_DAC_PERIOD_REG                     (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__PERIOD )
#define PWM_DAC_PERIOD_PTR                     ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__PERIOD )
#define PWM_DAC_PERIOD_BUF_REG                 (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__PERIOD_BUFF )
#define PWM_DAC_PERIOD_BUF_PTR                 ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__PERIOD_BUFF )
#define PWM_DAC_TRIG_CONTROL0_REG              (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__TR_CTRL0 )
#define PWM_DAC_TRIG_CONTROL0_PTR              ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__TR_CTRL0 )
#define PWM_DAC_TRIG_CONTROL1_REG              (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__TR_CTRL1 )
#define PWM_DAC_TRIG_CONTROL1_PTR              ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__TR_CTRL1 )
#define PWM_DAC_TRIG_CONTROL2_REG              (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__TR_CTRL2 )
#define PWM_DAC_TRIG_CONTROL2_PTR              ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__TR_CTRL2 )
#define PWM_DAC_INTERRUPT_REQ_REG              (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__INTR )
#define PWM_DAC_INTERRUPT_REQ_PTR              ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__INTR )
#define PWM_DAC_INTERRUPT_SET_REG              (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__INTR_SET )
#define PWM_DAC_INTERRUPT_SET_PTR              ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__INTR_SET )
#define PWM_DAC_INTERRUPT_MASK_REG             (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__INTR_MASK )
#define PWM_DAC_INTERRUPT_MASK_PTR             ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__INTR_MASK )
#define PWM_DAC_INTERRUPT_MASKED_REG           (*(reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__INTR_MASKED )
#define PWM_DAC_INTERRUPT_MASKED_PTR           ( (reg32 *) PWM_DAC_cy_m0s8_tcpwm_1__INTR_MASKED )


/***************************************
*       Registers Constants
***************************************/

/* Mask */
#define PWM_DAC_MASK                           ((uint32)PWM_DAC_cy_m0s8_tcpwm_1__TCPWM_CTRL_MASK)

/* Shift constants for control register */
#define PWM_DAC_RELOAD_CC_SHIFT                (0u)
#define PWM_DAC_RELOAD_PERIOD_SHIFT            (1u)
#define PWM_DAC_PWM_SYNC_KILL_SHIFT            (2u)
#define PWM_DAC_PWM_STOP_KILL_SHIFT            (3u)
#define PWM_DAC_PRESCALER_SHIFT                (8u)
#define PWM_DAC_UPDOWN_SHIFT                   (16u)
#define PWM_DAC_ONESHOT_SHIFT                  (18u)
#define PWM_DAC_QUAD_MODE_SHIFT                (20u)
#define PWM_DAC_INV_OUT_SHIFT                  (20u)
#define PWM_DAC_INV_COMPL_OUT_SHIFT            (21u)
#define PWM_DAC_MODE_SHIFT                     (24u)

/* Mask constants for control register */
#define PWM_DAC_RELOAD_CC_MASK                 ((uint32)(PWM_DAC_1BIT_MASK        <<  \
                                                                            PWM_DAC_RELOAD_CC_SHIFT))
#define PWM_DAC_RELOAD_PERIOD_MASK             ((uint32)(PWM_DAC_1BIT_MASK        <<  \
                                                                            PWM_DAC_RELOAD_PERIOD_SHIFT))
#define PWM_DAC_PWM_SYNC_KILL_MASK             ((uint32)(PWM_DAC_1BIT_MASK        <<  \
                                                                            PWM_DAC_PWM_SYNC_KILL_SHIFT))
#define PWM_DAC_PWM_STOP_KILL_MASK             ((uint32)(PWM_DAC_1BIT_MASK        <<  \
                                                                            PWM_DAC_PWM_STOP_KILL_SHIFT))
#define PWM_DAC_PRESCALER_MASK                 ((uint32)(PWM_DAC_8BIT_MASK        <<  \
                                                                            PWM_DAC_PRESCALER_SHIFT))
#define PWM_DAC_UPDOWN_MASK                    ((uint32)(PWM_DAC_2BIT_MASK        <<  \
                                                                            PWM_DAC_UPDOWN_SHIFT))
#define PWM_DAC_ONESHOT_MASK                   ((uint32)(PWM_DAC_1BIT_MASK        <<  \
                                                                            PWM_DAC_ONESHOT_SHIFT))
#define PWM_DAC_QUAD_MODE_MASK                 ((uint32)(PWM_DAC_3BIT_MASK        <<  \
                                                                            PWM_DAC_QUAD_MODE_SHIFT))
#define PWM_DAC_INV_OUT_MASK                   ((uint32)(PWM_DAC_2BIT_MASK        <<  \
                                                                            PWM_DAC_INV_OUT_SHIFT))
#define PWM_DAC_MODE_MASK                      ((uint32)(PWM_DAC_3BIT_MASK        <<  \
                                                                            PWM_DAC_MODE_SHIFT))

/* Shift constants for trigger control register 1 */
#define PWM_DAC_CAPTURE_SHIFT                  (0u)
#define PWM_DAC_COUNT_SHIFT                    (2u)
#define PWM_DAC_RELOAD_SHIFT                   (4u)
#define PWM_DAC_STOP_SHIFT                     (6u)
#define PWM_DAC_START_SHIFT                    (8u)

/* Mask constants for trigger control register 1 */
#define PWM_DAC_CAPTURE_MASK                   ((uint32)(PWM_DAC_2BIT_MASK        <<  \
                                                                  PWM_DAC_CAPTURE_SHIFT))
#define PWM_DAC_COUNT_MASK                     ((uint32)(PWM_DAC_2BIT_MASK        <<  \
                                                                  PWM_DAC_COUNT_SHIFT))
#define PWM_DAC_RELOAD_MASK                    ((uint32)(PWM_DAC_2BIT_MASK        <<  \
                                                                  PWM_DAC_RELOAD_SHIFT))
#define PWM_DAC_STOP_MASK                      ((uint32)(PWM_DAC_2BIT_MASK        <<  \
                                                                  PWM_DAC_STOP_SHIFT))
#define PWM_DAC_START_MASK                     ((uint32)(PWM_DAC_2BIT_MASK        <<  \
                                                                  PWM_DAC_START_SHIFT))

/* MASK */
#define PWM_DAC_1BIT_MASK                      ((uint32)0x01u)
#define PWM_DAC_2BIT_MASK                      ((uint32)0x03u)
#define PWM_DAC_3BIT_MASK                      ((uint32)0x07u)
#define PWM_DAC_6BIT_MASK                      ((uint32)0x3Fu)
#define PWM_DAC_8BIT_MASK                      ((uint32)0xFFu)
#define PWM_DAC_16BIT_MASK                     ((uint32)0xFFFFu)

/* Shift constant for status register */
#define PWM_DAC_RUNNING_STATUS_SHIFT           (30u)


/***************************************
*    Initial Constants
***************************************/

#define PWM_DAC_CTRL_QUAD_BASE_CONFIG                                                          \
        (((uint32)(PWM_DAC_QUAD_ENCODING_MODES     << PWM_DAC_QUAD_MODE_SHIFT))       |\
         ((uint32)(PWM_DAC_CONFIG                  << PWM_DAC_MODE_SHIFT)))

#define PWM_DAC_CTRL_PWM_BASE_CONFIG                                                           \
        (((uint32)(PWM_DAC_PWM_STOP_EVENT          << PWM_DAC_PWM_STOP_KILL_SHIFT))   |\
         ((uint32)(PWM_DAC_PWM_OUT_INVERT          << PWM_DAC_INV_OUT_SHIFT))         |\
         ((uint32)(PWM_DAC_PWM_OUT_N_INVERT        << PWM_DAC_INV_COMPL_OUT_SHIFT))   |\
         ((uint32)(PWM_DAC_PWM_MODE                << PWM_DAC_MODE_SHIFT)))

#define PWM_DAC_CTRL_PWM_RUN_MODE                                                              \
            ((uint32)(PWM_DAC_PWM_RUN_MODE         << PWM_DAC_ONESHOT_SHIFT))
            
#define PWM_DAC_CTRL_PWM_ALIGN                                                                 \
            ((uint32)(PWM_DAC_PWM_ALIGN            << PWM_DAC_UPDOWN_SHIFT))

#define PWM_DAC_CTRL_PWM_KILL_EVENT                                                            \
             ((uint32)(PWM_DAC_PWM_KILL_EVENT      << PWM_DAC_PWM_SYNC_KILL_SHIFT))

#define PWM_DAC_CTRL_PWM_DEAD_TIME_CYCLE                                                       \
            ((uint32)(PWM_DAC_PWM_DEAD_TIME_CYCLE  << PWM_DAC_PRESCALER_SHIFT))

#define PWM_DAC_CTRL_PWM_PRESCALER                                                             \
            ((uint32)(PWM_DAC_PWM_PRESCALER        << PWM_DAC_PRESCALER_SHIFT))

#define PWM_DAC_CTRL_TIMER_BASE_CONFIG                                                         \
        (((uint32)(PWM_DAC_TC_PRESCALER            << PWM_DAC_PRESCALER_SHIFT))       |\
         ((uint32)(PWM_DAC_TC_COUNTER_MODE         << PWM_DAC_UPDOWN_SHIFT))          |\
         ((uint32)(PWM_DAC_TC_RUN_MODE             << PWM_DAC_ONESHOT_SHIFT))         |\
         ((uint32)(PWM_DAC_TC_COMP_CAP_MODE        << PWM_DAC_MODE_SHIFT)))
        
#define PWM_DAC_QUAD_SIGNALS_MODES                                                             \
        (((uint32)(PWM_DAC_QUAD_PHIA_SIGNAL_MODE   << PWM_DAC_COUNT_SHIFT))           |\
         ((uint32)(PWM_DAC_QUAD_INDEX_SIGNAL_MODE  << PWM_DAC_RELOAD_SHIFT))          |\
         ((uint32)(PWM_DAC_QUAD_STOP_SIGNAL_MODE   << PWM_DAC_STOP_SHIFT))            |\
         ((uint32)(PWM_DAC_QUAD_PHIB_SIGNAL_MODE   << PWM_DAC_START_SHIFT)))

#define PWM_DAC_PWM_SIGNALS_MODES                                                              \
        (((uint32)(PWM_DAC_PWM_SWITCH_SIGNAL_MODE  << PWM_DAC_CAPTURE_SHIFT))         |\
         ((uint32)(PWM_DAC_PWM_COUNT_SIGNAL_MODE   << PWM_DAC_COUNT_SHIFT))           |\
         ((uint32)(PWM_DAC_PWM_RELOAD_SIGNAL_MODE  << PWM_DAC_RELOAD_SHIFT))          |\
         ((uint32)(PWM_DAC_PWM_STOP_SIGNAL_MODE    << PWM_DAC_STOP_SHIFT))            |\
         ((uint32)(PWM_DAC_PWM_START_SIGNAL_MODE   << PWM_DAC_START_SHIFT)))

#define PWM_DAC_TIMER_SIGNALS_MODES                                                            \
        (((uint32)(PWM_DAC_TC_CAPTURE_SIGNAL_MODE  << PWM_DAC_CAPTURE_SHIFT))         |\
         ((uint32)(PWM_DAC_TC_COUNT_SIGNAL_MODE    << PWM_DAC_COUNT_SHIFT))           |\
         ((uint32)(PWM_DAC_TC_RELOAD_SIGNAL_MODE   << PWM_DAC_RELOAD_SHIFT))          |\
         ((uint32)(PWM_DAC_TC_STOP_SIGNAL_MODE     << PWM_DAC_STOP_SHIFT))            |\
         ((uint32)(PWM_DAC_TC_START_SIGNAL_MODE    << PWM_DAC_START_SHIFT)))
        
#define PWM_DAC_TIMER_UPDOWN_CNT_USED                                                          \
                ((PWM_DAC__COUNT_UPDOWN0 == PWM_DAC_TC_COUNTER_MODE)                  ||\
                 (PWM_DAC__COUNT_UPDOWN1 == PWM_DAC_TC_COUNTER_MODE))

#define PWM_DAC_PWM_UPDOWN_CNT_USED                                                            \
                ((PWM_DAC__CENTER == PWM_DAC_PWM_ALIGN)                               ||\
                 (PWM_DAC__ASYMMETRIC == PWM_DAC_PWM_ALIGN))               
        
#define PWM_DAC_PWM_PR_INIT_VALUE              (1u)
#define PWM_DAC_QUAD_PERIOD_INIT_VALUE         (0x8000u)



#endif /* End CY_TCPWM_PWM_DAC_H */

/* [] END OF FILE */
