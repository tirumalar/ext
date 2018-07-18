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
#include "../IrisSelectServer.h"
extern "C"
{
#include "EdgeImage_private.h"
};

namespace tut {

struct TestData {
	IplImage *pyrImage;
	IrisSelectServer *server;
	int *pv[3];
	int w,h;
	CvRect roi;
	TestData():w(480),h(640){
		roi= cvRect(w/4 - 32, h/4 - 32, 64, 64);
		server = new IrisSelectServer(w/2, h/2);
		pyrImage = cvCreateImage(cvSize(w/2, h/2), IPL_DEPTH_8U, 1);
		for(int i=0;i<3;i++)
			pv[i] = (int *) calloc(10, sizeof(int));

	}
	~TestData() {
		if(pyrImage) cvReleaseImage(&pyrImage);
		if(server) delete server;

		for(int i=0;i<3;i++)
			free(pv[i]);
	}
};
typedef test_group<TestData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("Iris select server tests");
}

namespace tut {
template<>
template<>
void testobject::test<1>() {
	set_test_name("histogram values test");
	int img_indx[]={44,45,46,47,48,49,50,100,101,102,103};
	int result[]={1990,2644,2916,2771,2506,2161,1934,1572,981,1967,2264};
	for(int i=0;i<11;i++)
	{
		char *str = "data/Input/eye_%d.pgm";
		char tmp[200];
		sprintf(tmp,str,img_indx[i]);

		IplImage *testImg0 = cvLoadImage(tmp, CV_LOAD_IMAGE_GRAYSCALE);

		ensure_equals("image width",testImg0->width,w);
		ensure_equals("image height",testImg0->height,h);

		cvPyrDown( testImg0, pyrImage, CV_GAUSSIAN_5x5 );
		cvReleaseImage(&testImg0);

		server->ComputeFeatureVector(pyrImage,roi , pv[2]);
		sprintf(tmp,"hist[1] for image %d should match",i);
		ensure_equals(tmp,*(pv[2]+1),result[i]);
	}

}


template<>
template<>
void testobject::test<2>() {
	set_test_name("compute_EigenValsHist1");

	int Param[10];
	CvRect rect_asm;
	IplImage *testImg0 = cvLoadImage("data/Input/eye_100.pgm", CV_LOAD_IMAGE_GRAYSCALE);

	ensure_equals("image width",testImg0->width,w);
	ensure_equals("image height",testImg0->height,h);

	IplImage *img=pyrImage;
	int bufferSize=25*img->widthStep*sizeof(int);
	int *temp=(int *)malloc(bufferSize);
	cvPyrDown( testImg0, pyrImage, CV_GAUSSIAN_5x5 );
	cvReleaseImage(&testImg0);

	CvRect rect= roi;
	int k = (int)((256.0f*4*4*25)/(200.0f*128));

	rect_asm.x = MAX(4,(rect.x>>2)<<2);
	rect_asm.width = (rect.width+3)&(~3);
	rect_asm.y = MAX(1,rect.y);
	rect_asm.height = MIN(img->height-1,rect.height);

	memset(temp, 0, (rect.width*4)*25);

	Param[0] = rect.width;//input w
	Param[1] = rect.height;//input h
	Param[2] = img->widthStep; //inputstep
	Param[3] = rect.width*4; //Actual outputstep for other int buffer //int buffers
	Param[4] = (int)temp; //tempbuffer
	Param[5] = (int)k; //constant K

	unsigned char *imageData = (unsigned char *)(img->imageData+(rect_asm.y*img->widthStep)+rect_asm.x);
	int  hist1 = compute_EigenValsHist1(imageData,Param);
	int res = 1572;
	ensure_equals("hist",hist1,res);

	free(temp);
}

}
