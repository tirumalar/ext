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
#pragma once
#include <project.h>
#include "system.h"

struct OSDP_MSG_HEADER {
    unsigned char addr;
    union{
        short len;
        unsigned char length[2];
    }u; 
    unsigned char ctrl;
    unsigned char seq;
    unsigned char checksum;
};

typedef struct  {
    unsigned char buf[OSDP_PANEL_BUFFER_LENGTH];
    unsigned char len;
    unsigned char pending;
}PanelBuf;   
 
typedef struct  {
    unsigned char buf[OSDP_READER_BUFFER_LENGTH];
    unsigned char len;
}ReaderBuf;   

void initOSDP(void);   
void HandleOSDP(void);
/* OSDP Single */
void CopyI2cPanelBuffer();
void ClearOsdpBuffer(void);
void SendOSDP(void);
void ReadOSDP(void);
void SetPanelSpeed(char);

/* OSDP Dual */
CY_ISR_PROTO(KeepAliveInterruptHandler);
CY_ISR_PROTO(OSDPReaderRxInt);          // UART Rx interrupt
void initOSDPReaderUART(void);
void ClearOsdpReaderBuffer(void);
void SendOSDPReader(void);
void ReadOSDPReader(void);
void SetReaderSpeed(char);
void CopyI2cReaderBuffer(void);
int osdp_timeout;


/* Pass Through */
void Pass_OSDP(ReaderBuf *);


/* [] END OF FILE */
