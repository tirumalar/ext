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
#include "system.h"
#include "commandhandler.h"
#include "osdp.h"
#include "f2f.h"
#include "wiegand.h"
#include "i2cuart.h"
#include "toc.h"
#include "hid.h"
I2C_Regs MyI2C_Regs;
unsigned char run_mode;
int passmode_timeout;
int debug_msg_delay;

uint8 poe_en_stat=-1;

void USBUART_PutDataBig(unsigned char *p,int len)
#define min(a,b) ((a>b)?b:a)
{
    int to_send;
    while (len)
    {
        to_send=min(len,64);
        while(USBUART_CDCIsReady() == 0u);    /* Wait till component is ready to send more data to the PC */ 

        USBUART_PutData(p,to_send);
        p+=to_send;
        len-=to_send;
    }
}

void SendHID()
{
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



void   POE_OIM_En(uint8 en)
{
if (poe_en_stat ==en)
   return;
POE_OIM_SDn_Write(0);
POE_RST_Write(0);
CyDelay(1000);
POE_RST_Write(en);
POE_OIM_SDn_Write(en);
poe_en_stat=en;
}
#define RTC_ADDRESS (0XA2>>1)
void rtc_write_reg(uint8 reg, uint8 val)
{
   
     I2C_Master_MasterSendStart(RTC_ADDRESS,0);
     I2C_Master_MasterWriteByte(reg);
     I2C_Master_MasterWriteByte(val);
     I2C_Master_MasterSendStop();
}
void rtc_write(uint8 addr,unsigned char *buff, uint8 len)
{
    
    I2C_Master_MasterSendStart(RTC_ADDRESS,0);
    I2C_Master_MasterWriteByte(addr);
    while(len--)
     I2C_Master_MasterWriteByte(*buff++);
     I2C_Master_MasterSendStop();
}
void rtc_read(int addr, unsigned char *buff, int len)
{
    int x;
    
    I2C_Master_MasterSendStart(RTC_ADDRESS,0);
    I2C_Master_MasterWriteByte(addr);
    I2C_Master_MasterSendRestart(RTC_ADDRESS,1);
    
  //  I2C_Master_MasterSendStart(RTC_ADDRESS,1);
    for (x=0; x< len-1;x++)
       *buff++=I2C_Master_MasterReadByte(1);
    *buff++=I2C_Master_MasterReadByte(0);
    I2C_Master_MasterSendStop();
}

#define MAX_USB_COMMAND 255 +4
unsigned char usb_command_buffer[MAX_USB_COMMAND];
int usb_buf_len=0;
int tick_count;
int last_rx;
#define CMD_USB_WRITE 57
#define CMD_USB_READ  56
#define CMD_USB_BOOT_LOAD 58


void HandleUSB()

{
    int bcount;
        if (0u != USBUART_IsConfigurationChanged())
        {
            /* Initialize IN endpoints when device is configured. */
            if (0u != USBUART_GetConfiguration())
            {
                /* Enumeration is done, enable OUT endpoint to receive data 
                 * from host. */
                USBUART_CDC_Init();
            }
        }
    bcount=USBUART_GetCount();
    if (bcount)
        {
            int len;
            int data_len;
            
            if ((tick_count-last_rx)>100)
               usb_buf_len=0;
            last_rx = tick_count;
            USBUART_GetAll(&usb_command_buffer[usb_buf_len]);
            usb_buf_len+=bcount;
            data_len = (usb_command_buffer[3]);// | (usb_command_buffer[4]&0xff);
            if (usb_buf_len>3)
                {
                    int reg =  (usb_command_buffer[1]<<8) | (usb_command_buffer[2]&0xff);
                    if (usb_command_buffer[0]==CMD_USB_BOOT_LOAD)
                       {
                        if ((usb_command_buffer[1]==0x55) && (usb_command_buffer[2]=0xaa))
                            {
                             EEPROM_EraseSector(0);
                             EEPROM_WriteByte(0x55,0);
                             CySoftwareReset();

                            }
                            usb_buf_len=0;
                        }
                    if (usb_command_buffer[0]==CMD_USB_READ)
                        {
                        unsigned char *p = (unsigned char *)&MyI2C_Regs ;
                        memcpy(&usb_command_buffer[4],p+reg,data_len);
                        USBUART_PutDataBig(usb_command_buffer,4+data_len );       /* Send data back to PC */
                        usb_buf_len=0;
                        }
                    if ((usb_command_buffer[0]==CMD_USB_WRITE)  && ((usb_buf_len+4)>=data_len))
                        {
                            unsigned char *p = (unsigned char *)&MyI2C_Regs ;
                            memcpy(p+reg,&usb_command_buffer[4],data_len);
                            while(USBUART_CDCIsReady() == 0u);    /* Wait till component is ready to send more data to the PC */ 
                            USBUART_PutData(usb_command_buffer, 4);       /* Send data back to PC */
                            usb_buf_len=0;
                        }
                        
                }
        }
    
}


unsigned char f2f_alive_message[] = {0x0,0x34,0xE3,0xFD,0xA0,0x0,0x0,0x0,0x0};
extern char TOC_port_init;
void commandSendF2F(int len){
    LED_YEL_Write(OFF);
	SendF2F(MyI2C_Regs.buffer, len);  //len is bytes
    MyI2C_Regs.cmd = CMD_NONE;
    LED_YEL_Write(ON);
}
void commandSendWiegand(){
    LED_YEL_Write(ON);
	SendWeigand(MyI2C_Regs.buffer, MyI2C_Regs.bits);
    MyI2C_Regs.cmd = CMD_NONE;
    LED_YEL_Write(OFF);
}

void commandSendTOC(int len){
    card_ack = 1; //stop resending function
    MyI2C_Regs.status_out = 0; //clear old status.
    if (TOC_port_init == 0){
        initTocReaderUART();
        TOC_port_init = 1;
    }
                        //initTocReaderUART();
    SendWaveLynxReader(MyI2C_Regs.buffer,len);
    rxi = 0;
    tlv_ready=0;
    MyI2C_Regs.cmd = CMD_READ;                
}

void commandSendPAC(){

    LED_YEL_Write(OFF);
    SetupUART(PAC);
    SendPAC(MyI2C_Regs.buffer);
    MyI2C_Regs.cmd = CMD_NONE;
    LED_YEL_Write(ON);
}
void commandReadTOC(){
    if (card_in_buf == 0 && card_ack == 1){
        card_ack = 0;
        resetImgBuf();
    }
    if (TOC_port_init == 0){
        initTocReaderUART();
        TOC_port_init = 1;
    }
        //initTocReaderUART();
    ReadImageCard();
  
}


void HandleCommand()
{
    char cmd;
    int len;
    int i;
    cmd = MyI2C_Regs.cmd;
    run_mode = MyI2C_Regs.mode;
    len = (MyI2C_Regs.data_length[0] << 8) + MyI2C_Regs.data_length[1];
    
    
    if(cmd == CMD_SEND){
        if (run_mode == 0) run_mode = MODE_WIEGAND; //garbage in
        if (MyI2C_Regs.bits == 0) MyI2C_Regs.bits = 0x1A; //garbage in, right out
    }
    if((run_mode == MODE_WIEGAND) || (run_mode == MODE_WIEGAND + BASE_PASS) || (run_mode == MODE_WIEGAND + PIN_PASS)){
        isr_WD_SYNC_Enable();
    } else {
        isr_WD_SYNC_Disable();
    }
        if((run_mode == MODE_F2F) || (run_mode == MODE_F2F + BASE_PASS)){
        isr_F2F_edge_Enable();
    } else {
        isr_F2F_edge_Disable();
    }

    
    switch (run_mode)
    {
        case MODE_OSDP:
        case MODE_OSDP + BASE_PASS:
            if(0==isr_keepalive_GetState())
            {

                initOSDP();
            }
                MyI2C_Regs.buffer1[12] ++;
                HandleOSDP();
            return;
        case MODE_OSDP + BASE_TOC:
            if(cmd == CMD_SEND_TOC_READER){
                commandSendTOC(len);
            }
            if(cmd == CMD_READ){
                commandReadTOC();
            }
            if(0==isr_keepalive_GetState())
            {
                initOSDP();
            }
            HandleOSDP();
            return;
        case MODE_F2F:
            if (f2f_send_alive)
            {
                SendF2F(f2f_alive_message,5);
                Tamper_Timer_ISR_Disable();
                f2f_send_alive = 0;
                f2f_alive_delay_cnt = 0;
                Tamper_Timer_ISR_Enable();
            }
		    if(cmd == CMD_READ){
                LED_GRN_Write(OFF);
                while(ReadF2F(MyI2C_Regs.bits,0) == 0)
                {
                    if( MyI2C_Regs.cmd != CMD_READ ) 
                         break;
                    HandleUSB();
                }
                LED_GRN_Write(ON);
		    }	
		    if(cmd == CMD_SEND){
                commandSendF2F(len);
		    }
            break;
        case MODE_F2F + BASE_PASS:
            if (f2f_send_alive)
            {
                SendF2F(f2f_alive_message,5);
                Tamper_Timer_ISR_Disable();
                f2f_send_alive = 0;
                f2f_alive_delay_cnt = 0;
                Tamper_Timer_ISR_Enable();
            }
		    if(cmd == CMD_NONE){
                LED_GRN_Write(OFF);
                for (i=0; i<10; i++){ //wait a while
                    ReadF2F(MyI2C_Regs.bits,1);
                }
                LED_GRN_Write(ON);
		    }
		    if(cmd == CMD_SEND){
                commandSendF2F(len);
    		}
            break;
	    case MODE_F2F + BASE_TOC:
		    if(cmd == CMD_SEND){
                commandSendF2F(len);
    		}
            if(cmd == CMD_READ){
                commandReadTOC();
            }
            if(cmd == CMD_SEND_TOC_READER){
                commandSendTOC(len);
            }
  
		    break;
        case MODE_WIEGAND:
            if(cmd == CMD_READ){
                LED_GRN_Write(OFF);
                while(ReadWeigand(MyI2C_Regs.bits,0) == 0)
                {
                    if(MyI2C_Regs.cmd !=CMD_READ ) 
                      break;
                    HandleUSB();
                };
                LED_GRN_Write(ON);
            }
            if(cmd == CMD_SEND){
                commandSendWiegand();
            }
            break;
        case MODE_WIEGAND + BASE_TOC:
            if(cmd == CMD_SEND_TOC_READER){
                commandSendTOC(len);
            }
            if(cmd == CMD_READ){
                commandReadTOC();
            }
            if(cmd == CMD_SEND){
                commandSendWiegand();
            }
            
            break;
        case MODE_WIEGAND + BASE_PASS:
            if(cmd == CMD_NONE){// || (cmd == CMD_READ)){
                LED_GRN_Write(OFF);
              
                if (ReadWeigand(MyI2C_Regs.bits,1) == 1) {
                    LED_GRN_Write(ON);
                }
            }
            if(cmd == CMD_SEND){
                commandSendWiegand();
            }
            break;
        case MODE_WIEGAND + PIN_PASS:
            if(cmd == CMD_READ){
                LED_GRN_Write(OFF);
                        //ClearBuffer(); //seems better than without
                if (ReadWeigand(MyI2C_Regs.bits,0) == 1) {
                    LED_GRN_Write(ON);
                }
            }
            if(cmd == CMD_NONE){
                if (ReadWeigand(MyI2C_Regs.bits,1) == 1) {
//                   LED_GRN_Write(ON);
                }
            }
            if(cmd == CMD_SEND){
                commandSendWiegand();
            }
            break;
            
        case MODE_PAC:
            if(cmd == CMD_READ){
                LED_GRN_Write(OFF);
                ClearBuffer(); // added here - GRI 2/2/15
                SetupUART(PAC);
                while(ReadPAC(0)== 0)
                {
                    if(cmd != CMD_READ )
                            break;
                HandleUSB();
                }
                LED_GRN_Write(ON);
                OptSelect_Write(1);
            }
            if(cmd == CMD_SEND){
                commandSendPAC();
            }
            break;
        case MODE_PAC + BASE_TOC:
            if(cmd == CMD_SEND){
                commandSendPAC();
            }
            if(cmd == CMD_READ){
                commandReadTOC();
            }
            if(cmd == CMD_SEND_TOC_READER){
                commandSendTOC(len);
            }
            break;
        case MODE_PAC + BASE_PASS:
            if(cmd == CMD_NONE){
                LED_GRN_Write(OFF);
                SetupUART(PAC);
                ReadPAC(1);
                LED_GRN_Write(ON);
                OptSelect_Write(1);
            }
            if(cmd == CMD_SEND){
                commandSendPAC();
            }
            break;
        case MODE_HID:
        case MODE_HID + BASE_TOC:
            if(cmd == CMD_READ){
                commandReadTOC();
            }
            if(cmd == CMD_SEND_TOC_READER){
                commandSendTOC(len);
            }
        case MODE_HID + BASE_PASS:
            if(cmd == CMD_SEND){
                LED_YEL_Write(OFF);
                OptSelect_Write(1);
        //               SendHID_SP533();
                SendHID();
                MyI2C_Regs.cmd = CMD_NONE;
                LED_YEL_Write(ON);
            }
            break;
        default:
            break;
    }             
    
    
   switch (cmd)
    {
        case CMD_POE_ENABLE:
        {
            POE_OIM_En(MyI2C_Regs.buffer[0]);
                MyI2C_Regs.status_out=1;
                MyI2C_Regs.cmd=0;
        }
        break;
        case CMD_RGB:
            {
                uint8_t v;
                uint8_t *p;
                v= MyI2C_Regs.buffer[0];
                
                ST_LED_0_Write(v&0x1?0:1);
                ST_LED_1_Write(v&0x2?0:1);
                ST_LED_2_Write(v&0x4?0:1);
                MyI2C_Regs.status_out=1;
                MyI2C_Regs.cmd=0;
            }
            break;
//#define CMD_POE_ENABLE      8
        case CMD_RTC_READ:
            {
               //ST_LED_1_Write(0);
 
                rtc_read(0, MyI2C_Regs.buffer, 8);
                MyI2C_Regs.status_out=1;
                MyI2C_Regs.cmd=0;
               //ST_LED_1_Write(1);
                 break;
            }
        case CMD_RTC_WRITE:
            {
                rtc_write(0, MyI2C_Regs.buffer, 8);
                //MyI2C_Regs.status_out=1;
                MyI2C_Regs.cmd=0;
                break;
            }
    }
    if(cmd == CMD_BOOTLOAD){
        MyI2C_Regs.buffer[0] ++; 
        EZI2Cs_Stop();
//        BootloadableI2C_Load();
    }
    
}


/* [] END OF FILE */
