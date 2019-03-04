/*
 * extFocus.h
 *
 *  Created on: Oct 22, 2018
 *      Author: Mo
 */

#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>
#include <cmath>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/photo/photo.hpp>

#include "portcom.h"

#include "FileConfiguration.h"
#include "Configuration.h"

#include <aruco.h>
#include <dictionary.h>


using namespace cv;
using namespace std;


class extFocus{

	char text[512], textI1[512], textI2[512], textI3[512], textI4[512], textI5[512], textI6[512], textI7[512], textI8[512];
	char textI9[512], textI10[512], textI11[512],textI12[512],textI13[512],textI14[512];

	int m_allLEDhighVoltEnable,m_IrisLEDVolt,m_IrisLEDcurrentSet,m_IrisLEDtrigger,m_IrisLEDEnable,m_IrisLEDmaxTime;
	int m_AuxIrisCamExposureTime,m_AuxIrisCamDigitalGain,m_AuxIrisCamDataPedestal;
	int m_MainIrisCamExposureTime,m_MainIrisCamDigitalGain,m_MainIrisCamDataPedestal;
	int m_FaceIrisCamExposureTime,m_FaceIrisCamDigitalGain,m_FaceIrisCamDataPedestal;

	//int m_focusIrisTargetRectWidth,	m_focusIrisTargetRectHeight, m_focusFaceTargetRectWidth, m_focusFaceTargetRectHeight;

	int m_focusIrisTargetRectWidth = 180,	m_focusIrisTargetRectHeight = 100, m_focusFaceTargetRectWidth = 65, m_focusFaceTargetRectHeight = 45;

	Mat DisImg;
	int numCount;
	int m_thresholdVal;
	cv::Point pt_title, pt_title_t;

	int m_lX1,m_lY1,m_lX2,m_lY2,m_RX1,m_RY1,m_RX2,m_RY2,m_FX1,m_FY1,m_FX2,m_FY2;
	std::vector<aruco::Marker> m_arucoID;
	Mat blurImg;
	struct focusMatric{
		float mean;
		float sigma;
	};

	struct measureFocusOut{
		float mean1,mean2,mean3,mean4,mean5;
		float sigma1,sigma2,sigma3,sigma4,sigma5;
		Rect ROI1,ROI2,ROI3,ROI4,ROI5;
		Mat brightAdjImg1,brightAdjImg2,brightAdjImg3,brightAdjImg4,brightAdjImg5;
	};

	struct preProcessingImgF{
		Mat dstImg;
		Mat brightnessAdjImg;
	};

	struct twoMaxPeak{
		int peak1, peak2;
	};

	Mat img,grayImg, cropImg,lookUpTable;
	measureFocusOut measureFocus(Mat img, int camID, int width, int height, int x1, int y1, int x2, int y2);
	void BrightnessAndContrastAuto(const cv::Mat &src, cv::Mat &dst, float clipHistPercent=0);
	void adjustBrightnessContrast_clahe(Mat &src, Mat &dst, double clipLimit);
	void adjustBrightnessContrast_GammaCorrection(Mat &src, Mat &out, double gamma);
	preProcessingImgF preProcessingImg_focus(Mat &cropIm);
	focusMatric sobelBasedFocusMeasure(Mat &cropIm);
	focusMatric laplacianBasedFocusMeasure(Mat &grayImg);
	Mat rotate90(Mat src);
	std::vector<aruco::Marker> detectMarkers(Mat img, int camID, cv::Point ptr1, cv::Point ptr2, int w, int h);
	twoMaxPeak calThreshold(int width, int height,unsigned char *dsty, int limit);
	void displayInstruction(int camID);

public:
	extFocus(char* filename);
	virtual ~extFocus();

	FileConfiguration extConfig;
	bool m_quit, m_focusDebug;

	int leftCam, rightCam, faceCam;

	void DoStartCmd_focus();
	bool camControlFocus(Mat &img,int camID);



};
