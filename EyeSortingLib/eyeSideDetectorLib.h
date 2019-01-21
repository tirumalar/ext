//eyeSideDetectorLib.h
//Created on 8th June 2016
//Created by Mohammad Shamim Imtiaz

//Header file "eyeSideDetectorLib" includes eyeSideDetection functions for different devices


#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>

using namespace std;
// using namespace cv;

//Declear function for WNC images
int eyeSideDetectionWnc(IplImage* inputImg);

//Declear function for NANO images
int eyeSideDetectionNano(IplImage* inputImg);

//Declear function for MYRIS images
int eyeSideDetectionMyris(IplImage* inputImg);
