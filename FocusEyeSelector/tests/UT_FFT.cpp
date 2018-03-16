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
#include <cxcore.h>
#include <cv.h>
#include <highgui.h>
#include "../FFTSpoofDetector.h"
extern "C"
{
#include "EdgeImage_private.h"
};

namespace tut {

struct TestFFTData {
	FFTSpoofDetector *FFTSvr;
	IplImage *input;
	TestFFTData(){
		FFTSvr = new FFTSpoofDetector();
		input = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
	}
	~TestFFTData() {
		delete FFTSvr;
		cvReleaseImage(&input);
	}
};
typedef test_group<TestFFTData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("FFT based select server tests");
}

namespace tut {
template<>
template<>
	void testobject::test<1>() {
		set_test_name("Functionality");

		IplImage *inp = FFTSvr->m_fftOutImage;
		IplImage *out = FFTSvr->m_fftMagImage;

		for(int i=0;i<inp->height;i++){
			unsigned int k=0;
			unsigned int* outptr = (unsigned int*)(inp->imageData + i* inp->widthStep);
				for(int j=0;j<inp->width;j++){
				*outptr++ = k++;
			}
		}
		FFTSvr->SwapQuad();

		ensure_equals("SWAP output",0,memcmp(inp->imageData,out->imageData +128*out->widthStep + 512 ,512));

		long int sum = FFTSvr->ComputeSum((unsigned int *)inp->imageData, inp->widthStep,256,256);
		ensure_equals("Sum",sum,(255*(128)*256L));

		FFTSvr->ComputeMagnitude((complex_fract16 *)inp->imageData ,(unsigned int *)out->imageData,256,256);

		for(int i=0;i<inp->height;i++){
			unsigned int* inpptr = (unsigned int*)(inp->imageData + i* inp->widthStep);
			unsigned int* outptr = (unsigned int*)(out->imageData + i* out->widthStep);
				for(int j=0;j<inp->width;j++){
				unsigned int temp = *inpptr++;
				ensure_equals("Magnitude",*outptr++,temp*temp);
			}
		}
	}


}
