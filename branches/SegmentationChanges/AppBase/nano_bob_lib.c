#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>
#include "system.h"
#include <linux/i2c-dev.h> 
#ifdef CMX_C1
#define I2C_BUS	"/dev/i2c-1"
#else
#define I2C_BUS	"/dev/i2c-3"
#endif

static char _i2c_bus[] = I2C_BUS;
#define NANO_BOB_I2C_ADDR 0x38



#include "bob_com.h"

#define MIN(a,b) ( ((a)>(b)) ?(b):(a))


int internal_read_reg(int fd, unsigned char reg, unsigned int *val)
{
	int result;
	unsigned char buff[2];

	buff[0] = reg;

	result = write(fd, buff, 1);
	if (result != 1) {
		perror("write");
		return -1;
	}

	buff[0] = 0;
        usleep(100);
	result = read(fd, buff, 1);
	if (result != 1) {
                printf("error \n");
		perror("read");
		return -1;
	}

	if (val)
		*val = buff[0];

	return 0;
}
#define MAX_WRITE_LEN 255
#define MAX_READ_LEN 255

int internal_read_array(int fd, unsigned char reg, char *buf, int len)
{
	int result;
	unsigned char *p=buf;
	char buff[2];

	buff[0] = reg;

	result = write(fd, buff, 1);
	if (result != 1) {
		perror("write");
		return -1;
	}

       usleep(100);
	   result = read(fd, p, len);
	   if (result != len)
	   	   	   	   {
                	printf("error \n");
                	perror("read");
                	return -1;
	   				}

	   /*while(len--)
	   {
	   result = read(fd, p, 1);
	   if (result != 1)
	   	   	   	   {
                	printf("error \n");
                	perror("read");
                	return -1;
	   				}
	    p++;
	    }
*/

	return 0;
}

int internal_write_reg(int fd, unsigned char reg, unsigned int val)
{
	int result;
	unsigned char buff[2];

	buff[0] = reg;
	buff[1] = val;
	
	result = write(fd, buff, 2);

	if (result != 2)
		perror("write");
	else
		result = 0;

	return result;
}
int internal_write_array(int fd, unsigned char reg, void *ptr, int len)
{
	int result;
	unsigned char buff[MAX_WRITE_LEN];

	buff[0] = reg;
	memcpy(&buff[1],ptr,len);
	
	result = write(fd, buff, len+1);

	if (result != len+1)
		perror("write");
	else
		result = 0;

	return result;
}


int i2c_start_transaction()
{
	int fd, result;

	fd = open(_i2c_bus, O_RDWR);
	if (fd < 0) { 
		perror("open");
		return 0;
	}

	result = ioctl(fd, I2C_SLAVE, NANO_BOB_I2C_ADDR);
	if (result < 0) {
		perror("ioctl");
		close(fd);
		return 0;
	}

	return fd;
}


pthread_mutex_t lock;


void   BobMutexStart(void)
{
pthread_mutex_lock(&lock);
}
void   BobMutexEnd(void)
{
pthread_mutex_unlock(&lock);
}

int fd =0;

void BobSetBitVal(unsigned char bit, int val)
{
  BobMutexStart();

  internal_write_reg(fd, CMD_OFFSET+1,bit);
  if (val)
     internal_write_reg(fd, CMD_OFFSET,CMD_SET_OUTPUT_BITS);
  else
     internal_write_reg(fd, CMD_OFFSET,CMD_CLEAR_OUTPUT_BITS);

  BobMutexEnd();
}

int   BobSetRelay1(int val)     {  BobSetBitVal(RELAY_1_OUT,val); }
int   BobSetRelay2(int val)     {  BobSetBitVal(RELAY_1_OUT,val); }
int   BobSetSounderOUT(int val) {  BobSetBitVal(SOUND_OUT,val); }
int   BobSetLedROUT(int val)    {  BobSetBitVal(LED_OUT_RED,val); }
int   BobSetLedGOUT(int val)    {  BobSetBitVal(LED_OUT_GRN,val); }
void  BobSetTamperOUT(int val) {  BobSetBitVal(TAMPER_OUT,val); }


void (*BobInputCB)()= NULL;

int BobSetInputCB( void (*cb)() )
{
	BobInputCB = cb;
}

#define MAX_SEND 4
static int inputStat = 0;
static int txReadyStat = 0;

int BobUART1_TXReady()
{
  return txReadyStat;
}

int BobGetAllInputs(void)
{
 return inputStat;
}

int  BobGetSounderIn(){return inputStat&SOUNDER_IN; };
int  BobGetLedRIn(){return inputStat&LED_IN_RED; };
int  BobGetLedGIn(){return inputStat&LED_IN_GRN; };
int  BobGetTamperIn(){return inputStat&TAMPER_IN; };
int  BobGetReed1(){return inputStat&REED_1_IN; };
int  BobGetReed2(){return inputStat&REED_2_IN; };

int Bob_Send_UART1(char *data, int len )
{
  while (len)
     {
        while (BobUART1_TXReady()==0)
            {
	     usleep(1000);
	    }
	BobMutexStart();
	internal_write_reg(fd, CMD_OFFSET+1,MIN(len,MAX_SEND));
	internal_write_array(fd, CMD_OFFSET+2,data,MIN(len,MAX_SEND));
	internal_write_reg(fd, CMD_OFFSET,CMD_SEND_UART1);
	BobMutexEnd();
#if DEBUG_LEVEL >2
	printf("sent %d\n",MIN(len,MAX_SEND));
#endif
	txReadyStat =0;
        data += MIN(len,MAX_SEND);
        len -= MIN(len,MAX_SEND);
        
     }
}

#define CIRC_BUFF  256
int uart1_write_ptr =0;
int uart1_read_ptr =0;
char uart1_buff[CIRC_BUFF];

int Bob_Read_UART1(char *data, int len , int timeout)
{
  int read_count=0;
   while (len && timeout)
        {
	   if (uart1_write_ptr != uart1_read_ptr)
          {
	      *data++=uart1_buff[uart1_read_ptr];
	      len--;
 	      uart1_read_ptr = (uart1_read_ptr +1) % CIRC_BUFF;
	      read_count++;
	      }
         else
            {
        	 usleep(1000);
        	 timeout--;
            }
	}
    return read_count;
}

void BobHandleUart1RX()
{
    int len=0;
	char buff[255];
	char *p = buff;
	BobMutexStart();
	internal_read_reg(fd, UART1_RX_BYTES_OFFSET, &len);
    if (len)
         {
		 internal_read_array(fd, UART1_RX_BUFFER_OFFSET,buff,len);
		 // printf("len = %d\n",len);
		 while (len--)
	     {
             uart1_buff[uart1_write_ptr] = *p++;
	     // can add overflow detection
             uart1_write_ptr= (uart1_write_ptr+1) % CIRC_BUFF;
	     }
	 }
	 internal_write_reg(fd, UART1_RX_BYTES_OFFSET, 0);
	BobMutexEnd();
}

#define POLL_TIME 1000

void bob_thread(void *arg)
{
fd = i2c_start_transaction();
 unsigned int stat;
 if(fd==NULL)
     {
	printf("Error starting interface");
        return;
     }
 pthread_mutex_init(&lock,NULL);
 printf("Thread Started\n");
 while (1)
    {
     usleep(POLL_TIME);
     internal_read_reg(fd, 0, &stat);
 //printf("Stat %x\n",stat&0xff);
     if (stat & STAT_I_CHANGE)
          {
          internal_read_reg(fd,INPUT_OFFSET, &inputStat);
	  inputStat = stat;
          if (BobInputCB)
             BobInputCB();
   
	  }
    if (stat & STAT_TXUART1)
	  {
	  txReadyStat=1;
	  }
     if (stat & STAT_RXUART1)
          {
	   BobHandleUart1RX();
	  }
    internal_write_reg(fd, 0,0);
    }
  close (fd);
}


pthread_t com_thread;
int BobInitComs()
{
  pthread_create (&com_thread,NULL,bob_thread,NULL);
}


