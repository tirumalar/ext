/*
 * UT_SafePtr.cpp
 *
 *  Created on: 02-Sep-2009
 *      Author: mamigo
 */

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include "IrisData.h"
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "Configuration.h"
#include "SocketFactory.h"
#include "LoiteringDetector.h"
#include <unistd.h>
extern "C" {
#include "file_manip.h"
}

namespace tut {
struct LoiteringDetectionTestData {
		TestConfiguration cfg;//empty configuration
		LoiteringDetectionTestData() {
			cfg.setValue("Eyelock.LoiteringQSize", "2");
			cfg.setValue("GRI.cameraID","CAMERA1");
			cfg.setValue("Eyelock.LoiteringCardData","0x00123456789ABCDEF");
			cfg.setValue("Eyelock.LoiteringDebug","0");
		}
		~LoiteringDetectionTestData() {
		}
		void SetIrisData(IrisData& m_irisData,int halo,uint64_t t= 0,const std::string camera="CAMERA1"){
			m_irisData.setIlluminatorState(0);
			m_irisData.setSpecCentroid(1,2);
			m_irisData.setIrisCircle(4,5,6);
			m_irisData.setPupilCircle(7,8,9);
			m_irisData.setEyeIndex(10);
			m_irisData.setFrameIndex(11);
			m_irisData.setCamID((char*)camera.c_str());
			struct timeval m_timer;
			gettimeofday(&m_timer, 0);
			TV_AS_USEC(m_timer,ts);
			m_irisData.setTimeStamp(ts+t);
			m_irisData.setAreaScore(12.0f);
			m_irisData.setFocusScore(13.0f);
			m_irisData.setHalo(halo*1.0f);
			m_irisData.setBLC(15.0f);
		}
	};
	typedef test_group<LoiteringDetectionTestData> tg;
	typedef tg::object testobject;
}
namespace {
	tut::tg test_group("Loitering Detection Tests");
}

namespace tut {
	template<>
	template<>
	void testobject::test<1>() {
		set_test_name("Simple1 Test");
		LoiteringDetector proc(cfg);

		proc.init();
		IrisData m_irisData;

		for(int i=0;i<3;i++){
			SetIrisData(m_irisData,i,0);
			proc.enqueMsg(m_irisData);
		}
		SetIrisData(m_irisData,4,3000000);
		proc.enqueMsg(m_irisData);


//		proc.PrintData();

		proc.CheckLoitering();

		bool ret = proc.GetMasterLoiteringStatus();
		ensure("Loitering To be Sent For Master",ret);
		bool ret1 = proc.GetSlaveLoiteringStatus();
		ensure("Loitering To be SentFor Slave ",!ret1);
	}

	template<>
	template<>
	void testobject::test<2>() {
		set_test_name("Simple2 Test");
		LoiteringDetector proc(cfg);

		proc.init();
		IrisData m_irisData;

		for(int i=0;i<3;i++){
			SetIrisData(m_irisData,i,0);
			proc.enqueMsg(m_irisData);
		}
		proc.CheckLoitering();
		bool ret = proc.GetMasterLoiteringStatus();
		ensure("NoLoitering To be Sent For Master0",!ret);

		bool ret1 = proc.GetSlaveLoiteringStatus();
		ensure("Loitering To be SentFor Slave0 ",!ret1);

		proc.CheckLoitering();
		ret = proc.GetMasterLoiteringStatus();
		ensure("NoLoitering To be Sent For Master1",!ret);

		ret1 = proc.GetSlaveLoiteringStatus();
		ensure("Loitering To be SentFor Slave1 ",!ret1);

		proc.ClearVector();
		ensure("Vector cleared",!ret);

		proc.CheckLoitering();
		ret = proc.GetMasterLoiteringStatus();
		ensure("No Loitering To be Sent For Master2",!ret);
		ret1 = proc.GetSlaveLoiteringStatus();
		ensure("Loitering To be SentFor Slave2 ",!ret1);
	}
#if 0
	template<>
	template<>
	void testobject::test<3>() {
		set_test_name("Complex Test");
		LoiteringDetector proc(cfg);

		proc.init();
		IrisData m_irisData;

		for(int i=0;i<5;i++){
			SetIrisData(m_irisData,i,i*2000000);
			proc.enqueMsg(m_irisData);
		}

		for(int i=5;i<7;i++){
			SetIrisData(m_irisData,i,0);
			proc.enqueMsg(m_irisData);
		}

		proc.PrintData();

		bool ret = proc.CheckLoitering();
		ensure("No Loitering0",ret);
//		proc.PrintData();

		ret = proc.CheckLoitering();
		ensure("No Loitering1",ret);
//		proc.PrintData();

		ret = proc.CheckLoitering();
		ensure("No Loitering2",ret);
//		proc.PrintData();

		ret = proc.CheckLoitering();
		ensure("No Loitering3",ret);
//		proc.PrintData();

		ret = proc.CheckLoitering();
		ensure("No Loitering4",ret);
//		proc.PrintData();

		ret = proc.CheckLoitering();
		ensure("No Loitering5",!ret);
//		proc.PrintData();

		SetIrisData(m_irisData,10,10123438);
		proc.enqueMsg(m_irisData);

		printf("Print the 3 iris\n ");
		proc.PrintData();
		ret = proc.CheckLoitering();
		ensure("No Loitering6",ret);
		printf("Print the 1 iris\n ");
		proc.PrintData();

		ret = proc.CheckLoitering();
		ensure("No Loitering7",ret);
		proc.PrintData();

		ret = proc.CheckLoitering();
		ensure("No Loitering8",!ret);
		proc.PrintData();
	}
#endif
	template<>
	template<>
	void testobject::test<4>()
	{
		set_test_name("Flush loitering within stipulated time");
		LoiteringDetector proc(cfg);

		proc.init();
		IrisData m_irisData;

		for(int i=0;i<3;i++)
		{
			SetIrisData(m_irisData,i,0);
			proc.enqueMsg(m_irisData);
		}

		SetIrisData(m_irisData,4,3000000);
		proc.enqueMsg(m_irisData);

//		proc.PrintData();

		proc.CheckLoitering();
		ensure("FlushedHow is WITHIN_STIPULATED_TIME",WITHIN_STIPULATED_TIME == proc.FlushedHow());
	}

	template<>
	template<>
	void testobject::test<5>()
	{
		set_test_name("Flush loitering within the silent time");
		LoiteringDetector proc(cfg);

		proc.init();
		IrisData m_irisData;

		for(int i=0;i<3;i++)
		{
			SetIrisData(m_irisData,i,0);
			proc.enqueMsg(m_irisData);
		}

		SetIrisData(m_irisData,4,3000000);
		proc.enqueMsg(m_irisData);

//		proc.PrintData();

		proc.CheckLoitering();
		//now we have the matched time stamp after flush

		//fill in the data
		for(int i=0;i<3;i++)
		{
			SetIrisData(m_irisData,i,0);
			proc.enqueMsg(m_irisData);
		}

		SetIrisData(m_irisData,4,1000000);
		proc.enqueMsg(m_irisData);
		proc.CheckLoitering();

		ensure("FlushedHow is WITHIN_SILENT_TIME",WITHIN_SILENT_TIME == proc.FlushedHow());

	}

	template<>
	template<>
	void testobject::test<6>()
	{
		set_test_name("Flush loitering periodically");
		LoiteringDetector proc(cfg);

		proc.init();
		IrisData m_irisData;

		for(int i=0;i<3;i++)
		{
			SetIrisData(m_irisData,i,0);
			proc.enqueMsg(m_irisData);
		}

		SetIrisData(m_irisData,4,1000);
		proc.enqueMsg(m_irisData);

//		proc.PrintData();
		sleep(10);
		proc.CheckLoitering();
		ensure("FlushedHow is PERIODIC_CLEANING",PERIODIC_CLEANING == proc.FlushedHow());
	}

	template<>
	template<>
	void testobject::test<7>()
	{
		set_test_name("M+S Flush loitering within stipulated time");
		LoiteringDetector proc(cfg);

		proc.init();
		IrisData m_irisData;

		//for master
		SetIrisData(m_irisData,0,0);
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,3000000);
		proc.enqueMsg(m_irisData);

		//for slave
		SetIrisData(m_irisData,0,0,"CAMERA2");
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,3000000,"CAMERA2");
		proc.enqueMsg(m_irisData);


//		proc.PrintData();

		proc.CheckLoitering();
		ensure("M+S FlushedHow is WITHIN_STIPULATED_TIME",WITHIN_STIPULATED_TIME == proc.FlushedHow());
	}

	template<>
	template<>
	void testobject::test<8>()
	{
		set_test_name("M+S Flush loitering within the silent time");
		LoiteringDetector proc(cfg);

		proc.init();
		IrisData m_irisData;

		//for master
		SetIrisData(m_irisData,0,0);
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,5000000);
		proc.enqueMsg(m_irisData);

		//for slave
		SetIrisData(m_irisData,0,0,"CAMERA2");
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,5000000,"CAMERA2");
		proc.enqueMsg(m_irisData);

//		proc.PrintData();

		proc.CheckLoitering();
		//now we have the matched time stamp after flush

		//for master
		SetIrisData(m_irisData,0,0);
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,1000000);
		proc.enqueMsg(m_irisData);

		//for slave
		SetIrisData(m_irisData,0,0,"CAMERA2");
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,1000000,"CAMERA2");
		proc.enqueMsg(m_irisData);

		proc.CheckLoitering();

		ensure("M+S FlushedHow is WITHIN_SILENT_TIME",WITHIN_SILENT_TIME == proc.FlushedHow());

	}

	template<>
	template<>
	void testobject::test<9>()
	{
		set_test_name("M+S Flush loitering periodically");
		LoiteringDetector proc(cfg);

		proc.init();
		IrisData m_irisData;

		//for master
		SetIrisData(m_irisData,0,0);
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,1000);
		proc.enqueMsg(m_irisData);

		//for slave
		SetIrisData(m_irisData,0,0,"CAMERA2");
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,1000,"CAMERA2");
		proc.enqueMsg(m_irisData);

//		proc.PrintData();
		sleep(10);
		proc.CheckLoitering();
		ensure("M+S FlushedHow is PERIODIC_CLEANING",PERIODIC_CLEANING == proc.FlushedHow());
	}

	template<>
	template<>
	void testobject::test<10>()
	{
		set_test_name("M+S time mismatch Flush loitering within stipulated time");
		LoiteringDetector proc(cfg);

		proc.init();
		IrisData m_irisData;

		//for master
		SetIrisData(m_irisData,0,0);
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,3000000);
		proc.enqueMsg(m_irisData);

		//for slave
		SetIrisData(m_irisData,0,100,"CAMERA2");
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,3000100,"CAMERA2");
		proc.enqueMsg(m_irisData);


//		proc.PrintData();

		proc.CheckLoitering();
		ensure("M+S  time mismatch FlushedHow is WITHIN_STIPULATED_TIME",WITHIN_STIPULATED_TIME == proc.FlushedHow());
	}

	template<>
	template<>
	void testobject::test<11>()
	{
		set_test_name("M+S  time mismatch Flush loitering within the silent time");
		LoiteringDetector proc(cfg);

		proc.init();
		IrisData m_irisData;

		//for master
		SetIrisData(m_irisData,0,0);
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,5000000);
		proc.enqueMsg(m_irisData);

		//for slave
		SetIrisData(m_irisData,0,200,"CAMERA2");
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,5000200,"CAMERA2");
		proc.enqueMsg(m_irisData);

//		proc.PrintData();

		proc.CheckLoitering();
		//now we have the matched time stamp after flush

		//for master
		SetIrisData(m_irisData,0,0);
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,1000000);
		proc.enqueMsg(m_irisData);

		//for slave
		SetIrisData(m_irisData,0,200,"CAMERA2");
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,1000200,"CAMERA2");
		proc.enqueMsg(m_irisData);

		proc.CheckLoitering();

		ensure("M+S  time mismatch FlushedHow is WITHIN_SILENT_TIME",WITHIN_SILENT_TIME == proc.FlushedHow());

	}

	template<>
	template<>
	void testobject::test<12>()
	{
		set_test_name("M+S  time mismatch Flush loitering periodically");
		LoiteringDetector proc(cfg);

		proc.init();
		IrisData m_irisData;

		//for master
		SetIrisData(m_irisData,0,0);
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,1000);
		proc.enqueMsg(m_irisData);

		//for slave
		SetIrisData(m_irisData,0,400,"CAMERA2");
		proc.enqueMsg(m_irisData);

		SetIrisData(m_irisData,4,1400,"CAMERA2");
		proc.enqueMsg(m_irisData);

//		proc.PrintData();
		sleep(10);
		proc.CheckLoitering();
		ensure("M+S  time mismatch FlushedHow is PERIODIC_CLEANING",PERIODIC_CLEANING == proc.FlushedHow());
	}

}
