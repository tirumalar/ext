#include <tut/tut.hpp>
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "NWHDMatcher.h"
#include "AESClass.h"
#include "AesRW.h"
#include "RWDecorator.h"
#include "ImagePGM.h"
#include "PermuteServer.h"
#include "PermRW.h"
#include "FileRW.h"
#include <unistd.h>

#include "HTTPPOSTMsg.h"
#include "socket.h"


#include "NwMatcherSerialzer.h"
#include "NwMatchManager.h"
#include "DBAdapter.h"

#define DELETEMSG(x) if(x) {delete x; x = NULL;}

extern "C" {
#include "file_manip.h"
}
;
extern int seqindx;
namespace tut {
struct NWMatchMgrUniEye{
	TestConfiguration cfg;//empty configuration
	NWMatchMgrUniEye()
	: m_shiftX(0)
	, m_shiftY(0)
	{
		cfg.setValue("GRI.irisCodeDatabaseFile","./data/sqlite.db3");
		cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");
		cfg.setValue("GRI.HDMatcher.0.Type","LOCAL");
		cfg.setValue("GRI.HDMatcherCount","1");
		cfg.setValue("GRI.NwMmDebug","0");
		cfg.setValue("GRI.MatchMgrDebug","0");


		LoadEyelockDB();
	}
	~NWMatchMgrUniEye() {
	}

	string matchBuffer;
	int m_shiftX;
	int m_shiftY;

	void LoadEyelockDB()
	{
		char *buff = "./data/sqlite.db3";
		string matchBuffer1;
		int req = 50*(IRIS_SIZE_INCLUDING_MASK_PER_PERSON+GUID_SIZE);
		matchBuffer.resize(req);
		memset((void*)matchBuffer.c_str(),0,req);

		DBAdapter *db = new DBAdapter();
		db->OpenFile("./data/sqlite.db3");
		int person = db->GetUserCount();
		int ret = db->MakeMatchBuffer((char*)matchBuffer.c_str(),matchBuffer.length(),person,0);
		ensure("ret should be 0",ret == 0);
		delete db;
	}
	void GetIris(BinMessage** bnMsg,IrisData& irs,CHECK_IRIS chk,  int frameIdx, int illum, bool goodRadius,bool goodCentroid,bool goodCircle,bool corruptIt,int eyeIndex,int prevEyeIndex,bool segmentation)
	{
		//frameIdx
		//use the frame index as incremental for good eyes,
		//bad iris, use non incremental

		//illum
		//illuminator state to be used, for good eyes keep toggling for consecutive eyes
		///bad iris, keep the same for value for consecutive eyes

		if(CHECK_IRIS_LEFT == chk)
		{
			irs.setCamID("0");//for left camera
		}
		else
		{
			irs.setCamID("1");//for right camera
		}

		CURR_TV_AS_USEC(ts1);
		irs.setTimeStamp(ts1);

		//irs.setIrisBuffer(4+m_dbbuff+(m_irishdr.GetOneRecSizeinDB()));
		irs.setIrisBuffer((((unsigned char*)matchBuffer.c_str()) + IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON));
		irs.setFrameIndex(frameIdx);
		irs.setEyeIndex(eyeIndex);
		irs.setPrevIndex(prevEyeIndex);
		irs.setIlluminatorState(illum);
		irs.setSegmentation(segmentation);
		if(goodRadius)
			irs.setIrisRadiusCheck(true);//for good iris
		else
			irs.setIrisRadiusCheck(false);//for bad iris

		if(goodCentroid)
		{
			irs.setSpecCentroid(320+m_shiftX,240+m_shiftY);//For good iris
		}
		else
			irs.setSpecCentroid(320,240);//For bad iris

		if(goodCircle)
		{
			irs.setIrisCircle(320,240,100);//For good iris
		}
		else
			irs.setIrisCircle(320,240,99);//For bad iris



		if(corruptIt)
		{
			//corrupt the iris
			unsigned char* temp = irs.getIris();
			for(int i = 0; i<1280;i++)
			{
				temp[i] = 0x0;
			}
		}

		NwMatcherSerialzer ns;

		int ret = ns.GetSizeOfNwMsg(&irs);
		(*bnMsg) = new BinMessage(ret);
		ns.MakeNwMsg((*bnMsg)->GetBuffer(),&irs);


	}
};

typedef test_group<NWMatchMgrUniEye> tg;
typedef tg::object testobject;
}

namespace {
tut::tg test_group("NWMatchManager Uni eye match");
}

namespace tut {

template<>
template<>
void testobject::test<1>()
{
	set_test_name("no iris sent");

	NwMatchManager obj(cfg);
	ensure_equals("Iris state is none",obj.m_irisState,UT_IRIS_NONE);
}
template<>
template<>
void testobject::test<2>()
{
	set_test_name("one iris sent");
	cfg.setValue("Eyelock.IntraEyeTimeWindowEnable","1");
	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,0,true,true,true,false,0,-1,true);

	obj.process(bnMsg);
	ensure_equals("Iris state matched left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);
}

template<>
template<>
void testobject::test<3>()
{
	set_test_name("matched right eye");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("3 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("3 2 Iris right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);
}

template<>
template<>
void testobject::test<4>()
{
	set_test_name("none of the eyes matched 2l+1r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("Eyelock.IntraEyeTimeWindowEnable","1");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("4 1 Iris left",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);


	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("4 2 Iris right",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);

	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 51;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("4 3 Iris left",obj.m_irisState,UT_IRIS_NO_MATCH);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<5>()
{
	set_test_name("none of the eyes matched 2l+2r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("Eyelock.IntraEyeTimeWindowEnable","1");
	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("5 1 Iris left",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);


	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("5 2 Iris right",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);

	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 51;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("5 3 Iris left",obj.m_irisState,UT_IRIS_NO_MATCH);
	DELETEMSG(bnMsg);


	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	frameIndx = 951;
	illum = 0;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("5 2 Iris right",obj.m_irisState,UT_IRIS_NO_MATCH);
	DELETEMSG(bnMsg);

}

template<>
template<>
void testobject::test<6>()
{
	set_test_name("left eye illum corrupt 2l+1r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("6 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("6 2 Iris right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 51;
	illum = 0;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("6 3 Iris left",obj.m_irisState,UT_IRIS_ILLUM_L);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<7>()
{
	set_test_name("right eye illum corrupt 1l+2r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("7 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("7 2 Iris right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);


	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	frameIndx = 951;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("7 3 Iris right",obj.m_irisState,UT_IRIS_ILLUM_R);
	DELETEMSG(bnMsg);

}

template<>
template<>
void testobject::test<8>()
{
	set_test_name("left eye radius corrupt 2l+1r");
	/*
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("8 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 51;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,false,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("8 2 Iris right",obj.m_irisState,UT_IRIS_RAD_L);
	DELETEMSG(bnMsg);

	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("8 3 Iris right",obj.m_irisState,UT_IRIS_RAD_L);
	DELETEMSG(bnMsg);
*/
}

template<>
template<>
void testobject::test<9>()
{
	set_test_name("right eye radius corrupt 1l+2r");
/*
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("9 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);



	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("9 2 Iris right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	frameIndx = 951;
	illum = 0;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,false,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("9 3 Iris right",obj.m_irisState,UT_IRIS_RAD_R);
	DELETEMSG(bnMsg);
*/
}

template<>
template<>
void testobject::test<10>()
{
	set_test_name("left eye centroid corrupt 2l+1r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("10 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);



	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("10 2 Iris right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 51;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,false,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("10 3 Iris left",obj.m_irisState,UT_IRIS_CENTROID_L);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<11>()
{
	set_test_name("right eye centroid corrupt 1l+2r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("11 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);



	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("11 2 Iris right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	frameIndx = 951;
	illum = 0;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,false,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("11 3 Iris right",obj.m_irisState,UT_IRIS_CENTROID_R);
	DELETEMSG(bnMsg);

}

template<>
template<>
void testobject::test<12>()
{
	set_test_name("left eye prev track frame index corrupt 2l+1r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("12 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);



	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("12 2 Iris right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 55;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("12 3 Iris left",obj.m_irisState,UT_IRIS_MATCH_PREV_L);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<13>()
{
	set_test_name("right eye prev track frame index corrupt 1l+2r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("13 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);



	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("13 2 Iris right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	frameIndx = 988;
	illum = 0;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("13 3 Iris right",obj.m_irisState,UT_IRIS_MATCH_PREV_R);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<14>()
{
	set_test_name("left eye prev track eye index corrupt 2l+1r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("14 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);



	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("14 2 Iris right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	frameIndx = 51;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("14 3 Iris left",obj.m_irisState,UT_IRIS_MATCH_PREV_L);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<15>()
{
	set_test_name("right eye prev track eye index corrupt 1l+2r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("15 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);



	prevEyeIndx = -1;
	eyeIndx = 2;
	frameIndx = 950;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("15 2 Iris right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 951;
	illum = 0;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("15 3 Iris right",obj.m_irisState,UT_IRIS_MATCH_PREV_R);
	DELETEMSG(bnMsg);

}

template<>
template<>
void testobject::test<16>()
{
	set_test_name("left good eyes 2l+0r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("16 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	m_shiftX=10;
	m_shiftY=10;
	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 51;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("16 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_SUCCESS);
	DELETEMSG(bnMsg);

}

template<>
template<>
void testobject::test<17>()
{
	set_test_name("right good eyes 1l+2r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("17 1 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	eyeIndx = 3;
	prevEyeIndx = -1;
	frameIndx = 50;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("17 2 Iris right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);


	m_shiftX=10;
	m_shiftY=10;
	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 51;
	illum = 0;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("17 3 Iris right",obj.m_irisState,UT_IRIS_MATCH_SUCCESS);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<18>()
{
	set_test_name("left good eyes 3l+0r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("Eyelock.IntraEyeTimeWindowEnable","1");
	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("18 1 Iris left",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);


	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 51;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("18 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_PREV_L);
	DELETEMSG(bnMsg);


	m_shiftX=10;
	m_shiftY=10;
	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 52;
	illum = 0;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("18 3 Iris left",obj.m_irisState,UT_IRIS_MATCH_SUCCESS);
	DELETEMSG(bnMsg);

}

template<>
template<>
void testobject::test<19>()
{
	set_test_name("left good eyes 1l+3r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("Eyelock.IntraEyeTimeWindowEnable","1");
	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("19 1 Iris left",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);


	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 400;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("19 2 Iris right",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);

	m_shiftX=10;
	m_shiftY=10;
	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 401;
	illum = 0;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("19 3 Iris right",obj.m_irisState,UT_IRIS_MATCH_PREV_R);
	DELETEMSG(bnMsg);


	m_shiftX=20;
	m_shiftY=20;
	eyeIndx = 3;
	prevEyeIndx = eyeIndx;
	frameIndx = 402;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("19 4 Iris right",obj.m_irisState,UT_IRIS_MATCH_SUCCESS);
	DELETEMSG(bnMsg);

}

template<>
template<>
void testobject::test<20>()
{
	set_test_name("left eye refreshed");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("GRI.NwMmDebug","1");


	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 1;
	int frameIndx = 50;

	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	for(int i=0;i<3;i++)
	{
		illum = i&1;
		prevEyeIndx = eyeIndx;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		DELETEMSG(bnMsg);
	}

	illum = 1;
	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_LEFT);
	ensure_equals("20 3 Iris match",temp->getEyeIndex(),2);
	ensure_equals("20 4 Iris list size",obj.GetListSize(CHECK_IRIS_LEFT),MAX_IRIS_LIST_SIZE);



	illum = 1;
	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_LEFT);
	ensure_equals("20 6 Iris match",temp->getEyeIndex(),2);
	ensure_equals("20 7 Iris list size",obj.GetListSize(CHECK_IRIS_LEFT),MAX_IRIS_LIST_SIZE);

}

template<>
template<>
void testobject::test<21>()
{
	set_test_name("right eye refreshed");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("GRI.NwMmDebug","1");


	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 1;
	int frameIndx = 50;

	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);


	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	for(int i=0;i<3;i++)
	{
		illum = i&1;
		prevEyeIndx = eyeIndx;
		GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		DELETEMSG(bnMsg);
	}

	illum = 1;
	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_RIGHT);
	ensure_equals("21 3 Iris match",temp->getEyeIndex(),2);
	ensure_equals("21 4 Iris list size",obj.GetListSize(CHECK_IRIS_RIGHT),MAX_IRIS_LIST_SIZE);



	illum = 1;
	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_RIGHT);
	ensure_equals("21 6 Iris match",temp->getEyeIndex(),2);
	ensure_equals("21 7 Iris list size",obj.GetListSize(CHECK_IRIS_RIGHT),MAX_IRIS_LIST_SIZE);

}


template<>
template<>
void testobject::test<22>()
{
	set_test_name("left and right not matched eye 2l+1r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("GRI.NwMmDebug","1");


	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 1;
	int frameIndx = 50;

	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);


	illum = 0;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	illum = 1;
	eyeIndx = 1;
	prevEyeIndx = -1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("22 state",obj.m_irisState,UT_IRIS_MATCH_R);
}


template<>
template<>
void testobject::test<23>()
{
	set_test_name("left and right not matched eye 1l+2r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("GRI.NwMmDebug","1");


	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 1;
	int frameIndx = 50;

	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);


	illum = 0;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	illum = 1;
	eyeIndx = 1;
	prevEyeIndx = -1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("23 state",obj.m_irisState,UT_IRIS_MATCH_L);
}


template<>
template<>
void testobject::test<24>()
{
	set_test_name("left not matched eye 2l");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("GRI.NwMmDebug","1");
    cfg.setValue("Eyelock.IntraEyeTimeWindowEnable","1");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 1;
	int frameIndx = 50;

	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("24 state",obj.m_irisState,UT_IRIS_NONE);

	illum = 0;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("24 state",obj.m_irisState,UT_IRIS_NO_MATCH);
}


template<>
template<>
void testobject::test<25>()
{
	set_test_name("left n right not matched eye 2l+2r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("GRI.NwMmDebug","1");


	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 1;
	int frameIndx = 50;

	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);


	illum = 0;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	illum = 0;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	illum = 1;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("25 state",obj.m_irisState,UT_IRIS_NO_MATCH);
}


template<>
template<>
void testobject::test<26>()
{
	set_test_name("left n right not matched eye 2l+2r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("GRI.NwMmDebug","1");


	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 1;
	int frameIndx = 50;

	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);


	illum = 0;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	illum = 0;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	illum = 1;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("26 state",obj.m_irisState,UT_IRIS_CENTROID_R);
}

template<>
template<>
void testobject::test<27>()
{
	set_test_name("left not matched , right matched eye 1l+2r");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("GRI.NwMmDebug","1");


	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 1;
	int frameIndx = 50;

	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);


	illum = 0;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	m_shiftX=10;
	m_shiftY=10;
	illum = 1;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("27 state",obj.m_irisState,UT_IRIS_MATCH_SUCCESS);
}

template<>
template<>
void testobject::test<28>()
{
	set_test_name("left illum corrupt 2l");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("GRI.NwMmDebug","1");


	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 1;
	int frameIndx = 50;


	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	m_shiftX=10;
	m_shiftY=10;
	illum = 1;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("28 state",obj.m_irisState,UT_IRIS_ILLUM_L);
}

template<>
template<>
void testobject::test<29>()
{
	set_test_name("left eye refreshed");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("GRI.NwMmDebug","1");


	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 1;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);
	ensure_equals("29 2 Iris state",obj.m_irisState,UT_IRIS_ILLUM_L);

	for(int i=0;i<3;i++)
	{
		prevEyeIndx = eyeIndx;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		DELETEMSG(bnMsg);
	}

	illum = 1;
	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("29 4 Iris list size",obj.GetListSize(CHECK_IRIS_LEFT),MAX_IRIS_LIST_SIZE);


	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("29 5 Iris list size",obj.GetListSize(CHECK_IRIS_LEFT),MAX_IRIS_LIST_SIZE);


}

template<>
template<>
void testobject::test<30>()
{
	set_test_name("right eye refreshed");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("GRI.NwMmDebug","1");
    cfg.setValue("Eyelock.IntraEyeTimeWindowEnable","1");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 1;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);
	ensure_equals("30 2 Iris state",obj.m_irisState,UT_IRIS_NONE);

	for(int i=0;i<4;i++)
	{
		prevEyeIndx = eyeIndx;
		GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		DELETEMSG(bnMsg);
	}

	illum = 1;
	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("30 4 Iris list size",obj.GetListSize(CHECK_IRIS_RIGHT),MAX_IRIS_LIST_SIZE);


	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("30 5 Iris list size",obj.GetListSize(CHECK_IRIS_RIGHT),MAX_IRIS_LIST_SIZE);


}

template<>
template<>
void testobject::test<31>()
{
	set_test_name("left eye refreshed when match prev fails");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("GRI.NwMmDebug","1");


	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 3;
	int illum = 1;
	int frameIndx = 50;

	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);
	ensure_equals("31 1 Iris state",obj.m_irisState,UT_IRIS_MATCH_L);

	for(int i=0;i<3;i++)
	{
		illum = i&1;
		prevEyeIndx = eyeIndx;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		DELETEMSG(bnMsg);
	}

	ensure_equals("31 2 Iris state",obj.m_irisState,UT_IRIS_CENTROID_L);

	illum = 0;
	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("31 4 Iris list size",obj.GetListSize(CHECK_IRIS_LEFT),2);


	illum = 1;
	eyeIndx = 2;
	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx++,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	DELETEMSG(bnMsg);

	ensure_equals("31 5 Iris list size",obj.GetListSize(CHECK_IRIS_LEFT),1);

}


template<>
template<>
void testobject::test<32>()
{
	set_test_name("1 iris for left inter eye time within time window threshold");
	cfg.setValue("Eyelock.SpoofEnable","0");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("Eyelock.InterIrisTimeWindowThresholdMilliSec","800");
    cfg.setValue("Eyelock.IntraEyeTimeWindowEnable","1");
	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 55;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("32 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 55;
	eyeIndx = 56;
	frameIndx = 51;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("32 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_SUCCESS);
	DELETEMSG(bnMsg);


}


template<>
template<>
void testobject::test<33>()
{
	set_test_name("1 iris for left inter eye time exceeds time window threshold");
	cfg.setValue("Eyelock.SpoofEnable","0");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("Eyelock.InterIrisTimeWindowThresholdMilliSec","800");
    cfg.setValue("Eyelock.IntraEyeTimeWindowEnable","1");
	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 55;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("33 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	sleep(1);

	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 55;
	eyeIndx = 56;
	frameIndx = 51;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("33 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	temp = obj.GetMatchedIris(CHECK_IRIS_LEFT);
	ensure_equals("33 3 Iris match state",temp->getFrameIndex(),51);
	DELETEMSG(bnMsg);


}


template<>
template<>
void testobject::test<34>()
{
	set_test_name("1 iris for left inter eye time exceeds time window threshold with loose flush");
	cfg.setValue("Eyelock.SpoofEnable","0");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("Eyelock.InterIrisTimeWindowThresholdMilliSec","800");
    cfg.setValue("Eyelock.StrictlyFlushEyeLists","0");
    cfg.setValue("Eyelock.IntraEyeTimeWindowEnable","1");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 55;
	int illum = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("34 0 Iris left",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("34 1 Iris left",obj.m_irisState,UT_IRIS_NO_MATCH);
	DELETEMSG(bnMsg);

	sleep(1);

	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 55;
	eyeIndx = 56;
	frameIndx = 51;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("34 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_SUCCESS);
	DELETEMSG(bnMsg);

}


}
