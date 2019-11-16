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

#include <project.h>
#include "system.h"
#include "wiegand.h"
#include "f2f.h"
#include "power.h"
#include "inout.h"
#include "i2cuart.h"
#include "osdp.h"
#include "toc.h"
#include "commandhandler.h"
#include "hwinit.h"

/******************************************//**
All harware initialization 
**********************************************/
/***********************************
    Sets the breakout board version.
*************************************/
void setVersion()
{
    MyI2C_Regs.hw_version[0] = 0x71;
    MyI2C_Regs.hw_version[1] = 0x42;
    MyI2C_Regs.hw_version[2] = 0x0A;
    
    MyI2C_Regs.sw_version[0] = 6;
    MyI2C_Regs.sw_version[1] = 0;
    MyI2C_Regs.sw_version[2] = 2;
}


void initICM_HW(){
    
     CyGlobalIntEnable;  /* Uncomment this line to enable global interrupts. */

//    /* Intializing the Timers and interrupts */
    UART_TIMER_Start();
	UART_TIMER_ISR_StartEx(UART_TIMER_ISR_Interrupt);
    Tamper_Timer_Start();
    Tamper_Timer_ISR_StartEx(Tamper_Timer_ISR_Interrupt);
    Timeout_Timer_Start();
    Timeout_Timer_Isr_StartEx(Timeout_Timer_Isr_Interrupt);
    initWiegand(); ///< Wiegand initialization. All functions for Wiegand realization should be in wiegand.c
    initF2F(); ///< F2F initialization. All functions for F2F realization should be in f2f.c
    initI2C(); ///< I2C initialization. All functions for I2C communication should be implement in i2c.c 
  
    /* Starting UART for PAC & HID */
    UART_1_Start();
    SU_Start(&s,1,PARITY_EVEN,1);
    
    /* Turning off both LEDS */
    LED_GRN_Write(OFF); // LED for signaling Output
    LED_YEL_Write(OFF); // LED for signaling Input
    LED_RED_Write(OFF); // turn off tamper LED
    
    /* Intializing the I2C Reg value */
    MyI2C_Regs.factory_reset = 0;
    SetHardwareVals(TAMPERCR_POL);  // TAMPER_POL defaulted to 1 (normal config with active tamper = 0)

    setVersion();
    
    // initialize card reader tamper polarity bit:  1=normal
    // This is only for TEST and must be set by webconfig/motherboard.
    // tamperCR_polarity = 1;
    hw_old = 0;
//    TOC_port_init = 0;
    card_ack = 0;
    card_in_buf = 0;
    
    initOSDP(); ///< OSDP initialization. All functions for OSDP realization should be implement in osdp.c
    
    /* push button */
    buttonPressed=0;
    buttonReleased=0;
    buttonDuration=0;
    isWiegandDataReady = 0;
    wiegandBitCount = 0;
    wiegandByteCount = 0;
    clearWiegandBuffer();
    isr_WD_SYNC_Start();
//    isr_WD_SYNC_Disable();    
    isr_SW_Rst_Start(); 
    Clock_7_Start();
    Clock_8_Start();
    CyDelay(10);
    Status_Reg_TamperPort_Read();
    tp_old = Status_Reg_TamperPort_Read();


}


/* [] END OF FILE */
