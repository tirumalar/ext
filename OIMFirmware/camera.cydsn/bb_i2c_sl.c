#include "project.h"


#define STATE_IDLE 0
#define STATE_START 1
#define STATE_ACK   2
#define STATE_ACK_DONE 3
uint8 state = STATE_IDLE;

uint8 my_address = 0x32;
uint8 pSDA=1;
uint8 pSCL=1;
uint8 SCL=1,SDA=1;
uint8 rxbyte,bit_count=0;
uint8 txbyte=0;
uint8 icnt=0;
uint8 reg_ptr=0;
//uint8 a_space[255];
uint8 *a_space;
#define BYTE_MODE_IDLE 0
#define BYTE_MODE_ADDRESS_WRITE 1
#define BYTE_MODE_DATA_WRITE 2
#define BYTE_MODE_DATA_READ 3
uint8 byte_mode=0;
int acnt =0;
void Init_bb_i2c(uint8 *space)
{

       a_space=space;
}

char st[256];
void bb_i2c_ISR(void)
{   
    pSCL=SCL;
    pSDA=SDA;
    SCL=OLD_SCL_Read();
   SDA=OLD_SDA_Read();
  //  st[icnt]=(SCL<<1)|SDA;
   // icnt++;
    if (state==STATE_IDLE && (!SDA) && pSDA)
        {
        state=STATE_START;
        bit_count=0;
          byte_mode=   BYTE_MODE_IDLE;
        return;
        }


    if (state==STATE_START && (!pSCL) && (SCL)) // rising edge sample bit
       {
        if (byte_mode!=BYTE_MODE_DATA_READ)
            {
            rxbyte=rxbyte<<1;
                SDA=OLD_SDA_Read();
            if (SDA)
             rxbyte|=0x1;
             bit_count++;
            }
        if (bit_count==9)
                    bit_count=0;
        if (bit_count==8 && byte_mode!=BYTE_MODE_DATA_READ)
           {
           if (BYTE_MODE_IDLE == byte_mode)
                {
                     acnt++;
                if ((rxbyte&0xfe)==(my_address<<1))
                    {
                       
                    state=STATE_ACK;
                        OLD_SDA_Write(0);
                    if (rxbyte&0x1)
                        {
                        byte_mode=BYTE_MODE_DATA_READ;
                        txbyte=a_space[reg_ptr++];
                        bit_count=0;
                        }
                    else
                        byte_mode=BYTE_MODE_ADDRESS_WRITE;
                    }
               else
                    state = STATE_IDLE;
                }
            else
            if (BYTE_MODE_ADDRESS_WRITE == byte_mode)
                {
                    reg_ptr= rxbyte;
                    state = STATE_ACK;
                    byte_mode=BYTE_MODE_DATA_WRITE;
                }
            else
              if (byte_mode==BYTE_MODE_DATA_WRITE)
                {
                a_space[reg_ptr++]=rxbyte;
                state = STATE_ACK;
                }

           }
       }
    
    /// falling edge of clock we change the data
   if (state==STATE_ACK &&   (pSCL) && (!SCL))
        {
            OLD_SDA_Write(0);
            state = STATE_START;
         //   bit_count=0;
        }
    else
        if (state==STATE_START && (pSCL) && (!SCL))
        {
        OLD_SDA_Write(1);

       if (byte_mode==BYTE_MODE_DATA_READ)
          {
            bit_count++;
            if ((txbyte&0x80)|| bit_count>=8)
                 OLD_SDA_Write(1);
            else
                 OLD_SDA_Write(0);
            txbyte=txbyte<<1;

            if (bit_count==9)
                {
                  st[icnt]=reg_ptr;
                 icnt++;
                bit_count=0;
                txbyte=a_space[reg_ptr++];
 
                }
 
          }
        }
    if (SCL && SDA && (pSDA==0))
        {
            state = STATE_IDLE;
            byte_mode=BYTE_MODE_IDLE ;
            bit_count=0;
            
        }
}
