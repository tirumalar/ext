#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sendfile.h>
#include <string.h>
#include <tut/tut.hpp>
#include "CommonDefs.h"
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include <EyelockNanoDevice_server.h>
#include "EyeLockThread.h"
#include "LEDDispatcher.h"
#include "LEDConsolidator.h"
#include "AudioDispatcher.h"
#include "DBAdapter.h"
#include "UtilityFunctions.h"
#include "FileChunker.h"
#include "SDKDispatcher.h"
#include "md5.h"
#include <unistd.h>

int StartStream (std::string ipAddress, std::string portNo, bool secure){
	int32_t ret = 1;
    try
    {	TestConfiguration cfg;
    	EyelockNanoDeviceHandler handler(&cfg);
		ret = handler.startImageStream(ipAddress, portNo, secure, (ELKNS_ImageFormats::type)1);
	}
	catch (Exception ex){
	    printf("Caught exception in StartStream \n\n");
	    fflush(stdout);
	    ex.PrintException();
		ret = 1;
	}
	return ret;
}


EyeLockThread* InitEyeLockThread (){
	std::string fileName ("Eyelock.ini");
	FileConfiguration conf(fileName.c_str());
	EyeLockThread* ptr = new EyeLockThread(conf);
	EyeDispatcher *pDispatcher = new EyeDispatcher (conf);
	pDispatcher->init();
	ptr->m_pEyeDispatcher = pDispatcher;
	return ptr;
}

int CorrectStartStream (std::string ipAddress, std::string portNo, bool secure)
{
	int32_t ret = 1;
	EyeLockThread *ptr = NULL;
    try{
    	TestConfiguration cfg;
    	EyelockNanoDeviceHandler handler(&cfg);
    	ptr = InitEyeLockThread();
    	handler.SetEyelockThread(ptr);
		ret = handler.startImageStream(ipAddress, portNo, secure, (ELKNS_ImageFormats::type)1);
	}catch (Exception ex){
	    printf("Caught exception in CorrectStartStream\n\n");
	    fflush(stdout);
	    ex.PrintException();
		ret = 1;
	}
	return ret;
}

int StopStream (std::string ipAddress, std::string portNo)
{
	int32_t ret = 1;
    try{
    	TestConfiguration cfg;
     	EyelockNanoDeviceHandler handler(&cfg);
		ret = handler.stopImageStream(ipAddress, portNo);
	}catch (Exception ex){
	    printf("Caught exception in StopStream \n\n");
	    fflush(stdout);
	    ex.PrintException();
		ret = 1;
	}
	return ret;
}

int StopStreamWithDiffStart (std::string ipAddress, std::string portNo){
	EyeLockThread *ptr = NULL;
	int32_t ret = 1;
    try{
    	TestConfiguration cfg;
    	EyelockNanoDeviceHandler handler(&cfg);
    	ptr = InitEyeLockThread ();
    	handler.SetEyelockThread(ptr);
		ret = handler.startImageStream(ipAddress, portNo, false, (ELKNS_ImageFormats::type)1);
		if (!ret){
			ipAddress = "-1";
			portNo = "-1";
			ret = handler.stopImageStream(ipAddress, portNo);
		}
	}
	catch (Exception ex){
	    printf("Caught exception in StopStream \n\n");
	    fflush(stdout);
	    ex.PrintException();
		ret = 1;
	}
	return ret;
}

int CorrectStopStream (std::string ipAddress, std::string portNo){
	EyeLockThread *ptr = NULL;
	int32_t ret = 1;
    try{
    	TestConfiguration cfg;
    	EyelockNanoDeviceHandler handler(&cfg);
    	ptr = InitEyeLockThread ();
    	handler.SetEyelockThread(ptr);
    	ret = handler.startImageStream(ipAddress, portNo, false, (ELKNS_ImageFormats::type)1);
		if (!ret){
			ret = handler.stopImageStream(ipAddress, portNo);
		}
	}
	catch (Exception ex){
	    printf("Caught exception in CorrectStopStream\n\n");
	    fflush(stdout);
	    ex.PrintException();
		ret = 1;
	}
	return ret;
}
extern int CopyFile (char* Src, char* Dst);


namespace tut {
struct EyelockNanoData {
	int __dbPort;
	int __selDB;

	TestConfiguration cfg;
	SocketServer m_dbserver;
	int m_dbServerThread;
	pthread_t m_thread;

	EyelockNanoData():__dbPort(9000),__selDB(0),m_dbserver(__dbPort,eIPv4)
	{	}

	~EyelockNanoData(){
		m_dbserver.CloseInput();
		m_dbserver.CloseOutput();
	}

	static void OnAcceptClient(Socket& client){
		try
		{
			//lets see what the client wants
			BinMessage *rcvdMsg = new BinMessage(1024);
			client.Receive(*rcvdMsg);
			printf("A request from the client ---> %s\n",rcvdMsg->GetBuffer());
			ensure("There is a request for updated database",strcmp("MATCHED;GUID:",rcvdMsg->GetBuffer()) == 0);
			delete rcvdMsg;

			client.CloseInput();
			client.CloseOutput();
		}
		catch(NetIOException ex)
		{
			ex.PrintException();
		}
		catch(...)
		{
			printf("Client socket could not be closed properly\n");
		}
	}
	static void *StartListening(void* param){
		EyelockNanoData* ptr = (EyelockNanoData*)param;
		if(ptr)
			ptr->m_dbserver.Accept(OnAcceptClient);
	}
	void StartDBServer(){
		m_dbServerThread = pthread_create(&m_thread, NULL, EyelockNanoData::StartListening, this);
		//m_dbserver.Accept(OnAcceptClient);
	}
};
typedef test_group<EyelockNanoData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("EyelockNanoData Test");
}
namespace tut {
	template<>
	template<>
	void testobject::test<1>(){
		printf ("1\n");
		set_test_name("StartStream with correct parameters with no Initialization");
		std::string ipAddress ("192.168.9.45");
		std::string portNo ("4040");
		bool secure = false;
		int32_t ret = StartStream (ipAddress, portNo, secure);
		ensure("Bad test case",ret);
	}

	template<>
	template<>
	void testobject::test<2>(){
		printf ("2\n");
		set_test_name("StartStream with incorrect parameters with no Initialization");
		std::string ipAddress ("192.168.9.45");
		std::string portNo;
		bool secure = false;
		int32_t ret = StartStream (ipAddress, portNo, secure);
		ensure("Bad test case",ret);
	}

#ifndef KIRAN_TOFIX_FORSDK
	template<>
	template<>
	void testobject::test<3>(){
		printf ("3\n");
		set_test_name("Correct StartStream");
		std::string ipAddress ("localhost");
		std::string portNo ("4040");
		bool secure = false;
		int32_t ret = CorrectStartStream (ipAddress, portNo, secure);
		ensure("Good test case",ret==SUCCESS);
	}
#endif

	template<>
	template<>
	void testobject::test<4>(){
		printf ("4\n");
		set_test_name("StartStream with not having all parameters but Initialized");
		std::string ipAddress ("");
		std::string portNo ("2020");
		bool secure = false;
		int32_t ret = CorrectStartStream (ipAddress, portNo, secure);
		ensure("Bad test case",ret);
	}

	template<>
	template<>
	void testobject::test<5>(){
		printf ("5\n");
		set_test_name("StopStream without proper Initialization");
		std::string ipAddress ("192.168.9.45");
		std::string portNo ("4040");
		int32_t ret = StopStream (ipAddress, portNo);
		ensure("Bad test case",ret);
	}

	template<>
	template<>
	void testobject::test<6>(){
		printf ("6\n");
		set_test_name("StopStream without proper Initialization without proper parameters");
		std::string ipAddress ("192.168.9.45");
		std::string portNo;
		int32_t ret = StopStream (ipAddress, portNo);
		ensure("Bad test case",ret);
	}
#ifndef KIRAN_TOFIX_FORSDK
	template<>
	template<>
	void testobject::test<7>(){
		printf ("7\n");
		set_test_name("Correct StopStream");
		std::string ipAddress ("localhost");
		std::string portNo ("4040");
		int32_t ret = CorrectStopStream (ipAddress, portNo);
		ensure("Good test case",ret==SUCCESS);
	}
#endif

	template<>
	template<>
	void testobject::test<8>(){
		printf ("8\n");
		set_test_name("StopStream with Initialization without proper parameters");
		std::string ipAddress ("");
		std::string portNo ("2020");
		int32_t ret = CorrectStopStream (ipAddress, portNo);
		ensure("Bad test case",ret);
	}
#ifndef KIRAN_TOFIX_FORSDK
	template<>
	template<>
	void testobject::test<9>(){
		printf ("9\n");
		set_test_name("StopStream with different IP address");
		std::string ipAddress ("192.168.9.45");
		std::string portNo ("4040");
		int32_t ret = StopStreamWithDiffStart (ipAddress, portNo);
		ensure("Bad test case",ret==SUCCESS);
	}
#endif

	template<>
	template<>
	void testobject::test<10>(){
		printf ("10\n");
		set_test_name("StopStream with different IP address not all parameters properly initialized");
		std::string ipAddress ("192.168.9.45");
		std::string portNo;
		int32_t ret = StopStreamWithDiffStart (ipAddress, portNo);
		ensure("Bad test case",ret);
	}

	template<>
	template<>
	void testobject::test<11>(){
		printf("11\n");
		set_test_name("GetFirmwareVersion Test for Firmware_Revision");
		int32_t ret;
		int32_t retType = 0;
		EyelockNanoDeviceHandler handler(&cfg);
		std::string firmwareversion("MAMIGO");
		std::map <std::string, std::string>localmap;
		std::string version;
		handler.SetEyelockFirmwareVersion(firmwareversion);
		handler.GetFirmwareRevision(localmap,retType);
		version = localmap.find("RET_VALUE")->second;
		ret = version.compare("SUCCESS");
		if(!ret)
		{
			version = localmap.find("Firmware_Revision")->second;
			printf("FirmwareVersion is %s\n", version.c_str());
			int32_t cmp = version.compare(firmwareversion);
			ensure("FIRMWARE version MATCH", !cmp);
			ensure("Firmware version string empty", !version.empty());
		}
		ensure("Good test case",!ret);
	}

	template<>
	template<>
	void testobject::test<12>(){
		printf("12\n");
		set_test_name("GetFirmwareVersion Test for ICM Version");
		int32_t ret;
		int32_t retType = 2;
		EyelockNanoDeviceHandler handler(&cfg);
		std::map <std::string, std::string>localmap;
		char* fPath = "./data/BobVersion";
		cfg.setValue("EYELOCK.ICMVersionFilepath",fPath);
		FILE *fp = fopen("./data/BobVersion","w");
		fprintf(fp,"%s","ICM software version: 3.0.1");
		std::string icm("3.0.1");
		fclose(fp);
		handler.GetFirmwareRevision(localmap,retType);
		std::string sversion = localmap.find("RET_VALUE")->second;
		ret = sversion.compare("SUCCESS");
		if(!ret)
			{
				sversion = localmap.find("Icm_Revision")->second;
				int32_t cmp = icm.compare(sversion);
				printf("IcmVersion is %s\n", sversion.c_str());
				ensure("ICM_Version Match", !cmp);
				ensure("Icmware version string is empty", !sversion.empty());
			}
	   	ensure("Good test case",!ret);
	   	remove("./data/BobVersion");
	}

	template<>
	template<>
	void testobject::test<13>(){
		printf("13\n");
		set_test_name("GetFirmwareVersion Test for ALL Versions");
		int32_t ret;
		int32_t retType = 3;
		EyelockNanoDeviceHandler handler(&cfg);
		std::string firmwareversion("MAMIGO");
		std::map <std::string, std::string>localmap;
		char* fPath = "./data/BobVersion";
		cfg.setValue("EYELOCK.ICMVersionFilepath",fPath);
		FILE *fp = fopen("./data/BobVersion","w");
		fprintf(fp,"%s","ICM software version: 3.0.1");
		std::string icm("3.0.1");
		fclose(fp);
		handler.SetEyelockFirmwareVersion(firmwareversion);
		handler.GetFirmwareRevision(localmap,retType);
		std:: string version = localmap.find("RET_VALUE")->second;
		ret = version.compare("SUCCESS");
		if(!ret)
		{
				version = localmap.find("Firmware_Revision")->second;
				if(!version.empty())
				{
					printf("Firmware Version is %s\n", version.c_str());
					int32_t cmp = version.compare(firmwareversion);
					ensure("FIRMWARE version MATCH", !cmp);
					std::string icmversion = localmap.find("Icm_Revision")->second;
					if(!icmversion.empty())
					{
						printf("Icm Version is %s\n", icmversion.c_str());
					}
					else
					{
						ensure("Icm version string empty", !icmversion.empty());
					}
				 }
				else
				{
					printf("Unable to retrieve version");
					ensure("Firmware version string empty", !version.empty());
				}
		}
		ensure("Good test case",!ret);
		remove("./data/BobVersion");
	}

	template<>
	template<>
	void testobject::test<14>(){
		printf("14\n");
		set_test_name("GetFirmwareVersion Test for SDK");
		int32_t ret;
		int32_t retType = 1;
		EyelockNanoDeviceHandler handler(&cfg);
		std::string check;
		std::string version;
		std::map <std::string, std::string>localmap;
		handler.SetEyelockFirmwareVersion(version);
		handler.GetFirmwareRevision(localmap,retType);
		check = localmap.find("RET_VALUE")->second;
		ret = version.compare("INVALID_INPUT");
	    if(!ret){
	    	printf("Firmware cannot get SDK version.\n");
		}
	   ensure("good test case", ret);
	}

	template<>
	template<>
	void testobject::test<15>(){
		printf("15\n");
		set_test_name("GetFirmwareVersion Test for Proper Input");
		int32_t ret;
		int32_t retType = 99;
		EyelockNanoDeviceHandler handler(&cfg);
		std::string firmwareversion("MAMIGO");
		std::map <std::string, std::string>localmap;
		std::string version;
		handler.SetEyelockFirmwareVersion(firmwareversion);
		handler.GetFirmwareRevision(localmap,retType);
		version = localmap.find("RET_VALUE")->second;
		ret = version.compare("INVALID_INPUT");
		if(!ret){
			printf("INVALID INPUT\n");
		}
		ensure("Good test case",!ret);
	}

	template<>
	template<>
	void testobject::test<16>()
	{
		printf("16\n");
		set_test_name("GetFirmwareVersion Test for Proper Function call");
		int32_t ret;
		int32_t retType = 0;
		EyelockNanoDeviceHandler handler(&cfg);
		std::string firmwareversion("MAMIGO");
		std::map <std::string, std::string>localmap;
		std::string version;
		handler.GetFirmwareRevision(localmap,retType);
		handler.SetEyelockFirmwareVersion(firmwareversion);
		version = localmap.find("RET_VALUE")->second;
		ret = version.compare("INVALID INPUT");
		if(!ret){
		    printf("GetFirmwareVersion is called before Setting it\n");
		}
		ensure("Good test case",ret);
	}

template<>
template<>
void testobject::test<17>()
{
	printf("Test Started\n");
	printf("test<17>\n");
	cfg.setValue("Eyelock.Type","NTSGLM");

	LEDDispatcher *led = new LEDDispatcher(cfg);
	led->init();
	LEDConsolidator *con = new LEDConsolidator(cfg);
	con->init();

	con->SetLedDispatcher(led);
	led->Begin();
	con->Begin();
	sleep(1);

	EyelockNanoDeviceHandler h(&cfg);
	h.SetConsolidator(con);
	LedState val=LED_INITIAL;
	h.ChangeLedColor(1,10000000);
	sleep(2);

	uint64_t ts = 0;
	led->GetState(val,ts);
	uint8_t test = led->GetCurrentValue();
	ensure("LED COLOR SHOULD MATCH",test == (uint8_t)1);
	ensure("LED STATE SHOULD MATCH",val == LED_CONFUSION);

	val=LED_INITIAL;
	h.ChangeLedColor(3,5000000);
	sleep(2);

	led->GetState(val,ts);
	test = led->GetCurrentValue();
	ensure("LED COLOR SHOULD MATCH1",test == (uint8_t)5);
	//ensure("LED STATE SHOULD MATCH1",val == NWSET);

	val=LED_INITIAL;
	h.ChangeLedColor(0,3000000);
	sleep(2);

	led->GetState(val,ts);
	test = led->GetCurrentValue();
	ensure("LED COLOR SHOULD MATCH2",test == (uint8_t)0);
	//ensure("LED STATE SHOULD MATCH2",val == NWSET);

	led->End();
	con->End();
	delete led;
	delete con;
}


template<>
template<>
void testobject::test<18>()
{
	printf("test<18>\n");
	set_test_name("SetAudiolevel Test for proper Input");
	cfg.setValue("Eyelock.AuthorizationToneFile","./data/auth.wav");
	EyelockNanoDeviceHandler handler(&cfg);
	AudioDispatcher *pDispatcher = new AudioDispatcher(cfg);
	handler.SetAudioDispatcher(pDispatcher);
	const double vol = 5;
	double volc = 1;
	int32_t ret1 = handler.SetAudiolevel(vol);
	if(!ret1){
		volc = handler.GetAudiolevel();
		int32_t ret2 = (volc == vol) ? 0 :1;
		printf("Required Volume Setted properly in firmware\n");
	}
	ensure("Volume is set properly", !ret1);
	delete pDispatcher;
}

template<>
template<>
void testobject::test<19>()
{
	printf("test<19>\n");
	set_test_name("SetAudiolevel Test");
	cfg.setValue("Eyelock.AuthorizationToneFile","./data/auth.wav");

	EyelockNanoDeviceHandler handler(&cfg);
	AudioDispatcher *pDispatcher = new AudioDispatcher (cfg);
	handler.SetAudioDispatcher(pDispatcher);
	const double vol = -1.0;
	double gvol = -1.0;
	int ret = handler.SetAudiolevel(vol);
	ensure("volume set audio played",ret==0);
	gvol= handler.GetAudiolevel();
	ensure("Set volume -ve",gvol==0.0);
	const double vol1 = 100.0;
	ret = handler.SetAudiolevel(vol1);
	ensure("volume set audio played",ret==0);
	gvol= handler.GetAudiolevel();
	ensure("Set volume max",gvol==vol1);
	delete pDispatcher;
}

template<>
template<>
void testobject::test<20>(){
	printf("test<20>\n");
	const char *tampfile = "./tamper";
	set_test_name("IsDeviceTampered Test");
	remove(tampfile);
	cfg.setValue("Eyelock.TamperFileName",tampfile);
	EyelockNanoDeviceHandler handler(&cfg);
	int32_t ret = 1;
	ret = handler.IsDeviceTampered();
	ensure("Device not tampered",ret==DEVICE_WAS_NOT_TAMPERED);
	FILE *fp=fopen(tampfile,"w");
	if(fp)fclose(fp);
	ret = handler.IsDeviceTampered();
	ensure("Device was tampered",ret==DEVICE_WAS_TAMPERED);
	remove(tampfile);
}


template<>
template<>
void testobject::test<21>() {
	set_test_name("Test push DB");
	char* dbPath = "./data/pushsqliteTemp.db3";
	cfg.setValue("GRI.irisCodeDatabaseFile",dbPath);
	cfg.setValue("GRITrigger.WeigandEnable","true");
	EyelockNanoDeviceHandler handler(&cfg);

	CopyFile("./data/sqlite.db3",dbPath);
	NwMatchManager* pMatchManager = new NwMatchManager(cfg);
	pMatchManager->init();
	pMatchManager->Begin();
	handler.setNwMatchManager(pMatchManager);

	char* newDB = "./data/pushsqliteNewTemp.db3";
	CopyFile("./data/pushsqliteNew.db3",newDB);
	int sz = FileSize(newDB);
	int bstatus = handler.pushDB(newDB,ACD_Type::WIEGAND);
	ensure("DB successfully pushed",bstatus == SUCCESS);
	sleep(2);

	FILE *outfp = fopen(dbPath,"r+b");
	fseek(outfp, 0L, SEEK_END);
	size_t outsz = ftell(outfp);
	fclose(outfp);
	ensure("push DB should be matched with sent DB",sz == (int)outsz);
	pMatchManager->End();
	delete pMatchManager;
	remove("./data/pushsqliteTemp.db3");
	//	delete ptr;
}


template<>
template<>
void testobject::test<22>(){
    printf("test<22>\n");
    set_test_name("Test for ResetFirmware");
	int32_t retType = 0;
		{
		    cfg.setValue("GRI.FactoryReset","TestResetScript");
			EyelockNanoDeviceHandler handler(&cfg);
			remove("donefwreset");
			retType=handler.ResetFirmware();
			if(retType == FILE_DOES_NOT_EXIST){
				ensure("File Does not Exist",retType==FILE_DOES_NOT_EXIST);
			}
		}
		{
			cfg.setValue("GRI.FactoryReset","./data/Scripts/factoryReset.sh");
			EyelockNanoDeviceHandler handler(&cfg);
			retType=handler.ResetFirmware();
			sleep(40);
			ensure("Done ResetFirmware",retType==SUCCESS);
			ensure("Reset thread run",FileExists("donefwreset"));
		}
		remove("donefwreset");
	}

	template<>
	template<>
	void testobject::test<23>()
	{
		printf("test<23>\n");
		set_test_name("Test for RestartDevice");
		TestConfiguration cfg;
		int32_t retType = 0;
		EyelockNanoDeviceHandler handler(&cfg);
		remove("donereset");
		retType=handler.RestartDevice((EyelockNano::ELKNS_RestartTypes::type)0);
		ensure("Restart Device status",!retType);
		sleep(25);
		ensure("Reset thread run",FileExists("donereset"));
		remove("donereset");
	}
#if 1 //def Madhav
	template<>
    template<>
    void testobject::test<24>() {
		printf("test<24>\n");
		char* DATABASE_TEST_FILE = "./data/pushsqliteTemp.db3";
		CopyFile("./data/sqlite.db3",DATABASE_TEST_FILE);
		set_test_name("Test Upload DB");
		cfg.setValue("GRI.HDMatcherCount","1");
		cfg.setValue("GRI.HDMatcher.0.Type","LOCAL");
		cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");
		cfg.setValue("GRI.irisCodeDatabaseFile",DATABASE_TEST_FILE);
		cfg.setValue("GRITrigger.WeigandEnable","true");
		EyelockNanoDeviceHandler handler(&cfg);

		NwMatchManager* pMatchManager = new NwMatchManager(cfg);
		pMatchManager->init();
//
		handler.setNwMatchManager(pMatchManager);

	char* updateDBFile = "./data/updateSqliteSDKTemp.db3";
	CopyFile("./data/updateSqliteSDK.db3",updateDBFile);
	int bstatus = handler.updateDB(updateDBFile,ACD_Type::WIEGAND);
	ensure("DB successfully updated",bstatus == SUCCESS);
	pMatchManager->Begin();
	sleep(1);

	/*************Verification process***********************/
	string rusername,rleftiris,rrightiris,racd,acdnop;
	int len;
	DBAdapter* db = new DBAdapter();
	db->OpenFile((char*)DATABASE_TEST_FILE);
	int ret = db->GetUserData("BBBBBBBB-AAAA-335E-1601-C3424E98362C",rusername,rleftiris,rrightiris,racd,len,acdnop);
	db->CloseConnection();
	delete db;

	ensure("DB add record successfully", ret == 0);
	ensure("DB added record should be match", rusername == "VINOJ ADD");
	pMatchManager->End();
	delete pMatchManager;
	remove("./data/pushsqliteTemp.db3");
}
#endif


    template<>
	template<>
	void testobject::test<25>(){
		printf("25\n");
		set_test_name("Test for ConfigParameters");
		int32_t ret;
		EyelockNanoDeviceHandler handler(&cfg);
		std::map <int, std::string>confMap;
		handler.GetConfigParameters(confMap);
		ensure("Return map not empty", !confMap.empty());
		// TODO: reorganize test to avoid Eyelock.ini modification
		/*system("cp --preserve=all ./Eyelock.ini ./Eyelocktest.ini");

		EyelockNanoDeviceHandler handler(&cfg);
		std::map <std::string, std::string>confMap;
		handler.GetConfigParameters(confMap);
		ret= handler.SetConfigParameters(confMap);
		ensure("Good test case",ret==SUCCESS);
		{
			FileConfiguration conf("./Eyelocktest.ini");
			EyelockNanoDeviceHandler handler(&conf);
			float Volume = conf.getValue("GRI.AuthorizationToneVolume", 25.0f);
			int LEDBrightness = conf.getValue("GRI.LEDBrightness", 5);
			//ensure("retrieved LEDBrightness successfully", LEDBrightness == 6);
			bool	DualAuthenticationMode = conf.getValue("GRITrigger.DualAuthenticationMode",true);
			char  *m_flag = (char*)conf.getValue("GRITrigger.F2FOutput","PIN30-CONJ10");
			int  SupportNanoSDK = conf.getValue("GRI.SupportNanoSDK", 1);
			ensure("retrieved SupportNanoSDK successfully", SupportNanoSDK == 1);
			int	RepeatAuthorizationPeriod =conf.getValue("GRI.RepeatAuthorizationPeriod", 343);
			const char * filename = conf.getValue("GRI.FactoryReset","TestResetScript");
			string filevalue(filename);
			ensure("retrieved filename successfully", filevalue == "/home/root/factoryReset.sh");
			//const char * tampername = conf.getValue("EYELOCK.TAMPERFILENAME","TestResetScript");
			//string tampervalue(tampername);
			//ensure("retrieved filename successfully", tampervalue == "/home/root/tamper");
		}
		remove("./Eyelocktest.ini");*/
	}
#if 1//Madhav
	template<>
	template<>
	void testobject::test<26>(){
		printf("26\n");
		set_test_name("Test for setting one Config Parameter");
		// TODO: reorganize test to avoid Eyelock.ini modification
		/*int32_t ret;
		system("cp --preserve=all ./Eyelock.ini ./Eyelocktest.ini");

		EyelockNanoDeviceHandler handler(&cfg);
		std::map <std::string, std::string>confMap;
		confMap.insert(std::pair<std::string, std::string>("GRI.AuthorizationToneVolume", "30"));
		ret= handler.SetConfigParameters(confMap);
		ensure("Good test case",ret==SUCCESS);

		FileConfiguration c("Eyelocktest.ini");
		float val = c.getValue("GRI.AuthorizationToneVolume",0.0f);
		ensure("tone volume should be 30",val==30.0f);

		remove("./Eyelocktest.ini");*/
	}
#endif

template<>
template<>
void testobject::test<27>() {
	set_test_name("Retrieve IDs from database");
	cfg.setValue("GRI.irisCodeDatabaseFile","./data/sqlite.db3");
	EyelockNanoDeviceHandler handler(&cfg);
	std::string retrievedIDs;
	handler.RetreiveAllIDs(retrievedIDs);
	ensure("DB successfully updated",retrievedIDs.empty() == false);
	ensure("DB retrieved record should be match", strncmp(retrievedIDs.c_str(),"972E6E6C-D9DA-335E-1601-C3424E98362C",36) == 0);
}

template<>
template<>
void testobject::test<28>() {
	set_test_name("Chunk process Test");
	cfg.setValue("GRI.irisCodeDatabaseFile","./data/sqlite.db3");
	EyelockNanoDeviceHandler handler(&cfg);
     	string outFileName = "./data/Test28temp.db3";;
	remove(outFileName.c_str());

	string inputFile = "./data/sqlite.db3";
	int fileSz = FileSize(inputFile.c_str());

	/*******************Checking for chunking process**********************************/
	FileChunker::sendChunkFromFile(inputFile.c_str(),outFileName,&handler,5120);
	int outfileSz = FileSize(outFileName.c_str());
	remove(outFileName.c_str());
	ensure("Output file should be copied completely",fileSz == outfileSz);


	/*******************Checking for FULL file by increasing Buffer size***************/
	FileChunker::sendChunkFromFile(inputFile.c_str(),outFileName,&handler,512000);
	outfileSz = FileSize(outFileName.c_str());
	remove(outFileName.c_str());
	ensure("Output file should be copied completely",fileSz == outfileSz);

	/*******************Checking for Chunking in Existing file***************/
	outFileName = "./data/sqlite.db3";
	FileChunker::sendChunkFromFile(inputFile.c_str(),outFileName,&handler,5120);
	outfileSz = FileSize(outFileName.c_str());
	remove(outFileName.c_str());
	ensure("Output file should be copied completely",fileSz == outfileSz);



	/*******************Chunk Test for reverse Access************************/
	outFileName = "./data/sqlite.db3";
	FileChunker::receiveChunksFromDevice(inputFile.c_str(),outFileName,&handler,5120);
	outfileSz = FileSize(outFileName.c_str());
	remove(outFileName.c_str());
	ensure("Output file should be copied completely",fileSz == outfileSz);
}
  template<>
    template<>
    void testobject::test<29>() {
    	printf("29\n");
    	set_test_name("Retrieve IDs from database");
    	cfg.setValue("GRI.irisCodeDatabaseFile","./data/sqlite.db3");
    	EyelockNanoDeviceHandler handler(&cfg);
    	std::string retrievedIDs;
    	handler.RetreiveAllIDs(retrievedIDs);
    	ensure("DB successfully updated",retrievedIDs.empty() == false);
    	ensure("DB retrieved record should be match", strncmp(retrievedIDs.c_str(),"972E6E6C-D9DA-335E-1601-C3424E98362C",36) == 0);
    }
#if AS_IT_RUNS_ON_BUILDER_NOT_STOPPING
    template<>
    template<>
    void testobject::test<30>() {
      printf("30\n");
      set_test_name("RegisterCallBack for Person_Macth Event");
      EyelockNanoDeviceHandler handler(&cfg);
      std::string ipAddress("192.168.9.71");
      std::string port("8090");
      SDKDispatcher *pSDKDispatcher = new SDKDispatcher(&cfg);
      pSDKDispatcher->init();
      handler.SetSDKDispatcher(pSDKDispatcher);
      int32_t ret = handler.RegisterCallBack(ipAddress,port,(EyelockNano::ELKNS_EventTypes::type)1);
      std::set<std::string>MatchEventAddr;
      MatchEventAddr =  pSDKDispatcher->GeMatchAddressList();
      string Ip(ipAddress);
      Ip.append(":");
      Ip.append(port);
      printf("\nThe IP_Address and port provided is %s\n",Ip.c_str());
      pSDKDispatcher->AppendMatchServer(Ip);
      pSDKDispatcher->End();
      ensure("RegisterCallBack is now with the provided IPaddress for Match Event\n",ret == 0);
      }

    template<>
    template<>
    void testobject::test<31>() {
        printf("31\n");
        set_test_name("RegisterCallBack for  Tamper Event");
        EyelockNanoDeviceHandler handler(&cfg);
        std::string ipAddress1("192.168.9.96");
        std::string port1("6090");
        std::string ipAddress2("192.168.9.97");
        std::string port2("6091");
        SDKDispatcher *pSDKDispatcher = new SDKDispatcher(&cfg);
        pSDKDispatcher->init();
        handler.SetSDKDispatcher(pSDKDispatcher);
        int32_t ret1 = handler.RegisterCallBack(ipAddress1,port1,(EyelockNano::ELKNS_EventTypes::type)0);
        int32_t ret2 = handler.RegisterCallBack(ipAddress2,port2,(EyelockNano::ELKNS_EventTypes::type)0);
        std::set<std::string>TamperEventAddr;
        TamperEventAddr = pSDKDispatcher->GetTampreAddressList();
        string Ip1(ipAddress1);
        Ip1.append(":");
        Ip1.append(port1);
        pSDKDispatcher->AppendTamperServer(Ip1);
        printf("\nThe IP_Address and port provided is %s\n",Ip1.c_str());
        string Ip2(ipAddress2);
        Ip2.append(":");
        Ip2.append(port2);
        pSDKDispatcher->AppendTamperServer(Ip2);
        printf("\nThe IP_Address and port provided is %s\n",Ip2.c_str());
        pSDKDispatcher->End();
        delete pSDKDispatcher;
       // ensure("RegisterCallBack is now with the provided IPaddress for  Event\n",ret1 == 0);
        ensure("RegisterCallBack is now with the provided IPaddress for  Event\n",ret2 == 0);
     	}

    template<>
    template<>
    void testobject::test<32>() {
      printf("32\n");
      set_test_name("UnRegisterCallBack for Match Event");
      EyelockNanoDeviceHandler handler(&cfg);
      std::string ipAddress("192.168.9.71");
      std::string port("4090");
      SDKDispatcher *pSDKDispatcher = new SDKDispatcher(&cfg);
      pSDKDispatcher->init();
      handler.SetSDKDispatcher(pSDKDispatcher);
      int32_t ret1 = handler.RegisterCallBack(ipAddress,port,(EyelockNano::ELKNS_EventTypes::type)1);
      std::string ipAddress2("192.168.9.80");
      std::string port2("9090");
      handler.RegisterCallBack(ipAddress2,port2,(EyelockNano::ELKNS_EventTypes::type)1);
      ensure("Provided IpAddress and portno are Not Registered Properly to  implement Unregister..",ret1 == 0);
      int32_t ret2 = handler.UnregisterCallBack(ipAddress2,port2,(EyelockNano::ELKNS_EventTypes::type)1);
      pSDKDispatcher->End();
      ensure("\nGood Test Case\nUnRegisterCallBack is now Unregistered the provided IPaddress\n",ret2 == SUCCESS);
      sleep(1);
      delete pSDKDispatcher;
      }

    template<>
    template<>
     void testobject::test<33>() {
          printf("33\n");
          set_test_name("UnRegisterCallBack for Tamper Event");
          EyelockNanoDeviceHandler handler(&cfg);
          std::string ipAddress("192.168.9.78");
          std::string port("4090");
          SDKDispatcher *pSDKDispatcher = new SDKDispatcher(&cfg);
          pSDKDispatcher->init();
          handler.SetSDKDispatcher(pSDKDispatcher);
          int32_t ret1 = handler.RegisterCallBack(ipAddress,port,(EyelockNano::ELKNS_EventTypes::type)0);
          std::string ipAddress2("192.168.9.71");
          std::string port2("9090");
          handler.RegisterCallBack(ipAddress2,port2,(EyelockNano::ELKNS_EventTypes::type)0);
          ensure("Provided IpAddress and portno are Not Registered Properly to  implement Unregister..",ret1 == 0);
          int32_t ret2 = handler.UnregisterCallBack(ipAddress2,port2,(EyelockNano::ELKNS_EventTypes::type)0);
          pSDKDispatcher->End();
          ensure("\nGood Test Case\nUnRegisterCallBack is now Unregistered the provided IPaddress\n",ret2 == SUCCESS);
          sleep(1);
          delete pSDKDispatcher;
          }

    template<>
    template<>
    void testobject::test<34>() {
      printf("34\n");
      set_test_name("UnRegisterCallBack");
      EyelockNanoDeviceHandler handler(&cfg);
      std::string ipAddress("192.168.9.990");
      std::string port("8090");
      SDKDispatcher *pSDKDispatcher = new SDKDispatcher(&cfg);
      pSDKDispatcher->init();
      handler.SetSDKDispatcher(pSDKDispatcher);
      int32_t ret = handler.UnregisterCallBack(ipAddress,port,(EyelockNano::ELKNS_EventTypes::type)0);
      pSDKDispatcher->End();
      ensure("\nret");
      printf("\nBad Test Case......return = %d\n",ret);
      sleep(1);
      delete pSDKDispatcher;
    }
#endif

template<>
template<>
void testobject::test<35>(){
	set_test_name("Retrieve Logs");
	EyelockNanoDeviceHandler handler(&cfg);
	std::map <std::string, std::string>logmap;
	handler.RetrieveLogs(logmap);
	std::string logpath = logmap.find("File_Path")->second;
	std::string fullLog;
	if(logpath!="")
	{
	  FILE *fp = fopen((char*)logpath.c_str(),"rb");
	  if(fp!=NULL)
	  {
	    fseek(fp,0,SEEK_END);
	    size_t fSize = ftell(fp);
	    fullLog.resize(fSize);
	    fseek(fp,0L,SEEK_SET);
	    fread((char*)fullLog.c_str(),1,fSize,fp);
	    fclose(fp);
	    FILE * pFile;
        std::string filepath;
        pFile=fopen("./data/archieve.tar.gz","wb" );
	    fwrite (fullLog.c_str(), 1, fullLog.size(), pFile);
        fclose(pFile);
	    std::string fpath = "./data/Scripts/decompress.sh";
        std::string file_cmd;
        file_cmd.append("sh ");
        file_cmd.append(fpath);
	    RunCmd((char*) file_cmd.c_str());
	    ensure("Logs are compressed and Decompressed properly",fSize == fullLog.size());
	    remove("./data/archieve.tar.gz");
	    system("rm -r ./txtFile");
	  }
   }
}
    template<>
    template<>
    void testobject::test<36>() {
    	set_test_name("Get DB Check Sum");
    	char* dbPath = "./data/sqlite.db3";
    	cfg.setValue("GRI.irisCodeDatabaseFile",dbPath);
    	EyelockNanoDeviceHandler handler(&cfg);

    	std::string outcheckSum;
    	handler.getDBCheckSum(outcheckSum);

    	int fileSize = FileSize(dbPath);
    	FILE* fp = fopen(dbPath,"rb");
    	string fullFile;
    	fullFile.resize(fileSize);
    	fread((void*)fullFile.c_str(),fileSize,1,fp);

    	 MD5 md5 = MD5(fullFile);
    	 string md5string = md5.hexdigest();
    	ensure("CheckSum should not change",strncmp(outcheckSum.c_str(),md5string.c_str(),32) == 0);

    	}

    template<>
    template<>
    void testobject::test<37>() {
    	/*This test is written to check whether the call back is called in the port where callback
    	 * code is written.
    	 * Please change the ip address of the caller where callback is implemented
    	 * Make sure that the server is running
    	 * This will act as a Client
    	 */
    	set_test_name("Callback Test");
    	EyelockNanoDeviceHandler handler(&cfg);
    	StartDBServer();
    	std::string ipAddress("localhost");
    	//mr.setKey("123585868645",12);
    	std::string port("9000");
    	SDKDispatcher *pSDKDispatcher = new SDKDispatcher(&cfg);
    	pSDKDispatcher->init();
    	pSDKDispatcher->Begin();
    	handler.SetSDKDispatcher(pSDKDispatcher);
    	int32_t ret1 = handler.RegisterCallBack(ipAddress,port,(EyelockNano::ELKNS_EventTypes::type)1);

    	/*MatchResult mr;
    	mr.setGUID("3F2504E0-4F89-41D3-9A0C-0305E82C3301");
    	mr.setName("vinoj john hosan");
    	mr.setState(PASSED);*/
		SDKCallbackMsg msg(MATCH, "Match success ID is vinoj john hosan|1234|3F2504E0-4F89-41D3-9A0C-0305E82C3301");
		pSDKDispatcher->enqueMsg(msg);
    	sleep(10);
    	pSDKDispatcher->End();
    	delete pSDKDispatcher;

    }

    template<>
    template<>
    void testobject::test<38>() {
    	set_test_name("Update Firmware Test");
    	EyelockNanoDeviceHandler handler(&cfg);
    	handler.SetEyelockFirmwareVersion("3.00.XXX.UN-OFFICIAL");
    	//************* Slave upgrade check process********************************
    	system("rm -r data/tempath");
    	system("mkdir data/tempath");
    	system("tar -xvf data/NanoNXTUpdate_3.00.17356_Test.tar -C data/tempath");
    	std::map<string,string> filemapM;
    	filemapM.insert(std::pair<std::string,std::string>("HomePath","data/tempath/"));
    	filemapM.insert(std::pair<std::string,std::string>("Main_File","data/tempath/EyelockNxt_v3.00.17356_Slave.tar.gz"));// output whole file path
    	filemapM.insert(std::pair<std::string,std::string>("packagefilename","NanoNXTUpdate_3.00.17356.tar"));
    	filemapM.insert(std::pair<std::string,std::string>("nanoversion","3.00.17356"));//nanoversion
    	filemapM.insert(std::pair<std::string,std::string>("Master_File","EyelockNxt_v3.00.17356_Master.tar.gz"));//nanofilename
    	filemapM.insert(std::pair<std::string,std::string>("Slave_File","EyelockNxt_v3.00.17356_Slave.tar.gz"));//nanofilename
    	filemapM.insert(std::pair<std::string,std::string>("bobversion","3.00.00008"));//bobversion
    	filemapM.insert(std::pair<std::string,std::string>("Icm_File","nanoNxt_ICM_V3.0.8.cyacd"));//bobfilename

    	int ret = handler.UpdateFirmware(filemapM);
    	ensure("The update firmware should pass",ret == SUCCESS);
    	system("rm -r data/tempath");

    	//************* Master upgrade check process********************************
    	TestConfiguration tconf;
    	tconf.setValue("GRI.MASTERMODE","1");
    	tconf.setValue("EYELOCK.ICMVersionFilepath","/home/developer/ws_nanoNxt/Eyelock/data/BobVersion");
    	tconf.setValue("GRI.SlaveIP","127.0.0.1");
    	EyelockNanoDeviceHandler handlerMaster(&tconf);
    	handlerMaster.SetEyelockFirmwareVersion("3.00.XXX.UN-OFFICIAL");

    	system("mkdir data/tempath");
    	CopyFile("data/NanoNXTUpdate_3.00.17356_Test.tar","data/NanoNXTUpdate_3.00.17356_TestTemp.tar");
    	filemapM["HomePath"] ="data/tempath/";
    	filemapM["Main_File"] = "data/NanoNXTUpdate_3.00.17356_TestTemp.tar";// output whole file path
    	ret = handlerMaster.UpdateFirmware(filemapM);
    	ensure("The update firmware should pass",ret == SUCCESS);
    	system("rm -r data/tempath");
    }

template<>
    template<>
    void testobject::test<39>() {
    	set_test_name("Delete folder in thrift call");
    	EyelockNanoDeviceHandler handler(&cfg);

    	//************Delete a file*******************
    	string filePath = "data/DeleteTemp.txt";
    	FILE* fp = fopen(filePath.c_str(),"wb");
    	fclose(fp);
    	int ret = handler.DeleteDeviceFile(filePath,false);
    	ensure("Delete File should be success",ret == SUCCESS);
    	ensure("Delete File should be success",FileExists(filePath.c_str()) == false);

    	//************Delete a file*******************
    	string folderPath = "./data/DeleteTempFolder";
    	system("mkdir ./data/DeleteTempFolder");
    	ret = handler.DeleteDeviceFile(folderPath,true);
    	ensure("Delete File should be success",ret == SUCCESS);

    	struct stat sb;
    	ret = stat(folderPath.c_str(), &sb);
    	ensure("Delete File should be success",stat(folderPath.c_str(), &sb) != 0);


    }
    
    template<>
    template<>
    void testobject::test<40>() {
    	set_test_name("Send Relay Command test");
    	printf ("To be tested");
    	EyelockNanoDeviceHandler handler(&cfg);
    	int duration = 10000;
    	int ret = handler.SendRelayCommand(ELKNS_RelayTypes::GRANT_RELAY, duration);
    	ensure("Send Relay command",ret == SUCCESS);
    }

}
