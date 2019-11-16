/*
 * extFocus.cpp
 *
 *  Created on: Oct 22, 2018
 *      Author: MO
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
#include "extFocus.h"


using namespace cv;
using namespace std;



//extFocus::extFocus()
extFocus::extFocus(char* filename)
:extConfig(filename)
,m_allLEDhighVoltEnable(0)
,m_IrisLEDVolt(0)
,m_IrisLEDcurrentSet(0)
,m_IrisLEDtrigger(0)
,m_IrisLEDEnable(0)
,m_IrisLEDmaxTime(0)
,m_AuxIrisCamExposureTime(0)
,m_AuxIrisCamDigitalGain(0)
,m_AuxIrisCamDataPedestal(0)
,m_MainIrisCamExposureTime(0)
,m_MainIrisCamDigitalGain(0)
,m_MainIrisCamDataPedestal(0)
,m_focusIrisTargetRectWidth(0)
,m_focusIrisTargetRectHeight(0)
,m_focusFaceTargetRectWidth(0)
,m_focusFaceTargetRectHeight(0)
,m_FaceIrisCamExposureTime(0)
,m_FaceIrisCamDigitalGain(0)
,m_FaceIrisCamDataPedestal(0)
,numCount(0)
,m_quit(false)
,m_focusDebug(false)
,leftCam(0)
,rightCam(0)
,faceCam(0)
,m_thresholdVal(0)
,m_lX1(0)
,m_lY1(0)
,m_lX2(0)
,m_lY2(0)
,m_RX1(0)
,m_RY1(0)
,m_RX2(0)
,m_RY2(0)
,m_FX1(0)
,m_FY1(0)
,m_FX2(0)
,m_FY2(0)
,lookUpTable(1,256,CV_8U)
{
	m_allLEDhighVoltEnable = extConfig.getValue("FTracker.allLEDhighVoltEnable",1);

	m_IrisLEDVolt = extConfig.getValue("FTracker.focusIrisLEDVolt",30);
	m_IrisLEDcurrentSet = extConfig.getValue("FTracker.focusIrisLEDcurrentSet",40);
	m_IrisLEDtrigger = extConfig.getValue("FTracker.focusIrisLEDtrigger",3);
	m_IrisLEDEnable = extConfig.getValue("FTracker.focusIrisLEDEnable",3);
	m_IrisLEDmaxTime = extConfig.getValue("FTracker.focusIrisLEDmaxTime",4);

	//Important note::: EXT firmware translate gain 32 as an unity
	m_AuxIrisCamExposureTime = extConfig.getValue("FTracker.focusAuxIrisCamExposureTime",7);
	m_AuxIrisCamDigitalGain = extConfig.getValue("FTracker.focusAuxIrisCamDigitalGain",50);
	m_AuxIrisCamDataPedestal = extConfig.getValue("FTracker.focusAuxIrisCamDataPedestal",0);

	m_MainIrisCamExposureTime = extConfig.getValue("FTracker.focusMainIrisCamExposureTime",5);
	m_MainIrisCamDigitalGain = extConfig.getValue("FTracker.focusMainIrisCamDigitalGain",50);
	m_MainIrisCamDataPedestal = extConfig.getValue("FTracker.focusMainIrisCamDataPedestal",0);

	m_FaceIrisCamExposureTime = extConfig.getValue("FTracker.focusFaceIrisCamExposureTime",5);
	m_FaceIrisCamDigitalGain = extConfig.getValue("FTracker.focusFaceIrisCamDigitalGain",50);
	m_FaceIrisCamDataPedestal = extConfig.getValue("FTracker.focusFaceIrisCamDataPedestal",0);

	m_focusIrisTargetRectWidth = extConfig.getValue("FTracker.focusIrisTargetRectWidth",180);
	m_focusIrisTargetRectHeight = extConfig.getValue("FTracker.focusIrisTargetRectHeight",100);
	m_focusFaceTargetRectWidth = extConfig.getValue("FTracker.focusFaceTargetRectWidth",65);
	m_focusFaceTargetRectHeight = extConfig.getValue("FTracker.focusFaceTargetRectHeight",45);

	leftCam = 8192;
	rightCam = 8193;
	faceCam = 8194;

	m_lX1 = extConfig.getValue("FTracker.focusLeftFirstDig_x",110);
	m_lY1 = extConfig.getValue("FTracker.focusLeftFirstDig_y",50);
	m_lX2 = extConfig.getValue("FTracker.focusLeftSecondDig_x",920);
	m_lY2 = extConfig.getValue("FTracker.focusLeftSecondDig_y",800);
	m_RX1 = extConfig.getValue("FTracker.focusRightFirstDig_x",110);
	m_RY1 = extConfig.getValue("FTracker.focusRightFirstDig_y",50);
	m_RX2 = extConfig.getValue("FTracker.focusRightSecondDig_x",820);
	m_RY2 = extConfig.getValue("FTracker.focusRightSecondDig_y",800);
	m_FX1 = extConfig.getValue("FTracker.focusFaceFirstDig_x",390);
	m_FY1 = extConfig.getValue("FTracker.focusFaceFirstDig_y",480);
	m_FX2 = extConfig.getValue("FTracker.focusFaceSecondDig_x",480);
	m_FY2 = extConfig.getValue("FTracker.focusFaceSecondDig_y",560);
}

extFocus::~extFocus(){
	blurImg.release();
	img.release();
	grayImg.release();
	cropImg.release();
}


//
void extFocus::DoStartCmd_focus(){

	printf("Setting up Device Parameters\n");

	char cmd[512];

	// first turn off all cameras
	sprintf(cmd,"set_cam_mode(0x0,%d)",100);		//Turn on Alternate cameras
	port_com_send(cmd);


	//Parsing Face LED commands - same as DoStartCmd()
	sprintf(cmd, "psoc_write(2,%i) | psoc_write(1,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(3,%i)| psoc_write(6,%i)\n",
			m_IrisLEDVolt, m_allLEDhighVoltEnable, m_IrisLEDcurrentSet, m_IrisLEDtrigger, m_IrisLEDEnable, m_IrisLEDmaxTime);
	port_com_send(cmd);
	//printf(cmd);

	//New Changes
	sprintf(cmd, "psoc_write(9,%i)\n", 80);
	port_com_send(cmd);	// charge cap for max current 60 < range < 95

	port_com_send("wcr(0x1f,0x300C,1650)");		// required before flipping the incoming images
	port_com_send("wcr(0x1B,0x3040,0xC000)"); //Flipping of iris and AUX images

	//Parsing AUX cam settings commands - same as DoStartCmd()
	sprintf(cmd, "wcr(0x03,0x3012,%i) | wcr(0x03,0x301e,%i) | wcr(0x03,0x305e,%i)\n",
			m_AuxIrisCamExposureTime, m_AuxIrisCamDataPedestal, m_AuxIrisCamDigitalGain);
	port_com_send(cmd);
	//printf(cmd);

	//Parsing Main cam settings commands - same as DoStartCmd()
	//sprintf(cmd,"wcr(0x18,0x305e,16)|wcr(0x18,0x3012,8)");
	sprintf(cmd, "wcr(0x18,0x3012,%i) | wcr(0x18,0x301e,%i) | wcr(0x18,0x305e,%i)\n",
			m_MainIrisCamExposureTime, m_MainIrisCamDataPedestal, m_MainIrisCamDigitalGain);
	port_com_send(cmd);
	//printf(cmd);


	//Parsing Face cam settings commands - same as DoStartCmd()
	sprintf(cmd,"wcr(0x04,0x305e,25)|wcr(0x04,0x3012,6)",
			m_FaceIrisCamDigitalGain, m_FaceIrisCamExposureTime);
	port_com_send(cmd);
	printf(cmd);

	//following process will activate PLL for all cameras
	sprintf(cmd,"set_cam_mode(0x00,%d)",10);	//turn off the cameras before changing PLL
	cvWaitKey(100);								//Wait for 100 msec
	port_com_send("wcr(0x1f,0x302e,2) | wcr(0x1f,0x3030,44) | wcr(0x1f,0x302c,2) | wcr(0x1f,0x302a,6)");
	cvWaitKey(10);								//Wait for 10 msec

	//Turn on analog gain
	sprintf(cmd,"wcr(0x1b,0x30b0,%i)\n",((1 &0x3)<<4) | 0X80);
	port_com_send(cmd);

	//Leave the PLL always ON
	port_com_send("wcr(0x1f,0x301a,0x1998)");

	printf("Finish setting up Param \n");

}



Mat extFocus::rotate90(Mat src){
	//EyelockLog(logger, TRACE, "rotation90");
	Mat dst;
	transpose(src, dst);
	flip(dst,dst,0);

	src.release();
	return dst;
}



extFocus::preProcessingImgF extFocus::preProcessingImg_focus(Mat &cropIm){
	int num = 0;
	//adjust brightness with other methods
	//cropIm.copyTo(cropImg);
	//BrightnessAndContrastAuto(cropIm,cropImg,0);
	//adjustBrightnessContrast_clahe(cropIm,cropImg,4);

	//adjusting brightness with Gamma correction
	//Mat cropImg;
	double gamma = 0.5;
	adjustBrightnessContrast_GammaCorrection(cropIm,cropImg,gamma);

	//Mat blurImg;

	//Tested GaussianBlur and medianBlur but these bluring methods washing out few vertical texture
	//cv::GaussianBlur(cropImg, blurImg,cv::Size(5,5),0,0,BORDER_DEFAULT);
	//cv::medianBlur(cropImg, blurImg,21);


	//bilateralFilter works best as it remove spatial noise by preserving the edges
	int b_kernal =2;
	bilateralFilter(cropImg,blurImg, b_kernal, b_kernal*2, b_kernal/2, BORDER_DEFAULT);


	if (m_focusDebug){
		sprintf(text, "Blured_img_noise_removed%i.bmp",num);
		cv::imshow(text,blurImg);
		cv::imwrite(text,blurImg);
		cvWaitKey(0);
	}


	//grayImg(blurImg.size(),CV_8UC1);

	//applied histogram equalization and normalization for brightness and contrast distribution but the focus
	//matrix gets imbalanced by that
	//cout << grayImg.channels() << "		" << blurImg.channels() << endl;
	//equalizeHist(blurImg, grayImg);		//grayImg is the histogram equalized images

	blurImg.copyTo(grayImg);
	//cv::cvtColor(blurImg,grayImg,CV_BGR2GRAY);
	//grayImg = cropImg;

	if (m_focusDebug){
		sprintf(text, "Gray Img");
		cv::imshow(text,grayImg);
		//cv::imwrite(text,grayImg);
		cvWaitKey(0);
	}

	return {grayImg, cropImg};
}


void extFocus::adjustBrightnessContrast_GammaCorrection(Mat &src, Mat &out, double gamma){
	//gamma lesser then 1.0 will make the image darker (shift the histogram toward left) but
	//high contrast zone will be clearly visible but it will miss all the lower level of texture changes
	//gamma value greater then 1.0 will make the image brighter and high and low contrast changes will be clearly visible

	//Mat lookUpTable(1,256,CV_8U);	//initiate a lookup table 1 - 256
	uchar* x = lookUpTable.ptr();

	for (int i = 0; i < 256; i++){
		//convert 1-256 into 0.0-1.0 and apply gamma like norm(I(x,y))^gamma
		//then change the conversion into 0-255 again
		x[i] = saturate_cast<uchar> (pow(i/255.0, gamma) * 255.0);
	}

	//transform src image based on lookUpTable vals
	cv::LUT(src, lookUpTable, out);
}


void extFocus::adjustBrightnessContrast_clahe(Mat &src, Mat &dst, double clipLimit){

    // apply the CLAHE algorithm to the L channel
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(clipLimit);
    clahe->apply(src, dst);
}


void extFocus::BrightnessAndContrastAuto(const cv::Mat &src, cv::Mat &dst, float clipHistPercent)
{

    int histSize = 256;
    float alpha, beta;
    double minGray = 0, maxGray = 0;

    //to calculate gray-scale histogram
    cv::Mat gray;
    if (src.type() == CV_8UC1)
    	gray = src;
    else if (src.type() == CV_8UC3)
    	cvtColor(src, gray, CV_BGR2GRAY);
    else if (src.type() == CV_8UC4)
    	cvtColor(src, gray, CV_BGRA2GRAY);


    if (clipHistPercent == 0)
    {
        // keep full available range
        cv::minMaxLoc(gray, &minGray, &maxGray);
    }
    else
    {
        cv::Mat hist; //the gray-scale histogram

        float range[] = { 0, 256 };
        const float* histRange = { range };
        bool uniform = true;
        bool accumulate = false;
        calcHist(&gray, 1, 0, cv::Mat (), hist, 1, &histSize, &histRange, uniform, accumulate);

        // calculate cumulative distribution from the histogram
        std::vector<float> accumulator(histSize);
        accumulator[0] = hist.at<float>(0);
        for (int i = 1; i < histSize; i++)
        {
            accumulator[i] = accumulator[i - 1] + hist.at<float>(i);
        }

        // locate points that cuts at required value
        float max = accumulator.back();
        clipHistPercent *= (max / 100.0); //make percent as absolute
        clipHistPercent /= 2.0; // left and right wings
        // locate left cut
        minGray = 0;
        while (accumulator[minGray] < clipHistPercent)
            minGray++;

        // locate right cut
        maxGray = histSize - 1;
        while (accumulator[maxGray] >= (max - clipHistPercent))
            maxGray--;
    }

    // current range
    float inputRange = maxGray - minGray;

    alpha = (histSize - 1) / inputRange;   // alpha expands current range to histsize range
    beta = -minGray * alpha;             // beta shifts current range so that minGray will go to 0

    // Apply brightness and contrast normalization
    // convertTo operates with saurate_cast
    src.convertTo(dst, -1, alpha, beta);

    // restore alpha channel from source
    if (dst.type() == CV_8UC4)
    {
        int from_to[] = { 3, 3};
        cv::mixChannels(&src, 4, &dst,1, from_to, 1);
    }
    return;
}


extFocus::focusMatric extFocus::sobelBasedFocusMeasure(Mat &cropIm){

	//Later use for detect high exposure images
/*	double minVal, maxVal;
	Point minLoc, maxLoc;
	cv::minMaxLoc(cropIm, &minVal, &maxVal, &minLoc, &maxLoc);



	Mat bw;
	threshold( cropIm, bw, 0, 255,THRESH_BINARY);
	Mat nonZeroPix;
	cv::findNonZero(bw, nonZeroPix);

	int zeroPix = int(bw.rows * bw.cols) - int(nonZeroPix.rows * nonZeroPix.cols);

	if (minVal <= 2 || maxVal >= 230){
		printf("Min::: %i		Max::: %i\n", int(minVal),int(maxVal));

		printf("Zero Pix Number::: %i	%i\n", zeroPix);

	}*/

	int num = 0;
	//adjust brightness with other methods
	//cropIm.copyTo(cropImg);
	//BrightnessAndContrastAuto(cropIm,cropImg,0);
	//adjustBrightnessContrast_clahe(cropIm,cropImg,4);

	//adjusting brightness with Gamma correction
	Mat cropImg;
	double gamma = 0.5;

	extFocus::adjustBrightnessContrast_GammaCorrection(cropIm,cropImg,gamma);

	Mat blurImg;

	//Tested GaussianBlur and medianBlur but these bluring methods washing out few vertical texture
	//cv::GaussianBlur(cropImg, blurImg,cv::Size(5,5),0,0,BORDER_DEFAULT);
	//cv::medianBlur(cropImg, blurImg,21);


	//bilateralFilter works best as it remove spatial noise by preserving the edges
	int b_kernal =3;
	bilateralFilter(cropImg,blurImg, b_kernal, b_kernal*2, b_kernal/2, BORDER_DEFAULT);


	if (m_focusDebug){
		sprintf(text, "Blured_img_noise_removed%i.bmp",num);
		cv::imshow(text,blurImg);
		cv::imwrite(text,blurImg);
		cvWaitKey(0);
	}


	Mat grayImg(blurImg.size(),CV_8UC1);

	//applied histogram equalization and normalization for brightness and contrast distribution but the focus
	//matric gets imbalenced by that
	//cout << grayImg.channels() << "		" << blurImg.channels() << endl;
	//equalizeHist(blurImg, grayImg);		//grayImg is the histogram equalized images

	blurImg.copyTo(grayImg);
	//cv::cvtColor(blurImg,grayImg,CV_BGR2GRAY);
	//grayImg = cropImg;

	if (m_focusDebug){
		sprintf(text, "Gray Img");
		cv::imshow(text,grayImg);
		//cv::imwrite(text,grayImg);
		cvWaitKey(0);
	}


	//Sobel based operation
	//The purpose of using sobel operator is to get the contrast edges of vertical lines
	//Here we are only extarcting the x direction data


	//1st order derivative on x direction
	int x_order = 1;	//activate x directional feature
	int y_order = 0;	//de-active y directional feature
	int kernal = 3;
	int scale = 1;
	int delta = 0;
	int depth = CV_16S;	//CV_16S to avoid overflow

	Mat x_gradImg,y_gradImg;
	Mat scaledImgX,scaledImgY;

	cv::Sobel(grayImg,x_gradImg,depth,x_order,y_order,kernal,scale,delta,BORDER_DEFAULT);
	cv::convertScaleAbs(x_gradImg,scaledImgX);	//cal absolute val and convert to 8-bit integers


/*
	//Dont need to look for y direction as we dont have any edges there
	//if we keep this activate, for un-focus images it will peak the noise as horizental texture
	x_order = 0;	//de-activate x directional feature
	y_order = 1;	//active y directional feature
	cv::Sobel(grayImg,y_gradImg,depth,x_order,y_order,kernal,scale,delta,BORDER_DEFAULT);
	cv::convertScaleAbs(y_gradImg,scaledImgY);	//cal absolute val and convert to 8-bit integers
*/

	if (m_focusDebug){
		sprintf(text, "x_direction_line_feature%i.bmp",num);
		cv::imshow(text,scaledImgX);
		cv::imwrite(text,scaledImgX);
		cvWaitKey(0);

		sprintf(text, "y_direction_line_feature%i.bmp",num);
		cv::imshow(text,scaledImgY);
		cv::imwrite(text,scaledImgY);
		cvWaitKey(0);
	}


	//It will add the vertical and horizontal feature together
	//Mat fm = scaledImgX.mul(scaledImgX) + scaledImgY.mul(scaledImgY);


	Mat fmX = scaledImgX.mul(scaledImgX);

	//normalize all the texture analysis between 0.0 to 1.0 for scaling
	Mat fm;
	cv::normalize(fmX, fm, 0.0, 1.0, NORM_MINMAX, CV_32F);

	//statistic of texture analysis----> mean, std(sigma) and vaience(sigma^2)
	cv::Scalar mean,sigma;
	cv::meanStdDev(fm,mean,sigma);

	fm.release();
	fmX.release();
	scaledImgX.release();
	scaledImgY.release();
	x_gradImg.release();
	y_gradImg.release();
	grayImg.release();
	blurImg.release();


	return {float(mean.val[0]),float(sigma.val[0])};
}


extFocus::focusMatric extFocus::laplacianBasedFocusMeasure(Mat &grayImg){
	int num = 0;
	//Laplacian based operation

	//initialization of laplacian
	int kernal = 3;
	int scale = 1;
	int delta = 0;
	int depth = CV_16S;	//CV_16S to avoid overflow

	Mat lapImg;
	cv::Laplacian(grayImg, lapImg, depth, kernal, scale, delta, BORDER_DEFAULT );

	Mat ScaledLap;
	convertScaleAbs( lapImg, ScaledLap );

	if (m_focusDebug){
		sprintf(text, "x_direction_line_feature%i.bmp",num);
		cv::imshow(text,ScaledLap);
		cv::imwrite(text,ScaledLap);
		cvWaitKey(0);
	}

	Mat fm;
	cv::normalize(ScaledLap, fm, 0.0, 1.0, NORM_MINMAX, CV_32F);
	//ScaledLap.copyTo(fm);

	cv::Scalar mean,sigma;
	cv::meanStdDev(fm,mean,sigma);

	return {float(mean.val[0]),float(sigma.val[0])};

/*	CvFont font;
	double hScale=.1;
	double vScale=.1;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX, hScale,vScale,0,lineWidth);

	//printf("Mean:: %4.4f	std:: %4.4f	Var:: %4.4f\n", mean.val[0], sigma.val[0],sigma.val[0]*sigma.val[0]);
	//sprintf(textI1,"Mean: %0.2f std: %0.2f Var: %0.2f", mean.val[0], sigma.val[0],sigma.val[0]*sigma.val[0]);
	//sprintf(textI1,"Mean: %0.2f std: %0.2f", mean.val[0], sigma.val[0]);
	sprintf(textI1,"focus val mean: %0.2f    Var:: %0.2f ", float((mean.val[0]+0.005) * 100), float(((sigma.val[0]*sigma.val[0]) + 0.005) * 1000));
	//sprintf(textI1,"focus val mean: %4.4f    Var:: %4.4f ", float((mean.val[0]+0.0000) * 1), float((sigma.val[0]*sigma.val[0]) * 1));
	//focusScore << mean.val[0] * 100 << "," << sigma.val[0] << "," << sigma.val[0]*sigma.val[0] << endl;

	//cv::putText(grayImg,textI1,cvPoint(5,20), CV_FONT_HERSHEY_COMPLEX,1.0,cvScalar(255,255,255),1,CV_AA);
	sprintf(text, "focus Measure in Rect");
	cv::namedWindow(text);
	cv::imwrite("imgCheck.bmp",cropImg);
	cv::imshow("imgCheck.bmp",cropImg);
	cv::rectangle(img, crop, Scalar(255, 0, 0), 1, 0);
	cv::putText(img,textI1,cvPoint(crop.x,crop.y - 30), CV_FONT_HERSHEY_COMPLEX,1.0,cvScalar(255,0,0),1,CV_AA);
	cv::imshow(text,img);*/

}


extFocus::measureFocusOut extFocus::measureFocus(Mat img, int camID, int width, int height, int x1, int y1, int x2, int y2){

	if(img.empty())
		printf("Error reading image!\n");

	Mat cropIm;
	preProcessingImgF imgBox;
	focusMatric matric;
	measureFocusOut out;

	//working on a small patch 1 top left
	{
	//Rect crop1(5,50,width,height);
	Rect crop1(x1,y1,width,height);

	cropIm = img(crop1);

	imgBox = preProcessingImg_focus(cropIm);

	matric = sobelBasedFocusMeasure(imgBox.dstImg);


	out.mean1 = matric.mean;
	out.sigma1 = matric.sigma;
	out.ROI1 = crop1;
	out.brightAdjImg1 = imgBox.brightnessAdjImg;
	}



	//working on a small patch 2 top right
	{
	//Rect crop2(1010,50,width,height);
	Rect crop2(x2,y1,width,height);
	cropIm = img(crop2);

	imgBox = preProcessingImg_focus(cropIm);

	matric = sobelBasedFocusMeasure(imgBox.dstImg);

	//measureFocusOut out;
	out.mean2 = matric.mean;
	out.sigma2 = matric.sigma;
	out.ROI2 = crop2;
	out.brightAdjImg2 = imgBox.brightnessAdjImg;
	}


	//working on a small patch 3 bottom left
	{
	//Rect crop3(5,800,width,height);
	Rect crop3(x1,y2,width,height);
	cropIm = img(crop3);

	imgBox = preProcessingImg_focus(cropIm);

	matric = sobelBasedFocusMeasure(imgBox.dstImg);

	//measureFocusOut out;
	out.mean3 = matric.mean;
	out.sigma3 = matric.sigma;
	out.ROI3 = crop3;
	out.brightAdjImg3 = imgBox.brightnessAdjImg;
	}


	//working on a small patch 4 bottom right
	{
	//Rect crop4(1010,800,width,height);
	Rect crop4(x2,y2,width,height);
	cropIm = img(crop4);

	imgBox = preProcessingImg_focus(cropIm);

	matric = sobelBasedFocusMeasure(imgBox.dstImg);

	//measureFocusOut out;
	out.mean4 = matric.mean;
	out.sigma4 = matric.sigma;
	out.ROI4 = crop4;
	out.brightAdjImg4 = imgBox.brightnessAdjImg;
	}

	int mx1 = int(float(float(x1+x2)/2.0) + 0.5);
	int my1 = int(float(float(y1+y2)/2.0) + 0.5);

	//working on a small patch 5 Center
	{
	//Rect crop5(500,430,width,height);
	//Rect crop5(503,375,width,height);
	Rect crop5(mx1,my1,width,height);
	cropIm = img(crop5);

	imgBox = preProcessingImg_focus(cropIm);

	matric = sobelBasedFocusMeasure(imgBox.dstImg);

	//measureFocusOut out;
	out.mean5 = matric.mean;
	out.sigma5 = matric.sigma;
	out.ROI5 = crop5;
	out.brightAdjImg5 = imgBox.brightnessAdjImg;

	cropIm.release();
	}

	//Root mean square deviation of four corner values will detect crooked lens

	imgBox.dstImg.release();
	imgBox.brightnessAdjImg.release();
	return out;

/*	CvFont font;
	double hScale=.1;
	double vScale=.1;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX, hScale,vScale,0,lineWidth);

	//for sobel
	sprintf(textI1,"focus val: %0.0f", float((matric.mean+0.0005) * 100));		//scaling data from 0 - 100
	//for laplacian
	//sprintf(textI1,"focus val mean: %0.2f    Var:: %0.2f ", float((matric.mean+0.005) * 100), float(((matric.sigma*matric.sigma) + 0.005) * 1000));


	sprintf(text,"Focusing %s %s Camera ",camID & 0x01 ? "Left":"Right",camID & 0x80 ?  "AUX":"MAIN");
	cv::namedWindow(text);

	//Uncomment the following two lines if brightness change effects need to check
	//cv::imwrite("imgCheck.bmp",imgBox.brightnessAdjImg);
	//cv::imshow("imgCheck.bmp",imgBox.brightnessAdjImg);
	cv::rectangle(img, crop, Scalar(255, 0, 0), 1, 0);
	cv::putText(img,textI1,cvPoint(crop.x,crop.y - 10), CV_FONT_HERSHEY_COMPLEX,1.0,cvScalar(255,0,0),1,CV_AA);
	cv::imshow(text,img);*/


}


void extFocus::displayInstruction(int camID){
	bool side, camType;

	if (!(camID == 4)){
		side = camID & 0x01 ? 1:0;
		camType = camID & 0x80 ? 1:0;

		if (camType){
			sprintf(extFocus::textI9,"Insert AUX target in ...");
			if(side)
				sprintf(extFocus::textI11,"1. The Left AUX score is MAX");
			else
				sprintf(extFocus::textI11,"1. The Right AUX score is MAX");
		}else{
			sprintf(extFocus::textI9,"Insert Main target in ...");
			if(side)
				sprintf(extFocus::textI11,"1. The Left Main score is MAX");
			else
				sprintf(extFocus::textI11,"1. The Right Main score is MAX");
		}

	}else{
		sprintf(extFocus::textI9,"Expose FACE target ...");
		sprintf(extFocus::textI11,"1. The Face score is MAX");
	}


	sprintf(extFocus::textI8,"Instruction for focusing EXT");
	sprintf(extFocus::textI10,"Continue focusing until: ");
	sprintf(extFocus::textI12,"2. The Window says TARGET VERIFIED!");
	sprintf(extFocus::textI13,"Press 's' to save images");
	sprintf(extFocus::textI14,"Press 'q' to continue ...");



	cv::putText(DisImg,extFocus::textI9,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
	cv::putText(DisImg,extFocus::textI10,cvPoint(10,100), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
	cv::putText(DisImg,extFocus::textI11,cvPoint(10,150), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
	cv::putText(DisImg,extFocus::textI12,cvPoint(10,200), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
	cv::putText(DisImg,extFocus::textI13,cvPoint(10,250), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
	cv::putText(DisImg,extFocus::textI14,cvPoint(10,300), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
}


bool extFocus::camControlFocus(Mat &img,int camID){

	char key;
	int x1,y1,x2,y2;	//co-ordinates for corner Rect

	DisImg = cv::Mat::zeros(400, 650, CV_8UC3);

/*	sprintf(extFocus::textI8,"Instruction for focusing EXT");
	sprintf(extFocus::textI10,"Continue focusing until: ");
	sprintf(extFocus::textI12,"2. The Window says TARGET VERIFIED!");
	sprintf(extFocus::textI13,"Press 's' to save images");
	sprintf(extFocus::textI14,"Press 'q' to continue ...");

	if (camID == 24){
		sprintf(extFocus::textI9,"Put AUX target in ...");
		sprintf(extFocus::textI11,"1. The Left AUX score is MAX");
	}else if (camID == 16){
		sprintf(extFocus::textI9,"Put Main target in ...");
		sprintf(extFocus::textI11,"1. The Main score is MAX");
	}else if (){
		sprintf(extFocus::textI9,"Put Face target in ...");
		sprintf(extFocus::textI11,"1. The Face score is MAX");
	}

	cv::putText(DisImg,extFocus::textI9,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
	cv::putText(DisImg,extFocus::textI10,cvPoint(10,100), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
	cv::putText(DisImg,extFocus::textI11,cvPoint(10,150), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
	cv::putText(DisImg,extFocus::textI12,cvPoint(10,200), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
	cv::putText(DisImg,extFocus::textI13,cvPoint(10,250), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);
	cv::putText(DisImg,extFocus::textI14,cvPoint(10,300), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),0.5,CV_AA);*/

/*	cv::moveWindow(extFocus::textI8, 1260, 10);
	cv::imshow(extFocus::textI8, DisImg);*/

	measureFocusOut resultFocus;

	CvFont font;
	double hScale=.2;
	double vScale=.2;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_COMPLEX, hScale,vScale,0,lineWidth);

	//while(1){
	//vs->get(&w,&h,(char *)img.data);

	if (camID & 0x80 ?  1:0){
		//x1 = 10, y1 = 50, x2 = 1020, y2 = 800;
		//x1 = 110, y1 = 50, x2 = 920, y2 = 800;
		resultFocus = measureFocus(img, camID, m_focusIrisTargetRectWidth, m_focusIrisTargetRectHeight, m_lX1, m_lY1, m_lX2, m_lY2);
	}
	else{
		//x1 = 30, y1 = 70, x2 = 860, y2 = 750;
		//x1 = 110, y1 = 50, x2 = 820, y2 = 800;
		resultFocus = measureFocus(img, camID, m_focusIrisTargetRectWidth, m_focusIrisTargetRectHeight, m_RX1, m_RY1, m_RX2, m_RY2);
	}
	//resultFocus = measureFocus(img, vs->cam_id, width, height, x1, y1, x2, y2);

	if(camID == 4){
		//x1 = 390, y1 = 480, x2 = 480, y2 = 560;
		//x1 = 418, y1 = 578, x2 = 418, y2 = 578;
		Mat rotMat = rotate90(img);
		//printf("Successfully rotate image\n");
		m_focusFaceTargetRectWidth = 65;
		m_focusFaceTargetRectHeight = 45;
		resultFocus = measureFocus(rotMat, camID, m_focusFaceTargetRectWidth, m_focusFaceTargetRectHeight, m_FX1, m_FY1, m_FX2, m_FY2);


		//New Addition
		//The problem of taking difference is when image is washed out it still the diff as zero and when
		// the froat and rare target is focus the number is close to 10
		float FourCorAvg = (resultFocus.mean1 + resultFocus.mean2 + resultFocus.mean3 + resultFocus.mean4) / 4.0;
		//int diff = abs(((FourCorAvg - resultFocus.mean5) + 0.0005) * 100);

		// If we multiply the avg of rare target and front target val , it works for all scenarios
		// the val varies from 0 - 10,000 in ideal scenarios
		//Howver, our target val is close or above 6500
		//int scaleFront = (resultFocus.mean5 + 0.0005) * 100;
		int scaleRare = (FourCorAvg + 0.0005) * 100;
		//int mult = scaleFront * scaleRare;


		//for sobel

		//closing the display of focus cal of each block
		if (0){
			sprintf(extFocus::textI1,"focus: %0.0f", float((resultFocus.mean1+0.0005) * 100));		//scaling data from 0 - 100
			sprintf(extFocus::textI2,"focus: %0.0f", float((resultFocus.mean2+0.0005) * 100));		//scaling data from 0 - 100
			sprintf(extFocus::textI3,"focus: %0.0f", float((resultFocus.mean3+0.0005) * 100));		//scaling data from 0 - 100
			sprintf(extFocus::textI4,"focus: %0.0f", float((resultFocus.mean4+0.0005) * 100));		//scaling data from 0 - 100
			sprintf(extFocus::textI5,"focus: %0.0f", float((resultFocus.mean5+0.0005) * 100));		//scaling data from 0 - 100
		}
		sprintf(extFocus::textI6,"Face: %i", scaleRare);		//scaling data from 0 - 100
		//sprintf(extFocus::textI7,"Mult: %i", mult);		//scaling data from 0 - 10,000

		//for laplacian
		//sprintf(textI1,"focus val mean: %0.2f    Var:: %0.2f ", float((matric.mean+0.005) * 100), float(((matric.sigma*matric.sigma) + 0.005) * 1000));


		sprintf(extFocus::text,"Focusing %s Camera ","Face");
		cv::namedWindow(extFocus::text);

		//Uncomment the following two lines if brightness change effects need to check
		//cv::imwrite("imgCheck.bmp",resultFocus.brightAdjImg);
		//cv::imshow("imgCheck.bmp",resultFocus.brightAdjImg);

		cv::rectangle(rotMat, resultFocus.ROI1, Scalar(255, 0, 0), 2, 0);
		cv::rectangle(rotMat, resultFocus.ROI2, Scalar(255, 0, 0), 2, 0);
		cv::rectangle(rotMat, resultFocus.ROI3, Scalar(255, 0, 0), 2, 0);
		cv::rectangle(rotMat, resultFocus.ROI4, Scalar(255, 0, 0), 2, 0);
		//cv::rectangle(rotMat, resultFocus.ROI5, Scalar(255, 0, 0), 1, 0);

		if (0){
		cv::putText(rotMat,textI1,cvPoint(resultFocus.ROI1.x - 180,resultFocus.ROI1.y - 0), CV_FONT_HERSHEY_COMPLEX,0.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(rotMat,textI2,cvPoint(resultFocus.ROI2.x + 140,resultFocus.ROI2.y - 0), CV_FONT_HERSHEY_COMPLEX,0.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(rotMat,textI3,cvPoint(resultFocus.ROI3.x - 180,resultFocus.ROI3.y - 0), CV_FONT_HERSHEY_COMPLEX,0.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(rotMat,textI4,cvPoint(resultFocus.ROI4.x + 140,resultFocus.ROI4.y - 0), CV_FONT_HERSHEY_COMPLEX,0.5,cvScalar(255,0,0),1,CV_AA);
		}
		//cv::putText(rotMat,textI5,cvPoint(resultFocus.ROI5.x - 50,resultFocus.ROI5.y - 10), CV_FONT_HERSHEY_COMPLEX,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(rotMat,textI6,cvPoint((img.cols/2.0) - 200, 200), CV_FONT_HERSHEY_COMPLEX,2,cvScalar(255,0,0),2,CV_AA);
		//cv::putText(rotMat,textI7,cvPoint(img.rows/2.0, 100), CV_FONT_HERSHEY_COMPLEX,1.5,cvScalar(255,0,0),1,CV_AA);


		int cut = 50;
		cv::Rect ROI_rot(cut,cut,rotMat.cols - cut, rotMat.rows - cut);
		Mat rotMatROI = rotMat(ROI_rot);
		m_arucoID = detectMarkers(rotMatROI, camID, cv::Point(m_FX1,m_FY1), cv::Point(m_FX2,m_FY2), m_focusFaceTargetRectWidth, m_focusFaceTargetRectHeight);

		if (m_arucoID.empty() && camID == 4){
			pt_title.x = (rotMat.cols/2.0) - 330;
			pt_title.y = 100;

			pt_title_t.x = (rotMat.cols/2.0) - 300;
			pt_title_t.y = 100;

			sprintf(extFocus::textI7,"%s", "Face Target verified");		//scaling data from 0 - 100
			cv::putText(rotMat,textI7,cvPoint(pt_title.x, pt_title.y), CV_FONT_HERSHEY_COMPLEX,2,cvScalar(255,0,0),2,CV_AA);
		}
		else{
			pt_title.x = 50;
			pt_title.y = 100;

			sprintf(extFocus::textI7,"%s", "Wrong Target Expose Face target");		//scaling data from 0 - 100
			cv::putText(rotMat,textI7,cvPoint(pt_title.x, pt_title.y), CV_FONT_HERSHEY_COMPLEX,1.5,cvScalar(255,0,0),2,CV_AA);
		}


		cv::imshow(extFocus::text,rotMat);
		cv::moveWindow(extFocus::text, 20, 20);

		displayInstruction(camID);
		sprintf(extFocus::textI9,"Put Face target in ...");
		sprintf(extFocus::textI11,"1. The Face score is MAX");
		cv::moveWindow(extFocus::textI8, 1260, 10);
		cv::imshow(extFocus::textI8, DisImg);

		//for continuous streaming
		key = cv::waitKey(1);
		//For quit streaming
		if (key=='q'){
			char fName[50];
			sprintf(fName,"/home/root/data/calibration/%s_%d.bmp","FaceCam", numCount++);
			cv::imwrite(fName,rotMat);
			printf("saved %s\n",fName);
			numCount = 0;
			m_quit = true;
			destroyWindow(extFocus::text);
			destroyWindow(extFocus::textI8);

			return m_quit;
			//break;
		}
		//For saving images while streaming individual cameras
		if(key=='s')
		{
			if (camID == 8194){
				char fName[50];
				sprintf(fName,"%s_%d.bmp","Face_cam", numCount++);
				cv::imwrite(fName,rotMat);
				printf("saved %s\n",fName);
			}
			else{
				char fName[50];
				sprintf(fName,"%s_%s_%d.bmp",camID & 0x01 ? "Left":"Right",camID & 0x80 ?  "AUX":"MAIN", numCount++);
				cv::imwrite(fName,img);
				printf("saved %s\n",fName);
			}

		}

		rotMatROI.release();
		resultFocus.brightAdjImg1.release();
		resultFocus.brightAdjImg2.release();
		resultFocus.brightAdjImg3.release();
		resultFocus.brightAdjImg4.release();
		resultFocus.brightAdjImg5.release();
		rotMat.release();

		return m_quit;
	}
	else{

	//New Addition
	//The problem of taking difference is when image is washed out it still the diff as zero and when
	// the froat and rare target is focus the number is close to 10
	float FourCorAvg = (resultFocus.mean1 + resultFocus.mean2 + resultFocus.mean3 + resultFocus.mean4) / 4.0;
	int diff = abs(((FourCorAvg - resultFocus.mean5) + 0.0005) * 100);

	// If we multiply the avg of rare target and front target val , it works for all scenarios
	// the val varies from 0 - 10,000 in ideal scenarios
	//Howver, our target val is close or above 6500
	int scaleFront = (resultFocus.mean5 + 0.0005) * 100;
	int scaleRare = (FourCorAvg + 0.0005) * 100;
	//int mult = scaleFront * scaleRare;
	int multScaled = ((scaleFront * scaleRare) / 10000.0) * 100.0;

/*	printf("FourCorAvg ::::: %3.3f\n", FourCorAvg);
	printf("FourCorAvg ::::: %3.3f\n", resultFocus.mean5);
	printf("diff ::::: %i\n", diff);
	printf("Mult ::::: %i\n", mult);*/

	//for sobel
	sprintf(extFocus::textI1,"focus: %0.0f", float((resultFocus.mean1+0.0005) * 100));		//scaling data from 0 - 100
	sprintf(extFocus::textI2,"focus: %0.0f", float((resultFocus.mean2+0.0005) * 100));		//scaling data from 0 - 100
	sprintf(extFocus::textI3,"focus: %0.0f", float((resultFocus.mean3+0.0005) * 100));		//scaling data from 0 - 100
	sprintf(extFocus::textI4,"focus: %0.0f", float((resultFocus.mean4+0.0005) * 100));		//scaling data from 0 - 100
	sprintf(extFocus::textI5,"focus: %0.0f", float((resultFocus.mean5+0.0005) * 100));		//scaling data from 0 - 100
	sprintf(extFocus::textI6,"%s %s:%i", camID & 0x01 ? "Left":"Right",camID & 0x80 ?  "AUX":"MAIN", multScaled);		//scaling data from 0 - 100
	//sprintf(extFocus::textI7,"Mult1: %i", mult);		//scaling data from 0 - 10,000

	//for laplacian
	//sprintf(textI1,"focus val mean: %0.2f    Var:: %0.2f ", float((matric.mean+0.005) * 100), float(((matric.sigma*matric.sigma) + 0.005) * 1000));


	sprintf(extFocus::text,"Focusing %s %s Camera ",camID & 0x01 ? "Left":"Right",camID & 0x80 ?  "AUX":"MAIN");
	cv::namedWindow(extFocus::text);

	//Uncomment the following two lines if brightness change effects need to check
	//cv::imwrite("imgCheck.bmp",resultFocus.brightAdjImg);
	//cv::imshow("imgCheck.bmp",resultFocus.brightAdjImg);

	cv::rectangle(img, resultFocus.ROI1, Scalar(255, 0, 0), 2, 0);
	cv::rectangle(img, resultFocus.ROI2, Scalar(255, 0, 0), 2, 0);
	cv::rectangle(img, resultFocus.ROI3, Scalar(255, 0, 0), 2, 0);
	cv::rectangle(img, resultFocus.ROI4, Scalar(255, 0, 0), 2, 0);
	cv::rectangle(img, resultFocus.ROI5, Scalar(255, 0, 0), 2, 0);

	pt_title.x = (img.cols/2.0) - 300;
	pt_title.y = (img.rows/2.0) - 240;

	pt_title_t.x = (img.cols/2.0) - 300;
	pt_title_t.y = 100;

	//img.cols/2.0) - 270, 225)

	cv::putText(img,textI1,cvPoint(resultFocus.ROI1.x,resultFocus.ROI1.y - 10), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI2,cvPoint(resultFocus.ROI2.x,resultFocus.ROI2.y - 10), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI3,cvPoint(resultFocus.ROI3.x,resultFocus.ROI3.y - 10), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI4,cvPoint(resultFocus.ROI4.x,resultFocus.ROI4.y - 10), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI5,cvPoint(resultFocus.ROI5.x,resultFocus.ROI5.y - 10), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI6,pt_title, CV_FONT_HERSHEY_COMPLEX,2,cvScalar(255,0,0),2,CV_AA);
	//cv::putText(img,textI7,cvPoint(img.cols/2.0, 100), CV_FONT_HERSHEY_COMPLEX,1.5,cvScalar(255,0,0),1,CV_AA);

	if (camID & 0x80 ?  1:0){
		x1 = m_lX1, y1 = m_lY1, x2 = m_lX2, y2 = m_lY2;
	}
	else{
		x1 = m_RX1, y1 = m_RY1, x2 = m_RX2, y2 = m_RY2;
	}

	m_arucoID = detectMarkers(img, camID, cv::Point(x1,y1), cv::Point(x2,y2), m_focusFaceTargetRectWidth, m_focusFaceTargetRectHeight);

	if (!m_arucoID.empty()){
		if (camID & 0x80 ?  1:0){
			//printf("Cam ID :: %d\n", camID);
			bool camVer = (camID == 129 || camID ==130);
			if (camVer && m_arucoID[0].id == 24){
				sprintf(extFocus::textI7,"%s", "Target verified");		//scaling data from 0 - 100
				cv::putText(img,textI7,pt_title_t, CV_FONT_HERSHEY_COMPLEX,2,cvScalar(255,0,0),2,CV_AA);
			}else if(camVer && m_arucoID[0].id == 16){
				sprintf(extFocus::textI7,"%s", "WRONG Target Insert AUX target");		//scaling data from 0 - 100
				cv::putText(img,textI7,cvPoint(pt_title_t.x = 10, pt_title_t.y), CV_FONT_HERSHEY_COMPLEX,2,cvScalar(255,0,0),2,CV_AA);
			}
		}
		else{
			//printf("Cam ID :: %d\n", camID);
			bool camVer = (camID == 1 || camID ==2);
			if (camVer && m_arucoID[0].id == 16){
				sprintf(extFocus::textI7,"%s", "Target verified");		//scaling data from 0 - 100
				cv::putText(img,textI7,pt_title_t, CV_FONT_HERSHEY_COMPLEX,2,cvScalar(255,0,0),2,CV_AA);
			}else if(camVer && m_arucoID[0].id == 24){
				sprintf(extFocus::textI7,"%s", "WRONG Target Insert MAIN target");		//scaling data from 0 - 100
				cv::putText(img,textI7,cvPoint(pt_title_t.x = 10, pt_title_t.y), CV_FONT_HERSHEY_COMPLEX,2,cvScalar(255,0,0),2,CV_AA);
			}
		}

	}
	else{
		//If we want to activate this chain later for not having any markers in the FOV case
		if (0){
			//printf("Cam ID :: %d\n", camID);
			if (camID == 129 || camID == 130){
				sprintf(extFocus::textI7,"%s", "Focus the camera until the marker is visible");		//scaling data from 0 - 100
				cv::putText(img,textI7,cvPoint(pt_title_t.x = 20, pt_title_t.y), CV_FONT_HERSHEY_COMPLEX,1.4,cvScalar(255,0,0),2,CV_AA);
			}else if (camID == 1 || camID == 2){
				sprintf(extFocus::textI7,"%s", "Focus the camera until the marker is visible");		//scaling data from 0 - 100
				cv::putText(img,textI7,cvPoint(pt_title_t.x = 20, pt_title_t.y), CV_FONT_HERSHEY_COMPLEX,1.4,cvScalar(255,0,0),2,CV_AA);
			}else{
				sprintf(extFocus::textI7,"%s", "Focus the camera until the marker is visible");		//scaling data from 0 - 100
				cv::putText(img,textI7,cvPoint(pt_title_t.x = 20, pt_title_t.y), CV_FONT_HERSHEY_COMPLEX,1.4,cvScalar(255,0,0),2,CV_AA);
			}
		}
	}
/*
	sprintf(extFocus::textI7,"%s", camID & 0x80 ?  "AUX_target":"MAIN_target");		//scaling data from 0 - 100
	cv::putText(img,textI7,cvPoint(m_arucoID[0].getCenter().x, m_arucoID[0].getCenter().y), CV_FONT_HERSHEY_COMPLEX,1.5,cvScalar(255,255,0),1,CV_AA);
*/

	cv::imshow(extFocus::text,img);
	cv::moveWindow(extFocus::text, 20, 20);

	displayInstruction(camID);
	cv::moveWindow(extFocus::textI8, 1260, 10);
	cv::imshow(extFocus::textI8, DisImg);

	//for continuous streaming
	key = cv::waitKey(1);
	//For quit streaming
	if (key=='q'){
		char fName[50];
		sprintf(fName,"/home/root/data/calibration/%s_%s_%d.bmp",camID & 0x01 ? "Left":"Right",camID & 0x80 ?  "AUX":"MAIN", numCount++);
		cv::imwrite(fName,img);
		printf("saved %s\n",fName);
		numCount = 0;
		m_quit = true;
		destroyWindow(extFocus::text);
		destroyWindow(extFocus::textI8);

		return m_quit;
		//break;
	}
	//For saving images while streaming individual cameras
	if(key=='s')
	{
		char fName[50];
		sprintf(fName,"%s_%s_%d.bmp",camID & 0x01 ? "Left":"Right",camID & 0x80 ?  "AUX":"MAIN", numCount++);
		cv::imwrite(fName,img);
		printf("saved %s\n",fName);
	}

	//}

	//delete(vs);		// delete instances
	resultFocus.brightAdjImg1.release();
	resultFocus.brightAdjImg2.release();
	resultFocus.brightAdjImg3.release();
	resultFocus.brightAdjImg4.release();
	resultFocus.brightAdjImg5.release();
	return m_quit;
	}

}


std::vector<aruco::Marker> extFocus::detectMarkers(Mat img, int camID, cv::Point ptr1, cv::Point ptr2, int w, int h){

	Mat imgCopy, imgCrop;
	img.copyTo(imgCopy);

	w = w + ptr2.x;
	h = h + ptr2.y;

	cv::Rect ROI(ptr1.x, ptr1.y,w,h);


	aruco::MarkerDetector mDetector;
	aruco::MarkerDetector::MarkerCandidate mCandidate;
	mDetector.setDictionary("ARUCO_MIP_36h12");
	std::vector<aruco::Marker> markers;


	if(!img.empty()){
		if (camID == 4){
			markers = mDetector.detect(imgCopy);
		}
		else{
			imgCrop = img(ROI);
			twoMaxPeak twoPeak = calThreshold(imgCrop.cols,imgCrop.rows,(unsigned char *)(imgCrop.data),50);
			m_thresholdVal = double(twoPeak.peak1 + twoPeak.peak2)/2.0;
			threshold( imgCopy, imgCopy, m_thresholdVal, 255,THRESH_BINARY);
			markers = mDetector.detect(imgCopy);
		}

		//Draw markers with IDs
		for(size_t i = 0; i < markers.size(); i++){
			markers[i].draw(img, cv::Scalar(255,255,0), 5, false, false);
		}


		return markers;
	}
	else{
		printf("There is no Image to detect Aruco-markers!!!\n");
		return markers;
		exit(EXIT_FAILURE);

	}
	cv::imshow("binary", imgCopy);
	cvWaitKey(1);


	imgCopy.release();
	imgCrop.release();
	return markers;
}


extFocus::twoMaxPeak extFocus::calThreshold(int width, int height,unsigned char *dsty, int limit)
{
	//Percentile and Average Calculation
	unsigned char *dy = (unsigned char *)dsty;
	double hist[256]={0};
	int pix=0,i;
	int n = width * height;

	for (; n > 0; n--)
	{
		pix =(int) *dy;
		hist[pix]++;
		dy++;
	}

	float maxCount1 = 0, maxCount2 = 0;
	int maxCntPix1 = 0, maxCntPix2 = 0;
	float dis = float(width) * float(height);

	for(i=0;i<=255;i++)
	{
		float histValue = hist[i];

		if (histValue > maxCount1){
			maxCount1 = histValue;
			maxCntPix1 = i;
		}

		if ((std::abs(histValue - maxCount1) > 0) && (dis > std::abs(histValue - maxCount1))){
			dis = std::abs(histValue - maxCount1);
			maxCount2 = histValue;
			maxCntPix2 = i;
		}
	}

	return twoMaxPeak{maxCntPix1, maxCntPix2};
}

