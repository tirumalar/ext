/*
 * Interface to the adp8860 charge pump for the eyelock RGB leds 
 * 
 * All functions return zero on success, negative value on failure.
 */

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
#include "bob_com.h"
#include <linux/i2c-dev.h> 
pthread_t l_thread;

void listen_thread(void *arg)
{
 char dat[200];
 int len;
 int x;
 while (1)
    {
     len = Bob_Read_UART1(dat, 20, 10);
     if (len >0)
          {
	   printf("Read thread read (%d)->\n",len);
	   for (x=0;x < len; x++)
              printf("%x %s",dat[x],x%16==15?"\n":" ");
	   printf("\n",len);
	  }
     }
}

void TestCB ()
{
   printf(" Got cb %x\n",BobGetAllInputs());

}

int main(int argc, char **argv)
{
int val;
char temp[1024];
int x;

for (x=0;x< sizeof(temp);x++)
	 temp[x]=x;
printf("Starting This is a bob test program \n");
BobInitComs();

// this is the call back for IO changes
BobSetInputCB(TestCB);
// this thread listens from bobs Uart 1

pthread_create (&l_thread,NULL,listen_thread,NULL);

while (1)
  {
  val = getchar() - '0';
  if (val >= 0 && val <4)
       {
          if (val ==0)
		  BobSetRelay1(val);
          if (val ==1)
		Bob_Send_UART1(temp,16);
	}

  }
return 1;
}

