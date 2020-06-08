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
#include "osdp.h"

////////////////OSDP PANEL//////////////////

PanelBuf panelReadBuf, panelSendBuf;

struct OSDP_MSG_HEADER osdp_msg_header,osdp_reader_msg_header;

void initOSDP(){
    SetPanelSpeed(MyI2C_Regs.osdp_rate);
    UART_ACS_Start();
    ClearOsdpBuffer();
    
    Clock_6_Enable();
    Clock_6_SetDividerValue(CLKDV_9600); //set the baud rate to default 9600
    UART_READER_Start();
    ClearOsdpReaderBuffer();
    
    isr_keepalive_StartEx(KeepAliveInterruptHandler);
    OSDPReaderTimer_Start();
    initOSDPReaderUART();
}

void copyPanelBufI2C(PanelBuf *p)
{
    int i;
    MyI2C_Regs.osdp_data_length = p->len;
    if (p->len >  OSDP_PANEL_BUFFER_LENGTH) p->len = OSDP_PANEL_BUFFER_LENGTH;
    for (i=0; i < p->len; i++)
    {
        MyI2C_Regs.osdp_buffer[i] = p->buf[i];
        CyDelayUs(1u);
    }
}

void CopyI2cPanelBuffer()
{
    int i;
    panelSendBuf.len = MyI2C_Regs.osdp_data_length;
    if (panelSendBuf.len >  OSDP_PANEL_BUFFER_LENGTH) panelSendBuf.len = OSDP_PANEL_BUFFER_LENGTH;
    for (i=0; i < panelSendBuf.len; i++){
        panelSendBuf.buf[i] = MyI2C_Regs.osdp_buffer[i];
    }
    panelSendBuf.pending = 1;  //ready to send
    SendOSDP();
    panelSendBuf.pending = 0; //send out command
    MyI2C_Regs.buffer1[1] ++; 
}

void ClearOsdpBuffer()
{
    int i;
    UART_ACS_ClearRxBuffer();
    for ( i=0; i<OSDP_PANEL_BUFFER_LENGTH; i++)
    {
        MyI2C_Regs.osdp_buffer[i]=0;
        CyDelayUs(1u);
    }
    MyI2C_Regs.osdp_data_length = 0;
}
/*
unsigned char calcChecksum(unsigned char * msg, int len)
{
    unsigned char i, ret;
    unsigned char sum = 0;
    for(i = 0; i < len; i++)
    {
        sum += msg[i];
    }
    ret = (unsigned char)((0x100 - sum) & 0xFF);
    return ret;
}
*/
void SendOSDP()
{

//    MyI2C_Regs.buffer1[13]++;
    Mode_Sel_ACS_Write(1); //set io chip to 485
    TX_EN_Write(1);
    UART_ACS_LoadTxConfig();
    CyDelay(10); //Driver Output Enable Time, do not reduce.
    
    //panelBuf panelSendBuf;
    PanelBuf temp;
    temp.buf[0] = 0x00;
    temp.buf[1] = 0x00;
    temp.len = 2;
    UART_ACS_PutArray(temp.buf,temp.len);
    if(panelSendBuf.buf[0] == 0x53){
        UART_ACS_PutArray(panelSendBuf.buf,panelSendBuf.len);
       // UART_ACS_PutArray(temp.buf,temp.len);
        UART_ACS_PutCRLF(temp.buf[0]); 
//        CyDelay(5); //minmum 4m
    }
    //while(UART_ACS_GetTxBufferSize() != 0);
    
    CyDelay(50);

    TX_EN_Write(0);
    Mode_Sel_ACS_Write(1); //set the chip to RS485 mode
    UART_ACS_LoadRxConfig();    
    CyDelay(10);
    MyI2C_Regs.osdp_cmd = OSDP_READ; //send only once

    //should be in the send message function   MyI2C_Regs.osdp_status |= OSDP_PANEL_DATA_SENT;
}

/*
void sendOSDPAck(struct OSDP_MSG_HEADER msg){
    
    panelSendBuf.buf[0] = 0x53;
    panelSendBuf.buf[1] = msg.addr | 0x80;
    panelSendBuf.buf[2] = 7;
    panelSendBuf.buf[3] = 0;
    panelSendBuf.buf[4] = msg.ctrl;
    panelSendBuf.buf[5] = 0x40;
    panelSendBuf.buf[6] = calcChecksum(panelSendBuf.buf, 6);
    panelSendBuf.len = msg.u.len;
    SendOSDP();
}

void sendOSDPNak(struct OSDP_MSG_HEADER msg){
    
    panelSendBuf.buf[0] = 0x53;
    panelSendBuf.buf[1] = msg.addr | 0x80;
    panelSendBuf.buf[2] = 8;
    panelSendBuf.buf[3] = 0;
    panelSendBuf.buf[4] = msg.ctrl;
    panelSendBuf.buf[5] = 0x41;
    panelSendBuf.buf[6] = 0x01;
    panelSendBuf.buf[7] = calcChecksum(panelSendBuf.buf, 7);
    panelSendBuf.len = 8;
    SendOSDP();
}
*/

void ReadOSDP()
{
     UART_ACS_LoadRxConfig();
    if(UART_ACS_GetRxBufferSize() == 0){
//        MyI2C_Regs.buffer1[7] = 0;        
        return;
    }
    MyI2C_Regs.buffer1[2]++;
    int byteCount =0;
    unsigned char tmp = 0;
    char msg_started = 0;
    int i;
    Timeout_Timer_Isr_Disable();
    osdp_timeout = 0;
    Timeout_Timer_Isr_Enable();
    if (UART_ACS_GetRxBufferSize()>= 7)
    {
        MyI2C_Regs.buffer1[3]++;
/*        for(i=0;i<7;i++) {
            tmp = UART_ACS_GetChar();
                MyI2C_Regs.buffer1[8] = tmp;
            if (tmp == 0x53) {
                msg_started = 1;
                break;
            }
        } 
*/
 /*           tmp = UART_ACS_GetRxBufferSize();
            for(i = 0; i<tmp; i++){
                MyI2C_Regs.buffer[i] = UART_ACS_GetChar();
            }
   */         
            tmp = UART_ACS_GetChar();
            
            while(tmp!=0x53){
                MyI2C_Regs.buffer1[8] = tmp;
                if(UART_ACS_GetRxBufferSize()==0) return;
                tmp = UART_ACS_GetChar();
                
            }
            MyI2C_Regs.buffer1[7] ++;
            

                msg_started = 1;

            while(UART_ACS_GetRxBufferSize()<=4){

                if(osdp_timeout > 10000) return;
            }
     
        if (msg_started){
            tmp = UART_ACS_GetChar();  //ADDR
                            
            osdp_msg_header.addr = tmp;
            //ik if(tmp != 0) return;
            
            tmp = UART_ACS_GetChar();
            osdp_msg_header.u.length[0] = tmp;
            
            tmp = UART_ACS_GetChar();
            osdp_msg_header.u.length[1] = tmp;
            
            tmp = UART_ACS_GetChar();
            osdp_msg_header.ctrl = tmp;
            osdp_msg_header.seq = tmp & 0x03;
            
//            tmp = UART_ACS_GetChar();
//            osdp_msg_header.cmnd = tmp;
          MyI2C_Regs.buffer1[5]++;  
        } 
        else
        {
            //the message is not started yet
            return;
        }
        
        if (osdp_msg_header.u.len <7) return; //discard bad message, too short.
        
        byteCount = 0;
        panelReadBuf.buf[byteCount] = 0x53;
        MyI2C_Regs.buffer[byteCount] = panelReadBuf.buf[byteCount];
        byteCount++;
        panelReadBuf.buf[byteCount] = osdp_msg_header.addr;
        MyI2C_Regs.buffer[byteCount] = panelReadBuf.buf[byteCount];
        byteCount++;
        panelReadBuf.buf[byteCount] = osdp_msg_header.u.length[0];
        MyI2C_Regs.buffer[byteCount] = panelReadBuf.buf[byteCount];
        byteCount++;
        panelReadBuf.buf[byteCount] = osdp_msg_header.u.length[1];
        MyI2C_Regs.buffer[byteCount] = panelReadBuf.buf[byteCount];
        byteCount++;
        panelReadBuf.buf[byteCount] = osdp_msg_header.ctrl;
        MyI2C_Regs.buffer[byteCount] = panelReadBuf.buf[byteCount];
        byteCount++;
//        panelReadBuf.buf[byteCount] = osdp_msg_header.cmnd;
//        byteCount++;
        if(osdp_msg_header.u.len > OSDP_PANEL_BUFFER_LENGTH){
            //error, the message is too long
            ClearOsdpBuffer();
            osdp_msg_header.u.len = 0;
            return;
        } else {
            panelReadBuf.len = osdp_msg_header.u.len;
        }
        
        for (; byteCount < osdp_msg_header.u.len;){  //may need timeout here
            if (UART_ACS_GetRxBufferSize()>0){
                tmp = UART_ACS_GetChar();
                panelReadBuf.buf[byteCount] = tmp;
        MyI2C_Regs.buffer[byteCount] = panelReadBuf.buf[byteCount];                
                if (byteCount == (osdp_msg_header.u.len - 1))
                {
                    //last byte
                    osdp_msg_header.checksum = tmp;
                    break;
                }
                byteCount++;
            }
            if(osdp_timeout > 20000) return;
        }
        //last byte
/*      
        if (osdp_msg_header.ctrl & 0x04 == 0){  //0==CKSUM, 4==CRC
            tmp = calcChecksum(panelReadBuf.buf,osdp_msg_header.u.len-1);
            if (tmp != osdp_msg_header.checksum)
            {
                sendOSDPNak(osdp_msg_header); //no delay, send it back in <3ms
                return; //error!
            }
        }
*/        
        copyPanelBufI2C(&panelReadBuf);         //send to firmware
        CyDelay(20);
        MyI2C_Regs.osdp_status |= OSDP_PANEL_DATA_IN_READY;
            MyI2C_Regs.buffer1[6] ++; 
    }
}

void SetPanelSpeed(char rate){
    switch(rate){
        case BAUD_9600:
            Clock_5_SetDividerValue(CLKDV_9600 + 1); //  48000000/625=9600
            break;
        case BAUD_19200:
            Clock_5_SetDividerValue(CLKDV_19200 + 1);
            break;
        case BAUD_38400:
            Clock_5_SetDividerValue(CLKDV_38400 + 1);
            break;
        case BAUD_115200:
            Clock_5_SetDividerValue(CLKDV_115200 + 1);
            break;
        case BAUD_230400:
            Clock_5_SetDividerValue(CLKDV_230400);
            break;
        case BAUD_460800:
            Clock_5_SetDividerValue(CLKDV_460800);
            break;
        case BAUD_921600:
            Clock_5_SetDividerValue(CLKDV_921600);
            break;
        default:
            Clock_5_SetDividerValue(CLKDV_9600); //  48000000/625=9600
            break;    
    }
    Clock_5_Enable();
}

////////////////OSDP READER//////////////////
ReaderBuf readerSendBuffer, readerReadBuffer;
unsigned char OSDPReaderTask;
unsigned char osdp_reader_msg_started;
unsigned char osdp_reader_msg_ready;
int ori;

void initOSDPReaderUART()
{
    isr_READER_rx_SetVector(OSDPReaderRxInt);
    isr_READER_rx_StartEx(OSDPReaderRxInt);
    SetReaderSpeed(BAUD_9600);
    UART_READER_Start();
    UART_READER_LoadRxConfig();  //set to receiving mode
    CR_DE_Write(0);
    Mode_Sel_CR_Write(1); //set the chip to RS485 mode
    osdp_reader_msg_ready = 0;
    osdp_reader_msg_started = 0;
    ori = 0;
}


CY_ISR(OSDPReaderRxInt) //interrupt on Rx byte received
{   
    unsigned char uch;
    //move all available characters from Rx queue to RxBuffer
    while(UART_READER_ReadRxStatus() & UART_READER_RX_STS_FIFO_NOTEMPTY)
	{
        uch = UART_READER_ReadRxData(); //get serial port
        if (ori < OSDP_READER_BUFFER_LENGTH && osdp_reader_msg_started == 1)
        {
            readerReadBuffer.buf[ori] = uch; //put into the buffer
            ori++;
        }
        else
        {
            ori = 0;
        }
        
        if (ori == 0 && uch == 0x53) {
            osdp_reader_msg_started = 1;
            readerReadBuffer.buf[ori] = 0x53; //put into the buffer
            ori = 1;
        }
        else if (osdp_reader_msg_started == 1)
        {
            if (ori == 2)
            {
                osdp_reader_msg_header.addr = uch;
            }
            else if (ori == 3)
            {
                osdp_reader_msg_header.u.length[0] = uch;
            }
            else if (ori == 4)
            {
                osdp_reader_msg_header.u.length[1] = uch;
                if (osdp_reader_msg_header.u.len <5 ||
                    osdp_reader_msg_header.u.len > OSDP_READER_BUFFER_LENGTH){
                    //error, the message is too long, reset buffer
                    osdp_reader_msg_header.u.len = 0;
                    ori = 0;
                    osdp_reader_msg_started = 0;
                }
                else
                {
                    readerReadBuffer.len = osdp_reader_msg_header.u.len;
                }
            }
            else if (ori > 4)
            {
                if (ori>=osdp_reader_msg_header.u.len)  //ori is +1 larger than len
                {
                    //buffer full
                    osdp_reader_msg_ready = 1;
                    osdp_reader_msg_started = 0;
                }
            }
        }
	}   
}


void copyReaderBufI2C(ReaderBuf *p)
{
    int i;
    MyI2C_Regs.osdp_reader_data_length = p->len;
    for (i=0; i < p->len; i++)
    {
        MyI2C_Regs.osdp_reader_buffer[i] = p->buf[i];
        CyDelayUs(2u);   //=2 do not reduce
    }
    MyI2C_Regs.osdp_status |= OSDP_READER_DATA_IN_READY;
//    MyI2C_Regs.osdp_reader_cmd = OSDP_READ;  //clear completed command
//    OSDPReaderTimer_Start();
}
//uint8 InterruptCnt;

//keep alive timer interrupt for OSDP reader dual authentication
CY_ISR(KeepAliveInterruptHandler)
{
	/* Read Status register in order to clear the sticky Terminal Count (TC) bit 
	 * in the status register.
	 */
   	OSDPReaderTimer_STATUS;
//    OSDPReaderTask |= (0x1 << OSDP_SEND_READER);  //send the command to the card reader
}

void ClearOsdpReaderBuffer()
{
    int i;
    UART_READER_ClearRxBuffer();
    MyI2C_Regs.osdp_reader_data_length = 0;
    MyI2C_Regs.osdp_status &= ~OSDP_READER_DATA_IN_READY;
    for ( i=0; i<OSDP_READER_BUFFER_LENGTH; i++)
    {
        MyI2C_Regs.osdp_reader_buffer[i]=0;
        CyDelayUs(1u);
    }
}

void SendOSDPReader()
{
    Mode_Sel_CR_Write(1); //set the SP335e chip to RS485
    CR_DE_Write(1);
    UART_READER_LoadTxConfig();
    MyI2C_Regs.osdp_reader_cmd = OSDP_READ;
    CyDelay(3); //Driver Output Enable Time=3,do not reduce. 
    if(readerSendBuffer.buf[0] == 0x53)
    {
        UART_READER_PutArray(readerSendBuffer.buf,readerSendBuffer.len); 
        CyDelay(5); //minmum 4ms
    }
    osdp_reader_msg_ready = 0;   //clear receiving buffer
    osdp_reader_msg_started = 0;
    ori = 0;
    UART_READER_LoadRxConfig();  //set to receiving mode
    CR_DE_Write(0);
    Mode_Sel_CR_Write(1); //set the chip to RS485 mode
}

void ReadOSDPReader()
{
    if (osdp_reader_msg_ready == 1)
    {
        if (run_mode == MODE_OSDP + BASE_PASS)
        {
            Pass_OSDP(&readerReadBuffer); //send to panel
        }
        else
        {
            copyReaderBufI2C(&readerReadBuffer);  //send to firmware
        }
        osdp_reader_msg_ready = 0;
    }
}


void SetReaderSpeed(char rate){
    switch(rate){
        case BAUD_9600:
            Clock_6_SetDividerValue(CLKDV_9600); //  48000000/625=9600
            break;
        case BAUD_19200:
            Clock_6_SetDividerValue(CLKDV_19200);
            break;
        case BAUD_38400:
            Clock_6_SetDividerValue(CLKDV_38400);
            break;
        case BAUD_115200:
            Clock_6_SetDividerValue(CLKDV_115200);
            break;
        case BAUD_230400:
            Clock_6_SetDividerValue(CLKDV_230400);
            break;
        case BAUD_460800:
            Clock_6_SetDividerValue(CLKDV_460800);
            break;
        case BAUD_921600:
            Clock_6_SetDividerValue(CLKDV_921600);
            break;
        default:;    
    }
    Clock_6_Enable();
}


void CopyI2cReaderBuffer(){
    int i;
    unsigned char msg_size;
    
    msg_size=  MyI2C_Regs.osdp_reader_data_length;
    
    //fill up a temp buffer
    if (msg_size <= OSDP_READER_BUFFER_LENGTH){
        for ( i=0; i<msg_size; i++){
            readerSendBuffer.buf[i]=MyI2C_Regs.osdp_reader_buffer[i];
        }
        for ( i=msg_size; i< OSDP_READER_BUFFER_LENGTH; i++){
            readerSendBuffer.buf[i] = 0;
        }
        readerSendBuffer.len = msg_size;
    }
    // Send Reader
    if ( readerSendBuffer.len > 0){
        LED_GRN_Write(ON);
        SendOSDPReader();
        LED_GRN_Write(OFF);
    }
}


/****************************************
   Method to handle OSDP commands
   coming from device

*****************************************/
void HandleOSDP()
{
    int temp = 0;
//            if (MyI2C_Regs.osdp_cmd == OSDP_RATE) SetPanelSpeed(MyI2C_Regs.osdp_rate);
            temp = Clock_5_GetDividerRegister();
            MyI2C_Regs.buffer1[14] = temp & 0xff;  
            MyI2C_Regs.buffer1[13] = (temp >> 8) & 0xff;  

    //Now, Handle the Reader Messages
    if (MyI2C_Regs.osdp_reader_cmd != 0){
    	switch (MyI2C_Regs.osdp_reader_cmd)
        {
            case OSDP_NONE:
            case OSDP_BUSY:
                break;
            case OSDP_SEND:
                CopyI2cReaderBuffer();
                break;
            case OSDP_READ:
                break;
            case OSDP_RATE:
                SetReaderSpeed(MyI2C_Regs.osdp_reader_rate);
                MyI2C_Regs.osdp_reader_cmd = OSDP_READ;
                break;
            default:;
        }
        if ((MyI2C_Regs.osdp_status & OSDP_READER_DATA_IN_READY) == 0 
            && MyI2C_Regs.osdp_reader_cmd != OSDP_SEND){
            ReadOSDPReader();
        }
    }
   
    //Panel Messages
    if (MyI2C_Regs.osdp_cmd){
    	switch (MyI2C_Regs.osdp_cmd)
        {
            case OSDP_NONE:
            case OSDP_BUSY:
                break;
            case OSDP_SEND:
                MyI2C_Regs.buffer1[11] ++;
                CopyI2cPanelBuffer();  //fill up buffer for send to panel
                break;
            case OSDP_READ:
                break;
            case OSDP_RATE:
                SetPanelSpeed(MyI2C_Regs.osdp_rate);
                MyI2C_Regs.osdp_cmd = OSDP_READ;
                break;
            default:
                break;
        }
           if ((MyI2C_Regs.osdp_status & OSDP_PANEL_DATA_IN_READY) == 0 
            && MyI2C_Regs.osdp_cmd != OSDP_SEND){
                ReadOSDP();
            } 

    }
    return;
    // check if anything comes in

        
//    if ((MyI2C_Regs.osdp_status & OSDP_PANEL_DATA_IN_READY) == 0 
//        && 
        while(1)
        {
            if(MyI2C_Regs.osdp_cmd != OSDP_SEND)
            {
                ReadOSDP();
                 void HandleUSB();
                    HandleUSB();
            }
            else CopyI2cPanelBuffer();
            if(MyI2C_Regs.cmd == CMD_BOOTLOAD) break;
        }
}

void Pass_OSDP(ReaderBuf *p)
{
    int i;
    panelSendBuf.len = p->len;
    if (panelSendBuf.len >  OSDP_PANEL_BUFFER_LENGTH) panelSendBuf.len = OSDP_PANEL_BUFFER_LENGTH;
    for (i=0; i < panelSendBuf.len; i++){
        panelSendBuf.buf[i] = p->buf[i];
    }
    panelSendBuf.pending = 1;  //ready to send
    SendOSDP();
    panelSendBuf.pending = 0; //send out command
}

/* [] END OF FILE */
