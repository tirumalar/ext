/*
 * OSDPMessage.h
 *
 *  Created on: 26-Nov-2009
 *      Author: mamigo
 */

#ifndef OSDPMESSAGE_H_
#define OSDPMESSAGE_H_

#include "Configuration.h"
#include "LEDConsolidator.h"

#define OSDP_RUN_TIME		20		// 20ms
#define ENC_SIZE			16
#define RANDNUM_SIZE		8
#define CUID_SIZE			8
#define CRC_ENABLE			0
#define HDLEN				5
#define MAC_BYTES			4
#define osdp_RESET			0x01

// OSDP COMMAND
#define osdp_HEADER 		0x53
#define osdp_POLL 			0x60
#define osdp_ID 			0x61
#define osdp_CAP			0x62
#define osdp_LSTAT			0x64
#define osdp_RSTAT			0x67
#define osdp_OUT			0x68
#define osdp_LED			0x69
#define osdp_BUZ			0x6A
#define osdp_COMSET			0x6E
#define osdp_KEYSET			0x75
#define osdp_CHLNG			0x76
#define osdp_SCRYPT			0x77

// OSDP REPLY
#define osdp_ACK			0x40
#define osdp_NAK			0x41
#define osdp_PDID			0x45
#define osdp_PDCAP			0x46
#define osdp_LSTATR			0x48
#define osdp_RSTATR			0x4B
#define osdp_RAW			0x50
#define osdp_COM			0x54
#define osdp_CCRYPT			0x76
#define osdp_RMAC_I			0x78

// OSDP SCB TYPE
#define SCS_11				0x11
#define SCS_12				0x12
#define SCS_13				0x13
#define SCS_14				0x14
#define SCS_15				0x15
#define SCS_16				0x16
#define SCS_17				0x17
#define SCS_18				0x18


// typedef unsigned int uint16;


enum SECURETYPE{RAND=0, SCBK, DERIVEKEY, CRYPTOGRAM, MAC_I};
enum SECURESTATE{DEFAULT=0, SETUP, KEYSET, CONNECTED};

struct OsdpACSLEDCommand{
	bool changeLed;
	bool ledCommand;
	char ledData[14];
	uint64_t timestart;
	uint64_t onstart;
	uint64_t offstart;
};
struct OsdpReaderCapCommand{
	char readerID;
	char ledControl;
	char buzControl;
	char ledState;
	char buzState;
	char msgSqn;
	bool msgScb;

};

struct OsdpSecureChannel {
	bool isCP;
	bool isCustomKey;
	unsigned char mk[ENC_SIZE];
	unsigned char scbk[ENC_SIZE];
	unsigned char randA[RANDNUM_SIZE];		// CP - to reader
	unsigned char randB[RANDNUM_SIZE];		// PD - to panel
	unsigned char cuid[CUID_SIZE];			// PD - UID
	unsigned char enc[ENC_SIZE];
	unsigned char mac1[ENC_SIZE];
	unsigned char mac2[ENC_SIZE];
	unsigned char cryptA[ENC_SIZE];		// ServerCryptogram - CP
	unsigned char cryptB[ENC_SIZE];		// ClientCryptogram - PD
	unsigned char mac_I[ENC_SIZE];		// Initial R-MAC - PD
	unsigned char mac_C[ENC_SIZE];		// Command MAC - CP
	unsigned char mac_R[ENC_SIZE];		// Reply MAC - PD
	unsigned char icv[ENC_SIZE];		// Reply MAC - PD
	unsigned char lastBlock[ENC_SIZE];	// last block of message, encrypted by MAC-2
	int chanStat;						// default=0, setup=1, connect=2
};
struct OsdpSCB {
	unsigned char scbLen;
	unsigned char scbType;
	unsigned char scbData[16]; //this won't exceed 16 bytes
};
struct OsdpMessage {
	unsigned char msgCode;
	unsigned char msgData[128]; 	//this won't exceed our expected buffer size
	int msgLength;
	OsdpSCB msgSCB;
};


class OSDPMessage {
public:
	OSDPMessage(Configuration& conf);
	~OSDPMessage();
	const char *getName(){
		return "OSDPMessage";
	}

	//virtual bool UpdateMatchResult(MatchResult *result);
	//void Process();
	int ProcessPanelMsg(unsigned char *msg, int data_length);
	bool ProcessReaderMsg(unsigned char *msg, int data_length);
	void ProcessOSDPLedCommandFromACS(LEDConsolidator *m_ledConsolidator);
	void SendOSDPCommand(int command, unsigned char *data);
	void SaveMatchData(char *msg, int bytes, int bits);

	F2FDispatcher *m_f2fDispatcher;
	bool m_acsChange;
	bool m_tamperChange;
	int m_accessDataLength;
	bool isTamperSet;
	bool isMBTamper;
	bool isBoBTamper;
	bool m_Debug;
	bool m_tamperEnable;
	bool osdpStateChange;

	//OSDP message data
	//set this to true when the tamper state changes (or when there is something to report to an OSDP_POLL other than ACK
	bool stateChanged;
	unsigned char m_OSDPAddr;
	bool m_updateACD;
	OsdpACSLEDCommand m_osdpLedControl; 	// osdp_LED data 14 bytes
	OsdpReaderCapCommand m_osdpReader;
	bool m_osdpDebug, m_osdpReaderDebug;
	char m_osdpACSSqn;
	char osdpCardData[256];
	bool m_ShouldIQuit;

protected:
	OsdpSecureChannel m_ToReader;
	OsdpSecureChannel m_ToPanel;
	OsdpMessage m_ReplyPanel;
	OsdpMessage m_CommandReader;
	//OsdpSCB m_ReplySCBPanel;

	int m_State;
	int m_ODSPSeq;
	bool m_osdpCardDataAvailable;
	bool m_fjiaDebug;


	int m_osdpByteLength;
	int m_osdpBitLength;
	int m_ledControlledByInput;
	int m_OSDPBRate;
	int m_ReaderOSDPBRate;
	bool m_dualAuth;
	bool m_osdpSCEnabled;
	bool m_osdpReaderSCEnabled;
	unsigned char myID[CUID_SIZE];

	void parseControl(unsigned char ctrl, int & sqn, bool & crc, bool & scb, bool & multi);
	void buildControl(unsigned char & ctrl, int sqn, bool crc, bool scb, bool multi);
	unsigned short OSDPCRC(unsigned char * data, int len);
	static void DumpBuffer(unsigned char *buf, int bytes);
	void OSDPBuzCommand(int state);
	void OSDPLedCommand(int state);
	bool SetSecureChannelInfo(OsdpSecureChannel *chan, SECURETYPE type);
	bool EncryptOSDPData(const unsigned char *in, unsigned char *out, unsigned char *key, unsigned char *iv, int length);
	bool DecryptOSDPData(const unsigned char *in, unsigned char *out, unsigned char *key, unsigned char *iv, int length);
	bool ValidateHeader(unsigned char *msg, int dataLength, bool panel);
	bool ValidateCRC(unsigned char *msg, int dataLength, bool crc);
	void CalculateCRC(unsigned char *reply, short *replyLen, bool crc);
	bool ProcessPanelCommand(unsigned char command, unsigned char *msg, int dataLength);
	bool SetupPanelSCS(unsigned char *msg, int dataLength);
	bool GenerateMAC(OsdpSecureChannel *chan, unsigned char *msg, int len, int isMAC_C);
	int DataPadAndEncrypt(OsdpSecureChannel *chan, unsigned char *msg, int len);
	bool SetupReaderSCS(unsigned char *msg, int length, bool start);
	bool ValidateMAC(unsigned char *msg, int dataLength, bool panel);
	bool ProcessReaderReply(unsigned char response, unsigned char *data, int dataLength);
	void BuildOSDPCommand(int command, unsigned char *data);

};

#endif /* OSDPMessage_H_ */
