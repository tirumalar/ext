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
#ifndef __I2CUART_INCLUDED__   
#define __I2CUART_INCLUDED__  
#include <CyLib.h>
#include "system.h"
    
//#ifdef __cplusplus
//extern "C" {
//#endif
#define PAC_LENGTH 12

void setVersion();
void initI2C(void);    
void ClearBuffer();
void ClearBufferI2C();
void SendPAC(unsigned char *);
int ReadPAC(int);
void SetupUART(char);


//#ifdef __cplusplus
//}
//#endif

#endif 

/* [] END OF FILE */
