/*
 * DualFactor.h
 *
 *  Created on: 16-Oct-2016
 *      Author: fjia
 */

#ifndef DUALFACTOR_H_
#define DUALFACTOR_H_

#include "MatchType.h"

class Configuration;
class DBAdapter;

class DualFactor: public MatchType {
public:

	DualFactor(Configuration& conf);
	virtual ~DualFactor();

	virtual bool ValidateCard();
	virtual bool ValidatePin();
	virtual bool loadACD();
	virtual bool addACD(string acd, int acdlen, string acdnop, string name, string pin);
	virtual bool modifyACD(string acd, int acdlen, string acdnop, string new_acd, string new_acdnop, string pin);
	virtual bool UpdateMatchResult(MatchResult *result);
	virtual void initMatchBuffer();
	void removeParityBits(char *card, int len);
	virtual bool timeoutUser();
	virtual void SetAccessDataMask(int len, const char *dataMask);
	virtual void GetAccessDataMask();
	char * PrintPinNumber(unsigned char *pin, int bytes);

protected:
	int m_acdCount;
	bool m_dualParity;
	bool m_f2f;
	bool m_osdpACSEnabled;
	bool m_cardPinMatch;

	bool m_sendRawCardData;

	bool findCardInDB(unsigned char *card);
	bool findCardInNwMatch(unsigned char *card);
	bool findPinInDB(unsigned char *pin);

};

#endif /* DUALAUTHENTICATION_H_ */
