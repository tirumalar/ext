#if 0
#include "system.h"
#include "osdp.h"
I2C_Regs MyI2C_Regs;
unsigned char run_mode;
int passmode_timeout;
extern uint8 tlv_ready;
void SU_Isr(SOFT_UART_STRUCT *s);
void ReadImageCard(void);

extern unsigned char f2f_alive_message[];
uint8 poe_en_stat=-1;
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



/**************************************
  Debug Method: To Set I2C data buffer
***************************************/
void SetBuffer()
{
    MyI2C_Regs.buffer[0] = 0x02;
    MyI2C_Regs.buffer[1] = 0x32;
    MyI2C_Regs.buffer[2] = 0x30;
    MyI2C_Regs.buffer[3] = 0x42;
    MyI2C_Regs.buffer[4] = 0x35;
    MyI2C_Regs.buffer[5] = 0x32;
    MyI2C_Regs.buffer[6] = 0x36;
    MyI2C_Regs.buffer[7] = 0x42;
    MyI2C_Regs.buffer[8] = 0x35;
    MyI2C_Regs.buffer[9] = 0x42;
    MyI2C_Regs.buffer[10] = 0x31;
    MyI2C_Regs.buffer[11] = 0x77;
}
#if 0
void ClearBuffer()
{
    int i;
    for(i=0; i < BUFFER_LENGTH; i++)
    {
        MyI2C_Regs.buffer[i] = 0;
        CyDelayUs(1u); //need little delay for Release
    }
}
#endif
/***********************************************
            Weigand -HID Protocol
  Procedure to send weigand data in serial format
  Baud Rate: 57600 Databits: 8 
  Stop Bits: 2 Parity: Even 
  Bits to transmit : 18 bytes
  Zero paddings to the left before the weigand data 

************************************************/

void SendHID_SP533()
{
    int len;
    len = (MyI2C_Regs.data_length[0] << 8) + MyI2C_Regs.data_length[1];
    Clock_5_SetDividerValue(CLKDV_9600); //set the baud rate to default 9600
    UART_ACS_Init();
    CyDelay(20);
    Mode_Sel_ACS_Write(0); //set io chip to RS232
    TX_EN_Write(1);
    UART_ACS_LoadTxConfig();
    CyDelay(1); //Driver Output Enable Time, do not reduce. 
    UART_ACS_PutArray(MyI2C_Regs.buffer,len/8); 
    CyDelay(5); //minmum 4m
    UART_ACS_LoadRxConfig();
    TX_EN_Write(0);
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


void InitOSDP(){
    
    //initialize the OSDP
    Clock_5_Enable();
   // Clock_5_SetDividerValue(CLKDV_9600); //set the baud rate to default 9600
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

 /***********************************************
     Debug Code : Software UART for PAC Protocol
  ************************************************
 void SendPACTest()
{
      int i,j;
    LED_GRN_Write(ON);
    LED_YEL_Write(OFF);
  
    for( i = 0; i < 20; i++)
    {
        for( j= 0; j < 12 ; j++)
        {
            char c = MyI2C_Regs.buffer[j];
         SU_PutChar(&s,c);
        }
    }

    LED_GRN_Write(OFF);
    LED_YEL_Write(OFF);
}

****************************************************/

void displayError(int n){
int i;
    for (i = 0; i< n; i++) 
    {
        LED_RED_Write(OFF); //display error
        CyDelay(300);
        LED_RED_Write(ON);
        CyDelay(300);
    } 
}


//unsigned char f2f_alive_message[] = {0x0,0x34,0xE3,0xFD,0xA0,0x0,0x0,0x0,0x0};
char TOC_port_init;
char sendDebugMsg = 0;  //yqh debug mode
int debug_msg_delay;
/****************************************
   Method to handle commands
   coming from device

*****************************************/
#if 1
void HandleCommand1()
{
    char cmd;
    int len;
    int i;
/*    
    if (sendDebugMsg) //debug mode
    {
        MyI2C_Regs.mode = BASE_TOC + MODE_WIEGAND;
        MyI2C_Regs.cmd = CMD_SEND_TOC_READER;
//        MyI2C_Regs.buffer[0] = CMD_PING;
//        MyI2C_Regs.buffer[0] = CMD_CLEAR_EM_DATA;
//        MyI2C_Regs.buffer[1] = 0;
//        MyI2C_Regs.buffer[2] = 0;
        //B1 01 4E
//        MyI2C_Regs.buffer[0]=0xB1;
//        MyI2C_Regs.buffer[1]=0x01;
//        MyI2C_Regs.buffer[2]=0x4E;
//
        MyI2C_Regs.buffer[0]=0x6E;
        MyI2C_Regs.buffer[1]=22;
        MyI2C_Regs.buffer[2]=0;
        MyI2C_Regs.buffer[3]=1;
        MyI2C_Regs.buffer[4]=2;
        MyI2C_Regs.buffer[5]=3;
        MyI2C_Regs.buffer[6]=4;
        MyI2C_Regs.buffer[7]=5;
        MyI2C_Regs.buffer[8]=6;
        MyI2C_Regs.buffer[9]=7;
        MyI2C_Regs.buffer[10]=8;
        MyI2C_Regs.buffer[11]=9;
        MyI2C_Regs.buffer[12]=0x6E;
        MyI2C_Regs.buffer[13]=0x0b;
        MyI2C_Regs.buffer[14]=0;
        MyI2C_Regs.buffer[15]=1;
        MyI2C_Regs.buffer[16]=2;
        MyI2C_Regs.buffer[17]=3;
        MyI2C_Regs.buffer[18]=4;
        MyI2C_Regs.buffer[19]=5;
        MyI2C_Regs.buffer[20]=6;
        MyI2C_Regs.buffer[21]=7;
        MyI2C_Regs.buffer[22]=8;
        MyI2C_Regs.buffer[23]=9;
//        
        MyI2C_Regs.data_length[0] = 0;
//        MyI2C_Regs.data_length[1] = 2;
//        MyI2C_Regs.data_length[1] = 3;
        MyI2C_Regs.data_length[1] = 24;

        sendDebugMsg = 0;
    }
 */   
    cmd = MyI2C_Regs.cmd;
    run_mode = MyI2C_Regs.mode;
    len = (MyI2C_Regs.data_length[0] << 8) + MyI2C_Regs.data_length[1];
    

    switch (run_mode)
    {
        case MODE_OSDP:
        case MODE_OSDP + BASE_PASS:
        case MODE_OSDP + BASE_TOC:
            if(0==isr_keepalive_GetState())
            {
                InitOSDP();
            }
            HandleOSDP();
            return;
        case MODE_F2F:
        case MODE_F2F + BASE_PASS:
            if (f2f_send_alive)
            {
                SendF2F(f2f_alive_message,5);
                Tamper_Timer_ISR_Disable();
                f2f_send_alive = 0;
                f2f_alive_delay_cnt = 0;
                Tamper_Timer_ISR_Enable();
            }
            break;
        default:;
    }                        
	switch (cmd)
    {
        case CMD_POE_ENABLE:
        {
            POE_OIM_En(MyI2C_Regs.buffer[0]);
        }
        break;
        case CMD_RGB:
            {
                ST_LED_0_Write(MyI2C_Regs.buffer[0]&0x1?0:1);
                ST_LED_1_Write(MyI2C_Regs.buffer[0]&0x2?0:1);
                ST_LED_2_Write(MyI2C_Regs.buffer[0]&0x4?0:1);
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
        case CMD_NONE: //caution:Fang use CMD_NONE as the pass through read
                // process mode
                switch (run_mode)
                {
                    case MODE_OSDP:
                    case MODE_OSDP + BASE_PASS:
                    case MODE_OSDP + BASE_TOC:
                        if(0==isr_keepalive_GetState())
                        {
                            InitOSDP();
                        }
                        HandleOSDP();
                        return;
                    case MODE_WIEGAND + BASE_PASS: //pass through
                        LED_GRN_Write(OFF);
                        for (i=0; i<10; i++){ //wait a while
                            ReadWeigand(MyI2C_Regs.bits,1);
                        }
                        LED_GRN_Write(ON);
                        break;
                    case MODE_PAC + BASE_PASS:
                        LED_GRN_Write(OFF);
                        SetupUART(PAC);
                        ReadPAC(1);
                        LED_GRN_Write(ON);
                        OptSelect_Write(1);
                        break;
                    case MODE_F2F + BASE_PASS:
                        LED_GRN_Write(OFF);
                        for (i=0; i<10; i++){ //wait a while
                            ReadF2F(MyI2C_Regs.bits,1);
                        }
                        LED_GRN_Write(ON);
                        break;
                    default:;
                }                    
                break;
        case CMD_BOOTLOAD:
//                BootloadableI2C_Load();  //yqh debug mode
            EEPROM_EraseSector(0);
            EEPROM_WriteByte(0x55,0);
            CySoftwareReset();
                break;
        case CMD_READ:
                switch ( run_mode )
                {
                    case MODE_WIEGAND:
                        LED_GRN_Write(OFF);
                        //ClearBuffer(); //seems better than without
                        while(ReadWeigand(MyI2C_Regs.bits,0) == 0)
                        {
                             HandleUSB();

                            if(MyI2C_Regs.cmd !=CMD_READ ) 
                                break;
                        };
                        LED_GRN_Write(ON);
                        break;
                    case MODE_PAC:
                        LED_GRN_Write(OFF);
                        ClearBuffer(); // added here - GRI 2/2/15
                        SetupUART(PAC);
                        while(ReadPAC(0)== 0)
                        {
                                   HandleUSB();
                            if(cmd != CMD_READ )
                            break;
                        }
                        LED_GRN_Write(ON);
                        OptSelect_Write(1);
                        break;
                    case MODE_F2F:
                        LED_GRN_Write(OFF);
                        while(ReadF2F(MyI2C_Regs.bits,0) == 0)
                        {
                                   HandleUSB();
                            if( MyI2C_Regs.cmd != CMD_READ ) 
                                break;
                        };
                        LED_GRN_Write(ON);
                        break;
                    case MODE_OSDP:
                        break;
                    case BASE_TOC + MODE_WIEGAND:
                    case BASE_TOC + MODE_PAC:
                    case BASE_TOC + MODE_F2F:
                    case BASE_TOC + MODE_HID:
                    case BASE_TOC + MODE_OSDP:
                        if (card_in_buf == 0 && card_ack == 1)
                        {
                            card_ack = 0;
                            resetImgBuf();
                        }
                        if (TOC_port_init == 0)
                        {
                            initTocReaderUART();
                            TOC_port_init = 1;
                        }
                        //initTocReaderUART();
                        ReadImageCard();
                        break;
                    default:
                        break;
                }
                break;
	    case CMD_SEND:
                if (run_mode == 0) run_mode = MODE_WIEGAND; //garbage in
                if (MyI2C_Regs.bits == 0) MyI2C_Regs.bits = 0x1A; //garbage in, right out
                switch( run_mode )
                {
                    case MODE_PAC:
                    case MODE_PAC + BASE_TOC:
                    case MODE_PAC + BASE_PASS:
                        LED_YEL_Write(OFF);
                        SetupUART(PAC);
                        SendPAC(MyI2C_Regs.buffer);
                        MyI2C_Regs.cmd = CMD_NONE;
                        LED_YEL_Write(ON);
        				break;
                        
                    case MODE_F2F:
                    case MODE_F2F + BASE_TOC:
                    case MODE_F2F + BASE_PASS:    
                        LED_YEL_Write(OFF);
        				SendF2F(MyI2C_Regs.buffer, len);  //len is bytes
                        MyI2C_Regs.cmd = CMD_NONE;
                        LED_YEL_Write(ON);
        			    break;        
                        
                    case MODE_HID:
                    case MODE_HID + BASE_TOC:
                    case MODE_HID + BASE_PASS:
                       LED_YEL_Write(OFF);
                       OptSelect_Write(1);
        //               SendHID_SP533();
                       SendHID();
                       MyI2C_Regs.cmd = CMD_NONE;
                       LED_YEL_Write(ON);
        			   break;
                    
                    case MODE_WIEGAND:
                    case MODE_WIEGAND + BASE_TOC:
                    case MODE_WIEGAND + BASE_PASS:
                        LED_YEL_Write(ON);
            			SendWeigand(MyI2C_Regs.buffer, MyI2C_Regs.bits);
                        MyI2C_Regs.cmd = CMD_NONE;
                        LED_YEL_Write(OFF);
        				break;  
                    default:
                        //displayError(100);
                        break;
                }
                break;
        case CMD_SEND_TOC_READER:
                switch ( run_mode )
                {
                    case BASE_TOC + MODE_WIEGAND:
                    case BASE_TOC + MODE_PAC:
                    case BASE_TOC + MODE_F2F:
                    case BASE_TOC + MODE_HID:
                    case BASE_TOC + MODE_OSDP:
                        card_ack = 1; //stop resending function
                        MyI2C_Regs.status_out = 0; //clear old status.
                        if (TOC_port_init == 0)
                        {
                            initTocReaderUART();
                            TOC_port_init = 1;
                        }
                        //initTocReaderUART();
                        SendWaveLynxReader(MyI2C_Regs.buffer,len);
                        rxi = 0;
                        tlv_ready=0;
                        MyI2C_Regs.cmd = CMD_READ;
                        break;
                    default:
                        break;
                }                
                break;
        default:;   //cmd
	}
}

#endif

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
/*
void ReadImageCard()
{
    int k;
    if(tlv_ready == 1)
    {
        tlv_ready = 0;
        if (rxi == 0) return; //wrong size
        if (rxi <= IMAGE_TEMP_SIZE )
        {
            for (k=0; k<rxi; k++)
            {
                MyI2C_Regs.buffer[k] = ReaderRxBuffer[k];
                CyDelayUs(1);   //needed for Release Codes
            }
            rxi = 0;  //ready for next image
            MyI2C_Regs.data_length[0]= k>>8;   //higher 8 bits
            MyI2C_Regs.data_length[1]= k;        //lower 8 bits.
            MyI2C_Regs.dummy0[0]= (k>>8) & 0xFF;   //higher 8 bits
            MyI2C_Regs.dummy0[1]= k & 0xFF;        //lower 8 bits.
            card_ack = 0;
            card_in_buf = 1;
            MyI2C_Regs.status_out |= STAT_CHANGE | STAT_CARD_IN;
            MyI2C_Regs.cmd = CMD_NONE; //prevent to read it again
        }
        else
        {
            rxi = 0; //data is too big, read again
        }
    } //if
}
*/

#endif
