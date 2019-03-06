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
#include "project.h"
#include "motor.h"
#include "settings.h"
#include "math.h"
#define IC_ADDR   0x60
#define IC_ADDR1B 0x60|4
#define IC_ADDR2 0x61
#define IC_ADDR2B 0x61 |4 
#define EN_LED0 1
#define EN_LED1 1

//#define TEST_MODE

void LedRegWrite(uint8 addr, uint8 reg, uint8 val)
{
    uint8 r;
    r=I2C_LED_I2CMasterSendStart(addr,0);
    if(r!=0)
    {
         I2C_LED_I2CMasterSendStop();
        return;
}
    I2C_LED_I2CMasterWriteByte(reg);
    I2C_LED_I2CMasterWriteByte(val);
    I2C_LED_I2CMasterSendStop();
    
}

void LedInitRegs(uint8 addr)
{
    // set all leds to be pwm 
    // and write to MODE 1
       LedRegWrite(addr,0x14,0xaa);
       LedRegWrite(addr,0x15,0xaa);
       LedRegWrite(addr,0x16,0xaa);
       LedRegWrite(addr,0x17,0xaa);
       LedRegWrite(addr, 0,0x0);
 }
void LedSetRGB(uint8 addr,uint8 chan, uint8 r, uint8 g , uint8 b)
{
    //if (addr==IC_ADDR)
    //  return;
    LedRegWrite(addr,2+chan*3+2,r);
    LedRegWrite(addr,2+chan*3+1,g);
    LedRegWrite(addr,2+chan*3,b);
   // CyDelay(100);
}

volatile SETTINGS set;

#define ACTION_LED 1
#define ACTION_MOT_HOME 2
#define ACTION_MOT_ABS 3
#define ACTION_MOT_REL 4
#define ACTION_AUDIO_SET 5
#define ACTION_BOOTLOAD      6

void ShowMotError()
{
      LedSetRGB(IC_ADDR,0,200,200,200);

}
void DoLedUpdate(void)
{
    int x;
    
    for(x=0;x<5;x++)
        {
        if (set.chA&(1<<x)) 
                LedSetRGB(IC_ADDR,x,set.r,set.g,set.b);
        if (set.chB&(1<<x)) 
                LedSetRGB(IC_ADDR2,4-x,set.r,set.g,set.b);
        if (set.chC&(1<<x))
                LedSetRGB(IC_ADDR1B,x,set.r,set.g,set.b);
        if (set.chD&(1<<x)) 
                LedSetRGB(IC_ADDR2B,x,set.r,set.g,set.b);
        }
 }

int tof_init();
int tof_measure(void);



uint8_t MAG_ADDR =0x1e;

void I2C_WaitComplete( int stat)
{
    while(1)
    {     
        if ((I2C_MAG_Process() & stat))
           break;           
         CyDelayUs(5);
    }

}
void LISwriteReg(uint8 offset, uint8 dataToWrite)
{

    int status;
    uint8 s_data[2];
    s_data[0] = offset;
    s_data[1]=dataToWrite;
    
    I2C_MAG_WriteBuf(MAG_ADDR, s_data, 2, I2C_MAG_MODE_COMPLETE_XFER);
    I2C_WaitComplete(I2C_MAG_MSTAT_WR_CMPLT);
     //I2C_MAG_WriteBuf(MAG_ADDR, &dataToWrite, 1, I2C_MAG_MODE_COMPLETE_XFER);
     //I2C_WaitComplete(I2C_MAG_MSTAT_WR_CMPLT);
  
}

// Reads a mag register
uint8 LISreadReg( uint8 reg)
{

   unsigned char rxData[2];
   rxData[0]=0;
   I2C_MAG_WriteBuf(MAG_ADDR, &reg, 1, I2C_MAG_MODE_COMPLETE_XFER);
   I2C_WaitComplete(I2C_MAG_MSTAT_WR_CMPLT);
   I2C_MAG_ReadBuf(MAG_ADDR, rxData, 1, I2C_MAG_MODE_COMPLETE_XFER);
   I2C_WaitComplete(I2C_MAG_MSTAT_RD_CMPLT);
   return rxData[0];

}

uint8 LISreadBuff( uint8 reg, uint8 * buff, int len)
{

   unsigned char rxData[2];
   rxData[0]=0;
    reg|= 0x80; // auto inc
   I2C_MAG_WriteBuf(MAG_ADDR, &reg, 1, I2C_MAG_MODE_COMPLETE_XFER);
   I2C_WaitComplete(I2C_MAG_MSTAT_WR_CMPLT);
   I2C_MAG_ReadBuf(MAG_ADDR , buff, len, I2C_MAG_MODE_COMPLETE_XFER);
   I2C_WaitComplete(I2C_MAG_MSTAT_RD_CMPLT);
   return rxData[0];

}
// register addresses
//Magnetometer Registers
#define LIS3MDL_ADDRESS      0x1C
#define LIS3MDL_WHO_AM_I     0x0F  // should return 0x3D
#define LIS3MDL_CTRL_REG1    0x20
#define LIS3MDL_CTRL_REG2    0x21
#define LIS3MDL_CTRL_REG3    0x22
#define LIS3MDL_CTRL_REG4    0x23
#define LIS3MDL_CTRL_REG5    0x24
#define LIS3MDL_STATUS_REG   0x27   
#define LIS3MDL_OUT_X_L	     0x28  // data
#define LIS3MDL_OUT_X_H	     0x29
#define LIS3MDL_OUT_Y_L	     0x2A
#define LIS3MDL_OUT_Y_H	     0x2B
#define LIS3MDL_OUT_Z_L	     0x2C
#define LIS3MDL_OUT_Z_H	     0x2D
#define LIS3MDL_TEMP_OUT_L   0x2E
#define LIS3MDL_TEMP_OUT_H   0x2F  // data
#define LIS3MDL_INT_CFG	     0x30
#define LIS3MDL_INT_SRC	     0x31
#define LIS3MDL_INT_THS_L    0x32
#define LIS3MDL_INT_THS_H    0x33

void LISInit()
{
    int v;
    v=LISreadReg(LIS3MDL_WHO_AM_I);
    LISwriteReg(LIS3MDL_CTRL_REG1,0xec);// high perf 5hz
    LISwriteReg(LIS3MDL_CTRL_REG2,0x60); //16 gauss mode
    LISwriteReg(LIS3MDL_CTRL_REG3,0x0); // enable
}
void LISRead(short *vals)
{
    uint8 buff[8];
    //short vals[3];
    
    LISreadBuff( 0x20, buff, 6);
    LISreadBuff( 0x28, buff, 6);
    vals[0] = buff[0] | (buff[1]<<8);
    vals[1] = buff[2] | (buff[3]<<8);
    vals[2] = buff[4] | (buff[5]<<8);
    
    
    
}

void SetTamper()
{
    int x;
       for(x=0;x<5;x++)
    {
        if (EN_LED0)    
         LedSetRGB(IC_ADDR,x,60,0,60);
        if (EN_LED1)    
            LedSetRGB(IC_ADDR2,x,60,0,60);
    }
}


int32_t tick=0;
short dac_err=0;
short dac_in = 127*16;
int software_gain=1;
unsigned short dac_acc=0;
void DoDac()
{
    // a little delta signa to improve audio performance
    short dac_val;
#ifdef TEST_MODE
    tick++;
#endif
    PWM_DAC_ClearInterrupt(0xffff);
    dac_val = (dac_acc + dac_in)>>4;
    if (dac_val>255)
       dac_val=255;
    if (dac_val<0)
        dac_val=0;
    dac_acc += dac_in-(dac_val<<4);
    IDAC_2_SetValue(dac_val);
}


// data for the dac is sent over using pwm
// this measures the pulse width and then sends it to dac_in
void HandleCapture()
{
#ifndef TEST_MODE
    dac_in=(Timer_1_ReadCaptureBuf()-1800)*software_gain+2000;
#endif

    Timer_1_ClearInterrupt(0xffff);
}
#define VERSION 12
#define TAMPER_DIV 200000
uint8 st[8];
int stidx=0;


uint8 dac_msb;
uint8 dac_sync=0;
uint16 dac_rx_val;

void SetAllLeds(int r, int g, int b)
{
 int x;
    for(x=0;x<5;x++)
    {
         LedSetRGB(IC_ADDR,x,r,g,b);
         LedSetRGB(IC_ADDR2,x,r,g,b);
         LedSetRGB(IC_ADDR1B,x,r,g,b);
         LedSetRGB(IC_ADDR2B,x,r,g,b);
    }
}
 uint16_t sin_lu[]=
{   383,
    707,
    924,
   1000,
    924,
    707,
    383,
      0,
   -383,
   -707,
   -924,
  -1000,
   -924,
   -707,
   -383,
     -0};
void DoTest()
{
    if (tick>40000 && tick < 41000)
        {
         
//            AUD_SD_Write((set.aud_set&1)?1:0);
              AUD_SD_Write(1);
            AUD_GAIN0_Write(0);
               AUD_GAIN1_Write(1);
        }
    if (tick>50000 && tick < 100000)
        {
           dac_in = 2000+sin_lu[(tick/2)%16]; 
        }
    if (tick>100000 && tick < 150000)
        {
           dac_in = 2000+sin_lu[(tick)%16]; 
        }
    if (tick>150000 && tick < 200000)
        {
           dac_in = 2000+sin_lu[(tick/4)%16]; 
        }
    if (tick>200000)
       {
        AUD_GAIN1_Write(0);
        AUD_GAIN0_Write(0);
        //            AUD_SD_Write((set.aud_set&1)?1:0);
        AUD_SD_Write(0);
        dac_in=2000;

        AUD_OUT_Write(1);
        tick = 0;
        }
        
}
void DoInitLeds()
{
    SetAllLeds(100, 0,0);
    CyDelay(500);
    SetAllLeds(0, 100,0);
    CyDelay(500);
    SetAllLeds(0, 0,100);
    CyDelay(500);
    SetAllLeds(50, 50,0);
}

int main(void)
{
    int x,q,p;
    int tamper_time = TAMPER_DIV;
    CyGlobalIntEnable; /* Enable global interrupts. */
   CyDelay(150);
int z;
    /* Place your initialization/startup code here (e.g. MyInst_Start()) */
    I2C_LED_Start();
    EZI2C_CAM_EzI2CSetBuffer1(32, 16, &set.action);
    EZI2C_CAM_Start();
    EZI2C_CAM_EzI2CSetBuffer1(32, 16, &set.action);

    STEP_COUNT_Start();
    IDAC_1_Start();
    IDAC_1_SetValue(120);
    IDAC_2_Start();
    IDAC_2_SetValue(0);
    Comp_ISENSE_Start();
    
    
    
    PWM_M_Start();

    PWM_DAC_Start();
                                                                                        
    PWM_M_WriteCompare1(0);
    Control_PWM_Write(0);
    
    AUD_SD_Write(0);
    AUD_GAIN1_Write(0);
    AUD_GAIN0_Write(1);

    // this is a workaround for a floating address line
    // we look for it at 1 address if not look for the other
    
    x=LISreadReg(LIS3MDL_WHO_AM_I);
    if (x!=61)
           MAG_ADDR=0x1c;

    LISInit();
    // while(1);
   //      LISRead();
     LISreadReg( 0xf);
     Timer_1_Start();
     isr_Capture_Start();
     isr_DAC_Start();
     PWM_DAC_Start();
//    AUDIO_DAC_Start();
    //I2C_LED_sda_SetDriveMode(I2C_LED_sda_DM_STRONG);
    
    set.version=VERSION;
    set.mot_min_speed_a=100;
    set.mot_min_speed_d=50;
    set.mot_accel=50;
    set.mot_home_speed=140;
    
    while (0)
    {
     I2C_LED_sda_Write(0);
     I2C_LED_sda_Write(1);
     I2C_LED_scl_Write(0);
     I2C_LED_scl_Write(1);
    
    }
   // while(1)
      LedInitRegs(IC_ADDR2);
       LedInitRegs(IC_ADDR);
      LedInitRegs(IC_ADDR2B);
       LedInitRegs(IC_ADDR1B);
    DoInitLeds();
   //tof_init();
    
 // while(1)
   //   x=tof_measure();
       // #define AUDTEST
 #ifdef AUDTEST
        AUD_SD_Write(1);
            AUD_GAIN1_Write(1);
            AUD_GAIN0_Write(1);
           IDAC_2_SetValue(128);
        while(1)
{
    #define A 50
    uint8 sin[]={127,127+A,127,127-A};
           IDAC_2_SetValue(sin[(z++)%4]);
   //     AUD_SD_Write(1);
    CyDelayUs(500);
     //   AUD_SD_Write(0);
}
#endif
   #define PLATE_DETECT 4000000
    
while (1)
    {
#ifdef TEST_MODE
        DoTest();
#endif
          // IDAC_2_SetValue(128+(dac_in-1700)/16);
        if (set.action==0 && mot_active ==0 && (tamper_time++>TAMPER_DIV))
           {
            short vals[3];
            int mag;
            LISRead(vals);
            mag = vals[0]*vals[0]/10+vals[1]*vals[1]/10+vals[2]*vals[2]/10;
            if (mag > PLATE_DETECT)
               set.plate_detect=1;
            else
                {
                if (set.plate_detect)
                    SetTamper();
                  set.plate_detect=0;
                }
            tamper_time=0;
           }
        if (set.action==ACTION_LED)
           {
            unsigned char val = set.chA;
            // bit 7:5   0 set all
            //           1 set A
            //           2 Set B
            //           3 Set C
            set.chA=set.chB=set.chC=set.chD=0;
            switch ((val>>5))
            {
                case 0:set.chA=set.chB=set.chC=set.chD=val;break;
                case 1:set.chA=val;break;
                case 2:set.chC=val;break;
                case 3:set.chB=val;break;
                case 4:set.chD=val;break;
                case 5:set.chA=set.chC=val;break;
                case 6:set.chB=set.chD=val;break;
            }
            if ((set.chA>>5))
                set.chA=val;
            DoLedUpdate();
            set.action=0;
           }
       if (set.action==ACTION_MOT_HOME)
        {
            MotHome();
            set.action=0;
        }
       if (set.action==ACTION_MOT_ABS)
        {
            short move_pos= (set.mot_pos_hi<<8) +set.mot_pos_lo;
            if (MotMoveRel(move_pos-MotGetPos()))
               set.action=0;
            
        }
       if (set.action==ACTION_MOT_REL)
        {
            short move_pos= (set.mot_pos_hi<<8) +set.mot_pos_lo;
            if (MotMoveRel(move_pos))
               set.action=0;
            
        }
       if (set.action==ACTION_AUDIO_SET)
        {
            AUD_GAIN1_Write(0);
            AUD_GAIN0_Write(0);
            AUD_SD_Write((set.aud_set&1)?1:0);
          //  AUD_SD_Write(1);
  
            if(set.aud_set&1)
              CyDelayUs(5000);
            AUD_GAIN1_Write((set.aud_set&2)?1:0);
            AUD_GAIN0_Write((set.aud_set&4)?1:0);
            software_gain= (set.aud_set>>4)+1;
            set.action=0;
        }
     if (set.action==ACTION_BOOTLOAD)
        {
         LedSetRGB(IC_ADDR,0,100,0,0);
          set.action=0;
         // CyDelayUs(50000);
          CySoftwareReset();

        }  
     MotTask();
    set.mot_stat=mot_active;
    }
 
}

/* [] END OF FILE */
