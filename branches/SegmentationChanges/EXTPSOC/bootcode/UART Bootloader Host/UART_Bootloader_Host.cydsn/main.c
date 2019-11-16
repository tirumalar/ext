/*  -------------------------------------------------------------------------------------------------
* Copyright 2016, Cypress Semiconductor Corporation.
*
* This software is owned by Cypress Semiconductor Corporation (Cypress)
* and is protected by and subject to worldwide patent protection (United
* States and foreign), United States copyright laws and international
* treaty provisions. Cypress hereby grants to licensee a personal,
* non-exclusive, non-transferable license to copy, use, modify, create
* derivative works of, and compile the Cypress Source Code and derivative
* works for the sole purpose of creating custom software in support of
* licensee product to be used only in conjunction with a Cypress integrated
* circuit as specified in the applicable agreement. Any reproduction,
* modification, translation, compilation, or representation of this
* software except as specified above is prohibited without the express
* written permission of Cypress.
* 
* Disclaimer: CYPRESS MAKES NO WARRANTY OF ANY KIND,EXPRESS OR IMPLIED,
* WITH REGARD TO THIS MATERIAL, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
* Cypress reserves the right to make changes without further notice to the
* materials described herein. Cypress does not assume any liability arising
* out of the application or use of any product or circuit described herein.
* Cypress does not authorize its products for use as critical components in
* life-support systems where a malfunction or failure may reasonably be
* expected to result in significant injury to the user. The inclusion of
* Cypress' product in a life-support systems application implies that the
* manufacturer assumes all risk of such use and in doing so indemnifies
* Cypress against all charges.
* 
* Use may be limited by and subject to the applicable Cypress software
* license agreement.
* ---------------------------------------------------------------------------------------------------
* Copyright (c) Cypress Semiconductors 2000-2015. All Rights Reserved.
*
*****************************************************************************************************
*  Project Name: UART_Bootloader_Host_PSoC5LP
*  Project Revision: 1.00
*  Software Version: PSoC Creator 3.3 SP2
*  Device Tested: CY8C5868AXI-LP035
*  Compilers Tested: ARM GCC
*  Related Hardware: CY8CKIT-050
*****************************************************************************************************
****************************************************************************************************/

/****************************************************************************************************
* Project Description:
* This is a sample bootloader host program demonstrating PSoC5LP bootloading
* PSoC 3 or PSoC 4 or PSoC Analog Coprocessor or PSoC 5LP. The project is tested using CY8CKIT-050 with
* PSoC 5LP chip and CY8CKIT-030 (CY8CKIT-042) with PSoC 3 (PSoC 4) chip.
* PSoC 3 (PSoC 4, PSoC Analog Coprocessor) must be programmed with the UART Bootloader Program attached
* with the app note.
*
* Connections Required
* CY8CKIT-050 (PSoC 5LP DVK) :
*  P0[0] - Rx -  connected to Tx of PSoC3
*  P0[1] - Tx -  connected to Rx of PSoC3 
*  P6.1 is internally connected to SW2 on DVK.
*
* CY8CKIT-030 (PSoC 3 DVK) : PSoC  3 intially programmed with 
* UART_Bootloader program.
*  P0[0] - Rx - Connected to Tx of PSoC 5LP
*  P0[1] - Tx - Connected to Rx of PSoC 5LP
*  P6.1 is internally connected to SW2 on DVK.
*
* If you are using a PSoC 4 as your target, the connections are as follows:
* CY8CKIT-042 (PSoC 4 DVK) : PSoC  4 intially programmed with 
* UART_Bootloader (PSoC 4 project) program.
*  P0[0] - Rx - Connected to Tx of PSoC 5LP
*  P0[1] - Tx - Connected to Rx of PSoC 5LP
*  P0.7 is internally connected to SW2 on DVK.
*
*If you are using PSoC 5LP as your target, the connections on the DVK 
* remain same as that for CY8CKIT-030 (PSoC 3).
*
* Note that the GNDs of both DVKs should be connected together.
*
* Bootload function is defined in main.c: BootloadStringImage which uses 
* Bootloader Host APIs to bootload the contents of .cyacd file.
*
* BootloadStringImage function requires each line of .cyacd to be stored 
* as a seperate string in a string array. The total number of lines in 
* the file should also be manually calculated and stored as #define LINE_CNT
* The string image of .cyacd file is stored in StringImage.h file.
*
* The following events happens alternatively on each switch press
* On first switch press it will bootload 'stringImage_1' stored in StringImage.h
* using BootloadStringImage function . 
* On next switch press it will bootload 'stringImage_2' stored in StringImage.h
* using BootloadStringImage function .
*
* These bootloadable images display either "Hello" or "Bye" on the first row of 
* LCD(PSoC 3 DVK). In the case of PSoC 4, they blink Green and Blue LEDs.
* Note that in order to enter the bootloader from the application program 
* P6.1 - connected to SW2 (in the case of PSoC 4, P0.7 connected to SW2 on
* CY8CKIT-030) should be pressed so that the application program running on
* PSoC 3 (PSoC 4) enters Bootloader and is ready to bootload a new application
* code/data . 
*
****************************************************************************************************/

#include <device.h>
#include "string.h"
#include <cybtldr_parse.h>
#include <cybtldr_command.h>
#include <communication_api.h>
#include <cybtldr_api.h>
#include "StringImage.h"

/* Macros to choose among PSoC 3, PSoC 4 and PSoC 5LP as a target device */
#define PSoC_3       0
#define PSoC_5LP     1
#define PSoC_4		 2
#define PSoC_AC      2
/* If your target device is PSoC 4 or PSoC 5LP, change the value of this Macro
   to "PSoC_4" or "PSoC_5LP" respectively */
#define TARGET_DEVICE PSoC_4

/* This function bootloads the .cyacd file. It sends command packets and 
   flash data to the target. Based on the response from the target, it decides
   whether to continue bootloading or end it. */
uint16 BootloadStringImage(const char *bootloadImagePtr[],unsigned int lineCount );

/* This structure contains function pointers to the four communication layer 
   functions contained in the communication_api.c / .h */
CyBtldr_CommunicationsData comm1 ;

/* toggle alternates between the two bootloadable files 
   (Hello and Bye) of (Green and Blue) */
uint8 toggle = 0;


int main()
{
    /* error holds the success / failure message of the bootload operation */
	uint16 error = 0;

	/* Display the instruction to the user on how to start bootloading */
	LCD_Char_Start();	
	LCD_Char_Position(1,0);
	LCD_Char_PrintString("P6.1 to BL");
	
	/* Enable all interrupts */
	CYGlobalIntEnable;

	/* Initialize the communication structure element -comm1 */
	comm1.OpenConnection = &OpenConnection;
	comm1.CloseConnection = &CloseConnection;
	comm1.ReadData = &ReadData;
	comm1.WriteData =&WriteData;
	comm1.MaxTransferSize =64;
        
    for(;;)
    {
	    /* If Switch is pressed and if toggle =0,the image to be bootloaded is Hello */
		if(Pin_Switch_Read() == 0)
		{
			LCD_Char_Position(0,0);
			LCD_Char_PrintString("Bootloading...  ");
			
                 
			/* Select the Bootloadable files based on the target device */
            if(TARGET_DEVICE == PSoC_3)
            {	
				
                 /* Alternate between the two bootloadable files 
				    (Hello and Bye) based on toggle */    
                if (toggle == 0)
                {   
                    /* Bootload the Bootloadable_1.cyacd (Hello) */
                    error = BootloadStringImage(stringImage_1,LINE_CNT_1);
                }
                 else if(toggle == 1)
                {
                     /* Bootload the Bootloadable_2.cyacd (Bye) */
                     error = BootloadStringImage(stringImage_2,LINE_CNT_2);
                }
            }
            else if(TARGET_DEVICE == PSoC_5LP)
            {
				
                 /* Alternate between the two bootloadable files 
				    (Hello and Bye) based on toggle */    
                if (toggle == 0)
                {   
                     /* Bootload the Bootloadable_1.cyacd (Hello) */
                    error = BootloadStringImage(stringImage_3,LINE_CNT_3);
                }
                 else if(toggle == 1)
                {
                     /* Bootload the Bootloadable_2.cyacd (Bye) */
                     error = BootloadStringImage(stringImage_4,LINE_CNT_4);
                }
				
            }
			else if(TARGET_DEVICE == PSoC_4)
            {
				
                 /* Alternate between the two bootloadable files 
				    (Green and Blue) based on toggle */    
                if (toggle == 0)
                {   
                     /* Bootload the Bootloadable_1.cyacd (Green LED) */
                    error = BootloadStringImage(stringImage_5,LINE_CNT_5);
                }
                else if(toggle == 1)
                {
                     /* Bootload the Bootloadable_2.cyacd (Blue LED) */
                     error = BootloadStringImage(stringImage_6,LINE_CNT_6);
                }
				
            }
            else
            {
                 /* Do nothing if the target device is none of 
				    PSoC 3, PSoC 4 or PSoC 5LP */
            }
             	 
			/* Check if the bootload operation is successful */
            if(error == CYRET_SUCCESS)
            {
	            LCD_Char_Position(0,0);
				
                /* Display the success message */
				if(TARGET_DEVICE == PSoC_4)
				{
					if(toggle == 0)
	                {
	                    LCD_Char_PrintString("Bootloaded-Green");
	                    LCD_Char_Position(1,0);
	                    LCD_Char_PrintString("P6.1 to BL Blue ");
						toggle = 1;
	                }
	                else
	                {
	                    LCD_Char_PrintString("Bootloaded-Blue");
	                    LCD_Char_Position(1,0);
	                    LCD_Char_PrintString("P6.1 to BL Green");
	                    toggle = 0;
	                }
				}
				else
				{
	                if(toggle == 0)
	                {
	                    LCD_Char_PrintString("Bootloaded-Hello");
	                    LCD_Char_Position(1,0);
	                    LCD_Char_PrintString("P6.1 to BL Bye  ");
						toggle = 1;
	                }
	                else
	                {
	                    LCD_Char_PrintString("Bootloaded-Bye");
	                    LCD_Char_Position(1,0);
	                    LCD_Char_PrintString("P6.1 to BL Hello  ");
	                    toggle = 0;
	                }
				}
            }
            else 
            {
                /* Display the failure message along with the approiate error code */
                if(error & CYRET_ERR_COMM_MASK) /* Check for comm error*/
                {
                    LCD_Char_Position(0,0);
                    LCD_Char_PrintString("Communicatn Err ");
                }
                else /* Else Display the bootload error code */
                {
                    LCD_Char_Position(0,0);
                    LCD_Char_PrintString("Bootload Err :");
                    LCD_Char_PrintHexUint8(error);
					CyDelay(4000);
                }
            }   
        }
        else 
        {
		
            /* Do Nothing */
        }
	}
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
		err = CyBtldr_StartBootloadOperation(&comm1 ,siliconID, siliconRev ,&blVer);
		lineCntr++ ;
		while((err == CYRET_SUCCESS)&& ( lineCntr <  lineCount ))
		{
		
            /* Get the string length for the line*/
			lineLen =  strlen(bootloadImagePtr[lineCntr]);
			
			/*Parse row data*/
			err = CyBtldr_ParseRowData((unsigned int)lineLen,(unsigned char *)bootloadImagePtr[lineCntr], &arrayId, &rowNum, rowData, &rowSize, &checksum);
          	
			
			if (CYRET_SUCCESS == err)
            {
				/* Program Row */
				err = CyBtldr_ProgramRow(arrayId, rowNum, rowData, rowSize);
				
	            if (CYRET_SUCCESS == err)
				{
					/* Verify Row . Check whether the checksum received from bootloader matches
					* the expected row checksum stored in cyacd file*/
					checksum2 = (unsigned char)(checksum + arrayId + rowNum + (rowNum >> 8) + rowSize + (rowSize >> 8));
					err = CyBtldr_VerifyRow(arrayId, rowNum, checksum2);
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


/* [] END OF FILE */
