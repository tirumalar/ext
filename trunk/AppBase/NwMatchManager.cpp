/*
 * NwMatchManager.cpp
 *
 *  Created on: Aug 12, 2011
 *      Author: developer1
 */
#include <iostream>
#include <socket.h>
#include <sys/syscall.h>
#include <pthread.h>
#include <vector>
#include "SocketFactory.h"
#include "HTTPPOSTMsg.h"
#include "BiOmega.h"
#include "NwMatchManager.h"
#include "MatchManagerInterface.h"
#include "MatchManagerFactory.h"
#include "LEDDispatcher.h"
#include "logging.h"
#include "NwMatcherSerialzer.h"
#include "IrisData.h"
#include "LoiteringDetector.h"
#include "F2FDispatcher.h"
#include "MatchDispatcher.h"
#include "DBMap.h"
#include "UtilityFunctions.h"
#include "logging.h"
#include "DBAdapter_Keys.h"
#include "OpenSSLSupport.h"
#include "AESClass.h"
#include "DBAdapter.h"
#include <unistd.h>

const char logger[30] = "NwMatchManager";

NwMatchManager::NwMatchManager(Configuration& conf) :
		GenericProcessor(conf), m_queueSz(0), m_MatcherHDMStatus(true), m_ledDispatcher(NULL), m_logging(false),
		m_PingTimeStamp(0), m_PingInterval(5), m_imageProcessor(0), m_clientAddr(0), m_enableCentroidRatio(false),
		m_f2fDispatcher(NULL),m_singleIrisinDual(false), m_loiteringDetector(NULL), m_F2FDbDoneMsg(256),
		m_matchDispatcher(NULL), m_irisState(UT_IRIS_NONE), m_resultDestAddr(NULL), m_socketFactory(NULL),
		m_intraEyeTimeWindowEnable(false),m_irisCodeDatabaseFile(NULL),m_negativeMatchEnable(false),m_sleepTimeBetweenMatching(50000) {
	FlushIrisList(CHECK_IRIS_BOTH);

	m_queueFullBehaviour = OVERWRIE_OLD;
	m_matchScoreThresh = conf.getValue("GRI.matchScoreThresh", 0.13f);
	m_matchScoreThresh1 = conf.getValue("GRI.matchScoreThresh1", 0.27f);
	m_sleepTimeBetweenMatching = conf.getValue("Eyelock.SleepTimeBetweenMatching",50000);
	timeoutThreadSpawned = false;
#ifndef __BFIN__
	m_bioInstance = new BiOmega(640, 480, 1);
#else
	m_bioInstance = NULL;
#endif
#ifndef HBOX_PG
	m_irisCodeDatabaseFile = (char*) conf.getValue("GRI.irisCodeDatabaseFile","./data/sqlite.db3");
#else	
	m_irisCodeDatabaseFile = (char*) conf.getValue("GRI.irisCodeDatabaseFile","data/sqlite.db3");
#endif	
	m_queueFullBehaviour = DROP;
	m_Debug = conf.getValue("GRI.NwMmDebug", false);

	unsigned int maskcode = conf.getValue("GRI.MatcherFeatureMask", 255);
	m_maskval = (maskcode << 24) | (maskcode << 16) | (maskcode << 8)
			| (maskcode);

	MatchManagerFactory factory(conf);
	m_matchManager = factory.Create(conf, m_bioInstance, 0, 0);

	m_Master = conf.getValue("GRI.EyelockMaster", false);
	m_spoofEnable = conf.getValue("Eyelock.SpoofEnable", false);
	m_SpecCentroidRatioThreshold.x = conf.getValue(
			"Eyelock.SpecCentroidRatioThresholdX", 10.0f);
	m_SpecCentroidRatioThreshold.y = conf.getValue(
			"Eyelock.SpecCentroidRatioThresholdY", 0.0f);
	m_interIrisTimeWindowThresholdMilliSec = conf.getValue(
			"Eyelock.InterIrisTimeWindowThresholdMilliSec", 800);
	m_interIrisTimeWindowThresholdMilliSec =
			m_interIrisTimeWindowThresholdMilliSec * 1000; //usec

	m_enableCentroidRatio = conf.getValue("Eyelock.EnableCentroidRatio",
			m_spoofEnable);
	m_minTrackMatchThresh = conf.getValue("Eyelock.MinTrackMatchThresh",
			-1.00f);
	m_logging = conf.getValue("Eyelock.Logging", false);

	m_f2fResult.init();
	m_f2fResult.setState(DBRELOAD);
	const char *str1 = conf.getValue("Eyelock.F2FCardDataDbDone", "0x0000");
	MakeF2FMsg(str1, DBRELOAD, m_F2FDbDoneMsg);

	const char *svrAddr = conf.getValue("Eyelock.F2FDestAddr", "NONE");
	if (strcmp(svrAddr, "NONE") != 0) {
		m_resultDestAddr = HostAddress::MakeHost(svrAddr, eIPv4, false);
		int timeOutms = conf.getValue("Eyelock.F2FSocketTimeOutMillis", 500);
		m_timeOutSend.tv_sec = timeOutms / 1000;
		m_timeOutSend.tv_usec = (timeOutms % 1000) * 1000;
		m_socketFactory = new SocketFactory(conf);
	}
	m_dualMatchEnabled = conf.getValue("Eyelock.DualMatcherPolicy", false);
	m_singleIrisinDual = conf.getValue("Eyelock.SingleIrisinDual", true);

	m_intraEyeTimeWindowEnable = conf.getValue("Eyelock.IntraEyeTimeWindowEnable", false);
	m_intraEyeTimeWindowThreshold = conf.getValue(
			"Eyelock.IntraEyeTimeWindowThreshold", 2000); //it will work in conjunction with dual eye match
	m_intraEyeTimeWindowThreshold = m_intraEyeTimeWindowThreshold * 1000;
	m_striclyFlushLists = conf.getValue("Eyelock.StrictlyFlushEyeLists", true);
	m_checkUID = conf.getValue("Eyelock.CheckUID", true);

	m_negativeMatchEnable = conf.getValue("Eyelock.NegativeMatchEnable",false);

	// TODO: refactor.
	// Code duplication, same code: LEDDispatcher.cpp, NwMatchManager.cpp, OSDPMessage.cpp, F2FDispatcher.cpp
	// use one function from EyelockConfiguration for all?
	// get rid of bool value?
	bool dualAuthN = false;
	int authenticationMode = conf.getValue("Eyelock.AuthenticationMode",0);
	if (authenticationMode) {
		switch (authenticationMode)
		{
			case CARD_OR_IRIS:
				break;
			case CARD_AND_IRIS:
				dualAuthN = true;
				break;
			case CARD_AND_IRIS_PIN_PASS:
				dualAuthN = true;
				break;
			case PIN_AND_IRIS:
			case PIN_AND_IRIS_DURESS:
				dualAuthN = true;
				break;
			case CARD_AND_PIN_AND_IRIS:
			case CARD_AND_PIN_AND_IRIS_DURESS:
				dualAuthN = true;
				break;
			default:
				dualAuthN = false;
				break;
		}
	}
	else {
		dualAuthN = conf.getValue("GRITrigger.DualAuthenticationMode",false);
	}

	m_bNegativeMatchTimerActive = (conf.getValue(
			"Eyelock.EnableNegativeMatchTimeout", 0)
			|| dualAuthN);
	m_iWaitTime = conf.getValue("Eyelock.NegativeMatchTimeout", 5000);
	m_iResetTimer = conf.getValue("Eyelock.NegativeMatchResetTimer", 4);

	m_bNegativeTimeoutKill = false;
	m_bTimeoutActive = false;
	negativeMatchThreadRunning = false;
	CURR_TV_AS_SEC(current);
	m_tLastTimeOutWindowStarted = current;
	EyelockLog(logger, DEBUG, "Negative match timer enabled %s timeout is %d",
			(m_bNegativeMatchTimerActive) ? ("yes") : ("no"), m_iWaitTime);

	m_certFile = NULL;

	//xxlog("Eyelock Startup\n",0);
	EyelockEvent("Eyelock Startup\n",0);

}

void NwMatchManager::SetF2FDispatcher(F2FDispatcher *ptr){
	m_f2fDispatcher = ptr;
	m_matchManager->SetF2FDispatcher(ptr);
	ptr->SetMatchManager(m_matchManager);
}

NwMatchManager::~NwMatchManager() {
	if (m_matchManager)
		delete m_matchManager;
	if (m_resultDestAddr)
		delete m_resultDestAddr;
	if (m_socketFactory)
		delete m_socketFactory;
	if (m_bioInstance)
		delete m_bioInstance;
	if (m_certFile)
		free(m_certFile);

	FlushIrisList(CHECK_IRIS_BOTH);
	//xxlog("Eyelock Graceful Shutdown\n",0);
}

int NwMatchManager::getQueueSize(Configuration* conf) {
	m_queueSz = conf->getValue("GRI.NwMatchManagerQueueSize", 15);
	EyelockLog(logger, DEBUG, "NwMatchManager::QueSz %d ", m_queueSz);
	return m_queueSz;
}

Copyable* NwMatchManager::createNewQueueItem() {
	int msgSize = 2560 + 512;
	return (new BinMessage(msgSize));
}
void NwMatchManager::FlushLoitering() {
	if (m_loiteringDetector) {
		m_loiteringDetector->ClearVector();
	}
}

void NwMatchManager::MatchDetected(MatchResult *result) {
    int fr,ey;
    string cam;
    result->getFrameInfo(fr,ey,cam);
	CURR_TV_AS_USEC(t);
	int64_t diff = t-result->getTimeStamp();
	if (m_Debug)
		EyelockLog(logger, DEBUG, "%llu::MatchDetected %d %d %s diff %d\n",t/1000,fr,ey,cam.c_str(),diff);
	result->setNwValandSleep(0,(int)(diff/1000));
	m_mr=*result;
	if (m_matchDispatcher){
		m_matchDispatcher->enqueMsg(*result);
	}
}

void NwMatchManager::FlushQueue(bool onlyMatches) {
	int flushed = 0;
	bool emptied = false;
	for (int i = 0; i < m_inQueue.getSize(); i++) {
		Safe<Copyable *> & currMsg = m_inQueue[i];
		emptied = false;
		currMsg.lock();
		if (currMsg.isUpdated()) {
			HTTPPOSTMsg *p = (HTTPPOSTMsg *) currMsg.get();
			if ((p->getMsgType() == MATCH_MSG) || (!onlyMatches)) {
				currMsg.setUpdated(false);
				flushed++;
				emptied = true;
			}
		}
		currMsg.unlock();
		if (emptied)
			m_inQueue.decrCounter();
	}
	EyelockLog(logger, DEBUG, "NwMatchManager::Flushed %d Match requests", flushed);
}

void NwMatchManager::recoverFromBadState() {
//	printf("Entering NwMatchManager::recoverFromBadState\n");
	if (!m_MatcherHDMStatus) {
//		printf("NwMatchManager::recoverFromBadState\n");
		m_matchManager->RecoverFromBadState();
		FlushQueue(false);
		m_MatcherHDMStatus = true;
	}
}

void NwMatchManager::runDiagnostics() {
	RunHDMDiagnostics();
}

void NwMatchManager::RunHDMDiagnostics() {
	struct timeval curTime;
	gettimeofday(&curTime, 0);
	int currtimestamp = (curTime.tv_sec);
	if (currtimestamp > m_PingTimeStamp) {
		//printf("NwMatchManager::RunHDMDiagnostics\n");
		m_matchManager->SaveLogResult(); // Save log file
		bool checkHDMPing = m_matchManager->HDMDiagnostics();
		bool checkvalidity = m_matchManager->CheckValidityOfHDM();

		if(!(checkHDMPing && checkvalidity)){
			m_MatcherHDMStatus = false;
		}else{
			m_MatcherHDMStatus = true;
		}
		m_PingTimeStamp = currtimestamp + m_PingInterval;
	}
}

void NwMatchManager::afterEnque(Safe<Copyable*> & currMsg) {
	gettimeofday(&m_timer, 0);
	TV_AS_USEC(m_timer, currtimestamp);
	if (m_Debug) {
		EyelockLog(logger, DEBUG, "	 %lu %lu -> %llu ", m_timer.tv_sec,
				m_timer.tv_usec, currtimestamp);
	}
	BinMessage *p = (BinMessage *) currMsg.get();
	p->SetTime(currtimestamp);

	currMsg.setUpdated(true);
	m_inQueue++;

}

void NwMatchManager::ReloadDB() {
	m_matchManager->AssignDB();
}

bool NwMatchManager::CheckCentroidRatio(IrisData* prev, IrisData* curr,
		MatchResult* res) {
	bool ret = false;
	IrisData *p, *c;
	if (m_logging) {
		if (prev->getFrameIndex() < curr->getFrameIndex()) {
			prev->PrintIRISData(m_logging);
			curr->PrintIRISData(m_logging);
			p = prev;
			c = curr;
		} else {
			curr->PrintIRISData(m_logging);
			prev->PrintIRISData(m_logging);
			p = curr;
			c = prev;
		}
	}
	CvPoint2D32f diff[2] = { 0 };
	// compute normalized separation of centroid from the iris center for image 0
	diff[0].x = (prev->getSpecCentroid().x - prev->getIrisCircle().x) * 100
			/ prev->getIrisCircle().z;
	diff[0].y = (prev->getSpecCentroid().y - prev->getIrisCircle().y) * 100
			/ prev->getIrisCircle().z;
	// compute normalized separation of centroid from the iris center for image 1
	diff[1].x = (curr->getSpecCentroid().x - curr->getIrisCircle().x) * 100
			/ curr->getIrisCircle().z;
	diff[1].y = (curr->getSpecCentroid().y - curr->getIrisCircle().y) * 100
			/ curr->getIrisCircle().z;
	// Compare the two normalized separations

	if (fabs(diff[0].x - diff[1].x) > m_SpecCentroidRatioThreshold.x
			&& fabs(diff[0].y - diff[1].y) > m_SpecCentroidRatioThreshold.y) {
		if (m_logging) {
			struct timeval m_timer;
			gettimeofday(&m_timer, 0);
			TV_AS_USEC(m_timer, t);
			FILE *fp = fopen("dump.txt", "a");
			int le = 0;
			fprintf(fp, "SECONDMATCH;%llu;%d;%5.2f;%5.2f;%s;%d;%d;\n", t,
					res->getEyeIndex(), fabs(diff[0].x - diff[1].x),
					fabs(diff[0].y - diff[1].y), res->getUID(le),
					curr->getFrameIndex(), curr->getEyeIndex());
			fclose(fp);
		}
		ret = true; // Its NOT Spoof
	}
	return ret; //
}

void NwMatchManager::SendF2FMessage(HTTPPOSTMsg& outMsg) {
	int val = -2;
	char *ptr = outMsg.getF2F(val);
	if (m_Debug) {
		EyelockLog(logger, DEBUG, "NwMatchManager::Update F2F %.*s", 30, ptr);
		for (int i = 0; i < outMsg.GetSize(); i++) {
			EyelockLog(logger, DEBUG, "%02x ", ptr[i]);
		}
	}

	if (ptr) {
		m_f2fResult.setF2F(ptr);
	}
	int len, bits;
	char *test = m_f2fResult.getF2F(len, bits);
	if (bits) {

		if (m_f2fDispatcher) {
			m_f2fDispatcher->SetSendingEveryNSec(true);
			m_f2fDispatcher->enqueMsg(m_f2fResult);
		} else {
			try {
				if (m_socketFactory && m_resultDestAddr) {
					SocketClient client = m_socketFactory->createSocketClient("Eyelock.F2FDispatcherSecure");
					client.SetTimeouts(m_timeOutSend);
					client.ConnectByHostname(*m_resultDestAddr);
					client.Send(outMsg, MSG_DONTWAIT);
				}
			} catch (Exception& nex) {
				EyelockLog(logger, ERROR, "NwMatchManager::SendMessage failed");
				fflush(stdout);
				nex.PrintException();
			}
		}
	}
}

bool NwMatchManager::UploadDB(char* fname,DBMsgType msgtype){
	ScopeLock lock(m_DBUpdateLock);
	bool ret = ExecuteAddUpdate(msgtype, fname);
	return ret;
}

bool NwMatchManager::ExecuteAddUpdate( int msgtype,char* tempDbFileName) {
	bool ret = false;
	ret = m_matchManager->UpdateDB((DBMsgType)((msgtype)), tempDbFileName);
	if (ret) {
		if (eREPLACEDB == msgtype) {
			ReloadDB();
		} else {
			TIME_OP("ADDED/DELETED",
			     m_matchManager->UpdateDBMap()
			);
			printf("Added/Deleted seems done successfully\n");
		}
	}
	remove(tempDbFileName);
	return ret;
}

void NwMatchManager::ProcessReceiveUsrFromSDK(HTTPPOSTMsg* msg) {
	if (m_ledDispatcher)m_ledDispatcher->SetDBUploadState();
	ScopeLock lock(m_DBUpdateLock);
	int msgtype;
	string fname;
	bool ret = msg->getSDKDBMsg(msgtype,fname);
	if(ret){
		ret = ExecuteAddUpdate(msgtype,(char*)fname.c_str());
	}
	if(m_ledDispatcher)m_ledDispatcher->SetInitialState();
}

int NwMatchManager::GetUserCount(bool excludeDummies)
{
	int count = -1;
	MatchManagerInterface* pMatchManagerInterface = GetMM();
	if (pMatchManagerInterface != NULL)
	{
		DBAdapter* pDbAdapter = pMatchManagerInterface->GetDbAdapter();
		if (pDbAdapter != NULL)
		{
			// get records count excluding dummies (names content "emptyxxx")
			count = pDbAdapter->GetUserCount(excludeDummies);
			//EyelockLog(logger, DEBUG, "Records count %d received from DbAdapter", count);
		}
		else
		{
			EyelockLog(logger, ERROR, "Cannot access DBAdapter");
		}
	}
	else
	{
		EyelockLog(logger, ERROR, "Cannot access MatchManagerInterface");
	}
	return count;
}

void NwMatchManager::ProcessReloadMsg(HTTPPOSTMsg* msg) {
	if (m_ledDispatcher)
		m_ledDispatcher->SetDBUploadState();
	ScopeLock lock(m_DBUpdateLock);
	bool ret=false;
	int msgtype;
	int filenumber = -1;
	int sd = 0;
	int st = 0;
	int isEncrypt = false;
	try{
		if (msg->getReloadDBParsedMsg(msgtype, filenumber, sd, st, isEncrypt)) {
			if (m_Debug)
				EyelockLog(logger, DEBUG, "msgtype %d, isEncrypt %d", msgtype,isEncrypt);
			if(eRELOADDB == msgtype){
				ReloadDB();
			}else{
				char tempDbFileName[1024] = { };
				sprintf(tempDbFileName, "%s_%d.tmp", m_irisCodeDatabaseFile,filenumber);
				bool ret = true;
				if (isEncrypt) {
					// Decrypting the SQLITE DB file here
					ret = DecryptDB(tempDbFileName);
				}
				if (ret)
					ret = ExecuteAddUpdate(msgtype, tempDbFileName);
				char ack[32] = { };
				if(ret){
					if(eREPLACEDB == msgtype){
						memcpy(ack,"RECEIVEDB;DONE;",15);
					}else{
						memcpy(ack,"UPDATEUSR;DONE;",15);
					}
				}else{
					if(eREPLACEDB == msgtype){
						memcpy(ack,"RECEIVEDB;NACK;",15);
					}else{
						memcpy(ack,"UPDATEUSR;NACK;",15);
					}
				}
				BinMessage Ack(ack, strlen(ack));
				EyelockLog(logger, DEBUG, "Sending ack for DB MSG %.*s", strlen(ack),Ack.GetBuffer());
				Socket client = SocketFactory::wrapSocket(sd, (SecureTrait*) (st));
				client.Send(Ack);
				client.CloseInput();
			}
		}
	} catch (Exception& ex) {
		ex.PrintException();
		EyelockLog(logger, ERROR, "Unable to process db reload network,ignoring");
	} catch(...){
		EyelockLog(logger, ERROR, "Sorry could not send the acknowledgement");
	}

	if(m_f2fDispatcher)m_f2fDispatcher->SetSendingEveryNSec(true);
	usleep(10000);
	if(m_ledDispatcher)m_ledDispatcher->SetInitialState();
	SendF2FMessage(m_F2FDbDoneMsg);
}

void NwMatchManager::process(Copyable *inpmsg) {
	HTTPPOSTMsg* msg = (HTTPPOSTMsg*) inpmsg;
	if (msg->isReloadDB()) {
		ProcessReloadMsg(msg);
		return;
	}else if(msg->isReceiveUsr()){
		ProcessReceiveUsrFromSDK(msg);
	}else{
		processmatch(inpmsg);
	}
	usleep(m_sleepTimeBetweenMatching);
	return;
}

void NwMatchManager::FlushIrisList(CHECK_IRIS chk) {
	switch (chk) {
	case CHECK_IRIS_BOTH:
		m_leftiris.FlushIris();
		m_rightiris.FlushIris();
		if (m_Debug)
			EyelockLog(logger, DEBUG, "Flushed the iris lists");
		break;
	case CHECK_IRIS_LEFT:
		m_leftiris.FlushAllButOne();
		if (m_Debug)
			EyelockLog(logger, DEBUG, "Flushed the left iris list");
		break;
	case CHECK_IRIS_RIGHT:
		m_rightiris.FlushAllButOne();
		if (m_Debug)
			EyelockLog(logger, DEBUG, "Flushed the right iris list");
		break;

	default:
		break;
	}
}

bool NwMatchManager::CheckIlluminator(CHECK_IRIS chk) {
	if (true == DontProceed4UniIris(chk)) {
		if (m_Debug) {
			if (CHECK_IRIS_LEFT == chk)
				EyelockLog(logger, DEBUG, "\nDontProceed4UniIris %s left eye", __FUNCTION__);
			else
				EyelockLog(logger, DEBUG, "\nDontProceed4UniIris %s right eye", __FUNCTION__);
		}
		return true;
	}

	bool bSuccess = false;
	//for left iris
	if (CHECK_IRIS_LEFT == chk) {
		if (NULL != m_leftiris.m_prevTrackedIris) {
			if (m_leftiris.m_matchedIris->getIlluminatorState()
					!= m_leftiris.m_prevTrackedIris->getIlluminatorState()) {
				bSuccess = true;
			}
		}
	} else {
		if (NULL != m_rightiris.m_prevTrackedIris) {
			if (m_rightiris.m_matchedIris->getIlluminatorState()
					!= m_rightiris.m_prevTrackedIris->getIlluminatorState()) {
				bSuccess = true;
			}
		}
	}
	return bSuccess;
}

bool NwMatchManager::PrevTracked(CHECK_IRIS chk, bool& bFlush) {
	bool bSuccess = false;
	if (CHECK_IRIS_LEFT == chk) {
		if (m_leftiris.m_iris.size() < 2 || NULL == m_leftiris.m_matchedIris) //for uni eye match
				{
			if (m_rightiris.m_iris.size() < 2
					|| NULL == m_rightiris.m_matchedIris)
				return false;
			else
				return true;
		}
		bSuccess = true;
		std::list<IrisData*>::reverse_iterator rviter =
				m_leftiris.m_iris.rbegin();
		if ((*rviter) == m_leftiris.m_matchedIris) {
			rviter++;
			m_leftiris.m_prevTrackedIris = (*rviter);
		} else {
			bFlush = true;
			rviter++;
			for (; rviter != m_leftiris.m_iris.rend(); rviter++) {
				if ((*rviter) == m_leftiris.m_matchedIris) {
					rviter--;
					m_leftiris.m_prevTrackedIris = (*rviter);
					break;
				}
			}

		}
	} else {
		if (m_rightiris.m_iris.size() < 2 || NULL == m_rightiris.m_matchedIris) //for uni eye match
				{
			if (m_leftiris.m_iris.size() < 2
					|| NULL == m_leftiris.m_matchedIris)
				return false;
			else
				return true;
		}

		bSuccess = true;
		std::list<IrisData*>::reverse_iterator rviter =
				m_rightiris.m_iris.rbegin();
		if ((*rviter) == m_rightiris.m_matchedIris) {
			rviter++;
			m_rightiris.m_prevTrackedIris = (*rviter);
		} else {
			bFlush = true;
			rviter++;
			for (; rviter != m_rightiris.m_iris.rend(); rviter++) {
				if ((*rviter) == m_rightiris.m_matchedIris) {
					rviter--;
					m_rightiris.m_prevTrackedIris = (*rviter);
					break;
				}
			}

		}

	}

	return bSuccess;
}

bool NwMatchManager::MatchPrevEntries(CHECK_IRIS chk) {
	if (true == DontProceed4UniIris(chk)) {
		if (m_Debug) {
			if (CHECK_IRIS_LEFT == chk)
				EyelockLog(logger, DEBUG, "\nDontProceed4UniIris %s left eye", __FUNCTION__);
			else
				EyelockLog(logger, DEBUG, "\nDontProceed4UniIris %s right eye", __FUNCTION__);
		}
		return true;
	}

	bool bSuccess = false;
	IrisData* prev = NULL;
	IrisData* curr = NULL;

	//for left iris
	char whichEye[10];
	strcpy(whichEye, (CHECK_IRIS_LEFT == chk) ? "left" : "right");

	if (CHECK_IRIS_LEFT == chk) {
		if (m_leftiris.IsPrevMatchedReady()) {
			curr = m_leftiris.m_matchedIris;
			prev = m_leftiris.m_prevTrackedIris;
		}

	} else {
		if (m_rightiris.IsPrevMatchedReady()) {
			curr = m_rightiris.m_matchedIris;
			prev = m_rightiris.m_prevTrackedIris;
		}

	}

	if (prev) {
		if (false == prev->getSegmentation()) //check the segmentation for tracked tracked eye
				{
			if (m_Debug)
				EyelockLog(logger, DEBUG, "\n Bad segmentation for %s tracked iris", whichEye);
			return bSuccess;
		}

		if (false == curr->getSegmentation()) //check the segmentation for matched eye
				{
			if (m_Debug)
				EyelockLog(logger, DEBUG, "\n Bad segmentation for %s matched iris", whichEye);
			return bSuccess;
		}

		std::pair<int, float> re21 = m_bioInstance->MatchIrisCodeSingle(
				(char*) curr->getIris(), (char*) prev->getIris(), m_maskval);
		if (re21.second < m_matchScoreThresh1
				&& re21.second > m_minTrackMatchThresh) {
			bSuccess = true;
			if (CHECK_IRIS_LEFT == chk)
				EyelockLog(logger, DEBUG, "\n IRIS left re21.second = %f ", re21.second);
			else
				EyelockLog(logger, DEBUG, "\n IRIS right re21.second = %f ", re21.second);
		}

		if (m_Debug) {
			if (bSuccess) {

				EyelockLog(logger, DEBUG, "\n Matched iris for %s eye....... ", whichEye);
				EyelockLog(logger, DEBUG,
						"\n Previous info ..frameIdx=%d..PrevTrackedIdx=%d..EyeIdx=%d ",
						prev->getFrameIndex(), prev->getPrevIndex(),
						prev->getEyeIndex());
				EyelockLog(logger, DEBUG,
						"\n Matched info ..frameIdx=%d..PrevTrackedIdx=%d..EyeIdx=%d ",
						curr->getFrameIndex(), curr->getPrevIndex(),
						curr->getEyeIndex());
			} else {
				EyelockLog(logger, DEBUG, "\n Previous tracked %s eye did not match", whichEye);
			}

		}
	} else {
		if (m_Debug)
			EyelockLog(logger, DEBUG, "\n Previous tracked %s eye not found ", whichEye);
	}

	return bSuccess;
}

bool NwMatchManager::CheckCentroid(CHECK_IRIS chk, bool& bFlushList) {
	if (true == DontProceed4UniIris(chk)) {
		if (m_Debug) {
			if (CHECK_IRIS_LEFT == chk)
				EyelockLog(logger, DEBUG, "\nDontProceed4UniIris %s left eye", __FUNCTION__);
			else
				EyelockLog(logger, DEBUG, "\nDontProceed4UniIris %s right eye", __FUNCTION__);
		}
		return true;
	}

	bool bSuccess = false;
	//for left iris
	if (CHECK_IRIS_LEFT == chk) {
		bSuccess = CheckCentroidRatio(m_leftiris.m_prevTrackedIris,
				m_leftiris.m_matchedIris, m_leftiris.m_matchedResult);
		if (false == bSuccess) {
			if (1
					== abs(
							m_leftiris.m_prevTrackedIris->getFrameIndex()
									- m_leftiris.m_matchedIris->getFrameIndex()))
				bFlushList = true;
		}
	} else {
		bSuccess = CheckCentroidRatio(m_rightiris.m_prevTrackedIris,
				m_rightiris.m_matchedIris, m_rightiris.m_matchedResult);
		if (false == bSuccess) {
			if (1
					== abs(
							m_rightiris.m_prevTrackedIris->getFrameIndex()
									- m_rightiris.m_matchedIris->getFrameIndex()))
				bFlushList = true;
		}
	}
	return bSuccess;
}

void NwMatchManager::SetMatchedIris(CHECK_IRIS chk, MatchResult *result) {
	if (m_leftiris.m_matchedIris && m_rightiris.m_matchedIris)
		return;

	if (CHECK_IRIS_LEFT == chk) {
		if (NULL == m_leftiris.m_matchedIris) {
			m_leftiris.m_matchedResult = new MatchResult(*result);
			m_leftiris.m_matchedIris = m_leftiris.m_iris.back();
			SetIrisState(UT_IRIS_MATCH_L);
		}
	} else {
		if (NULL == m_rightiris.m_matchedIris) {
			m_rightiris.m_matchedResult = new MatchResult(*result);
			m_rightiris.m_matchedIris = m_rightiris.m_iris.back();
			SetIrisState(UT_IRIS_MATCH_R);
		}
	}
}

void NwMatchManager::Check4IrisTimeWindowThreshold(IrisData* irisCurr) {
	/*
	 if(m_spoofEnable)
	 {
	 if(m_leftiris.CheckSpoofThreshold(m_spoofTimeThresholdSec,irisCurr->getTimeStamp(),m_striclyFlushList4IntraEye))
	 if(m_Debug)EyelockLog(logger, DEBUG, " Spoof threshold exceeded for left");

	 if(m_rightiris.CheckSpoofThreshold(m_spoofTimeThresholdSec,irisCurr->getTimeStamp(),m_striclyFlushList4IntraEye))
	 if(m_Debug)EyelockLog(logger, DEBUG, " Spoof threshold exceeded for right");
	 }
	 */
	if (m_dualMatchEnabled) {
		if (m_leftiris.m_iris.size() > 0 && m_rightiris.m_iris.size() > 0) {
			bool shallFlush = false;
			int leftTSdiff =
					abs(
							int(
									irisCurr->getTimeStamp()
											- m_leftiris.m_iris.back()->getTimeStamp()));
			int rightTSdiff =
					abs(
							int(
									irisCurr->getTimeStamp()
											- m_rightiris.m_iris.back()->getTimeStamp()));
			if (leftTSdiff > m_intraEyeTimeWindowThreshold
					|| rightTSdiff > m_intraEyeTimeWindowThreshold) {
				shallFlush = true;
			}

			if (m_Debug)
				EyelockLog(logger, DEBUG,
						"\n left = %llu, right = %llu , leftTSdiff = %d, rightTSdiff = %d",
						m_leftiris.m_iris.back()->getTimeStamp(),
						m_rightiris.m_iris.back()->getTimeStamp(), leftTSdiff,
						rightTSdiff);

			if (shallFlush) {

				if (m_striclyFlushLists) {
					if (m_Debug)
						EyelockLog(logger, DEBUG,
								"\n Did not capture both eyes within the iris time window threshold. Flushing each.");
					m_leftiris.FlushIris();
					m_rightiris.FlushIris();
				} else {
					if (m_Debug)
						EyelockLog(logger, DEBUG,
								"\n Did not capture both eyes within the iris time window threshold. Flushing all but one.");
					m_leftiris.FlushAllButOne();
					m_rightiris.FlushAllButOne();
				}
			}
		}
	} else {
		if (m_leftiris.CheckInterEyeTimeWindowThreshold(
				m_interIrisTimeWindowThresholdMilliSec,
				irisCurr->getTimeStamp(), m_striclyFlushLists))
			if (m_Debug)
				EyelockLog(logger, DEBUG, " Iris time window threshold exceeded for left");

		if (m_rightiris.CheckInterEyeTimeWindowThreshold(
				m_interIrisTimeWindowThresholdMilliSec,
				irisCurr->getTimeStamp(), m_striclyFlushLists))
			if (m_Debug)
				EyelockLog(logger, DEBUG, " Iris time window threshold exceeded for right");
	}
}

void NwMatchManager::FillIrisLists(MatchResult *result, IrisData* irisCurr) {
	if (result && result->getScore() < m_matchScoreThresh) {
		FlushLoitering();
		result->setState(PASSED);
	}

	//fill the respective lists
	if (m_leftiris.m_iris.empty()) {
		m_leftiris.m_iris.push_back(irisCurr);
		if (result && result->getScore() < m_matchScoreThresh)
			SetMatchedIris(CHECK_IRIS_LEFT, result);
	} else {
		if (m_rightiris.m_iris.empty()) {
			IrisData* last = m_leftiris.m_iris.back();
			int len = MAX(strlen(last->getCamID()),
					strlen(irisCurr->getCamID()));
			if (0 == memcmp(last->getCamID(), irisCurr->getCamID(), len)) {
				m_leftiris.m_iris.push_back(irisCurr);
				if (result && result->getScore() < m_matchScoreThresh)
					SetMatchedIris(CHECK_IRIS_LEFT, result);
			} else {
				m_rightiris.m_iris.push_back(irisCurr);
				if (result && result->getScore() < m_matchScoreThresh)
					SetMatchedIris(CHECK_IRIS_RIGHT, result);

			}
		} else {
			IrisData* last = m_rightiris.m_iris.back();
			int len = MAX(strlen(last->getCamID()),
					strlen(irisCurr->getCamID()));
			if (0 == memcmp(last->getCamID(), irisCurr->getCamID(), len)) {
				m_rightiris.m_iris.push_back(irisCurr);
				if (result && result->getScore() < m_matchScoreThresh)
					SetMatchedIris(CHECK_IRIS_RIGHT, result);
			} else {
				m_leftiris.m_iris.push_back(irisCurr);
				if (result && result->getScore() < m_matchScoreThresh)
					SetMatchedIris(CHECK_IRIS_LEFT, result);
			}
		}
	}
}

void NwMatchManager::RefreshIrisLists() {

	if (m_leftiris.m_iris.size() == MAX_IRIS_LIST_SIZE) {
		if (m_Debug)
			EyelockLog(logger, DEBUG, " Refreshing left iris list");
		m_leftiris.RefreshList();
	}

	if (m_rightiris.m_iris.size() == MAX_IRIS_LIST_SIZE) {
		if (m_Debug)
			EyelockLog(logger, DEBUG, " Refreshing right iris list");
		m_rightiris.RefreshList();
	}

}

bool NwMatchManager::DontProceed4UniIris(CHECK_IRIS chk) {
	if (m_dualMatchEnabled)
		return false; //no point in going further

	CHECK_IRIS uniMatcherValidEye = CHECK_IRIS_NONE;
	bool bSuccess = false;
	if (CHECK_IRIS_LEFT == chk) {
		if (NULL == m_leftiris.m_matchedIris) {
			if (NULL != m_rightiris.m_matchedIris) {
				if (m_rightiris.m_iris.size() >= 2
						&& NULL != m_rightiris.m_prevTrackedIris) {
					uniMatcherValidEye = CHECK_IRIS_RIGHT;
				}
			}
		} else {
			if (m_leftiris.m_iris.size() >= 2
					&& NULL != m_leftiris.m_prevTrackedIris) {
				uniMatcherValidEye = CHECK_IRIS_LEFT;
			}
		}
	} else {
		if (NULL == m_rightiris.m_matchedIris) {
			if (NULL != m_leftiris.m_matchedIris) {
				if (m_leftiris.m_iris.size() >= 2
						&& NULL != m_leftiris.m_prevTrackedIris) {
					uniMatcherValidEye = CHECK_IRIS_LEFT;
				}
			}
		} else {
			if (m_rightiris.m_iris.size() >= 2
					&& NULL != m_rightiris.m_prevTrackedIris) {
				uniMatcherValidEye = CHECK_IRIS_RIGHT;
			}
		}

	}

	bSuccess = !(uniMatcherValidEye & chk);
	return bSuccess;
}

bool NwMatchManager::CheckUID() {
	bool bSuccess = false;
	int lenL = -1;
	char *uidLeft = m_leftiris.m_matchedResult->getUID(lenL);
	int lenR = -1;
	char *uidRight = m_rightiris.m_matchedResult->getUID(lenR);
	int len = lenR > lenL ? lenR : lenL;
	if (0 == len) //the uid has not been sent therefore dont check it further
			{
		bSuccess = true;
		if (m_Debug)
			EyelockLog(logger, DEBUG, "UID length is zero");
	} else if (0 == memcmp(uidLeft, uidRight, len)) //the uids must be different for both eyes
			{
		bSuccess = true;
	}

	if (m_Debug)
		EyelockLog(logger, DEBUG, "UID of left = %s, right = %s", uidLeft, uidRight);

	return bSuccess;
}
void * timeoutThreadHelper(void * data) {
	long int tid = syscall(SYS_gettid);
	printf("***************%s thread %u ********************* \n","timeoutThreadHelper",tid);
	NwMatchManager * pt = (NwMatchManager *) data;
	pt->timeoutThread(data);

	return NULL;
}

void* NwMatchManager::timeoutThread(void * data) {
	/*bool m_bNegativeMatchTimerActive;
	 int m_iWaitTime;
	 int m_iResetTimer;  //reset timer matches the repeat authorization timer
	 time_t m_tLastTimeOutWindowStarted;
	 bool m_bNegativeTimeoutKill;
	 bool m_bTimeoutActive;*/
	NwMatchManager * pt = (NwMatchManager *) data;
	bool doQuit = ShouldIQuit();
	while(!doQuit)
	{
		doQuit = ShouldIQuit();
		while(!pt->m_bTimeoutActive)
		{
			bool doQuit = ShouldIQuit();
			if(doQuit)
				return 0;
			usleep(50000); //should be 20 hz:

		}

		EyelockLog(logger, DEBUG, "NwMatchManager::timeoutThread thread start");
		
		CURR_TV_AS_MSEC(start);
		int current = 0;
		while (current < pt->m_iWaitTime) {
			if (pt->m_bNegativeTimeoutKill) {
				if (m_Debug)
					EyelockLog(logger, DEBUG, "NwMatchManager::timeoutThread killed timeout thread.");
				CURR_TV_AS_SEC(current);
				pt->m_tLastTimeOutWindowStarted = current;

				pt->m_bTimeoutActive = false;
				break;
			}
			usleep(500000);
			CURR_TV_AS_MSEC(curr);
			current = curr - start;
		
			if (m_Debug)
				EyelockLog(logger, DEBUG, "NwMatchManager::timeoutThread time till timeout:  %d, %d.",current, pt->m_iWaitTime);
		}
		if(pt->m_bTimeoutActive)
		{
			//trigger negative result
			CURR_TV_AS_SEC(current);
			pt->m_tLastTimeOutWindowStarted = current;
			pt->m_bTimeoutActive = false;
			pt->failMatch.init();
			pt->failMatch.setState(FAILED);
			EyelockLog(logger, DEBUG, "NwMatchManager::timeoutThread Match timeout (failed).");
			//xxlog("Match failed\n", 0);
			//EyelockEvent("Match failed\n", 0);
			MatchDetected(&failMatch);

			// fjia: cleanup for memory leak
			FlushLoitering();
			FlushIrisList(CHECK_IRIS_BOTH);
		}
	}
}

void logMatch(string guid,string name){
	//EyelockLog(logger, DEBUG, "Matched %s, name is %s\n", guid.c_str(), name.c_str());
	//xxlog("Match success ID is %s\n", name.c_str()  );
	//EyelockEvent("Match success ID is %s\n", name.c_str()  );
}

void NwMatchManager::ResetNegativeMatch() {
	if (m_bNegativeMatchTimerActive) {
		//a match succeeded, kill the timeout thread
		//EyelockLog(logger, DEBUG,"NwMatchManager::timeoutThread match success, shutdown timeout thread.");
		m_bNegativeTimeoutKill = true;
		CURR_TV_AS_SEC(current);
		m_tLastTimeOutWindowStarted = current;
	}
}

void NwMatchManager::processmatch(Copyable *inpmsg) {

	bool tempdualMatch  = m_dualMatchEnabled;

	try {
		if(m_negativeMatchEnable){
			if(m_Debug)EyelockLog(logger, DEBUG, "foo match! timer active %d timeout %d timer running %d",
					m_bNegativeMatchTimerActive, m_iResetTimer, m_bTimeoutActive);
			//This is the primary matcher entry point -Michael Hester
			//I'll work this using a pThread.  When that thread times out it will inject a message into the queue
			//that says "timeout".  The thread can be killed beforehand by setting the kill variable to true.  THat can
			//be done in this function.
			if (m_bNegativeMatchTimerActive && !m_bTimeoutActive) {
				//time_t current = clock();
				//time_t dt = current - m_tLastTimeOutWindowStarted;
				//double dtx = (double) dt / CLOCKS_PER_SEC;
				CURR_TV_AS_SEC(current);
				int dtx = (current > m_tLastTimeOutWindowStarted) ? (current-m_tLastTimeOutWindowStarted) : m_iResetTimer;
				if(m_Debug)EyelockLog(logger, DEBUG, "foo timer %d vs remaining %d", (int) (dtx),m_iResetTimer);
				//to start a timeout window: the previous window must be done and the timeout must not be active.
				if (!m_bTimeoutActive && (int) (dtx) > m_iResetTimer) {
					pthread_t threadx;
					m_bNegativeTimeoutKill = false;
					m_tLastTimeOutWindowStarted = current;
					m_bTimeoutActive = true;
					if (!negativeMatchThreadRunning) {
						negativeMatchThreadRunning = true;
						pthread_create(&threadx, NULL, timeoutThreadHelper, this);
					}
					if(m_Debug)EyelockLog(logger, DEBUG, "NwMatchManager::timeoutThread Match timeout thread created.");
				}
			}
		}

		BinMessage* iriscodeMsg = (BinMessage*) inpmsg;
		MatchResult *result = NULL;
		NwMatcherSerialzer nws;
		m_mr.reset();
		m_mr.setState(FAILED);

		IrisData *irisCurr = new IrisData();

		nws.ExtractNwMsg(irisCurr, iriscodeMsg->GetBuffer());
		if(atoi(irisCurr->getCamID()) != 0){
			CURR_TV_AS_USEC(ts1);
			irisCurr->setTimeStamp(ts1);
		}

		if (m_loiteringDetector)
			m_loiteringDetector->enqueMsg(*irisCurr);

		if (false == irisCurr->getIrisRadiusCheck())//check if the iris radius is good
				{
			if (m_Debug)
				EyelockLog(logger, DEBUG, " Discard iris for bad radius");
			return;
		}

		Check4IrisTimeWindowThreshold(irisCurr);
		RefreshIrisLists();

		//try matching each of the
		if ((NULL == m_leftiris.m_matchedIris) || (NULL == m_rightiris.m_matchedIris)) {
			XTIME_OP("NwMATCHM matcher",
					result=m_matchManager->DoMatch((unsigned char*)iriscodeMsg->GetBuffer()));
			result->setFrameInfo(irisCurr->getFrameIndex(),irisCurr->getEyeIndex(),(char*)irisCurr->getCamID());
			result->setTimeStamp(irisCurr->getTimeStamp());
			if((result-> getScore() < m_matchScoreThresh)&& m_singleIrisinDual){
				if(result->GetPersonIrisInfo() == SINGLE){
					tempdualMatch = false;
				}
			}
			if (false == m_matchManager->CheckValidityOfHDM()){
				if (m_Debug)EyelockLog(logger, DEBUG, " Discard iris for bad hamming distance");
				return;
			}
		}

		FillIrisLists(result, irisCurr);

		if (m_Debug)
			EyelockLog(logger, DEBUG, " Left size = %d  $$ Right size = %d",m_leftiris.m_iris.size(), m_rightiris.m_iris.size());

		if (tempdualMatch) {
			if (m_leftiris.m_iris.size() < 2 || m_rightiris.m_iris.size() < 2) {
				return;
			}
		} else {
			if(!m_intraEyeTimeWindowEnable){
				if(!((m_leftiris.m_iris.size() >0) || (m_rightiris.m_iris.size() > 0))) {
					return;
				}
			}else{
				if (m_leftiris.m_iris.size() < 2 && m_rightiris.m_iris.size() < 2) {
					return;
				}
			}
		}

		if (tempdualMatch) {
			if ((NULL == m_leftiris.m_matchedIris)|| (NULL == m_rightiris.m_matchedIris)) {
				if ((NULL != m_leftiris.m_matchedIris)|| (NULL != m_rightiris.m_matchedIris)) {
					FlushLoitering();
				}
				SetIrisState(UT_IRIS_NO_MATCH);
				return;
			}
		} else {
			if ((NULL == m_leftiris.m_matchedIris)&& (NULL == m_rightiris.m_matchedIris)) {
				SetIrisState(UT_IRIS_NO_MATCH);
				return;
			}
		}

		if (m_spoofEnable) {

			bool bFlushLeft = false;
			if (false == PrevTracked(CHECK_IRIS_LEFT, bFlushLeft)) {
				if (m_Debug)
					EyelockLog(logger, DEBUG, "Previous tracked left eye not found");
				return;
			}
			if (false == CheckIlluminator(CHECK_IRIS_LEFT)) {
				FlushLoitering();
				SetIrisState(UT_IRIS_ILLUM_L);
				if (m_Debug)
					EyelockLog(logger, DEBUG, " Left illuminator failed");
				return;
			}

			bool bFlushRight = false;
			if (false == PrevTracked(CHECK_IRIS_RIGHT, bFlushRight)) {
				if (m_Debug)
					EyelockLog(logger, DEBUG, "Previous tracked right eye not found");
				return;
			}

			if (false == CheckIlluminator(CHECK_IRIS_RIGHT)) {
				FlushLoitering();
				SetIrisState(UT_IRIS_ILLUM_R);
				if (m_Debug)
					EyelockLog(logger, DEBUG, " right illuminator failed");
				return;
			}

			//match with previous iris entries
			if (false == MatchPrevEntries(CHECK_IRIS_LEFT)) {
				if (bFlushLeft) {
					FlushLoitering();
					FlushIrisList(CHECK_IRIS_LEFT);
				}
				SetIrisState(UT_IRIS_MATCH_PREV_L);
				if (m_Debug)
					EyelockLog(logger, DEBUG, " Left match prev failed");
				return;
			}

			if (false == MatchPrevEntries(CHECK_IRIS_RIGHT)) {
				if (bFlushRight) {
					FlushLoitering();
					FlushIrisList(CHECK_IRIS_RIGHT);
				}
				SetIrisState(UT_IRIS_MATCH_PREV_R);
				if (m_Debug)
					EyelockLog(logger, DEBUG, " right match prev failed");
				return;
			}

			if (m_enableCentroidRatio) {
				bool bFlushList = false;
				//check centroid
				if (false == CheckCentroid(CHECK_IRIS_LEFT, bFlushList)) {
					if (bFlushList) {
						FlushLoitering();
						FlushIrisList(CHECK_IRIS_LEFT);
					}
					SetIrisState(UT_IRIS_CENTROID_L);
					if (m_Debug)
						EyelockLog(logger, DEBUG, " Left centroid failed");
					return;
				}
				bFlushList = false;
				if (false == CheckCentroid(CHECK_IRIS_RIGHT, bFlushList)) {
					if (bFlushList) {
						FlushLoitering();
						FlushIrisList(CHECK_IRIS_RIGHT);
					}
					SetIrisState(UT_IRIS_CENTROID_R);
					if (m_Debug)
						EyelockLog(logger, DEBUG, " right centroid failed");
					return;
				}
			}

			if (m_Debug)
				EyelockLog(logger, DEBUG, "Successfully passed the spoof test");
		}

		SetIrisState(UT_IRIS_MATCH_SUCCESS);

#ifndef UNITTEST
		if (tempdualMatch) {
			if (false == CheckUID())
				return;
			int len = 0;
			char * id = m_leftiris.m_matchedResult->getUID(len);
			logMatch(m_leftiris.m_matchedResult->getGUID(),m_leftiris.m_matchedResult->getName());
			MatchDetected(m_leftiris.m_matchedResult);
			MatchDetected(m_rightiris.m_matchedResult);
			ResetNegativeMatch();
		} else {
			if (m_leftiris.m_matchedResult){
				int len = 0;
				char * id = m_leftiris.m_matchedResult->getUID(len);
				logMatch(m_leftiris.m_matchedResult->getGUID(),m_leftiris.m_matchedResult->getName());
				MatchDetected(m_leftiris.m_matchedResult);
			}else{
				int len = 0;
				char * id = m_rightiris.m_matchedResult->getUID(len);
				logMatch(m_rightiris.m_matchedResult->getGUID(),m_rightiris.m_matchedResult->getName());
				MatchDetected(m_rightiris.m_matchedResult);
			}
			ResetNegativeMatch();
		}
#endif
		FlushLoitering();
		FlushIrisList(CHECK_IRIS_BOTH);

	} catch (Exception& nex) {
		nex.PrintException();
	} catch (const char *msg) {
		std::cout << msg << endl;
	} catch (...) {
		std::cout << "Unknown exception eye matcher during accepting iris"
				<< endl;
	}

}
bool NwMatchManager::DecryptDB( char* tempDbFileName) {
	if (m_Debug)
		EyelockLog(logger, DEBUG, " DecryptDB() - filename %s", tempDbFileName);

	char certPW[50] = {0};
	if (!m_certFile) {
		m_certFile = (char *)malloc(100);
		if (!m_certFile){
			EyelockLog(logger, ERROR, "malloc failed in m_certFile");
			return false;
		}

		vector<pair<string,int64_t> > keysVec;
		DBAdapter_Keys db;
#ifdef HBOX_PG		
		if (0 == db.OpenFile((char*)"keys.db3")) {
			EyelockLog(logger, ERROR, "Failed to open file keys.db3");
#else
		if (0 == db.OpenFile((char*)"keys.db")) {
			EyelockLog(logger, ERROR, "Failed to open file keys.db");
#endif			
			return false;
		}

		if(0 != db.ReadDB(keysVec)){
			db.CloseConnection();
			return false;
		}
		db.CloseConnection();

		unsigned int i;
		for(i=0;i<keysVec.size();i++){
			if(0 != strcmp(keysVec[i].first.c_str(),"eyelock-pc")){
				sprintf(m_certFile, "./rootCert/certs/%s.key", keysVec[i].first.c_str());
				break;
			}
		}
		if (i == keysVec.size()) {
			strcpy(m_certFile, "./rootCert/certs/eyelock-pc.key");
		}
	}
	strcpy(certPW, "eyelock");
	if (m_Debug)
		EyelockLog(logger, DEBUG, " DecryptDB() - key %s, pw %s", m_certFile, certPW);

	// read file to an array
	FILE *fp = fopen(tempDbFileName, "r");
	if (fp == NULL) {
		EyelockLog(logger, ERROR, "Failed to open file %s", tempDbFileName);
		return false;
	}

	fseek(fp, 0, SEEK_END);
	unsigned int size = ftell(fp);
	rewind (fp);
	unsigned char *buffer = (unsigned char *)malloc(size);
	unsigned char *bufferOut = (unsigned char *)malloc(size);
	if (buffer == NULL || bufferOut == NULL) {
		EyelockLog(logger, ERROR, "Failed to allocate memory %d bytes", size);
		return false;
	}

	unsigned int len = fread (buffer,sizeof(char),size,fp);
	if (len != size) {
		EyelockLog(logger, ERROR, "Failed to read DB file to buffer %d bytes", size);
		fclose (fp);
		return false;
	}
	fclose (fp);

	// decrypt DB
	unsigned char keyDecrypt[128];
	unsigned char ivDecrypt[128];
	int result;
	// [128 bytes RSA encrypted key] [128 bytes RSA encrypted iv] [remaining bytes AES encrypted payload]
	memset(keyDecrypt, 0, 128);
	memset(ivDecrypt, 0, 128);
	unsigned char *key = &buffer[0];
	unsigned char *iv = &buffer[128];
	unsigned char *aesData = buffer + 128 * 2;
	len = len - 128 * 2;

	//OpenSSLSupport *mySSL = new OpenSSLSupport();
	result = OpenSSLSupport::instance().privateDecrypt(key, keyDecrypt, m_certFile, certPW);
	if (result == -1) {
		EyelockLog(logger, ERROR, "Failed to decrypt key");
		return false;
	}
	result = OpenSSLSupport::instance().privateDecrypt(iv, ivDecrypt, m_certFile, certPW);
	if (result == -1) {
		EyelockLog(logger, ERROR, "Failed to decrypt iv");
		return false;
	}

	AES *myAes = new AES();
	myAes->SetKey(keyDecrypt,32);	// 32 bytes for key after decrypted
	myAes->SetIV(ivDecrypt,16);		// 16 bytes for iv after decrypted
	myAes->Decrypt(aesData,bufferOut,len);
	//delete mySSL;
	delete myAes;

	fp = fopen(tempDbFileName, "w");
	if (fp == NULL || len != fwrite (bufferOut, sizeof(char), len, fp)) {
		EyelockLog(logger, ERROR, "Failed to write back DB file from buffer %d bytes", size);
		if (fp)
			fclose (fp);
		return false;
	}
	fflush(fp);
	fclose (fp);
	free (buffer);
	free (bufferOut);

	return true;
}

