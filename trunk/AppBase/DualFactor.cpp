/*
 * DualFactor.cpp
 *
 *  Created on: 16-Oct-2016
 *      Author: fjia
 */

#include <iostream>

#include "DualFactor.h"
#include "DBAdapter.h"
#include "MatchManagerInterface.h"
//#include "Configuration.h"
#include "MatchType.h"
//#include "Synchronization.h"
#include "logging.h"
extern "C" {
#include "BobListener.h"
}
const char logger[30] = "DualFactor";

const unsigned char pinBurstCode4[12] = {0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B};
//const unsigned char pinBurstCode8[12] = {0x0F, 0x1E, 0x2D, 0x3C, 0x4B, 0x5A, 0x69, 0x78, 0x87, 0x96, 0xA5, 0xB4};
const unsigned char pinBurstCode8[12] = {0xF0, 0xE1, 0xD2, 0xC3, 0xB4, 0xA5, 0x96, 0x87, 0x78, 0x69, 0x5A, 0x4B};

DualFactor::DualFactor(Configuration& conf):MatchType(conf)
{
	m_f2f = conf.getValue("GRITrigger.F2FEnable",false);
	m_dualParity = true; 	// conf.getValue("GRITrigger.DualAuthenticationParity",true);
	m_cardPinMatch = false;
	m_sendRawCardData = conf.getValue("Eyelock.SendRawCardData",true);
}

void DualFactor::initMatchBuffer()
{
#ifdef OTO_MATCH
	if(m_matchManager)
		m_matchManager->ClearLocalMatchBuffer();
#endif
}

bool DualFactor::modifyACD(string acd, int acdlen, string acdnop, string new_acd, string new_acdnop, string pin)
{
	if(m_Debug)
		EyelockLog(logger, TRACE, "DualFactor::modifyACD()  ");

	if (acdlen != m_accessDataLength || m_pUserData == NULL) {
		EyelockLog(logger, ERROR, "DualFactor::modifyACD() - input parameters wrong or m_pUserData NULL");
		return false;
	}
	int len = (m_accessDataLength+7)/8;


	for (int i=0; i < m_acdCount; i++) {
		int result;
#ifdef DB_NOPARITY
		if (!m_dualParity) {
			result = memcmp((char *)acdnop.c_str(), m_pUserData[i], len);
			if (result == 0) {
				EyelockLog(logger,  DEBUG, "modify ACDNOP at %d of %d!!!\n", i, m_acdCount);
				memcpy(m_pUserData[i], new_acdnop.c_str(), len);
				memcpy(m_pUserData[i]+len, new_acd.c_str(), len);
				return true;
			}
		}
		else
#endif
		{
			result = memcmp((char *)acd.c_str(), m_pUserData[i], len);
			if (result == 0) {
				EyelockLog(logger, DEBUG, "modify ACD at %d of %d!!!\n", i, m_acdCount);
				memcpy(m_pUserData[i], new_acd.c_str(), len);
				strncpy(m_pUserData[i]+len, pin.c_str(), MAX_PIN_BYTE_LENGTH);
				return true;
			}
		}
	}
	return false;
}

bool DualFactor::addACD(string acd, int acdlen, string acdnop, string name, string pin)
{
	if(m_Debug)
		EyelockLog(logger, TRACE, "DualFactor::addACD()  ");

	if (acdlen != m_accessDataLength || m_pUserData == NULL) {
		EyelockLog(logger, ERROR, "DualFactor::addACD() - input parameters wrong or m_pUserData NULL ");
		return false;
	}

	int len = (m_accessDataLength+7)/8;
	char *data_ptr = NULL;
#ifdef DB_NOPARITY
	if (!m_dualParity)
		data_ptr = (char *)malloc(len * 2);
	else
#endif
		data_ptr = (char *)malloc(len+MAX_PIN_BYTE_LENGTH+100);
	if (data_ptr == NULL) {
		EyelockLog(logger, ERROR, "DualFactor::addACD() - malloc failed ");
		return false;
	}
#ifdef DB_NOPARITY
	if (!m_dualParity) {
		memcpy(data_ptr, acdnop.c_str(), len);
		memcpy(data_ptr+len, acd.c_str(), len);
	}
	else
#endif
	memcpy(data_ptr, acd.c_str(), len);
	strncpy(data_ptr+len, pin.c_str(), MAX_PIN_BYTE_LENGTH);
	memcpy(data_ptr+len+MAX_PIN_BYTE_LENGTH, name.c_str(), 100);


	if (m_acdCount < MAX_USERS){
		m_pUserData[m_acdCount] = data_ptr;
		m_acdCount++;
	}

	return true;
}

bool DualFactor::loadACD()
{
	if(m_Debug)
		EyelockLog(logger, TRACE, "DualFactor::loadACD()  ");

	if (m_pUserData == NULL || m_dbAdapter == NULL)
		return false;

	for (int i=0; i < MAX_USERS; i++){
		if (m_pUserData[i]){
			free(m_pUserData[i]);
			m_pUserData[i] = NULL;
		}
	}

	int len = (m_accessDataLength+7)/8;
	m_acdCount = 0;

	m_acdCount = m_dbAdapter->GetUserCount();
	if (m_acdCount > MAX_USERS)
		m_acdCount = MAX_USERS;

	for (int i=0; i < m_acdCount; i++){
#ifdef DB_NOPARITY
		if (!m_dualParity)
			m_pUserData[i] = (char *)malloc(len*2);
		else
#endif
			m_pUserData[i] = (char *)malloc(len+MAX_PIN_BYTE_LENGTH+100);	// name length 100 bytes
		if (m_pUserData[i] == NULL) {
			EyelockLog(logger, ERROR, "DualFactor::loadACD() - malloc failed %d ", i);
			return false;
		}
	}

	//int cardtype = (!m_dualParity) ? 2 : 1;
	//m_dbAdapter->getAllACD(&m_pUserData[0], len, 1);
	int getAllACDstatus = m_dbAdapter->getAllACD(&m_pUserData[0], len, MAX_PIN_BYTE_LENGTH, 100, 1);
	// TODO: implement reliable DB schema check to be used through all the code
	if (getAllACDstatus != 0)
	{
		EyelockLog(logger, DEBUG, "m_dbAdapter->getAllACD PIN overload failed, calling legacy overload");
		m_dbAdapter->getAllACD(&m_pUserData[0], len, 1);
	}

	if(m_Debug) {
		EyelockLog(logger, DEBUG, "get all ACD: count %d, len %d, cardtype %d\n", m_acdCount, len, 1);
		for (int i=0; i < m_acdCount; i++){
			char *cardID = PrintCardData((unsigned char *)m_pUserData[i], len);
			char *pin = m_pUserData[i]+len;
			char *pinID = PrintCardData((unsigned char*)pin, strlen(pin));
			EyelockLog(logger, DEBUG, "DualFactor::loadACD() - ACD for user %d, name %s, card %s, pin %s", i, m_pUserData[i]+len+MAX_PIN_BYTE_LENGTH, cardID, pinID);
			if(cardID) free(cardID);
			if(pinID) free(pinID);
		}
	}

	return true;
}

bool DualFactor::ValidateCard()
{
	if(m_Debug)
		EyelockLog(logger, DEBUG, "DualFactor::ValidateCard() - m_accessDataLength %d ", m_accessDataLength);

	if (m_dualAuthMatched == CARD_MATCHED)
		return false;

	unsigned char *cardData = m_pCardData;
	int bytes = (m_accessDataLength+7)/8;

	if (m_cardLen != m_accessDataLength)
		return false;
	
	if (m_f2f) {
		// add 0 in the first and last byte
		for (int j = bytes-2; j > 0; j--) {
			cardData[j] = cardData[j - 1];
		}
		cardData[0] = 0;
		cardData[bytes-1] = 0;
	}

	// filter unused parity bits and data bits in card data
	for (int i=0; i<bytes; i++) {
		cardData[i] = cardData[i] & m_cardMask[i];
	}

	int matched = findCardInDB(cardData) || findCardInNwMatch(cardData);

#ifdef OTO_MATCH
	if (matched && m_matchManager) {
		// update matchBuffer
		string acd;
		acd = string(cardData,cardData+bytes);
		//int cardtype = (!m_dualParity) ? 2 : 1;
		m_matchManager->ClearLocalMatchBuffer();
		usleep(1000);
		m_matchManager->UpdateLocalMatchBuffer(acd, m_accessDataLength, 1);
	}
#endif
	if (matched == true) {
		// save card data
		memcpy(matchedCardData, cardData, CARD_DATA_SIZE);
	}
	return matched;
}

bool DualFactor::ValidatePin()
{
#define PIN_BURST_POUND_4		pinBurstCode4[11]
#define PIN_BURST_POUND_8		pinBurstCode8[11]
#define PIN_BURST_ASTERISK_4	pinBurstCode4[10]
#define PIN_BURST_ASTERISK_8	pinBurstCode8[10]

	if(m_Debug)
		EyelockLog(logger, DEBUG, "DualFactor::ValidatePin() - length of PIN received: %d", m_cardLen);

	if (m_dualAuthMatched == CARD_MATCHED || m_authMode < PIN_AND_IRIS)
		return false;

	unsigned char *pinData = m_pCardData;
	int bytes = m_cardLen;

	// pin number ending '#' or '*'
	char pinEnding;
	if (m_pinBurstBits == 8) {
		pinEnding = pinData[bytes-1];
		if (pinEnding == PIN_BURST_ASTERISK_8)
			m_duress = true;
		else if (pinEnding == PIN_BURST_POUND_8)
			m_duress = false;
		else
			return false;
		for (int i=0; i<bytes-1; i++) {
			for (int j=0; j<10; j++){
				if (pinData[i] == pinBurstCode8[j]){
					pinData[i] = '0' + j;
					break;
				}
			}
		}
	}
	else if (m_pinBurstBits == 4) {
		for (int i=0; i<bytes; i++) {
			pinData[i] >>= 4;
			pinData[i] += '0';
		}
		/*
		char tmp[bytes];
		int j = 0;
		for (int i=0; i<index; i++) {
			tmp[j++] = '0' + ((pinData[i] & 0xF0) >> 4);
			if (j < bytes)
				tmp[j++] = '0' + (pinData[i] & 0x0F);
		}
		memcpy(pinData, tmp, bytes);
		*/
		pinEnding = pinData[bytes-1]-'0';

		if (pinEnding == PIN_BURST_ASTERISK_4)
			m_duress = true;
		else if (pinEnding == PIN_BURST_POUND_4)
			m_duress = false;
		else
			return false;
	}

	pinData[bytes-1] = '\0';
	int matched = findPinInDB(pinData);

	return matched;
}

bool DualFactor::findCardInDB(unsigned char *card)
{
	if(m_Debug)
		EyelockLog(logger, DEBUG, "DualFactor::findCardInDB() - m_accessDataLength %d ", m_accessDataLength);

	int len = (m_accessDataLength+7)/8;
	bool matched = false;

	if (m_pUserData == NULL)
		return false;

	// search card from memory
	for (int i=0; i<m_acdCount; i++) {
		unsigned char dbmasked[len];
		unsigned char *dbdata = (unsigned char *)m_pUserData[i];
		// filter unused parity bits and data bits in DB data
		for (int j=0; j<len; j++) {
			dbmasked[j] = dbdata[j] & m_cardMask[j];
		}

		if (memcmp(card, dbmasked, len) == 0) {
			matched = true;
			memcpy(card, dbdata, len); 	// store unmasked data format
			m_matchIndex = i;
			break;
		}
	}

	return matched;
}
bool DualFactor::findPinInDB(unsigned char *pin)
{
	if(m_Debug)
		EyelockLog(logger, DEBUG, "DualFactor::findPinInDB()");

	int len = (m_accessDataLength+7)/8;
	bool matched = false;

	if (m_pUserData == NULL)
		return false;

	if (m_pinAuth) {
		// search card from memory
		for (int i=0; i<m_acdCount; i++) {
			const char *pindb = m_pUserData[i]+len;
			if (strncmp(reinterpret_cast<const char*>(pin), pindb, MAX_PIN_BYTE_LENGTH) == 0) {
				matched = true;
				m_matchIndex = i;
				break;
			}
		}
	}
	else if (m_matchIndex != -1) {
		const char *pindb = m_pUserData[m_matchIndex]+len;
		if (strncmp(reinterpret_cast<const char*>(pin), pindb, MAX_PIN_BYTE_LENGTH) == 0) {
			matched = true;
		}
	}
	return matched;
}
bool DualFactor::findCardInNwMatch(unsigned char *card)
{
	if(m_Debug)
		EyelockLog(logger, DEBUG, "DualFactor::findCardInNwMatch() - m_accessDataLength %d ", m_accessDataLength);

	// send card data to Network Matcher
	int len = (m_accessDataLength+7)/8;
	bool matched = false;

	char carddata[len+2];
	carddata[0] = m_accessDataLength & 0xFF;
	carddata[1] = (m_accessDataLength >> 8) & 0xFF;
	memcpy(&carddata[2], card, len);

	if(m_matchManager){
		matched = m_matchManager->PCMatcherMsg(carddata, len+2, m_cardUser);
		if (matched)
			m_matchIndex = NWM_INDEX;
	}

	return matched;
}


void DualFactor::removeParityBits(char *card, int accessDataLength)
{
	int bytes = (accessDataLength+7)/8;

	for (int x=0; x < bytes; x++)
	{
		if (x == 0)
		{   // Remove 1st bit
			if (accessDataLength == 35)
			{ // 2nd bit is parity
				card[x] &= 0x3F;
	        }
	        else
	        {
	        	card[x] &= 0x7F;
	        }
	    }
	    else if (x == bytes-1) {        // Remove last bit
	        char mask = 0xFF;
	        int bits = accessDataLength % 8;
	        mask = mask << (8 - bits + 1);

	        card[x] &= mask;

	   }
	}

}

bool DualFactor::UpdateMatchResult(MatchResult *msg)
{
	MatchResultState state = msg->getState();

	if (m_dualAuthMatched == CARD_MATCHED) {
#ifdef OTO_MATCH
		if(m_matchManager)
			m_matchManager->ClearLocalMatchBuffer();
#endif
		if (state==PASSED) {
			int bytes =0,bits=0;
			char *ptr = msg->getF2F(bytes,bits);
			if (!m_pinAuth) { // "card and iris" and "multifactor"
				memcpy(m_pCardData, matchedCardData, CARD_DATA_SIZE);
				if (bytes == (m_accessDataLength+7)/8 && memcmp(m_pCardData, ptr, bytes) == 0) {
					msg->setState(PASSED);

					if (m_sendRawCardData)
					{
						if (m_Debug)
						{
							printf("The card data \"from DB\" : ");
							msg->printF2FData();
						}

						char *appendedCardData = new char[bytes + 2];
						memcpy(appendedCardData + 2, m_pReceivedCardData, bytes);
						*((short*) appendedCardData) = (short) m_accessDataLength;
						msg->setF2F(appendedCardData);
						delete[] appendedCardData;

						if (m_Debug)
						{
							printf("The card data \"from card reader (stored in memory previously)\" : ");
							msg->printF2FData();
						}
					}
				}
				else {
					msg->setState(FAILED);
				}
			}
			else {
				int i;
				for(i=0; i<m_acdCount; i++) {
					const char *pindb = m_pUserData[i]+bytes;
					unsigned char *carddb = (unsigned char *)m_pUserData[i];
					if (strncmp(reinterpret_cast<const char*>(m_pCardData), pindb, MAX_PIN_BYTE_LENGTH) == 0 && memcmp(carddb, ptr, bytes) == 0) {
						msg->setState(PASSED);
						m_matchIndex = i;
						break;
					}
				}
				if (i == m_acdCount) {
					msg->setState(FAILED);
				}
				memcpy(m_pCardData, m_pUserData[m_matchIndex], (m_accessDataLength+7)/8);
			}
		}
		else
			msg->setState(FAILED);

	}
	else {
		if (state!=CONFUSION)
			msg->setState(CONFUSION);
	}

	return true;
}

bool DualFactor::timeoutUser()
{
	if (m_dualAuthMatched == CARD_MATCHED ){
#ifdef OTO_MATCH
		if(m_matchManager)
			m_matchManager->ClearLocalMatchBuffer();
#endif
		return true;
	}
	else
		return false;
}

void DualFactor::SetAccessDataMask(int len, const char *dataMask)
{
	m_accessDataLength = len;

	if (dataMask) {
		int bytes = (len+7)/8;
		memset(m_cardMask, 0, MAX_DATA_BYTES);	// reset to all 0's
		for (int i=0; i<bytes; i++){
			m_cardMask[i] = dataMask[i];

			if(m_Debug)
				EyelockLog(logger, DEBUG, "i=%d, data 0x%x, m_cardMask 0x%x\n", i, dataMask[i], m_cardMask[i]);
		}
	}
}

void DualFactor::GetAccessDataMask()
{
	if(!m_dbAdapter) {
		EyelockLog(logger, ERROR, "Can't get DB Adapter");
		return;
	}

	string parityMask = "";
	string dataMask = "";
	int acdlen;
	string username,acd;
	if(0 == m_dbAdapter->GetACSData(username,acd,acdlen, parityMask, dataMask)){
		SetAccessDataMask(acdlen, dataMask.c_str());
	}

}

char * DualFactor::PrintPinNumber(unsigned char *pin, int bytes)
{
	char *cardID = (char *)malloc(bytes*3);
	char *tmp = &cardID[0];
	if (m_pinBurstBits == 8) {
		for (int i=0; i<bytes; i++) {
			for (int j=0; j<10; j++){
				if (pin[i] == pinBurstCode8[j]){
					pin[i] = '0' + j;
					break;
				}
			}
		}
	}
	for (int x=0; x < bytes; x++)
	{
		sprintf(tmp, "%02x", pin[x]-'0');
		tmp +=2;
		if (x != (bytes-1))
			sprintf(tmp++, "-");
	}
	sprintf(tmp, "\0");
	//EyelockLog(logger, DEBUG, cardID);
	return cardID;
}

DualFactor::~DualFactor() {
	if (m_pUserData){
		for (int i=0; i < MAX_USERS; i++){
			if(m_pUserData[i])
				free(m_pUserData[i]);
		}
	}
}




