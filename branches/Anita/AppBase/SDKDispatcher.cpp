/*
 * SDKDispatcher.cpp
 *
 *  Created on: Jan 1, 2015
 *      Author: developer
 */

#include "SDKDispatcher.h"
#include "SocketFactory.h"
#include "UtilityFunctions.h"
#include "CommonDefs.h"
#include <iostream>
#include <stdio.h>
extern "C"{
#include "file_manip.h"
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <set>
}

#include "logging.h"

const char logger[] = "SDKDispatcher";

using namespace std;

SDKDispatcher::SDKDispatcher(Configuration* conf):GenericProcessor(*conf),m_debug(false),m_lastTamperSent(0), m_conf(conf) {
	m_socketFactory = new SocketFactory(*conf);
	m_tamperFilePath = m_conf->getValue("Eyelock.TamperFileName","/home/root/tamper");
	int timeOutms = m_conf->getValue("Eyelock.socketTimeOutMilliSDK", 5000);
	m_timeOut.tv_sec = timeOutms / 1000;
	m_timeOut.tv_usec = (timeOutms % 1000) * 1000;
	m_debug = m_conf->getValue("Eyelock.SDKDispatcherDebug",false);
	m_tamperRepeatTimeSec = m_conf->getValue("Eyelock.TamperRepeatTimeSec",10);
	m_queueFullBehaviour = OVERWRIE_OLD;
}

SDKDispatcher::~SDKDispatcher() {
	if(m_socketFactory){
		delete m_socketFactory;
	}
}
int SDKDispatcher::getQueueSize(Configuration* conf){
	return conf->getValue("Eyelock.SDKDispatcherQSize",5);
}

Copyable *SDKDispatcher::createNewQueueItem(){
	return new SDKCallbackMsg();
}



bool SDKDispatcher::DeleteMatchServer(string server){
	bool ret = false;
	ScopeLock lock(m_matchLock);
	std::set<string>::iterator position =  m_matchDestAddr.find(server);
	if(position != m_matchDestAddr.end())
	{
		m_matchDestAddr.erase(position);
		ret = true;
		int32_t save = Save();
		if(!save)
			EyelockLog(logger, INFO, "SDK callback destination file was successfully saved");
		else
			EyelockLog(logger, ERROR, "Failed to save SDK callback destination file");
	}
	return ret;
}

bool SDKDispatcher::DeleteTamperServer(string server)
{
	bool ret = false;
	ScopeLock lock(m_tamperLock);
	std::set<string>::iterator position = m_tamperDestAddr.find(server);
	if(position != m_tamperDestAddr.end())
	{
		m_tamperDestAddr.erase(position);
		ret = true;
		int32_t save = Save();
		if(!save)
			EyelockLog(logger, INFO, "SDK callback destination file was successfully saved");
		else
			EyelockLog(logger, ERROR, "Failed to save SDK callback destination file");
	}
	return ret;
}

void SDKDispatcher::AppendTamperServer(string address){
	ScopeLock lock(m_tamperLock);
	std::set<string>::iterator it;
	std::pair<std::set<string>::iterator,bool> ret;
	ret = m_tamperDestAddr.insert(address);
	if (ret.second==true)
	{
		int32_t save = Save();
		if(!save)
			EyelockLog(logger, INFO, "SDK callback destination file was successfully saved");
		else
			EyelockLog(logger, ERROR, "Failed to save SDK callback destination file");
	}
	else
	{
		EyelockLog(logger, INFO, "Requested destination exists, no insertion needed");
	}

}
void SDKDispatcher::AppendMatchServer(string address){
	ScopeLock lock(m_matchLock);
	std::set<string>::iterator it;
	std::pair<std::set<string>::iterator,bool> ret;
	ret = m_matchDestAddr.insert(address);
	if (ret.second==true)
	{
		int32_t save = Save();
		if(!save)
			EyelockLog(logger, INFO, "Callback destination file was successfully saved");
		else
			EyelockLog(logger, ERROR, "Failed to save callback destination file");
	}
	else
		EyelockLog(logger, INFO, "Requested destination exists, no insertion needed");
}

void SDKDispatcher::ClearMatchServer(){
	ScopeLock lock(m_matchLock);
	m_matchDestAddr.clear();
}

void SDKDispatcher::ClearTamperServer(){
	ScopeLock lock(m_tamperLock);
	m_tamperDestAddr.clear();
}

set<string> SDKDispatcher::GetTampreAddressList(){
	ScopeLock lock(m_tamperLock);
	return m_tamperDestAddr;
}

set<string> SDKDispatcher::GeMatchAddressList(){
	ScopeLock lock(m_matchLock);
	return m_matchDestAddr;
}


bool SDKDispatcher::SendMsg(HostAddress &add,timeval &sendtimeOut,timeval &conntimeOut,BinMessage& out_msg,bool bblock){
	bool ret = false;

	char *messageStr = (char *)calloc(out_msg.GetSize() + 1, sizeof(char));
	memcpy(messageStr, out_msg.GetBuffer(), out_msg.GetSize());
	EyelockLog(logger, DEBUG, "Sending %s to %s", messageStr, add.GetHostName());
	free(messageStr);

	try{
		SocketClient client=m_socketFactory->createSocketClient("Eyelock.SDKDispatcherSecure");
		client.SetTimeouts(conntimeOut);
		client.Connect(add);
		client.SetTimeouts(sendtimeOut);
		if(bblock){
			client.Send(out_msg);
			ret = true;
		}else{
			client.SendAll(out_msg);
			ret = true;
		}
		EyelockLog(logger, TRACE, "Socket operation completed");
	}
	catch(const char *msg){
		EyelockLog(logger, ERROR, "Failed to send callback: %s", msg);
	}
	catch(exception ex){
		EyelockLog(logger, ERROR, "Failed to send callback: %s", ex.what());
	}
	catch(Exception& ex){
		EyelockLog(logger, ERROR, "Failed to send callback");
		ex.PrintException();
	}
	EyelockLog(logger, TRACE, "Sending message completed");
	return ret;
}

void SDKDispatcher::SendTamperMsg(SDKCallbackMsg& msg){
	for(std::set<string>::iterator it=m_tamperDestAddr.begin(); it!=m_tamperDestAddr.end(); ++it)
	{
		ScopeLock lock(m_tamperLock);
		std::string myset = *it;
		try
		{
			HostAddress haddr(myset.c_str());
			EyelockLog(logger, DEBUG, "Sending tamper message to: %s", myset.c_str());
			BinMessage *pBinMsg = msg.GetBinMsg();
			SendMsg(haddr,m_timeOut,m_timeOut, *pBinMsg);
			delete pBinMsg;
		}
		catch (...)
		{
			EyelockLog(logger, ERROR, "Unable to send callback to %s", myset.c_str());
		}
	}
}

void SDKDispatcher::SendMatchMsg(SDKCallbackMsg& msg){
	for(std::set<string>::iterator it=m_matchDestAddr.begin(); it!=m_matchDestAddr.end(); ++it){
		ScopeLock lock(m_matchLock);
		std::string t = *it;
		EyelockLog(logger, TRACE, "Parsing destination: %s", t.c_str());
		try
		{
			HostAddress haddr(t.c_str());
			EyelockLog(logger, DEBUG, "Sending match message to: %s", t.c_str());
			BinMessage *pBinMsg = msg.GetBinMsg();
			SendMsg(haddr,m_timeOut,m_timeOut,*pBinMsg);
			delete pBinMsg;
		}
		catch (...)
		{
			EyelockLog(logger, ERROR, "Unable to send callback to %s", t.c_str());
		}
	}
}

void SDKDispatcher::SendMatchMsg(string matchInfo){
	SDKCallbackMsg msg(MATCH, matchInfo);
	SendMatchMsg(msg);
}

bool SDKDispatcher::enqueMsg(SDKCallbackMsg& msg)
{
	EyelockLog(logger, TRACE, "Enqueing message of type %d", msg.GetEventType());

	bool ret = GenericProcessor::enqueMsg(msg);
	EyelockLog(logger, TRACE, "Enqueing message result: %d", ret);

	return ret;
}

void SDKDispatcher::process(Copyable *inputMsg) {
	EyelockLog(logger, TRACE, "Processing message");
	SDKCallbackMsg msg;
	msg.CopyFrom(*inputMsg);
	EyelockLog(logger, TRACE, "Message copied");
	if (msg.GetEventType() == TAMPERSET || msg.GetEventType() == TAMPERCLEAR)
	{
		SendTamperMsg(msg);
	}
	else if (msg.GetEventType() == MATCH)
	{
		SendMatchMsg(msg);
	}
	else
	{
		EyelockLog(logger, ERROR, "Unknown message type");
	}
}

void SDKDispatcher::init()
{
	GenericProcessor::init();
	int32_t ret = Load();
	if(!ret)
	    EyelockLog(logger, DEBUG, "Callback destination list loaded successfully");
	else
		EyelockLog(logger, ERROR, "Failed to load callback destination list");
}

int32_t SDKDispatcher::Save()
{
	int32_t ret = 1;
	std::string path;
#ifdef __ARM__
	path = m_conf->getValue("Eyelock.SDKRegisterIPs","/home/root/SDKRegisterIPs.txt");
#else
	path = m_conf->getValue("Eyelock.SDKRegisterIPs","/home/developer/SDKRegisterIPs.txt");
#endif
	std::set<string>::iterator it;
	std::set<string>::iterator itm;
	remove(path.c_str());
	FILE *fp = fopen(path.c_str(),"w");
	if(fp)
	{
		for (std::set<string>::iterator it=m_matchDestAddr.begin(); it!=m_matchDestAddr.end(); ++it){
			std::string mset("MATCHES$");
			mset.append(*it);
			mset.append("\n");
			EyelockLog(logger, DEBUG, "Saving MATCH callback destination: %s", mset.c_str());
			fwrite(mset.c_str(),1,mset.size(),fp);
			ret = 0;
		}
		for (std::set<string>::iterator itm=m_tamperDestAddr.begin(); itm!=m_tamperDestAddr.end(); ++itm){
			std::string tset("TAMPERS$");
			tset.append(*itm);
			tset.append("\n");
			EyelockLog(logger, DEBUG, "Saving TAMPER callback destination: %s", tset.c_str());
			fwrite(tset.c_str(),1,tset.size(),fp);
			ret = 0;
		}
		fclose(fp);
	}
	return ret;
}

int32_t SDKDispatcher::Load()
{
	int32_t ret = 1;
	std::string path;

#ifdef __ARM__
	path = m_conf->getValue("Eyelock.SDKRegisterIPs","/home/root/SDKRegisterIPs.txt");
#else
	path = m_conf->getValue("Eyelock.SDKRegisterIPs","/home/developer/SDKRegisterIPs.txt");
#endif
	FILE *fp = fopen(path.c_str(),"r");
	m_tamperDestAddr.clear();
	m_matchDestAddr.clear();
	if(fp != NULL)
	{
		while(1)
		{
			char buffer[100]={0};
			char* p = fgets(buffer,100,fp);
			if(p == NULL) break;
			string str = buffer;
			int32_t sz = str.size();
			//cout<<"string is :"<<str<<endl;
			//cout<<"size of string is :"<<sz<<endl;
			int num = sz;
			if(strncmp(str.c_str(),"MATCHES$",8) == 0)
			{
				--num;
				num = num -8;
				std::string addr = str.substr(8, num);
				EyelockLog(logger, DEBUG, "Adding MATCH callback destination: %s", addr.c_str());
				m_matchDestAddr.insert(addr);
				ret =0;
			}
			else if(strncmp(str.c_str(),"TAMPERS$",8)==0)
			{
				--num;
				num = num -8;
				std::string addr = str.substr(8, num);
				EyelockLog(logger, DEBUG, "Adding TAMPER callback destination: %s", addr.c_str());
				m_tamperDestAddr.insert(addr);
				ret =0;
			}
			else
				EyelockLog(logger, ERROR, "Error parsing SDK callback destination: %s", str.c_str());
		}
		fclose(fp);
	}
	return ret;
}

SDKCallbackMsg::SDKCallbackMsg(SDKCallbackType eventType, string info): m_eventType(eventType), m_msg("")
{
	EyelockLog(logger, TRACE, "Creating callback message of type: %d, with info: %s", (int)eventType, info.c_str());
	switch (m_eventType)
	{
		case TAMPERSET:
		{
			m_msg = "TAMPERED;";
			m_msg.append(GetDeviceTypeAndId());
			break;
		}
		case TAMPERCLEAR:
		{
			m_msg = "TAMPERCLEAR;";
			m_msg.append(GetDeviceTypeAndId());
			break;
		}
		case MATCH:
		{
			m_msg = GetDeviceTypeAndId();
			m_msg.append(info);
			break;
		}
		default:
		{
			EyelockLog(logger, ERROR, "Unsupported callback message type is specified: %d", (int)eventType);
			break;
		}
	}
};

string SDKCallbackMsg::GetDeviceTypeAndId()
{
	string deviceTypeStr = "DEVICETYPE:1;";

	char myID[10];

    strcpy(myID, "0000");
#ifndef HBOX_PG
	FILE *fp = fopen("/home/root/id.txt", "r");
#else	
    FILE *fp = fopen("id.txt", "r");
#endif	
    if (fp != NULL)
    {
 	   if (fgets(myID, 10, fp) != NULL)
 	   {
 		   myID[strlen(myID)-1] = '\0';
 	   }
 	   fclose(fp);
    }
    else
    {
  	   EyelockLog(logger, ERROR, "Unable to open device ID file");
    }
    string deviceIdStr = "DEVICEID:" + string(myID) + ";";
    return deviceTypeStr + deviceIdStr;
}
