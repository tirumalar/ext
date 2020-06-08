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

// GRI 2-2-15: Modified ReadWiegand.

#include <project.h>
#include "system.h"
#include "wiegand.h"
#include "Pulse_50us.h"


void initWiegand(){
      /*Wiegand*/
    Pulse_50us_Init();
    isr_50us_StartEx(Pin_50us_Reset);
}

CY_ISR(Pin_50us_Reset)
{
    OptSelect_Write(2);
	PS_D1_Write(0);
    isr_50us_GetState();
    Pulse_50us_ReadStatusRegister();
    Pulse_50us_RestoreConfig();
    Pulse_50us_Init();
}
void send_D0_50us_pulse()
{
    OptSelect_Write(3); 
    Pulse_50us_Enable();
}
void send_D1_50us_pulse()
{
    PS_D1_Write(1);
    Pulse_50us_Enable();
}

void clearWiegandBuffer(){
    for(int i = 0; i < WIEGAND_BUFFER_LENGTH; i++){
        wiegandBuffer[i] = 0;
    }
}

void SendWeigand(unsigned char *bufp, int nBits)  //Function to send Wiegand Data
{
	int x,y;
	char  d1;
    char count = 0;
    
	/*                        Wiegand protocol
	    A pulse width of 50us to define data
	    A wait period of 2ms after transmitting the above defined pulse
    */
    if (nBits > (WIEGAND_BUFFER_LENGTH << 3)) return; //garbage in
    OptSelect_Write(2); //0, both output high
    PS_D1_Write(0);
    CyDelay(5);
	for (x=0; x< WIEGAND_BUFFER_LENGTH; x++) // Loop over data written from the OMAP
	{
        d1 = bufp[x];
		for (y=0; y <8; y++)  //Transmit each bit at a time
		{	
			count++;
            if (count > nBits)
			{
                OptSelect_Write(2); //0, both output high
                PS_D1_Write(0);
    
                return;
			}

		    if (d1&0x80) // Check if it a 1 or a 0
			{
                send_D1_50us_pulse();
			}
			else
			{
                send_D0_50us_pulse();
			}			    
			d1=d1<<1;
            CyDelayUs(1030); //1030 = 2ms
		}
	}

}

int ReadWeigand(int nBits, int passmode)
{
    
    if(isWiegandDataReady){
        
        isr_WD_SYNC_Disable();
            MyI2C_Regs.buffer1[9] = (wiegandBitCount>>8) & 0xFF;
            MyI2C_Regs.buffer1[10] = wiegandBitCount & 0xFF;

        if(wiegandBitCount){//== nBits){
           
 //           MyI2C_Regs.data_length[0] = (nBits>>8) & 0xFF;
 //           MyI2C_Regs.data_length[1] = nBits & 0xFF;
           
            if(passmode || ((run_mode == MODE_WIEGAND + PIN_PASS) && (wiegandBitCount == 8 || wiegandBitCount == 4)))
            {
                SendWeigand(wiegandBuffer, wiegandBitCount);//nBits);
            }
            else
            {
             MyI2C_Regs.bits = wiegandBitCount;                
             MyI2C_Regs.data_length[0] = (wiegandBitCount>>8) & 0xFF;
             MyI2C_Regs.data_length[1] = wiegandBitCount & 0xFF;
                for (int i = 0; i <= wiegandByteCount; i ++)
                {
                    MyI2C_Regs.buffer[i] = wiegandBuffer[i];
                    CyDelayUs(1u);  //add some delay for I2C  ??????
                }
                card_ack = 0;
                card_in_buf = 1;
                MyI2C_Regs.status_out |= STAT_CHANGE | STAT_CARD_IN ;
                MyI2C_Regs.cmd = CMD_NONE;
                MyI2C_Regs.buffer1[12]++;
            
            }
        }
	    isWiegandDataReady = 0;
        testBitCount = 0;
        wiegandByteCount = 0;
        wiegandBitCount = 0;
    	clearWiegandBuffer();
        isr_WD_SYNC_Enable();
        return 1;
    }
    return 0;
}



/* [] END OF FILE */
