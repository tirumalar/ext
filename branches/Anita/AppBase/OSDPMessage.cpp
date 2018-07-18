/*
 * OSDPMessage.cpp
 *
 *  Created on: 23-Jun-2016
 *      Author: fjia
 */
#include <iostream>
#include <sstream>
#include <iomanip>
#include <fcntl.h>
#include <unistd.h>
#include "OSDPMessage.h"
//#include "Configuration.h"
#include "MatchedSignal.h"
#include "F2FDispatcher.h"
#include "CommonDefs.h"
#include "LEDConsolidator.h"
#include "logging.h"
#include "OpenSSLSupport.h"
#include "AESClass.h"
extern "C" {
#include "include/BobListener.h"
#include "file_manip.h"
}

using namespace std;
class F2FDispatcher;

std::string hexStr(unsigned char *data, int len);
const char logger[30] = "OSDPMessage";

unsigned short getShortData(unsigned char * ptr);
unsigned char calcChecksum(unsigned char * msg, int len);
static int fCrcTblInit(uint16 * pTbl);

OSDPMessage::OSDPMessage(Configuration& conf)
{
	m_OSDPAddr = conf.getValue("Eyelock.OSDPAddress", 0);
	m_osdpDebug = conf.getValue("Eyelock.OSDPDebug",false);
	m_osdpReaderDebug = conf.getValue("Eyelock.OSDPReaderDebug",false);
	m_ledControlledByInput = conf.getValue("GRITrigger.DualAuthNLedControlledByACS", false);
	m_OSDPBRate = conf.getValue("Eyelock.OSDPBaudRate", 9600);
	m_ReaderOSDPBRate = conf.getValue("Eyelock.ReaderOSDPBaudRate", 9600);

	// TODO: refactor.
	// Code duplication, same code: LEDDispatcher.cpp, NwMatchManager.cpp, OSDPMessage.cpp, F2FDispatcher.cpp
	// use one function from EyelockConfiguration for all?
	// get rid of bool value?
	m_dualAuth = false;
	int authenticationMode = conf.getValue("Eyelock.AuthenticationMode",0);
	if (authenticationMode){
		switch (authenticationMode)
		{
			case CARD_OR_IRIS:
				break;
			case CARD_AND_IRIS:
				m_dualAuth = true;
				break;
			case CARD_AND_IRIS_PIN_PASS:
				m_dualAuth = true;
				break;
			case PIN_AND_IRIS:
			case PIN_AND_IRIS_DURESS:
				m_dualAuth = true;
				break;
			case CARD_AND_PIN_AND_IRIS:
			case CARD_AND_PIN_AND_IRIS_DURESS:
				m_dualAuth = true;
				break;
			default:
				m_dualAuth = false;
				break;
		}
	}
	else {
		m_dualAuth = conf.getValue("GRITrigger.DualAuthenticationMode",false);
	}

	m_osdpSCEnabled = true;
	m_osdpReaderSCEnabled = conf.getValue("Eyelock.OSDPSecure",false);
	m_osdpCardDataAvailable = false;
	isTamperSet = false;
	m_fjiaDebug = false;
	memset((void *)&m_osdpLedControl, 0, sizeof(OsdpACSLEDCommand));
	memset((void *)&m_osdpReader, 0, sizeof(OsdpReaderCapCommand));
	m_osdpACSSqn = 0;
	memset((void *)&m_ToPanel, 0, sizeof(OsdpSecureChannel));
	memset((void *)&m_ToReader, 0, sizeof(OsdpSecureChannel));
	unsigned char cuid[] = {0x02, 0x00, 0x75, 0x32, 0x03, 0x06, 0xd3, 0xe3};
	//unsigned char cuid[] = {0x00, 0x06, 0x8E, 0x00, 0x00, 0x39, 0x37, 0x33};	// read cuid
	memcpy(m_ToPanel.cuid, cuid, CUID_SIZE);
	m_ToReader.isCP = true;
	unsigned char mk[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};
	memcpy(m_ToPanel.mk, mk, ENC_SIZE);
	memcpy(m_ToReader.mk, mk, ENC_SIZE);

	// restore the m_ToPanel structure
	FILE *fp = fopen("/home/osdps","r");
	if (fp) {
		int len = fread(&m_ToPanel, 1, sizeof(OsdpSecureChannel), fp);
		if (len != sizeof(OsdpSecureChannel))
			EyelockLog(logger, ERROR, "Restore m_ToPanel failed");
	    fclose(fp);
	}
}

#if 0
void OSDPMessage::Process()
{
	EyelockLog(logger, INFO, "OSDP => *** OSDPMessages thread ***");
	unsigned char msg[128] = {0};
	int msglen = 0;
	static uint64_t timestart = 0;

	while (!m_ShouldIQuit)
    {
		CURR_TV_AS_MSEC(timecurr);
		unsigned int elapsed = timecurr-timestart;
		unsigned int diff = (OSDP_RUN_TIME > elapsed) ? (OSDP_RUN_TIME - elapsed) : 1;
		usleep(diff*1000);

		CURR_TV_AS_MSEC(timenew);
		timestart = timenew;
		//printf("curr %llu start %llu\n", timenew, timestart);
		if (BoBOSDPPanelMsg()) {
			msglen = BobGetOSDPPanelData(msg);
			if (msglen == 0)
				EyelockLog(logger, ERROR, "Read OSDP Panel Message failed");
			else
				ProcessPanelMsg(msg, msglen);

			printf("===== m_ShouldIQuit %d\n", m_ShouldIQuit);
			DumpBuffer(msg, msglen);
		}

		if (BoBOSDPReaderMsg()) {
			msglen = BobGetOSDPReaderData(msg);
			if (msglen == 0)
				EyelockLog(logger, ERROR, "Read OSDP Reader Message failed");
			else
				ProcessReaderMsg(msg, msglen);
		}
    }
	EyelockLog(logger, INFO, "OSDPThread stopped!");

}
#endif
int OSDPMessage::ProcessPanelMsg(unsigned char *msg, int dataLength)
{
	bool status = false;
	unsigned char reply[128];
	short replyLen = 0;
	short replyDataLen = 0;
	memset((void *)&m_ReplyPanel, 0, sizeof(m_ReplyPanel));

	if(m_osdpDebug) {
		printf("PANEL: received --- ");
		DumpBuffer(msg, dataLength);
	}

	if (ValidateHeader(msg, dataLength, true)) {
		unsigned char control = msg[4];
		int sqn;
		bool crc, scb, multi;
		int crclen = 1;
		parseControl(control, sqn, crc, scb, multi);
		if (crc)
			crclen = 2;
		status = ValidateCRC(msg, dataLength, crc);
		if (!status) {
			if (m_osdpDebug)
				EyelockLog(logger, DEBUG, "Invalid CRC or checksum in panel message");
			// reply osdp_NAK
			m_ReplyPanel.msgData[0] = 1; 	//checksum error
		}
		else if (scb) {
			if (!m_osdpSCEnabled) {
				// reply osdp_NAK
				m_ReplyPanel.msgData[0] = 0x05; 	// not support security block
				scb = 0;
				control &= ~0x08;
			}
			else {
				status = SetupPanelSCS(msg, dataLength-crclen);
				if (status == false) {
					m_ReplyPanel.msgData[0] = 0x06; 	// sc error
				}
			}
		}
		else {
			status = ProcessPanelCommand(msg[HDLEN], msg+HDLEN+1, dataLength-HDLEN-crclen-1);
			if (status == false) {
				m_ReplyPanel.msgData[0] = 0x03; 	// unknown command code
			}
		}
		if (status == false) {
			m_ReplyPanel.msgCode = osdp_NAK;
			m_ReplyPanel.msgLength = 1;
		}

		// build reply message header
		int pos = 0;
		reply[pos++] = osdp_HEADER;
		reply[pos++] = msg[1] + 0x80; //set the reply bit
		//next 2 bytes are the address...  we'll get to that later
		pos+=2;
		reply[pos++] = control;
		replyLen += HDLEN;
		//if we had a security block copy it back here
		if(scb)
		{
			reply[pos++] = m_ReplyPanel.msgSCB.scbLen;
			reply[pos++] = m_ReplyPanel.msgSCB.scbType;
			if (m_ReplyPanel.msgSCB.scbLen > 2) {
				unsigned char * scbReplyData = reply + pos;
				memcpy(scbReplyData, m_ReplyPanel.msgSCB.scbData, m_ReplyPanel.msgSCB.scbLen-2);
			}
			replyLen += m_ReplyPanel.msgSCB.scbLen;
			pos = replyLen;
		}
		// reply data
		unsigned char * replyDataPos = reply + pos;
		memcpy(replyDataPos, &m_ReplyPanel, m_ReplyPanel.msgLength+1);
		replyLen += m_ReplyPanel.msgLength+1;
		pos += m_ReplyPanel.msgLength+1;
		unsigned char type = m_ReplyPanel.msgSCB.scbType;
		if(scb && (type == SCS_16 || type == SCS_18))
		{
			// set MAC 4 bytes mac
			unsigned char * macReplyPos = reply + pos;
			//memcpy(macReplyPos, &mac, 4);
			replyLen += MAC_BYTES;
			reply[2] = replyLen+crclen;
			reply[3] = (replyLen+crclen) >> 8;
			GenerateMAC(&m_ToPanel, reply, pos, false);
			memcpy(macReplyPos, m_ToPanel.mac_R, MAC_BYTES);

		}
		CalculateCRC(reply, &replyLen, crc);
		if(m_osdpDebug) {
			//printf("PANEL: reply --- ");
			EyelockLog(logger, DEBUG, "PANEL: reply --- ");
			DumpBuffer(reply, replyLen);
		}

		if (replyLen < 128) {
			//write back to the bob
			BoBSendOSDPResponse((void*)reply, (int)replyLen);
		}
#if 0
		if (scb){
			// store m_ToPanel
			FILE *fp = fopen("osdp.txt","a");
			if (fp) {
				int results = fwrite((const char *)&m_ToPanel, 1, sizeof(m_ToPanel), fp);
				//int results = fputs((const char *)&m_ToPanel, fp);
				if (results != sizeof(m_ToPanel)) {
					EyelockLog(logger, ERROR, "Save key failed");
				}
				fclose(fp);
			}
		}
#endif
	}

	return true;
}


bool OSDPMessage::SetupPanelSCS(unsigned char *msg, int dataLength)
{
	int datalen = 0;
	OsdpSCB *scb = (OsdpSCB *)(msg+HDLEN);
	int pos = HDLEN+scb->scbLen;
	unsigned char
	command = msg[pos++]; //command byte comes before the data
	unsigned char *data = msg + pos;
	unsigned char reply[128];

	if (m_fjiaDebug)
		printf("SetupPanelSCS() scbType %d, dataLength %d", scb->scbType, dataLength);

	switch(scb->scbType)
	{
		case SCS_11:
		{
			//unsigned char tmp[] = {0x53, 0x00, 0x13, 0x00, 0x0d, 0x03, 0x11, 0x00, 0x76, 0xf9, 0x0c, 0x9b, 0xcf, 0x43, 0x8f, 0x58, 0xe8, 0x72, 0x7f};
			//memcpy(msg, tmp, dataLength);
			datalen = 8; 	// 8-byte randA
			if (scb->scbLen != 3 || command != osdp_CHLNG || pos+datalen != dataLength) {
				m_ReplyPanel.msgSCB.scbLen = 0;
				return false;
			}

			memcpy(m_ToPanel.randA, data, RANDNUM_SIZE);
			m_ToPanel.isCustomKey = scb->scbData[0];

			// init data
			// Reply SCS_12
			m_ReplyPanel.msgSCB.scbLen = 3;
			m_ReplyPanel.msgSCB.scbType = SCS_12;
			m_ReplyPanel.msgSCB.scbData[0] = scb->scbData[0];

			SetSecureChannelInfo(&m_ToPanel, RAND);
			//if (m_ToPanel.isCustomKey)
				//SetSecureChannelInfo(&m_ToPanel, SCBK);
			//else if (m_ToPanel.chanStat != CONNECTED)
			if (!m_ToPanel.isCustomKey)
				memcpy(m_ToPanel.scbk, m_ToPanel.mk, ENC_SIZE);
			SetSecureChannelInfo(&m_ToPanel, DERIVEKEY);
			SetSecureChannelInfo(&m_ToPanel, CRYPTOGRAM);

			// reply osdp_CCRYPT
			m_ReplyPanel.msgCode = osdp_CCRYPT;
			m_ReplyPanel.msgLength = CUID_SIZE + RANDNUM_SIZE + ENC_SIZE;
			unsigned char *dataptr = m_ReplyPanel.msgData;
			memcpy(dataptr, m_ToPanel.cuid, CUID_SIZE);
			dataptr += CUID_SIZE;
			memcpy(dataptr, m_ToPanel.randB, RANDNUM_SIZE);
			dataptr += RANDNUM_SIZE;
			memcpy(dataptr, m_ToPanel.cryptB, ENC_SIZE);
			m_ToPanel.chanStat = SETUP;
			break;
		}

		case SCS_13:
			datalen = ENC_SIZE; 	// 16-byte cryptA
			if (scb->scbLen != 3 || command != osdp_SCRYPT || pos+datalen != dataLength) {
				m_ReplyPanel.msgSCB.scbLen = 0;
				return false;
			}

			// Reply SCS_14
			m_ReplyPanel.msgSCB.scbLen = 3;
			m_ReplyPanel.msgSCB.scbType = SCS_14;

			if (memcmp(&(m_ToPanel.cryptA), data, ENC_SIZE)) {
				m_ReplyPanel.msgSCB.scbData[0] = 0xFF;		// Server Cryptogram not ok
				m_ReplyPanel.msgCode = osdp_NAK;
				m_ReplyPanel.msgLength = 1;
				m_ReplyPanel.msgData[0] = 0x05;
			}
			else {
				m_ReplyPanel.msgSCB.scbData[0] = 0x01;
				SetSecureChannelInfo(&m_ToPanel, MAC_I);
				m_ReplyPanel.msgCode = osdp_RMAC_I;
				m_ReplyPanel.msgLength = ENC_SIZE;
				memcpy(&(m_ReplyPanel.msgData[0]), &(m_ToPanel.mac_I), ENC_SIZE);
			}

			break;
		case SCS_15:
		case SCS_17:
		{
			// set default osdp_NAK scb
			m_ReplyPanel.msgSCB.scbLen = 2;
			m_ReplyPanel.msgSCB.scbType = SCS_18;

			datalen = 0; 	// does not contain encrypted DATA, e.g osdp_POLL
			unsigned char comdata[128];
			memset(comdata, 0, 128);

			if (scb->scbLen != 2)
				return false;
			if (!ValidateMAC(msg, dataLength, true)) {
				EyelockLog(logger, DEBUG, "Invalid MAC data");
				return false;
			}

			if (scb->scbType == SCS_17) {
				datalen = dataLength-pos-MAC_BYTES;
				if (datalen % ENC_SIZE) {
					EyelockLog(logger, DEBUG, "Invalid encrypted data length %d", datalen);
					return false;
				}
				DecryptOSDPData(data, comdata, m_ToPanel.enc, m_ToPanel.mac_R, datalen);
			}
			bool result = ProcessPanelCommand(command, comdata, datalen);
			if (result == false) {
				EyelockLog(logger,  DEBUG, "ProcessPanelCommand() failed, command 0x%x, datalen %d", command, datalen);
				return false;
			}

			// create the reply message PD->CP
			if (m_ReplyPanel.msgLength == 0) {
				// Reply SCS_16
				m_ReplyPanel.msgSCB.scbLen = 2;
				m_ReplyPanel.msgSCB.scbType = SCS_16;
				//m_ReplyPanel.msgSCB.scbData[0] = m_ReplyPanel.msgCode;
			}
			else {
				// Reply SCS_18
				m_ReplyPanel.msgSCB.scbLen = 2;
				m_ReplyPanel.msgSCB.scbType = SCS_18;
				unsigned char *pddata = m_ReplyPanel.msgData;
				int len = m_ReplyPanel.msgLength;

				int newlen = DataPadAndEncrypt(&m_ToPanel, pddata, len);
				if (newlen == 0){
					EyelockLog(logger, ERROR, "Failed in data encrypt()");
					return false;
				}
				m_ReplyPanel.msgLength = newlen;
			}
		}
			break;

		default:
			break;
	}
	//DumpBuffer((unsigned char *)&m_ToPanel, sizeof(m_ToPanel));
	return true;
}

bool OSDPMessage::ProcessPanelCommand(unsigned char command, unsigned char *data, int dataLength)
{
	if (m_osdpDebug)
		EyelockLog(logger, TRACE, "ProcessPanelCommand() - command 0x%x, datalen %d bytes", command, dataLength);

	bool postProcess = false;
	int OSDPBRate;
	int pos = 0;

	unsigned char replyCode = 0;
	int replyLength = 0;
	unsigned char *replyData = m_ReplyPanel.msgData;

	if (command < osdp_POLL)
		return false;

	switch(command)
	{
		case osdp_POLL:
			//got a poll! the reply to a poll depends on if the state changed.
			if(osdpStateChange)  //todo: we may later want to have different state change detectors
			{
				//if the state changed reply like we do to the LSTAT
				//got LSTAT request... reply is the status!
				replyCode = osdp_LSTATR;
				replyLength = 2;
				replyData[0] = isTamperSet; 	//tamper state  1 = tamper
				replyData[1] = 0; 				//power state  1 = power loss

				osdpStateChange = false;
				if (m_osdpDebug)
					EyelockLog(logger, DEBUG, "OSDP state change - tamper %d, %d bytes, %d-%d\n", isTamperSet, replyLength, replyData[0],replyData[1]);
			}
			else if(m_osdpCardDataAvailable)
			{
				replyCode = osdp_RAW;
				replyLength = m_osdpByteLength + 4;
				replyData[0] = 0;
				replyData[1] = 1;
				short sdataLen = ((short)m_osdpBitLength);
				memcpy(replyData+2, &sdataLen, 2);
				memcpy(replyData+4, osdpCardData, m_osdpByteLength);
				m_osdpCardDataAvailable = false;
				m_osdpLedControl.changeLed = true;
				if (m_osdpDebug)
					EyelockLog(logger, DEBUG, "OSDP data %d bytes, %d-%d-%d-%d-%d-%d-%d-%d\n", replyLength, replyData[0],replyData[1],replyData[2],replyData[3],replyData[4],replyData[5],replyData[6],replyData[7]);
			}
			else //reply with an ack
				replyCode = osdp_ACK;
			break;
		case osdp_ID:
			replyCode = osdp_PDID;
			replyLength = 12;
			memcpy(replyData, m_ToPanel.cuid, CUID_SIZE);
			replyData[8] = 0x6b;
			replyData[9] = 0x03;
			replyData[10] = 0x02;
			replyData[11] = 0x02;
#if 0
			replyData[8] = 0x33;
			replyData[9] = 0x01;
			replyData[10] = 0x8b;
			replyData[11] = 0x00;

			// vendor code
			replyData[0] = 0x50;
			replyData[1] = 0xc2;
			replyData[2] = 0xed;
			//model number
			replyData[3] = 1;
			replyData[4] = 1;
			//serial number... not used by osdp
			replyData[5] = replyData[6] = replyData[7] = replyData[8] = 0;
			//firmware code
			replyData[9] = 1;
			replyData[10] = 0;
			replyData[11] = 2;
#endif
			//printf("replyCode %d, replyLength %d\n", replyCode, replyLength);
			break;
		case osdp_CAP:
		{
			//this should be 3 bytes for each support code we supply. function code, compliance, number
			replyCode = osdp_PDCAP;
			int i = 0;
			//card data format
			replyData[i++]=3;
			replyData[i++]= 1;
			replyData[i++] = 0;
			//led control
			replyData[i++]=4;
			replyData[i++]= 4;
			replyData[i++] = 0;
			//buzzer
			replyData[i++]=5;
			replyData[i++]= 0;
			replyData[i++] = 0;
			//check character
			replyData[i++]= 8;
			replyData[i++]= 0;
			replyData[i++] = 0;
			if (m_osdpSCEnabled){
				//check security
				replyData[i++]= 9;
				replyData[i++]= 1;
				replyData[i++] = 0;
			}
			replyLength = i;
			break;
		}

		case osdp_LSTAT:
			//got LSTAT request... reply is the status!
			replyCode = osdp_LSTATR;
			replyLength = 2;
			replyData[0] = isTamperSet; //tamper state  1 = tamper
			replyData[1] = 0; //power state  1 = power loss

			break;
		//case 0x65: ISTAT not required
		//case 0x66  OSTAT not required
		case osdp_RSTAT:
			//got RSTAT
			replyCode = osdp_RSTATR;
			replyLength = 1;
			replyData[0] = 0; //could report a tamper state to this one too... the panel will poll the LSTAT for that though
			break;

		case osdp_OUT:
		{
			int outputNumber = data[0];
			int controlCode = data[1];
			short timer = *((short*)data +2);

			replyCode = osdp_ACK;
		}
			break;
		case osdp_LED:
			if (m_osdpDebug)
				EyelockLog(logger, DEBUG, "\n\n\nCommand osdp_LED, ACSControl %d, changeLed %d, ledCommand %d\n", m_ledControlledByInput, m_osdpLedControl.changeLed, m_osdpLedControl.ledCommand);
			if (m_ledControlledByInput && m_osdpLedControl.changeLed)
			{
				memcpy(m_osdpLedControl.ledData, data, 14);	// 14 bytes in osdp_LED data
				m_osdpLedControl.ledCommand = true;
			}
			replyCode = osdp_ACK;
			break;
		case osdp_BUZ:
			// Not support
			replyCode = osdp_ACK;
			break;
		case osdp_COMSET:
			m_OSDPAddr = data[0];
			OSDPBRate = *((int*)data + 1);

			replyCode = osdp_COM;
			replyLength = 5;
			replyData[0] = m_OSDPAddr;
			memcpy(replyData+1, &m_OSDPBRate, sizeof(int));
			//todo: after this writes set the baud rate and address
			postProcess =true;
			break;
		case osdp_KEYSET:
			// Not support
			if (data[0] == 0x01 && data[1] == 0x10){
				memcpy(m_ToPanel.scbk, data+2, ENC_SIZE);
				replyCode = osdp_ACK;
				replyLength = 0;
				m_ToPanel.chanStat = CONNECTED;
				m_ToPanel.isCustomKey = true;
				// save to a file
				FILE *fp = fopen("/home/osdps","a");
				if (fp) {
					int results = fwrite((const char *)&m_ToPanel, 1, sizeof(m_ToPanel), fp);
					if (results != sizeof(m_ToPanel)) {
						EyelockLog(logger, ERROR, "Save key failed");
					}
					fclose(fp);
				}
			}
			else{
				EyelockLog(logger, ERROR, "invalid KEYSET message");
				replyCode = osdp_NAK;
				replyLength = 1;
				replyData[0] = 0x06;
			}
			break;
		default:
			//reply an ack to anything we don't recognize specifically yet
			return false;
	}

	m_ReplyPanel.msgLength = replyLength;
	m_ReplyPanel.msgCode = replyCode;
	return true;
}

void OSDPMessage::OSDPLedCommand(int state)
{
	unsigned char data[14];
	data[0] = 0;
	data[1] = 0;
	// temporary
	data[2] = 1;
	data[3] = 0;
	data[4] = 0;
	data[5] = 0;
	data[6] = 0;
	data[7] = 0;
	data[8] = 0;

	if (state == CARD_MATCHED) {
		// permanent
		data[9] = 1;
		data[10] = 10;
		data[11] = 0;
		data[12] = 2;	// green on
		data[13] = 2;	// green off
	}
	else if (state == NOT_MATCHED) {
		// permanent 0 0 0 0 0 2 0 0 0 1 a a 1 0
		data[9] = 1;
		data[10] = 10;
		data[11] = 10;
		data[12] = 1;	// red on
		data[13] = 1;	// red off
	}
	else {
		// temporary
		data[2] = 2;
		data[3] = 3;	// 300ms
		data[4] = 3;	// 300ms
		data[5] = 1;	// red on
		data[6] = 0;	// off
		data[7] = 18;	// flash 3 times: 3*(3+3)=18
		data[8] = 0;
		// permanent
		data[9] = 0;
		data[10] = 0;
		data[11] = 0;
		data[12] = 0;
		data[13] = 0;
	}

	SendOSDPCommand(osdp_LED, data);
	m_osdpReader.ledState = 0;
}

void OSDPMessage::OSDPBuzCommand(int state)
{
	unsigned char data[5];
	data[0] = 0;
	data[1] = 2;

	if (state == CARD_MATCHED) {
		data[2] = 1;	// 100ms
		data[3] = 1;	// 100ms
		data[4] = 3;	// repeat 3 times
	}
	else if (state == NOT_MATCHED) {
		data[2] = 3;	// 300ms
		data[3] = 3;	// 300ms
		data[4] = 4;	// repeat 4 times, one from reader
	}
	else {
		data[2] = 3;	// 300ms
		data[3] = 3;	// 300ms
		data[4] = 1;	// repeat 1 times, one from reader
	}
	SendOSDPCommand(osdp_BUZ, data);
	m_osdpReader.buzState = 0;
}

void OSDPMessage::BuildOSDPCommand(int command, unsigned char *data)
{
	unsigned char *dataPtr = m_CommandReader.msgData;
	int dataLen = 0;

	m_CommandReader.msgCode = command;
	switch (command)
	{
		case osdp_RESET:	// reset reader, send osdp_ID command with sqn=0
			m_osdpReader.msgSqn = 0;
			if (m_osdpReader.msgScb){
				SetupReaderSCS(NULL, 0, true);
				dataLen = m_CommandReader.msgLength;
			}
			else {
				m_osdpReader.msgScb = false;
				m_CommandReader.msgCode = osdp_ID;
				dataPtr[0] = 0x00;
				dataLen = 1;
			}
			break;
		case osdp_POLL:
			dataLen = 0;
			break;
		case osdp_ID:
		case osdp_CAP:
			dataPtr[0] = 0x00;
			dataLen = 1;
			break;
		case osdp_LSTAT:
			dataLen = 0;
			break;
		case osdp_LED:
			memcpy(dataPtr, data, 14);
			dataLen = 14;
			break;
		case osdp_BUZ:
			memcpy(dataPtr, data, 5);
			dataLen = 5;
			break;
		case osdp_COMSET:
			dataPtr[0] = 0;
			*((int*)dataPtr +1) = m_ReaderOSDPBRate;
			dataLen = 5;
			break;
		case osdp_CHLNG:
		case osdp_SCRYPT:
			dataLen = m_CommandReader.msgLength;
			break;
		case osdp_KEYSET:
			printf("++++++++++ osdp_CHLNG =====\n ");
			m_ToReader.chanStat = KEYSET;
			dataLen = m_CommandReader.msgLength;
			break;

		default:
			dataLen = 0;
			return;
	}
	m_CommandReader.msgLength = dataLen;

}

void OSDPMessage::SendOSDPCommand(int command, unsigned char *data)
{
	if (m_osdpReaderDebug)
		EyelockLog(logger,  TRACE, "SendOSDPCommand() start - command 0x%x ", command);
	BuildOSDPCommand(command, data);
	unsigned char msg[128];
	short crclen = 0, totalLen = 0, pos = 0;
	OsdpSCB *scb = &m_CommandReader.msgSCB;

	msg[pos++] = osdp_HEADER;	// pos = 0
	msg[pos++] = 0;				// pos = 1, m_cardReaderAddress;
	pos += 2;					// pos = 2,3, msg length
	// pos = 4, control
	msg[pos] = m_osdpReader.msgSqn;
	if (CRC_ENABLE){
		msg[pos] += 0x04;
		crclen = 2;
	}
	pos++;

	// set scb
	if (m_osdpReader.msgScb){
		msg[pos-1] += 0x08;
#if 0
		if (m_ToReader.chanStat == KEYSET || m_ToReader.chanStat == CONNECTED){
			if (m_CommandReader.msgLength == 0) {
				scb->scbLen = 2;
				scb->scbType = SCS_15;
				//m_ReplyPanel.msgSCB.scbData[0] = m_ReplyPanel.msgCode;
			}
			else {
				scb->scbLen = 2;
				scb->scbType = SCS_17;
				unsigned char *pddata = m_CommandReader.msgData;
				int len = m_CommandReader.msgLength;

				int newlen = DataPadAndEncrypt(&m_ToReader, pddata, len);
				m_CommandReader.msgLength = newlen;
			}
		}
#endif
		memcpy(&msg[pos], scb, scb->scbLen);
		pos += scb->scbLen;
	}

	msg[pos++] = m_CommandReader.msgCode;
	memcpy(msg+pos, m_CommandReader.msgData, m_CommandReader.msgLength);
	pos += m_CommandReader.msgLength;
	totalLen = pos;
	// set MAC
	if(m_osdpReader.msgScb && (scb->scbType == SCS_15 || scb->scbType == SCS_17))
	{
		//4 bytes mac
		unsigned char * macReplyPos = msg + pos;
		//memcpy(macReplyPos, &mac, 4);
		totalLen += MAC_BYTES;
		msg[2] = totalLen+crclen;
		msg[3] = (totalLen+crclen) >> 8;
		GenerateMAC(&m_ToReader, msg, pos, true);
		memcpy(macReplyPos, m_ToReader.mac_C, MAC_BYTES);

	}
	CalculateCRC(msg, &totalLen, CRC_ENABLE);

	if(m_osdpReaderDebug) {
		printf("READER: send ==== ");
		DumpBuffer(msg, totalLen);
	}

	BoBSendReaderOSDPCommand((void *)msg, (int)totalLen);
}

bool OSDPMessage::ProcessReaderMsg(unsigned char *msg, int dataLength)
{
	if (dataLength == 1) {	// reset reader
		if (m_osdpReaderDebug)
			EyelockLog(logger,  TRACE, "ProcessReaderMsg()- reset command");
		SendOSDPCommand(osdp_RESET, 0);
		return true;
	}

	if(m_osdpReaderDebug) {
		printf("READER: received ==== ");
		DumpBuffer(msg, dataLength);
	}

	if (!ValidateHeader(msg, dataLength, false)){
		if (m_osdpReaderDebug)
			EyelockLog(logger,  DEBUG, "Invalid OSDP header");
		return false;
	}

	unsigned char control = msg[4];
	int sqn;
	bool crc, scb, multi;
	//int crclen = 1;
	parseControl(control, sqn, crc, scb, multi);

	//the next 2 bytes are the length of the whole message from SOM to CKSUM
	//short datalen = *((short*)(msg + 2));  //msg + 2 is the pointer for msg[2]

	// validate sqn
	if (sqn != m_osdpReader.msgSqn){
		if (m_osdpReaderDebug)
			EyelockLog(logger,  TRACE, "\ninvalid sqn %d expected %d\n", sqn, m_osdpReader.msgSqn);
		return false;
	}
	m_osdpReader.msgSqn++;
	if (m_osdpReader.msgSqn > 3)
		m_osdpReader.msgSqn = 1;
	// validate CRC
	int crclen = (crc) ? 2:1;
	bool messageOK = ValidateCRC(msg, dataLength, crc);
	if (!messageOK) {
		if (m_osdpReaderDebug)
			EyelockLog(logger,  DEBUG, "Invalid CRC or checksum in reader message");
		return false;
	}

	if (scb){
		//scbLen = msg[5];
		SetupReaderSCS(msg, dataLength-crclen, false);
	}
	else {
		int response = msg[HDLEN];
		unsigned char *data = msg + HDLEN + 1;
		ProcessReaderReply(response, data, dataLength-HDLEN-crclen-1);
	}
	if (m_fjiaDebug)
		memset(msg, 0, 128);
	if (m_osdpReader.ledState)
		OSDPLedCommand(m_osdpReader.ledState);
	//else if (m_osdpReader.buzControl > 1 && m_osdpReader.buzState)
	else if (m_osdpReader.buzState)
		OSDPBuzCommand(m_osdpReader.buzState);
	else
		SendOSDPCommand(m_CommandReader.msgCode, NULL);

}

bool OSDPMessage::ProcessReaderReply(unsigned char response, unsigned char *data, int dataLength)
{
	if (m_osdpReaderDebug)
		EyelockLog(logger,  TRACE, "ProcessReaderReply() 0x%x", response);

	unsigned char command = osdp_POLL;
	// check reply message
	switch (response)
	{
		case osdp_PDID:
			command = osdp_CAP;
			break;
		case osdp_RAW:
		{
			printf("READER card data: received --- ");
			DumpBuffer(data, dataLength);
			unsigned char reader = data[0];
			unsigned char format = data[1];
			if (format == 1){	// Wiegand
				short bitNumber = (short)data[2];
				memcpy(osdpCardData, &data[4], (bitNumber+7)/8);
				m_f2fDispatcher->m_cardRead = CARD_READY;	// set CARD_READY flag to process card data
			}
			command = osdp_POLL;
			break;
		}
		case osdp_PDCAP:
		{
			command = osdp_POLL;
			//  0x53 0x80 0x1f 0x0 0x2 0x46 0x1 0x4 0x1 0x2 0x4 0x1 0x3 0x1 0x0 0x4 0x4 0x1 0x5 0x2 0x1 0x6 0x0 0x0 0x7 0x0 0x0 0x8 0x1 0x0 0x8e
			for (int i = 0; i < dataLength; i = i+3) {
				if (data[i] == 4) {			// LED Control
					m_osdpReader.ledControl = data[i+1];
				}
				else if (data[i] == 5) {	// Buzzer Control
					m_osdpReader.buzControl = data[i+1];
				}
				else if (data[i] == 9) {	// Security
					if (m_osdpReaderSCEnabled && data[i+1]==1){
						if (m_osdpReaderDebug)
							EyelockLog(logger,  DEBUG, "READER setup secure channel ");
						SetupReaderSCS(NULL, 0, true);
						command = osdp_CHLNG;
					}
				}
			}

			//OSDPLedCommand(CARD_INIT);
			break;
		}
//		case osdp_CCRYPT:
//			SetupReaderSCS((unsigned char *)msg+HDLEN, dataLength, false);
//			command = osdp_CHLNG;
//			break;

		case osdp_NAK:
			return true;
		case osdp_ACK:
			printf("chanStat %d\n", m_ToReader.chanStat);
			if (m_ToReader.chanStat == KEYSET){
				m_ToReader.isCustomKey = true;
				SetupReaderSCS(NULL, 0, true);
				command = osdp_CHLNG;
			}
			else
				command = osdp_POLL;
			break;
		default:
			command = osdp_POLL;
			break;
	}
// printf("&&&&&&&&&&& led control %d, state %d &&&&&&&7\n", m_osdpReader.ledControl, m_osdpReader.ledState);
	//if (m_osdpReader.ledControl > 0 && m_osdpReader.ledState)
	m_CommandReader.msgCode = command;
	return true;
}

bool OSDPMessage::SetupReaderSCS(unsigned char *msg, int length, bool init)
{
	if (m_osdpReaderDebug)
		EyelockLog(logger,  TRACE, "SetupReaderSCS() start - init %d, length %d", init, length);
	unsigned char *data = m_CommandReader.msgData;
	OsdpSCB *scb = &m_CommandReader.msgSCB;
	OsdpSecureChannel *reader = &m_ToReader;
	if (init)
	{
		m_osdpReader.msgScb = true;
		reader->chanStat = SETUP;
		// try to send key
		// Set the default key
		if (!reader->isCustomKey)
			memcpy(reader->scbk, reader->mk, ENC_SIZE);

		unsigned char rand[] = {0xB0, 0xB1, 0xB2, 0xB3, 0xB4, 0xB5, 0xB6, 0xB7};
		memcpy(reader->randA, rand, RANDNUM_SIZE);
		SetSecureChannelInfo(&m_ToReader, RAND);
		memcpy(data, rand, RANDNUM_SIZE);
		m_CommandReader.msgCode = osdp_CHLNG;
		m_CommandReader.msgLength = RANDNUM_SIZE;
		scb->scbLen = 3;
		scb->scbType = SCS_11;
		scb->scbData[0] = reader->isCustomKey;	// default key
		return true;

	}

	int pos = HDLEN;
	unsigned char scblen = msg[pos++];
	unsigned char scbtype = msg[pos++];
	unsigned char *scbData = msg + pos;
	unsigned char command = msg[HDLEN+scblen]; //command byte comes before the data
	unsigned char *msgdata = msg + HDLEN + scblen + 1;
	int datalen = 0;


	switch(scbtype)
	{
		case SCS_12:
		{
			datalen = CUID_SIZE + RANDNUM_SIZE + ENC_SIZE;
			if (scblen != 3 || command != osdp_CCRYPT || datalen != length-(HDLEN+scblen+1)){
				if (m_osdpReaderDebug)
					EyelockLog(logger,  DEBUG, "Invalid SCS_12, scblen %d, command 0x%x, datalen %d", scblen, command, datalen);
				return false;
			}
			unsigned char crypt[ENC_SIZE];
			memcpy(reader->cuid, msgdata, CUID_SIZE);
			memcpy(reader->randB, msgdata+CUID_SIZE, RANDNUM_SIZE);
			memcpy(crypt, msgdata+CUID_SIZE+RANDNUM_SIZE, ENC_SIZE);
			reader->isCustomKey = scbData[0];

			// Process SCS_12
			// c) computer a set of session keys
			// d) generate the Server and Client Cryptogram
			//SetSecureChannelInfo(reader, CRYPTOGRAM);
			SetSecureChannelInfo(reader, DERIVEKEY);

			// a) Verify the PD's cryptogram
			if (memcmp(crypt, reader->cryptB, ENC_SIZE)){
				EyelockLog(logger, ERROR, "Verify the PD's cryptogram failed");
				DumpBuffer(reader->cryptB, ENC_SIZE);
				return false;
			}

			memcpy(data, reader->cryptA, ENC_SIZE);
			m_CommandReader.msgCode = osdp_SCRYPT;
			m_CommandReader.msgLength = ENC_SIZE;
			scb->scbLen = 3;
			scb->scbType = SCS_13;
			scb->scbData[0] = reader->isCustomKey;	// default key = 0
			break;
		}
		case SCS_14:
		{
			datalen = ENC_SIZE;
			if (scblen != 3 || command != osdp_RMAC_I || datalen != length-(HDLEN+scblen+1)){
				if (m_osdpReaderDebug)
					EyelockLog(logger,  DEBUG, "Invalid SCS_14");
				return false;
			}

			if (scbData[0] == 0xFF) {
				// Server Cryptogram failed, start SCS-11 again
				SetupReaderSCS(NULL, 0, true);
			}
			else if (scbData[0] == 0x01){
				memcpy(reader->mac_R, msgdata, ENC_SIZE);
				if (!reader->isCustomKey && reader->chanStat != KEYSET){
					// send osdp_KEYSET
					data[0] = 0x01;	// scbk
					data[1] = ENC_SIZE;
					SetSecureChannelInfo(reader, SCBK);
					if(m_fjiaDebug) {
						unsigned char mk[] = {0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F};
						memcpy(reader->scbk, mk, ENC_SIZE);
					}
					memcpy(data+2, reader->scbk, ENC_SIZE);
					m_CommandReader.msgCode = osdp_KEYSET;
					m_CommandReader.msgLength = ENC_SIZE+2;
					scb->scbLen = 2;
					scb->scbType = SCS_17;
				}
				else{
					m_CommandReader.msgCode = osdp_POLL;
					m_CommandReader.msgLength = 0;
					scb->scbLen = 2;
					scb->scbType = SCS_15;
					reader->chanStat = CONNECTED;
				}
			}

			break;
		}
		case SCS_16:
		case SCS_18:
		{
			// set default osdp_POLL
			scb->scbLen = 2;
			scb->scbType = SCS_15;

			datalen = 0; 	// does not contain encrypted DATA, e.g osdp_POLL
			unsigned char comdata[128];
			memset(comdata, 0, 128);

			if (scb->scbLen != 2)
				return false;
			if (!ValidateMAC(msg, length, false)) {
				EyelockLog(logger, DEBUG, "Invalid MAC data");
				return false;
			}

			if (scb->scbType == SCS_18) {
				datalen = length-pos-1-MAC_BYTES;
				if (datalen % ENC_SIZE) {
					EyelockLog(logger, DEBUG, "Invalid encrypted data length %d", datalen);
					return false;
				}
				DecryptOSDPData(data, comdata, m_ToReader.enc, m_ToReader.mac_R, datalen);
			}
			bool result = ProcessReaderReply(command, comdata, datalen);
			if (result == false) {
				EyelockLog(logger,  DEBUG, "ProcessReaderReply() failed, command 0x%x, datalen %d", command, datalen);
				return false;
			}

			if (m_ToReader.chanStat != SETUP){
				// create the command message CP->PD
				if (m_CommandReader.msgLength == 0) {
					// Command SCS_15
					m_CommandReader.msgSCB.scbLen = 2;
					m_CommandReader.msgSCB.scbType = SCS_15;
				}
				else {
					// Command SCS_17
					m_CommandReader.msgSCB.scbLen = 2;
					m_CommandReader.msgSCB.scbType = SCS_17;

				}
			}
			break;
		}
		default:
			break;
	}

	if (m_CommandReader.msgSCB.scbType == SCS_17) {
		unsigned char *pddata = m_CommandReader.msgData;
		int len = m_CommandReader.msgLength;

		int newlen = DataPadAndEncrypt(&m_ToReader, pddata, len);
		if (newlen == 0){
			EyelockLog(logger, ERROR, "Failed in data encrypt()");
			return false;
		}
		m_CommandReader.msgLength = newlen;
	}
	return true;

}

void OSDPMessage::ProcessOSDPLedCommandFromACS(LEDConsolidator *ledConsolidator)
{
	if(m_osdpDebug)
		EyelockLog(logger, TRACE, "ProcessOSDPLedCommandFromACS() Start m_ledConsolidator %d, ledCommand %d", ledConsolidator, m_osdpLedControl.ledCommand);
	if(m_osdpLedControl.ledCommand && ledConsolidator && m_osdpLedControl.ledData[2]) {
		// m_osdpLedMsg[0] - reader number, m_osdpLedMsg[1] - LED number
		unsigned int onTimer = (unsigned int)m_osdpLedControl.ledData[3] * 100;
		unsigned int offTimer = (unsigned int)m_osdpLedControl.ledData[4] * 100;
		char onColor = m_osdpLedControl.ledData[5];
		char offColor = m_osdpLedControl.ledData[6];
		unsigned short tempTimer = ((((unsigned short)m_osdpLedControl.ledData[8]) << 8) + (unsigned short)m_osdpLedControl.ledData[7]) * 100; // in MS
		//printf("\n====   start %llu, elapsed %llu\n", m_osdpLedMsg.timestart, timeelapsed);
		if (onColor == offColor || offTimer == 0)
			onTimer = tempTimer;

		CURR_TV_AS_MSEC(timecurr);
		if(m_osdpDebug) {
			EyelockLog(logger, TRACE, "ledTime %d, curr %llu, start %llu\n", tempTimer, timecurr, m_osdpLedControl.timestart);
			//DumpBuffer((unsigned char *)m_osdpLedControl.ledData, 14);
		}
		if (!m_osdpLedControl.timestart)
			m_osdpLedControl.timestart = timecurr;

		uint64_t timeelapsed = timecurr-m_osdpLedControl.timestart;

		LEDResult l;
		l.setState(LED_NWSET);
		l.setGeneratedState(eREMOTEGEN);

		if (timeelapsed < (uint64_t)tempTimer) {
			if (m_fjiaDebug)
				printf("==== temp %d, curr %llu, start %llu, onstart %llu, offstart %llu\n", tempTimer, timecurr, m_osdpLedControl.timestart, m_osdpLedControl.onstart, m_osdpLedControl.offstart);

			//set the on color
			if (timeelapsed == 0 || (m_osdpLedControl.offstart && timecurr-m_osdpLedControl.offstart >= offTimer)) {
				if(m_osdpDebug)
					EyelockLog(logger, DEBUG, "ProcessOSDPLedCommandFromACS() - OSDP LED ON: color %d, time %d ", onColor, onTimer);

				if (timeelapsed == 0) {
					ledConsolidator->m_changeLedState = false;
				}

				// set LED ON
				m_osdpLedControl.offstart = 0;
				m_osdpLedControl.onstart = timecurr;

				if (onColor == 1)
					l.setNwValandSleep(24, 2*onTimer*1000);		// red = 24, time in us
				else if (onColor == 2)
					l.setNwValandSleep(4, 2*onTimer*1000);		// green = 4,
				else
					l.setNwValandSleep(0, 2*onTimer*1000);		// off = 0,
				ledConsolidator->enqueMsg(l);

			}
			if (m_osdpLedControl.onstart && timecurr-m_osdpLedControl.onstart >= (uint64_t)onTimer) {
				if(m_osdpDebug)
					EyelockLog(logger, DEBUG, "ProcessOSDPLedCommandFromACS() - OSDP LED OFF: color %d, time %d ", offColor, offTimer);

				// set LED OFF
				m_osdpLedControl.onstart = 0;
				m_osdpLedControl.offstart = timecurr;

				if (offColor == 1)
					l.setNwValandSleep(24, 2*offTimer*1000);	// red = 24, set 2*offTimer to avoid gap
				else if (offColor == 2)
					l.setNwValandSleep(4, 2*offTimer*1000);		// green = 4,
				else
					l.setNwValandSleep(0, 2*offTimer*1000);		// off = 0,
				ledConsolidator->enqueMsg(l);
			}
		}
		else {
			if(m_osdpDebug)
				EyelockLog(logger, DEBUG, "ProcessOSDPLedCommandFromACS() - OSDP LED DONE: time curr %llu, time start %llu ", timecurr, m_osdpLedControl.timestart);
			ledConsolidator->m_changeLedState = true;
			memset((void *)&m_osdpLedControl, 0, sizeof(OsdpACSLEDCommand));
			l.setState(LED_INITIAL);
			ledConsolidator->enqueMsg(l);
		}

		m_osdpLedControl.changeLed = false;
	}
//	else {
//		memset((void *)&m_osdpLedControl, 0, sizeof(OsdpACSLEDCommand));
//	}
}


OSDPMessage::~OSDPMessage()
{
	m_ShouldIQuit = 1;
}




void OSDPMessage::SaveMatchData(char *msg, int bytes, int bits)
{
	m_osdpByteLength = bytes;
	m_osdpBitLength = bits;
	memcpy(osdpCardData, msg, bytes);
	m_osdpCardDataAvailable = true;

}
bool OSDPMessage::ValidateHeader(unsigned char *msg, int dataLength, bool panel)
{
	if(msg[0] != osdp_HEADER || dataLength < 7)
		return false;

	unsigned char address = msg[1];
	if (panel){
		if (address != 0x7F && address != m_OSDPAddr)	// invalid address, message is not for this nano
			return false;
	}
	else {
		if (address != 0xFF && address != 0x80)
			return false;
	}

	//the next 2 bytes are the length of the whole message from SOM to CKSUM
	unsigned short datalen = *((unsigned short*)(msg + 2));  //msg + 2 is the pointer for msg[2]
	if(datalen > 128 || datalen != dataLength)
		return false;

	return true;
}


bool OSDPMessage::ValidateCRC(unsigned char *msg, int dataLength, bool crc)
{
	//if we have a CRC, then the crc location is datalen - 2
	//if we have a CKSUM then the checksum is datalen -1
	int crcksumpos = dataLength -1;
	bool messageOK = false;
	if(crc)
	{
		crcksumpos -= 1;
		unsigned short crc = getShortData(msg + crcksumpos);
		unsigned short testCrc = OSDPCRC(msg, crcksumpos);
		messageOK = (crc == testCrc) ? true:false;
	}
	else
	{
		//checksum
		unsigned char cksum = msg[crcksumpos];
		unsigned char testcksum = calcChecksum(msg, crcksumpos); //everything but the checksum character!
		messageOK = (cksum == testcksum) ? true:false;
	}

	return messageOK;
}

bool OSDPMessage::ValidateMAC(unsigned char *msg, int dataLength, bool panel)
{
	unsigned char mac[4];
	int macpos = dataLength-MAC_BYTES;
	OsdpSecureChannel *chan;
	bool isMAC_C;
	unsigned char *newMAC;
	if (m_fjiaDebug){
		printf("ValidateMAC: len %d, ", dataLength);
		DumpBuffer(msg, dataLength);
	}
	// extract MAC
	for (int i=0; i<MAC_BYTES; i++){
		mac[i] = msg[macpos+i];
		msg[macpos+i] = 0;
	}

	if (panel){
		isMAC_C = true;
		chan = &m_ToPanel;
		newMAC = chan->mac_C;
	}
	else {
		isMAC_C = false;
		chan = &m_ToReader;
		newMAC = chan->mac_R;
	}

	if (GenerateMAC(chan, msg, macpos, isMAC_C)) {
		if (memcmp(mac, newMAC, MAC_BYTES) == 0)
			return true;
	}

	if (m_fjiaDebug){
		printf("Expected newMAC: ");
		DumpBuffer(newMAC, ENC_SIZE);
	}
	EyelockLog(logger, ERROR, "MAC failed");
	return false;

}
void OSDPMessage::CalculateCRC(unsigned char *reply, short *replyLen, bool crc)
{
	if (m_fjiaDebug){
		printf("CalculateCRC() crc %d, length %d, msg:\n", crc, *replyLen);
		DumpBuffer(reply, *replyLen);
	}
	//if we have a CRC, then the crc location is datalen - 2
	//if we have a CKSUM then the checksum is datalen -1
	unsigned char *replyLengthPos = reply+2;
	int pos = *replyLen;
	if(crc)
	{
		*replyLen += 2;
		memcpy(replyLengthPos, replyLen, 2);
		unsigned short scrc = OSDPCRC(reply, pos);
		memcpy((reply+pos), &scrc, 2);
	}
	else
	{
		*replyLen += 1;
		memcpy(replyLengthPos, replyLen, 2);
		unsigned char cksum = calcChecksum(reply, pos);
		reply[pos] = cksum;
	}
}

// OSDP Message Field
void OSDPMessage::parseControl(unsigned char ctrl, int & sqn, bool & crc, bool & scb, bool & multi)
{
	//the control is the bitmask set
	//0,1 are sqn
	//2 is crc
	//3 is scb
	//4-6, 7 are deprecated
	sqn = ctrl & 0x3;
	crc = ((ctrl & 0x4) != 0);
	scb = ((ctrl & 0x8)!= 0);
	multi = false;
}
void OSDPMessage::buildControl(unsigned char & ctrl, int sqn, bool crc, bool scb, bool multi)
{
	//to build the control we start with the CRc then just add the other bits
	int ret = sqn;
	//reply the same sqn to
	if(crc)
		ret += 0x4;
	if(scb)
		ret += 0x8;
	if(multi)
		ret += 0x80;
	ctrl = (unsigned char) ret;
}


//here and below:  Crc table
static uint16 nCrcTblValid = 0;
static uint16 cCrcTable[256];

unsigned short OSDPMessage::OSDPCRC(unsigned char * data, int len)
{
	uint16 nCrc = 0;
	int ii = 0 ;
	if(nCrcTblValid == 0)
	{
		nCrcTblValid = fCrcTblInit(cCrcTable);
	}
	nCrc = 0x1d0f;
	for(ii = 0; ii < len; ii++)
	{
		nCrc = (nCrc<<8) ^ cCrcTable[((nCrc>>8) ^ data[ii]) & 0xFF];
	}
	return nCrc;

}

int OSDPMessage::DataPadAndEncrypt(OsdpSecureChannel *chan, unsigned char *msg, int len)
{
	if(m_fjiaDebug) {
		EyelockLog(logger, TRACE, "DataPadAndEncrypt() - CP %d, msg:", chan->isCP);
		DumpBuffer(msg, len);
	}
	/*
	 * Append the character 0x80 to the data block, then continue to append as many characters of 0x00 as
	 * are required to make the size of the data block to be evenly divisible by the block size of 16.
	 */

	int pos = 0;
	unsigned char icv[ENC_SIZE] = {0};
	//unsigned char mac[ENC_SIZE] = {0};
	//unsigned char icv[ENC_SIZE] = {0};
	//unsigned char *macc, macr;
	int block = len/ENC_SIZE;
	unsigned char data[128];

	for (int i=0; i<ENC_SIZE; i++){
		if (!chan->isCP)
			icv[i] = ~chan->mac_C[i];
		else
			icv[i] = ~chan->mac_R[i];
	}

	memcpy(data, msg, len);
#if 0
	for (i=0; i<block && pos+ENC_SIZE<len; i++) {
		memcpy(data+i*ENC_SIZE, msg+i*ENC_SIZE, ENC_SIZE);
		//EncryptOSDPData(data, msg+i*ENC_SIZE, chan->enc, icv);
		pos += ENC_SIZE;
	}

	// padding
	int left = len % ENC_SIZE;
	if (left) {
		pos += left;
		msg[pos] = 0x80;
		pos++;
		while (pos < (block+1)*ENC_SIZE) {
			msg[pos] = 0;
			pos++;
		}
	}

	//if ( i != 0)
		//memcpy(icv, mac, ENC_SIZE);
	memcpy(data, msg+i*ENC_SIZE, ENC_SIZE);
#endif
	// padding
	int left = len % ENC_SIZE;
	if (left) {
		pos = len;
		data[pos++] = 0x80;
		while (pos < (block+1)*ENC_SIZE) {
			data[pos++] = 0;
		}
	}
	if (m_fjiaDebug) {
		printf("icv: ");
		DumpBuffer(icv, ENC_SIZE);
		printf("data: ");
		DumpBuffer(data, pos);
	}
	EncryptOSDPData(data, msg, chan->enc, icv, pos);

	if (m_fjiaDebug) {
		printf("encrypted data: ");
		DumpBuffer(msg, pos);
	}
	if ((pos < len) || (pos % ENC_SIZE))
			return 0;
	return pos;

}

bool OSDPMessage::GenerateMAC(OsdpSecureChannel *chan, unsigned char *msg, int len, int isMAC_C)
{
	if (m_fjiaDebug) {
		EyelockLog(logger, TRACE, "GenerateMAC() start - MAC_C %d, msgLen %d", isMAC_C, len);
		DumpBuffer(msg, len);
	}
	/*
	 * C-MAC – the ICV value for generating the C-MAC is the previously received R-MAC (MAC_I)
	 * R-MAC – the ICV value for generating the R-MAC is the previously received C-MAC
	 */

	int pos = 0;
	unsigned char data[ENC_SIZE] = {0};
	unsigned char mac[ENC_SIZE] = {0};
	unsigned char icv[ENC_SIZE] = {0};
	int block = len/ENC_SIZE;
	int i = 0;

	if (isMAC_C)
		memcpy(icv, chan->mac_R, ENC_SIZE);
	else
		memcpy(icv, chan->mac_C, ENC_SIZE);

	for (i=0; i<block && pos+ENC_SIZE<len; i++) {
		if (m_fjiaDebug){
			printf("icv: ");
			DumpBuffer(icv, ENC_SIZE);
		}
		memcpy(data, msg+i*ENC_SIZE,ENC_SIZE);
		EncryptOSDPData(data, mac, chan->mac1, icv, ENC_SIZE);
		pos += ENC_SIZE;
		memcpy(icv, mac, ENC_SIZE);
		if (m_fjiaDebug){
			printf("block %d, data: ", i);
			DumpBuffer(data, ENC_SIZE);
			DumpBuffer(mac, ENC_SIZE);
		}
	}

	// padding
	int left = len % 16;
	memcpy(data, msg+i*ENC_SIZE, left);
	if (left) {
		pos = left;
		data[pos] = 0x80;
		pos++;
		while (pos < ENC_SIZE) {
			data[pos++] = 0;
		}
	}

	EncryptOSDPData(data, mac, chan->mac2, icv, ENC_SIZE);
	if (m_fjiaDebug){
		printf("icv: ");
		DumpBuffer(icv, ENC_SIZE);
		printf("padding data: ");
		DumpBuffer(data, ENC_SIZE);
		DumpBuffer(mac, ENC_SIZE);
	}
	if (isMAC_C)
		memcpy(chan->mac_C, mac, ENC_SIZE);
	else
		memcpy(chan->mac_R, mac, ENC_SIZE);

	return true;
}

bool OSDPMessage::SetSecureChannelInfo(OsdpSecureChannel *chan, SECURETYPE type)
{
	if (m_osdpReaderDebug || m_osdpDebug)
		EyelockLog(logger,  DEBUG, "SetSecureChannelInfo() start - type %d", type);
	unsigned char defaultIV[ENC_SIZE];
	unsigned char data[ENC_SIZE];
	memset(data, 0, ENC_SIZE);
	memset(defaultIV, 0, ENC_SIZE);
	switch (type)
	{
		case RAND:
		{
			FILE* fr = fopen("/dev/urandom", "r");
			if (!fr)
				EyelockLog(logger, ERROR, "Open /dev/urandom failed");
			else {
				unsigned char *random;
				if (chan->isCP)
					random = chan->randA;
				else
					random = chan->randB;
				int result = fread(random, sizeof(char), RANDNUM_SIZE, fr);
			    if (result != RANDNUM_SIZE)
			    	EyelockLog(logger, ERROR, "Get random number failed");
			    fclose (fr);
			}
			break;
		}
		case SCBK:
			/*
			 * SCBK = Enc( cUID || (~cUID), MK )
			 */
			memcpy(data, chan->cuid, CUID_SIZE);
			for (int i=0; i<RANDNUM_SIZE; i++)
			{
			    data[8+i] = ~chan->cuid[i];
			};
			EncryptOSDPData(data, chan->scbk, chan->mk, defaultIV, ENC_SIZE);
			break;
		case DERIVEKEY:
			/*
			 * S-ENC: 0x01,0x82,rnd[0],rnd[1],rnd[2],rnd[3],rnd[4],rnd[5],0,0,…
			 * S-MAC1: 0x01,0x01,rnd[0],rnd[1],rnd[2],rnd[3],rnd[4],rnd[5],0,0,…
             * S-MAC2: 0x01,0x02,rnd[0],rnd[1],rnd[2],rnd[3],rnd[4],rnd[5],0,0,…
			 */
			data[0] = 0x01;
			data[1] = 0x82;
			data[2] = chan->randA[0];
			data[3] = chan->randA[1];
			data[4] = chan->randA[2];
			data[5] = chan->randA[3];
			data[6] = chan->randA[4];
			data[7] = chan->randA[5];
			EncryptOSDPData(data, chan->enc, chan->scbk, defaultIV, ENC_SIZE);
			if (m_fjiaDebug){
				printf("scbk: ");
				DumpBuffer(chan->scbk, ENC_SIZE);
				printf("enc: ");
				DumpBuffer(chan->enc, ENC_SIZE);
			}

			data[0] = 0x01;
			data[1] = 0x01;
			EncryptOSDPData(data, chan->mac1, chan->scbk, defaultIV, ENC_SIZE);

			data[0] = 0x01;
			data[1] = 0x02;
			EncryptOSDPData(data, chan->mac2, chan->scbk, defaultIV, ENC_SIZE);
			if (m_fjiaDebug){
				printf("mac2: ");
				DumpBuffer(chan->mac2, ENC_SIZE);
				printf("mac1: ");
				DumpBuffer(chan->mac1, ENC_SIZE);
			}
			break;
		case CRYPTOGRAM:
			/*
			 * ClientCryptogram = ENC( RND.A[8] || RND.B[8], S-ENC )
			 * ServerCryptogram = ENC( RND.B[8] || RND.A[8], S-ENC )
			 */
			memcpy(data, chan->randA, 8);
			memcpy(data+8, chan->randB, 8);
			EncryptOSDPData(data, chan->cryptB, chan->enc, defaultIV, ENC_SIZE);

			memcpy(data, chan->randB, 8);
			memcpy(data+8, chan->randA, 8);
			EncryptOSDPData(data, chan->cryptA, chan->enc, defaultIV, ENC_SIZE);
			if (m_fjiaDebug){
				printf("cryptA: ");
				DumpBuffer(chan->cryptA, ENC_SIZE);
				printf("cryptB: ");
				DumpBuffer(chan->cryptB, ENC_SIZE);
			}
			break;
		case MAC_I:
			/*
			 * Initial R-MAC (osdp_RMAC_I)
			 */
			memcpy(data, chan->cryptA, 16);
			EncryptOSDPData(data, chan->mac_I, chan->mac1, defaultIV, ENC_SIZE);
			memcpy(data, chan->mac_I, 16);
			EncryptOSDPData(data, chan->mac_I, chan->mac2, defaultIV, ENC_SIZE);
			// store for mac_R
			memcpy(chan->mac_R, chan->mac_I, ENC_SIZE);
			break;

		default:
			EyelockLog(logger, ERROR, "Invalid BLE Mode");
			break;
	}
	return true;
}
bool OSDPMessage::EncryptOSDPData(const unsigned char *in, unsigned char *out, unsigned char *key, unsigned char *iv, int length)
{
	AES *myAES = new AES();
	if (!myAES)
		return false;
	myAES->SetKey(key,ENC_SIZE);	// 16 bytes key
	if (iv)
		myAES->SetIV(iv,ENC_SIZE);
	myAES->EncryptAes128(in,out,length);
	delete myAES;
	return true;
}
bool OSDPMessage::DecryptOSDPData(const unsigned char *in, unsigned char *out, unsigned char *key, unsigned char *icv, int length)
{
	if (m_fjiaDebug){
		printf("DecryptOSDPData() in: ");
		DumpBuffer((unsigned char *)in,length);
		printf("DecryptOSDPData() key: ");
		DumpBuffer(key, ENC_SIZE);
		printf("DecryptOSDPData() icv: ");
		DumpBuffer(icv, ENC_SIZE);
	}

	unsigned char iv[ENC_SIZE];
	for (int i=0; i<ENC_SIZE; i++){
		iv[i] = ~icv[i];
	}

	AES *myAES = new AES();
	if (!myAES)
		return false;
	myAES->SetKey(key,ENC_SIZE);	// 16 bytes key
	myAES->SetIV(iv,ENC_SIZE);

	int size = myAES->DecryptAes128(in,out,length);
	delete myAES;
	if (m_fjiaDebug){
		printf("DecryptOSDPData() out: ");
		DumpBuffer(out, size);
	}

	if (size == length)
		return true;
	else
		return false;
}

void OSDPMessage::DumpBuffer(unsigned char *buf, int bytes)
{
        for (int x=0; x < bytes; x++)
        {
                printf(" 0x%x", buf[x]);
        }
        printf("\n");
}

/*
bool OSDPMessage::UpdateMatchResult(MatchResult * msg)
{
	EyelockLog(logger, TRACE, "UpdateMatchResult() Start");

	MatchResultState state = msg->getState();

	if(state==PASSED){
		int bytes =0,bits=0;
		char *ptr = msg->getF2F(bytes,bits);
		SaveMatchData(ptr, bytes, bits);
	}
	return true;
}
*/


static int fCrcTblInit(uint16 * pTbl)
{
	int ii, jj;
	uint16 ww;
	for(ii =0; ii < 256; ii++)
	{
		ww = (uint16)(ii<<8);
		for(jj = 0; jj < 8; jj++)
		{
			if(ww & 0x8000)
			{
				ww = (uint16)(((uint16)ww << 1) ^ 0x1021);
			}
			else
			{
				ww = (uint16)(ww <<1);
			}
		}
		pTbl[ii] = ww;
	}
	return 1;
}

unsigned char calcChecksum(unsigned char * msg, int len)
{
	unsigned char sum = 0;
	for(int i = 0; i < len; i++)
	{
		sum += msg[i];

	}
	unsigned char ret = (unsigned char)((0x100 - sum) & 0xFF);
	return ret;
}



void buildWaveFile(int length)
{
	FILE * f = fopen("/home/root/tones/osdp.wav", "r");
	FILE * fx = fopen("/home/root/tones/osdpFullLength.wav", "w");

	unsigned char header[44];
	fread(header,1,44,f);
	int * dataSize = (int*)(header +40);
	int * chunkSize = (int*)(header + 4);
	int tempds = *dataSize;
	printf("datasize %d", *dataSize);
	unsigned char * data = new unsigned char[(*dataSize)];
	fread(data, 1, *dataSize, f);

	int writeSize = *dataSize * length;

	fseek(fx, 0, SEEK_SET);
	*dataSize = writeSize;
	*chunkSize = *dataSize + 36;
	fwrite(header,1,44,fx);
	for(int p = 0; p < length; p++)
		fwrite(data,1, tempds, fx);
	fclose(f);
	fclose(fx);
	delete [] data;

}
std::string hexStr(unsigned char *data, int len)
{
    std::stringstream ss;
    ss<<std::hex;
    for(int i(0);i<len;++i)
    {
        if(data[i] <= 0xF)
        	ss<<"0";
    	ss<<(int)data[i];
    }
    return ss.str();
}
unsigned short getShortData(unsigned char * ptr)
{
        return *((unsigned short*)ptr);

}

