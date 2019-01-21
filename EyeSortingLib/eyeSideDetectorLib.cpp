//eyeSideDetectorLib.cpp
//Created on 8th June 2016
//Created by Mohammad Shamim Imtiaz


//eyeSideDetectorLib contain all the necessary functions to run eyeSideDetction (function)
// Three eyeSideDetction function can be found here:
//1. eyeSideDetctionWnc -> For WNC device
//2. eyeSideDetctionNano -> For NANO device
//3. eyeSideDetctionMyris -> For MYRIS device





#include <stdio.h>
#include <string.h>
#include <iostream>
#include <fstream> 
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv/highgui.h>
#include <bitset>
#include <math.h>  
#include <stdint.h> 
#include <ctime>
#include "eyeSideDetectorLib.h"
#include <iostream>
#include <vector>


using namespace std;
// using namespace cv;


//structure eyeSideDetector for saving multiple out put of finctions
struct eyeSideDetector {
	int max1stBin;
	int max2ndBin;
	vector<int> posR;
	vector<int> posL;
};









//createHistigramImage is a function of eyeSideDetector structure.
//Input: gray image, Hist iamges
//output maximum occurance that took place between 0 to 100
// and maximum occurance that took place between 101 to 255
struct eyeSideDetector createHistigramImage(IplImage* grayImg)
{
	struct eyeSideDetector b;
	
	CvHistogram *hist;
	int histSize = 256;
	float range[] = {0,256};
	float* ranges[] = {range};

	// create array to hold histogram values
	hist = cvCreateHist(1,&histSize, CV_HIST_ARRAY, ranges,1);
	
	// calculate histogram values
	cvCalcHist(&grayImg, hist,0,NULL);
	int maxOne = 0;

	//find max occurance between 0 to 100
	for(int i = 0; i < 101; i++)
	{
		if (cvGetReal1D(hist->bins,i) > maxOne)
		{
			maxOne = cvGetReal1D(hist->bins,i);
			b.max1stBin = i;
		}
		else {}
	}


	int maxSec = 0;
	//find max occurance between 101 to 255
	for(int i = 101; i < histSize; i++)
	{

		if (cvGetReal1D(hist->bins,i) > maxSec)
		{
			maxSec = cvGetReal1D(hist->bins,i);
			b.max2ndBin = i;

		}
		else {}

	}

	//Release memory
	cvReleaseHist(&hist);
	return b;

}









//threshCalDifStep function use two max occurance peak from createHistigramImage function and
// use it with the information of stwp size to define the threshold level and then use that threshold level
//to create a binary image
//Input: maxPeak1 and maxPeak2 from createHistigramImage function, step from filter, gray image as input image
//and output image
void threshCalDifStep(int maxPeak1, int maxPeak2, float step, IplImage *grayImg, IplImage *outputImg)
{
	float thresh;
	int maxVal = 255;
	
	//if filter step is less greater than zero
	if (step > 0) 
	{
		//measure threshold values using division
		thresh = (maxPeak1 - maxPeak2)/step;

		//Thresold image
		cvThreshold(grayImg, outputImg, thresh, maxVal, CV_THRESH_BINARY_INV);
	}
	else if (step == 0)
	{
		//measure threshold values
		thresh = (maxPeak1 - maxPeak2);

		//Thresold image
		cvThreshold(grayImg, outputImg, thresh, maxVal, CV_THRESH_BINARY_INV);
	}
	else if (step < 0)
	{
		//measure threshold values using multiplication
		thresh = (maxPeak1 - maxPeak2)*(-1*step);

		//Thresold image
		cvThreshold(grayImg, outputImg, thresh, maxVal, CV_THRESH_BINARY_INV);
	}
	

}








//densityCal is used to calulate the density of white pixel
//Input: binary image
double densityCal(IplImage *inputImg)
{
	//count non zero values
	double noOfWhitePix = 0; // countNonZero(inputImg);
	
	//Count total pix
	double totalPix = inputImg->height * inputImg->width;
	
	//Measure the ratio
	double density = noOfWhitePix/totalPix;

	return density;

}








// coutMechanism function will move in both left and right direction at certain center point
//it will output two vector saying that how many time it hits white pixel in left and right direction

struct eyeSideDetector coutMechanism(IplImage* thresholdImgX)
{
	struct eyeSideDetector posX;
	IplImage* dilateImg = cvCreateImage(cvGetSize(thresholdImgX),8,1);
	
	cvDilate(thresholdImgX, dilateImg, NULL, 1);
	
	int row = thresholdImgX->height;
	int col = thresholdImgX->width;

	int centerX = thresholdImgX->width/2;
	int centerY = thresholdImgX->height;	
	
	int endPnt, strPnt01, strPnt02;

	//if the row and col of image is neither 480 nor 640, 
	//the sartpoint and end pint of moving direction will be
	if (row !=480 && col!= 640)
	{
		endPnt = col - (centerX+120);

		strPnt01 = centerX + 120;
		strPnt02 = centerX - 120;
	}
	//if the row and col of image is 480 and 640, 
	//the sartpoint and end pint of moving direction will be
	else
	{
		endPnt = col - (centerX+200);

		strPnt01 = centerX + 200;
		strPnt02 = centerX - 200;
	}	
	

	int pixRight, pixLeft;	
	
	//move to right direction and check white pix
	for (int m = 0; m < centerY; m = m + 20)
	{
		//uchar* ptr1 = (uchar*) ( cloneThreshImgX->imageData + m * cloneThreshImgX->widthStep );
		
		for (int n = 0; n < endPnt; n++)
		{
			//assign white value for display purpose
			//ptr1[(strPnt01 + n)] = 255;

			pixRight = CV_IMAGE_ELEM(dilateImg, unsigned char, m, strPnt01 + n);
			

			//check white pix
			if (pixRight > 0)
			{
				posX.posR.push_back(strPnt01 + n);
				break;
			}
			

		}
	}	
	
	
	//move to left direction and check white pix
	for (int m = 0; m < centerY; m = m + 20)
	{
		//uchar* ptr1 = (uchar*) ( cloneThreshImgX->imageData + m * cloneThreshImgX->widthStep );

		for (int n = 0; n < endPnt; n++)
		{
			//assign white value for display purpose
			//ptr1[strPnt02 - n] = 255;
			pixLeft = CV_IMAGE_ELEM(dilateImg, unsigned char, m, strPnt02 - n);

			//check white pix
			if (pixLeft > 0)
			{
				posX.posL.push_back(strPnt02 - n);
				break;
			}


		}
	}

	//Release memory
	cvReleaseImage(&dilateImg);
	return posX;
	
}







// This function will work on WNC images
int eyeSideDetectionWnc(IplImage* inputImg)
{
	IplImage *binaryImg = cvCreateImage(cvGetSize(inputImg),IPL_DEPTH_8U,1);

	int row, col, channels, totalPix;
	
	//binary conversion
	cvThreshold(inputImg, binaryImg, 0, 255, CV_THRESH_BINARY);
	
	IplImage *imgTemp = cvCloneImage(binaryImg);	//Create a clone of the binary image


	//initializing contour measuring stage
	CvSeq* contours; //hold teh pointer to a contour in the in the memory block
	CvSeq* result;	//hold sequence of points of a contour
	CvMemStorage *storage = cvCreateMemStorage(0); //Storage area of all contours	
	
	//Finb all contours in the image
	cvFindContours(imgTemp, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
	cvPoint(0,0);
	double maxArea = 0.0;
	CvPoint *pt[4];

	

	//iterating through each contours
	while(contours)
	{
		result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0);

		//Find contours with four vector points
		if (result->total == 4)
		{
			double area = 0; // abs(cvContourArea (contours, CV_WHOLE_SEQ));

			//contours with four vector points will have the maximum area of the image
			if (area > maxArea)
			{
				maxArea = area;
				
				//iterating through each point
				for(int i = 0; i < 4; i++)
				{
					pt[i] = (CvPoint*)cvGetSeqElem(result,i);
					
				}

			}

		}
		//move to next contour
		contours = contours->h_next;
	}	


	//measure width and height
	int width = pt[2]->x - pt[0]->x;
	int height = pt[2]->y - pt[0]->y;

	//crop the rect image
	IplImage *inputImgX = cvCloneImage(inputImg);

	CvRect roi = {pt[0]->x, pt[0]->y, width, height};
	// Rect roi(pt[0]->x, pt[0]->y, width, height);
	cvSetImageROI(inputImgX, roi);
	CvSize size;
	size.height = roi.height;
	size.width = roi.width;	


	IplImage *imageCropped = cvCloneImage(inputImgX);;

	
	// different attributes of cropped image
	row = imageCropped->height;
	col = imageCropped->width;
	channels = imageCropped->nChannels;
	totalPix = row * col;	
	

	int maxBin1, maxBin2;	
	
	
	//createHistigramImage is a function whihc will meeausre the histogram of input image
	// and devide the max peak in 0 tp 100 and 100 to 255. Later on, it will measure the 
	//maximum peak into those ranges. SO there will be two output: peak1 (from 0 to 100)
	//and peak2 (from 101 to 255)
	struct eyeSideDetector maxBins = createHistigramImage(imageCropped);
	

	// define filter steps
	vector<float> vec1;
	
	
	//from 1.06 to 1.09
	for (float i = 1.06; i <=1.09 ; i=i+0.01)
	{
		//cout << i << endl;
		vec1.push_back(i);
	}

	//from 1.1 to 1.9
	for (float i = 1.1; i < 2 ; i=i+0.1)
	{
		//cout << i << endl;
		vec1.push_back(i);
	}
	
	//from 2 to 15.5
	for (float i = 2; i < 16 ; i = i+ 0.5)
	{
		//cout << i << endl;
		vec1.push_back(i);
	}

	//from 17 to 19
	for (float i = 17; i < 20 ; i++)
	{
		//cout << i << endl;
		vec1.push_back(i);
	}

	//from 21 to 25
	for (float i = 21; i < 27 ; i = i+2)
	{
		//cout << i << endl;
		vec1.push_back(i);
	}	
	
	
	


	
	double threshStep;
	double whitePixDensity;
	int errorLimit = 3;

	int check  = 0;	
	

	int sideIndicator;
		//If the two peak difference falls between 150 to 255
	if ((maxBins.max2ndBin - maxBins.max1stBin) >= 150)
	{
		//upperbound of density
		double densityUpThresh = 0.32;
		
		//iterate through each filter step
		for(int i= 0; i < vec1.size(); i++)
		{
			//take 1st filter step
			threshStep = vec1[i];
			//define threshold image
			IplImage* thresholdImg = cvCreateImage(cvGetSize(imageCropped),8,1);
			//create binary image using threshCalDifStep function
			threshCalDifStep(maxBins.max2ndBin, maxBins.max1stBin, threshStep, imageCropped,thresholdImg);

			//check teh white pixel density using densityCal function
			whitePixDensity =  densityCal(thresholdImg); 
			

			// check whther the white pixel density is lower than the upper bound of density threshold
			if (whitePixDensity <= densityUpThresh)
			{
				// use coutMechanism function here, It will give you two output: 1. how many time it hit white pixel on the 
				// left side of the image and 2. how many time it hit white pixel on the right side of the image
				struct eyeSideDetector PosXCount = coutMechanism(thresholdImg);


				//absolute difference between the two putput of coutMechanism function
				int absDiffPos = labs(PosXCount.posR.size() - PosXCount.posL.size());


				/*
				// save thresholded image
				sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC4.jpg",i);
				cvSaveImage(buffer, cloneThreshImg);
				*/


				//if the length of the vector that contains the number of times it hit white pixel on the 
				// left side is less than the lenght of vector of number of times it hit white pixel on the 
				// right side and their absolute differecne is greater than the error limit than it is a left eye!!!
				if ((PosXCount.posR.size() > PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Right image indocator
					sideIndicator = 0;

					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();

					//rightImg = rightImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);	
					*/


					break;
				}

				// Vise versa
				else if ((PosXCount.posR.size() < PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Left image indicator
					sideIndicator = 1;

					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();


					//leftImg = leftImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}

				//otherwise
				else
				{
					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
				}

			}
			//Release memory
			cvReleaseImage(&thresholdImg);
				
		}


	}
	
		//If the two peak difference falls between 0 to 149
	else

	{
		
		//upperbound of density
		double densityUpThresh = 0.32;

		//iterate through each filter step
		for(int i= 0; i < vec1.size(); i++)
		{
			//take 1st filter step
			threshStep = vec1[i];

			//define threshold image
			IplImage* thresholdImg = cvCreateImage(cvGetSize(imageCropped),8,1);
			//create binary image using threshCalDifStep function
			threshCalDifStep(maxBins.max2ndBin, maxBins.max1stBin, threshStep, imageCropped,thresholdImg); 


			//check teh white pixel density using densityCal function
			whitePixDensity =  densityCal(thresholdImg); 

			
			// check whther the white pixel density is lower than the upper bound of density threshold
			if (whitePixDensity <= densityUpThresh)
			{
				// use coutMechanism function here
				struct eyeSideDetector PosXCount = coutMechanism(thresholdImg);
				
				//absolute difference between the two putput of coutMechanism function
				int absDiffPos = labs(PosXCount.posR.size() - PosXCount.posL.size());

				/*
				// save thresholded image
				sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC4.jpg",i);
				cvSaveImage(buffer, cloneThreshImg);
				*/




				//if the length of the vector that contains the number of times it hit white pixel on the 
				// left side is less than the lenght of vector of number of times it hit white pixel on the 
				// right side and their absolute differecne is greater than the error limit than it is a left eye!!!
				if ((PosXCount.posR.size() > PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Right image indicator
					sideIndicator = 0;

					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();

					
					//rightImg = rightImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}

				//Vice versa
				else if ((PosXCount.posR.size() < PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Left image indicator
					sideIndicator = 1;

					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
					//leftImg = leftImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}
				
				//otherwise
				else
				{
					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
										
				}


			}

			//Release memory
			cvReleaseImage(&thresholdImg);	
		}

	}
	


	//Release memory
	vec1.clear();
	
	cvReleaseMemStorage(&storage);

	cvReleaseImage(&imgTemp);
	cvReleaseImage(&binaryImg);
	cvReleaseImage(&imageCropped);
	cvReleaseImage(&inputImgX);

	
	
	
	return sideIndicator;
	
	
	
}








// This function will work on NANO images
int eyeSideDetectionNano(IplImage* inputImg)
{
	IplImage *binaryImg = cvCreateImage(cvGetSize(inputImg),IPL_DEPTH_8U,1);

	int row, col, channels, totalPix;
	
	//binary conversion
	cvThreshold(inputImg, binaryImg, 0, 255, CV_THRESH_BINARY);
	
	IplImage *imgTemp = cvCloneImage(binaryImg);	//Create a clone of the binary image


	//initializing contour measuring stage
	CvSeq* contours; //hold teh pointer to a contour in the in the memory block
	CvSeq* result;	//hold sequence of points of a contour
	CvMemStorage *storage = cvCreateMemStorage(0); //Storage area of all contours	
	
	//Finb all contours in the image
	cvFindContours(imgTemp, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
	cvPoint(0,0);
	double maxArea = 0.0;
	CvPoint *pt[4];

	

	//iterating through each contours
	while(contours)
	{
		result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0);

		//Find contours with four vector points
		if (result->total == 4)
		{
			double area = 0; //abs(cvContourArea (contours, CV_WHOLE_SEQ));

			//contours with four vector points will have the maximum area of the image
			if (area > maxArea)
			{
				maxArea = area;
				
				//iterating through each point
				for(int i = 0; i < 4; i++)
				{
					pt[i] = (CvPoint*)cvGetSeqElem(result,i);
					
				}

			}

		}
		//move to next contour
		contours = contours->h_next;
	}	


	//measure width and height
	int width = pt[2]->x - pt[0]->x;
	int height = pt[2]->y - pt[0]->y;

	//crop the rect image
	IplImage *inputImgX = cvCloneImage(inputImg);

	CvRect roi = {pt[0]->x, pt[0]->y, width, height};
	// Rect roi(pt[0]->x, pt[0]->y, width, height);
	cvSetImageROI(inputImgX, roi);
	CvSize size;
	size.height = roi.height;
	size.width = roi.width;	


	IplImage *imageCropped = cvCloneImage(inputImgX);;

	
	// different attributes of cropped image
	row = imageCropped->height;
	col = imageCropped->width;
	channels = imageCropped->nChannels;
	totalPix = row * col;	
	

	int maxBin1, maxBin2;	
	
	
	//createHistigramImage is a function whihc will meeausre the histogram of input image
	// and devide the max peak in 0 tp 100 and 100 to 255. Later on, it will measure the 
	//maximum peak into those ranges. SO there will be two output: peak1 (from 0 to 100)
	//and peak2 (from 101 to 255)
	struct eyeSideDetector maxBins = createHistigramImage(imageCropped);
	

	//for (float i = 0; i < vec2.size() ; i ++)
	//{
	//	cout << vec2[i] << endl;
	//}	


	
	double threshStep;
	double whitePixDensity;
	int errorLimit = 3;

	int check  = 0;	
	

	int sideIndicator;
	//If the two peak difference falls between 150 to 255

	if ((maxBins.max2ndBin - maxBins.max1stBin) >= 150)
	{
		//upperbound of density
		double densityUpThresh = 0.56;
		
		//double densityUpThresh = 0.56;
		// define filter steps 1
		vector<float> vec1;
	
	
		//from -1.5 to -1.05
		for (float i = -1.5; i <= -1.05 ; i=i+0.05)
		{
			//cout << i << endl;
			vec1.push_back(i);
		}

		//place 0
		vec1.push_back(0.0);

		//from 1.06 to 1.09
		for (float i = 1.06; i <= 1.09 ; i=i+0.01)
		{
			//cout << i << endl;
			vec1.push_back(i);
		}
	
		//from 1.1 to 2.1
		for (float i = 1.1; i <= 2.1 ; i = i+ 0.1)
		{
			//cout << i << endl;
			vec1.push_back(i);
		}

		//from 2.5 to 6
		for (float i = 2.5; i <= 6 ; i = i + 0.5)
		{
			//cout << i << endl;
			vec1.push_back(i);
		}

		//iterate through each filter step
		for(int i= 0; i < vec1.size(); i++)
		{
			//take 1st filter step
			threshStep = vec1[i];
			//define threshold image
			IplImage* thresholdImg = cvCreateImage(cvGetSize(imageCropped),8,1);
			//create binary image using threshCalDifStep function
			threshCalDifStep(maxBins.max2ndBin, maxBins.max1stBin, threshStep, imageCropped,thresholdImg);

			//check teh white pixel density using densityCal function
			whitePixDensity =  densityCal(thresholdImg); 
			

			// check whther the white pixel density is lower than the upper bound of density threshold
			if (whitePixDensity <= densityUpThresh)
			{
				// use coutMechanism function here, It will give you two output: 1. how many time it hit white pixel on the 
				// left side of the image and 2. how many time it hit white pixel on the right side of the image
				struct eyeSideDetector PosXCount = coutMechanism(thresholdImg);


				//absolute difference between the two putput of coutMechanism function
				int absDiffPos = labs(PosXCount.posR.size() - PosXCount.posL.size());


				/*
				// save thresholded image
				sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC4.jpg",i);
				cvSaveImage(buffer, cloneThreshImg);
				*/


				//if the length of the vector that contains the number of times it hit white pixel on the 
				// left side is less than the lenght of vector of number of times it hit white pixel on the 
				// right side and their absolute differecne is greater than the error limit than it is a left eye!!!
				if ((PosXCount.posR.size() > PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Right image indocator
					sideIndicator = 0;

					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();

					//rightImg = rightImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);	
					*/


					break;
				}

				// Vise versa
				else if ((PosXCount.posR.size() < PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Left image indicator
					sideIndicator = 1;


					//cout<< "Ed value:" << maxBins.max2ndBin - maxBins.max1stBin << endl;
					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();


					//leftImg = leftImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}

				//otherwise
				else
				{
					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
				}

			}
			//Release memory
			cvReleaseImage(&thresholdImg);
				
		}
	
	//Release memory
	vec1.clear();
	}
	



	//If the two peak difference falls between 150 to 255
	else if ((maxBins.max2ndBin - maxBins.max1stBin) >= 50 && (maxBins.max2ndBin - maxBins.max1stBin) < 150)
	{
		//upperbound of density
		double densityUpThresh = 0.34;
		
		// define filter steps 2
		vector<float> vec2;
	
	
		//from -1.6 to -1.05
		for (float i = -1.6; i <= -1.05 ; i=i+0.05)
		{
			//cout << i << endl;
			vec2.push_back(i);
		}

		//place 0
		vec2.push_back(0.0);

		//from 1.06 to 1.09
		for (float i = 1.06; i <= 1.09 ; i=i+0.01)
		{
			//cout << i << endl;
			vec2.push_back(i);
		}
	
		//from 1.1 to 2.1
		for (float i = 1.1; i <= 2.1 ; i = i+ 0.1)
		{
			//cout << i << endl;
			vec2.push_back(i);
		}

		//from 2.5 to 5
		for (float i = 2.5; i <= 5 ; i = i + 0.5)
		{
			//cout << i << endl;
			vec2.push_back(i);
		}
	



		//iterate through each filter step
		for(int i= 0; i < vec2.size(); i++)
		{
			//take 1st filter step
			threshStep = vec2[i];
			//define threshold image
			IplImage* thresholdImg = cvCreateImage(cvGetSize(imageCropped),8,1);
			//create binary image using threshCalDifStep function
			threshCalDifStep(maxBins.max2ndBin, maxBins.max1stBin, threshStep, imageCropped,thresholdImg);

			//check teh white pixel density using densityCal function
			whitePixDensity =  densityCal(thresholdImg); 
			

			// check whther the white pixel density is lower than the upper bound of density threshold
			if (whitePixDensity <= densityUpThresh)
			{
				// use coutMechanism function here, It will give you two output: 1. how many time it hit white pixel on the 
				// left side of the image and 2. how many time it hit white pixel on the right side of the image
				struct eyeSideDetector PosXCount = coutMechanism(thresholdImg);


				//absolute difference between the two putput of coutMechanism function
				int absDiffPos = labs(PosXCount.posR.size() - PosXCount.posL.size());


				/*
				// save thresholded image
				sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC4.jpg",i);
				cvSaveImage(buffer, cloneThreshImg);
				*/


				//if the length of the vector that contains the number of times it hit white pixel on the 
				// left side is less than the lenght of vector of number of times it hit white pixel on the 
				// right side and their absolute differecne is greater than the error limit than it is a left eye!!!
				if ((PosXCount.posR.size() > PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Right image indocator
					sideIndicator = 0;

					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();

					//rightImg = rightImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);	
					*/


					break;
				}

				// Vise versa
				else if ((PosXCount.posR.size() < PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Left image indicator
					sideIndicator = 1;


					//cout<< "Ed value:" << maxBins.max2ndBin - maxBins.max1stBin << endl;
					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();


					//leftImg = leftImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}

				//otherwise
				else
				{
					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
				}

			}
			//Release memory
			cvReleaseImage(&thresholdImg);
				
		}

	//Release memory
	vec2.clear();
	}



	//If the two peak difference falls between 0 to 149
	else

	{
		
		//upperbound of density
		double densityUpThresh = 0.2;


		// define filter steps 3
		vector<float> vec3;
	
		//from -1.5 to -1.05
		for (float i = -60; i <= -2 ; i++)
		{
			//cout << i << endl;
			vec3.push_back(i);
		}


		//from -1.5 to -1.05
		for (float i = -1.9; i <= -1.05 ; i=i+0.05)
		{
			//cout << i << endl;
			vec3.push_back(i);
		}

		//place 0
		vec3.push_back(0.0);


		//iterate through each filter step
		for(int i= 0; i < vec3.size(); i++)
		{
			//take 1st filter step
			threshStep = vec3[i];

			//define threshold image
			IplImage* thresholdImg = cvCreateImage(cvGetSize(imageCropped),8,1);
			//create binary image using threshCalDifStep function
			threshCalDifStep(maxBins.max2ndBin, maxBins.max1stBin, threshStep, imageCropped,thresholdImg); 


			//check teh white pixel density using densityCal function
			whitePixDensity =  densityCal(thresholdImg); 

			
			// check whther the white pixel density is lower than the upper bound of density threshold
			if (whitePixDensity <= densityUpThresh)
			{
				// use coutMechanism function here
				struct eyeSideDetector PosXCount = coutMechanism(thresholdImg);
				
				//absolute difference between the two putput of coutMechanism function
				int absDiffPos = labs(PosXCount.posR.size() - PosXCount.posL.size());

				/*
				// save thresholded image
				sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC4.jpg",i);
				cvSaveImage(buffer, cloneThreshImg);
				*/




				//if the length of the vector that contains the number of times it hit white pixel on the 
				// left side is less than the lenght of vector of number of times it hit white pixel on the 
				// right side and their absolute differecne is greater than the error limit than it is a left eye!!!
				if ((PosXCount.posR.size() > PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Right image indicator
					sideIndicator = 0;

					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();

					
					//rightImg = rightImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}

				//Vice versa
				else if ((PosXCount.posR.size() < PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Left image indicator
					sideIndicator = 1;


					//cout<< "Ed value:" << maxBins.max2ndBin - maxBins.max1stBin << endl;
					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
					//leftImg = leftImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}
				
				//otherwise
				else
				{
					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
										
				}


			}

			//Release memory
			cvReleaseImage(&thresholdImg);	
		}

	//Release memory
	vec3.clear();
	}
	




	//Release memory
	cvReleaseMemStorage(&storage);

	cvReleaseImage(&imgTemp);
	cvReleaseImage(&binaryImg);
	cvReleaseImage(&imageCropped);
	cvReleaseImage(&inputImgX);

	
	
	
	return sideIndicator;
	
	
	
}










// This function will work on Myris images
int eyeSideDetectionMyris(IplImage* inputImg)
{
	IplImage *binaryImg = cvCreateImage(cvGetSize(inputImg),IPL_DEPTH_8U,1);

	int row, col, channels, totalPix;
	
	//binary conversion
	cvThreshold(inputImg, binaryImg, 0, 255, CV_THRESH_BINARY);
	
	IplImage *imgTemp = cvCloneImage(binaryImg);	//Create a clone of the binary image


	//initializing contour measuring stage
	CvSeq* contours; //hold teh pointer to a contour in the in the memory block
	CvSeq* result;	//hold sequence of points of a contour
	CvMemStorage *storage = cvCreateMemStorage(0); //Storage area of all contours	
	
	//Finb all contours in the image
	cvFindContours(imgTemp, storage, &contours, sizeof(CvContour), CV_RETR_TREE, CV_CHAIN_APPROX_NONE);
	cvPoint(0,0);
	double maxArea = 0.0;
	CvPoint *pt[4];
	

	//iterating through each contours
	while(contours)
	{
		result = cvApproxPoly(contours, sizeof(CvContour), storage, CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0);

		//Find contours with four vector points
		if (result->total > 3)
		{
			double area = 0; //abs(cvContourArea (contours, CV_WHOLE_SEQ));

			//contours with four vector points will have the maximum area of the image
			if (area > maxArea)
			{
				maxArea = area;
				
				//iterating through each point
				for(int i = 0; i < 4; i++)
				{
					pt[i] = (CvPoint*)cvGetSeqElem(result,i);
					
				}

			}

		}
		//move to next contour
		contours = contours->h_next;
	}	


	//measure width and height
	int width = pt[2]->x - pt[0]->x;
	int height = pt[2]->y - pt[0]->y;

	//crop the rect image
	IplImage *inputImgX = cvCloneImage(inputImg);


	if ( width <= 0 || height <= 0)
	{
		
	}
	else
	{
		CvRect roi = {pt[0]->x, pt[0]->y, width, height};
		// Rect roi(pt[0]->x, pt[0]->y, width, height);
		cvSetImageROI(inputImgX, roi);
		CvSize size;
		size.height = roi.height;
		size.width = roi.width;	
	}



	IplImage *imageCropped = cvCloneImage(inputImgX);;

	
	// different attributes of cropped image
	row = imageCropped->height;
	col = imageCropped->width;
	channels = imageCropped->nChannels;
	totalPix = row * col;	
	

	int maxBin1, maxBin2;	
	
	
	//createHistigramImage is a function whihc will meeausre the histogram of input image
	// and devide the max peak in 0 tp 100 and 100 to 255. Later on, it will measure the 
	//maximum peak into those ranges. SO there will be two output: peak1 (from 0 to 100)
	//and peak2 (from 101 to 255)
	struct eyeSideDetector maxBins = createHistigramImage(imageCropped);
	

	//for (float i = 0; i < vec3.size() ; i ++)
	//{
	//	cout << vec3[i] << endl;
	//}	


	
	double threshStep;
	double whitePixDensity;
	int errorLimit = 3;

	int check  = 0;	
	

	int sideIndicator;
	//If the two peak difference falls between 150 to 255

	if ((maxBins.max2ndBin - maxBins.max1stBin) >= 225)
	{
		//upperbound of density
		double densityUpThresh = 0.85;
		
		//double densityUpThresh = 0.56;
		// define filter steps 1
		vector<float> vec1;
	
	
		//from 2 to 15.5
		for (float i = 2; i <= 15.5; i = i + 0.5)
		{
			//cout << i << endl;
			vec1.push_back(i);
		}


		//from 17 to 19
		for (float i = 17; i <= 19 ; i++)
		{
			//cout << i << endl;
			vec1.push_back(i);
		}
	


		//from 21 to 47
		for (float i = 21; i <= 47 ; i = i+ 2)
		{
			//cout << i << endl;
			vec1.push_back(i);
		}



		//from 53 to 59
		for (float i = 53; i <= 59 ; i = i + 6)
		{
			//cout << i << endl;
			vec1.push_back(i);
		}



		//from 63 to 69
		for (float i = 63; i <= 69 ; i = i + 6)
		{
			//cout << i << endl;
			vec1.push_back(i);
		}



		//from 73 to 79
		for (float i = 73; i <= 79 ; i = i + 6)
		{
			//cout << i << endl;
			vec1.push_back(i);
		}



		//from 83 to 89
		for (float i = 83; i <= 89 ; i = i + 6)
		{
			//cout << i << endl;
			vec1.push_back(i);
		}



		//from 93 to 99
		for (float i = 93; i <= 99 ; i = i + 6)
		{
			//cout << i << endl;
			vec1.push_back(i);
		}


		//iterate through each filter step
		for(int i= 0; i < vec1.size(); i++)
		{
			//take 1st filter step
			threshStep = vec1[i];
			//define threshold image
			IplImage* thresholdImg = cvCreateImage(cvGetSize(imageCropped),8,1);
			//create binary image using threshCalDifStep function
			threshCalDifStep(maxBins.max2ndBin, maxBins.max1stBin, threshStep, imageCropped,thresholdImg);

			//check teh white pixel density using densityCal function
			whitePixDensity =  densityCal(thresholdImg); 
			

			// check whther the white pixel density is lower than the upper bound of density threshold
			if (whitePixDensity <= densityUpThresh)
			{
				// use coutMechanism function here, It will give you two output: 1. how many time it hit white pixel on the 
				// left side of the image and 2. how many time it hit white pixel on the right side of the image
				struct eyeSideDetector PosXCount = coutMechanism(thresholdImg);


				//absolute difference between the two putput of coutMechanism function
				int absDiffPos = labs(PosXCount.posR.size() - PosXCount.posL.size());


				/*
				// save thresholded image
				sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC4.jpg",i);
				cvSaveImage(buffer, cloneThreshImg);
				*/


				//if the length of the vector that contains the number of times it hit white pixel on the 
				// left side is less than the lenght of vector of number of times it hit white pixel on the 
				// right side and their absolute differecne is greater than the error limit than it is a left eye!!!
				if ((PosXCount.posR.size() > PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Right image indocator
					sideIndicator = 0;

					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();

					//rightImg = rightImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);	
					*/


					break;
				}

				// Vise versa
				else if ((PosXCount.posR.size() < PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Left image indicator
					sideIndicator = 1;


					//cout << "HIst step: " << (maxBins.max2ndBin - maxBins.max1stBin) << endl;
					//cout << "              " << endl;

					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();


					//leftImg = leftImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}

				//otherwise
				else
				{
					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
				}

			}
			//Release memory
			cvReleaseImage(&thresholdImg);
				
		}
	
	//Release memory
	vec1.clear();
	}
	



	//If the two peak difference falls between 150 to 255
	else if ((maxBins.max2ndBin - maxBins.max1stBin) >= 150 && (maxBins.max2ndBin - maxBins.max1stBin) < 225)
	{
		//upperbound of density
		double densityUpThresh = 0.85;
		
		// define filter steps 2
		vector<float> vec2;
	
	
		//from 1.5 to 1.9
		for (float i = 1.5; i <= 1.9 ; i= i + 0.1)
		{
			//cout << i << endl;
			vec2.push_back(i);
		}


		//from 2 to 15
		for (float i = 2; i <= 15 ; i = i + 0.5)
		{
			//cout << i << endl;
			vec2.push_back(i);
		}
	



		//iterate through each filter step
		for(int i= 0; i < vec2.size(); i++)
		{
			//take 1st filter step
			threshStep = vec2[i];
			//define threshold image
			IplImage* thresholdImg = cvCreateImage(cvGetSize(imageCropped),8,1);
			//create binary image using threshCalDifStep function
			threshCalDifStep(maxBins.max2ndBin, maxBins.max1stBin, threshStep, imageCropped,thresholdImg);

			//check teh white pixel density using densityCal function
			whitePixDensity =  densityCal(thresholdImg); 
			

			// check whther the white pixel density is lower than the upper bound of density threshold
			if (whitePixDensity <= densityUpThresh)
			{
				// use coutMechanism function here, It will give you two output: 1. how many time it hit white pixel on the 
				// left side of the image and 2. how many time it hit white pixel on the right side of the image
				struct eyeSideDetector PosXCount = coutMechanism(thresholdImg);


				//absolute difference between the two putput of coutMechanism function
				int absDiffPos = labs(PosXCount.posR.size() - PosXCount.posL.size());


				/*
				// save thresholded image
				sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC4.jpg",i);
				cvSaveImage(buffer, cloneThreshImg);
				*/


				//if the length of the vector that contains the number of times it hit white pixel on the 
				// left side is less than the lenght of vector of number of times it hit white pixel on the 
				// right side and their absolute differecne is greater than the error limit than it is a left eye!!!
				if ((PosXCount.posR.size() > PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Right image indocator
					sideIndicator = 0;

					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();

					//rightImg = rightImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);	
					*/


					break;
				}

				// Vise versa
				else if ((PosXCount.posR.size() < PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Left image indicator
					sideIndicator = 1;


					//cout << "HIst step: " << (maxBins.max2ndBin - maxBins.max1stBin) << endl;
					//cout << "              " << endl;


					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();


					//leftImg = leftImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}

				//otherwise
				else
				{
					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
				}

			}
			//Release memory
			cvReleaseImage(&thresholdImg);
				
		}

	//Release memory
	vec2.clear();
	}



	//If the two peak difference falls between 0 to 149
	else if ((maxBins.max2ndBin - maxBins.max1stBin) >= 50 && (maxBins.max2ndBin - maxBins.max1stBin) < 150)

	{
		
		//upperbound of density
		double densityUpThresh = 0.85;


		// define filter steps 3
		vector<float> vec3;
	
		//from -1.2 to -1.05
		for (float i = -1.2; i <= -1.05 ; i = i + 0.05)
		{
			//cout << i << endl;
			vec3.push_back(i);
		}

		//Insert zero
		vec3.push_back(0);


		//from -1.06 to -1.09
		for (float i = 1.06; i <= 1.09 ; i= i + 0.01)
		{
			//cout << i << endl;
			vec3.push_back(i);
		}



		//from 1.1 to 2
		for (float i = 1.1; i <= 2.1 ; i= i + 0.1)
		{
			//cout << i << endl;
			vec3.push_back(i);
		}


		//from 2.5 to 15
		for (float i = 2.5; i <= 15 ; i= i + 0.5)
		{
			//cout << i << endl;
			vec3.push_back(i);
		}



		//iterate through each filter step
		for(int i= 0; i < vec3.size(); i++)
		{
			//take 1st filter step
			threshStep = vec3[i];

			//define threshold image
			IplImage* thresholdImg = cvCreateImage(cvGetSize(imageCropped),8,1);
			//create binary image using threshCalDifStep function
			threshCalDifStep(maxBins.max2ndBin, maxBins.max1stBin, threshStep, imageCropped,thresholdImg); 


			//check teh white pixel density using densityCal function
			whitePixDensity =  densityCal(thresholdImg); 

			
			// check whther the white pixel density is lower than the upper bound of density threshold
			if (whitePixDensity <= densityUpThresh)
			{
				// use coutMechanism function here
				struct eyeSideDetector PosXCount = coutMechanism(thresholdImg);
				
				//absolute difference between the two putput of coutMechanism function
				int absDiffPos = labs(PosXCount.posR.size() - PosXCount.posL.size());

				/*
				// save thresholded image
				sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC4.jpg",i);
				cvSaveImage(buffer, cloneThreshImg);
				*/




				//if the length of the vector that contains the number of times it hit white pixel on the 
				// left side is less than the lenght of vector of number of times it hit white pixel on the 
				// right side and their absolute differecne is greater than the error limit than it is a left eye!!!
				if ((PosXCount.posR.size() > PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Right image indicator
					sideIndicator = 0;

					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();

					
					//rightImg = rightImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}

				//Vice versa
				else if ((PosXCount.posR.size() < PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Left image indicator
					sideIndicator = 1;


					//cout << "HIst step: " << (maxBins.max2ndBin - maxBins.max1stBin) << endl;
					//cout << "              " << endl;


					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
					//leftImg = leftImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}
				
				//otherwise
				else
				{
					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
										
				}


			}

			//Release memory
			cvReleaseImage(&thresholdImg);	
		}

	//Release memory
	vec3.clear();
	}
	


	else

	{
		
		//upperbound of density
		double densityUpThresh = 0.7;


		// define filter steps 3
		vector<float> vec4;
	
		//from -19 to -2
		for (float i = -19; i <= -2 ; i++)
		{
			//cout << i << endl;
			vec4.push_back(i);
		}


		//from -1.2 to -1.05
		for (float i = -1.9; i <= -1.05; i = i + 0.05)
		{
			//cout << i << endl;
			vec4.push_back(i);
		}

		//Insert zero
		vec4.push_back(0);


		//from -1.06 to -1.09
		for (float i = 1.06; i <= 1.09 ; i= i + 0.01)
		{
			//cout << i << endl;
			vec4.push_back(i);
		}



		//from 1.1 to 2
		for (float i = 1.1; i <= 2.1 ; i= i + 0.1)
		{
			//cout << i << endl;
			vec4.push_back(i);
		}


		//from 2.5 to 8
		for (float i = 2.5; i <= 8 ; i= i + 0.5)
		{
			//cout << i << endl;
			vec4.push_back(i);
		}


		//iterate through each filter step
		for(int i= 0; i < vec4.size(); i++)
		{
			//take 1st filter step
			threshStep = vec4[i];

			//define threshold image
			IplImage* thresholdImg = cvCreateImage(cvGetSize(imageCropped),8,1);
			//create binary image using threshCalDifStep function
			threshCalDifStep(maxBins.max2ndBin, maxBins.max1stBin, threshStep, imageCropped,thresholdImg); 


			//check teh white pixel density using densityCal function
			whitePixDensity =  densityCal(thresholdImg); 

			
			// check whther the white pixel density is lower than the upper bound of density threshold
			if (whitePixDensity <= densityUpThresh)
			{
				// use coutMechanism function here
				struct eyeSideDetector PosXCount = coutMechanism(thresholdImg);
				
				//absolute difference between the two putput of coutMechanism function
				int absDiffPos = labs(PosXCount.posR.size() - PosXCount.posL.size());

				/*
				// save thresholded image
				sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC4.jpg",i);
				cvSaveImage(buffer, cloneThreshImg);
				*/




				//if the length of the vector that contains the number of times it hit white pixel on the 
				// left side is less than the lenght of vector of number of times it hit white pixel on the 
				// right side and their absolute differecne is greater than the error limit than it is a left eye!!!
				if ((PosXCount.posR.size() > PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Right image indicator
					sideIndicator = 0;

					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();

					
					//rightImg = rightImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}

				//Vice versa
				else if ((PosXCount.posR.size() < PosXCount.posL.size()) &&  absDiffPos >= errorLimit)
				{
					//Left image indicator
					sideIndicator = 1;


					//cout << "HIst step: " << (maxBins.max2ndBin - maxBins.max1stBin) << endl;
					//cout << "              " << endl;


					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
					//leftImg = leftImg + 1;

					/*
					//save count mechanism image
					sprintf(buffer,"C:/Users/mimtiaz/Documents/Visual Studio 2010/Projects/EyeLockProjects/eyeSideDetectionTest03/10to8bit/outputImg/%uC6.jpg",c);
					cvSaveImage(buffer, cloneThreshImg);
					*/


					break;
				}
				
				//otherwise
				else
				{
					//Release memory
					cvReleaseImage(&thresholdImg);
					PosXCount.posR.clear();
					PosXCount.posL.clear();
										
				}


			}

			//Release memory
			cvReleaseImage(&thresholdImg);	
		}

	//Release memory
	vec4.clear();
	}



	//Release memory
	cvReleaseMemStorage(&storage);

	cvReleaseImage(&imgTemp);
	cvReleaseImage(&binaryImg);
	cvReleaseImage(&imageCropped);
	cvReleaseImage(&inputImgX);

	
	
	
	return sideIndicator;
	
	
	
}
