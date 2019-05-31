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

// GRI 1-13-2015 - Revised for 3.1.1;  TAMPER_OUT must not be set in SetHardwareVals - has been moved to HandleInputs.
// GRI 2-04-2015 - Revised for 3.1.5;  Extensive revision to input signal checker to add debounce/noise immunity.

#include <project.h>
#include "system.h"
#include "i2cuart.h"
#include "toc.h"

//int pwr_cnt;

// this function reads the current settings from the file register 
// and configures the outputs accordingly
void  SetHardwareVals(char val)
{
    // 1 - low ; 0 - high

    if((val&LED_OUT_RED) == LED_OUT_RED) 
	    LEDR_OUT_Write(1);
    else
        LEDR_OUT_Write(0);
        
    if((val&LED_OUT_GRN) == LED_OUT_GRN) 
	    LEDG_OUT_Write(1);
    else
        LEDG_OUT_Write(0);
        
	if((val&SOUNDER_OUT) == SOUNDER_OUT) 
	    SOUND_OUT_Write(1);
    else
        SOUND_OUT_Write(0);

    // TAMPER_OUT logic is now moved to HandleInputs.  (GRI 1-13-15, 2-3-15)
    // TAMPER_POL is incoming webconfig polarity bit for CR tamper.
    if((val&TAMPERCR_POL) == TAMPERCR_POL)  
        tamperCR_polarity = 1;
    else
        tamperCR_polarity = 0;
                
    if((val&RELAY_1_OUT) == RELAY_1_OUT) 
        RELAY_A_Write(1);
    else
        RELAY_A_Write(0);
        
    if((val&RELAY_2_OUT) == RELAY_2_OUT) 
        RELAY_B_Write(1);
    else
        RELAY_B_Write(0);
	
    if((val&TAMPER_OUT) == TAMPER_OUT)  
        tamper_out_polarity = 0;  //0 is high, same as input.
    else
        tamper_out_polarity = 1;

} 

/*void  SetHardwareVals(char val)
{
    // 1 - low ; 0 - high
    LEDR_OUT_Write(val&LED_OUT_RED);
    LEDG_OUT_Write((val&LED_OUT_GRN) >> 1);
    SOUND_OUT_Write((val&SOUNDER_OUT) >> 2);
    tamperCR_polarity = (val&TAMPERCR_POL) >> 3;
    RELAY_A_Write((val&RELAY_1_OUT) >> 4);
    RELAY_B_Write((val&RELAY_2_OUT) >> 5);
    tamper_out_polarity = 0x1 & (~((val&TAMPER_OUT) >> 6)); //0 is high, same as input. 
    // TAMPER_OUT logic is now moved to HandleInputs.  (GRI 1-13-15, 2-3-15)
    // TAMPER_POL is incoming webconfig polarity bit for CR tamper.
} 
*/
#if 0
uint8 GetHardwareVals()
{
    uint8 val = ~(0x07 & (~Status_Reg_HwPort_Read()));
    return val;
}
#endif

uint8 GetHardwareVals()
{
uint8 val = 0;// = (0x07 & (Status_Reg_HwPort_Read()));
   // val = (0x07 & ((LEDR_IN_Read()? 0:1) | (LEDG_IN_Read()? 0:2))) ;
    val = ~(0x07 & ((LEDR_IN_Read()? 0:1) | (LEDG_IN_Read()? 0:2))) ;
  // val = (LEDR_IN_Read()? 1:0) | (LEDG_IN_Read()? 2:0) ;

    return val;
}

uint8 GetResetButton()
{
	uint8 val = 0;
    
    if (buttonReleased){
        buttonReleased = 0;
        if(buttonDuration < FCTRYRST_TIME_MS){ //less than 5 seconds, ignore
            val = 0; 
            MyI2C_Regs.factory_reset = 0;
        }else if(buttonDuration >=FCTRYRST_TIME_MS && buttonDuration < RSTR_TIME_MS){ //5~45 seconds
            LED_RED_Write(ON);
            LED_GRN_Write(ON);
            LED_YEL_Write(ON);
            val = BTN_STATUS_CHG | FCTRY_RST;  // active reset initiated
            MyI2C_Regs.factory_reset = FACRST_CODE;
        }else if(buttonDuration>=RSTR_TIME_MS){ //45 or more seconds, factory restore
            LED_RED_Write(ON);
            LED_GRN_Write(ON);
            LED_YEL_Write(ON);
            val = BTN_STATUS_CHG | FCTRY_RSTR; 
            MyI2C_Regs.factory_reset = RESTRE_CODE;
        }
    }
    return val;
}

uint8 GetTamperVals()
{
	uint8 val = 0;
	//val = 0x07 & ~Status_Reg_TamperPort_Read();
    return val;
}

void  HandleOutputs()   // this executes every 1 ms on Tamper_Timer_ISR_Interrupt   (AD - ?)
{
	char stat = MyI2C_Regs.status_in;
    if( stat & STAT_CHANGE)
    {
        if(stat & STAT_ACS_OUT)
           SetHardwareVals(MyI2C_Regs.acs_inputs);
        
        if(stat & STAT_CARD_ACK){
            MyI2C_Regs.status_in = 0;
            card_ack = 1; //acked
            MyI2C_Regs.buffer1[16]++;
            resetImgBuf();
        }
        MyI2C_Regs.status_in = 0; 
    }
    
}

void HandleTamper(){
    uint8 statusTamper = 0;
    statusTamper = 0x03 & GetTamperVals();
    char first_read;
    char second_read;
   
    /* TAMPER OUT signal to ACS Panel  GRI 1-13-15
       this accounts for configuration of tamper input from card reader  GRI 1-13-15 */
  
       
    if(((statusTamper & 0x2) == 0x2)  
        || (((statusTamper & 0x1) == 1) && (tamperCR_polarity == 1 ))
        || (((statusTamper & 0x1) == 0) && (tamperCR_polarity == 0 )))
    {
        if (tamper_out_polarity)
            TAMPER_OUT_Write(TAMPERHIGH); 
        else
            TAMPER_OUT_Write(TAMPERLOW);
        LED_RED_Write(ON); //LED on
    }
    else
    {
        if(tamper_out_polarity)
            TAMPER_OUT_Write(TAMPERLOW); 
        else
            TAMPER_OUT_Write(TAMPERHIGH);
        LED_RED_Write(OFF); //LED off
    }
    first_read = statusTamper;//GetTamperVals();  // read inputs first time
	if (first_read != tp_old)  //change occurred?
	{
        CyDelayUs(100); //delay 100us for debounce
        second_read = statusTamper;//GetTamperVals();  // read inputs second time
        
        if (first_read == second_read)    // data is stable?
        {   
            // If Data is stable, change the status reg
	        MyI2C_Regs.tamper_in = second_read;                //set output reg to MB
            tp_old = second_read;                                   //store latest value
	        MyI2C_Regs.status_out |= (STAT_TAMPER_IN | STAT_CHANGE);   //indicate change in ACS_IN
        }
	}
    
           // If Data is stable, change the status reg
//	MyI2C_Regs.tamper_in = statusTamper;                //set output reg to MB
//    MyI2C_Regs.status_out |= (STAT_TAMPER_IN | STAT_CHANGE);   //indicate change in ACS_IN
 
}

void HandleInputs()   // this executes every 1 ms on Tamper_Timer_ISR_Interrupt     (AD - ?)
{
    char first_read;
    char second_read;
    HandleTamper();
    //This section checks for input signal changes
    first_read = GetHardwareVals();  // read inputs first time
	if (first_read != hw_old)  //change occurred?
	{
        CyDelayUs(100); //delay 100us for debounce
        second_read = GetHardwareVals();  // read inputs second time
        
        if (first_read == second_read)    // data is stable?
        {   
            // If Data is stable, change the status reg
            
	        MyI2C_Regs.acs_outputs = second_read;                //set output reg to MB
            hw_old = second_read;                                   //store latest value
	        MyI2C_Regs.status_out |= (STAT_ACS_IN | STAT_CHANGE);   //indicate change in ACS_IN
        }
	}
    
 
    
    first_read = GetResetButton();
    if (first_read)
    {
        MyI2C_Regs.pushbtn_in = first_read; //set the push button status
    }
}

/* [] END OF FILE */
