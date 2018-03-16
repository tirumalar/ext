/*
 * Interface to the adp8860 charge pump for the eyelock RGB leds 
 * 
 * All functions return zero on success, negative value on failure.
 */

#include "nano_i2cbootload.h"
#include <unistd.h>

static char _i2c_bus[] = I2C_BUS;
int fd;
int debugFlag=0;
int delayTime=500;

int ReadData(unsigned char* buf, int size)
{
   unsigned char buff[128];
   int result,i;
   
   result = read(fd, buff, size);

   if (result != size)
	perror("Error:On Write: ");
   else
	result = 0;
   memcpy(buf,buff,size); 
   
   if(debugFlag)
   {
      printf("I2c read:");
      for(i = 0; i < size; i++) 
	 printf("0x%x,", buff[i]);     
      printf("\n");
   }
     
   return result;
}


int WriteData(unsigned char* buf,int size)
{
   int i,result;
   
   result = write(fd, buf, size);

   if (result != size)
	perror("Error:On Write: ");
   else
	result = 0;

   if(debugFlag)
   {	
   	printf ("I2c write:"); 	
	for(i= 0 ; i < size; i++)
	       printf("0x%x,", buf[i]);                
	printf("\n");
   }   

   return result;
}

int i2c_start_transaction()
{
	int result;

	fd = open(_i2c_bus, O_RDWR);
	if (fd < 0) {
		perror("Error:On Open: ");
		return 1;
	}

	result = ioctl(fd, I2C_SLAVE, NANO_BOOTLOAD_I2C_ADDR);
	if (result < 0) {
		perror("Error:On ioctl slave address");
		close(fd);
		return 2;
	}
	return 0;
}


int i2c_stop_transaction()
{
   if(fd > 0)
   {
     close(fd);
     return 1;
   }	
   return 0;
}

void update(unsigned char id,unsigned short rownum)
{
   printf("Programming row: %d \n",rownum);
   fflush(stdout);
}


void programPSOC(char* fileName,int dbFlag,int dlyTime)
{
  int result=99;
  CyBtldr_CommunicationsData comm ;
  CyBtldr_ProgressUpdate *pud;

  debugFlag = dbFlag;
  delayTime = dlyTime; 	  
  comm.OpenConnection=i2c_start_transaction;
  comm.CloseConnection=i2c_stop_transaction;
  comm.ReadData =ReadData;
  comm.WriteData=WriteData; 
  comm.MaxTransferSize=64;
  
  pud = &update;
  
  result = CyBtldr_Program(fileName,&comm, pud);

  if(result == 0)
    printf("\n Programmed ICM Successfully \n");
}






