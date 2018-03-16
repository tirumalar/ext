/*
 * PortableTemplate.h
 *
 *  Created on: 16-Oct-2016
 *      Author: fjia
 */

#ifndef PORTABLETEMPLATE_H_
#define PORTABLETEMPLATE_H_

#include "MatchType.h"

#define TOC_DATA_RSA_AES

class Configuration;

enum TOCBLE{TOC_BLE_INIT=0, TOC_BLE_MODE, TOC_BLE_MODE_ACK, TOC_BLE_TIMEOUT, TOC_BLE_TIMEOUT_ACK, TOC_BLE_DEVICE_INFO, TOC_BLE_DONE};

class PortableTemplate: public MatchType {
public:

	PortableTemplate(Configuration& conf);
	virtual ~PortableTemplate();
	virtual bool ValidateCard();
	virtual bool CheckReaderData();
	virtual bool addUser(string perid, string name, string acd);
	bool deleteUser(string perid);
	virtual bool timeoutUser();
	virtual bool UpdateMatchResult(MatchResult *result);
	virtual void initMatchBuffer();
	
protected:
	int m_tocCardIndex;
	int m_TOCReaderBLEConfig;
	int m_tocCardExpiredTime;
	int m_tocMobileMode;
	bool m_tocCustomKey;
	char *m_tocCustFile;
	char *m_tocCustPW;

	bool checkTOCdata(unsigned char *pData);
	void TOCReaderBLEConfig();
	void TOCReaderBLEConfigResp(unsigned char *resp);
	static void ReaderSettingTimeoutCB(void *pthread);
};

#endif /* PORTABLETEMPLATE_H_ */
