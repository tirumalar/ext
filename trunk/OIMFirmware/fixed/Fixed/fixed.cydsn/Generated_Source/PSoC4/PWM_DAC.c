/*******************************************************************************
* File Name: PWM_DAC.c
* Version 2.10
*
* Description:
*  This file provides the source code to the API for the PWM_DAC
*  component
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

#include "PWM_DAC.h"

uint8 PWM_DAC_initVar = 0u;


/*******************************************************************************
* Function Name: PWM_DAC_Init
********************************************************************************
*
* Summary:
*  Initialize/Restore default PWM_DAC configuration.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_Init(void)
{

    /* Set values from customizer to CTRL */
    #if (PWM_DAC__QUAD == PWM_DAC_CONFIG)
        PWM_DAC_CONTROL_REG = PWM_DAC_CTRL_QUAD_BASE_CONFIG;
        
        /* Set values from customizer to CTRL1 */
        PWM_DAC_TRIG_CONTROL1_REG  = PWM_DAC_QUAD_SIGNALS_MODES;

        /* Set values from customizer to INTR */
        PWM_DAC_SetInterruptMode(PWM_DAC_QUAD_INTERRUPT_MASK);
        
         /* Set other values */
        PWM_DAC_SetCounterMode(PWM_DAC_COUNT_DOWN);
        PWM_DAC_WritePeriod(PWM_DAC_QUAD_PERIOD_INIT_VALUE);
        PWM_DAC_WriteCounter(PWM_DAC_QUAD_PERIOD_INIT_VALUE);
    #endif  /* (PWM_DAC__QUAD == PWM_DAC_CONFIG) */

    #if (PWM_DAC__TIMER == PWM_DAC_CONFIG)
        PWM_DAC_CONTROL_REG = PWM_DAC_CTRL_TIMER_BASE_CONFIG;
        
        /* Set values from customizer to CTRL1 */
        PWM_DAC_TRIG_CONTROL1_REG  = PWM_DAC_TIMER_SIGNALS_MODES;
    
        /* Set values from customizer to INTR */
        PWM_DAC_SetInterruptMode(PWM_DAC_TC_INTERRUPT_MASK);
        
        /* Set other values from customizer */
        PWM_DAC_WritePeriod(PWM_DAC_TC_PERIOD_VALUE );

        #if (PWM_DAC__COMPARE == PWM_DAC_TC_COMP_CAP_MODE)
            PWM_DAC_WriteCompare(PWM_DAC_TC_COMPARE_VALUE);

            #if (1u == PWM_DAC_TC_COMPARE_SWAP)
                PWM_DAC_SetCompareSwap(1u);
                PWM_DAC_WriteCompareBuf(PWM_DAC_TC_COMPARE_BUF_VALUE);
            #endif  /* (1u == PWM_DAC_TC_COMPARE_SWAP) */
        #endif  /* (PWM_DAC__COMPARE == PWM_DAC_TC_COMP_CAP_MODE) */

        /* Initialize counter value */
        #if (PWM_DAC_CY_TCPWM_V2 && PWM_DAC_TIMER_UPDOWN_CNT_USED && !PWM_DAC_CY_TCPWM_4000)
            PWM_DAC_WriteCounter(1u);
        #elif(PWM_DAC__COUNT_DOWN == PWM_DAC_TC_COUNTER_MODE)
            PWM_DAC_WriteCounter(PWM_DAC_TC_PERIOD_VALUE);
        #else
            PWM_DAC_WriteCounter(0u);
        #endif /* (PWM_DAC_CY_TCPWM_V2 && PWM_DAC_TIMER_UPDOWN_CNT_USED && !PWM_DAC_CY_TCPWM_4000) */
    #endif  /* (PWM_DAC__TIMER == PWM_DAC_CONFIG) */

    #if (PWM_DAC__PWM_SEL == PWM_DAC_CONFIG)
        PWM_DAC_CONTROL_REG = PWM_DAC_CTRL_PWM_BASE_CONFIG;

        #if (PWM_DAC__PWM_PR == PWM_DAC_PWM_MODE)
            PWM_DAC_CONTROL_REG |= PWM_DAC_CTRL_PWM_RUN_MODE;
            PWM_DAC_WriteCounter(PWM_DAC_PWM_PR_INIT_VALUE);
        #else
            PWM_DAC_CONTROL_REG |= PWM_DAC_CTRL_PWM_ALIGN | PWM_DAC_CTRL_PWM_KILL_EVENT;
            
            /* Initialize counter value */
            #if (PWM_DAC_CY_TCPWM_V2 && PWM_DAC_PWM_UPDOWN_CNT_USED && !PWM_DAC_CY_TCPWM_4000)
                PWM_DAC_WriteCounter(1u);
            #elif (PWM_DAC__RIGHT == PWM_DAC_PWM_ALIGN)
                PWM_DAC_WriteCounter(PWM_DAC_PWM_PERIOD_VALUE);
            #else 
                PWM_DAC_WriteCounter(0u);
            #endif  /* (PWM_DAC_CY_TCPWM_V2 && PWM_DAC_PWM_UPDOWN_CNT_USED && !PWM_DAC_CY_TCPWM_4000) */
        #endif  /* (PWM_DAC__PWM_PR == PWM_DAC_PWM_MODE) */

        #if (PWM_DAC__PWM_DT == PWM_DAC_PWM_MODE)
            PWM_DAC_CONTROL_REG |= PWM_DAC_CTRL_PWM_DEAD_TIME_CYCLE;
        #endif  /* (PWM_DAC__PWM_DT == PWM_DAC_PWM_MODE) */

        #if (PWM_DAC__PWM == PWM_DAC_PWM_MODE)
            PWM_DAC_CONTROL_REG |= PWM_DAC_CTRL_PWM_PRESCALER;
        #endif  /* (PWM_DAC__PWM == PWM_DAC_PWM_MODE) */

        /* Set values from customizer to CTRL1 */
        PWM_DAC_TRIG_CONTROL1_REG  = PWM_DAC_PWM_SIGNALS_MODES;
    
        /* Set values from customizer to INTR */
        PWM_DAC_SetInterruptMode(PWM_DAC_PWM_INTERRUPT_MASK);

        /* Set values from customizer to CTRL2 */
        #if (PWM_DAC__PWM_PR == PWM_DAC_PWM_MODE)
            PWM_DAC_TRIG_CONTROL2_REG =
                    (PWM_DAC_CC_MATCH_NO_CHANGE    |
                    PWM_DAC_OVERLOW_NO_CHANGE      |
                    PWM_DAC_UNDERFLOW_NO_CHANGE);
        #else
            #if (PWM_DAC__LEFT == PWM_DAC_PWM_ALIGN)
                PWM_DAC_TRIG_CONTROL2_REG = PWM_DAC_PWM_MODE_LEFT;
            #endif  /* ( PWM_DAC_PWM_LEFT == PWM_DAC_PWM_ALIGN) */

            #if (PWM_DAC__RIGHT == PWM_DAC_PWM_ALIGN)
                PWM_DAC_TRIG_CONTROL2_REG = PWM_DAC_PWM_MODE_RIGHT;
            #endif  /* ( PWM_DAC_PWM_RIGHT == PWM_DAC_PWM_ALIGN) */

            #if (PWM_DAC__CENTER == PWM_DAC_PWM_ALIGN)
                PWM_DAC_TRIG_CONTROL2_REG = PWM_DAC_PWM_MODE_CENTER;
            #endif  /* ( PWM_DAC_PWM_CENTER == PWM_DAC_PWM_ALIGN) */

            #if (PWM_DAC__ASYMMETRIC == PWM_DAC_PWM_ALIGN)
                PWM_DAC_TRIG_CONTROL2_REG = PWM_DAC_PWM_MODE_ASYM;
            #endif  /* (PWM_DAC__ASYMMETRIC == PWM_DAC_PWM_ALIGN) */
        #endif  /* (PWM_DAC__PWM_PR == PWM_DAC_PWM_MODE) */

        /* Set other values from customizer */
        PWM_DAC_WritePeriod(PWM_DAC_PWM_PERIOD_VALUE );
        PWM_DAC_WriteCompare(PWM_DAC_PWM_COMPARE_VALUE);

        #if (1u == PWM_DAC_PWM_COMPARE_SWAP)
            PWM_DAC_SetCompareSwap(1u);
            PWM_DAC_WriteCompareBuf(PWM_DAC_PWM_COMPARE_BUF_VALUE);
        #endif  /* (1u == PWM_DAC_PWM_COMPARE_SWAP) */

        #if (1u == PWM_DAC_PWM_PERIOD_SWAP)
            PWM_DAC_SetPeriodSwap(1u);
            PWM_DAC_WritePeriodBuf(PWM_DAC_PWM_PERIOD_BUF_VALUE);
        #endif  /* (1u == PWM_DAC_PWM_PERIOD_SWAP) */
    #endif  /* (PWM_DAC__PWM_SEL == PWM_DAC_CONFIG) */
    
}


/*******************************************************************************
* Function Name: PWM_DAC_Enable
********************************************************************************
*
* Summary:
*  Enables the PWM_DAC.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_Enable(void)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    PWM_DAC_BLOCK_CONTROL_REG |= PWM_DAC_MASK;
    CyExitCriticalSection(enableInterrupts);

    /* Start Timer or PWM if start input is absent */
    #if (PWM_DAC__PWM_SEL == PWM_DAC_CONFIG)
        #if (0u == PWM_DAC_PWM_START_SIGNAL_PRESENT)
            PWM_DAC_TriggerCommand(PWM_DAC_MASK, PWM_DAC_CMD_START);
        #endif /* (0u == PWM_DAC_PWM_START_SIGNAL_PRESENT) */
    #endif /* (PWM_DAC__PWM_SEL == PWM_DAC_CONFIG) */

    #if (PWM_DAC__TIMER == PWM_DAC_CONFIG)
        #if (0u == PWM_DAC_TC_START_SIGNAL_PRESENT)
            PWM_DAC_TriggerCommand(PWM_DAC_MASK, PWM_DAC_CMD_START);
        #endif /* (0u == PWM_DAC_TC_START_SIGNAL_PRESENT) */
    #endif /* (PWM_DAC__TIMER == PWM_DAC_CONFIG) */
    
    #if (PWM_DAC__QUAD == PWM_DAC_CONFIG)
        #if (0u != PWM_DAC_QUAD_AUTO_START)
            PWM_DAC_TriggerCommand(PWM_DAC_MASK, PWM_DAC_CMD_RELOAD);
        #endif /* (0u != PWM_DAC_QUAD_AUTO_START) */
    #endif  /* (PWM_DAC__QUAD == PWM_DAC_CONFIG) */
}


/*******************************************************************************
* Function Name: PWM_DAC_Start
********************************************************************************
*
* Summary:
*  Initializes the PWM_DAC with default customizer
*  values when called the first time and enables the PWM_DAC.
*  For subsequent calls the configuration is left unchanged and the component is
*  just enabled.
*
* Parameters:
*  None
*
* Return:
*  None
*
* Global variables:
*  PWM_DAC_initVar: global variable is used to indicate initial
*  configuration of this component.  The variable is initialized to zero and set
*  to 1 the first time PWM_DAC_Start() is called. This allows
*  enabling/disabling a component without re-initialization in all subsequent
*  calls to the PWM_DAC_Start() routine.
*
*******************************************************************************/
void PWM_DAC_Start(void)
{
    if (0u == PWM_DAC_initVar)
    {
        PWM_DAC_Init();
        PWM_DAC_initVar = 1u;
    }

    PWM_DAC_Enable();
}


/*******************************************************************************
* Function Name: PWM_DAC_Stop
********************************************************************************
*
* Summary:
*  Disables the PWM_DAC.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_Stop(void)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_BLOCK_CONTROL_REG &= (uint32)~PWM_DAC_MASK;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetMode
********************************************************************************
*
* Summary:
*  Sets the operation mode of the PWM_DAC. This function is used when
*  configured as a generic PWM_DAC and the actual mode of operation is
*  set at runtime. The mode must be set while the component is disabled.
*
* Parameters:
*  mode: Mode for the PWM_DAC to operate in
*   Values:
*   - PWM_DAC_MODE_TIMER_COMPARE - Timer / Counter with
*                                                 compare capability
*         - PWM_DAC_MODE_TIMER_CAPTURE - Timer / Counter with
*                                                 capture capability
*         - PWM_DAC_MODE_QUAD - Quadrature decoder
*         - PWM_DAC_MODE_PWM - PWM
*         - PWM_DAC_MODE_PWM_DT - PWM with dead time
*         - PWM_DAC_MODE_PWM_PR - PWM with pseudo random capability
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetMode(uint32 mode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_CONTROL_REG &= (uint32)~PWM_DAC_MODE_MASK;
    PWM_DAC_CONTROL_REG |= mode;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetQDMode
********************************************************************************
*
* Summary:
*  Sets the the Quadrature Decoder to one of the 3 supported modes.
*  Its functionality is only applicable to Quadrature Decoder operation.
*
* Parameters:
*  qdMode: Quadrature Decoder mode
*   Values:
*         - PWM_DAC_MODE_X1 - Counts on phi 1 rising
*         - PWM_DAC_MODE_X2 - Counts on both edges of phi1 (2x faster)
*         - PWM_DAC_MODE_X4 - Counts on both edges of phi1 and phi2
*                                        (4x faster)
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetQDMode(uint32 qdMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_CONTROL_REG &= (uint32)~PWM_DAC_QUAD_MODE_MASK;
    PWM_DAC_CONTROL_REG |= qdMode;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetPrescaler
********************************************************************************
*
* Summary:
*  Sets the prescaler value that is applied to the clock input.  Not applicable
*  to a PWM with the dead time mode or Quadrature Decoder mode.
*
* Parameters:
*  prescaler: Prescaler divider value
*   Values:
*         - PWM_DAC_PRESCALE_DIVBY1    - Divide by 1 (no prescaling)
*         - PWM_DAC_PRESCALE_DIVBY2    - Divide by 2
*         - PWM_DAC_PRESCALE_DIVBY4    - Divide by 4
*         - PWM_DAC_PRESCALE_DIVBY8    - Divide by 8
*         - PWM_DAC_PRESCALE_DIVBY16   - Divide by 16
*         - PWM_DAC_PRESCALE_DIVBY32   - Divide by 32
*         - PWM_DAC_PRESCALE_DIVBY64   - Divide by 64
*         - PWM_DAC_PRESCALE_DIVBY128  - Divide by 128
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetPrescaler(uint32 prescaler)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_CONTROL_REG &= (uint32)~PWM_DAC_PRESCALER_MASK;
    PWM_DAC_CONTROL_REG |= prescaler;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetOneShot
********************************************************************************
*
* Summary:
*  Writes the register that controls whether the PWM_DAC runs
*  continuously or stops when terminal count is reached.  By default the
*  PWM_DAC operates in the continuous mode.
*
* Parameters:
*  oneShotEnable
*   Values:
*     - 0 - Continuous
*     - 1 - Enable One Shot
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetOneShot(uint32 oneShotEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_CONTROL_REG &= (uint32)~PWM_DAC_ONESHOT_MASK;
    PWM_DAC_CONTROL_REG |= ((uint32)((oneShotEnable & PWM_DAC_1BIT_MASK) <<
                                                               PWM_DAC_ONESHOT_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetPWMMode
********************************************************************************
*
* Summary:
*  Writes the control register that determines what mode of operation the PWM
*  output lines are driven in.  There is a setting for what to do on a
*  comparison match (CC_MATCH), on an overflow (OVERFLOW) and on an underflow
*  (UNDERFLOW).  The value for each of the three must be ORed together to form
*  the mode.
*
* Parameters:
*  modeMask: A combination of three mode settings.  Mask must include a value
*  for each of the three or use one of the preconfigured PWM settings.
*   Values:
*     - CC_MATCH_SET        - Set on comparison match
*     - CC_MATCH_CLEAR      - Clear on comparison match
*     - CC_MATCH_INVERT     - Invert on comparison match
*     - CC_MATCH_NO_CHANGE  - No change on comparison match
*     - OVERLOW_SET         - Set on overflow
*     - OVERLOW_CLEAR       - Clear on  overflow
*     - OVERLOW_INVERT      - Invert on overflow
*     - OVERLOW_NO_CHANGE   - No change on overflow
*     - UNDERFLOW_SET       - Set on underflow
*     - UNDERFLOW_CLEAR     - Clear on underflow
*     - UNDERFLOW_INVERT    - Invert on underflow
*     - UNDERFLOW_NO_CHANGE - No change on underflow
*     - PWM_MODE_LEFT       - Setting for left aligned PWM.  Should be combined
*                             with up counting mode
*     - PWM_MODE_RIGHT      - Setting for right aligned PWM.  Should be combined
*                             with down counting mode
*     - PWM_MODE_CENTER     - Setting for center aligned PWM.  Should be
*                             combined with up/down 0 mode
*     - PWM_MODE_ASYM       - Setting for asymmetric PWM.  Should be combined
*                             with up/down 1 mode
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetPWMMode(uint32 modeMask)
{
    PWM_DAC_TRIG_CONTROL2_REG = (modeMask & PWM_DAC_6BIT_MASK);
}



/*******************************************************************************
* Function Name: PWM_DAC_SetPWMSyncKill
********************************************************************************
*
* Summary:
*  Writes the register that controls whether the PWM kill signal (stop input)
*  causes asynchronous or synchronous kill operation.  By default the kill
*  operation is asynchronous.  This functionality is only applicable to the PWM
*  and PWM with dead time modes.
*
*  For Synchronous mode the kill signal disables both the line and line_n
*  signals until the next terminal count.
*
*  For Asynchronous mode the kill signal disables both the line and line_n
*  signals when the kill signal is present.  This mode should only be used
*  when the kill signal (stop input) is configured in the pass through mode
*  (Level sensitive signal).

*
* Parameters:
*  syncKillEnable
*   Values:
*     - 0 - Asynchronous
*     - 1 - Synchronous
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetPWMSyncKill(uint32 syncKillEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_CONTROL_REG &= (uint32)~PWM_DAC_PWM_SYNC_KILL_MASK;
    PWM_DAC_CONTROL_REG |= ((uint32)((syncKillEnable & PWM_DAC_1BIT_MASK)  <<
                                               PWM_DAC_PWM_SYNC_KILL_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetPWMStopOnKill
********************************************************************************
*
* Summary:
*  Writes the register that controls whether the PWM kill signal (stop input)
*  causes the PWM counter to stop.  By default the kill operation does not stop
*  the counter.  This functionality is only applicable to the three PWM modes.
*
*
* Parameters:
*  stopOnKillEnable
*   Values:
*     - 0 - Don't stop
*     - 1 - Stop
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetPWMStopOnKill(uint32 stopOnKillEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_CONTROL_REG &= (uint32)~PWM_DAC_PWM_STOP_KILL_MASK;
    PWM_DAC_CONTROL_REG |= ((uint32)((stopOnKillEnable & PWM_DAC_1BIT_MASK)  <<
                                                         PWM_DAC_PWM_STOP_KILL_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetPWMDeadTime
********************************************************************************
*
* Summary:
*  Writes the dead time control value.  This value delays the rising edge of
*  both the line and line_n signals the designated number of cycles resulting
*  in both signals being inactive for that many cycles.  This functionality is
*  only applicable to the PWM in the dead time mode.

*
* Parameters:
*  Dead time to insert
*   Values: 0 to 255
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetPWMDeadTime(uint32 deadTime)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_CONTROL_REG &= (uint32)~PWM_DAC_PRESCALER_MASK;
    PWM_DAC_CONTROL_REG |= ((uint32)((deadTime & PWM_DAC_8BIT_MASK) <<
                                                          PWM_DAC_PRESCALER_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetPWMInvert
********************************************************************************
*
* Summary:
*  Writes the bits that control whether the line and line_n outputs are
*  inverted from their normal output values.  This functionality is only
*  applicable to the three PWM modes.
*
* Parameters:
*  mask: Mask of outputs to invert.
*   Values:
*         - PWM_DAC_INVERT_LINE   - Inverts the line output
*         - PWM_DAC_INVERT_LINE_N - Inverts the line_n output
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetPWMInvert(uint32 mask)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_CONTROL_REG &= (uint32)~PWM_DAC_INV_OUT_MASK;
    PWM_DAC_CONTROL_REG |= mask;

    CyExitCriticalSection(enableInterrupts);
}



/*******************************************************************************
* Function Name: PWM_DAC_WriteCounter
********************************************************************************
*
* Summary:
*  Writes a new 16bit counter value directly into the counter register, thus
*  setting the counter (not the period) to the value written. It is not
*  advised to write to this field when the counter is running.
*
* Parameters:
*  count: value to write
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_WriteCounter(uint32 count)
{
    PWM_DAC_COUNTER_REG = (count & PWM_DAC_16BIT_MASK);
}


/*******************************************************************************
* Function Name: PWM_DAC_ReadCounter
********************************************************************************
*
* Summary:
*  Reads the current counter value.
*
* Parameters:
*  None
*
* Return:
*  Current counter value
*
*******************************************************************************/
uint32 PWM_DAC_ReadCounter(void)
{
    return (PWM_DAC_COUNTER_REG & PWM_DAC_16BIT_MASK);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetCounterMode
********************************************************************************
*
* Summary:
*  Sets the counter mode.  Applicable to all modes except Quadrature Decoder
*  and the PWM with a pseudo random output.
*
* Parameters:
*  counterMode: Enumerated counter type values
*   Values:
*     - PWM_DAC_COUNT_UP       - Counts up
*     - PWM_DAC_COUNT_DOWN     - Counts down
*     - PWM_DAC_COUNT_UPDOWN0  - Counts up and down. Terminal count
*                                         generated when counter reaches 0
*     - PWM_DAC_COUNT_UPDOWN1  - Counts up and down. Terminal count
*                                         generated both when counter reaches 0
*                                         and period
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetCounterMode(uint32 counterMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_CONTROL_REG &= (uint32)~PWM_DAC_UPDOWN_MASK;
    PWM_DAC_CONTROL_REG |= counterMode;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_WritePeriod
********************************************************************************
*
* Summary:
*  Writes the 16 bit period register with the new period value.
*  To cause the counter to count for N cycles this register should be written
*  with N-1 (counts from 0 to period inclusive).
*
* Parameters:
*  period: Period value
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_WritePeriod(uint32 period)
{
    PWM_DAC_PERIOD_REG = (period & PWM_DAC_16BIT_MASK);
}


/*******************************************************************************
* Function Name: PWM_DAC_ReadPeriod
********************************************************************************
*
* Summary:
*  Reads the 16 bit period register.
*
* Parameters:
*  None
*
* Return:
*  Period value
*
*******************************************************************************/
uint32 PWM_DAC_ReadPeriod(void)
{
    return (PWM_DAC_PERIOD_REG & PWM_DAC_16BIT_MASK);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetCompareSwap
********************************************************************************
*
* Summary:
*  Writes the register that controls whether the compare registers are
*  swapped. When enabled in the Timer/Counter mode(without capture) the swap
*  occurs at a TC event. In the PWM mode the swap occurs at the next TC event
*  following a hardware switch event.
*
* Parameters:
*  swapEnable
*   Values:
*     - 0 - Disable swap
*     - 1 - Enable swap
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetCompareSwap(uint32 swapEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_CONTROL_REG &= (uint32)~PWM_DAC_RELOAD_CC_MASK;
    PWM_DAC_CONTROL_REG |= (swapEnable & PWM_DAC_1BIT_MASK);

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_WritePeriodBuf
********************************************************************************
*
* Summary:
*  Writes the 16 bit period buf register with the new period value.
*
* Parameters:
*  periodBuf: Period value
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_WritePeriodBuf(uint32 periodBuf)
{
    PWM_DAC_PERIOD_BUF_REG = (periodBuf & PWM_DAC_16BIT_MASK);
}


/*******************************************************************************
* Function Name: PWM_DAC_ReadPeriodBuf
********************************************************************************
*
* Summary:
*  Reads the 16 bit period buf register.
*
* Parameters:
*  None
*
* Return:
*  Period value
*
*******************************************************************************/
uint32 PWM_DAC_ReadPeriodBuf(void)
{
    return (PWM_DAC_PERIOD_BUF_REG & PWM_DAC_16BIT_MASK);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetPeriodSwap
********************************************************************************
*
* Summary:
*  Writes the register that controls whether the period registers are
*  swapped. When enabled in Timer/Counter mode the swap occurs at a TC event.
*  In the PWM mode the swap occurs at the next TC event following a hardware
*  switch event.
*
* Parameters:
*  swapEnable
*   Values:
*     - 0 - Disable swap
*     - 1 - Enable swap
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetPeriodSwap(uint32 swapEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_CONTROL_REG &= (uint32)~PWM_DAC_RELOAD_PERIOD_MASK;
    PWM_DAC_CONTROL_REG |= ((uint32)((swapEnable & PWM_DAC_1BIT_MASK) <<
                                                            PWM_DAC_RELOAD_PERIOD_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_WriteCompare
********************************************************************************
*
* Summary:
*  Writes the 16 bit compare register with the new compare value. Not
*  applicable for Timer/Counter with Capture or in Quadrature Decoder modes.
*
* Parameters:
*  compare: Compare value
*
* Return:
*  None
*
* Note:
*  It is not recommended to use the value equal to "0" or equal to 
*  "period value" in Center or Asymmetric align PWM modes on the 
*  PSoC 4100/PSoC 4200 devices.
*  PSoC 4000 devices write the 16 bit compare register with the decremented 
*  compare value in the Up counting mode (except 0x0u), and the incremented 
*  compare value in the Down counting mode (except 0xFFFFu).
*
*******************************************************************************/
void PWM_DAC_WriteCompare(uint32 compare)
{
    #if (PWM_DAC_CY_TCPWM_4000)
        uint32 currentMode;
    #endif /* (PWM_DAC_CY_TCPWM_4000) */

    #if (PWM_DAC_CY_TCPWM_4000)
        currentMode = ((PWM_DAC_CONTROL_REG & PWM_DAC_UPDOWN_MASK) >> PWM_DAC_UPDOWN_SHIFT);

        if (((uint32)PWM_DAC__COUNT_DOWN == currentMode) && (0xFFFFu != compare))
        {
            compare++;
        }
        else if (((uint32)PWM_DAC__COUNT_UP == currentMode) && (0u != compare))
        {
            compare--;
        }
        else
        {
        }
        
    
    #endif /* (PWM_DAC_CY_TCPWM_4000) */
    
    PWM_DAC_COMP_CAP_REG = (compare & PWM_DAC_16BIT_MASK);
}


/*******************************************************************************
* Function Name: PWM_DAC_ReadCompare
********************************************************************************
*
* Summary:
*  Reads the compare register. Not applicable for Timer/Counter with Capture
*  or in Quadrature Decoder modes.
*  PSoC 4000 devices read the incremented compare register value in the 
*  Up counting mode (except 0xFFFFu), and the decremented value in the 
*  Down counting mode (except 0x0u).
*
* Parameters:
*  None
*
* Return:
*  Compare value
*
* Note:
*  PSoC 4000 devices read the incremented compare register value in the 
*  Up counting mode (except 0xFFFFu), and the decremented value in the 
*  Down counting mode (except 0x0u).
*
*******************************************************************************/
uint32 PWM_DAC_ReadCompare(void)
{
    #if (PWM_DAC_CY_TCPWM_4000)
        uint32 currentMode;
        uint32 regVal;
    #endif /* (PWM_DAC_CY_TCPWM_4000) */

    #if (PWM_DAC_CY_TCPWM_4000)
        currentMode = ((PWM_DAC_CONTROL_REG & PWM_DAC_UPDOWN_MASK) >> PWM_DAC_UPDOWN_SHIFT);
        
        regVal = PWM_DAC_COMP_CAP_REG;
        
        if (((uint32)PWM_DAC__COUNT_DOWN == currentMode) && (0u != regVal))
        {
            regVal--;
        }
        else if (((uint32)PWM_DAC__COUNT_UP == currentMode) && (0xFFFFu != regVal))
        {
            regVal++;
        }
        else
        {
        }

        return (regVal & PWM_DAC_16BIT_MASK);
    #else
        return (PWM_DAC_COMP_CAP_REG & PWM_DAC_16BIT_MASK);
    #endif /* (PWM_DAC_CY_TCPWM_4000) */
}


/*******************************************************************************
* Function Name: PWM_DAC_WriteCompareBuf
********************************************************************************
*
* Summary:
*  Writes the 16 bit compare buffer register with the new compare value. Not
*  applicable for Timer/Counter with Capture or in Quadrature Decoder modes.
*
* Parameters:
*  compareBuf: Compare value
*
* Return:
*  None
*
* Note:
*  It is not recommended to use the value equal to "0" or equal to 
*  "period value" in Center or Asymmetric align PWM modes on the 
*  PSoC 4100/PSoC 4200 devices.
*  PSoC 4000 devices write the 16 bit compare register with the decremented 
*  compare value in the Up counting mode (except 0x0u), and the incremented 
*  compare value in the Down counting mode (except 0xFFFFu).
*
*******************************************************************************/
void PWM_DAC_WriteCompareBuf(uint32 compareBuf)
{
    #if (PWM_DAC_CY_TCPWM_4000)
        uint32 currentMode;
    #endif /* (PWM_DAC_CY_TCPWM_4000) */

    #if (PWM_DAC_CY_TCPWM_4000)
        currentMode = ((PWM_DAC_CONTROL_REG & PWM_DAC_UPDOWN_MASK) >> PWM_DAC_UPDOWN_SHIFT);

        if (((uint32)PWM_DAC__COUNT_DOWN == currentMode) && (0xFFFFu != compareBuf))
        {
            compareBuf++;
        }
        else if (((uint32)PWM_DAC__COUNT_UP == currentMode) && (0u != compareBuf))
        {
            compareBuf --;
        }
        else
        {
        }
    #endif /* (PWM_DAC_CY_TCPWM_4000) */
    
    PWM_DAC_COMP_CAP_BUF_REG = (compareBuf & PWM_DAC_16BIT_MASK);
}


/*******************************************************************************
* Function Name: PWM_DAC_ReadCompareBuf
********************************************************************************
*
* Summary:
*  Reads the compare buffer register. Not applicable for Timer/Counter with
*  Capture or in Quadrature Decoder modes.
*
* Parameters:
*  None
*
* Return:
*  Compare buffer value
*
* Note:
*  PSoC 4000 devices read the incremented compare register value in the 
*  Up counting mode (except 0xFFFFu), and the decremented value in the 
*  Down counting mode (except 0x0u).
*
*******************************************************************************/
uint32 PWM_DAC_ReadCompareBuf(void)
{
    #if (PWM_DAC_CY_TCPWM_4000)
        uint32 currentMode;
        uint32 regVal;
    #endif /* (PWM_DAC_CY_TCPWM_4000) */

    #if (PWM_DAC_CY_TCPWM_4000)
        currentMode = ((PWM_DAC_CONTROL_REG & PWM_DAC_UPDOWN_MASK) >> PWM_DAC_UPDOWN_SHIFT);

        regVal = PWM_DAC_COMP_CAP_BUF_REG;
        
        if (((uint32)PWM_DAC__COUNT_DOWN == currentMode) && (0u != regVal))
        {
            regVal--;
        }
        else if (((uint32)PWM_DAC__COUNT_UP == currentMode) && (0xFFFFu != regVal))
        {
            regVal++;
        }
        else
        {
        }

        return (regVal & PWM_DAC_16BIT_MASK);
    #else
        return (PWM_DAC_COMP_CAP_BUF_REG & PWM_DAC_16BIT_MASK);
    #endif /* (PWM_DAC_CY_TCPWM_4000) */
}


/*******************************************************************************
* Function Name: PWM_DAC_ReadCapture
********************************************************************************
*
* Summary:
*  Reads the captured counter value. This API is applicable only for
*  Timer/Counter with the capture mode and Quadrature Decoder modes.
*
* Parameters:
*  None
*
* Return:
*  Capture value
*
*******************************************************************************/
uint32 PWM_DAC_ReadCapture(void)
{
    return (PWM_DAC_COMP_CAP_REG & PWM_DAC_16BIT_MASK);
}


/*******************************************************************************
* Function Name: PWM_DAC_ReadCaptureBuf
********************************************************************************
*
* Summary:
*  Reads the capture buffer register. This API is applicable only for
*  Timer/Counter with the capture mode and Quadrature Decoder modes.
*
* Parameters:
*  None
*
* Return:
*  Capture buffer value
*
*******************************************************************************/
uint32 PWM_DAC_ReadCaptureBuf(void)
{
    return (PWM_DAC_COMP_CAP_BUF_REG & PWM_DAC_16BIT_MASK);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetCaptureMode
********************************************************************************
*
* Summary:
*  Sets the capture trigger mode. For PWM mode this is the switch input.
*  This input is not applicable to the Timer/Counter without Capture and
*  Quadrature Decoder modes.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - PWM_DAC_TRIG_LEVEL     - Level
*     - PWM_DAC_TRIG_RISING    - Rising edge
*     - PWM_DAC_TRIG_FALLING   - Falling edge
*     - PWM_DAC_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetCaptureMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_TRIG_CONTROL1_REG &= (uint32)~PWM_DAC_CAPTURE_MASK;
    PWM_DAC_TRIG_CONTROL1_REG |= triggerMode;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetReloadMode
********************************************************************************
*
* Summary:
*  Sets the reload trigger mode. For Quadrature Decoder mode this is the index
*  input.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - PWM_DAC_TRIG_LEVEL     - Level
*     - PWM_DAC_TRIG_RISING    - Rising edge
*     - PWM_DAC_TRIG_FALLING   - Falling edge
*     - PWM_DAC_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetReloadMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_TRIG_CONTROL1_REG &= (uint32)~PWM_DAC_RELOAD_MASK;
    PWM_DAC_TRIG_CONTROL1_REG |= ((uint32)(triggerMode << PWM_DAC_RELOAD_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetStartMode
********************************************************************************
*
* Summary:
*  Sets the start trigger mode. For Quadrature Decoder mode this is the
*  phiB input.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - PWM_DAC_TRIG_LEVEL     - Level
*     - PWM_DAC_TRIG_RISING    - Rising edge
*     - PWM_DAC_TRIG_FALLING   - Falling edge
*     - PWM_DAC_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetStartMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_TRIG_CONTROL1_REG &= (uint32)~PWM_DAC_START_MASK;
    PWM_DAC_TRIG_CONTROL1_REG |= ((uint32)(triggerMode << PWM_DAC_START_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetStopMode
********************************************************************************
*
* Summary:
*  Sets the stop trigger mode. For PWM mode this is the kill input.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - PWM_DAC_TRIG_LEVEL     - Level
*     - PWM_DAC_TRIG_RISING    - Rising edge
*     - PWM_DAC_TRIG_FALLING   - Falling edge
*     - PWM_DAC_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetStopMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_TRIG_CONTROL1_REG &= (uint32)~PWM_DAC_STOP_MASK;
    PWM_DAC_TRIG_CONTROL1_REG |= ((uint32)(triggerMode << PWM_DAC_STOP_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_SetCountMode
********************************************************************************
*
* Summary:
*  Sets the count trigger mode. For Quadrature Decoder mode this is the phiA
*  input.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - PWM_DAC_TRIG_LEVEL     - Level
*     - PWM_DAC_TRIG_RISING    - Rising edge
*     - PWM_DAC_TRIG_FALLING   - Falling edge
*     - PWM_DAC_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetCountMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_TRIG_CONTROL1_REG &= (uint32)~PWM_DAC_COUNT_MASK;
    PWM_DAC_TRIG_CONTROL1_REG |= ((uint32)(triggerMode << PWM_DAC_COUNT_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_TriggerCommand
********************************************************************************
*
* Summary:
*  Triggers the designated command to occur on the designated TCPWM instances.
*  The mask can be used to apply this command simultaneously to more than one
*  instance.  This allows multiple TCPWM instances to be synchronized.
*
* Parameters:
*  mask: A combination of mask bits for each instance of the TCPWM that the
*        command should apply to.  This function from one instance can be used
*        to apply the command to any of the instances in the design.
*        The mask value for a specific instance is available with the MASK
*        define.
*  command: Enumerated command values. Capture command only applicable for
*           Timer/Counter with Capture and PWM modes.
*   Values:
*     - PWM_DAC_CMD_CAPTURE    - Trigger Capture/Switch command
*     - PWM_DAC_CMD_RELOAD     - Trigger Reload/Index command
*     - PWM_DAC_CMD_STOP       - Trigger Stop/Kill command
*     - PWM_DAC_CMD_START      - Trigger Start/phiB command
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_TriggerCommand(uint32 mask, uint32 command)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    PWM_DAC_COMMAND_REG = ((uint32)(mask << command));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: PWM_DAC_ReadStatus
********************************************************************************
*
* Summary:
*  Reads the status of the PWM_DAC.
*
* Parameters:
*  None
*
* Return:
*  Status
*   Values:
*     - PWM_DAC_STATUS_DOWN    - Set if counting down
*     - PWM_DAC_STATUS_RUNNING - Set if counter is running
*
*******************************************************************************/
uint32 PWM_DAC_ReadStatus(void)
{
    return ((PWM_DAC_STATUS_REG >> PWM_DAC_RUNNING_STATUS_SHIFT) |
            (PWM_DAC_STATUS_REG & PWM_DAC_STATUS_DOWN));
}


/*******************************************************************************
* Function Name: PWM_DAC_SetInterruptMode
********************************************************************************
*
* Summary:
*  Sets the interrupt mask to control which interrupt
*  requests generate the interrupt signal.
*
* Parameters:
*   interruptMask: Mask of bits to be enabled
*   Values:
*     - PWM_DAC_INTR_MASK_TC       - Terminal count mask
*     - PWM_DAC_INTR_MASK_CC_MATCH - Compare count / capture mask
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetInterruptMode(uint32 interruptMask)
{
    PWM_DAC_INTERRUPT_MASK_REG =  interruptMask;
}


/*******************************************************************************
* Function Name: PWM_DAC_GetInterruptSourceMasked
********************************************************************************
*
* Summary:
*  Gets the interrupt requests masked by the interrupt mask.
*
* Parameters:
*   None
*
* Return:
*  Masked interrupt source
*   Values:
*     - PWM_DAC_INTR_MASK_TC       - Terminal count mask
*     - PWM_DAC_INTR_MASK_CC_MATCH - Compare count / capture mask
*
*******************************************************************************/
uint32 PWM_DAC_GetInterruptSourceMasked(void)
{
    return (PWM_DAC_INTERRUPT_MASKED_REG);
}


/*******************************************************************************
* Function Name: PWM_DAC_GetInterruptSource
********************************************************************************
*
* Summary:
*  Gets the interrupt requests (without masking).
*
* Parameters:
*  None
*
* Return:
*  Interrupt request value
*   Values:
*     - PWM_DAC_INTR_MASK_TC       - Terminal count mask
*     - PWM_DAC_INTR_MASK_CC_MATCH - Compare count / capture mask
*
*******************************************************************************/
uint32 PWM_DAC_GetInterruptSource(void)
{
    return (PWM_DAC_INTERRUPT_REQ_REG);
}


/*******************************************************************************
* Function Name: PWM_DAC_ClearInterrupt
********************************************************************************
*
* Summary:
*  Clears the interrupt request.
*
* Parameters:
*   interruptMask: Mask of interrupts to clear
*   Values:
*     - PWM_DAC_INTR_MASK_TC       - Terminal count mask
*     - PWM_DAC_INTR_MASK_CC_MATCH - Compare count / capture mask
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_ClearInterrupt(uint32 interruptMask)
{
    PWM_DAC_INTERRUPT_REQ_REG = interruptMask;
}


/*******************************************************************************
* Function Name: PWM_DAC_SetInterrupt
********************************************************************************
*
* Summary:
*  Sets a software interrupt request.
*
* Parameters:
*   interruptMask: Mask of interrupts to set
*   Values:
*     - PWM_DAC_INTR_MASK_TC       - Terminal count mask
*     - PWM_DAC_INTR_MASK_CC_MATCH - Compare count / capture mask
*
* Return:
*  None
*
*******************************************************************************/
void PWM_DAC_SetInterrupt(uint32 interruptMask)
{
    PWM_DAC_INTERRUPT_SET_REG = interruptMask;
}


/* [] END OF FILE */
