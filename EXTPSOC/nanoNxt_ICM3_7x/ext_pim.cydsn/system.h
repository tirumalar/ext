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
#ifndef __SYSTEM_INCLUDED__   
#define __SYSTEM_INCLUDED__  

// GRI 1-13-2015 - Revised for 3.1.1; TAMPERCR_POL name changed to indicate polarity setting for CR Tamper in webconfig. 

#include "softuart.h"
#include "wavelynx.h"
    
#define BUFFER_LENGTH 20
#define WIEGAND_BUFFER_LENGTH 30  //200 bits = 25 bytes
#define OSDP_PANEL_BUFFER_LENGTH 128
#define OSDP_READER_BUFFER_LENGTH 128
#define IMAGE_TEMP_SIZE 5364
#define IMAGE_BUFFER_SIZE 6000
extern unsigned char ReaderRxBuffer[IMAGE_BUFFER_SIZE];
    
/* I2C Regs Starts at 0x38 */

typedef struct I2C_Regs {           // I2C interface structure
    char status_out;                //0 status out to nxt
    char status_in;                 //1 status in from nxt
    char cmd;                       //2 commands from motherboard
    unsigned char mode;             //3 ACS mode type
    char acs_outputs;               //4 read by motherboard
    char acs_inputs;                //5 set by motherboard
    char factory_reset;             //6 0x42
    unsigned char data_length[2];   //7
    unsigned char bits;             //9 Wiegand bits 1A=26
    unsigned char tamper_in;        //10
    unsigned char pushbtn_in;       //11
    unsigned char mobile_mode;      //12
    unsigned char send_mobile;      //13
    unsigned char dummy0[53];       //14
    char sw_version[3];             //67 0x43
    char hw_version[3];             //70 0x46
    unsigned char buffer1[BUFFER_LENGTH];    //73
    unsigned char osdp_cmd;         //93 0x49
    unsigned char osdp_status;      //94 0x4A
    unsigned char osdp_data_length; //95 0x4B
    unsigned char osdp_rate;        //96 0x4C
    unsigned char osdp_buffer[OSDP_PANEL_BUFFER_LENGTH];  //97 0x4D
    unsigned char osdp_reader_cmd;  //225 0xCD
    unsigned char osdp_reader_data_length; //226
    unsigned char osdp_reader_rate; //227
    unsigned char osdp_reader_buffer[OSDP_READER_BUFFER_LENGTH];//228 0xD0
    unsigned char buffer[100];    //73
    unsigned char dummy1[BUFFER_LENGTH];
}I2C_Regs;

extern I2C_Regs MyI2C_Regs;
SOFT_UART_STRUCT s;
extern unsigned char run_mode;

/* portable template */
extern int rxi;
extern char sendDebugMsg;
extern uint8 readerbootloading;

/* Auxillary IO variables */
char currentAccessType;
char hw_old;
char tp_old;
int tamperCR_polarity;
int tamper_out_polarity;

/* Dual auth variables */
int card_ack;   // added GRI 2/2/15
int card_in_buf;    // added GRI 2/3/15,m...

/* F2F card read variables */
extern int f2f_pulsewidth;
extern int f2f_timeout;
extern int osdp_timeout;

/* F2F reader acknowledgement controls */
extern char f2f_acknowledge_active;
extern char f2f_ack_delay_cnt;

/* F2F panel keep alive */
extern char f2f_send_alive;
extern int f2f_alive_delay_cnt;
extern int debug_msg_delay;

/* OSDP task flags */
extern unsigned char OSDPReaderTask;

/* Pass Mode task switching controls */
extern int passmode_timeout;  //counter

/* Button pressed */
extern char buttonPressed;
extern char buttonReleased;
extern long buttonDuration;

/*0 OUTPUT STATUS VALUES */
#define STAT_CHANGE     0x01
#define STAT_CARD_IN    0x02
#define STAT_ACS_IN     0x04 
#define STAT_TAMPER_IN  0x08


/*1 INPUT STATUS VALUES */
//#define STAT_CHANGE     0x01 //Fang said not used
#define STAT_ACS_OUT    0x02
#define STAT_OSDP_OUT   0x04  //not used
#define STAT_CARD_ACK   0x08
#define STAT_RELAY_1    0x10
#define STAT_RELAY_2    0x20

/*2 OLD COMMANDS
#define CMD_NONE            0
#define CMD_BOOTLOAD        1
#define CMD_READ_CARD       2
#define CMD_TRIGGER_RELAY1  3
#define CMD_TRIGGER_RELAY2  4
#define CMD_SEND_F2F        5
#define CMD_SEND_HID        6
#define CMD_SEND_PAC        12
#define CMD_SEND_WEIGAND    26
 */
// Command
#define CMD_NONE            0
#define CMD_BOOTLOAD        1
#define CMD_READ            2
#define CMD_SEND            3
#define CMD_SEND_TOC_READER 4
#define CMD_RTC_READ        5
#define CMD_RTC_WRITE       6
#define CMD_RGB             7
#define CMD_POE_ENABLE      8
#define CMD_POE_DISABLE      9

  
// Access Mode/Type/Protocol
#define MODE_NONE           0
#define MODE_RELAY          1
#define MODE_OSDP           2
#define MODE_HID            3
#define MODE_PAC            4
#define MODE_F2F            5
#define MODE_WIEGAND        6

#define BASE_SD             0  //single and dual mode
#define BASE_TOC            50
#define BASE_PASS           100
#define PIN_PASS            150

/*4 ACS OUTPUT VALUES */
#define LED_IN_RED          0x1
#define LED_IN_GRN          0x2
#define SOUNDER_IN          0x4
//#define TAMPERCR_IN         0x8
//#define REED_1_IN           0x10
//#define REED_2_IN           0x20
#define PWR_RESET           0x40

/*5 ACS INPUT VALUES */
#define LED_OUT_RED     0x1
#define LED_OUT_GRN     0x2
#define SOUNDER_OUT     0x4
#define TAMPERCR_POL    0x8  //Name changed to indicate polarity setting for CR Tamper in webconfig GRI 1/2015
#define RELAY_1_OUT     0x10
#define RELAY_2_OUT     0x20
#define TAMPER_OUT      0x40 //panel tamper output polarity

/*
10 Tamper_in
#define BOB_TAMPER_INPUT_OFFSET            10
Bit 1: reader tamper
Bit 2: REED-1
Bit 3: REED-2
*/
#define TAMPERCR_IN         0x1
#define REED_1_IN           0x2
#define REED_2_IN           0x4

/*
11 Button Status Change flag
Bit 1: status change
Bit 2: factory reset
Bit 3: factory restore
*/
#define BTN_STATUS_CHG      0x1
#define FCTRY_RST           0x2
#define FCTRY_RSTR          0x4



/* TAMPER OUT */
#define TAMPERHIGH 0
#define TAMPERLOW 1


/* INTERNAL DEFINE */
#define PAC 12
#define HID 26

/* LED */
#define ON  0
#define OFF 1


/* OSDP CMD and READER CMD*/
#define OSDP_NONE           0
#define OSDP_BUSY           1
#define OSDP_SEND           2
#define OSDP_READ           3
#define OSDP_RATE           4

/* OSDP STATUS */
#define OSDP_PANEL_DATA_SENT        0x01  //not used
#define OSDP_PANEL_DATA_IN_READY    0x02
#define OSDP_READER_DATA_SENT       0x10  //not used
#define OSDP_READER_DATA_IN_READY   0x20

/* OSDP RATE */
#define BAUD_9600   1
#define BAUD_19200  2
#define BAUD_38400  3
#define BAUD_115200 4
#define BAUD_230400 5
#define BAUD_460800 6
#define BAUD_921600 7

#define UART_EXT_CLK 48000000
/* BAUD */
#define CLKDV_9600 (UART_EXT_CLK/8/9600) //0x271
#define CLKDV_19200 (UART_EXT_CLK/8/19200) //0x138
#define CLKDV_38400 (UART_EXT_CLK/8/38400) //0x156
#define CLKDV_115200 (UART_EXT_CLK/8/115200) //0x34     //0x33 ICM
#define CLKDV_230400 (UART_EXT_CLK/8/230400)
#define CLKDV_460800 (UART_EXT_CLK/8/460800)
//#define CLKDV_460800 (UART_EXT_CLK/8/454545)
#define CLKDV_921600 (UART_EXT_CLK/8/921600)


/* OSDP task */
#define OSDP_READER_KEEP_ALIVE      1
#define OSDP_SEND_READER            2

/* Factory Reset */
#define FACRST_CODE         0xC6
#define RESTRE_CODE         0x99
#define FCTRYRST_TIME_MS    5000
#define RSTR_TIME_MS        45000

/*I2C Reg Address 

status_output       0           0H
status_input        1           1
command             2           2
data_length         3           3
acs_outputs         4           4
acs_inputs          5           5
buffer[20]          6           6
dummy               26          1A
sw_version[3]       67          43
hw_version[3]       70          46
osdp_cmd;           73          49
osdp_status;        74          4A
osdp_data_length;   75          4B
osdp_rate;          76          4C
osdp_buffer         77          4D
osdp_reader_cmd     205         CD
osdp_reader_data_length 206     CE
osdp_reader_rate    207         CF
osdp_reader_buffer  208         D0
                    336         150H

*/

// wiegand 
extern char isWiegandDataReady;
extern char isWiegandDataStart;
extern unsigned char wiegandBuffer[WIEGAND_BUFFER_LENGTH];
extern int wiegandBitCount;
extern int wiegandByteCount;
extern int testBitCount;

#endif
/* [] END OF FILE */

