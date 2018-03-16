/*
 * LoiteringDetector.cpp
 *
 *  Created on: Oct 11, 2012
 *      Author: mamigo
 */
#include <iostream>
#include "Configuration.h"
#include "IrisData.h"
#include "SocketFactory.h"
#include "LoiteringDetector.h"
#include "NwLEDDispatcher.h"
#include "LEDConsolidator.h"
#include <unistd.h>

#include "UtilityFunctions.h"
#include <vector>


LoiteringDetector::LoiteringDetector(Configuration& conf)
:GenericProcessor(conf)
,m_outMasterMsg(1024*3)
,m_outSlaveMsg(1024*3)
,m_sleepTimeuSec(500)
,m_sendmsgMaster(false)
,m_sendmsgSlave(false)
,m_Debug(false)
,m_LedConsolidator(NULL)
,m_matchedTimeStamp(0),m_socketFactory(NULL),m_resultDestAddr(NULL)
,m_flushState(FLUSH_UNKNOWN)
{
	m_timeDiffence = conf.getValue("Eyelock.DuckTimeDifferenceMilliSec",3000);
	m_timeDiffence = m_timeDiffence*1000;

	m_silentTime = conf.getValue("GRI.RepeatAuthorizationPeriod",2000);
	m_silentTime = m_silentTime*1000;

	const char *svrAddr = conf.getValue("Eyelock.LoiteringDestAddr", "NONE");
	if(strcmp(svrAddr,"NONE") != 0){
		m_resultDestAddr= HostAddress::MakeHost(svrAddr, eIPv4, false);
		m_socketFactory = new SocketFactory(conf);
	}
	int timeOutms = conf.getValue("Eyelock.LoiteringSocketTimeOutMillis", 500);
	m_timeOut.tv_sec = timeOutms / 1000;
	m_timeOut.tv_usec = (timeOutms % 1000) * 1000;

	m_msgFormat=conf.getValue("Eyelock.LoiteringMsgFormat","LOITERING;Camera:%s;Time:%llu;HALO:%f;IRIS:%d:");
	m_sleepTimeuSec = conf.getValue("Eyelock.LoiteringSleepMilliSec",1000);
	m_sleepTimeuSec = m_sleepTimeuSec*1000;

	const char *camid =  conf.getValue("GRI.cameraID","NONE");
	m_CamStr.assign(camid);

	m_Debug = conf.getValue("Eyelock.LoiteringDebug",false);

	const char *svrAddr1 = conf.getValue("Eyelock.TSMasterDestAddr", "NONE");
	if(strcmp(svrAddr1, "NONE") == 0){
		m_tsDestAddrpresent = false;
	}else
		m_tsDestAddrpresent = true;

	m_flushingTime = conf.getValue("Eyelock.LoiteringFlushingTimeMilliSec",0);
	m_flushingTime = m_flushingTime*1000 + m_timeDiffence + m_silentTime + m_sleepTimeuSec;
}

LoiteringDetector::~LoiteringDetector() {
	if(m_resultDestAddr)
		delete m_resultDestAddr;
	if(m_socketFactory)
		delete m_socketFactory;
}

void LoiteringDetector::SendMessage()
{
	LEDResult ld;
	ld.setState(LED_LOITERING);
	if(m_tsDestAddrpresent){
		if(m_nwLedDispatcher)m_nwLedDispatcher->enqueMsg(ld);
		return;
	}
	if(m_LedConsolidator){
		if(m_LedConsolidator)m_LedConsolidator->enqueMsg(ld);
	}
}


bool LoiteringDetector::DetectLoitering(vector<IrisData>& irisVector,BinMessage& outMsg)
{
	int count = -1;
	bool checkloitering = false;
	bool sendmsg = false;
	float min = 1000;
	int sz = irisVector.size();
	if(sz > 1){
         if((irisVector[sz-1].getTimeStamp() - irisVector[0].getTimeStamp()) > m_timeDiffence){
        	  if(m_Debug)printf("Loitering because diff TS %llu - %llu > %lld \n",irisVector[sz-1].getTimeStamp(), irisVector[0].getTimeStamp(),m_timeDiffence);
              checkloitering = true;
          }
    }

	if(checkloitering){
		for(int i = 0;i < sz;i++){
            //printf("%s %llu %d %d %f \n",irisVector[i].getCamID(),irisVector[i].getTimeStamp()/1000000,irisVector[i].getFrameIndex(),irisVector[i].getEyeIndex(),irisVector[i].getHalo());
            if(irisVector[i].getHalo() < min){
			  min = irisVector[i].getHalo();
				count = i;
			}
        }
        if(count > -1){
        	if(m_Debug)printf("Intruder and send IRIS %d Cam ID %s %d\n",count,irisVector[count].getCamID(),irisVector[count].getFrameIndex());
            //CreateMessage(&irisVector[count],outMsg);
            sendmsg = true;
        }
        irisVector.clear();
    }
    return sendmsg;
}
void LoiteringDetector::ClearVectors(bool & flushdone){
	m_irisVectorMaster.clear();
	m_irisVectorSlave.clear();
	flushdone= true;
}


void LoiteringDetector::CheckLoitering(){

	if(m_Debug)
		PrintData();
	ScopeLock lock(m_vectorLock);
	m_sendmsgMaster = false;
	m_sendmsgSlave = false;
	bool flushdone = false;
	bool loiteringdetected = false;
	if(m_Debug){
		if((m_irisVectorMaster.size() > 0) || (m_irisVectorSlave.size() > 0)){
			printf("Loitering Vector Sz %d %d\n",m_irisVectorMaster.size(),m_irisVectorSlave.size());
		}
	}

	CURR_TV_AS_USEC(ts);

	if(m_matchedTimeStamp){
		if((ts - m_matchedTimeStamp) <=  m_silentTime){
			if(m_Debug)printf("LoiteringDetector::Clearing Vector %llu \n",ts/1000000);
			ClearVectors(flushdone);
			m_flushState = WITHIN_SILENT_TIME;
		}
	}

	if(!flushdone){
		m_sendmsgMaster = DetectLoitering(m_irisVectorMaster,m_outMasterMsg);
		if(m_sendmsgMaster){
			loiteringdetected = true;
		}

		m_sendmsgSlave = DetectLoitering(m_irisVectorSlave,m_outSlaveMsg);
		if(m_sendmsgSlave){
			loiteringdetected = true;
		}
	}

	if(loiteringdetected){
		CURR_TV_AS_USEC(ts);
		ClearVectors(loiteringdetected);
		m_matchedTimeStamp= ts;
		m_flushState = WITHIN_STIPULATED_TIME;
	}else{
		if(m_irisVectorMaster.size()){
			if((ts - m_irisVectorMaster[0].getEnqueTimeStamp()) > m_flushingTime){
				ClearVectors(loiteringdetected);
				m_flushState = PERIODIC_CLEANING;
			}
		}
		if(m_irisVectorSlave.size()){
			if((ts - m_irisVectorSlave[0].getEnqueTimeStamp()) > m_flushingTime){
				ClearVectors(loiteringdetected);
				m_flushState = PERIODIC_CLEANING;
			}
		}
	}

	return ;
}

void LoiteringDetector::PrintData(){
	ScopeLock lock(m_vectorLock);
	for(int i=0;i<m_irisVectorMaster.size();i++){
		printf("%s %llu [%llu] %d %d %f \n",m_irisVectorMaster[i].getCamID(),m_irisVectorMaster[i].getTimeStamp()/1000,m_irisVectorMaster[i].getEnqueTimeStamp()/1000,m_irisVectorMaster[i].getFrameIndex(),m_irisVectorMaster[i].getEyeIndex(),m_irisVectorMaster[i].getHalo());
	}
	for(int i=0;i<m_irisVectorSlave.size();i++){
		printf("%s %llu [%llu] %d %d %f \n",m_irisVectorSlave[i].getCamID(),m_irisVectorSlave[i].getTimeStamp()/1000,m_irisVectorSlave[i].getEnqueTimeStamp()/1000,m_irisVectorSlave[i].getFrameIndex(),m_irisVectorSlave[i].getEyeIndex(),m_irisVectorSlave[i].getHalo());
	}
}


bool LoiteringDetector::enqueMsg(Copyable& msg) {
	ScopeLock lock(m_vectorLock);
	IrisData& id = (IrisData&) msg;
	CURR_TV_AS_USEC(ts)
	id.setEnqueTimeStamp(ts);
	if(m_CamStr.compare(id.getCamID())==0){
		m_irisVectorMaster.push_back(id);
	}else
		m_irisVectorSlave.push_back(id);

	return true;
}

bool LoiteringDetector::ClearVector() {
	ScopeLock lock(m_vectorLock);
	if(m_Debug)
		printf("LoiteringDetector::Clear the Loitering Queue\n ");
	m_irisVectorMaster.clear();
	m_irisVectorSlave.clear();
	m_sendmsgMaster = false;
	m_sendmsgSlave = false;

	CURR_TV_AS_USEC(ts)
	m_matchedTimeStamp= ts;
	return true;
}


unsigned int LoiteringDetector::MainLoop() {
	std::string name = "LoiteringDetector::";
	try {
		while (!ShouldIQuit()) {
			CheckLoitering();
			if(m_sendmsgMaster || m_sendmsgSlave){
				if(m_Debug){
					if(m_sendmsgMaster)
						printf("Loitering Detected in Master \n");
					if(m_sendmsgSlave)
						printf("Loitering Detected in Slave \n");
				}
				SendMessage();
			}
			Frequency();
			usleep(m_sleepTimeuSec);
		}
	} catch (std::exception& ex) {
		cout << name << ex.what() << endl;
		cout << name << "exiting thread" << endl;
	} catch (Exception& ex1) {
		ex1.PrintException();
		cout << name << "exiting thread" << endl;
	} catch (const char *msg) {
		cout << name << msg << endl;
		cout << name << "exiting thread" << endl;
	} catch (...) {
		cout << name << "Unknown exception! exiting thread" << endl;
	}

	return 0;
}
