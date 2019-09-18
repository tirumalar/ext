/*
 * geoMetricCalibration.h
 *
 *  Created on: Feb 20, 2019
 *      Author: Mo
 */

/*
 * geomtericCalibration.h
 *
 *  Created on: Feb 20, 2019
 *      Author: Mo
 */

#include <stdlib.h>
#include <string>
#include <iostream>
#include <math.h>
#include <cmath>
#include <vector>

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


class geomtericCalibration{


	int m_allLEDhighVoltEnable, m_calibVolt, m_calibCurrent, m_calibTrigger, m_calibLEDEnable, m_calibLEDMaxTime;
	int m_faceCamExposureTime, m_faceCamDigitalGain, m_faceCamDataPedestal;
	int m_AuxIrisCamExposureTime, m_AuxIrisCamDigitalGain, m_AuxIrisCamDataPedestal;
	int m_MainIrisCamExposureTime, m_MainIrisCamDigitalGain, m_MainIrisCamDataPedestal;

	int m_thresholdVal, rightCam, leftCam, faceCam;
	Mat smallImg, imgCopy, imgCopy1;

	int m_agc_val_cal;
	int numCount;

	struct twoMaxPeak{
		int peak1, peak2;
	};

	struct digPoints{
		cv::Point ptr1,ptr2;
	};

	twoMaxPeak calThreshold(int width, int height,unsigned char *dsty, int limit);


public:
	cv::Point pt1,pt2,pt3,pt4,pt5,pt6,pt7,pt8;
	bool m_calDebug, m_calTwoPoint;
	unsigned int Deviceid;

	int m_ImageWidth;
	int m_ImageHeight;

	bool m_bCalibImageSizeIs1280; // Flag used for knowing size of image used for calibration

	geomtericCalibration(char* filename);
	virtual ~geomtericCalibration();
	FileConfiguration geoConfig;

	void DoStartCmd_CamCal();
	std::vector<aruco::Marker> gridBooardMarker(Mat img, int cam, bool calDebug);
	vector<float> calibratedRect(std::vector<aruco::Marker> markerIris, std::vector<aruco::Marker> markerFace, int row, int col);
	Mat rotate90(Mat src);
	cv::Point processDigRectPoint1(vector<float> x);
	cv::Point processDigRectPoint2(vector<float> x);
	void calibDataWrite(cv::Rect auxRect, vector<float> rectRightAux,vector<float> rectLeftAux, vector<float> rectRightMain, vector<float> rectLeftMain);
	void brightnessAdjust(Mat outImg, int cam, bool calDebug);
	float AGC(int width, int height,unsigned char *dsty, int limit);

};

