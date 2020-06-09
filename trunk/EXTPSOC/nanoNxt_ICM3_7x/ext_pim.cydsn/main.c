/* ========================================
 *
 * NanoNXT Breakout Board
 * Copyright Eyelock.Inc, 2014, 2015
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * 
 * Version History:
 * ================
 * 3.0.0 : Ported Nano Breakout board functionality without HID/F2F.
 * 3.0.1 : Relay output moved from regular output mode to auxillary output mode
 * 3.0.2 : F2F functionality Added.
 * 3.0.3 : HID functionality added using SW UART for 26 bit weigand & SW UART tested on PAC comaptibilty 
 * 3.0.4 : Tamper In is directly routed to Tamper out
 * 3.0.5 : Software Reset button functionality added
 * 3.0.6 : HID functionality modified to support all types of weigand bit formats. 
 * 3.0.7 : Bootloader functionality added.
 * 3.0.8 : Reed Tamper Inverted for Beta.
 * 3.0.9 : Ported code for ICM 3(PSOC 5).
 * 3.1.0 : Bootloader functionality added to ICM3.
 * 3.1.1 : GRI 1-13-15: Tamper signals repaired. Tamper moved to main code block.  Tamper polarity added.
 * 3.1.2 : GRI 1-28-15: Tamper moved to HandleInputs.  I2C bus clock was corrected to be 100kHz to match motherboard (was 400kHz).
 * 3.1.3 : GRI 1-30-15: Removed all "ClearBuffer()" instances.
 * 3.1.4 : GRI 2-02-15: Added ACK bit for card reads to prevent hanging after card read. PSoC will wait for ACK for 5 sec, then re-notify MB.  Changed TamperTimer period to 1ms. 
 * 3.1.5 : GRI 2-04-15: Extensive revision to input signal handler to add noise immunity/debounce (see end of inout.c).
 * 3.1.6 : Added F2F card reader functionality
 * 3.1.7 : Debouncer added to WD0 && Functionality added to detect pulsewidth in F2F
 * 3.1.8 : Added F2F conditionality to turn on timers
 * 3.1.9 : Fixed the bootloader bug for PSOC creator 3.1  Use the debug NxtICM3_bootloader for reference
 * 3.1.10 : Make F2F pulse timing adaptive to the input signal
 * 3.1.15 : OSDP Dual Authentication
 * 3.1.16 : support F2F variable length messages
 * 3.2.0 : Fix F2F to accommodate prompt signal ATH-870
 * 3.2.1 : support Tamper Out Polarity Settings in the Web
 * 3.2.2 : reverse the tamper output polarity
 * 3.2.3 : 16 bits I2C
 * 3.2.4 : branched from 3.2.2, fixed pulse width narrowed by tamper interrupt problem
 * 3.2.5 : help firmware set the card in status again in one second if no ack comming.
 * 3.3.2 : same as 3.2.5 for GA release
 * 3.4.3 : add TOC template on card
 * 3.4.4 : add Pass Through mode
 * 3.4.5 : Increased the timeout delay on PAC for Pass Through Mode.
 * 3.4.6 : Add PAC for TOC Mode.
 * 3.4.7 : Add RS485 sending function for the BLE Mode and the Iris Timeout interface commands.
 * 3.4.8 : Fixed OSDP compiling issue, use osdp.h function prototype.
 * 3.4.9 : Fixed ATH-1171 - Pass through (Iris or Card mode) does not 
            authenticate two consecutive cards problem
 * 5.0.2 : Add 200 bits Wiegand
 * 5.0.4 : Simplified OSDP, fixed reader message misalignment problem
 * ========================================
*/
#include <project.h>
#include "system.h"
#include "commandhandler.h"
#include "hwinit.h"
#include "osdp.h"
#include "inout.h"
#include "wiegand.h"
#include "f2f.h"
#include "power.h"
#include "inout.h"
#include "i2cuart.h"
#include "osdp.h"
#include "toc.h"

unsigned char bu[16];
int z=0;
char TOC_port_init;


/***********************************
    Sets the breakout board version.
*************************************/
void SetVersion()
{
    MyI2C_Regs.hw_version[0] = 0x71;
    MyI2C_Regs.hw_version[1] = 0x42;
    MyI2C_Regs.hw_version[2] = 0x0B;
    
    //MyI2C_Regs.sw_version[0] = 6;
    //MyI2C_Regs.sw_version[1] = 0;
    //MyI2C_Regs.sw_version[2] = 5;
    
    MyI2C_Regs.sw_version[0] = 7;
    MyI2C_Regs.sw_version[1] = 0;
    MyI2C_Regs.sw_version[2] = 0;
}



int main()
{
    int ack_time_cnt; // counter for acknowledge timeout
    int tmp;
    
    POE_RST_Write(1);
    POE_OIM_SDn_Write(0);
    CyGlobalIntEnable;  /* Uncomment this line to enable global interrupts. */
    ST_LED_0_Write(1);
    ST_LED_1_Write(1);
    ST_LED_2_Write(1);
    SF2F_Write(1);
    I2C_Master_Start();
    isr_WD_SYNC_Start();
    rtc_write_reg(0x25, 0x02);
    rtc_write_reg(0x28, 0x00);
    ST_LED_1_Write(0);
    rtc_read(0, MyI2C_Regs.buffer, 8);
    ST_LED_1_Write(1);
    rtc_read(0,bu,8);
    SleepTimer_Start();
    isr_Sleep_Start();
    ST_LED_1_Write(0);
    CyDelay(500);
    ST_LED_1_Write(1);
    ST_LED_2_Write(0);
    CyDelay(500);
    ST_LED_2_Write(1);
    ST_LED_0_Write(0);
    CyDelay(500);
    ST_LED_0_Write(1);
    ST_LED_2_Write(0);
    ST_LED_1_Write(0);
    USBUART_Start(0, USBUART_5V_OPERATION);
    EEPROM_Start();
    CySetTemp();
    if (EEPROM_ReadByte(0)==0x55)
    {
        EEPROM_EraseSector(0);
        ST_LED_2_Write(1);
    }
    POE_OIM_En(1);
//    /* Intializing the Timers and interrupts */

    UART_TIMER_Start();
	UART_TIMER_ISR_StartEx(UART_TIMER_ISR_Interrupt);
    Tamper_Timer_Start();
    Tamper_Timer_ISR_StartEx(Tamper_Timer_ISR_Interrupt);
    Timeout_Timer_Start();
    Timeout_Timer_Isr_StartEx(Timeout_Timer_Isr_Interrupt);

    /*Wiegand*/
    Pulse_50us_Init();
    isr_50us_StartEx(Pin_50us_Reset);

    /*F2F*/
    isr_F2F_edge_StartEx(isr_F2F_edge_Interrupt);
    
	
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
    
    SetVersion();
    
    // initialize card reader tamper polarity bit:  1=normal
    // This is only for TEST and must be set by webconfig/motherboard.
    // tamperCR_polarity = 1;
    hw_old = 0;
    tp_old = 0;
    TOC_port_init = 0;
    card_ack = 0;
    
    initOSDP();
    
    /* push button */
    buttonPressed=0;
    buttonReleased=0;
    buttonDuration=0;
    isr_SW_Rst_Start(); 
      //ik  TX_EN_Write(1);

    UART_ACS_LoadRxConfig();
    //while(1)
    //      UART_ACS_PutArray("test test",strlen("test test")); 
    if (UART_ACS_GetRxBufferSize()> 0)
    {
        tmp = UART_ACS_GetChar();
    }
    POE_OIM_En(1);
    while(1)
    {
        static int st_div=0;
        HandleUSB();
        HandleCommand();
        if ((MyI2C_Regs.status_out==0 ) && isWiegandDataReady)
            MyI2C_Regs.status_out |= STAT_CHANGE | STAT_CARD_IN;
        continue;
        if (0)
        {

            EEPROM_EraseSector(0);
            EEPROM_WriteByte(0x55,0);
            CySoftwareReset();
        }
        if (card_in_buf == 1)  // card read requires ack from motherboard
        {
            ack_time_cnt = 0;
            while (card_ack == 0)   // waiting for card read acknowledge here
            {
                  HandleUSB();
                CyDelay(10);    //
                ack_time_cnt++;  // increment time counter every 10ms while waiting for ack
                if (ack_time_cnt >= 100)    //  re-notify and break.
                {
                    MyI2C_Regs.status_out |= STAT_CHANGE | STAT_CARD_IN ;   // if timeout, set status bit a second time to notify motherboard
                    MyI2C_Regs.buffer1[11]++;
                    break;
                }
            }//while
        } //if
    } //main loop
} //end of main



/* [] END OF FILE */
