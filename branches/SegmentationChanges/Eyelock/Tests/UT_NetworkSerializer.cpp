/*
 * FFT.cpp
 *
 *  Created on: Mar 21, 2012
 *      Author: developer1
 */

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
#include <cv.h>
#include <cxcore.h>
#include <IrisData.h>
#include <NwMatcherSerialzer.h>

extern "C"{
#include "file_manip.h"
}
namespace tut {
	struct NwMSGData {
		NwMSGData() {
			m_nwMsgSer = new NwMatcherSerialzer();
			m_irisData = new IrisData();
		}
		~NwMSGData() {
			delete m_nwMsgSer;
			delete m_irisData;
		}
		NwMatcherSerialzer *m_nwMsgSer;
		IrisData *m_irisData;

	};
	typedef test_group<NwMSGData> tg;
	typedef tg::object testobject;
}
namespace {
	tut::tg test_group("NwMatcherSerialzer tests");
}

namespace tut {
	template<>
	template<>
	void testobject::test<1>() {
		set_test_name("NwMatcherSerialzer Test1");

		m_irisData->setIlluminatorState(0);
		m_irisData->setSpecCentroid(1,2);
		m_irisData->setIrisCircle(4,5,6);
		m_irisData->setPupilCircle(7,8,9);
		m_irisData->setEyeIndex(10);
		m_irisData->setFrameIndex(11);
		m_irisData->setCamID("CAMERA1");
		uint64_t t= 0x11223344;
		m_irisData->setTimeStamp(t);
		m_irisData->setAreaScore(12.0f);
		m_irisData->setFocusScore(13.0f);
		m_irisData->setHalo(14.0f);
		m_irisData->setBLC(15.0f);


		int sz = m_irisData->getSizeofIrisData();
		printf("Iris Data SZ %d\n",sz);
		int ts = m_nwMsgSer->GetSizeOfNwMsg(m_irisData);

//		printf("Serialized  SZ %d\n",ts);

		char *buff = (char*)calloc(1,ts);

		int ret = m_nwMsgSer->MakeNwMsg(buff,m_irisData);
//		printf("Actual Buffer Value %d\n",ret);
//		for(int i=1;i<=ret;i++){
//			printf("%02d ",buff[i]);
//			if(i%16 == 0)printf("\n");
//		}
		ensure("The Buffer Size should be same ",ret==ts);

//		printf("Start Extracting\n");
		//Deserializer:
	    IrisData recIrisData;
	    ret =  m_nwMsgSer->ExtractNwMsg(&recIrisData,buff);
	    ensure("Extraction ",ret==true);

	    ensure("ILL State ",m_irisData->getIlluminatorState() == recIrisData.getIlluminatorState());
	    ensure("EyeIndex ",m_irisData->getEyeIndex() == recIrisData.getEyeIndex());
	    ensure("FrameIndex ",m_irisData->getFrameIndex() == recIrisData.getFrameIndex());
	    ensure("SpecCentroid.x ",m_irisData->getSpecCentroid().x == recIrisData.getSpecCentroid().x);
	    ensure("SpecCentroid.y ",m_irisData->getSpecCentroid().y == recIrisData.getSpecCentroid().y);
	    ensure("PupilCircle.x ",m_irisData->getPupilCircle().x == recIrisData.getPupilCircle().x);
	    ensure("PupilCircle.y ",m_irisData->getPupilCircle().y == recIrisData.getPupilCircle().y);
	    ensure("PupilCircle.z ",m_irisData->getPupilCircle().z == recIrisData.getPupilCircle().z);
	    ensure("IrisCircle.x ",m_irisData->getIrisCircle().x == recIrisData.getIrisCircle().x);
	    ensure("IrisCircle.y ",m_irisData->getIrisCircle().y == recIrisData.getIrisCircle().y);
	    ensure("IrisCircle.z ",m_irisData->getIrisCircle().z == recIrisData.getIrisCircle().z);

	    ensure("AreaScore ",m_irisData->getAreaScore() == recIrisData.getAreaScore());
	    ensure("FocusScore ",m_irisData->getFocusScore() == recIrisData.getFocusScore());
	    ensure("HALO ",m_irisData->getHalo() == recIrisData.getHalo());
	    ensure("BLC ",m_irisData->getBLC() == recIrisData.getBLC());

	    ensure("IRIS DATA ",0==memcmp(m_irisData->getIris(),recIrisData.getIris(),2560));

//	    printf("%s  == %s \n",m_irisData->getCamID(),recIrisData.getCamID());
	    ensure("CameraId ",0==memcmp(m_irisData->getCamID(),recIrisData.getCamID(),strlen(recIrisData.getCamID())));

	}
}
