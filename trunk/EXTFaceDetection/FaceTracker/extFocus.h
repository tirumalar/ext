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



using namespace cv;
using namespace std;


class extFocus{
	bool quit = false;
	char text[512], textI1[512], textI2[512], textI3[512], textI4[512], textI5[512];
	int numCount = 0;

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


	measureFocusOut measureFocus(Mat img, int camID, int width, int height, int x1, int y1, int x2, int y2);
	void BrightnessAndContrastAuto(const cv::Mat &src, cv::Mat &dst, float clipHistPercent=0);
	void adjustBrightnessContrast_clahe(Mat &src, Mat &dst, double clipLimit);
	void adjustBrightnessContrast_GammaCorrection(Mat &src, Mat &out, double gamma);
	preProcessingImgF preProcessingImg_focus(Mat &cropIm);
	focusMatric sobelBasedFocusMeasure(Mat &cropIm);
	focusMatric laplacianBasedFocusMeasure(Mat &grayImg);


public:
	void DoStartCmd_focus();
	bool camControlFocus(Mat &img,int camID);
	bool focusDebug = false;


};



/*

//focusing cameras
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



void DoStartCmd_focus();
//void RunCamFocus();
bool camControlFocus(Mat &img,int camID);
measureFocusOut measureFocus(Mat img, int camID, int width, int height, int x1, int y1, int x2, int y2);
void BrightnessAndContrastAuto(const cv::Mat &src, cv::Mat &dst, float clipHistPercent=0);
void adjustBrightnessContrast_clahe(Mat &src, Mat &dst, double clipLimit);
void adjustBrightnessContrast_GammaCorrection(Mat &src, Mat &out, double gamma);
preProcessingImgF preProcessingImg_focus(Mat &cropIm);
focusMatric sobelBasedFocusMeasure(Mat &cropIm);
focusMatric laplacianBasedFocusMeasure(Mat &grayImg);

*/


