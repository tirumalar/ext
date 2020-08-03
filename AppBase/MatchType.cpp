/*
 *  MatchType.cpp
 *
 *  Created on: 16-Oct-2016
 *      Author: fjia
 */

#include <iostream>
#include <malloc.h>
#include "MatchType.h"
//#include "DBAdapter.h"
#include "logging.h"

const char logger[30] = "MatchType";

MatchType::MatchType(Configuration& conf):m_pUserData(NULL), m_pCardData(NULL), m_pReceivedCardData(NULL), m_matchManager(NULL), m_dbAdapter(NULL), matchedCardData(NULL)
{
	m_Debug = conf.getValue("Eyelock.SystemReadyDebug", false);
	m_numOfCard = 0;
	m_cardLen = 0;
	m_matchIndex = -1;
	m_duress = false;
	m_pCardData = (unsigned char *)malloc(MAX_PIN_BYTE_LENGTH);
	m_pReceivedCardData = (unsigned char *)malloc(CARD_DATA_SIZE);
	matchedCardData = (unsigned char *)malloc(CARD_DATA_SIZE);
	if (m_pCardData == NULL)
		EyelockLog(logger, ERROR, "Failed at malloc m_pCardData");

	m_pUserData = (char **)calloc(MAX_USERS, sizeof(char*));
	if (m_pUserData == NULL)
		EyelockLog(logger, ERROR, "Failed at malloc m_pUserData");

	memset(m_cardMask, 0xff, MAX_DATA_BYTES);
}

MatchType::~MatchType()
{
	if (m_pCardData)
		free(m_pCardData);
	if (m_pReceivedCardData)
		free(m_pReceivedCardData);
	if (m_pUserData)
		free(m_pUserData);
	if (matchedCardData)
		free(matchedCardData);
}

bool MatchType::timeoutUser()
{
	if (m_dualAuthMatched == CARD_MATCHED )
		return true;
	else
		return false;
}

char * MatchType::PrintCardData(unsigned char *card, int bytes)
{
	char *cardID = (char *)malloc(bytes*3);
	char *tmp = &cardID[0];
	for (int x=0; x < bytes; x++)
	{
		sprintf(tmp, "%02x", card[x]);
		tmp +=2;
		if (x != (bytes-1))
			sprintf(tmp++, "-");
	}
	sprintf(tmp, "\0");
	//EyelockLog(logger, DEBUG, cardID);
	return cardID;
}

void MatchType::clearCardData()
{
	memset(m_pCardData, 0, m_cardLen);
	memset(m_pReceivedCardData, 0, CARD_DATA_SIZE);
	m_cardLen = 0;
	m_matchIndex = -1;
}

bool MatchType::getUserNameFromCard(unsigned char *card, char *username)
{
	if(m_Debug)
		EyelockLog(logger, TRACE, "MatchType::getUserNameFromCard() %s, matchindex %d", PrintCardData(card, (m_accessDataLength+7)/8), m_matchIndex);

	memset(username, 0, NAME_SIZE);
	username[0] = '\0';

	if (m_matchIndex == NWM_INDEX) {
		memcpy((void *)username, (void *)m_cardUser, NAME_SIZE);
		return true;
	}

	if (m_pUserData == NULL || m_matchIndex == -1) {
		return false;
	}

	int len = (m_accessDataLength+7)/8;

	if (memcmp(card, m_pUserData[m_matchIndex], len) == 0){
		memcpy((void *)username, (void *)(m_pUserData[m_matchIndex]+len+MAX_PIN_BYTE_LENGTH), NAME_SIZE);
	}
	m_matchIndex = -1;
	if(m_Debug) {
		EyelockLog(logger, DEBUG, "get user name %s", username);
	}

	return true;
}

void MatchType::DumpBuffer(unsigned char *buf, int bytes)
{
        for (int x=0; x < bytes; x++)
        {
                printf(" 0x%x", buf[x]);
        }
        printf("\n");
}

