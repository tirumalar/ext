/*
 * PortableTemplate.cpp
 *
 *  Created on: 16-Oct-2016
 *      Author: fjia
 */
#include <unistd.h>
#include "PortableTemplate.h"
#include "Configuration.h"
//#include "I2CBus.h"
//#include "Synchronization.h"
#include "UtilityFunctions.h"
#include "OpenSSLSupport.h"
#include "AESClass.h"
#include "logging.h"
#include <iostream>
extern "C" {
#include "BobListener.h"
}
const char logger[30] = "PortableTemplate";

PortableTemplate::PortableTemplate(Configuration& conf) : MatchType(conf)
{
	if(m_Debug)
		EyelockLog(logger, TRACE, "PortableTemplate Created %d\n", this);
	m_tocCardExpiredTime = conf.getValue("GRITrigger.TOCCardExpiredTime", 10) * 1000;	// 10 sec = 10*1000 ms
	m_tocMobileMode = conf.getValue("GRITrigger.MobileMode",2);
	m_tocCustomKey = conf.getValue("GRITrigger.PortableTemplatesUseCustomKey", false);
	m_tocCustFile = 0;
	m_tocCustPW = 0;

	m_numOfCard = 0;
	m_tocCardIndex = 0;
	m_TOCReaderBLEConfig = TOC_BLE_INIT;

	m_pCardData = (unsigned char *)malloc(3000);
	if (m_pCardData == NULL)
		EyelockLog(logger, ERROR, "Failed at malloc m_pCardData");

	if (access("TOCReaderVersion", F_OK ) != -1) {
		if( remove( "TOCReaderVersion" ) != 0 )
			EyelockLog(logger, ERROR, "Failed to remove TOCReaderVersion file");
	}
	// set command to TOC reader to config BLE mode and timeout
	TOCReaderBLEConfig();
}

PortableTemplate::~PortableTemplate() {
	if (m_tocCustFile) {
		free((void *)m_tocCustFile);
		free((void *)m_tocCustPW);
	}
}

void PortableTemplate::initMatchBuffer()
{
	// clear DB
	if(m_matchManager) {
		m_matchManager->DeleteAllUser();
		m_matchManager->ClearLocalMatchBuffer();
	}
}

bool PortableTemplate::ValidateCard()
{
	if(m_Debug)
		EyelockLog(logger, TRACE, "PortableTemplate::ValidateCard() start - card length %d", m_cardLen);

	unsigned char *tocData = m_pCardData;
	int bytes = m_cardLen;

	// Check TOC data
	if (m_TOCReaderBLEConfig){
		return false;
	}

	// process TOC data
	// { Main packet TLV  [ Payload TLV {payload V (permuted)} crc TLV {crc V (not permuted)} ]  }
	// Template: 0x0E 0x82 0xLH 0xLL 0x39 0x82 0xELH 0xELL data

	if (tocData[0] != 0x0E || tocData[1] != 0x82 || bytes < 2000){
		EyelockLog(logger, ERROR, "Read card data failed - invalid message");
		return false;
	}

	short len = tocData[2];
	len = (len << 8) + tocData[3];

	if (bytes != len + 4){
		EyelockLog(logger, ERROR, "Read card data failed - invalid message length");
		return false;
	}

	if (checkTOCdata(&tocData[4]))
	{
		bool ret = false;
		if (m_matchManager) {
			// update matchBuffer and save card data
			unsigned char *payload = &tocData[8];		// 8 bytes head
			ret = m_matchManager->AddUserInLocalMatchBuffer(payload);
			if (ret == true) {
				char *pUser = m_pUserData[m_tocCardIndex-1];
				memcpy(pUser, payload, (m_accessDataLength+7)/8);
				if (m_numOfCard == 1) {
					memcpy(m_pCardData, payload, (m_accessDataLength+7)/8);
					m_matchIndex = m_tocCardIndex-1;
				}
				if(m_Debug)
					EyelockLog(logger, DEBUG, "number of cards %d available", m_numOfCard);
			}
			else {
				EyelockLog(logger, ERROR, "Invalid card data");
			}
		}
		return ret;
	}

	return false;

}

bool PortableTemplate::CheckReaderData(){
	if(m_Debug)
		EyelockLog(logger, TRACE, "PortableTemplate::CheckReaderData() Start");

	unsigned char *tocData = m_pCardData;

	if (m_TOCReaderBLEConfig){
		TOCReaderBLEConfigResp(tocData);
		return true;
	}
	return false;
}

bool PortableTemplate::UpdateMatchResult(MatchResult *msg)
{
	MatchResultState state = msg->getState();

	if (m_dualAuthMatched == CARD_MATCHED && state!=PASSED) {
		msg->setState(FAILED);
	}

	return true;
}

bool PortableTemplate::addUser(string perid, string name, string acd)
{
	if(m_Debug)
		EyelockLog(logger, TRACE, "PortableTemplate::addUser() Start");

	if (m_pUserData == NULL || m_tocCardIndex >= MAX_USERS)
		return false;

	int acslen = (m_accessDataLength+7)/8;
	int len = acslen + NAME_SIZE+ sizeof(int) + GUID_SIZE;
	char *data_ptr = (char *)malloc(len);
	if (data_ptr == NULL) {
		EyelockLog(logger, ERROR, "PortableTemplate::addUser() - malloc failed ");
		return false;
	}

	*data_ptr = m_tocCardExpiredTime/CHECK_TOC_TIME; // number of 10sec
	memcpy(data_ptr, acd.c_str(), acslen);
	memcpy(data_ptr+acslen, name.c_str(), NAME_SIZE);
	char *tocdata_ptr = data_ptr + acslen + NAME_SIZE;
	*tocdata_ptr = m_tocCardExpiredTime/CHECK_TOC_TIME; // number of 10sec
	memcpy(tocdata_ptr+sizeof(int), perid.c_str(), GUID_SIZE);

	m_pUserData[m_tocCardIndex] = data_ptr;
	m_tocCardIndex++;
	m_numOfCard++;

	if(m_Debug)
		EyelockLog(logger, DEBUG, "Add TOC user %s ", perid.c_str());

	return true;
}

bool PortableTemplate::timeoutUser()
{

	bool result = false;

	if (m_numOfCard == 0 || m_pUserData == NULL)
		return result;

	int acslen = (m_accessDataLength+7)/8;
	for (int i=0; i<m_tocCardIndex; i++) {
		if (m_pUserData[i]){
			char *tocUser = m_pUserData[i] + acslen + NAME_SIZE;
			int time = *tocUser;
			string perid = string(tocUser+4, tocUser+4+GUID_SIZE);
			time--;
			printf("++++++ %d, %d, time %d\n", m_tocCardIndex, i, time);
			if (time == 0) {
				if(m_Debug)
					EyelockLog(logger, DEBUG, "TOC user timeout %s, index %d, numOfCard %d", perid.c_str(), i, m_numOfCard);
				// delete user
				result = deleteUser(perid);
				//EyelockEvent("Match failure, no iris present - %s", perid.c_str());
			}
			else
				*tocUser = time;
		}
	}

	if (m_numOfCard)
		result = false;
	else
		result = true;

	return result;
}

bool PortableTemplate::deleteUser(string perid)
{
	bool result = false;

	if (m_numOfCard == 0 || m_pUserData == NULL)
		return false;

	int acslen = (m_accessDataLength+7)/8;
	for (int i=0; i<m_tocCardIndex; i++) {
		if (m_pUserData[i]){
			char *pTocUser = m_pUserData[i] + acslen + NAME_SIZE;
			if (!memcmp(pTocUser+4,perid.c_str(),GUID_SIZE)) {
				if(m_Debug)
					EyelockLog(logger, DEBUG, "Delete TOC user %s ", perid.c_str());
				// delete user
				if (m_matchManager)
					m_matchManager->DeleteUserInLocalMatchBuffer(perid);
				free(m_pUserData[i]);
				m_pUserData[i] = NULL;
				m_numOfCard--;
				break;
			}
		}
	}

	if (m_numOfCard == 0) {
		m_tocCardIndex = 0;
		if(m_matchManager)
			m_matchManager->ClearLocalMatchBuffer();
	}
	else if (m_numOfCard == 1){
		for (int i=0; i<m_tocCardIndex; i++) {
			if (m_pUserData[i]){
				memcpy(m_pCardData, m_pUserData[i], (m_accessDataLength+7)/8);
				m_matchIndex = i;
				break;
			}
		}
	}
	else {
		m_matchIndex = -1;
	}

	return result;
}

uint16_t CalculateCrc16(uint8_t *pData, uint16_t numBytes);
bool PortableTemplate::checkTOCdata(unsigned char *pData)
{
	if(m_Debug)
		EyelockLog(logger, TRACE, "PortableTemplate::checkTOCdata() Start");

	unsigned short len = pData[2];
	len = (len << 8) + pData[3];

	if (pData[0] != 0x39)	// DATA_EYELOCK_TEMPLATE	0x39
		return false;
	if (pData[1] != 0x82)
		return false;

	// check CRC
	unsigned char *pCRC = pData + 4 + len;
	unsigned short crc = pCRC[2];
	crc = (crc << 8) + pCRC[3];
	if (pCRC[0] != 0x0C || pCRC[1] != 0x02)	// CRC:	T=0x0C L=0x02
		return false;
	if (CalculateCrc16(pData, len+4) != crc)
		return false;

#ifdef TOC_DATA_RSA_AES
	static int fileReady = 0;
	OpenSSLSupport *mySSL;
	AES *myAes;
	unsigned char pbuffer[5000] = {0};
	if (!fileReady) {
		m_tocCustFile = (char *)calloc(100,1);
		m_tocCustPW = (char *)calloc(50,1);
		if (!m_tocCustomKey) {
			strcpy(m_tocCustFile, "./rootCert/certs/eyelock-pc.key");
			strcpy(m_tocCustPW, "eyelock");
		}
		else {
			const unsigned char fileKey[32] = {0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF,0x00,0x11,0x22,0x33,0x44,0x55,0x66,0x77,0x88,0x99,0xAA,0xBB,0xCC,0xDD,0xEE,0xFF};
			const unsigned char fileIv[16] = {0xFF,0xEE,0xDD,0xCC,0xBB,0xAA,0x99,0x88,0x77,0x66,0x55,0x44,0x33,0x22,0x11,0x00};
			myAes = new AES();
			myAes->SetKey(fileKey,32);	// 32 bytes key
			myAes->SetIV(fileIv,16);	// 16 bytes iv
			int filelen = myAes->DecryptFile("/home/portableTemplateCerts/selectedPFX.enc",pbuffer);
			//EyelockLog(logger, DEBUG, "decrypted selectedPFX.enc %s\n", pbuffer);
			//DumpBuffer((unsigned char *)pbuffer, 100);
			delete myAes;

			if (filelen > 0){
				unsigned char *ppasswd = (unsigned char*) memchr ((const char *)pbuffer, '\n', strlen((const char *)pbuffer));
				memcpy(m_tocCustFile, pbuffer, (int)(ppasswd-pbuffer-1));	// carriage return - extra char
				m_tocCustFile[ppasswd-pbuffer-1] = '\0';
				sprintf(m_tocCustPW, "%s", ppasswd+1);
				if (m_tocCustPW[0] == '0' && m_tocCustPW[1] == '0')
					strcpy(m_tocCustPW, "");
			}
			else {
				EyelockLog(logger, ERROR, "Failed to decrypt selectedPFX.enc");
				return false;
			}
			// openssl pkcs12 -in testcert.pfx -passin pass:”” -nocerts -out load.key -passout pass:""
			char temp[200];
			sprintf(temp, "openssl pkcs12 -in %s -passin pass:%s -nocerts -out load.key -passout pass:eyelock" , m_tocCustFile, m_tocCustPW);
			//EyelockLog(logger, DEBUG, "decrypted cmd %s\n", temp);
			RunSystemCmd(temp);
			//sleep(1);
			strcpy(m_tocCustFile, "load.key");
			strcpy(m_tocCustPW, "eyelock");
		}
		fileReady = 1;
	}

	unsigned char keyDecrypt[128] = {0};
	unsigned char ivDecrypt[128] = {0};
	int result;
	// [128 bytes RSA encrypted key] [128 bytes RSA encrypted iv] [remaining bytes AES encrypted payload]
	unsigned char *key = &pData[4];
	unsigned char *iv = &pData[4] + 128;
	unsigned char *aesData = &pData[4] + 128 * 2;
	len = len - 128 * 2;
	//mySSL = new OpenSSLSupport();
	result = OpenSSLSupport::instance().privateDecrypt(key, keyDecrypt, m_tocCustFile, m_tocCustPW);
	if (result == -1) {
		EyelockLog(logger, ERROR, "Failed to decrypt key");
		fileReady = 0;
		return false;
	}
	result = OpenSSLSupport::instance().privateDecrypt(iv, ivDecrypt, m_tocCustFile, m_tocCustPW);
	if (result == -1) {
		EyelockLog(logger, ERROR, "Failed to decrypt iv");
		fileReady = 0;
		return false;
	}

	myAes = new AES();
	myAes->SetKey(keyDecrypt,32);	// 32 bytes for key after decrypted
	myAes->SetIV(ivDecrypt,16);		// 16 bytes for iv after decrypted
	myAes->Decrypt(aesData,pbuffer,len);
	delete myAes;

#else
	// permute data recover
	int m_iFeatureSize = 2560;
	unsigned char pbuffer[m_iFeatureSize*2+200];
	unsigned char encryptedBuffer[m_iFeatureSize*2+200];
	memset(pbuffer, 0, m_iFeatureSize*2+200);
	memset(encryptedBuffer, 0, m_iFeatureSize*2+200);
	memcpy(encryptedBuffer, pData+4, len);

	PermuteServer *m_PermServer = new PermuteServer(m_iFeatureSize/2, 1);

    int loc = 0;
	m_PermServer->Recover(encryptedBuffer+loc, pbuffer+loc, pbuffer+loc+m_iFeatureSize/2);
	loc+= m_iFeatureSize;
	m_PermServer->Recover(encryptedBuffer+loc, pbuffer+loc, pbuffer+loc+m_iFeatureSize/2);
#endif

	memcpy(&pData[4], pbuffer, len);
	//DumpBuffer(pData, len+4);

	if(m_Debug)
		EyelockLog(logger, TRACE, "PortableTemplate::checkTOCdata() End");
	return true;
}

void PortableTemplate::TOCReaderBLEConfig()
{
	if(m_Debug)
		EyelockLog(logger, TRACE, "PortableTemplate::TOCReaderBLEConfig() start");
	// set command to TOC reader to config BLE mode and timeout

	char buf[10];
	int cmdLen = 0;
	int result = 0;

	if (m_TOCReaderBLEConfig == TOC_BLE_INIT) {
		// CMD_ SET_BLE_MODE - B1 01 value
		// Passthrough:  0x4E
		// Button Press: 0x42
		// Pin Entry:    0x50
		cmdLen = 3;
		buf[0] = 0xB1;
		buf[1] = 0x01;
		switch (m_tocMobileMode)
		{
			case 1:
				buf[2] = 0x4E;
				break;
			case 2:
				buf[2] = 0x42;
				break;
			case 3:
				buf[2] = 0x50;
				break;
			default:
				EyelockLog(logger, ERROR, "Invalid BLE Mode");
				break;
		}

		m_TOCReaderBLEConfig = TOC_BLE_MODE;
	}
	else if (m_TOCReaderBLEConfig == TOC_BLE_MODE_ACK) {
		// CMD_ SET_TIMEOUT - B2 04 00 00 EA 60
		// Value:  00 00 EA 60 = 1 min in millisecond
		cmdLen = 6;
		buf[0] = 0xB2;
		buf[1] = 0x04;
		buf[2] = *((char *)&m_tocCardExpiredTime + 3);
		buf[3] = *((char *)&m_tocCardExpiredTime + 2);
		buf[4] = *((char *)&m_tocCardExpiredTime + 1);
		buf[5] = *((char *)&m_tocCardExpiredTime + 0);

		m_TOCReaderBLEConfig = TOC_BLE_TIMEOUT;
	}
	else if (m_TOCReaderBLEConfig == TOC_BLE_TIMEOUT_ACK) {
		buf[0] = 0x31; 	// CMD_GET_DEVICE_INFO	(0x31)
		buf[1] = 0;
		cmdLen = 2;
		m_TOCReaderBLEConfig = TOC_BLE_DEVICE_INFO;

		printf("send DEVICE_INFO 0x%x 0x%x, len %d\n", buf[0], buf[1], 2);
	}
	result = BobSetData((void *)buf, cmdLen);
    if(result != 0)
    	EyelockLog(logger, ERROR, "Set TOC BLE Config data failed ");

	result = BobSetDataLength(cmdLen) || BobSetCommand(BOB_COMMAND_BLE_CMD);
	if(result != 0)
	    EyelockLog(logger, ERROR, "set BOB_COMMAND_BLE_CMD failed");

	BoBSetTimer(ReaderSettingTimeoutCB, 1000, 0, this);	// 1sec
}

void PortableTemplate::TOCReaderBLEConfigResp(unsigned char *resp)
{
	if(m_Debug)
		EyelockLog(logger, TRACE, "PortableTemplate::TOCReaderBLEConfigResp() start");

	static int count = 0;
	bool success = false;

	// Check TOC reader response
	if (m_TOCReaderBLEConfig == TOC_BLE_MODE || m_TOCReaderBLEConfig == TOC_BLE_TIMEOUT) {
		if (resp && (resp[0] == 0x5C && resp[1] == 0x01 && resp[2] == 0x00)) {	// Reader Responds: 5C 01 00
			success = true;
		}
		else {
			// failed, send again
			if (count++ < 3) {
				//EyelockLog(logger, ERROR, "Set TOC BLE Command %d failed at count %d", m_TOCReaderBLEConfig, count);
				m_TOCReaderBLEConfig--;
				TOCReaderBLEConfig();
			}
			else {
				EyelockLog(logger, ERROR, "Set TOC BLE Command %d failed ", m_TOCReaderBLEConfig);
				success = true;
			}
		}

		if (success) {
			m_TOCReaderBLEConfig++;
			TOCReaderBLEConfig();
			count = 0;
			BoBCancelTimer(ReaderSettingTimeoutCB);
		}
	}
	else if (m_TOCReaderBLEConfig == TOC_BLE_DEVICE_INFO) {
		/*
		 ******************************************************
		 * Device info: 3811010103010101010002010105000000000B
		 * Main Firmware Major Release                      1
		 * Main Firmware Minor Release                      1
		 * Main Firmware Build                              3
		 * Boot Loader Firmware Major Release          		1
		 * Boot Loader Firmware Minor Release          		1
		 * Boot Loader Firmware Build                       1
		 * Hardware Major Release                           1
		 * Hardware Minor Release                           0
		 * Hardware Build                                   2
		 * BLE Firmware Major Release                       1
		 * BLE Minor Release                                1
		 * BLE Build                                        5
		 * Keypad Firmware Major Release                    0
		 * Keypad Minor Release                             0
		 * Keypad Build                                     0
		 * Operational Configuration:                  0x000B
		 ******************************************************
		 */
		if (resp && resp[0] == 0x38 && resp[1] >= 0x11) {
			EyelockLog(logger, DEBUG, "Get TOC Reader Info");
			FILE *file = fopen("TOCReaderVersion", "w"); // write only
			if (file){
				fprintf(file, "Main FW version: %d.%d.%d\n", resp[2], resp[3], resp[4]);
				fprintf(file, "Bootloader FW version: %x.%x.%x\n", resp[5], resp[6], resp[7]);
				fprintf(file, "HW version: %x.%x.%x\n", resp[8], resp[9], resp[10]);
				fprintf(file, "BLE version: %x.%x.%x\n", resp[11], resp[12], resp[13]);
				fprintf(file, "Keypad FW version: %x.%x.%x\n", resp[14], resp[15], resp[16]);
				fprintf(file, "Configuration: 0x%x, 0x%x", resp[17], resp[18]);
				EyelockLog(logger, INFO, "Main FW version: %d.%d.%d\n", resp[2], resp[3], resp[4]);
				EyelockLog(logger, INFO, "Bootloader FW version: %x.%x.%x\n", resp[5], resp[6], resp[7]);
				EyelockLog(logger, INFO, "HW version: %x.%x.%x\n", resp[8], resp[9], resp[10]);
				EyelockLog(logger, INFO, "BLE version: %x.%x.%x\n", resp[11], resp[12], resp[13]);
				EyelockLog(logger, INFO, "Keypad FW version: %x.%x.%x\n", resp[14], resp[15], resp[16]);
				EyelockLog(logger, INFO, "Configuration: 0x%x, 0x%x", resp[17], resp[18]);
				fclose(file);
				success = true;
			}
		}
		else {
			// failed, send again - not resend per Yiqing's request
			if (count++ < 0) {
				//EyelockLog(logger, ERROR, "Set TOC BLE Command %d failed at count %d", m_TOCReaderBLEConfig, count);
				m_TOCReaderBLEConfig--;
				TOCReaderBLEConfig();
			}
			else {
				EyelockLog(logger, ERROR, "Get TOC Reader INFO failed ");
				success = true;
			}
		}
		if (success) {
			m_TOCReaderBLEConfig = TOC_BLE_INIT;
			BoBSetACSLedRedOut();
			count = 0;
			BoBCancelTimer(ReaderSettingTimeoutCB);
			BobSetCommand(BOB_COMMAND_READ);

			EyelockLog(logger, INFO, "Ready for Card");
		}
	}

}

void PortableTemplate::ReaderSettingTimeoutCB(void *pThread)
{
	EyelockLog(logger, TRACE, "PortableTemplate::ReaderSettingTimeoutCB() ");
	PortableTemplate * myptr = (PortableTemplate * )pThread;
	if (myptr->m_TOCReaderBLEConfig)
		myptr->TOCReaderBLEConfigResp(NULL);
}



// This function performs a 16 bit CCITT CRC on the data pointed to by pData.
uint16_t UpdateCrc16Ccitt (uint16_t crcVal, uint8_t dataByte)
{
    uint16_t updatedCrc;

    dataByte ^= (uint8_t)(crcVal & 0x00FF);
    dataByte ^= dataByte << 4;

    updatedCrc = ((((uint16_t)dataByte << 8) | (uint8_t)(crcVal >> 8)) ^
                    (uint8_t)(dataByte >> 4) ^ ((uint16_t)dataByte << 3));

    return updatedCrc;
}
// This function updates a CRC16 value according to this Polynomial:  x^16 + x^12 + x^5 + 1 (0x8408)
uint16_t CalculateCrc16(uint8_t *pData, uint16_t numBytes)
{
	uint16_t i;
	uint16_t crcVal;

    // crcVal = startValue;
    crcVal = 0x6363;
	for (i=0; i<numBytes; i++)
	{
        crcVal = UpdateCrc16Ccitt(crcVal, pData[i]);
	}
	//printf("crc value %d\n", crcVal);
	return (crcVal);
}










