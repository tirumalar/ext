#include "EyelockNanoDevice_server.h"
#include "FileConfiguration.h"
#include "EyelockConfiguration.h"
#include "AudioDispatcher.h"
#include <thrift/protocol/TBinaryProtocol.h>
#include <thrift/transport/TSSLSocket.h>
#include <thrift/server/TSimpleServer.h>
#include <thrift/transport/TServerSocket.h>
#include <thrift/transport/TBufferTransports.h>
#include "EyeLockThread.h"
#include "LEDDispatcher.h"
#include "HTTPPOSTMsg.h"
#include "UtilityFunctions.h"
#include "OpenSSLSupport.h"
#include "DBAdapter.h"
#include <FileChunker.h>
#include "SDKDispatcher.h"
#include "EyelockClientAccessManager.h"
#include "UpgradeFirmwareHelper.h"
#include <pthread.h>
#include <string>
#include <cctype>
#include <stdlib.h>
#include <sys/time.h>
#include <stdio.h>
#include <algorithm>
#include <sys/time.h>
#include <stdio.h>
#include <fstream>
#include <istream>
#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>
#include <ctime>
#include <dirent.h>
#include <unistd.h>
#include <boost/algorithm/string.hpp>
#include <openssl/md5.h>
#include <regex.h>
#include <shadow.h>
#include "logging.h"
#include <limits.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/stat.h> // open function for accessing /dev/urandom
#include <fcntl.h> // open mode for accessing /dev/urandom

#include <sys/utsname.h> // uname in InitPasswordStorage

#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>

extern "C"
{
	#include "BobListener.h"
}

const char logger[] = "EyelockNanoDeviceHandler";

using namespace std;
using namespace apache::thrift;
using namespace apache::thrift::protocol;
using namespace apache::thrift::transport;
using namespace EyelockNano;
using namespace EyelockConfigurationNS;
using boost::shared_ptr;
using namespace boost::archive::iterators;

using namespace ::EyelockNano;
struct to_upper {
  int operator() ( int ch )
  {
    return std::toupper ( ch );
  }
};
EyelockNanoDeviceHandler::EyelockNanoDeviceHandler(Configuration * conf) :m_conf(conf),m_eyelockConf("Eyelock.ini"),m_DBMutexEnable(false),
		m_pEyeLockThread(NULL), m_nNano_Mode(-1), m_sStream_IPAddress(""), m_sStream_PortNo(""),
		m_sVersion(""),pledConsolidator(NULL),m_pRGBController(NULL), m_volume(-1), m_pAudioDispatcher(NULL),m_pSDKDispatcher(NULL),m_pSlave(NULL),m_bMaster(false),m_bSlavePresent(true) {

	m_DBMutexEnable = conf->getValue("Eyelock.SDKDBMutexEnable", true);
	m_bSecurelistener = (bool)conf->getValue("Eyelock.SecureThrift", false);
	m_bMaster = (strcmp(m_conf->getValue("GRI.masterMode","2"),"1") == 0) ? true : false;

	//TODO: PC-side sends database files for push/update to this directory (hardcode). It needs to be defined in one place for both parties.
	m_dataDir = "/home/root/data";

	m_passStorageDefault = "/etc/shadow";
	m_passStorageCustom = "/home/www-internal/shadow";
	m_passStorage = m_passStorageDefault;
	m_unameRoFs = "3.0.0-BSP-dm37x-2.4-2Eyelock_NXT_6.0";
	m_bIsRoFs = false;

	m_bSlavePresent = conf->getValue("Eyelock.USBSlaveEnabled", true);
	if(m_bMaster)
	{
		if (m_bSlavePresent)
		{
			ThriftcallforMasterSlave();
		}
	}
}

void EyelockNanoDeviceHandler::SetEyelockThread(EyeLockThread *ptr) {
	m_pEyeLockThread = ptr;
}

int32_t EyelockNanoDeviceHandler::startImageStream(const std::string& ipaddress,
		const std::string& portno, const bool secure, const ELKNS_ImageFormats::type format) {
	// Store the current mode of the nano device
	EyelockLog(logger, INFO, "Starting image stream to: %s:%s (secure: %d), format = %d", ipaddress.c_str(), portno.c_str(), secure, format);
	int32_t ret = FLOW_ERR;
	if (m_pEyeLockThread) {
		m_nNano_Mode = m_pEyeLockThread->getMode();
		m_nFrameType = m_pEyeLockThread->getFrameType();
		m_bDetectEyes = m_pEyeLockThread->getShouldDetectEyes();

		if ((!ipaddress.empty()) && (!portno.empty()))
		{
			string host(ipaddress);
			host.append(":");
			host.append(portno);
			m_sStream_IPAddress = ipaddress;
			m_sStream_PortNo = portno;
			switch (format)
			{
			case 0:
				m_pEyeLockThread->setShouldDetectEyes(false);
				m_pEyeLockThread->setFrameType (0);
				break;
			case 1:
				m_pEyeLockThread->setShouldDetectEyes(false);
				m_pEyeLockThread->setFrameType (2);
				break;
			case 2:
				m_pEyeLockThread->setShouldDetectEyes(true);
				break;
			}
			m_pEyeLockThread->GetEyelockModeMessageHandler()->SetEyeLockMode(
					host, EyelockCamera);

			m_pEyeLockThread->setSecureComm(secure);
			ret = SUCCESS;
		}
	}
	return ret;
}

int32_t EyelockNanoDeviceHandler::stopImageStream(const std::string& ipaddress,
		const std::string& portno) {
	EyelockLog(logger, INFO, "Stopping image stream to: %s:%s", ipaddress.c_str(), portno.c_str());
	int32_t ret = INPUT_ERR;
	if (m_pEyeLockThread) {
		int nano_Mode = m_pEyeLockThread->getMode();
		if (m_nNano_Mode < 0) {
			ret = FLOW_ERR;
			return ret;
		}
		if ((ipaddress.empty()) || (portno.empty())
				|| ((m_sStream_IPAddress.empty()) || (m_sStream_PortNo.empty()))) {
			return ret;
		}

		if ((m_sStream_IPAddress.find(ipaddress) < 0)
				|| (m_sStream_PortNo.find(portno) < 0)) {
			return ret;
		}
		// Reset the mode back to original state
		m_pEyeLockThread->setShouldDetectEyes(m_bDetectEyes);
		m_pEyeLockThread->setFrameType (m_nFrameType);
//		m_pEyeLockThread->setMode(m_nNano_Mode);
		string host;
		m_pEyeLockThread->GetEyelockModeMessageHandler()->SetEyeLockMode(host,EyelockMatcher);
		m_pEyeLockThread->resetSecureComm();
		ret = SUCCESS;
	}
	return ret;
}

void EyelockNanoDeviceHandler::SetEyelockFirmwareVersion(
		std::string firmwareRevision) {
	//m_sVersion = firmwareRevision;
	m_sVersion = "5.05.1905";
}

std::string EyelockNanoDeviceHandler::GetIcmVersion()
{
	const char* filepath = m_conf->getValue("EYELOCK.ICMVersionFilepath","/home/root/BobVersion");
	FILE *fp = fopen(filepath,"r");
	std::string Version;
	char buffer[100]={0};
	if(fp != NULL)
	{
		fgets(buffer,99,fp);
		fclose(fp);
		Version.insert(0, buffer);
		std::string ICM ("ICM software version:");
		int32_t pos = Version.find(ICM);
		if (pos > -1)
		{
			Version.erase (0, pos+ICM.length()+1);
			Version.erase(Version.find_last_not_of(" \n\r\t")+1);
		}
		else
		{
			Version.clear();
		}
		return Version;
	}
}

void EyelockNanoDeviceHandler::GetSlaveVersion(std::string& _return)
{
	if(!m_sVersion.empty())
		_return = m_sVersion;
}

void EyelockNanoDeviceHandler::GetFirmwareRevision(std::map<std::string, std::string> & mymap, const int32_t reType)
{
	switch (reType)
	{
		case 0:
		{
			if (!m_sVersion.empty())
			{
				mymap.insert(std::pair<std::string,std::string> ("Firmware_Revision", m_sVersion));
			}
			else
			{
				mymap.insert(std::pair<std::string,std::string> ("RET_VALUE", "ERR_VERSION"));
				break;
			}
#ifdef __ARM__
			if(m_bMaster)
			{

				if (m_bSlavePresent)
				{
					int slaveConnectionStatus = slaveConnectionValidation();
					if (slaveConnectionStatus != SUCCESS)
					{
						EyelockLog(logger, ERROR, "Slave connection validation failed: %d", slaveConnectionStatus);
						mymap.insert(std::pair<std::string,std::string> ("RET_VALUE", "SLAVE_UNSUCCESSFUL"));
					}
				}

				string SlaveVersion;
				if (m_pSlave != NULL)
				{
					m_pSlave->GetSlaveVersion(SlaveVersion);
				}
				else
				{
					SlaveVersion = "0.0.0";
				}

				if (!SlaveVersion.empty())
				{
					mymap.insert(std::pair<std::string,std::string> ("Slave_Firmware_Revision", SlaveVersion));
				}
				else
				{
					mymap.insert(std::pair<std::string,std::string> ("RET_VALUE", "SLAVE_UNSUCCESSFUL"));
					break;
				}

				mymap.insert(std::pair<std::string,std::string> ("RET_VALUE", "SUCCESS"));
			}
#endif
		}

		break;

		case 2:
		{
			std::string Icm_Revision = GetIcmVersion();
			if(!Icm_Revision.empty())
			{
				mymap.insert(std::pair<std::string,std::string> ("RET_VALUE", "SUCCESS"));
				mymap.insert(std::pair<std::string,std::string>("Icm_Revision", Icm_Revision));
			}
			else
			{
				mymap.insert(std::pair<std::string,std::string> ("RET_VALUE", "ERR_VERSION"));
			}
		}
		break;

		case 3:
		{
			if (!m_sVersion.empty())
			{
				mymap.insert(std::pair<std::string,std::string> ("Firmware_Revision", m_sVersion));
			}
			else
			{
				mymap.insert(std::pair<std::string,std::string> ("RET_VALUE", "ERR_VERSION"));
				break;
			}
#ifdef __ARM__
			if(m_bMaster)
			{

				if (m_bSlavePresent)
				{
					int slaveConnectionStatus = slaveConnectionValidation();
					if (slaveConnectionStatus != SUCCESS)
					{
						EyelockLog(logger, ERROR, "Slave connection validation failed: %d", slaveConnectionStatus);
						mymap.insert(std::pair<std::string,std::string> ("RET_VALUE", "SLAVE_UNSUCCESSFUL"));
					}
				}

				string SlaveVersion;
				if (m_pSlave != NULL)
				{
					m_pSlave->GetSlaveVersion(SlaveVersion);
				}
				else
				{
					SlaveVersion = "0.0.0";
				}

				if (!SlaveVersion.empty())
				{
					mymap.insert(std::pair<std::string,std::string> ("Slave_Firmware_Revision", SlaveVersion));
				}
				else
				{
					mymap.insert(std::pair<std::string,std::string> ("RET_VALUE", "SLAVE_UNSUCCESSFUL"));
					break;
				}
			}
#endif
			std::string Icm_Revision = GetIcmVersion();
			if(!Icm_Revision.empty())
			{
				mymap.insert(std::pair<std::string,std::string> ("RET_VALUE", "SUCCESS"));
				mymap.insert(std::pair<std::string,std::string>("Icm_Revision", Icm_Revision));
			}
			else
			{
				mymap.insert(std::pair<std::string,std::string> ("RET_VALUE", "ERR_VERSION"));
			}
		}

		break;

		default:
			mymap.insert(std::pair<std::string,std::string> ("RET_VALUE","INVALID_INPUT"));
	}
}

void EyelockNanoDeviceHandler::SetRGBcontrollerNano(RGBController *ptr) {
	m_pRGBController = ptr;
}
void EyelockNanoDeviceHandler::SetConsolidator(LEDConsolidator *ptr) {
	pledConsolidator = ptr;
}
void EyelockNanoDeviceHandler::SetSDKDispatcher(SDKDispatcher  *ptr){
	m_pSDKDispatcher = ptr;
}
int32_t EyelockNanoDeviceHandler:: ChangeLedColor(const int8_t mask,const int32_t t) {

	int colorcode = (int) mask;
	int m_val;
	switch (colorcode) {
	case 0:
		m_val = 0;
		break;
	case 1:
		m_val = 1;
		break;
	case 2:
		m_val = 4;
		break;
	case 3:
		m_val = 5;
		break;
	case 4:
		m_val = 16;
		break;
	case 5:
		m_val = 17;
		break;
	case 6:
		m_val = 20;
		break;
	case 7:
		m_val = 21;
		break;

	}
	LEDResult l;
	l.setState(LED_NWSET);
	l.setGeneratedState(eREMOTEGEN);
	l.setNwValandSleep(m_val, t);
	if (pledConsolidator) {
		pledConsolidator->enqueMsg(l);
	}
	return 0;
}
void EyelockNanoDeviceHandler::SetAudioDispatcher(AudioDispatcher *ptr) {
	m_pAudioDispatcher = ptr;
}

int32_t EyelockNanoDeviceHandler::SetAudiolevel(const double volumelevel) {
	float lv = (float) volumelevel;
	int32_t ret = UNSUCCESSFUL;
	if(m_pAudioDispatcher)
		ret = m_pAudioDispatcher->SetAudiolevel(lv);
	else
		EyelockLog(logger, ERROR, "Setting audio level failed: m_pAudioDispatcher is NULL");
	return ret;
}

double EyelockNanoDeviceHandler::GetAudiolevel(void) {
	double vol;
	if(m_pAudioDispatcher)
		vol = m_pAudioDispatcher->GetAudiolevel();
	else
		EyelockLog(logger, ERROR, "Getting audio level failed: m_pAudioDispatcher is NULL");
	return vol;
}

int32_t EyelockNanoDeviceHandler::IsDeviceTampered(void) {
	int32_t ret = DEVICE_WAS_NOT_TAMPERED;
	const char*fpath = m_conf->getValue("Eyelock.TamperFileName",
			"/home/root/tamper");
	bool fexist = FileExists(fpath);
	if (fexist) {
		ret = DEVICE_WAS_TAMPERED;
	}
	return ret;
}

void EyelockNanoDeviceHandler::setNwMatchManager(
		NwMatchManager* _nwMatchManager) {
	m_pNwMatchManager = _nwMatchManager;
}

int32_t EyelockNanoDeviceHandler::ACD_DataTypeValidation(const ACD_Type::type dbtype){
	bool F2F = m_conf->getValue("GRITrigger.F2FEnable",false);
	bool Wig = m_conf->getValue("GRITrigger.WeigandEnable",false);
	bool pac = m_conf->getValue("GRITrigger.PACEnable",false);
	bool relay = m_conf->getValue("GRITrigger.RelayEnable", false);

	switch(dbtype){
	case ACD_Type::RELAY:
		if(!relay)
			return WRONG_ACD_DATA_TYPE;
		break;
	case ACD_Type::F2F:
		if(!F2F)
			return WRONG_ACD_DATA_TYPE;
		break;

	case ACD_Type::PAC:
		if(!pac)
			return WRONG_ACD_DATA_TYPE;
		break;

	case ACD_Type::WIEGAND:
		if(!Wig)
			return WRONG_ACD_DATA_TYPE;
		break;
	default:
		break;
	}
	return SUCCESS;
}

int32_t EyelockNanoDeviceHandler::pushDB(const std::string& fullDB, const ACD_Type::type dbtype) {
	EyelockLog(logger, INFO, "Pushing database %s", fullDB.c_str());

	if (!m_pNwMatchManager)	return UNSUCCESSFUL;
	//int32_t t = ACD_DataTypeValidation(dbtype);
	//if(t != SUCCESS) return t;

	sync();

	char* filename = (char*)fullDB.c_str();

	bool updateDbRet, enqueMsgRet;
	updateDbRet = m_pNwMatchManager->GetMM()->UpdateDB((DBMsgType) eREPLACEDB, filename);

	if (updateDbRet)
	{
		EyelockLog(logger, INFO, "Enqueuing RELOADDB message");
		HTTPPOSTMsg passnext(32);
		sprintf(passnext.GetBuffer(), "RELOADDB;%d;0;0;0;", eRELOADDB);
		enqueMsgRet = m_pNwMatchManager->enqueMsg(passnext);
		EyelockLog(logger, INFO, "Enqueuing RELOADDB message returned %d", enqueMsgRet);
	}

	remove(filename);
	sync();

	if (updateDbRet && enqueMsgRet)
	{
		return SUCCESS;
	}
	else
	{
		return UNSUCCESSFUL;
	}
}

int32_t EyelockNanoDeviceHandler::updateDB(const std::string& upDB, const ACD_Type::type dbtype) {

	EyelockLog(logger, INFO, "Updating database %s", upDB.c_str());
	if(!m_pNwMatchManager) return UNSUCCESSFUL;

	/*int32_t t = ACD_DataTypeValidation(dbtype);
	if(t != SUCCESS) return t;*/
	HTTPPOSTMsg dbmsg(128);
	int sz = sprintf(dbmsg.GetBuffer(),"RECEIVEUSR;%d;%s;",eUPDATEDB,upDB.c_str());
	dbmsg.SetSize(sz);
	bool ret = false;
	if(m_DBMutexEnable){
		m_pNwMatchManager->ProcessReceiveUsrFromSDK(&dbmsg);
		ret = true;
	}else{
		ret = m_pNwMatchManager->enqueMsg(dbmsg);
	}

	if(ret == false) return UNSUCCESSFUL;
	return SUCCESS;
}

void* ProcessforRestartFirmware(void * data) {
	long int tid = syscall(SYS_gettid);
	EyelockLog(logger, INFO, "ProcessforRestartFirmware: thread %ld", tid);
	std::string filename = std::string((char*) data);
	EyelockLog(logger, INFO, "Running %s", filename.c_str());

	std::string file_cmd;
	sleep(10);
	file_cmd.append("sh ");
	file_cmd.append(filename);
	RunCmd((char*) file_cmd.c_str());
#ifndef __ARM__
	RunCmd("touch donefwreset");
#endif
	return NULL;
}
int32_t EyelockNanoDeviceHandler::ResetFirmware() {

	int status = FILE_DOES_NOT_EXIST;

	pthread_t threadx;
	int iret1;
	const char * filename = m_conf->getValue("GRI.FactoryReset","/home/root/scripts/factoryReset.sh");
	FILE *ptr = NULL;
	ptr = fopen(filename, "r");
	if (ptr != NULL) {
		iret1 = pthread_create(&threadx, NULL, ProcessforRestartFirmware,
				(void*) filename);

		status = SUCCESS;
	}
	if (ptr)
		fclose(ptr);

	return status;
}

int32_t EyelockNanoDeviceHandler::PingDevice()
{
	if (m_bSlavePresent)
	{
		int slaveConnectionStatus = slaveConnectionValidation();
		if (slaveConnectionStatus != SUCCESS)
		{
			EyelockLog(logger, ERROR, "Slave connection validation failed: %d", slaveConnectionStatus);
			return slaveConnectionStatus;
		}
	}

	int32_t status = SUCCESS;
	if(m_bMaster)
	{
		EyelockLog(logger, DEBUG, "PingDevice on Master");
		if(m_pSlave != NULL)
		{
			status =m_pSlave->PingDevice();
			status =  getSlaveStatus(status);
		}
	}
	else
	{
		EyelockLog(logger, DEBUG, "PingDevice on Slave");
	}
	return status;
}

int64_t EyelockNanoDeviceHandler::GetTime() {
	return (int64_t)GetEpochMilliseconds();
}

int32_t EyelockNanoDeviceHandler::SyncTime(const int64_t nanoTime, const int64_t hostTime, const int32_t pingTimeout) {
	int64_t current = GetEpochMilliseconds();
	int64_t delta = current - nanoTime;
	if (delta < 0 || delta > (int64_t) pingTimeout)
	{
		EyelockLog(logger, ERROR, "Time synchronization was aborted due to a ping timeout");
	}
	else
	{
		int64_t approxTime = hostTime + delta / 2;
		int64_t seconds = approxTime / 1000;
		int64_t milliSeconds = approxTime % 1000;

		struct timeval tv;
		struct timezone tz;
		EyelockLog(logger, INFO, "Time synchronization: nano time 1: %ld | nano time 2: %ld | host time: %ld", nanoTime, current, hostTime);
		tv.tv_sec = seconds;
		tv.tv_usec = milliSeconds * 1000;
		int result = settimeofday(&tv, &tz);
		if (result == 0)
		{
			RunSystemCmd("/sbin/hwclock -w");
			int64_t updatedTime = GetEpochMilliseconds();
			EyelockLog(logger, INFO, "Time was updated, current timestamp is %ld", updatedTime);
			return SUCCESS;
		}
	}
	return UNSUCCESSFUL;
}

void* ProcessforRestartDevice(void *data)
{
	long int tid = syscall(SYS_gettid);
	EyelockLog(logger, DEBUG, "ProcessforRestartDevice: thread %ld", tid);

	ELKNS_RestartTypes::type *restart = (ELKNS_RestartTypes::type*) data;
	if (*restart == 0)
	{
		EyelockLog(logger, INFO, "Rebooting device");
		string script = "/home/root/fwHandler.sh";
		stringstream ss;
		ss << "chmod 777 " << script;
		ss << ";" << script << " reboot";
		RunCmd(ss.str().c_str()); ss.str("");
	}
	else if (*restart == 1)
	{
		EyelockLog(logger, INFO, "Restarting Eyelock process");
		RunCmd("killall -KILL Eyelock");
	}
#ifndef __ARM__
	RunCmd("touch donereset"); // is used in UT_NanoDeviceHandler.cpp
#endif
	return NULL;
}

int32_t EyelockNanoDeviceHandler::RestartDevice(const ELKNS_RestartTypes::type restart)
{
	if (m_bSlavePresent)
	{
		int slaveConnectionStatus = slaveConnectionValidation();
		if (slaveConnectionStatus != SUCCESS)
		{
			EyelockLog(logger, ERROR, "Slave connection validation failed: %d", slaveConnectionStatus);
			return slaveConnectionStatus;
		}
	}

	int32_t status = SUCCESS;
    if (m_bMaster)
	{
		if (restart == ELKNS_RestartTypes::REBOOT_EYELOCK) // killing Eyelock process on slave. Reboot device is performed by master board itself.
		{
			if (m_pSlave != NULL)
			{
				status = getSlaveStatus(m_pSlave->RestartDevice(restart));
			}
		}
		else if (restart == ELKNS_RestartTypes::REBOOT_DEVICE)
		{
			string script = "/home/root/fwHandler.sh";
			ifstream scriptFile(script.c_str());
			if (scriptFile.good())
			{
				EyelockLog(logger, DEBUG, "Reboot script check passed");
			}
			else
			{
				EyelockLog(logger, ERROR, "Unable to open reboot script file");
				status = UNSUCCESSFUL;
			}
			scriptFile.close();
		}
	}
    if (status != SUCCESS)
    {
    	return status;
    }

    m_restartType = restart;
	pthread_t threadx;
	int iret1 = pthread_create(&threadx, NULL, ProcessforRestartDevice, (void*)&m_restartType);
	EyelockLog(logger, INFO, "Creating thread for restart status: %d", iret1);

	return status;
}

void EyelockNanoDeviceHandler::GetConfigParameters(map<int32_t, string> & outParametersMap) {
	EyelockLog(logger, INFO, "Requested configuration parameters");

	m_eyelockConf.getAllParameters(outParametersMap);
}

int32_t EyelockNanoDeviceHandler::SetConfigParameters(const map<int32_t, string> & inputParametersMap) {
	EyelockLog(logger, INFO, "Setting configuration parameters");

	return m_eyelockConf.setParametersMap(inputParametersMap);
}

int32_t EyelockNanoDeviceHandler::SetIntParameter(const string& paramName, const int64_t value) {
	EyelockLog(logger, TRACE, "Setting int configuration parameter %s to %d", paramName.c_str(), value);
	return NOT_IMPLEMENTED;
}


void EyelockNanoDeviceHandler::GetIntParameter(GetIntReturn& _return, const string& paramName) {
	EyelockLog(logger, TRACE, "Getting int configuration parameter %s", paramName.c_str());
	_return.status = NOT_IMPLEMENTED;
	_return.value = 0;
}

int32_t EyelockNanoDeviceHandler::SetUIntParameter(const string& paramName, const int64_t value) {
	EyelockLog(logger, TRACE, "Setting uint configuration parameter %s to %l", paramName.c_str(), value);

	ELKNS_Status_T setStat = (ELKNS_Status_T) m_eyelockConf.setUIntParameter(paramName, (unsigned int) value);
	if (setStat != SUCCESS)
	{
		return setStat;
	}

	if (!m_eyelockConf.save())
	{
		return FILE_ACCESS_ERROR;
	}

	return SUCCESS;
}

void EyelockNanoDeviceHandler::GetUIntParameter(GetIntReturn& _return, const string& paramName) {
	EyelockLog(logger, TRACE, "Getting uint configuration parameter %s", paramName.c_str());
	unsigned int value = 0;
	_return.status = m_eyelockConf.getUIntParameter(paramName, value);
	_return.value = (int64_t) value;
}

int32_t EyelockNanoDeviceHandler::SetDoubleParameter(const string& paramName, const double value) {
	EyelockLog(logger, TRACE, "Setting float configuration parameter %s to %f", paramName.c_str(), value);

	ELKNS_Status_T setStat = (ELKNS_Status_T) m_eyelockConf.setFloatParameter(paramName, (float) value);
	if (setStat != SUCCESS)
	{
		return setStat;
	}

	if (!m_eyelockConf.save())
	{
		return FILE_ACCESS_ERROR;
	}

	return SUCCESS;
}

void EyelockNanoDeviceHandler::GetDoubleParameter(GetDoubleReturn& _return, const string& paramName) {
	EyelockLog(logger, TRACE, "Getting float parameter %s", paramName.c_str());
	float value = 0.0;
	_return.status = m_eyelockConf.getFloatParameter(paramName, value);
	_return.value = (double) value;
}

int32_t EyelockNanoDeviceHandler::SetBoolParameter(const string& paramName, const bool value) {
	EyelockLog(logger, TRACE, "Setting bool configuration parameter %s to %d", paramName.c_str(), value);

	ELKNS_Status_T setStat = (ELKNS_Status_T) m_eyelockConf.setBoolParameter(paramName, value);
	if (setStat != SUCCESS)
	{
		return setStat;
	}

	if (!m_eyelockConf.save())
	{
		return FILE_ACCESS_ERROR;
	}

	return SUCCESS;
}

void EyelockNanoDeviceHandler::GetBoolParameter(GetBoolReturn& _return, const string& paramName) {
	EyelockLog(logger, TRACE, "Getting bool configuration parameter %s", paramName.c_str());
	bool value = false;
	_return.status = m_eyelockConf.getBoolParameter(paramName, value);
	_return.value = value;
}

int32_t EyelockNanoDeviceHandler::SetStrParameter(const string& paramName, const string& value) {
	EyelockLog(logger, TRACE, "Setting string configuration parameter %s to %s", paramName.c_str(), value.c_str());

	ELKNS_Status_T setStat = (ELKNS_Status_T) m_eyelockConf.setStrParameter(paramName, value);
	if (setStat != SUCCESS)
	{
		return setStat;
	}

	if (!m_eyelockConf.save())
	{
		return FILE_ACCESS_ERROR;
	}

	return SUCCESS;
}

void EyelockNanoDeviceHandler::GetStrParameter(GetStrReturn& _return, const string& paramName) {
	EyelockLog(logger, TRACE, "Getting string configuration parameter %s", paramName.c_str());
	string value;
	_return.status = m_eyelockConf.getStrParameter(paramName, value);
	_return.value = value;
}

int32_t EyelockNanoDeviceHandler::ResetConfigParameters() {
	EyelockLog(logger, TRACE, "Resetting configuration parameters");
	return NOT_IMPLEMENTED;
}


void EyelockNanoDeviceHandler::RetreiveAllIDs(std::string& _return)
{
	EyelockLog(logger, INFO, "Requested record IDs");

	_return.clear();
	if (m_pNwMatchManager != NULL)
	{
		MatchManagerInterface* matchManager = m_pNwMatchManager->GetMM();
		if (matchManager != NULL)
		{
			DBAdapter* pDbAdapter = matchManager->GetDbAdapter();
			if (pDbAdapter != NULL)
			{
				// get records count excluding dummies (names content "emptyxxx")
				std::vector<std::string> outStringVec;
				int ret = pDbAdapter->getAllUserIDs(outStringVec);
				if(ret != 0)
				{
					EyelockLog(logger, ERROR, "Cannot get all user IDs from DBAdapter");
					return;
				}
				EyelockLog(logger, DEBUG, "Records IDs retrieved");

				_return.reserve(37*outStringVec.size()); // 36 - GUID length + semicolon
				for(std::vector<std::string>::iterator itr = outStringVec.begin();itr != outStringVec.end();itr++)
				{
					_return.append(*itr);
					_return.append(";");
				}
				EyelockLog(logger, DEBUG, "String with IDs composed");
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
	}
	else
	{
		EyelockLog(logger, ERROR, "Cannot access NwMatchManager");
	}
}

void EyelockNanoDeviceHandler::RetrieveLogs(std::map<std::string, std::string> & logMap)
{
	EyelockLog(logger, INFO, "Requested logs");
	if (m_bSlavePresent)
	{
		int slaveConnectionStatus = slaveConnectionValidation();
		if (slaveConnectionStatus != SUCCESS)
		{
			EyelockLog(logger, ERROR, "Slave connection validation failed: %d", slaveConnectionStatus);
			logMap.insert(std::pair<std::string,std::string> ("RET_VALUE", "SLAVE_UNSUCCESSFUL"));
		}
	}

#ifndef __ARM__
   const char * Compressfpath = m_conf->getValue("GRI.CompressLogstest","/home/developer/ws_nanoNxt/Eyelock/data/Scripts/compressLogs.sh");
   const char * ZipallCompressfpath = m_conf->getValue("GRI.ZipallCompressLogstest","/home/developer/ws_nanoNxt/Eyelock/data/Scripts/ZipAllCompressedLogs.sh");
   const char * Logsfpath = m_conf->getValue("GRI.EyelockLogstest","/home/developer/ws_nanoNxt/Eyelock");
   const char * Archievesfpath = m_conf->getValue("GRI.EyelockArchievestest","/home/developer/ws_nanoNxt/Eyelock/txtFile");
#else
   const char * Compressfpath = m_conf->getValue("GRI.CompressLogs","/home/root/scripts/compressLogs.sh");
   const char * ZipallCompressfpath = m_conf->getValue("GRI.ZipallCompressLogs","/home/root/scripts/ZipAllCompressedLogs.sh");
   const char * Logsfpath = m_conf->getValue("GRI.EyelockLogs","/home/root");
   const char * Archievesfpath = m_conf->getValue("GRI.EyelockArchieves","/home/root/txtFile");
#endif
   std::map<std::string, std::string> slavePath;
   std::map<std::string, std::string>::iterator it;
   int32_t status;
   std::string outSlavePath;
   std::string	slavefpath ;

   if(m_bMaster)
   {
	   if(m_pSlave != NULL)
	   {
     	  m_pSlave->RetrieveLogs(slavePath);
     	  try
     	  {
     	 	 it = slavePath.find("File_Path");
     		 if (it != slavePath.end())
     		 {
     			 slavefpath = slavePath.find("File_Path")->second;
     		      outSlavePath.append(Archievesfpath);
     	          outSlavePath.append("/slaveArchieve.tar.gz");
     		 }
     	  }
     	  catch (TException& tx)
     	  {
     	 	  EyelockLog(logger, ERROR, "Error retrieving logs from slave board: %s", tx.what());
    	  }
	   }
   }

   std::string file_cmd;
   std::string Filename="";
   file_cmd.append("sh ");
   file_cmd.append(Compressfpath);
   std::string argsM= " ";
   argsM.append(Logsfpath);
   file_cmd.append(argsM);
   RunCmd((char*)file_cmd.c_str());
   std::string fileSpath;
   fileSpath.append(Archievesfpath);
   fileSpath.append("/masterArchieve.tar.gz");
   if(!m_bMaster)
	   logMap.insert(std::pair<std::string,std::string> ("File_Path", fileSpath));

   //compress master and slave ziped file
   if(m_bMaster)
   {
	   if(m_pSlave != NULL){
		   status = FileChunker::receiveChunksFromDevice(slavefpath,outSlavePath,m_pSlave,512000);
		   std::string pathtoDelete;
		   pathtoDelete.append(Archievesfpath);
		   m_pSlave->DeleteDeviceFile(pathtoDelete,true);
	   }

	   std::string file_cmd1;
	   file_cmd1.append("sh ");
	   file_cmd1.append(ZipallCompressfpath);
	   std::string args= " ";
	   args.append(Archievesfpath);
	   file_cmd1.append(args);
	   RunCmd((char*)file_cmd1.c_str());
	   std::string combfilepath;
	   combfilepath.append(Archievesfpath);
	   combfilepath.append("/combinearchieve.tar.gz");
	   bool fexist = FileExists(ZipallCompressfpath);
	   if (!fexist)
	   {
			logMap.insert(std::pair<std::string,std::string> ("RET_VALUE", "FILE_DOES_NOT_EXIST"));
	   }
	   else
	   {
		   logMap.insert(std::pair<std::string,std::string> ("File_Path", combfilepath));
	   }
	   if(status==0)
	   {
		   logMap.insert(std::pair<std::string,std::string> ("SlaveRET_VALUE", "SUCCESS"));
	   }
	   else if(status==1)
	   {
			logMap.insert(std::pair<std::string,std::string> ("SlaveRET_VALUE", "UNSUCCESSFUL"));
		}
	}
}

void EyelockNanoDeviceHandler::receiveChunkAndAppendFile(std::map<std::string, std::string> & _return, const std::vector<std::string> & chunkList){

	FileChunker::receiveChunkAndAppendFile(_return,chunkList);
}

void EyelockNanoDeviceHandler::neededChunkFromFile(std::vector<std::string> & _return, const std::map<std::string, std::string> & neededchunkInfo)
{
	FileChunker::neededChunkFromFile(_return,neededchunkInfo);
}
 int32_t EyelockNanoDeviceHandler::RegisterCallBack(const std::string& ipaddress, const std::string& portno, const ELKNS_EventTypes::type Event)
 {
	EyelockLog(logger, INFO, "Registering callback for destination: %s:%s, event type %d", ipaddress.c_str(), portno.c_str(), (int) Event);
	int32_t ret = UNSUCCESSFUL;
 	if ((!ipaddress.empty()) && (!portno.empty())) {
 	string Ip(ipaddress);
 	Ip.append(":");
 	Ip.append(portno);
 	if (!Ip.empty() && m_pSDKDispatcher != NULL)
 	 {
 		 if(Event == 1)
 		 {
 			 m_pSDKDispatcher->AppendMatchServer(Ip);
 		 }
 		 if(Event == 0)
 		 {
 			 m_pSDKDispatcher->AppendTamperServer(Ip);
 		 }
 		 return ret=0;
 	 }
 	}
 	else
 	 ret = INVALID_INPUT_DATA;
 	 return ret;
 }

  int32_t EyelockNanoDeviceHandler::UnregisterCallBack(const std::string& ipaddress, const std::string& portno, const ELKNS_EventTypes::type Event)
  {
	 EyelockLog(logger, INFO, "Unregistering callback for destination: %s:%s, event type %d", ipaddress.c_str(), portno.c_str(), (int) Event);
	 int32_t ret = UNSUCCESSFUL;
 	 if ((!ipaddress.empty()) && (!portno.empty())) {
 	 string Ip(ipaddress);
 	 Ip.append(":");
 	 Ip.append(portno);
 	 bool statusM = false;
 	 bool statusT = false;
 	 if (!Ip.empty() && m_pSDKDispatcher != NULL)
 	  {
 		 if(!m_pSDKDispatcher) return ret;
 		 if(Event == 1)
 		 {
 			 statusM = m_pSDKDispatcher->DeleteMatchServer(Ip);
 			 EyelockLog(logger, INFO, "The status of Delete Server for Match Event is %d", statusM);
 		 }
 		 if(Event == 0)
 		 {
 			 statusT = m_pSDKDispatcher->DeleteTamperServer(Ip);
 			 EyelockLog(logger, INFO, "The status of Delete Server for Tamper Event is %d", statusT);
 		 }
 		 ret = (statusM  || statusT);
 		 if(ret)
 			 return !ret;
 		 else
 			EyelockLog(logger, INFO, "Provided IpAddress is not found to Unregister");
 		 	return ret;
 	  }
 	 else
 	  {
 		ret=INVALID_INPUT_DATA;
 		return ret;
 	  }
     }
  }

  void EyelockNanoDeviceHandler::getDBCheckSum(std::string& _return)
  {
	  EyelockLog(logger, INFO, "Requested database checksum");
      FILE *fp;
      char output[1024];

      std::string dbPath = m_conf->getValue("GRI.IRISCODEDATABASEFILE","data/sqlite.db3");
      _return = getmd5(dbPath); // Size of the Checksum
    }

 int32_t EyelockNanoDeviceHandler::ThriftcallforMasterSlave()
    {
  	      int32_t ret = SUCCESS;
#ifdef __ARM__
  	 	  const char* slaveIP = m_conf->getValue("GRI.SlaveIP","192.168.40.2");
  	 	  int port = 8090;

  	 	  try
  	 	  {
  	 		  TSocket socket = TSocket(slaveIP, port);
  	 		 socket.setConnTimeout(10000);       //time in ms
  	 		  socket.open();
  	 	  }
  	 	  catch (TException& tx)
  	 	  {
  	 		  EyelockLog(logger, ERROR, "Failed to create socket for communication with the slave: %s", tx.what());
  	 		  return SLAVE_CONNECTION_FLOW_ERROR;
  	 	  }

  	 	  try
  	 	  {

			  boost::shared_ptr<TSocket> socket(new TSocket(slaveIP, port));
			  boost::shared_ptr<TTransport> transport(new TBufferedTransport(socket));
			  boost::shared_ptr<TProtocol> protocol(new TBinaryProtocol(transport));
			  transport->open();
			  if(m_pSlave != NULL)
			  {
				  delete m_pSlave;
				  m_pSlave = NULL;
			  }
			  m_pSlave = new EyelockNanoDeviceClient(protocol);
  	 		  ret = m_pSlave->PingDevice();
  	 	  }

  	 	  catch (TException& tx)
  	 	  {
  	 		  EyelockLog(logger, ERROR, "Failed to connect to the slave: %s", tx.what());
  	 		  ret = CONNECTION_REFUSED;
  	 		  return ret;
  	 	  }
#endif
  	 	  return ret;
    }

 int32_t EyelockNanoDeviceHandler::slaveConnectionValidation(){
	 if(m_bMaster != true) return SUCCESS;
	  int32_t ret = SLAVE_CONNECTION_FLOW_ERROR;
	  try{
	  if(m_pSlave != NULL)
		  ret = m_pSlave->PingDevice();
	  }catch(TException ex){
		  EyelockLog(logger, ERROR, "Slave connection validation failed: %s", ex.what());
	  }
	  int count = 0;
	  while(ret != SUCCESS && count < 3){
		  ret = ThriftcallforMasterSlave();
		  count++;
	  }

	  return ret;
  }
 int32_t EyelockNanoDeviceHandler::getSlaveStatus(int32_t status)
 {
	  switch(status){
	  case SUCCESS:
		  return SUCCESS;
	  case FILE_DOES_NOT_EXIST:
		  return SLAVE_FILE_DOES_NOT_EXIST;
	  }
	  return SLAVE_UNSUCCESSFUL;
 }


 int32_t EyelockNanoDeviceHandler::DeleteDeviceFile(const std::string& path, const bool isDirectory) {

	EyelockLog(logger, INFO, "Requested removing %s (directory: %d)", path.c_str(), isDirectory);
	if(isDirectory)
    {
    	stringstream ss;
    	ss<<"rm -rf " + path;
    	RunSystemCmd(ss.str().c_str());
    }
    else
    	remove(path.c_str());

    return SUCCESS;
  }

 int32_t EyelockNanoDeviceHandler::SendRelayCommand(const ELKNS_RelayTypes::type relayType, const int32_t duration) {

	 if (duration <= 0) // TODO: maximum?
	 {
		 EyelockLog(logger, ERROR, "SendRelayCommand: invalid duration value: %d", duration);
		 return INVALID_INPUT_DATA;
	 }

	 int relayIndex = -1;
	 switch (relayType)
	 {
		 case ELKNS_RelayTypes::GRANT_RELAY:
		 {
			 relayIndex = 1;
			 break;
		 }
		 case ELKNS_RelayTypes::DENY_RELAY:
		 {
			 relayIndex = 2;
			 break;
		 }
		 default:
		 {
			 EyelockLog(logger, ERROR, "SendRelayCommand: invalid relay type: %d", (int) relayType);
			 return INVALID_INPUT_DATA;
		 }
	 }

	 int setRelayOutRet = BoBSetACSRelayOut(relayIndex);
	 if (setRelayOutRet == 0)
	 {
		 // set relay timer
		 // duration in milliseconds is expected
		 EyelockLog(logger, DEBUG, "SendRelayCommand: triggering relay N%d, %d ms", relayIndex, duration);
		 BoBSetRelayTimer(duration);
		 return SUCCESS;
	 }
	 else
	 {
		 EyelockLog(logger, ERROR, "SendRelayCommand: failed to trigger relay (%d)", setRelayOutRet);
		 return UNSUCCESSFUL;
	 }
 }

void* ScriptRunner(void *command){
 	long int tid = syscall(SYS_gettid);
 	EyelockLog(logger, INFO, "Script runner: thread %ld", tid);
 	////should updateInProgress flag be created before any changes? seems that autorestore will restore old version if error before new restore point creation
 	//system("touch /home/updateInProgress.txt");

	string tempCommand(*((string*)command));
	delete (string*)command;
	EyelockLog(logger, INFO, "Script runner: executing %s", tempCommand.c_str());
	sleep(10);

 	RunCmd(tempCommand.c_str());
	return NULL;
	}

 int32_t EyelockNanoDeviceHandler::UpdateFirmware(const std::map<std::string, std::string> & argMap) {
	EyelockLog(logger, INFO, "Requested FW upgrade");

	int ret = UNSUCCESSFUL;
	stringstream ss;
	string firmwareDir = "/home/firmware";

	string downloadedFirmwareFile = argMap.find("Main_File")->second;
	EyelockLog(logger, INFO, "FW file: %s", downloadedFirmwareFile.c_str());

	char* args[] = {"tar", "-xvf", (char *) downloadedFirmwareFile.c_str(), "-C", (char *) firmwareDir.c_str(), NULL};
	RunCmdSafe("/bin/tar", args, true);
	sync();

	string fwHanlderFile = firmwareDir + "/fwHandler.tar.gz";
	string fwHanlderSigFile = fwHanlderFile + ".sig";
	if (OpenSSLSupport::instance().verifySign(fwHanlderFile.c_str(), fwHanlderSigFile.c_str(), "/home/root/rootCert/BSkey.pub"))
	{
		EyelockLog(logger, INFO, "Upgrade script verification passed");

		DecryptFile(fwHanlderFile, firmwareDir);

		string scriptName = "fwHandler.sh";
		string script = firmwareDir + "/fwHandler/" + scriptName;
		EyelockLog(logger, INFO, "Decrypting done, running %s", script.c_str());
		ifstream scriptFile(script.c_str());
		if (scriptFile.good())
		{
			ss << "chmod 777 " << script;
			RunSystemCmd(ss.str().c_str()); ss.str("");

			map<string, string>::const_iterator it;
			it = argMap.find("callbackDestIp"); // client-side sends all 3 or sends nothing
			if (it != argMap.end())
			{
				char* args[] = {(char*) scriptName.c_str(), "upgrade", (char*) it->second.c_str(), (char*) argMap.find("callbackDestPort")->second.c_str(), (char*) argMap.find("callbackDestCommType")->second.c_str(), NULL};
				RunCmdSafe(script.c_str(), args, false);
			}
			else
			{
				char* args[] = {(char*) scriptName.c_str(), "upgrade", NULL};
				RunCmdSafe(script.c_str(), args, false);
			}
			return SUCCESS;
		}
		else
		{
			EyelockLog(logger, ERROR, "Upgrade script corrupted");
			ret = UNSUCCESSFUL;
		}
	}
	else
	{
		EyelockLog(logger, ERROR, "Upgrade tool signature verification failed");
		ret = INVALID_INPUT_DATA;
	}

	ss << "cd " << firmwareDir << "; rm -r fwHandler; rm *.*";
	RunSystemCmd(ss.str().c_str());

	char* rm_args[] = {"rm", (char *) downloadedFirmwareFile.c_str(), NULL};
	RunCmdSafe("/bin/rm", rm_args, true);
	sync();

	return ret;
 }

 int32_t EyelockNanoDeviceHandler::RestoreFirmware(const string& restorePointName, const std::map<std::string, std::string> & argMap) {

	 EyelockLog(logger, INFO, "Requested FW restore");
	 if (restorePointName.empty())
	 {
		 return INVALID_INPUT_DATA;
	 }

	 vector<string> existingRestorePoints;
	 getRestorePoints(existingRestorePoints);
	 if(find(existingRestorePoints.begin(), existingRestorePoints.end(), restorePointName) == existingRestorePoints.end())
	 {
		 return UNSUCCESSFUL;
	 }

	 string script = "/home/root/fwHandler.sh";
	 ifstream scriptFile(script.c_str());
	 if (scriptFile.good())
	 {
		 EyelockLog(logger, INFO, "Restore script check passed");

		 stringstream ss;

		 ss << "chmod 777 " << script;
		 RunSystemCmd(ss.str().c_str()); ss.str("");

		 // command formation

		 string firmwareDir = "/home/firmware";
		 ss << "cd " << firmwareDir << ";";
		 ss << script << " " << "restore" << " " << restorePointName;
		 map<string, string>::const_iterator it;
		 it = argMap.find("callbackDestIp"); // client-side sends all 3 or sends nothing
		 if (it != argMap.end())
		 {
			 ss << " " << it->second << " " << argMap.find("callbackDestPort")->second << " " << argMap.find("callbackDestCommType")->second;
		 }
		 string *command = new string(ss.str());

		 pthread_t threadx;
		 int ret = pthread_create(&threadx, NULL, ScriptRunner, (void*)command);
		 if (!ret)
		 {
			 EyelockLog(logger, INFO, "Restore script was run");
			 return SUCCESS; // script was run successfully
		 }
		 else
		 {
			EyelockLog(logger, ERROR, "Restore script running failed: %d", ret);
			return UNSUCCESSFUL;
		 }

	 }
	 else
	 {

		 if(!runScriptAndCheckReturn("//home//root//scripts//sdk_restoreFirmware.sh", restorePointName)) {
			 EyelockLog(logger, INFO, "Restoring succeeded");
			 return RestartDevice(ELKNS_RestartTypes::REBOOT_DEVICE);
		 } else {
			 EyelockLog(logger, ERROR, "Restoring failed");
			 return UNSUCCESSFUL;
		 }
	 }
 }

 void EyelockNanoDeviceHandler::getRestorePoints(std::vector<std::string> & _return) {
	  EyelockLog(logger, DEBUG, "Requested restore points");
	  struct dirent *entry;
	  DIR *dp;
	  dp = opendir("//home//firmware//nano//restorepoints//");

	  vector<string> sv;

	  if (dp != NULL) {
		    while (entry = readdir(dp)) {
			string s = entry->d_name;
			if((s.find("root_")==0) & (s.find(".tgz")==(s.length()-4)))
				sv.push_back(s);
		  }
		  closedir(dp);

		  sort(sv.begin(), sv.end());
	  }
	  _return = sv;
 }


 int32_t EyelockNanoDeviceHandler::DeleteRestorePoint(const string& restorePointName) {
	 EyelockLog(logger, DEBUG, "Requested removing restore point %s", restorePointName.c_str());
	 return runScriptAndCheckReturn("//home//root//scripts//sdk_deleteRestorePoint.sh", restorePointName);
 }

 int32_t EyelockNanoDeviceHandler::deleteOldestRestorePoint(int limit) {
		vector<string> restorePoints;
		getRestorePoints(restorePoints);
		if (restorePoints.size()>=limit-1) return DeleteRestorePoint(restorePoints[0]);
		return 0;
 }

 int32_t EyelockNanoDeviceHandler::createRestorePoint(string restorePointName) {
		 return runScriptAndCheckReturn("//home//root//scripts//sdk_createRestorePoint.sh", restorePointName);
 }

 string EyelockNanoDeviceHandler::constructRestorePointName(string currentVersion) {
		time_t rawtime;
		struct tm * timeinfo;
		char buffer [80];
		time (&rawtime);
		timeinfo = localtime (&rawtime);
		strftime(buffer,80,"%Y%m%d_%H%M%S",timeinfo);
		std::stringstream ss;
		ss << "root_" << buffer << "_" << currentVersion << ".tgz";
		return ss.str();
 }


 void EyelockNanoDeviceHandler::CreateCustomKey(std::string& _return) {
	 EyelockLog(logger, DEBUG, "Requested custom key creation");

   std::string hostname = RunCmd("cat /etc/hostname"); int hostnameLength = hostname.length();
   if (hostnameLength>1)
   {
	   hostname = hostname.substr(0, hostnameLength-1);
   }
   else
   {
	   _return = "";
	   return;
   }

   std::string binFileName = hostname + ".bin";
   stringstream ss;
   ss << "cd /home/root; ./KeyMgr -c -n 7305 -d 0 -h " << hostname << " -o " << binFileName;
   RunCmd(ss.str().c_str()); ss.str("");

   std::ifstream ifs(binFileName.c_str(), std::ifstream::binary);
   std::string content( (std::istreambuf_iterator<char>(ifs) ), (std::istreambuf_iterator<char>() ) );

	MD5_CTX md5handler;
	unsigned char md5digest[MD5_DIGEST_LENGTH];

	int keySize = 10240;
	MD5((const unsigned char*)content.c_str(), keySize, md5digest);

	// // debug output
	// for (int i=0; i<MD5_DIGEST_LENGTH; i++)
	// {
	// 	printf("%02x", md5digest[i]);
	// };

    _return = content;
 }

 int32_t EyelockNanoDeviceHandler::ConfirmCustomKey(const std::string& keyMd5remote) {
   EyelockLog(logger, INFO, "Requested custom key confirmation");

   std::string hostname = RunCmd("cat /etc/hostname"); int hostnameLength = hostname.length();
   if (hostnameLength>1)
   {
	   hostname = hostname.substr(0, hostnameLength-1);
   }
   else
   {
	   return UNSUCCESSFUL;
   }

   std::string binFileName = hostname + ".bin";
   std::string keyMd5local = getmd5(binFileName);

   if (keyMd5remote != keyMd5local)
   {
	   return UNSUCCESSFUL;

	   // TODO: remove generated files
   }

   stringstream ss;
   ss << "cd /home/root; ./KeyMgr -p -i -1 -d 0 -h " << hostname << " -c " << hostname << ".crt -k " << hostname << ".key";
   RunCmd(ss.str().c_str()); ss.str("");
   return RestartDevice(ELKNS_RestartTypes::REBOOT_EYELOCK);
 }

 int32_t EyelockNanoDeviceHandler::SwitchToDefaultKey() {
	 EyelockLog(logger, DEBUG, "Requested switching to default key");

   RunCmd("cd /home/root; ./KeyMgr -r");
   return RestartDevice(ELKNS_RestartTypes::REBOOT_EYELOCK);

 }

 int32_t EyelockNanoDeviceHandler::GetKeyType() {
	 EyelockLog(logger, DEBUG, "Requested key type");

   std::string keysNumberStr = RunCmd("cd /home/root; ./KeyMgr -n");
   int keysNumberStrLength = keysNumberStr.length();
   if (keysNumberStrLength > 2)
   {
	   keysNumberStr = keysNumberStr.substr(keysNumberStrLength-2, 1);
   }
   else
   {
	   return -1;
   }

   int keysNumber = atoi(keysNumberStr.c_str());
   if (keysNumber == 3)
   {
	   return 1;
   }
   else if (keysNumber == 2)
   {
	   return 0;
   }
   else
   {
	   return -1;
   }

 }

 void EyelockNanoDeviceHandler::GetDeviceId(std::string& _return) {
	 EyelockLog(logger, DEBUG, "Requested device ID");

   char myID[10];

   strcpy(myID, "0000");
   FILE *fp = fopen("/home/root/id.txt", "r");
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
   std::string deviceIdStr(myID);
   _return = deviceIdStr;
 }


void EyelockNanoDeviceHandler::RemoveTempDbFiles()
{
	EyelockLog(logger, INFO, "Removing SDK temporary database files from %s", m_dataDir.c_str());
	stringstream ss;
	ss << "rm " << m_dataDir << "/sqliteSDK*";
	RunCmd(ss.str().c_str());
}

void EyelockNanoDeviceHandler::InitPasswordStorage()
{
	m_passStorage = m_passStorageDefault;

	if (m_conf->getValue("Eyelock.HardwareType", 0) == 0) // 0 = NXT, 1 = EXT, 2 = HBOX
	{
		EyelockLog(logger, DEBUG, "Filesystem is not read-only");
		return;
	}

	// check if custom password storage already exists
	ifstream passStorageCustomIfs(m_passStorageCustom.c_str(), ifstream::in);
	if (passStorageCustomIfs.good())
	{
		m_passStorage = m_passStorageCustom;
		EyelockLog(logger, DEBUG, "Custom password storage already exists");
		passStorageCustomIfs.close();
		return;
	}

	ifstream passStorageDefaultIfs(m_passStorageDefault.c_str(), ifstream::in);
	if (!passStorageDefaultIfs.good())
	{
		EyelockLog(logger, ERROR, "Cannot access default password storage");
		m_passStorage = m_passStorageDefault; // should not be the case. If it does happen, let further code to try again

		return;
	}

	ofstream passStorageCustomOfs(m_passStorageCustom.c_str(), ofstream::out);
	if (!passStorageCustomOfs.good())
	{
		m_passStorage = m_passStorageDefault;
		EyelockLog(logger, ERROR, "Cannot create custom password storage");
		// TODO: signal to WebConfig to notify user about the problem: device must be fixed and password must be changed
		return;
	}

	string userPassEntry;
	while (getline(passStorageDefaultIfs, userPassEntry))
	{
		//EyelockLog(logger, TRACE, "userPassEntry: %s", userPassEntry.c_str());
		// ignoring exclamation (when validating /etc/shadow and the account is locked)
		if (userPassEntry.at(0) == '!')
		{
			userPassEntry.erase(userPassEntry.begin());
		}

		vector<string> entryParts;
		boost::split(entryParts, userPassEntry, boost::is_any_of(":"));
		//EyelockLog(logger, TRACE, "entryParts[0]: %s", entryParts[0].c_str());
		if (entryParts[0].compare("admin") == 0 || entryParts[0].compare("installer") == 0)
		{
			passStorageCustomOfs << userPassEntry << endl;
		}

	}
	passStorageDefaultIfs.close();
	passStorageCustomOfs.close();

	m_passStorage = m_passStorageCustom;

}

int32_t EyelockNanoDeviceHandler::GenerateSaltStr(string& salt)
{
	// generate random byte array using the appropriate random generator
    int randSaltRawSize = 6;
	char randSaltRaw[randSaltRawSize];

	int randomData = open("/dev/urandom", O_RDONLY);
	if (randomData < 0)
	{
		EyelockLog(logger, ERROR, "Random generator is not available");
		return UNSUCCESSFUL;
	}
	else
	{
	    ssize_t result = read(randomData, randSaltRaw, randSaltRawSize);
	    if (result < 0)
	    {
	    	EyelockLog(logger, ERROR, "Failed to read random data");
	    	return UNSUCCESSFUL;
	    }
	    close(randomData);
	}

	// The characters in crypt "salt" are drawn from the set [a-zA-Z0-9./]
	// so, base64 encode (MIME) then replace "+" with "."
	std::stringstream os;
    os << "$1$";
	typedef base64_from_binary<transform_width<const char *,6,8> >base64_text;
	std::copy(base64_text(randSaltRaw), base64_text(randSaltRaw + randSaltRawSize), std::ostream_iterator<char>(os));
	os << "$";
	salt = os.str();
	boost::replace_all(salt, "+", ".");

	return SUCCESS;
}

int32_t EyelockNanoDeviceHandler::SetPassword(const std::string& userName, const std::string& oldPassword, const std::string& newPassword)
{
	EyelockLog(logger, INFO, "Changing password for %s", userName.c_str());
	if ((userName.compare("installer") != 0) && (userName.compare("admin") != 0))
	{
		return INVALID_INPUT_DATA;
	}

	// It seems that maximum password length is limited by the passwd implementation
	// crypt library generates MD5, so password length shouldn't really matter here
	if ((newPassword.length() >= 72)
		|| (newPassword.find("\"") != std::string::npos)
		|| (newPassword.find("\'") != std::string::npos)
		// The same validation as in WebConfig:
		// Please enter a password which is at least 8 characters long, uses at least one upper case,
		// lower case letter, one digit and one symbol character. Quotation marks not allowed'
		|| (!ValidatePasswordStrength(userName, newPassword)))
	{
		return INVALID_INPUT_DATA;
	}

	int32_t oldPasswordStatus = ValidatePassword(userName, oldPassword);
	if (oldPasswordStatus != SUCCESS)
	{
		EyelockLog(logger, INFO, "Old password validation failed for %s", userName.c_str());
		return ACCESS_DENIED;
	}

	if (m_conf->getValue("Eyelock.HardwareType", 0) > 0)
	{
		string salt;
		int32_t saltStat = GenerateSaltStr(salt);
		if (saltStat != SUCCESS)
		{
			EyelockLog(logger, DEBUG, "Cannot generate salt, status: %d", saltStat);
			return saltStat;
		}

		string encryptedNewPass(crypt(newPassword.c_str(), salt.c_str()));

		ifstream passStorageFs(m_passStorage.c_str(), ifstream::in);
		if (!passStorageFs.good())
		{
			EyelockLog(logger, ERROR, "Cannot open password storage");
			return FILE_ACCESS_ERROR;
		}

		// using a temporary file to modify the storage in the form of an atomic operation
		string tempPassStorage = m_passStorage + ".tmp";
		ofstream tempPassStorageFs(tempPassStorage.c_str(), ofstream::out);
		if (!tempPassStorageFs.good())
		{
			EyelockLog(logger, ERROR, "Cannot create temp password file");
			return FILE_ACCESS_ERROR;
		}

		// updating user's password
		string userPassEntry;
		vector<string> entryParts;
		while (getline(passStorageFs, userPassEntry))
		{
			boost::split(entryParts, userPassEntry, boost::is_any_of(":"));
			if (entryParts[0].compare(userName) == 0)
			{
				entryParts[1] = encryptedNewPass;
				userPassEntry = boost::algorithm::join(entryParts, ":");
			}
			tempPassStorageFs << userPassEntry << endl;
		}

		tempPassStorageFs.close();
		passStorageFs.close();

		if (rename(tempPassStorage.c_str(), m_passStorage.c_str()))
		{
			remove(tempPassStorage.c_str());
			EyelockLog(logger, ERROR, "Cannot update password storage");
			return FILE_ACCESS_ERROR;
		}
	}
	else
	{
		// legacy implementation
		stringstream cmd;
		cmd << "echo -e \"" << newPassword << "\\n" << newPassword << "\\n\" | passwd -a MD5 " << userName << ";passwd -l " << userName;
		EyelockLog(logger, DEBUG, "Executing passwd");
		RunCmd(cmd.str().c_str());
	}

	return ValidatePassword(userName, newPassword);
}

int32_t EyelockNanoDeviceHandler::ValidatePassword(const std::string& userName, const std::string& password)
{
	EyelockLog(logger, DEBUG, "Validating password for %s", userName.c_str());

	string userPassEntry;

	InitPasswordStorage();

	ifstream passStorageFs(m_passStorage.c_str(), ifstream::in);
	if (!passStorageFs.good())
	{
		EyelockLog(logger, ERROR, "Cannot open password storage");
		return UNSUCCESSFUL;
	}

	vector<string> entryParts;
	bool userFound = false;
	while (getline(passStorageFs, userPassEntry))
	{
		boost::split(entryParts, userPassEntry, boost::is_any_of(":"));
		if (entryParts[0].compare(userName) == 0)
		{
			userFound = true;
			break;
		}
	}
	passStorageFs.close();

	if (!userFound)
	{
		EyelockLog(logger, ERROR, "Cannot find user %s entry in password storage", userName.c_str());
		return UNSUCCESSFUL;
	}

	string passWithSalt = entryParts[1];
	// ignoring exclamation when working with /etc/shadow and the account is locked
	if (passWithSalt.at(0) == '!')
	{
		passWithSalt.erase(passWithSalt.begin());
	}

	// according GNU libc description of crypt, the whole string including correct password can be passed as "salt" argument for verification
	string encrypted(crypt(password.c_str(), passWithSalt.c_str()));

	if (strcmp(encrypted.c_str(), passWithSalt.c_str()) == 0)
	{
		return SUCCESS;
	}
	else
	{
		EyelockLog(logger, DEBUG, "Password mismatch for %s", userName.c_str());
		return UNSUCCESSFUL;
	}
}

bool EyelockNanoDeviceHandler::ValidatePasswordStrength(const std::string& userName, const std::string& password)
{
	if (userName == "installer" && password == "installer")
	{
		return true;
	}
	if (userName == "admin" && password == "admin")
	{
		return true;
	}

	if (password.length() < 8)
	{
		return false;
	}
	bool lowerFound = false, upperFound = false, digitFound = false, specialFound = false;
	for (std::string::const_iterator it = password.begin(); it != password.end(); it++)
	{
		if (islower(*it))
		{
			lowerFound = true;
		}
		else if (isupper(*it))
		{
			upperFound = true;
		}
		else if (isdigit(*it))
		{
			digitFound = true;
		}
		else
		{
			specialFound = true;
		}
	}
	return lowerFound && upperFound && digitFound && specialFound;
}

int32_t EyelockNanoDeviceHandler::GetRecordsCount() {

	EyelockLog(logger, DEBUG, "Requested records count");
	int32_t result = -1;
	if (m_pNwMatchManager != NULL)
	{
		result = m_pNwMatchManager->GetUserCount(true);
	}
	else
	{
		EyelockLog(logger, ERROR, "Cannot access NwMatchManager");
	}

	return result;
}

void EyelockNanoDeviceHandler::GetFileChecksum(std::string& _return, const std::string& file) {

	EyelockLog(logger, DEBUG, "Requested checksum of %s", file.c_str());

	_return = "";
	ifstream fileStream(file.c_str());
	if (fileStream.good()) // this check will also reject shell command injections
	{
		// md5 calculation is only allowed for files in /home/root/data and /home/firmware
		EyelockLog(logger, TRACE, "Retrieving absolute path of %s", file.c_str());
		char *absolutePath = realpath(file.c_str(), NULL);
		if (absolutePath == NULL)
		{
			EyelockLog(logger, ERROR, "Unable to define absolute path for %s: %s", file.c_str(), strerror(errno));
			return;
		}

		EyelockLog(logger, DEBUG, "File %s absolute path: %s", file.c_str(), absolutePath);
		if (strncmp(absolutePath, "/home/root/data", 15) != 0 && strncmp(absolutePath, "/home/firmware", 14) != 0)
		{
			EyelockLog(logger, ERROR, "MD5 calculation of %s is forbidden", absolutePath);
			free(absolutePath);
			return;
		}
		EyelockLog(logger, TRACE, "MD5 calculation of %s is allowed", absolutePath);
		free(absolutePath);

		_return = getmd5(file); // invokes command line openssl via popen(). TODO: check if this can affect camera driver
		EyelockLog(logger, DEBUG, "File %s md5: %s", file.c_str(), _return.c_str());
	}
	else
	{
		EyelockLog(logger, ERROR, "File %s doesn't exist", file.c_str());
	}
}



