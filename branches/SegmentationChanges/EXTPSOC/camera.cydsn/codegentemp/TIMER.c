/*******************************************************************************
* File Name: TIMER.c
* Version 2.10
*
* Description:
*  This file provides the source code to the API for the TIMER
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

#include "TIMER.h"

uint8 TIMER_initVar = 0u;


/*******************************************************************************
* Function Name: TIMER_Init
********************************************************************************
*
* Summary:
*  Initialize/Restore default TIMER configuration.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_Init(void)
{

    /* Set values from customizer to CTRL */
    #if (TIMER__QUAD == TIMER_CONFIG)
        TIMER_CONTROL_REG = TIMER_CTRL_QUAD_BASE_CONFIG;
        
        /* Set values from customizer to CTRL1 */
        TIMER_TRIG_CONTROL1_REG  = TIMER_QUAD_SIGNALS_MODES;

        /* Set values from customizer to INTR */
        TIMER_SetInterruptMode(TIMER_QUAD_INTERRUPT_MASK);
        
         /* Set other values */
        TIMER_SetCounterMode(TIMER_COUNT_DOWN);
        TIMER_WritePeriod(TIMER_QUAD_PERIOD_INIT_VALUE);
        TIMER_WriteCounter(TIMER_QUAD_PERIOD_INIT_VALUE);
    #endif  /* (TIMER__QUAD == TIMER_CONFIG) */

    #if (TIMER__TIMER == TIMER_CONFIG)
        TIMER_CONTROL_REG = TIMER_CTRL_TIMER_BASE_CONFIG;
        
        /* Set values from customizer to CTRL1 */
        TIMER_TRIG_CONTROL1_REG  = TIMER_TIMER_SIGNALS_MODES;
    
        /* Set values from customizer to INTR */
        TIMER_SetInterruptMode(TIMER_TC_INTERRUPT_MASK);
        
        /* Set other values from customizer */
        TIMER_WritePeriod(TIMER_TC_PERIOD_VALUE );

        #if (TIMER__COMPARE == TIMER_TC_COMP_CAP_MODE)
            TIMER_WriteCompare(TIMER_TC_COMPARE_VALUE);

            #if (1u == TIMER_TC_COMPARE_SWAP)
                TIMER_SetCompareSwap(1u);
                TIMER_WriteCompareBuf(TIMER_TC_COMPARE_BUF_VALUE);
            #endif  /* (1u == TIMER_TC_COMPARE_SWAP) */
        #endif  /* (TIMER__COMPARE == TIMER_TC_COMP_CAP_MODE) */

        /* Initialize counter value */
        #if (TIMER_CY_TCPWM_V2 && TIMER_TIMER_UPDOWN_CNT_USED && !TIMER_CY_TCPWM_4000)
            TIMER_WriteCounter(1u);
        #elif(TIMER__COUNT_DOWN == TIMER_TC_COUNTER_MODE)
            TIMER_WriteCounter(TIMER_TC_PERIOD_VALUE);
        #else
            TIMER_WriteCounter(0u);
        #endif /* (TIMER_CY_TCPWM_V2 && TIMER_TIMER_UPDOWN_CNT_USED && !TIMER_CY_TCPWM_4000) */
    #endif  /* (TIMER__TIMER == TIMER_CONFIG) */

    #if (TIMER__PWM_SEL == TIMER_CONFIG)
        TIMER_CONTROL_REG = TIMER_CTRL_PWM_BASE_CONFIG;

        #if (TIMER__PWM_PR == TIMER_PWM_MODE)
            TIMER_CONTROL_REG |= TIMER_CTRL_PWM_RUN_MODE;
            TIMER_WriteCounter(TIMER_PWM_PR_INIT_VALUE);
        #else
            TIMER_CONTROL_REG |= TIMER_CTRL_PWM_ALIGN | TIMER_CTRL_PWM_KILL_EVENT;
            
            /* Initialize counter value */
            #if (TIMER_CY_TCPWM_V2 && TIMER_PWM_UPDOWN_CNT_USED && !TIMER_CY_TCPWM_4000)
                TIMER_WriteCounter(1u);
            #elif (TIMER__RIGHT == TIMER_PWM_ALIGN)
                TIMER_WriteCounter(TIMER_PWM_PERIOD_VALUE);
            #else 
                TIMER_WriteCounter(0u);
            #endif  /* (TIMER_CY_TCPWM_V2 && TIMER_PWM_UPDOWN_CNT_USED && !TIMER_CY_TCPWM_4000) */
        #endif  /* (TIMER__PWM_PR == TIMER_PWM_MODE) */

        #if (TIMER__PWM_DT == TIMER_PWM_MODE)
            TIMER_CONTROL_REG |= TIMER_CTRL_PWM_DEAD_TIME_CYCLE;
        #endif  /* (TIMER__PWM_DT == TIMER_PWM_MODE) */

        #if (TIMER__PWM == TIMER_PWM_MODE)
            TIMER_CONTROL_REG |= TIMER_CTRL_PWM_PRESCALER;
        #endif  /* (TIMER__PWM == TIMER_PWM_MODE) */

        /* Set values from customizer to CTRL1 */
        TIMER_TRIG_CONTROL1_REG  = TIMER_PWM_SIGNALS_MODES;
    
        /* Set values from customizer to INTR */
        TIMER_SetInterruptMode(TIMER_PWM_INTERRUPT_MASK);

        /* Set values from customizer to CTRL2 */
        #if (TIMER__PWM_PR == TIMER_PWM_MODE)
            TIMER_TRIG_CONTROL2_REG =
                    (TIMER_CC_MATCH_NO_CHANGE    |
                    TIMER_OVERLOW_NO_CHANGE      |
                    TIMER_UNDERFLOW_NO_CHANGE);
        #else
            #if (TIMER__LEFT == TIMER_PWM_ALIGN)
                TIMER_TRIG_CONTROL2_REG = TIMER_PWM_MODE_LEFT;
            #endif  /* ( TIMER_PWM_LEFT == TIMER_PWM_ALIGN) */

            #if (TIMER__RIGHT == TIMER_PWM_ALIGN)
                TIMER_TRIG_CONTROL2_REG = TIMER_PWM_MODE_RIGHT;
            #endif  /* ( TIMER_PWM_RIGHT == TIMER_PWM_ALIGN) */

            #if (TIMER__CENTER == TIMER_PWM_ALIGN)
                TIMER_TRIG_CONTROL2_REG = TIMER_PWM_MODE_CENTER;
            #endif  /* ( TIMER_PWM_CENTER == TIMER_PWM_ALIGN) */

            #if (TIMER__ASYMMETRIC == TIMER_PWM_ALIGN)
                TIMER_TRIG_CONTROL2_REG = TIMER_PWM_MODE_ASYM;
            #endif  /* (TIMER__ASYMMETRIC == TIMER_PWM_ALIGN) */
        #endif  /* (TIMER__PWM_PR == TIMER_PWM_MODE) */

        /* Set other values from customizer */
        TIMER_WritePeriod(TIMER_PWM_PERIOD_VALUE );
        TIMER_WriteCompare(TIMER_PWM_COMPARE_VALUE);

        #if (1u == TIMER_PWM_COMPARE_SWAP)
            TIMER_SetCompareSwap(1u);
            TIMER_WriteCompareBuf(TIMER_PWM_COMPARE_BUF_VALUE);
        #endif  /* (1u == TIMER_PWM_COMPARE_SWAP) */

        #if (1u == TIMER_PWM_PERIOD_SWAP)
            TIMER_SetPeriodSwap(1u);
            TIMER_WritePeriodBuf(TIMER_PWM_PERIOD_BUF_VALUE);
        #endif  /* (1u == TIMER_PWM_PERIOD_SWAP) */
    #endif  /* (TIMER__PWM_SEL == TIMER_CONFIG) */
    
}


/*******************************************************************************
* Function Name: TIMER_Enable
********************************************************************************
*
* Summary:
*  Enables the TIMER.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_Enable(void)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();
    TIMER_BLOCK_CONTROL_REG |= TIMER_MASK;
    CyExitCriticalSection(enableInterrupts);

    /* Start Timer or PWM if start input is absent */
    #if (TIMER__PWM_SEL == TIMER_CONFIG)
        #if (0u == TIMER_PWM_START_SIGNAL_PRESENT)
            TIMER_TriggerCommand(TIMER_MASK, TIMER_CMD_START);
        #endif /* (0u == TIMER_PWM_START_SIGNAL_PRESENT) */
    #endif /* (TIMER__PWM_SEL == TIMER_CONFIG) */

    #if (TIMER__TIMER == TIMER_CONFIG)
        #if (0u == TIMER_TC_START_SIGNAL_PRESENT)
            TIMER_TriggerCommand(TIMER_MASK, TIMER_CMD_START);
        #endif /* (0u == TIMER_TC_START_SIGNAL_PRESENT) */
    #endif /* (TIMER__TIMER == TIMER_CONFIG) */
    
    #if (TIMER__QUAD == TIMER_CONFIG)
        #if (0u != TIMER_QUAD_AUTO_START)
            TIMER_TriggerCommand(TIMER_MASK, TIMER_CMD_RELOAD);
        #endif /* (0u != TIMER_QUAD_AUTO_START) */
    #endif  /* (TIMER__QUAD == TIMER_CONFIG) */
}


/*******************************************************************************
* Function Name: TIMER_Start
********************************************************************************
*
* Summary:
*  Initializes the TIMER with default customizer
*  values when called the first time and enables the TIMER.
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
*  TIMER_initVar: global variable is used to indicate initial
*  configuration of this component.  The variable is initialized to zero and set
*  to 1 the first time TIMER_Start() is called. This allows
*  enabling/disabling a component without re-initialization in all subsequent
*  calls to the TIMER_Start() routine.
*
*******************************************************************************/
void TIMER_Start(void)
{
    if (0u == TIMER_initVar)
    {
        TIMER_Init();
        TIMER_initVar = 1u;
    }

    TIMER_Enable();
}


/*******************************************************************************
* Function Name: TIMER_Stop
********************************************************************************
*
* Summary:
*  Disables the TIMER.
*
* Parameters:
*  None
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_Stop(void)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_BLOCK_CONTROL_REG &= (uint32)~TIMER_MASK;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_SetMode
********************************************************************************
*
* Summary:
*  Sets the operation mode of the TIMER. This function is used when
*  configured as a generic TIMER and the actual mode of operation is
*  set at runtime. The mode must be set while the component is disabled.
*
* Parameters:
*  mode: Mode for the TIMER to operate in
*   Values:
*   - TIMER_MODE_TIMER_COMPARE - Timer / Counter with
*                                                 compare capability
*         - TIMER_MODE_TIMER_CAPTURE - Timer / Counter with
*                                                 capture capability
*         - TIMER_MODE_QUAD - Quadrature decoder
*         - TIMER_MODE_PWM - PWM
*         - TIMER_MODE_PWM_DT - PWM with dead time
*         - TIMER_MODE_PWM_PR - PWM with pseudo random capability
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_SetMode(uint32 mode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_CONTROL_REG &= (uint32)~TIMER_MODE_MASK;
    TIMER_CONTROL_REG |= mode;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_SetQDMode
********************************************************************************
*
* Summary:
*  Sets the the Quadrature Decoder to one of the 3 supported modes.
*  Its functionality is only applicable to Quadrature Decoder operation.
*
* Parameters:
*  qdMode: Quadrature Decoder mode
*   Values:
*         - TIMER_MODE_X1 - Counts on phi 1 rising
*         - TIMER_MODE_X2 - Counts on both edges of phi1 (2x faster)
*         - TIMER_MODE_X4 - Counts on both edges of phi1 and phi2
*                                        (4x faster)
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_SetQDMode(uint32 qdMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_CONTROL_REG &= (uint32)~TIMER_QUAD_MODE_MASK;
    TIMER_CONTROL_REG |= qdMode;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_SetPrescaler
********************************************************************************
*
* Summary:
*  Sets the prescaler value that is applied to the clock input.  Not applicable
*  to a PWM with the dead time mode or Quadrature Decoder mode.
*
* Parameters:
*  prescaler: Prescaler divider value
*   Values:
*         - TIMER_PRESCALE_DIVBY1    - Divide by 1 (no prescaling)
*         - TIMER_PRESCALE_DIVBY2    - Divide by 2
*         - TIMER_PRESCALE_DIVBY4    - Divide by 4
*         - TIMER_PRESCALE_DIVBY8    - Divide by 8
*         - TIMER_PRESCALE_DIVBY16   - Divide by 16
*         - TIMER_PRESCALE_DIVBY32   - Divide by 32
*         - TIMER_PRESCALE_DIVBY64   - Divide by 64
*         - TIMER_PRESCALE_DIVBY128  - Divide by 128
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_SetPrescaler(uint32 prescaler)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_CONTROL_REG &= (uint32)~TIMER_PRESCALER_MASK;
    TIMER_CONTROL_REG |= prescaler;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_SetOneShot
********************************************************************************
*
* Summary:
*  Writes the register that controls whether the TIMER runs
*  continuously or stops when terminal count is reached.  By default the
*  TIMER operates in the continuous mode.
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
void TIMER_SetOneShot(uint32 oneShotEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_CONTROL_REG &= (uint32)~TIMER_ONESHOT_MASK;
    TIMER_CONTROL_REG |= ((uint32)((oneShotEnable & TIMER_1BIT_MASK) <<
                                                               TIMER_ONESHOT_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_SetPWMMode
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
void TIMER_SetPWMMode(uint32 modeMask)
{
    TIMER_TRIG_CONTROL2_REG = (modeMask & TIMER_6BIT_MASK);
}



/*******************************************************************************
* Function Name: TIMER_SetPWMSyncKill
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
void TIMER_SetPWMSyncKill(uint32 syncKillEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_CONTROL_REG &= (uint32)~TIMER_PWM_SYNC_KILL_MASK;
    TIMER_CONTROL_REG |= ((uint32)((syncKillEnable & TIMER_1BIT_MASK)  <<
                                               TIMER_PWM_SYNC_KILL_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_SetPWMStopOnKill
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
void TIMER_SetPWMStopOnKill(uint32 stopOnKillEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_CONTROL_REG &= (uint32)~TIMER_PWM_STOP_KILL_MASK;
    TIMER_CONTROL_REG |= ((uint32)((stopOnKillEnable & TIMER_1BIT_MASK)  <<
                                                         TIMER_PWM_STOP_KILL_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_SetPWMDeadTime
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
void TIMER_SetPWMDeadTime(uint32 deadTime)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_CONTROL_REG &= (uint32)~TIMER_PRESCALER_MASK;
    TIMER_CONTROL_REG |= ((uint32)((deadTime & TIMER_8BIT_MASK) <<
                                                          TIMER_PRESCALER_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_SetPWMInvert
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
*         - TIMER_INVERT_LINE   - Inverts the line output
*         - TIMER_INVERT_LINE_N - Inverts the line_n output
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_SetPWMInvert(uint32 mask)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_CONTROL_REG &= (uint32)~TIMER_INV_OUT_MASK;
    TIMER_CONTROL_REG |= mask;

    CyExitCriticalSection(enableInterrupts);
}



/*******************************************************************************
* Function Name: TIMER_WriteCounter
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
void TIMER_WriteCounter(uint32 count)
{
    TIMER_COUNTER_REG = (count & TIMER_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TIMER_ReadCounter
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
uint32 TIMER_ReadCounter(void)
{
    return (TIMER_COUNTER_REG & TIMER_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TIMER_SetCounterMode
********************************************************************************
*
* Summary:
*  Sets the counter mode.  Applicable to all modes except Quadrature Decoder
*  and the PWM with a pseudo random output.
*
* Parameters:
*  counterMode: Enumerated counter type values
*   Values:
*     - TIMER_COUNT_UP       - Counts up
*     - TIMER_COUNT_DOWN     - Counts down
*     - TIMER_COUNT_UPDOWN0  - Counts up and down. Terminal count
*                                         generated when counter reaches 0
*     - TIMER_COUNT_UPDOWN1  - Counts up and down. Terminal count
*                                         generated both when counter reaches 0
*                                         and period
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_SetCounterMode(uint32 counterMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_CONTROL_REG &= (uint32)~TIMER_UPDOWN_MASK;
    TIMER_CONTROL_REG |= counterMode;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_WritePeriod
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
void TIMER_WritePeriod(uint32 period)
{
    TIMER_PERIOD_REG = (period & TIMER_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TIMER_ReadPeriod
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
uint32 TIMER_ReadPeriod(void)
{
    return (TIMER_PERIOD_REG & TIMER_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TIMER_SetCompareSwap
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
void TIMER_SetCompareSwap(uint32 swapEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_CONTROL_REG &= (uint32)~TIMER_RELOAD_CC_MASK;
    TIMER_CONTROL_REG |= (swapEnable & TIMER_1BIT_MASK);

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_WritePeriodBuf
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
void TIMER_WritePeriodBuf(uint32 periodBuf)
{
    TIMER_PERIOD_BUF_REG = (periodBuf & TIMER_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TIMER_ReadPeriodBuf
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
uint32 TIMER_ReadPeriodBuf(void)
{
    return (TIMER_PERIOD_BUF_REG & TIMER_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TIMER_SetPeriodSwap
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
void TIMER_SetPeriodSwap(uint32 swapEnable)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_CONTROL_REG &= (uint32)~TIMER_RELOAD_PERIOD_MASK;
    TIMER_CONTROL_REG |= ((uint32)((swapEnable & TIMER_1BIT_MASK) <<
                                                            TIMER_RELOAD_PERIOD_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_WriteCompare
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
void TIMER_WriteCompare(uint32 compare)
{
    #if (TIMER_CY_TCPWM_4000)
        uint32 currentMode;
    #endif /* (TIMER_CY_TCPWM_4000) */

    #if (TIMER_CY_TCPWM_4000)
        currentMode = ((TIMER_CONTROL_REG & TIMER_UPDOWN_MASK) >> TIMER_UPDOWN_SHIFT);

        if (((uint32)TIMER__COUNT_DOWN == currentMode) && (0xFFFFu != compare))
        {
            compare++;
        }
        else if (((uint32)TIMER__COUNT_UP == currentMode) && (0u != compare))
        {
            compare--;
        }
        else
        {
        }
        
    
    #endif /* (TIMER_CY_TCPWM_4000) */
    
    TIMER_COMP_CAP_REG = (compare & TIMER_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TIMER_ReadCompare
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
uint32 TIMER_ReadCompare(void)
{
    #if (TIMER_CY_TCPWM_4000)
        uint32 currentMode;
        uint32 regVal;
    #endif /* (TIMER_CY_TCPWM_4000) */

    #if (TIMER_CY_TCPWM_4000)
        currentMode = ((TIMER_CONTROL_REG & TIMER_UPDOWN_MASK) >> TIMER_UPDOWN_SHIFT);
        
        regVal = TIMER_COMP_CAP_REG;
        
        if (((uint32)TIMER__COUNT_DOWN == currentMode) && (0u != regVal))
        {
            regVal--;
        }
        else if (((uint32)TIMER__COUNT_UP == currentMode) && (0xFFFFu != regVal))
        {
            regVal++;
        }
        else
        {
        }

        return (regVal & TIMER_16BIT_MASK);
    #else
        return (TIMER_COMP_CAP_REG & TIMER_16BIT_MASK);
    #endif /* (TIMER_CY_TCPWM_4000) */
}


/*******************************************************************************
* Function Name: TIMER_WriteCompareBuf
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
void TIMER_WriteCompareBuf(uint32 compareBuf)
{
    #if (TIMER_CY_TCPWM_4000)
        uint32 currentMode;
    #endif /* (TIMER_CY_TCPWM_4000) */

    #if (TIMER_CY_TCPWM_4000)
        currentMode = ((TIMER_CONTROL_REG & TIMER_UPDOWN_MASK) >> TIMER_UPDOWN_SHIFT);

        if (((uint32)TIMER__COUNT_DOWN == currentMode) && (0xFFFFu != compareBuf))
        {
            compareBuf++;
        }
        else if (((uint32)TIMER__COUNT_UP == currentMode) && (0u != compareBuf))
        {
            compareBuf --;
        }
        else
        {
        }
    #endif /* (TIMER_CY_TCPWM_4000) */
    
    TIMER_COMP_CAP_BUF_REG = (compareBuf & TIMER_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TIMER_ReadCompareBuf
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
uint32 TIMER_ReadCompareBuf(void)
{
    #if (TIMER_CY_TCPWM_4000)
        uint32 currentMode;
        uint32 regVal;
    #endif /* (TIMER_CY_TCPWM_4000) */

    #if (TIMER_CY_TCPWM_4000)
        currentMode = ((TIMER_CONTROL_REG & TIMER_UPDOWN_MASK) >> TIMER_UPDOWN_SHIFT);

        regVal = TIMER_COMP_CAP_BUF_REG;
        
        if (((uint32)TIMER__COUNT_DOWN == currentMode) && (0u != regVal))
        {
            regVal--;
        }
        else if (((uint32)TIMER__COUNT_UP == currentMode) && (0xFFFFu != regVal))
        {
            regVal++;
        }
        else
        {
        }

        return (regVal & TIMER_16BIT_MASK);
    #else
        return (TIMER_COMP_CAP_BUF_REG & TIMER_16BIT_MASK);
    #endif /* (TIMER_CY_TCPWM_4000) */
}


/*******************************************************************************
* Function Name: TIMER_ReadCapture
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
uint32 TIMER_ReadCapture(void)
{
    return (TIMER_COMP_CAP_REG & TIMER_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TIMER_ReadCaptureBuf
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
uint32 TIMER_ReadCaptureBuf(void)
{
    return (TIMER_COMP_CAP_BUF_REG & TIMER_16BIT_MASK);
}


/*******************************************************************************
* Function Name: TIMER_SetCaptureMode
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
*     - TIMER_TRIG_LEVEL     - Level
*     - TIMER_TRIG_RISING    - Rising edge
*     - TIMER_TRIG_FALLING   - Falling edge
*     - TIMER_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_SetCaptureMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_TRIG_CONTROL1_REG &= (uint32)~TIMER_CAPTURE_MASK;
    TIMER_TRIG_CONTROL1_REG |= triggerMode;

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_SetReloadMode
********************************************************************************
*
* Summary:
*  Sets the reload trigger mode. For Quadrature Decoder mode this is the index
*  input.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - TIMER_TRIG_LEVEL     - Level
*     - TIMER_TRIG_RISING    - Rising edge
*     - TIMER_TRIG_FALLING   - Falling edge
*     - TIMER_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_SetReloadMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_TRIG_CONTROL1_REG &= (uint32)~TIMER_RELOAD_MASK;
    TIMER_TRIG_CONTROL1_REG |= ((uint32)(triggerMode << TIMER_RELOAD_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_SetStartMode
********************************************************************************
*
* Summary:
*  Sets the start trigger mode. For Quadrature Decoder mode this is the
*  phiB input.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - TIMER_TRIG_LEVEL     - Level
*     - TIMER_TRIG_RISING    - Rising edge
*     - TIMER_TRIG_FALLING   - Falling edge
*     - TIMER_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_SetStartMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_TRIG_CONTROL1_REG &= (uint32)~TIMER_START_MASK;
    TIMER_TRIG_CONTROL1_REG |= ((uint32)(triggerMode << TIMER_START_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_SetStopMode
********************************************************************************
*
* Summary:
*  Sets the stop trigger mode. For PWM mode this is the kill input.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - TIMER_TRIG_LEVEL     - Level
*     - TIMER_TRIG_RISING    - Rising edge
*     - TIMER_TRIG_FALLING   - Falling edge
*     - TIMER_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_SetStopMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_TRIG_CONTROL1_REG &= (uint32)~TIMER_STOP_MASK;
    TIMER_TRIG_CONTROL1_REG |= ((uint32)(triggerMode << TIMER_STOP_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_SetCountMode
********************************************************************************
*
* Summary:
*  Sets the count trigger mode. For Quadrature Decoder mode this is the phiA
*  input.
*
* Parameters:
*  triggerMode: Enumerated trigger mode value
*   Values:
*     - TIMER_TRIG_LEVEL     - Level
*     - TIMER_TRIG_RISING    - Rising edge
*     - TIMER_TRIG_FALLING   - Falling edge
*     - TIMER_TRIG_BOTH      - Both rising and falling edge
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_SetCountMode(uint32 triggerMode)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_TRIG_CONTROL1_REG &= (uint32)~TIMER_COUNT_MASK;
    TIMER_TRIG_CONTROL1_REG |= ((uint32)(triggerMode << TIMER_COUNT_SHIFT));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_TriggerCommand
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
*     - TIMER_CMD_CAPTURE    - Trigger Capture/Switch command
*     - TIMER_CMD_RELOAD     - Trigger Reload/Index command
*     - TIMER_CMD_STOP       - Trigger Stop/Kill command
*     - TIMER_CMD_START      - Trigger Start/phiB command
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_TriggerCommand(uint32 mask, uint32 command)
{
    uint8 enableInterrupts;

    enableInterrupts = CyEnterCriticalSection();

    TIMER_COMMAND_REG = ((uint32)(mask << command));

    CyExitCriticalSection(enableInterrupts);
}


/*******************************************************************************
* Function Name: TIMER_ReadStatus
********************************************************************************
*
* Summary:
*  Reads the status of the TIMER.
*
* Parameters:
*  None
*
* Return:
*  Status
*   Values:
*     - TIMER_STATUS_DOWN    - Set if counting down
*     - TIMER_STATUS_RUNNING - Set if counter is running
*
*******************************************************************************/
uint32 TIMER_ReadStatus(void)
{
    return ((TIMER_STATUS_REG >> TIMER_RUNNING_STATUS_SHIFT) |
            (TIMER_STATUS_REG & TIMER_STATUS_DOWN));
}


/*******************************************************************************
* Function Name: TIMER_SetInterruptMode
********************************************************************************
*
* Summary:
*  Sets the interrupt mask to control which interrupt
*  requests generate the interrupt signal.
*
* Parameters:
*   interruptMask: Mask of bits to be enabled
*   Values:
*     - TIMER_INTR_MASK_TC       - Terminal count mask
*     - TIMER_INTR_MASK_CC_MATCH - Compare count / capture mask
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_SetInterruptMode(uint32 interruptMask)
{
    TIMER_INTERRUPT_MASK_REG =  interruptMask;
}


/*******************************************************************************
* Function Name: TIMER_GetInterruptSourceMasked
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
*     - TIMER_INTR_MASK_TC       - Terminal count mask
*     - TIMER_INTR_MASK_CC_MATCH - Compare count / capture mask
*
*******************************************************************************/
uint32 TIMER_GetInterruptSourceMasked(void)
{
    return (TIMER_INTERRUPT_MASKED_REG);
}


/*******************************************************************************
* Function Name: TIMER_GetInterruptSource
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
*     - TIMER_INTR_MASK_TC       - Terminal count mask
*     - TIMER_INTR_MASK_CC_MATCH - Compare count / capture mask
*
*******************************************************************************/
uint32 TIMER_GetInterruptSource(void)
{
    return (TIMER_INTERRUPT_REQ_REG);
}


/*******************************************************************************
* Function Name: TIMER_ClearInterrupt
********************************************************************************
*
* Summary:
*  Clears the interrupt request.
*
* Parameters:
*   interruptMask: Mask of interrupts to clear
*   Values:
*     - TIMER_INTR_MASK_TC       - Terminal count mask
*     - TIMER_INTR_MASK_CC_MATCH - Compare count / capture mask
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_ClearInterrupt(uint32 interruptMask)
{
    TIMER_INTERRUPT_REQ_REG = interruptMask;
}


/*******************************************************************************
* Function Name: TIMER_SetInterrupt
********************************************************************************
*
* Summary:
*  Sets a software interrupt request.
*
* Parameters:
*   interruptMask: Mask of interrupts to set
*   Values:
*     - TIMER_INTR_MASK_TC       - Terminal count mask
*     - TIMER_INTR_MASK_CC_MATCH - Compare count / capture mask
*
* Return:
*  None
*
*******************************************************************************/
void TIMER_SetInterrupt(uint32 interruptMask)
{
    TIMER_INTERRUPT_SET_REG = interruptMask;
}


/* [] END OF FILE */
