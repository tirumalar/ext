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
#include "f2f.h"

/* F2F panel keep alive */
char f2f_send_alive;
int f2f_alive_delay_cnt;
void initF2F(){
    
    /*F2F*/
    isr_F2F_edge_StartEx(isr_F2F_edge_Interrupt);
    
}
void MakeLineTransition(){
    uint8 flag = PS_D0_Read();
	if (flag){
        OptSelect_Write(2); //0
    } else {
        OptSelect_Write(3); //1
    }
    CyDelayUs(400);
}


void SendF2F(const unsigned char *buf, const int length){
   int x,b,len;
	char  d0;
    
    len = 3 + length; //length is in bytes
    if(len > BUFFER_LENGTH) len = BUFFER_LENGTH; //if garbage in
    
    Tamper_Timer_ISR_Disable();
    f2f_send_alive = 0;
    f2f_alive_delay_cnt = 0;
    Tamper_Timer_ISR_Enable();
    OptSelect_Write(1); 
    OptSelect_Write(2); 
    for (x=0; x< len; x++)
	{   	
		d0 = buf[x];	
        for (b=0; b <8; b++)
		{
            MakeLineTransition();
            if(d0&0x80){
                MakeLineTransition();
            } else {
                CyDelayUs(400);
            }
            d0<<=1;
        }
    }
    OptSelect_Write(2); //0
    CyDelay(165); //165, the time for ACS message processing and ACS ACK is 83ms
}




int ReadF2F(int nBits, int passmode)
{
    int bitCount;
    int byteCount;
    int hp;
    char nbit;
    int i;
    unsigned char started;
    int width;
    unsigned char buffer[BUFFER_LENGTH],pass_buf[BUFFER_LENGTH+1];
    
    // enable ack line
    PS_D1_Write(0);
    hp = 21;  //21 17
    byteCount = 0;  
    nbit = 0;
    bitCount = 1;
    started = 0;
    card_in_buf = 0;
    
    for (i=0; i<BUFFER_LENGTH; i++)
    {
        buffer[i]=0;
    }
    
    if (passmode)
    {
        Timeout_Timer_Isr_Disable();
        passmode_timeout = 0;
        Timeout_Timer_Isr_Enable();
    }

    while( (byteCount*8+bitCount) <= nBits  )
    {
        
        //if((bitCount == 0) && (byteCount == 0))
        {
           void HandleUSB();
           HandleUSB();
        }
        if (passmode == 1)
        {
            if(passmode_timeout > 2000)
            {
                passmode_timeout = 0;  //may be removed
                return(0);
            }
        }
        isr_F2F_edge_Disable();
        width = f2f_pulsewidth;             
        isr_F2F_edge_Enable();
        
        if(width > 0)
        {
            if (passmode)
            {
                Timeout_Timer_Isr_Disable();
                passmode_timeout = 0;
                Timeout_Timer_Isr_Enable();
            }

            isr_F2F_edge_Disable();
            f2f_pulsewidth = 0;
            isr_F2F_edge_Enable();
            
            if (bitCount > 7)
            {
                bitCount = 0;
                byteCount++;
            }

            if(started == 1 && width >= ((int)(0.75*hp)) && width < 0x40 ) {
                nbit = 0;
                bitCount++;
            }
            /* A pulse width of 1msec is considered a 0 && two pulse widths of 0.5 msec is consdiered as 1
               Timeout timer is configured to update timeoutcnt every 50 usecs      */
            else if( width < ((int)(0.75*hp)))
            {
                nbit++;
                if(nbit > 1)
                {
                  buffer[byteCount] |= (0x01 << (7-bitCount));   
                  bitCount++;
                  nbit = 0; 
                  started = 1;
                }
            } 
            else if (width >= 0x40 && started == 1 )
            {
                break;
            }
        } //got pulse
       
    }  //while
    
    MyI2C_Regs.data_length[0] = nBits>>8;
    MyI2C_Regs.data_length[1] = nBits;

    
    if (passmode)
    {
        if (byteCount > 4){
            pass_buf[0]=0;  //add a 0 in front for the clock regenerate
            for (i=0; i<BUFFER_LENGTH; i++)
            {
                pass_buf[i+1] = buffer[i];
            }
            SendF2F(pass_buf, byteCount);  //length is bytes
        }
    }
    else  //send to firmware
    {
        for (i=0; i<BUFFER_LENGTH; i++)
        {
            MyI2C_Regs.buffer[i] = buffer[i];
            CyDelayUs(1u);  //add some delay for I2C bus
        }

        MyI2C_Regs.status_out |= STAT_CHANGE | STAT_CARD_IN ;
        card_in_buf = 1;
        card_ack = 0;
        MyI2C_Regs.cmd = CMD_NONE;  //to prevent send again
        MyI2C_Regs.buffer1[14]++;
        return 1;
    }
    return 0;
}


/* [] END OF FILE */
