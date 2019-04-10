/*******************************************************************************
* File Name: communication_api.c  
* Version 1.0
*
* Description:
* This file is created by the author . This contains definitions of APIs 
* used in structure 'CyBtldr_CommunicationsData' defined in cybtldr_api.h ,
* using SPI commuincations component 
********************************************************************************/

#include "communication_api.h"
#include <device.h>

/*******************************************************************************
* Function Name: OpenConnection
********************************************************************************
*
* Summary:
*  Initializes the communications component : In this case UART
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
	UART_Start();
	return(CYRET_SUCCESS);
}


/*******************************************************************************
* Function Name: CloseConnection
********************************************************************************
*
* Summary:
*  Clears the status and stops the communications component (UART).
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
	UART_Stop();
	return(CYRET_SUCCESS);
}

/*******************************************************************************
* Function Name: WriteData
********************************************************************************
*
* Summary:
*  Writes the specified number of bytes using the communications component (UART)
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
	uint16 timeOut =1;
		
	/* Clears TX and RX FIFOs and the status registers */
	UART_ClearRxBuffer();
	UART_ClearTxBuffer();
	
	/* Send the data*/   
	UART_PutArray(wrData, byteCnt);
	
 	/* Wait till send operation is complete or timeout  */
	while(!(UART_ReadTxStatus() & UART_TX_STS_COMPLETE))
	{
		timeOut++;
		/* Check for timeout and if so exit with communication error code*/
		if(timeOut == 0)
		{
			return(CYRET_ERR_COMM_MASK);
		}	
	}
				
	return(CYRET_SUCCESS);
}


/*******************************************************************************
* Function Name: ReadData
********************************************************************************
*
* Summary:
*  Reads the specified number of bytes usign the communications component (UART)
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
	uint16 timeOut =1;
	uint8 dataIndexCntr = 0;
    
	
	/* Clears TX and RX FIFOs and the status registers */	
	UART_ClearRxBuffer();
	UART_ClearTxBuffer();
	
	/* Wait until timeout  */
	while(UART_GetRxBufferSize() == 0u)
	{
		timeOut++;
		/* Check for timeout and if so exit with communication error code*/
		if(timeOut == 0)
		{			
			return(CYRET_ERR_COMM_MASK);
		}	
	}
	
    
	/* Read the data bytes */	
	while (byteCnt>0)
	{
		if(UART_GetRxBufferSize() > 0u)
		{
		rdData[dataIndexCntr]=UART_GetChar();
		dataIndexCntr++;
		byteCnt--;
		}
	}
	
	return(CYRET_SUCCESS);
}

/* [] END OF FILE */

