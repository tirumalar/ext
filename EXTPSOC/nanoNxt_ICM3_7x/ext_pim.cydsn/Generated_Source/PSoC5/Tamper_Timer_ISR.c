/*******************************************************************************
* File Name: Tamper_Timer_ISR.c  
* Version 1.70
*
*  Description:
*   API for controlling the state of an interrupt.
*
*
*  Note:
*
********************************************************************************
* Copyright 2008-2015, Cypress Semiconductor Corporation.  All rights reserved.
* You may use this file only in accordance with the license, terms, conditions, 
* disclaimers, and limitations in the end user license agreement accompanying 
* the software package with which this file was provided.
*******************************************************************************/


#include <cydevice_trm.h>
#include <CyLib.h>
#include <Tamper_Timer_ISR.h>


#if !defined(Tamper_Timer_ISR__REMOVED) /* Check for removal by optimization */

/*******************************************************************************
*  Place your includes, defines and code here 
********************************************************************************/
/* `#START Tamper_Timer_ISR_intc` */
#include "Tamper_Timer.h"
#include "inout.h"
#include "system.h"
#include "PS_D1.h"
/* `#END` */

#ifndef CYINT_IRQ_BASE
#define CYINT_IRQ_BASE      16
#endif /* CYINT_IRQ_BASE */
#ifndef CYINT_VECT_TABLE
#define CYINT_VECT_TABLE    ((cyisraddress **) CYREG_NVIC_VECT_OFFSET)
#endif /* CYINT_VECT_TABLE */

/* Declared in startup, used to set unused interrupts to. */
CY_ISR_PROTO(IntDefaultHandler);


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_Start
********************************************************************************
*
* Summary:
*  Set up the interrupt and enable it. This function disables the interrupt, 
*  sets the default interrupt vector, sets the priority from the value in the
*  Design Wide Resources Interrupt Editor, then enables the interrupt to the 
*  interrupt controller.
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
void Tamper_Timer_ISR_Start(void)
{
    /* For all we know the interrupt is active. */
    Tamper_Timer_ISR_Disable();

    /* Set the ISR to point to the Tamper_Timer_ISR Interrupt. */
    Tamper_Timer_ISR_SetVector(&Tamper_Timer_ISR_Interrupt);

    /* Set the priority. */
    Tamper_Timer_ISR_SetPriority((uint8)Tamper_Timer_ISR_INTC_PRIOR_NUMBER);

    /* Enable it. */
    Tamper_Timer_ISR_Enable();
}


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_StartEx
********************************************************************************
*
* Summary:
*  Sets up the interrupt and enables it. This function disables the interrupt,
*  sets the interrupt vector based on the address passed in, sets the priority 
*  from the value in the Design Wide Resources Interrupt Editor, then enables 
*  the interrupt to the interrupt controller.
*  
*  When defining ISR functions, the CY_ISR and CY_ISR_PROTO macros should be 
*  used to provide consistent definition across compilers:
*  
*  Function definition example:
*   CY_ISR(MyISR)
*   {
*   }
*   Function prototype example:
*   CY_ISR_PROTO(MyISR);
*
* Parameters:  
*   address: Address of the ISR to set in the interrupt vector table.
*
* Return:
*   None
*
*******************************************************************************/
void Tamper_Timer_ISR_StartEx(cyisraddress address)
{
    /* For all we know the interrupt is active. */
    Tamper_Timer_ISR_Disable();

    /* Set the ISR to point to the Tamper_Timer_ISR Interrupt. */
    Tamper_Timer_ISR_SetVector(address);

    /* Set the priority. */
    Tamper_Timer_ISR_SetPriority((uint8)Tamper_Timer_ISR_INTC_PRIOR_NUMBER);

    /* Enable it. */
    Tamper_Timer_ISR_Enable();
}


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_Stop
********************************************************************************
*
* Summary:
*   Disables and removes the interrupt.
*
* Parameters:  
*   None
*
* Return:
*   None
*
*******************************************************************************/
void Tamper_Timer_ISR_Stop(void)
{
    /* Disable this interrupt. */
    Tamper_Timer_ISR_Disable();

    /* Set the ISR to point to the passive one. */
    Tamper_Timer_ISR_SetVector(&IntDefaultHandler);
}


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_Interrupt
********************************************************************************
*
* Summary:
*   The default Interrupt Service Routine for Tamper_Timer_ISR.
*
*   Add custom code between the coments to keep the next version of this file
*   from over writting your code.
*
* Parameters:  
*
* Return:
*   None
*
*******************************************************************************/
CY_ISR(Tamper_Timer_ISR_Interrupt)
{
    #ifdef Tamper_Timer_ISR_INTERRUPT_INTERRUPT_CALLBACK
        Tamper_Timer_ISR_Interrupt_InterruptCallback();
    #endif /* Tamper_Timer_ISR_INTERRUPT_INTERRUPT_CALLBACK */ 

    /*  Place your Interrupt code here. */
    /* `#START Tamper_Timer_ISR_Interrupt` */
    //This is a 1 ms timer interrupt
    Tamper_Timer_ReadStatusRegister();
    
    switch(run_mode)
    {
        case MODE_F2F:
        case MODE_F2F + BASE_TOC:
        case MODE_F2F + BASE_PASS:    
            if ( f2f_acknowledge_active ){
                f2f_ack_delay_cnt++;
                if (f2f_ack_delay_cnt == 30){
                    PS_D1_Write(1);
                }
                if (f2f_ack_delay_cnt > 35){
                    PS_D1_Write(0);
                    f2f_acknowledge_active = 0;
                }
            }        
            f2f_alive_delay_cnt++;
            if ( f2f_alive_delay_cnt == 5000)
            {
                f2f_send_alive = 1;
            }
            break;
        case BASE_TOC + MODE_WIEGAND:
        case BASE_TOC + MODE_PAC:
//        case BASE_TOC + MODE_F2F:
        case BASE_TOC + MODE_HID:
        case BASE_TOC + MODE_OSDP:
            debug_msg_delay++;
            if (debug_msg_delay >= 5000) //5 seconds
            {
                debug_msg_delay = 0;
//                sendDebugMsg = 1; //0 yqh debug, clear in main() after sent.
            }
            break;
        default:;
    }

    if (buttonPressed){
        buttonDuration++;
    }    
    HandleOutputs();
    HandleInputs();
   
    /* `#END` */
}


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_SetVector
********************************************************************************
*
* Summary:
*   Change the ISR vector for the Interrupt. Note calling Tamper_Timer_ISR_Start
*   will override any effect this method would have had. To set the vector 
*   before the component has been started use Tamper_Timer_ISR_StartEx instead.
* 
*   When defining ISR functions, the CY_ISR and CY_ISR_PROTO macros should be 
*   used to provide consistent definition across compilers:
*
*   Function definition example:
*   CY_ISR(MyISR)
*   {
*   }
*
*   Function prototype example:
*     CY_ISR_PROTO(MyISR);
*
* Parameters:
*   address: Address of the ISR to set in the interrupt vector table.
*
* Return:
*   None
*
*******************************************************************************/
void Tamper_Timer_ISR_SetVector(cyisraddress address)
{
    cyisraddress * ramVectorTable;

    ramVectorTable = (cyisraddress *) *CYINT_VECT_TABLE;

    ramVectorTable[CYINT_IRQ_BASE + (uint32)Tamper_Timer_ISR__INTC_NUMBER] = address;
}


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_GetVector
********************************************************************************
*
* Summary:
*   Gets the "address" of the current ISR vector for the Interrupt.
*
* Parameters:
*   None
*
* Return:
*   Address of the ISR in the interrupt vector table.
*
*******************************************************************************/
cyisraddress Tamper_Timer_ISR_GetVector(void)
{
    cyisraddress * ramVectorTable;

    ramVectorTable = (cyisraddress *) *CYINT_VECT_TABLE;

    return ramVectorTable[CYINT_IRQ_BASE + (uint32)Tamper_Timer_ISR__INTC_NUMBER];
}


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_SetPriority
********************************************************************************
*
* Summary:
*   Sets the Priority of the Interrupt. 
*
*   Note calling Tamper_Timer_ISR_Start or Tamper_Timer_ISR_StartEx will 
*   override any effect this API would have had. This API should only be called
*   after Tamper_Timer_ISR_Start or Tamper_Timer_ISR_StartEx has been called. 
*   To set the initial priority for the component, use the Design-Wide Resources
*   Interrupt Editor.
*
*   Note This API has no effect on Non-maskable interrupt NMI).
*
* Parameters:
*   priority: Priority of the interrupt, 0 being the highest priority
*             PSoC 3 and PSoC 5LP: Priority is from 0 to 7.
*             PSoC 4: Priority is from 0 to 3.
*
* Return:
*   None
*
*******************************************************************************/
void Tamper_Timer_ISR_SetPriority(uint8 priority)
{
    *Tamper_Timer_ISR_INTC_PRIOR = priority << 5;
}


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_GetPriority
********************************************************************************
*
* Summary:
*   Gets the Priority of the Interrupt.
*
* Parameters:
*   None
*
* Return:
*   Priority of the interrupt, 0 being the highest priority
*    PSoC 3 and PSoC 5LP: Priority is from 0 to 7.
*    PSoC 4: Priority is from 0 to 3.
*
*******************************************************************************/
uint8 Tamper_Timer_ISR_GetPriority(void)
{
    uint8 priority;


    priority = *Tamper_Timer_ISR_INTC_PRIOR >> 5;

    return priority;
}


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_Enable
********************************************************************************
*
* Summary:
*   Enables the interrupt to the interrupt controller. Do not call this function
*   unless ISR_Start() has been called or the functionality of the ISR_Start() 
*   function, which sets the vector and the priority, has been called.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void Tamper_Timer_ISR_Enable(void)
{
    /* Enable the general interrupt. */
    *Tamper_Timer_ISR_INTC_SET_EN = Tamper_Timer_ISR__INTC_MASK;
}


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_GetState
********************************************************************************
*
* Summary:
*   Gets the state (enabled, disabled) of the Interrupt.
*
* Parameters:
*   None
*
* Return:
*   1 if enabled, 0 if disabled.
*
*******************************************************************************/
uint8 Tamper_Timer_ISR_GetState(void)
{
    /* Get the state of the general interrupt. */
    return ((*Tamper_Timer_ISR_INTC_SET_EN & (uint32)Tamper_Timer_ISR__INTC_MASK) != 0u) ? 1u:0u;
}


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_Disable
********************************************************************************
*
* Summary:
*   Disables the Interrupt in the interrupt controller.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void Tamper_Timer_ISR_Disable(void)
{
    /* Disable the general interrupt. */
    *Tamper_Timer_ISR_INTC_CLR_EN = Tamper_Timer_ISR__INTC_MASK;
}


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_SetPending
********************************************************************************
*
* Summary:
*   Causes the Interrupt to enter the pending state, a software method of
*   generating the interrupt.
*
* Parameters:
*   None
*
* Return:
*   None
*
* Side Effects:
*   If interrupts are enabled and the interrupt is set up properly, the ISR is
*   entered (depending on the priority of this interrupt and other pending 
*   interrupts).
*
*******************************************************************************/
void Tamper_Timer_ISR_SetPending(void)
{
    *Tamper_Timer_ISR_INTC_SET_PD = Tamper_Timer_ISR__INTC_MASK;
}


/*******************************************************************************
* Function Name: Tamper_Timer_ISR_ClearPending
********************************************************************************
*
* Summary:
*   Clears a pending interrupt in the interrupt controller.
*
*   Note Some interrupt sources are clear-on-read and require the block 
*   interrupt/status register to be read/cleared with the appropriate block API 
*   (GPIO, UART, and so on). Otherwise the ISR will continue to remain in 
*   pending state even though the interrupt itself is cleared using this API.
*
* Parameters:
*   None
*
* Return:
*   None
*
*******************************************************************************/
void Tamper_Timer_ISR_ClearPending(void)
{
    *Tamper_Timer_ISR_INTC_CLR_PD = Tamper_Timer_ISR__INTC_MASK;
}

#endif /* End check for removal by optimization */


/* [] END OF FILE */
