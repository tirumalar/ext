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
#include "hid.h"


void SendHID(){
   
    Tamper_Timer_Stop();
    
    char serialOut[20];
	char tmp;
	int i,j;
	int padBytes;
    int xbytes;
    int ybits;
    MyI2C_Regs.data_length[0] = 0;
    MyI2C_Regs.data_length[1] = 26;
    xbytes =  MyI2C_Regs.data_length[1]/8;
    ybits =   MyI2C_Regs.data_length[1] - (xbytes*8);
    padBytes = 18  -  xbytes;
    
    if(ybits  >  0)
	   padBytes = padBytes - 1;
    
    for (i = 0;i < padBytes;i++)
    {
       serialOut[i] = 0x00;
       SU_PutChar(&s,serialOut[i]);
    }
	
    serialOut[14] = 0x01 << ybits;
	tmp = MyI2C_Regs.buffer[0] & (0xFF << (8-ybits));		
    tmp = tmp >> (8-ybits);
	serialOut[i] |= tmp; 
	SU_PutChar(&s,serialOut[i]);
	i++;
		
	for (j = 1; j< (18- padBytes); j++)
	{
		serialOut[i] = (MyI2C_Regs.buffer[j-1] << ybits );// & (0xFF << ybits);
		tmp = (MyI2C_Regs.buffer[j] >> (8-ybits));// & (0xFF >> (8-ybits));
		serialOut[i] |= tmp;
		SU_PutChar(&s,serialOut[i]);
		i++;
	}
    
    Tamper_Timer_Start();    
    serialOut[i] = 0x00;
}


/* [] END OF FILE */
