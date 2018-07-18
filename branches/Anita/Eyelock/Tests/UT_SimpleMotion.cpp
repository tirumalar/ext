/*
 * UT_Configuration.cpp
 *
 *  Created on: 9 Dec, 2008
 *      Author: akhil
 */
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include "FileConfiguration.h"
#include "TestConfiguration.h"
#include "ImageProcessor.h"

extern "C"{
#include "file_manip.h"
}

namespace tut {

struct TestMotionData {
	TestConfiguration cfg;//empty configuration
	IplImage *ptr1,*ptr2;
	TestMotionData() {
		cfg.setValue("GRIMotion.MinRatioMovingFramePixels","0.1");
		int width=384;
		int height = 288;

		//cfg.setValue("test.fileNamePattern","data/focus_%03d.pgm");
		ptr1 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
		ptr2 = cvCreateImage(cvSize(width,height),IPL_DEPTH_8U,1);
	}
	~TestMotionData() {
		cvReleaseImage(&ptr1);
		cvReleaseImage(&ptr2);
	}
};

typedef test_group<TestMotionData> tg;
typedef tg::object testobject;
}
namespace {
tut::tg test_group("GRI Motion Detection Test");
}

namespace tut {

template<>
template<>
void testobject::test<1>() {
		set_test_name("Diff each pixel is exact 50 ");
		cfg.setValue("GRIMotion.AbsDiffThreshold","50");

		for(int i=0;i<ptr1->height;i++)
		{
			unsigned char *img1 = (unsigned char *)(ptr1->imageData + i* ptr1->widthStep);
			unsigned char *img2 = (unsigned char *)(ptr2->imageData + i* ptr2->widthStep);

			for(int j=0;j<ptr1->width;j++)
			{
				*img1++ = 25;
				*img2++ = 75;
			}
		}
		SimpleMotionDetection mot(&cfg,2592,1944);
		bool ret = mot.Difference(ptr1,ptr2);
		ensure("Difference ",!ret);
	}

template<>
template<>
void testobject::test<2>() {


	set_test_name("Diff each pixel is just 51 test");
	cfg.setValue("GRIMotion.AbsDiffThreshold","50");

	for(int i=0;i<ptr1->height;i++)
	{
		unsigned char *img1 = (unsigned char *)(ptr1->imageData + i* ptr1->widthStep);
		unsigned char *img2 = (unsigned char *)(ptr2->imageData + i* ptr2->widthStep);

		for(int j=0;j<ptr1->width;j++)
		{
			*img1++ = 25;
			*img2++ = 76;
		}
	}
	SimpleMotionDetection mot(&cfg,2592,1944);
	bool ret = mot.Difference(ptr1,ptr2);
	ensure("Difference ",ret);
	}

	template<>
	template<>
	void testobject::test<3>() {

	set_test_name("Diff each pixel is uneven...");
	cfg.setValue("GRIMotion.AbsDiffThreshold","50");

	for(int i=0;i<ptr1->height;i++)
	{
		unsigned char *img1 = (unsigned char *)(ptr1->imageData + i* ptr1->widthStep);
		unsigned char *img2 = (unsigned char *)(ptr2->imageData + i* ptr2->widthStep);

		for(int j=0;j<ptr1->width;j++)
		{
			*img1++ = j;
			*img2++ = j+50;
		}
	}
	SimpleMotionDetection mot(&cfg,2592,1944);
	bool ret = mot.Difference(ptr1,ptr2);
	ensure("Difference ",ret);
	}

	template<>
	template<>
	void testobject::test<4>() {

	set_test_name("Process test");
	cfg.setValue("GRIMotion.AbsDiffThreshold","50");
	cfg.setValue("GRIMotion.Debug","0");
	cfg.setValue("GRIMotion.Level","4");
	cfg.setValue("GRIMotion.QSize","5");
	cfg.setValue("GRIMotion.History","5");

	int w = 2592>>3;
	int h = 1944>>3;

	IplImage *inp = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,1);
	unsigned int *ptr = (unsigned int *)(inp->imageData);
	SimpleMotionDetection mot(&cfg,2592,1944);
	struct timeval timest;
	int i;
	for(i =0;i<4;i++){
		*ptr = i;
		ensure_equals("QueueFull ",mot.m_Queuefull,0);
		ensure_equals("Queue Position ",mot.m_ImageQueue.curPos(),i);
		mot.ProcessImage(inp,&timest);
	}

	*ptr = 0xAAAAAAAA;
	ensure_equals("QueueFull ",mot.m_Queuefull,1);
	ensure_equals("Queue Position ",mot.m_ImageQueue.curPos(),i++);
	mot.ProcessImage(inp,&timest);

	*ptr = 0xBBBBBBBB;
	ensure_equals("Queue Position ",mot.m_ImageQueue.curPos(),0);
	mot.ProcessImage(inp,&timest);

	*ptr = 0xCCCCCCCC;
	ensure_equals("Queue Position ",mot.m_ImageQueue.curPos(),1);
	mot.ProcessImage(inp,&timest);
	}

	template<>
	template<>
	void testobject::test<5>() {

	set_test_name("Process test Level 5");
	cfg.setValue("GRIMotion.AbsDiffThreshold","50");
	cfg.setValue("GRIMotion.Debug","0");
	cfg.setValue("GRIMotion.Level","6");
	cfg.setValue("GRIMotion.QSize","5");
	cfg.setValue("GRIMotion.History","5");

	int w = 2592>>3;
	int h = 1944>>3;

	IplImage *inp = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,1);

	unsigned int *ptr = (unsigned int *)(inp->imageData);
	SimpleMotionDetection mot(&cfg,2592,1944);
	struct timeval timest;
	int i;
	for(i =0;i<4;i++){
		*ptr = i;
		ensure_equals("QueueFull ",mot.m_Queuefull,0);
		ensure_equals("Queue Position ",mot.m_ImageQueue.curPos(),i);
		mot.ProcessImage(inp,&timest);
	}

	*ptr = 0xAAAAAAAA;
	ensure_equals("QueueFull ",mot.m_Queuefull,1);
	ensure_equals("Queue Position ",mot.m_ImageQueue.curPos(),i++);
	mot.ProcessImage(inp,&timest);

	*ptr = 0xBBBBBBBB;
	ensure_equals("Queue Position ",mot.m_ImageQueue.curPos(),0);
	mot.ProcessImage(inp,&timest);

	*ptr = 0xCCCCCCCC;
	ensure_equals("Queue Position ",mot.m_ImageQueue.curPos(),1);
	mot.ProcessImage(inp,&timest);
	}
}

