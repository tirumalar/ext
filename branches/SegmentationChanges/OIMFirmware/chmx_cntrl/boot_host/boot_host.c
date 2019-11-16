#include <string.h>
// #include "StringImage.h"
#include "communication_api.h"
#include "cybtldr_api.h"
#include "cybtldr_command.h"
#include "cybtldr_parse.h"
#include "cybtldr_utils.h"
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

CyBtldr_CommunicationsData comm1 ;
uint16 BootloadStringImage(const char *bootloadImagePtr[],unsigned int lineCount );

char * serial_port = NULL;
int  serial_speed = 115200;


// see cybtldr_utils.h
// host error
void error_info_host(uint16 error)
{
  switch(error)
    {
    case CYRET_ERR_FILE:
      printf("[ERROR] File is not accessable [0x%X]\n",error);
      break;
    case CYRET_ERR_EOF:
      printf("[ERROR] Reached the end of the file [0x%X]\n",error);
      break;
    case CYRET_ERR_LENGTH:
      printf("[ERROR] The amount of data available is outside the expected range [0x%X]\n",error);
      break;
    case CYRET_ERR_DATA:
      printf("[ERROR] The data is not of the proper form [0x%X]\n",error);
      break;
    case CYRET_ERR_CMD:
      printf("[ERROR] The command is not recognized [0x%X]\n",error);
      break;
    case CYRET_ERR_DEVICE:
      printf("[ERROR] The expected device does not match the detected device [0x%X]\n",error);
      break;
    case CYRET_ERR_VERSION:
      printf("[ERROR] The bootloader version detected is not supported [0x%X]\n",error);
      break;
    case CYRET_ERR_CHECKSUM:
      printf("[ERROR] The checksum does not match the expected value [0x%X]\n",error);
      break;
    case CYRET_ERR_ARRAY:
      printf("[ERROR] The flash array is not valid [0x%X]\n",error);
      break;
    case CYRET_ERR_ROW:
      printf("[ERROR] The flash row is not valid [0x%X]\n",error);
      break;
    case CYRET_ERR_BTLDR:
      printf("[ERROR] The bootloader is not ready to process data [0x%X]\n",error);
      break;
    case CYRET_ERR_ACTIVE:
      printf("[ERROR] The application is currently marked as active [0x%X]\n",error);
      break;
    case CYRET_ERR_UNK:
      printf("[ERROR] The operation was aborted [0x%X]\n",error);
      break;
    case CYRET_ABORT:
      printf("[ERROR] The operation was aborted [0x%X]\n",error);
      break;
    case CYRET_ERR_COMM_MASK:
      printf("[ERROR] The communications object reported an error [0x%X]\n",error);
      break;
    case CYRET_ERR_BTLDR_MASK:
      printf("[ERROR] The bootloader reported an error [0x%X]\n",error);
      break;
    default:
      printf("[ERROR] An unknown error occured [0x%X]\n",error);
      break;
    }
}

// bootloader error
void error_info_bootldr(uint16 error)
{
  switch(error)
    {
    case CYBTLDR_STAT_SUCCESS:
      printf("[ERROR] Completed successfully [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_KEY:
      printf("[ERROR] The provided key does not match the expected value [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_VERIFY:
      printf("[ERROR] The verification of flash failed [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_LENGTH:
      printf("[ERROR] The amount of data available is outside the expected range [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_DATA:
      printf("[ERROR] The data is not of the proper form [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_CMD:
      printf("[ERROR] The command is not recognized [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_DEVICE:
      printf("[ERROR] The expected device does not match the detected device [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_VERSION:
      printf("[ERROR] The bootloader version detected is not supported [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_CHECKSUM:
      printf("[ERROR] The checksum does not match the expected value [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_ARRAY:
      printf("[ERROR] The flash array is not valid [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_ROW:
      printf("[ERROR] The flash row is not valid [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_PROTECT:
      printf("[ERROR] The flash row is protected and can not be programmed [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_APP:
      printf("[ERROR] The application is not valid and cannot be set as active [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_ACTIVE:
      printf("[ERROR] The application is currently marked as active [0x%X]\n",error);
      break;
    case CYBTLDR_STAT_ERR_UNK:
    default:
      printf("[ERROR] An unknown error occured [0x%X]\n",error);
      break;
    }
}

void parseCyacdData(char *data, int *lines, char *stringImage[],int len)
    {
    char *p=data;
    int idx=0;

    while(p<data+len)
	{
	stringImage[idx++]=p;
	while ((p<data+len) && *p!='\r')
	    p++;
	*p++=0;
	if (*p=='\n')
	    *p++=0;
	}

    *lines= idx;
    }
char *stringImage[300];
int  program_psoc(char *data, int len, int psoc)
{


  int error;
  int lines;
  static int rlines=0;

 // stringImage = (char ***)malloc(sizeof(char**));
  //readCyacd(argv[optind],&lines,stringImage);
  //if (rlines==0)
      parseCyacdData(data,&rlines,stringImage,len);

  lines = rlines;
  comm1.OpenConnection = &OpenConnection;
  comm1.CloseConnection = &CloseConnection;
  comm1.ReadData = &ReadData;
  comm1.WriteData =&WriteData;
  comm1.MaxTransferSize =64;

  /* Bootloadable Blinking LED.cyacd */
  // error = BootloadStringImage(stringImage,LINE_CNT);
  error = BootloadStringImage(stringImage,lines);
  // error = BootloadStringImage(stringImage_6,LINE_CNT_6);

  if(error == CYRET_SUCCESS)
    {
      printf("[INFO] Bootloader operation succesful\n");
    } else
    {
      error_info_bootldr(error);
    }
  return error;
}

/****************************************************************************************************
 * Function Name: BootloadStringImage
 *****************************************************************************************************
 *
 * Summary:
 *  Bootloads the .cyacd file contents which is stored as string array
 *
 * Parameters:
 * bootloadImagePtr - Pointer to the string array
 * lineCount - No. of lines in the .cyacd file(No: of rows in the string array)
 *
 * Return:
 *  Returns a flag to indicate whether the bootload operation was successful or not
 *
 *
 ****************************************************************************************************/
uint16 BootloadStringImage(const char *bootloadImagePtr[],unsigned int lineCount )
{
  uint16 err=0;
  unsigned char arrayId;
  unsigned short rowNum;
  unsigned short rowSize;
  unsigned char checksum ;
  unsigned char checksum2;
  unsigned long blVer=0;
  /* rowData buffer size should be equal to the length of data to be sent for
   *  each flash row. It equals 288 , if ECC  is disabled in the bootloadable project,
   *  else 255 (in the case of PSoC 4 the flash row size is 128) */
  unsigned char rowData[288];
  unsigned int lineLen;
  unsigned long  siliconID;
  unsigned char siliconRev;
  unsigned char packetChkSumType;
  unsigned int lineCntr ;
	
  /* Initialize line counter */
  lineCntr = 0;

  /* Get length of the first line in cyacd file*/
  lineLen = strlen(bootloadImagePtr[lineCntr]);

  /* Parse the first line(header) of cyacd file to extract
     siliconID, siliconRev and packetChkSumType */
  err = CyBtldr_ParseHeader(lineLen ,(unsigned char *)bootloadImagePtr[lineCntr] , &siliconID , &siliconRev ,&packetChkSumType);

  /* Set the packet checksum type for communicating with bootloader.
     The packet checksum type to be used is determined from the
     cyacd file header information */
  CyBtldr_SetCheckSumType((CyBtldr_ChecksumType)packetChkSumType);

  if(err==CYRET_SUCCESS)
    {
      /* Start Bootloader operation */
    //  printf("Starting bootload\n");
      err = CyBtldr_StartBootloadOperation(&comm1 ,siliconID, siliconRev ,&blVer);
    //  printf("Starting bootload err %d\n",err);
      lineCntr++ ;
      while((err == CYRET_SUCCESS)&& ( lineCntr <  lineCount ))
	{

	  /* Get the string length for the line*/
	  lineLen =  strlen(bootloadImagePtr[lineCntr]);

	  /*Parse row data*/
	  err = CyBtldr_ParseRowData((unsigned int)lineLen,(unsigned char *)bootloadImagePtr[lineCntr], &arrayId, &rowNum, rowData, &rowSize, &checksum);
	  if (err)
	      printf(" parse err %d\n",err);

	  if (CYRET_SUCCESS == err)
            {
	      /* Program Row */
	      err = CyBtldr_ProgramRow(arrayId, rowNum, rowData, rowSize);
	   //   printf("%x %x %x\n",arrayId,rowNum,rowSize);
	      if (err!=0)
		 printf(" prog err %d\n",err);
	      if (CYRET_SUCCESS == err)
		{
		  /* Verify Row . Check whether the checksum received from bootloader matches
		   * the expected row checksum stored in cyacd file*/
		  checksum2 = (unsigned char)(checksum + arrayId + rowNum + (rowNum >> 8) + rowSize + (rowSize >> 8));
		  err = CyBtldr_VerifyRow(arrayId, rowNum, checksum2);
		  if (err!=0)
		      printf(" verr err %d\n",err);
		}
            }
	  /* Increment the linCntr */
	  lineCntr++;
	}
      /* End Bootloader Operation */
      CyBtldr_EndBootloadOperation();
    }
  return(err);
}
