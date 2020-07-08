/*
 * MatchType.h
 *
 *  Created on: 16-Oct-2016
 *      Author: fjia
 */

#ifndef MATCHTYPE_H_
#define MATCHTYPE_H_

#include <stdio.h>
#include <string>
#include "MatchManagerInterface.h"

#define MAX_DATA_BYTES		100
#define GUID_SIZE 			36
#define NAME_SIZE 			100
#define NWM_INDEX			0xFFFFFFFF

#define CARD_DATA_SIZE MAX_PIN_BYTE_LENGTH

using namespace std;

class MatchManagerInterface;
class DBAdapter;

enum DualAuthNMatch{NOT_MATCHED=1, BOTH_MATCHED=2, CARD_MATCHED=3, IRIS_MATCHED=4, WAIT_FOR_PIN};
enum CRADREAD{CARD_INIT=0, CARD_READY, CARD_TIMEOUT};
enum MatchMode{IRIS_ONLY=1, CARD_OR_IRIS, CARD_AND_IRIS, CARD_AND_IRIS_PIN_PASS, PIN_AND_IRIS, CARD_AND_PIN_AND_IRIS, PIN_AND_IRIS_DURESS, CARD_AND_PIN_AND_IRIS_DURESS};

class MatchType {
public:
	MatchType(Configuration& conf);
	virtual ~MatchType();
	virtual bool loadACD() {return true;}
	virtual bool addACD(string acd, int acdlen, string acdnop, string name, string ping) {return true;}
	virtual bool modifyACD(string acd, int acdlen, string acdnop, string new_acd, string new_acdnop, string pin) {return true;}
	virtual bool ValidateCard(){return false;}
	virtual bool ValidatePin(){return false;}
	virtual bool CheckReaderData(){return false;}
	virtual bool getUserNameFromCard(unsigned char *card, char *username);
	virtual bool addUser(string perid, string name, string acd){return false;}
	virtual bool timeoutUser();
	virtual bool UpdateMatchResult(MatchResult *result) {return true;}
	virtual void initMatchBuffer(){};
	virtual void SetAccessDataMask(int len, const char *dataMask) {return;}
	virtual void GetAccessDataMask(){return;};


	char * PrintCardData(unsigned char *card, int bytes);
	virtual char * PrintPinNumber(unsigned char *card, int bytes) {return NULL;}
	void clearCardData();
	static void DumpBuffer(unsigned char *buf, int bytes);

	DualAuthNMatch m_dualAuthMatched;
	MatchManagerInterface *m_matchManager;
	unsigned char *m_pCardData;
	unsigned char *m_pReceivedCardData;
	int m_cardLen;
	int m_numOfCard;
	int m_accessDataLength;
	int m_matchIndex;
	bool m_pinAuth;
	int m_authMode;
	int m_pinBurstBits;
	bool m_duress;
	char m_cardUser[NAME_SIZE];
	unsigned char m_cardMask[MAX_DATA_BYTES];
	unsigned char *matchedCardData;
	DBAdapter* m_dbAdapter;

protected:
	char **m_pUserData;
	
	void PrintForDebug(const char *data){
	    printf("\nMatch Key Content: ");
	    unsigned char *ptr = (unsigned char*)(data);
	    for(int i = 0;i < ((m_accessDataLength + 7) >> 3);i++)
	        printf("%02x ", *ptr++);
	    printf("\n");
	}

	bool m_Debug;





};

#endif /* MATCHTYPE_H_ */
