/*
 * BobListener.h
 *
 *  Created on: Sept 4, 2014
 *      Author: fjia
 */

#ifndef BOBLISTENER_H_
#define BOBLISTENER_H_

// Bob defines

// New BoB I2C Address
#define NANO_BOB_I2C_ADDR				0x38
// ACS Address
#define BOB_STATUS_IN_OFFSET 			0
#define BOB_STATUS_OUT_OFFSET			1
#define BOB_COMMAND_OFFSET				2
#define BOB_ACS_TYPE_OFFSET				3
#define BOB_ACS_INPUT_OFFSET			4
#define BOB_ACS_OUTPUT_OFFSET			5
#define BOB_FACTORY_RESET_OFFSET		6	// Factory Reset
#define BOB_DATA_LENGTH_OFFSET 			7	// 2 bytes, high=48, low=49
#define BOB_ACS_TYPE_BITS_OFFSET 		9
#define BOB_TAMPER_INPUT_OFFSET			10
#define BOB_FACTORY_STATUS_OFFSET		11


// Version Address
#define BOB_SW_VERSION_OFFSET			67	// 3 bytes
#define BOB_HW_VERSION_OFFSET			70	// 3 bytes
#define BOB_ACCESS_DATA_OFFSET 			73	// 20 bytes
// OSDP Address
#define BOB_OSDP_CMD_OFFSET 			73+20	//(0x49)
#define BOB_OSDP_STATUS_OFFSET 			74+20	//(0x4A)
#define BOB_OSDP_DATA_LENGTH_OFFSET		75+20	//(0x4B)
#define BOB_OSDP_BAUDRATE_OFFSET 		76+20	//(0x4C)
#define BOB_OSDP_DATA_OFFSET 			77+20	//(0x4D)	// 128 bytes
#define BOB_OSDP_READER_CMD_OFFSET		205+20	//(0xCD)
#define BOB_OSDP_READER_LENGTH_OFFSET	206+20	//(0xCE)
#define BOB_OSDP_READER_BAUDRATE_OFFSET	207+20	//(0xCF)
#define BOB_OSDP_READER_DATA_OFFSET		208+20	//(0xD0)	//48 bytes

// Access Type/Protocol
#define BOB_ACCESS_TYPE_RELAY			1
#define BOB_ACCESS_TYPE_OSDP			2
#define BOB_ACCESS_TYPE_HID				3
#define BOB_ACCESS_TYPE_PAC				4
#define BOB_ACCESS_TYPE_F2F				5
#define BOB_ACCESS_TYPE_WIEGAND			6

#define BOB_ACCESS_TYPE_SING_DUAL_BASE	0
#define BOB_ACCESS_TYPE_TOC_BASE		50
#define BOB_ACCESS_TYPE_PASS_BASE		100

// Command
#define BOB_COMMAND_NONE	 			0
#define BOB_COMMAND_RESERVED			1	// Bootloader
#define BOB_COMMAND_READ	 			2
#define BOB_COMMAND_SEND	 			3
#define BOB_COMMAND_BLE_CMD	 			4
#define BOB_COMMAND_RTCREAD_CMD	 		5
#define BOB_COMMAND_RTCWRITE_CMD	 	6
#define BOB_COMMAND_SET_LED			 	7
#define BOB_COMMAND_OIM_ON			 	8
#define BOB_COMMAND_OIM_OFF			 	9

// ACS Status
#define BOB_STATUS_IN_CHANGE			0x01
#define BOB_STATUS_IN_CARD				0x02
#define BOB_STATUS_IN_ACS				0x04
#define BOB_STATUS_IN_TAMPER			0x08


#define BOB_STATUS_OUT_CHANGE			0x01
#define BOB_STATUS_OUT_ACS				0x02
#define BOB_STATUS_OUT_CARD_ACK			0x08

#define BOB_ACS_IN_LED_RED   			0x01
#define BOB_ACS_IN_LED_GRN   			0x02
#define BOB_ACS_IN_SOUNDER   			0x04

// Tamper Input
#define BOB_TAMPER_IN_READER    		0x01
#define BOB_TAMPER_IN_REED_1    		0x02
#define BOB_TAMPER_IN_REED_2    		0x04


#define BOB_ACS_OUT_LED_RED 			0x01
#define BOB_ACS_OUT_LED_GRN 			0x02
#define BOB_ACS_OUT_SOUNDER 			0x04
#define BOB_ACS_OUT_TAMPER_READER  		0x08	// Eyelock.TamperSignalHighToLow
#define BOB_ACS_OUT_RELAY_1 			0x10
#define BOB_ACS_OUT_RELAY_2 			0x20
#define BOB_ACS_OUT_TAMPER_PANEL  		0x40

// OSDP COMMAND
#define BOB_OSDP_CMD_DISABLE			0
#define BOB_OSDP_CMD_BUSY				1
#define BOB_OSDP_CMD_SEND_DATA			2
#define BOB_OSDP_CMD_READ_DATA			3
#define BOB_OSDP_CMD_SET_BAUDRATE		4


// OSDP STATUS
#define BOB_OSDP_STAT_DATA_EMPTY			0x00
#define BOB_OSDP_STAT_DATA_TO_SEND			0x01
#define BOB_OSDP_STAT_DATA_TO_READ			0x02
#define BOB_READER_OSDP_STAT_DATA_EMPTY		0x00
#define BOB_READER_OSDP_STAT_DATA_TO_SEND	0x10
#define BOB_READER_OSDP_STAT_DATA_TO_READ	0x20

// OSDP BAUD RATE
#define BOB_OSDP_BAUDRATE_DISABLE		0
#define BOB_OSDP_BAUDRATE_9600			1
#define BOB_OSDP_BAUDRATE_19200			2
#define BOB_OSDP_BAUDRATE_38400			3
#define BOB_OSDP_BAUDRATE_115200		4

// Mobile mode
#define BOB_MOBILE_MODE_WALK 			1
#define BOB_MOBILE_MODE_TAP 			2
#define BOB_MOBILE_MODE_PING 			3

// Factory Reset
#define BOB_FACTORY_RESET_OFFSET		6
#define BOB_FACTORY_STATUS_OFFSET		11
#define BOB_FACTORY_STATUS_CHANGE		0x01
#define BOB_FACTORY_STATUS_RESET		0x02
#define BOB_FACTORY_STATUS_RESTORE		0x04

// functions
int i2c_start_transaction();
int  BobGetData(void *ptr, int len);

void  BobSetRelay1(int val);
void  BobSetRelay2(int val);
void  BobSetSounderOUT(int val);
void  BobSetLedROUT(int val);
void  BobSetLedGOUT(int val);
void  BobSetTamperOUT(int val);
int   BobGetAllInputs(void);
int   BoBSetACSRelayOut(int);
void  BoBClearACSRelayOneOut();
void  BoBClearACSRelayTwoOut();
int   BobSetCardReadAck();
int   BobSetOSDPBaudRate(int rate, int reader);


int BobSetData(void *ptr, int len);
int BobSetACSType(int val);
int BobSetCommand(int val);
int BobSetDataAndRunCommand(void *ptr, int len, int cmd);
int BoBGetCommand();
int BobSetDataLength(int val);
int BobSetACSOutput(unsigned int value);
int BobClearACSOutput(unsigned int value);
int BobSetACSTypeBits(int val);

// returns NULL len =0 if no data was received
int Bob_Read_last_Wiegand(char *data,  int *len);
int Bob_Send_Weigand(char *data, int len);
int Bob_Set_WeigandMode(int mode);

#define BOB_MODE_WEIGAND 1
#define BOB_MODE_F2F     2



int Bob_Read_Local_Tamperl();
int Bob_Clear_Local_Tamper();


void BobInitComs();
void BobCloseComs();
void BoBSetQuit();
void BobSetPollACS(int pollACS);
void BobSetPollResetButton(int pollResetButton);

// callback
typedef void (*callback)();
typedef int (*callback2)(void *, int);
// set call back function which gets called if an input changes
// ie led in tamper in sound in

void BobSetInputCB(callback cb);
void BobSetOsdpCB(callback2 cb);
void BobSetReaderOsdpCB(callback2 cb);

void BoBSendOSDPResponse(void * pData, int length);
void BoBSendReaderOSDPCommand(void * data, int length);
void BobClearOSDPStatus(unsigned int value);
void BoBSetTimer(void (*cb)(void *), int ms, int repeat, void *thread);
void BoBCancelTimer(void (*cb)(void *));
void BoBSetRelayTimer(int ms);
void BoBSetRelayTimerNew(int, int);
void BobSetMyPtr(void *pData);
void BobSetMyCardStat(void *stat);
void * BobGetMyPtr();
// gets called if a weigand input is received
//int BobSetWeigandCB(callback cb);

int BoBStatusChangeCard();
int BoBStatusChangeAcs();
int BoBStatusChangeTamper();
int BoBGetACSTamperIn(int bit);
void BoBLogACSTamperIn();
int BoBGetACSGreenLedIn();
int BoBGetACSRedLedIn();
int BoBGetACSSoundIn();
int BoBGetACSFactoryReset();
void BoBClearACSFactoryReset();
void BoBSetACSTamperPol(int polFlag,int reader);
void BoBSetACSLedRedOut();
void BoBClearACSLedRedOut();
void BoBSetACSLedGreenOut();
void BoBClearACSLedGreenOut();
void BoBSetACSSoundOut(int, int);
void BoBClearACSSoundOut();
void BoBSetACSTamperOut(int polFlag,int tamper);
void setinputAcs(int val);
int BoBOSDPPanelMsg();
int BoBOSDPReaderMsg();
int BobGetOSDPPanelData(unsigned char *ptr);
int BobGetOSDPReaderData(unsigned char *ptr);

int BobReadReg(unsigned char reg, unsigned int *val);
int BobWriteReg(unsigned char reg, unsigned int val);
int BobReadArray(unsigned char reg, char *buf, int len);
int BobWriteArray(unsigned char reg, void *ptr, int len);

unsigned long BobCurrentTimeInMS();


#endif /* BOBLISTENER_H_ */
