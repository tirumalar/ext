#include <tut/tut.hpp>
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "NWHDMatcher.h"
#include "AESClass.h"
#include "AesRW.h"
#include "RWDecorator.h"
#include "IrisDBHeader.h"
#include "ImagePGM.h"
#include "PermuteServer.h"
#include "PermRW.h"
#include "FileRW.h"


#include "HTTPPOSTMsg.h"
#include "socket.h"
#include <unistd.h>

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
struct NWMatchMgrDualEye{
	TestConfiguration cfg;//empty configuration
	NWMatchMgrDualEye()
	: m_shiftX(0)
	, m_shiftY(0)
	{
		cfg.setValue("Eyelock.DualMatcherPolicy","1");
		cfg.setValue("GRI.irisCodeDatabaseFile","./data/sqlite.db3");
		cfg.setValue("GRI.HDMatcher.0.BuffSize","300000");
		cfg.setValue("GRI.HDMatcher.0.Type","LOCAL");
		cfg.setValue("GRI.HDMatcherCount","1");
		cfg.setValue("GRI.NwMmDebug","1");

		LoadEyelockDB();
	}
	~NWMatchMgrDualEye() {
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
			//ensure_equals("Frame index should be even for left eye",frameIdx & 1,0);
			irs.setCamID("0");//for left camera
			//irs.setIrisBuffer(4+m_dbbuff+(m_irishdr.GetOneRecSizeinDB()));
			irs.setIrisBuffer((((unsigned char*)matchBuffer.c_str()) + IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON));

		}
		else
		{
			//ensure_equals("Frame index should be odd for right eye",frameIdx & 1,1);
			irs.setCamID("1");//for right camera
			//irs.setIrisBuffer(4+m_dbbuff+(m_irishdr.GetOneRecSizeinDB()));
			irs.setIrisBuffer((((unsigned char*)matchBuffer.c_str()) + IRIS_SIZE_INCLUDING_MASK_AND_GUID_PER_PERSON));
		}

		CURR_TV_AS_USEC(ts1);
		irs.setTimeStamp(ts1);

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

typedef test_group<NWMatchMgrDualEye,32> tg;
typedef tg::object testobject;
}

namespace {
tut::tg test_group("NWMatchManager Dual eye match");
}

namespace tut {

template<>
template<>
void testobject::test<1>()
{
	set_test_name("NWMatcher with dual eye enabled 1");

	NwMatchManager obj(cfg);
	ensure_equals("Iris state is none",obj.m_irisState,UT_IRIS_NONE);
}
template<>
template<>
void testobject::test<2>()
{
	set_test_name("NWMatcher with dual eye enabled 2");

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
	set_test_name("NWMatcher with dual eye enabled 3");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,0,true,true,true,false,0,-1,true);

	obj.process(bnMsg);
	ensure_equals("Iris state matched left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,0,true,true,true,false,0,-1,true);
	obj.process(bnMsg);
	ensure_equals("Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);
}


template<>
template<>
void testobject::test<4>()
{
	set_test_name("send only left eye");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 0;
	for(int i =0;i<5;i++)
	{
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("Iris state matched left",obj.m_irisState,UT_IRIS_MATCH_L);
		DELETEMSG(bnMsg);
		prevEyeIndx = eyeIndx;
	}

}


template<>
template<>
void testobject::test<5>()
{
	set_test_name("send only right eye");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 0;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,0,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("1 Iris state matched left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	for(int i =0;i<5;i++)
	{
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("2 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
		DELETEMSG(bnMsg);
		prevEyeIndx = eyeIndx;
	}

}

template<>
template<>
void testobject::test<6>()
{
	set_test_name("left iris not match");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 0;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,0,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("Iris state matched left",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);
}

template<>
template<>
void testobject::test<7>()
{
	set_test_name("right iris not matched");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 0;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,0,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("Iris state matched left",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);


	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,0,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("Iris state matched right",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<8>()
{
	set_test_name("left iris not matched multiple dip");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 0;

	for(int i = 0; i<5 ;i++)
	{
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("Iris state matched left multiple dip",obj.m_irisState,UT_IRIS_NONE);
		DELETEMSG(bnMsg);
		prevEyeIndx = eyeIndx;
	}


}

template<>
template<>
void testobject::test<9>()
{
	set_test_name("right iris not matched multiple dip");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 0;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,0,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("9 Iris state matched left",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);


	for(int i = 0; i<5 ;i++)
	{
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("Iris state matched right multiple dip",obj.m_irisState,UT_IRIS_NONE);
		DELETEMSG(bnMsg);
		prevEyeIndx = eyeIndx;
	}


}




template<>
template<>
void testobject::test<10>()
{
	set_test_name("left match and right not matched multiple dip");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 0;

	for(int i = 0; i<2;i++)
	{
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("10 Iris state matched left",obj.m_irisState,UT_IRIS_MATCH_L);
		DELETEMSG(bnMsg);
		prevEyeIndx = eyeIndx;
	}

	prevEyeIndx = -1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,0,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("10 Iris state matched left->yes : right->no",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);
	prevEyeIndx = eyeIndx;


	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,1,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("10 Left iris has matched but right is not matched",obj.m_irisState,UT_IRIS_NO_MATCH);
	DELETEMSG(bnMsg);

}

template<>
template<>
void testobject::test<11>()
{
	set_test_name("right match and left not matched multiple dip");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 0;

	for(int i = 0; i<2;i++)
	{
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,illum,true,true,true,true,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("11 Iris state not matched left",obj.m_irisState,UT_IRIS_NONE);
		DELETEMSG(bnMsg);
		prevEyeIndx = eyeIndx;
	}

	prevEyeIndx = -1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,0,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("11 Iris state matched right->yes : left->no",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);
	prevEyeIndx = eyeIndx;


	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("11 Right iris has matched but left is not matched",obj.m_irisState,UT_IRIS_NO_MATCH);
	DELETEMSG(bnMsg);

}



template<>
template<>
void testobject::test<12>()
{
	set_test_name("right match and left match multiple dip with left illum corrupt");

	cfg.setValue("Eyelock.SpoofEnable","1");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 0;

	for(int i = 0; i<MAX_IRIS_LIST_SIZE-1;i++)
	{
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,1,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("12 Iris state matched left",obj.m_irisState,UT_IRIS_MATCH_L);
		DELETEMSG(bnMsg);
		prevEyeIndx = eyeIndx;
	}

	prevEyeIndx = -1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("12 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);
	prevEyeIndx = eyeIndx;


	for(int i = 0;i<MAX_IRIS_LIST_SIZE-1 ;i++)
	{
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("12 loop check illuminator",obj.m_irisState,UT_IRIS_ILLUM_L);
		DELETEMSG(bnMsg);
	}

}


template<>
template<>
void testobject::test<13>()
{
	set_test_name("right match and left match multiple dip with right illum corrupt");
	cfg.setValue("Eyelock.SpoofEnable","1");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 0;

	for(int i = 0; i<MAX_IRIS_LIST_SIZE-1;i++)
	{
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("13 Iris state matched left",obj.m_irisState,UT_IRIS_MATCH_L);
		DELETEMSG(bnMsg);
		prevEyeIndx = eyeIndx;
	}

	prevEyeIndx = -1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("13 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);
	prevEyeIndx = eyeIndx;


	for(int i = 0;i<MAX_IRIS_LIST_SIZE-1;i++)
	{
		GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,1,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("13 loop check illuminator",obj.m_irisState,UT_IRIS_ILLUM_R);
		DELETEMSG(bnMsg);
	}

}

template<>
template<>
void testobject::test<14>()
{
	set_test_name("right match and left match multiple dip with left bad radius");
/*
	cfg.setValue("Eyelock.SpoofEnable","1");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 0;

	for(int i = 0; i<5;i++)
	{
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,illum,false,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("14 Iris state matched left",obj.m_irisState,UT_IRIS_MATCH_L);
		DELETEMSG(bnMsg);
		prevEyeIndx = eyeIndx;
	}

	prevEyeIndx = -1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("14 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);
	prevEyeIndx = eyeIndx;


	for(int i = 0;i<5 ;i++)
	{
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("14 loop check radius",obj.m_irisState,UT_IRIS_RAD_L);
		DELETEMSG(bnMsg);
	}
*/
}

template<>
template<>
void testobject::test<15>()
{
	set_test_name("right match and left match multiple dip with right bad radius");
/*
	cfg.setValue("Eyelock.SpoofEnable","1");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 0;

	for(int i = 0; i<5;i++)
	{
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,0,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("15 Iris state matched left",obj.m_irisState,UT_IRIS_MATCH_L);
		DELETEMSG(bnMsg);
		prevEyeIndx = eyeIndx;
	}

	prevEyeIndx = -1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,1,false,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("15 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);
	prevEyeIndx = eyeIndx;


	for(int i = 0;i<5 ;i++)
	{
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,1,illum,false,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("15 loop check radius",obj.m_irisState,UT_IRIS_RAD_R);
		DELETEMSG(bnMsg);
	}
*/
}

template<>
template<>
void testobject::test<16>()
{
	set_test_name("right match and left match multiple dip with left previous eye index not matched");

	cfg.setValue("Eyelock.SpoofEnable","1");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	int prevEyeIndx = -1;
	int eyeIndx = 10;
	int frameIndx = 50;


	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,1,true,true,true,true,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("16 Iris left corrupt",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);


	for(int i = 0; i<3;i++)
	{
		int illum = i&1;
		prevEyeIndx = eyeIndx;
		eyeIndx = i;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("16 Iris left check previous eye index",obj.m_irisState,UT_IRIS_MATCH_L);
		DELETEMSG(bnMsg);
	}

	prevEyeIndx = -1;
	eyeIndx = 0;
	frameIndx = 40;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,0,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("16 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);


	prevEyeIndx = eyeIndx;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,++frameIndx,1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("16 2 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_PREV_L);
	DELETEMSG(bnMsg);

//flushed the lists, begins new entries
	for(int i = 0;i<3 ;i++)
	{
		int illum = i&1;
		prevEyeIndx = eyeIndx;
		GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,++frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("16 loop check previous eye index",obj.m_irisState,UT_IRIS_MATCH_PREV_L);
		DELETEMSG(bnMsg);
	}

}


template<>
template<>
void testobject::test<17>()
{
	set_test_name("right match and left match multiple dip with right previous eye index not matched");

	cfg.setValue("Eyelock.SpoofEnable","1");
	cfg.setValue("GRI.MatcherFeatureMask","15");
	cfg.setValue("GRI.matchScoreThresh","0.30");
	cfg.setValue("Eyelock.MinTrackMatchThresh","-1");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp=NULL;
	int prevEyeIndx = -1;
	int eyeIndx = 0;
	int frameIndx = 50;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("17 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	for(int i = 0; i<3;i++)
	{
		prevEyeIndx = eyeIndx;
		int illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("17 Iris left check previous eye index",obj.m_irisState,UT_IRIS_MATCH_L);
		DELETEMSG(bnMsg);
	}

	temp = obj.GetMatchedIris(CHECK_IRIS_LEFT);
	ensure_equals("17 match the left eye index",temp->getEyeIndex(),0);
	ensure_equals("17 illum the left eye index",temp->getIlluminatorState(),1);

	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 40;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,0,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("17 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_RIGHT);
	ensure_equals("17 1 match the right eye index",temp->getEyeIndex(),10);

	prevEyeIndx = eyeIndx;
	frameIndx = 40;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("17 2 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_PREV_R);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<18>()
{
	set_test_name("check/ensure left and  right match");
	cfg.setValue("Eyelock.SpoofEnable","1");

	BinMessage* bnMsg = NULL;
	NwMatchManager obj(cfg);
	IrisData iris;
	IrisData* temp;
	int prevEyeIndx = -1;
	int eyeIndx = 55;
	int frameIndx = 50;
	int illum = 1;

	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("18 Iris left ",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_LEFT);
	ensure_equals("18 match the eye index",temp->getEyeIndex(),55);

	for(int i = 0; i<3;i++)
	{
		prevEyeIndx = eyeIndx;
		eyeIndx = 0;
		illum = i&1;
		GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
		obj.process(bnMsg);
		ensure_equals("18 Iris left check previous eye index",obj.m_irisState,UT_IRIS_MATCH_L);
		DELETEMSG(bnMsg);

		temp = obj.GetMatchedIris(CHECK_IRIS_LEFT);
		ensure_equals("18 loop match the eye index",temp->getEyeIndex(),55);
	}

	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 40;
	illum = 0;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("18 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_RIGHT);
	ensure_equals("18 right eye match the eye index",temp->getEyeIndex(),eyeIndx);


	eyeIndx = 1;
	prevEyeIndx = 1;
	frameIndx = 40;
	illum = 1;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,++frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("18 2 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_PREV_R);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<19>()
{
	set_test_name("only 2 iris for left and right and check for previous iris failed for left");
	cfg.setValue("Eyelock.SpoofEnable","1");

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
	ensure_equals("19 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_LEFT);
	ensure_equals("19 1 match the eye index",temp->getEyeIndex(),55);

	prevEyeIndx = 56;
	eyeIndx = 57;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx+1,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("19 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_LEFT);
	ensure_equals("19 2 match the eye index",temp->getEyeIndex(),55);


	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 40;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("19 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_RIGHT);
	ensure_equals("19 1 right eye match the eye index",temp->getEyeIndex(),10);

	prevEyeIndx = eyeIndx;
	eyeIndx = 10;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx+1,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("19 2 Iris state ",obj.m_irisState,UT_IRIS_MATCH_PREV_L);
	DELETEMSG(bnMsg);
}


template<>
template<>
void testobject::test<20>()
{
	set_test_name("only 2 iris for left and right and check for previous iris failed for right");
	cfg.setValue("Eyelock.SpoofEnable","1");

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
	ensure_equals("20 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_LEFT);
	ensure_equals("20 1 match the eye index",temp->getEyeIndex(),55);

	prevEyeIndx = eyeIndx;
	eyeIndx = 56;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx+1,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("20 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_LEFT);
	ensure_equals("20 2 match the eye index",temp->getEyeIndex(),55);


	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("20 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	temp = obj.GetMatchedIris(CHECK_IRIS_RIGHT);
	ensure_equals("20 1 right eye match the eye index",temp->getEyeIndex(),10);

	prevEyeIndx = 11;
	eyeIndx = 12;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx+1,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("20 2 Iris state ",obj.m_irisState,UT_IRIS_MATCH_PREV_R);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<21>()
{
	set_test_name("2 iris alternatively for left and right  and check for previous iris failed for left incorrect frame index");
	cfg.setValue("Eyelock.SpoofEnable","1");

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
	ensure_equals("21 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("21 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	prevEyeIndx = 55;
	eyeIndx = 56;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("21 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);



	prevEyeIndx = 10;
	eyeIndx = 12;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx+1,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("21 2 Iris state ",obj.m_irisState,UT_IRIS_MATCH_PREV_L);
	DELETEMSG(bnMsg);

}

template<>
template<>
void testobject::test<22>()
{
	set_test_name("2 iris alternatively for left and right  and check for previous iris failed for right incorrect frame index");
	cfg.setValue("Eyelock.SpoofEnable","1");

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
	ensure_equals("22 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("22 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	prevEyeIndx = 55;
	eyeIndx = 56;
	frameIndx = 50;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("22 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);



	prevEyeIndx = 10;
	eyeIndx = 12;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("22 2 Iris state ",obj.m_irisState,UT_IRIS_MATCH_PREV_R);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<23>()
{
	set_test_name("2 iris alternatively for left and right  and check for previous iris failed for left incorrect eye index");
	cfg.setValue("Eyelock.SpoofEnable","1");

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
	ensure_equals("23 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("23 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	prevEyeIndx = 57;
	eyeIndx = 58;
	frameIndx = 50;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("23 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);



	prevEyeIndx = 10;
	eyeIndx = 12;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("23 2 Iris state ",obj.m_irisState,UT_IRIS_MATCH_PREV_L);
	DELETEMSG(bnMsg);

}

template<>
template<>
void testobject::test<24>()
{
	set_test_name("2 iris alternatively for left and right  and check for previous iris failed for right incorrect eye index");
	cfg.setValue("Eyelock.SpoofEnable","1");

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
	ensure_equals("24 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("24 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	prevEyeIndx = 55;
	eyeIndx = 56;
	frameIndx = 50;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("24 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);



	prevEyeIndx = 14;
	eyeIndx = 14;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("24 2 Iris state ",obj.m_irisState,UT_IRIS_MATCH_PREV_R);
	DELETEMSG(bnMsg);

}

template<>
template<>
void testobject::test<25>()
{
	set_test_name("2 iris alternatively for left and right  and incorrect centroid for left");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");

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
	ensure_equals("25 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("25 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	prevEyeIndx = 55;
	eyeIndx = 55;
	frameIndx = 50;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("25 2.1 Iris left",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	prevEyeIndx = 55;
	eyeIndx = 55;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum+1,true,false,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("25 2.2 Iris left",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	prevEyeIndx = 10;
	eyeIndx = 10;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("25 2 Iris state ",obj.m_irisState,UT_IRIS_CENTROID_L);
	DELETEMSG(bnMsg);



}

template<>
template<>
void testobject::test<26>()
{
	set_test_name("2 iris alternatively for left and right  and incorrect centroid for right");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

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
	ensure_equals("26 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("26 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 55;
	eyeIndx = 56;
	frameIndx = 50;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("26 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);



	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 10;
	eyeIndx = 10;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,++frameIndx,illum+1,true,false,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("26 2 Iris state ",obj.m_irisState,UT_IRIS_CENTROID_R);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<27>()
{
	set_test_name("2 iris alternatively for left and right  and incorrect circle for left");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","35.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

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
	ensure_equals("27 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);


	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("27 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	m_shiftX = 5;
	m_shiftY = 5;
	prevEyeIndx = 55;
	eyeIndx = 55;
	frameIndx = 50;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("27 2.1 Iris left",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 55;
	eyeIndx = 55;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum+1,true,true,false,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("27 2.2 Iris left",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 10;
	eyeIndx = 10;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("27 2 Iris state ",obj.m_irisState,UT_IRIS_CENTROID_L);
	DELETEMSG(bnMsg);



}


template<>
template<>
void testobject::test<28>()
{
	set_test_name("2 iris alternatively for left and right  and incorrect circle for right");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

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
	ensure_equals("28 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("28 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);


	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 55;
	eyeIndx = 56;
	frameIndx = 50;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("28 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);



	m_shiftX = 5;
	m_shiftY = 5;
	prevEyeIndx = 10;
	eyeIndx = 10;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,++frameIndx,illum+1,true,true,false,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("28 2 Iris state ",obj.m_irisState,UT_IRIS_CENTROID_R);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<29>()
{
	set_test_name("2 iris alternatively for left and right  everything is alright");
	cfg.setValue("Eyelock.SpoofEnable","1");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");

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
	ensure_equals("29 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("29 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);


	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 55;
	eyeIndx = 56;
	frameIndx = 50;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("29 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);



	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 10;
	eyeIndx = 10;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("29 2 Iris state ",obj.m_irisState,UT_IRIS_MATCH_SUCCESS);
	DELETEMSG(bnMsg);

}


template<>
template<>
void testobject::test<30>()
{
	set_test_name("2 iris alternatively for left and right intra eye exceeds  time window threshold");
	cfg.setValue("Eyelock.SpoofEnable","0");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("Eyelock.IntraEyeTimeWindowThreshold","990");

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
	ensure_equals("30 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("30 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);


	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 55;
	eyeIndx = 56;
	frameIndx = 50;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("30 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);

	sleep(1);


	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 10;
	eyeIndx = 10;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("30 3 Iris state ",obj.m_irisState,UT_IRIS_MATCH_L);
	temp = obj.GetMatchedIris(CHECK_IRIS_LEFT);
	ensure_equals("30 4 Iris match state",temp->getFrameIndex(),84);
	DELETEMSG(bnMsg);

}



template<>
template<>
void testobject::test<31>()
{
	set_test_name("2 iris alternatively for left and right  loosely flush lists");
	cfg.setValue("Eyelock.SpoofEnable","0");
   	cfg.setValue("Eyelock.EnableCentroidRatio","1");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdX","9.0f");
    cfg.setValue("Eyelock.SpecCentroidRatioThresholdY","0.0f");
    cfg.setValue("Eyelock.IntraEyeTimeWindowThreshold","990");
    cfg.setValue("Eyelock.StrictlyFlushEyeLists","0");

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
	ensure_equals("31 Iris left",obj.m_irisState,UT_IRIS_NONE);
	DELETEMSG(bnMsg);

	eyeIndx = 10;
	prevEyeIndx = -1;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("31 1 Iris state matched right",obj.m_irisState,UT_IRIS_MATCH_R);
	DELETEMSG(bnMsg);


	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 55;
	eyeIndx = 56;
	frameIndx = 51;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("31 2 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	sleep(1);

	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 10;
	eyeIndx = 10;
	frameIndx = 84;
	GetIris(&bnMsg,iris,CHECK_IRIS_RIGHT,++frameIndx,illum+1,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("31 3 Iris state ",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

	m_shiftX = 10;
	m_shiftY = 10;
	prevEyeIndx = 55;
	eyeIndx = 56;
	frameIndx = 52;
	GetIris(&bnMsg,iris,CHECK_IRIS_LEFT,frameIndx,illum,true,true,true,false,eyeIndx,prevEyeIndx,true);
	obj.process(bnMsg);
	ensure_equals("31 4 Iris left",obj.m_irisState,UT_IRIS_MATCH_L);
	DELETEMSG(bnMsg);

}



}
