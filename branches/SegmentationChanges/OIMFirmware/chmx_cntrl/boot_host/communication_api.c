/*******************************************************************************
* File Name: communication_api.c  
* Version 1.0
*
* Description:
* This file is created by the author . This contains definitions of APIs 
* used in structure 'CyBtldr_CommunicationsData' defined in cybtldr_api.h ,
* using I2C commuincations component 
********************************************************************************/

#include "communication_api.h"
#include <stdlib.h>

#include "sys/alt_stdio.h"
#include "system.h"
#include "altera_avalon_pio_regs.h"
#include "../i2c_opencores.h"
#include "../fixed_brd.h"
#include "unistd.h"
/* MicroC/OS-II definitions */
#include "includes.h"

unsigned long g_prog_base = I2C_OPENCORES_2_BASE;
int           g_prog_address = FIXED_ID;
int           g_prog_busy_val = 0xa2;

/*******************************************************************************
* Function Name: OpenConnection
********************************************************************************
*
* Summary:
*  Initializes the communications component : In this case I2C
*
* Parameters:  
*  void
*
* Return: 
*  Returns a flag to indicate whether the operation was successful or not
*
*
*******************************************************************************/
int OpenConnection(void)
{
	
	
	return(CYRET_SUCCESS);
}


/*******************************************************************************
* Function Name: CloseConnection
********************************************************************************
*
* Summary:
*  Clears the status and stops the communications component(I2C).
*
* Parameters:  
*  void
*
* Return: 
*  Returns a flag to indicate whether the operation was successful or not
*
*
*******************************************************************************/
int CloseConnection(void)
{
	/* Clear any previous status */

	return(CYRET_SUCCESS);
}

/*******************************************************************************
* Function Name: WriteData
********************************************************************************
*
* Summary:
*  Writes the specified number of bytes usign the communications component(I2C)
*
* Parameters:  
*  wrData - Pointer to write data buffer
*  byteCnt - No. of bytes to be written 
*
* Return: 
*  Returns a flag to indicate whether the operation was successful or not
*
*
*******************************************************************************/
int WriteData(uint8* wrData, int byteCnt)
{
    int x;
    if (I2C_ACK!=I2C_start(g_prog_base,g_prog_address,0))
	 {
		    printf("got nack\n");
		    return -1;
		    }
	for (x=0;x<byteCnt-1;x++)
		I2C_write(g_prog_base,wrData[x],0);  // write to register 3 command; no stop

	I2C_write(g_prog_base,wrData[x],1);  // write to register 3 command; no stop
	


	alt_busy_sleep(3000);
#if 0
	printf("wrote ");
	for (x=0;x<byteCnt;x++)
		    printf("%x ",wrData[x]);
		printf("\n");
#endif
	return(CYRET_SUCCESS);
	
}


/*******************************************************************************
* Function Name: ReadData
********************************************************************************
*
* Summary:
*  Reads the specified number of bytes usign the communications component(I2C)
*
* Parameters:  
*  rdData - Pointer to read data buffer
*  byteCnt - No. of bytes to be read 
*
* Return: 
*  Returns a flag to indicate whether the operation was successful or not
*
*
*******************************************************************************/
int ReadData(uint8* rdData, int byteCnt)
{
	int x;
	int retry;
for (retry=0; retry<100;retry++)
    {
	if (I2C_ACK!=I2C_start(g_prog_base,g_prog_address,1))
	    {
	    printf("got nack\n");
	    return -1;
	    }
	for (x=0;x<byteCnt-1;x++)
	    rdData[x]=I2C_read(g_prog_base,0);

	rdData[x]=I2C_read(g_prog_base,1);
#if 0
	printf("Read ");
	for (x=0;x<byteCnt;x++)
	    printf("%x ",rdData[x]);
	printf("\n");
#endif
	if (rdData[0]!=g_prog_busy_val)
	    return(CYRET_SUCCESS);
	alt_busy_sleep(2000);
    }
printf ("Retry error\n");
  return -1;
}

/* [] END OF FILE */

