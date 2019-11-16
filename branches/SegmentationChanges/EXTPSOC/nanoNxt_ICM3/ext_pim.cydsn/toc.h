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
#ifndef __TOC_INCLUDED__   
#define __TOC_INCLUDED__  
#include <CyLib.h>
#include "system.h"
    
    
#define SP_ENVELOPE         0x0E
#define RSP_MESSAGE			0x5C
extern uint8 tlv_ready;
    
CY_ISR_PROTO(ImgRxInt);          // process Image Template Rx interrupt



void ReadImageCard();

void initTocReaderUART(void);
void resetImgBuf(void);  
uint16_t UpdateCrc16Ccitt (uint16_t crc, uint8_t data);

//send to rs485 port
void SendWaveLynxReader(unsigned char *, int);

#endif 

/* [] END OF FILE */
