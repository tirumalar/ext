/* =======================================
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
#include "LED_0.h"
#include "LED_1.h"
#include "LED_2.h"
#include "i2cuart.h"
#include "osdp.h"


void initI2C(){
    	/* Starting EZI2C */
    ClearBufferI2C();
	EZI2Cs_Start();
    EZI2Cs_SetBuffer1(sizeof(MyI2C_Regs), sizeof(MyI2C_Regs), (volatile uint8 *) &MyI2C_Regs );
 
}
void ClearBufferI2C(){
   memset(&MyI2C_Regs, 0, sizeof(MyI2C_Regs));
}
void ClearBuffer()
{
    int i;
    for(i=0; i < BUFFER_LENGTH; i++)
    {
        MyI2C_Regs.buffer[i] = 0;
        CyDelayUs(1u); //need little delay for Release
    }
}

void SendPAC(unsigned char *bufp)
{
    int i,j;
    LED_GRN_Write(ON);
    LED_YEL_Write(ON);
  
    for( i = 0; i < 20; i++)
    {
        for( j= 0; j < PAC_LENGTH ; j++)
         UART_1_PutChar(bufp[j]);
    }

    LED_GRN_Write(OFF);
    LED_YEL_Write(OFF);
}

int ReadPAC(int passmode)
{
    int byteCount =0;
    char tmp;
    char ret = 0;
    unsigned char buffer[BUFFER_LENGTH];  
    int i;

    if(UART_1_ReadRxStatus() ==   UART_1_RX_STS_FIFO_NOTEMPTY)
    {
        Timeout_Timer_Isr_Disable();
        passmode_timeout = 0;
        Timeout_Timer_Isr_Enable();
        while(byteCount < PAC_LENGTH)
        {
            if(UART_1_ReadRxStatus() ==   UART_1_RX_STS_FIFO_NOTEMPTY)
            {
                tmp = UART_1_GetChar();
                if(tmp == 0x02)
                {
                    buffer[byteCount] = tmp;
                    byteCount++;
                }
                else if(byteCount > 0)
                {
                    buffer[byteCount] = tmp;
                    byteCount++;
                }
            }
            if (passmode_timeout > 2000) return(0);
        }
        
        
        MyI2C_Regs.data_length[0] = 0;
        MyI2C_Regs.data_length[1] = byteCount<<3;

        
        if ( passmode)
        {
            SendPAC(buffer);
        }
        else
        {
            for (i=0; i<PAC_LENGTH; i++)
            {
                MyI2C_Regs.buffer[i] = buffer[i];
                CyDelayUs(1u);  //add some delay for I2C
            }
            UART_1_ClearRxBuffer();
            card_in_buf = 1;
            card_ack = 0;
            MyI2C_Regs.status_out |= STAT_CHANGE | STAT_CARD_IN;
            MyI2C_Regs.cmd = CMD_NONE;
//            MyI2C_Regs.buffer1[13]++;
            ret = 1;
        }
    }
    return ret;
}

void SetupUART(char accessType)
{
    if(currentAccessType != accessType)
    {
        switch(accessType)
        {
            case PAC:
               OptSelect_Write(0);
                break;
            case HID:
               OptSelect_Write(1);
                break;
                
        }
        currentAccessType = accessType;
    }
}

/* [] END OF FILE */

