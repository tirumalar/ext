/* ========================================
 *
 * Copyright YOUR COMPANY, THE YEAR
 * All Rights Reserved
 * UNPUBLISHED, LICENSED SOFTWARE.
 *
 * CONFIDENTIAL AND PROPRIETARY INFORMATION
 * WHICH IS THE PROPERTY OF your company.
 *
 * ========================================
*/
#include <project.h>
#include "toc.h"
#include "osdp.h"
#include "wavelynx.h"

uint8 tlv_ready;
uint8 readerbootloading;




void ReadImageCard()
{
    int k;
    if(tlv_ready == 1)
    {
        tlv_ready = 0;
        if (rxi == 0) return; //wrong size
        if (rxi <= IMAGE_TEMP_SIZE )
        {
            for (k=0; k<rxi; k++)
            {
                MyI2C_Regs.buffer[k] = ReaderRxBuffer[k];
                CyDelayUs(1);   //needed for Release Codes
            }
            rxi = 0;  //ready for next image
            MyI2C_Regs.data_length[0]= k>>8;   //higher 8 bits
            MyI2C_Regs.data_length[1]= k;        //lower 8 bits.
            MyI2C_Regs.dummy0[0]= (k>>8) & 0xFF;   //higher 8 bits
            MyI2C_Regs.dummy0[1]= k & 0xFF;        //lower 8 bits.
            card_ack = 0;
            card_in_buf = 1;
            MyI2C_Regs.status_out |= STAT_CHANGE | STAT_CARD_IN;
            MyI2C_Regs.cmd = CMD_NONE; //prevent to read it again
            MyI2C_Regs.buffer1[10] ++;
        }
        else
        {
            rxi = 0; //data is too big, read again
        }
    } //if
}


void initTocReaderUART()
{
    rxi = 0;
    isr_READER_rx_SetVector(ImgRxInt);
    isr_READER_rx_StartEx(ImgRxInt);
    SetReaderSpeed(BAUD_460800);
    UART_READER_Start();
    UART_READER_LoadRxConfig();  //set to receiving mode
    CR_DE_Write(0);
    Mode_Sel_CR_Write(1); //set the chip to RS485 mode
    tlv_ready = 0;
}

unsigned char	ReaderRxBuffer[IMAGE_BUFFER_SIZE];     // Rx circular buffer to hold all incoming command
int rxi; //received index
int tlv_head_len;
int tlv_data_len;
int tlv_len;
unsigned char multi_byte_len;
unsigned char tocTag;
CY_ISR(ImgRxInt) //interrupt on Rx byte received
{   
    unsigned char uch;
    //move all available characters from Rx queue to RxBuffer
    while(UART_READER_ReadRxStatus() & UART_READER_RX_STS_FIFO_NOTEMPTY)
	{
        uch = UART_READER_ReadRxData(); //get serial port
        if (rxi < IMAGE_TEMP_SIZE) ReaderRxBuffer[rxi] = uch; //put into the buffer
        if (rxi == 0){
            tocTag = ReaderRxBuffer[0];
            switch (tocTag)
            {
                case SP_ENVELOPE:
                case SP_ENVELOPE_CRC:
                case RSP_MESSAGE:
                case DATA_DEVICE_INFO:
                case DATA_EYELOCK_TEMPLATE:
                case DATA_PACKET:
                case CMD_PING:
                break;
//                default: return; //comment off for bad respond of bootloading
                default: return;
            }

        }
        /*  If the Length field is greater than or equal to 128 bytes, 
            then a multi byte format is needed to represent the Length of the Value field. 
            In this case the most significant bit of the first byte in the Length field
            is always set to 1. 
            Length = 8180 indicates a length of 128 (0x80) bytes
            Length = 820200 indicates a length of 512 (0x0200) bytes.
            Length = 8303C102 indicates a length of 246,018 (0x03C102) bytes  */
        if (rxi == 1){
            if (ReaderRxBuffer[rxi] & 0x80){ //check if the len is more than 127
                multi_byte_len = 1;
                tlv_data_len = ReaderRxBuffer[rxi] & 0x7F; //get the tlv data length
                tlv_len = 0;
                tlv_head_len = 2 + tlv_data_len; //head length = tag_id + length field length
                rxi++;
                return;
            }else{
                multi_byte_len = 0;
                tlv_len = ReaderRxBuffer[rxi] & 0x7F; //get the tlv data length
                tlv_len = 2 + tlv_len; //length = tag_id + length field length
                rxi++;
                return;
            }
        }
        if (multi_byte_len == 1 && tlv_data_len > 0 && rxi > 1){        //get the package length
            tlv_len = (tlv_len << 8) + ReaderRxBuffer[rxi]; //2 bytes
            tlv_data_len--;                      //remaining counter
            if (tlv_data_len == 0){              //no more length bytes
                tlv_len += tlv_head_len;
                if ( tlv_len == 0)             //package is too short 1
                {
                    rxi = 0; //bad data, read again.
                    return;
                }
            }
        }
        if (rxi <= tlv_len && rxi <= IMAGE_TEMP_SIZE)
        {
            rxi++;
        }
		if (tlv_len !=0 && (rxi >= tlv_len || rxi >= IMAGE_TEMP_SIZE))
        {
            tlv_ready = 1;
        }
	}   
}

void resetImgBuf()
{
    rxi = 0; //clear old card after ack
    tlv_ready = 0; //ready for new cards
    card_in_buf = 0; //release the current card
    tlv_len = 0; //reset last card length
}

void SendWaveLynxReader(unsigned char *buffer, int len)
{
//    CyDelay(50); //add some delay before sending 40. 
//    Mode_Sel_CR_Write(1); //set the SP335e chip to RS485
    CR_DE_Write(1);
    UART_READER_ClearTxBuffer();
    UART_READER_LoadTxConfig();
    CyDelayUs(20); //20us Driver Output Enable Time, do not reduce. 
    UART_READER_PutArray(buffer,len); 
    CyDelayUs(120); //minmum 4ms 120us
    UART_READER_LoadRxConfig();  //set to receiving mode
    CR_DE_Write(0);
//    Mode_Sel_CR_Write(1); //set the chip to RS485 mode
    resetImgBuf();  //clear everything before receive the reply
    UART_READER_ClearRxBuffer();
}

/**********************************************************************
*
*  Function Name: CalculateCrc16
*
*  Inputs:
*    pData – Points to the data to perform the CRC on
*    numBytes – Number of valid bytes in pData[].
*    startValue – Initialization value (always 0x6363)
*
*  Outputs
*    None
*
*  Return Values
*    The 16 bit value representing the CCITT CRC resulting from 
*    performing the algorithm on the data in pData[].
*
*  Description:  This function performs a 16 bit CCITT CRC on the data 
*                pointed to by pData.
*
**********************************************************************/
uint16_t CalculateCrc16(uint8_t *pData, uint16_t numBytes, uint16_t startValue)
{
uint16_t i;
	uint16_t crcVal;

    	crcVal = startValue;
	for (i=0; i<numBytes; i++)
	{
        crcVal = UpdateCrc16Ccitt(crcVal, pData[i]);
	}
	return (crcVal);
}

uint16_t UpdateCrc16Ccitt (uint16_t crc, uint8_t data)
{
    data ^= (crc & 0xFF);
    data ^= data << 4;

    return ((((uint16_t)data << 8) | ((crc >> 8) & 0xFF )) ^ (uint8_t)(data >> 4) 
          ^ ((uint16_t)data << 3));
}

/* [] END OF FILE */
