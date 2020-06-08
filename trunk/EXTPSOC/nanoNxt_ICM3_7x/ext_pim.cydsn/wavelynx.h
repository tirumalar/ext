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
//4.1.1	Command Tag List
#define CMD_PING                    0x20
#define CMD_AV_CONTROL              0x30
#define CMD_GET_DEVICE_INFO         0x31
#define CMD_CLEAR_DATA              0x32
#define CMD_GET_DATA                0x33
#define CMD_PUT_DATA                0x34
#define CMD_READ_CRED               0x35
#define CMD_WRITE_CRED		        0x36
#define CMD_WRITE_BLE_CRED          0x3B
#define CMD_POLL_BLE_EVENTS         0xB0
#define CMD_SET_BLE_MODE            0xB1
#define CMD_SET_BLE_TIMEOUT         0xB2
#define CMD_PROCESS_EM_DATA	        0x6A
#define CMD_CLEAR_EM_DATA		    0x6B
#define CMD_PUT_EM_DATA		        0x6E


//4.1.2	Data Tag List
#define DATA_DEVICE_INFO		    0x38
#define DATA_EYELOCK_TEMPLATE	    0x39
#define DATA_PACKET			        0x3A

//4.1.3	Response Tag List
#define RSP_MESSAGE			        0x5C

//4.1.4	Special Tag List
#define SP_ENVELOPE_CRC		        0x0C
#define SP_ENVELOPE			        0x0E

//Below is a list of values that are used with this TLV tag.
#define VAL_SUCCESS			        0x00
#define VAL_FAILED			        0x01
#define VAL_TIMEOUT			        0x02
#define VAL_CARD_NOT_FOUND		    0x03
#define VAL_APP_NOT_FOUND		    0x04
#define VAL_AUTH_ERROR		        0x05
#define VAL_OUT_OF_RANGE		    0x06
#define VAL_INVALID_DATA		    0x07

/*
Note: The 16 bit Operational Configuration word shown in bytes 6 and 7 of the TLV Value field above is structured as follows:

Maximum Allowed RF Baud Rate:	 (BIT0 | BIT1)
	(Internal Use)				    BIT 2
	125 KHz Credential Support		BIT 3
	13.56 MHz Credential Support	BIT 4
	BLE Support				        BIT 5
	Wiegand Interface Support		BIT 6
	RS485 Interface Support		    BIT 7
	Unused, RFU				        BITS 8 through 15
    
Example:	T   L   V
38 08 01 00 09 01 02 01 00 B0  
Firmware Major Release:		1
Firmware Minor Release		0
Firmware Build			9
Hardware Major Release		1
Hardware Minor Release		2
Hardware Build			1
Operational Configuration:		0x00B0

Operational Configuration Breakdown:
Maximum Allowed RF Baudrate:	106 Kbits/Second
125 KHz Credentials:			Not supported
13.56 MHz Credentials:		Supported
Bluetooth Low Energy:		Supported
Wiegand Interface:			Not Supported
RS485 Interface:			Supported

*/
    
/*
4.2.15	 CMD_ SET_BLE_MODE	(0xB1)
Description:
Command sent by the HC to set the mode of the Reader’s BLE interface. The Reader’s BLE interface will support the following modes:

Passthrough: No mobile app interaction required to send template.
Button Press: A button must be pressed on the mobile app to send template.
Pin Entry: A unique PIN must be entered and confirmed on the app to send template.

The TLV values corresponding to each mode are as follows:
Passthrough:  0x4E
Button Press: 0x42
Pin Entry:      0x50

The expected response to this command is a RSP_MESSAGE with a value indicating success indicating that the command was received.

Send Usage:		HC Only
Receive Usage:	Reader Only 

Examples:		T   L   V
Passthrough
HC sends:		B1 01 4E
Reader Responds:	5C 01 00

Button Press
HC sends:		B1 01 42
Reader Responds:	5C 01 00

Pin Entry
HC sends:		B1 01 50
Reader Responds:	5C 01 00
*/

/*
4.2.16	 CMD_ SET_TIMEOUT	(0xB2)
		Description:
The command used to set the duration which the mobile app will not recognize or reconnect to the same reader again.

The value of the timeout will be a 32bit(4 byte) unsigned integer representing the timeout duration in milliseconds. This integer will be sent in Big Endian format (starting with the most significant byte).

The expected response to this command is a RSP_MESSAGE with a value indicating success indicating that the command was received.

Send Usage:		HC Only
Receive Usage:	Reader Only 

Examples:		T   L   V
		60,000 millisecond (1 minute) delay:
HC Sends:                  B2 04 00 00 EA 60
Reader Responds:      5C 01 00
*/

 uint16_t CalculateCrc16(uint8_t *pData, uint16_t numBytes, uint16_t startValue) ;  
/* [] END OF FILE */
