/*
 * Interface to the adp8860 charge pump for the eyelock RGB leds
 *
 * All functions return zero on success, negative value on failure.
 */


#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <nano_i2cbootload.h>
#include <nano_i2ccommand.h>
#include <unistd.h>

void getVersion()
{
   char version[50];
   getSWVersion(version);
   getHWVersion(version);
   //getMTBSWVersion(version);
   //getMTBHWVersion(version);

}


void startProgram(char* fname,int dbFlag,int dlyTime)
{
   int i;

   setBootload();

   for(i=0; i< 1000; i++) //Sleep for 30 msecond to move to bootloader
      usleep(1000);

   printf("Programming %s to ICM \nPlease wait while ICM is being programmed... \n",fname);

   programPSOC(optarg,dbFlag,dlyTime);

}

int StartLoadReaderFile(char* fname)
{

   printf("Loading %s to reader \nPlease wait while the reader is being programmed... \n",fname);

   int result = 0;
   unsigned int len = 0;
   unsigned int size = 0;
   unsigned char *buffer, *data;

	// open file and read to an array
	FILE *fp = fopen(fname, "r");
	if (fp == NULL) {
		printf("Failed to open file %s", fname);
		return 0;
	}

	fseek(fp, 0, SEEK_END);
	size = ftell(fp);
	rewind (fp);
	buffer = (unsigned char *)malloc(size);
	if (buffer == NULL) {
		printf("Failed to allocate memory %d bytes\n", size);
		return 0;
	}

	len = fread (buffer,1, size,fp);
	if (len != size) {
		printf("Failed to read binary file to buffer %d bytes\n", size);
		fclose (fp);
		return 0;
	}
	fclose (fp);

	// clear the serial flash memory
	printf("StartLoadReaderFile() clear ...\n");
	result = clearReaderMemory();
	if (!result) {
		printf("Failed to clear reader memory\n");
		free (buffer);
		return 0;
	}
	sleep(5);

	// send data to reader
	len = 0;
	data = buffer;
	int packet_size = READER_DATA_SIZE;

	printf("StartLoadReaderFile() send data ...\n");
	int count = 0;
	while (len < size) {
		//usleep(100000);
		count++;

		if (len + packet_size < size) {
			result = sendReaderData(data, packet_size, 0);
			len += packet_size;
			data += packet_size;
		}
		else if (len <= size) {
			result = sendReaderData(data, size-len, 1);
			len = size;
		}
		if (!result) {
			printf("Failed to send reader data");
			free (buffer);
			return 0;
		}
		printf("Total %d bytes sent out, packet %d\n", len, count);

	}
	free (buffer);
	sleep(5);

	printf("StartLoadReaderFile() process ...\n");
	result = startBootloadProcess();
	if (!result) {
		printf("Failed to start bootloader process\n");
		return 0;
	}

	printf("\n Programming Reader Start ... \n");
	return 1;
}

void usage()
{
  printf("------------------- Usage ---------------------- \n");
  printf(" Version   : icm_communicator -v  \n");
  printf(" Debug     : icm_communicator -d  \n");
  printf(" Delay     : icm_communicator -t  <Time (in ms)> \n");
  printf(" Usage     : icm_communicator -u  \n");
  printf(" Program   : icm_communicator -p <ICM FileName> \n");
  printf(" Reader    : icm_communicator -r <Reader FileName> \n");
//  printf(" I2C       : icm_communicator -i <dump (1), set (2))> \n");
//  printf(" I2C offset: icm_communicator -o <offset> \n");
//  printf(" I2C value : icm_communicator -n <value> \n");
  printf("----------------------------------------------- \n");
}


int main(int argc, char **argv)
{

  int c;
  int dbFlag = 0;
  int dlyTime = 30;	//minimum 14
  int offset = 0;
  int value = 0;
  char *datafile;
  int i2coption = 0;
  opterr = 0;
  printf("Test Print -- Inside main of icm communicator\n");
  while ((c = getopt (argc, argv, "duvt:p:r:o:n:i:")) != -1)
  {
     switch (c)
     {
       case 'p':
	 startProgram(optarg,dbFlag,dlyTime);
         break;

       case 'v':
         getVersion();
         break;

       case 'd':
         dbFlag = 1;
         break;

       case 'u':
         usage();
         break;

       case 't':
          dlyTime = atoi(optarg);
          printf("delay time: %d msecs\n",dlyTime);
          break;

       case 'r':
    	 if (!StartLoadReaderFile(optarg)) {
    		 printf("Loading %s to reader failed, please try again. \n",optarg);
    	 }
    	   //StartLoadReaderFile(optarg,dbFlag,dlyTime);
         break;

       case 'i':
    	 i2coption = atoi(optarg);
         break;

       case 'o':
         offset = atoi(optarg);
         break;

       case 'n':
         value = atoi(optarg);
         break;

       case 'a':
         datafile = optarg;
         break;

       default:
         printf("Error: Invalid Arguments, please check usage for valid arguments \n ");
         usage();

    }
  }

  if (i2coption == 1) {
	  printf("i2cdump: start address 0x38, size 250. \n");
	  dumpI2C();
  }
  else if (i2coption == 2) {
	  printf("i2cset: set register %d value %d. \n", offset, value);
	  setI2C(offset, value);
  }
  else if (i2coption == 3) {
	  printf("i2cset: set data from file %s, start offset %d. \n", datafile, offset);
	  setI2CArray(offset, datafile);
  }

  if(optind == 1)
  {
    printf("Error: Invalid Arguments, please check usage for valid arguments \n ");
    usage();
  }

  return 0;
}
