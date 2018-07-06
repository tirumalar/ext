#include "nano_i2ccommand.h"
#include <unistd.h>

//static char _i2c_bus[] = I2C_BUS;
#define NANO_CMD_I2C_ADDR 0x38
#define NANO_MTB_I2C_ADDR 0x2E
#define BOB_STATUS_IN_OFFSET 			0
#define BOB_STATUS_OUT_OFFSET			1
#define BOB_COMMAND_OFFSET				2
#define BOB_ACS_TYPE_OFFSET				3
#define BOB_DATA_LENGTH_OFFSET 			7	// 2 bytes, high=48, low=49
#define BOB_ACCESS_DATA_OFFSET 			73	// 20 bytes

int internal_read_reg(int fd, unsigned char reg)
{
	int result;
	unsigned char buff[2];

	buff[0] = reg;

	result = write(fd, buff, 1);
	if (result != 1) {
		//perror(" Error: On Readwrite: ");
		return -1;
	}
	

	usleep(100);
		
	result = read(fd, buff, 1);
	if (result != 1) {
                //perror("Error: On Read: ");
		return -1;
	}


	return (int)buff[0];
}

int internal_write_reg(int fd, unsigned char reg, unsigned int val)
{
	int result;
	unsigned char buff[2];

	buff[0] = reg;
	buff[1] = val;
	
	result = write(fd, buff, 2);

	if (result != 2)
		perror("Error:On Write: ");
	else
		result = 0;

	return result;
}

int internal_read_reg16(int fd, unsigned char reg)
{
	int result;
	unsigned char buff[3];
	if(fd == 0)
        return -1;

	buff[0] = 0;
	buff[1] = reg;

	result = write(fd, buff, 2);
	if (result != 2) {
		//perror(" Error: On Readwrite: ");
		return -1;
	}

	usleep(100);

	result = read(fd, buff, 1);
	if (result != 1) {
                //perror("Error: On Read: ");
		return -1;
	}


	return (int)buff[0];
}

int internal_write_reg16(int fd, unsigned int reg, unsigned int val)
{
	int result, counter;
	unsigned char buff[3];
	if(fd == 0)
        return -1;

	counter = 3; //try to send the command to ICM counter times.

	buff[0] = (reg >> 8) & 0xFF;
	buff[1] = reg & 0xFF;
	buff[2] = val & 0xFF;

	result = write(fd, buff, 3);
	while(counter > 0)
	{
		if (result != 3)
  			usleep(500);	
		else
			return 0; //success

		counter--;
		result = write(fd, buff, 3); //send the command again
	}

	return(-1);
}

int internal_read_array(int fd, unsigned char reg, char *buf, int len)
{
	int result;
	char buff[255];

	if(fd == 0)
        return -1;

	buff[0] = 0;
	buff[1] = reg;
	//EyelockLog(logger, DEBUG, "BoB => @@@ read data array fd=%d reg=%d, len=%d @@@", fd, reg, len);
	result = write(fd, buff, 2);
	if (result != 2) {
		perror("write");
		return -1;
	}

	usleep(100);
	//usleep(50);
	result = read(fd, buff, len);
	if (result != len)
	{
        perror("read data");
        return -1;
	}

	memcpy(buf, buff, len);

	return result;
}
int internal_write_array(int fd, unsigned char reg, void *ptr, int len)
{
	int result = -1;
	unsigned char buff[255];
	//EyelockLog(logger, DEBUG, "BoB => @@@ write data array reg=%d, len=%d @@@", reg, len);
	if(fd == 0)
        return -1;

	buff[0] = 0;
	buff[1] = reg;
	if (ptr)
		memcpy(&buff[2],ptr,len);
	else
		return -1;

	result = write(fd, buff, len+2);

	if (result != len+2) {
		perror("write");
		return -1;
	}
	else
		result = 0;

	usleep(50);
	return result;
}

int i2c_start(int i2cAddr)
{
	int fd, result;

	fd = open(_i2c_bus, O_RDWR);
	if (fd < 0) {
		perror("Error:On Open: I2C ");
		return 0;
	}

	result = ioctl(fd, I2C_SLAVE, i2cAddr);
	if (result < 0) {
		perror("Error:On ioctl Slave Address");
		if(fd) close(fd);
		return 0;
	}

	return fd;
}

void getSWVersion(char* swversion)
{
  
  int version[3];
  int fd = i2c_start(NANO_CMD_I2C_ADDR);
  if(fd == 0)
     printf("nanoNxt ICM Software Version: -1.-1.-1 \n");

  usleep(100);
  version[0] = internal_read_reg16(fd,67);
  usleep(100);
  version[1] = internal_read_reg16(fd,68);
  usleep(100);
  version[2] = internal_read_reg16(fd,69);
  usleep(100);
  if(fd) close(fd);

  printf("nanoNXT ICM Software Version: %d.%d.%d \n",version[0],version[1],version[2]);
  sprintf(swversion,"%d.%d.%d",version[0],version[1],version[2]);
}

void getMTBSWVersion(char* swversion)
{
  int version[3];
  int fd = i2c_start(NANO_MTB_I2C_ADDR);
  if(fd == 0)
     printf("nanoNxt MTB Software Version: -1.-1.-1 \n");
 
  version[0] = internal_read_reg(fd,10);
  version[1] = internal_read_reg(fd,11);
  version[2] = internal_read_reg(fd,12);
  if(fd) close(fd);

  printf("nanoNXT MTB Software Version: %d.%d.%d \n",version[0],version[1],version[2]);
  sprintf(swversion,"%d.%d.%d",version[0],version[1],version[2]);

}

void getMTBHWVersion(char* hwversion)
{

  int version[3];
  int fd = i2c_start(NANO_MTB_I2C_ADDR);
  if(fd == 0)
     printf("nanoNXT MTB Hardware Version: -1.-1.-1 \n");

  version[0] = internal_read_reg(fd,13);
  version[1] = internal_read_reg(fd,14);
  version[2] = internal_read_reg(fd,15);
  if(fd) close(fd);

 if(version[0] == -1)
  {
    printf("nanoNXT MTB Hardware Version: -1.-1.-1 \n"); 
    sprintf(hwversion,"-1.-1.-1");
  }
  else
  {
    printf("nanoNXT MTB Hardware Version: %02x.%02x.%02x \n",version[0],version[1],version[2]); 
    sprintf(hwversion,"%02x.%02x.%02x",version[0],version[1],version[2]);
  }
}

void getHWVersion(char* hwversion)
{

  int version[3];
  int fd = i2c_start(NANO_CMD_I2C_ADDR);
  if(fd == 0)
     printf("nanoNXT ICM Hardware Version: -1.-1.-1 \n");

  usleep(100);
  version[0] = internal_read_reg16(fd,70);
  usleep(100);
  version[1] = internal_read_reg16(fd,71);
  usleep(100);
  version[2] = internal_read_reg16(fd,72);
  usleep(100);
  if(fd) close(fd);

  if(version[0] == -1)
  {
    printf("nanoNXT ICM Hardware Version: -1.-1.-1 \n"); 
    sprintf(hwversion,"-1.-1.-1");
  }
  else
  {
    printf("nanoNXT ICM Hardware Version: %02x.%02x.%02x \n",version[0],version[1],version[2]); 
    sprintf(hwversion,"%02x.%02x.%02x",version[0],version[1],version[2]);
  }
}

void setBootload()
{
  int result, fd, addr;

  addr = NANO_CMD_I2C_ADDR;
  fd = i2c_start(addr);
 
  if(fd == 0)
  {
     printf("Unable to communicate with I2C bus, device address=%d \n",addr);
     return ;	
  }
	
  usleep(500);	
  result = internal_write_reg16(fd,2,1); //send the bootloading command

  usleep(500);	
  if(fd) close(fd);

  if(result == 0)
  {
   	printf("Starting ICM bootloader.. \n");
  }
}

int clearReaderMemory()
{
  int result, fd, addr;
  char msg[2] = {0x6B, 0x00};
  char setlen[2] = {0x00, 0x02};

  addr = NANO_CMD_I2C_ADDR;
  fd = i2c_start(addr);

  if(fd == 0)
  {
     printf("Unable to communicate with I2C bus, device address=%d \n",addr);
     return 0;
  }

  // init registers
  usleep(500);
  result = internal_write_reg16(fd,BOB_ACS_TYPE_OFFSET,56);
  if (result)
  {
     printf("Unable to set ACS type, device address=%d \n",BOB_ACS_TYPE_OFFSET);
     if(fd) close(fd);
     return 0;
  }

  usleep(500);
  internal_write_reg16(fd, BOB_STATUS_OUT_OFFSET, 9);
  usleep(500);
  internal_write_reg16(fd, BOB_STATUS_IN_OFFSET, 0);

  // send clear command 0x6B 0x00
  usleep(500);
  result = internal_write_array(fd,BOB_DATA_LENGTH_OFFSET,setlen, 2);
  if (result)
  {
     printf("Unable to send data length, device address=%d \n",BOB_DATA_LENGTH_OFFSET);
     if(fd) close(fd);
     return 0;
  }

  usleep(500);
  result = internal_write_array(fd,BOB_ACCESS_DATA_OFFSET,msg, 2);
  if (result)
  {
     printf("Unable to send CMD_CLEAR_EM_DATA (0x6B), device address=%d \n",BOB_ACCESS_DATA_OFFSET);
     if(fd) close(fd);
     return 0;
  }

  usleep(500);
  result = internal_write_reg16(fd,BOB_COMMAND_OFFSET,4);
  if (result)
  {
     printf("Unable to set COMMAND (4), device address=%d \n",BOB_COMMAND_OFFSET);
     if(fd) close(fd);
     return 0;
  }
  printf("sending clear msg '0x6B 0x00'\n");

  usleep(500);
  if(fd) close(fd);

  result = checkReaderReply();
  if(result == 1)
  {
   	printf("Memory cleared, Sending binary file ... \n");
  }

  return result;

}

int checkReaderReply()
{
	int result = 0;
	int fd, status;
	char reply[10]={0xFF};
	int count = 0;

	usleep(10000);

	while (count < 400) {
		usleep(5000);
		fd = i2c_start(NANO_CMD_I2C_ADDR);
		if(fd == 0) {
			printf("Unable to communicate with I2C bus, device address=%d \n", 0x38);
			return 0;
		}
		status = internal_read_reg16(fd,BOB_STATUS_IN_OFFSET);
		if ((status & 0x01) && (status & 0x02)) {	// reply ready
			//printf("status: 0x%x\n", status);
			usleep(500);
			internal_write_reg16(fd, BOB_STATUS_IN_OFFSET, 0);
			usleep(500);
			int icmlen = internal_read_reg16(fd,BOB_DATA_LENGTH_OFFSET+1);
			usleep(500);
			int icmcmd = internal_read_reg16(fd,BOB_COMMAND_OFFSET);
			if (icmlen != 3 || icmcmd != 0) {
				printf("I2C register errors! length %d, command %d\n", icmlen, icmcmd);
				result = 0;
			}

			usleep(1000);
			result = internal_read_array(fd, BOB_ACCESS_DATA_OFFSET, reply, 3);
			if (result == 3) {
				usleep(500);
				internal_write_reg16(fd, BOB_STATUS_OUT_OFFSET, 9);
				if (reply[0] == 0x5C && reply[1] == 0x01 && reply[2] == 0x00) {
					result = 1;
					break;
				}
				else {
					printf("reader reply failed: value 0x%x, 0x%x, 0x%x\n", reply[0],reply[1],reply[2]);
					result = 0;
				}
			}
			else {
				printf("I2C read data failed: result %d!\n", result);
				result = 0;
			}
		}

		count++;
		if(fd) close(fd);
	}

	if (count == 400) {
		printf("Failed to get reader reply in 2sec\n");
		result = 0;
	}

	if(fd) close(fd);
	return result;
}

int sendReaderData(unsigned char *data, int datalen, int lastpack)
{
	int result, fd, addr;
	unsigned char msg[128];
	char len[2];

	msg[0] = 0x6E;
	msg[1] = datalen+1;
	msg[2] = lastpack;

	memcpy(&msg[3], data, datalen);

	addr = NANO_CMD_I2C_ADDR;
	fd = i2c_start(addr);
	if(fd == 0)
	{
		printf("Unable to communicate with I2C bus, device address=%d \n",addr);
	    return 0;
	}

	len[0] = 0;
	len[1] = datalen + 3;
	usleep(500);
	result = internal_write_array(fd,BOB_DATA_LENGTH_OFFSET,len, 2);
	if (result)
	{
		printf("Unable to send data length, device address=%d \n",BOB_DATA_LENGTH_OFFSET);
	    if(fd) close(fd);
	    return 0;
	}

	usleep(500);
	result = internal_write_array(fd,BOB_ACCESS_DATA_OFFSET,msg, datalen+3);
	if (result)
	{
	    printf("Unable to send CMD_PUT_EM_DATA (0x6E), device address=%d \n",BOB_ACCESS_DATA_OFFSET);
	    if(fd) close(fd);
	    return 0;
	}

	usleep(500);
	result = internal_write_reg16(fd,BOB_COMMAND_OFFSET,4);
	if (result)
	{
		printf("Unable to set COMMAND (4), device address=%d \n",BOB_COMMAND_OFFSET);
	    if(fd) close(fd);
	    return 0;
	}

	usleep(500);
	if(fd) close(fd);
//	int x=0;
//	for (x=0; x < datalen+3; x++)
//	        {
//	                printf(" 0x%x", msg[x]);
//	        }
//	        printf("\n");

	result = checkReaderReply();
	if(result == 0)
		printf("Packet sent failed %d bytes \n", datalen+3);

	return result;
}

int startBootloadProcess()
{
	  int result, fd, addr;
	  char msg[2] = {0x6A, 0x00};
	  char setlen[2] = {0x00, 0x02};

	  usleep(200000);

	  addr = NANO_CMD_I2C_ADDR;
	  fd = i2c_start(addr);
	  if(fd == 0)
	  {
	     printf("Unable to communicate with I2C bus, device address=%d \n",addr);
	     return 0;
	  }

	  usleep(500);
	  result = internal_write_array(fd,BOB_DATA_LENGTH_OFFSET,setlen, 2);
	  if (result)
	  {
	     printf("Unable to send data length, device address=%d \n",BOB_DATA_LENGTH_OFFSET);
	     if(fd) close(fd);
	     return 0;
	  }

	  usleep(500);
	  result = internal_write_array(fd,BOB_ACCESS_DATA_OFFSET,msg, 2);
	  if (result)
	  {
	     printf("Unable to send CMD_PROCESS_EM_DATA (0x6A), device address=%d \n",BOB_ACCESS_DATA_OFFSET);
	     if(fd) close(fd);
	     return 0;
	  }

	  usleep(500);
	  result = internal_write_reg16(fd,BOB_COMMAND_OFFSET,4);
	  if (result)
	  {
	     printf("Unable to set COMMAND (4), device address=%d \n",BOB_COMMAND_OFFSET);
	     if(fd) close(fd);
	     return 0;
	  }

	  usleep(500);
	  if(fd) close(fd);
	  printf("sending process msg '0x6A 0x00'\n");

	  result = checkReaderReply();
	  if(result == 0)
	  {
	   	printf("PROCESS command failed... \n");
	  }

	  return result;


}

void dumpI2C()
{
	char buf[256];
	int fd;
	fd = i2c_start(NANO_CMD_I2C_ADDR);
	if(fd == 0) {
		printf("Unable to communicate with I2C bus, device address=%d \n", 0x38);
		return;
	}
	usleep(500);
	internal_read_array(fd, 0, buf, 250);
	usleep(500);
	close(fd);
	int i;
    for (i=0; i < 250; i++) {
        if (i%10)
            printf(" %2X", buf[i]);
        else {
        	printf("\n");
        	printf("0x%2X", buf[i]);
        }
    }
    printf("\n");
}

void setI2C(int offset, int value)
{
	int fd, result;
	fd = i2c_start(NANO_CMD_I2C_ADDR);
	if(fd == 0) {
		printf("Unable to communicate with I2C bus, device address=%d \n", 0x38);
		return;
	}
	usleep(500);
	result = internal_write_reg16(fd,offset,value);
	if (result)
	{
	     printf("Unable to set offset %d value %d \n",offset, value);
	}

	usleep(500);
	if(fd) close(fd);

}

void setI2CArray(int offset, char *datafile)
{
	int fd, result, len;
	char buffer[128];

	// open file and read to an array
	FILE *fp = fopen(datafile, "r");
	if (fp == NULL) {
		printf("Failed to open file %s", datafile);
		return;
	}

	len = fread (buffer, 1, 128, fp);
	if (len == 0) {
		printf("Failed to read data from file to buffer \n");
		fclose (fp);
		return;
	}
	fclose (fp);

	fd = i2c_start(NANO_CMD_I2C_ADDR);
	if(fd == 0) {
		printf("Unable to communicate with I2C bus, device address=%d \n", 0x38);
		return;
	}
	usleep(500);
	result = internal_write_array(fd,offset,buffer,len);
	if (result)
	{
	     printf("Unable to write array offset %d length %d \n",offset, len);
	}

	usleep(500);
	if(fd) close(fd);

}
