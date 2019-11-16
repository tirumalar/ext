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



//
void extFocus::DoStartCmd_focus(){

	printf("Setting up Device Parameters\n");

	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");

	int allLEDhighVoltEnable = fconfig.getValue("FTracker.allLEDhighVoltEnable",1);
	int faceLEDVolt = fconfig.getValue("FTracker.faceLEDVolt",30);
	int faceLEDcurrentSet = fconfig.getValue("FTracker.faceLEDcurrentSet",20);
	int faceLEDtrigger = fconfig.getValue("FTracker.faceLEDtrigger",4);
	int faceLEDEnable = fconfig.getValue("FTracker.faceLEDEnable",4);
	int faceLEDmaxTime = fconfig.getValue("FTracker.faceLEDmaxTime",4);


	int AuxIrisCamExposureTime = fconfig.getValue("FTracker.AuxIrisCamExposureTime",8);
	int AuxIrisCamDigitalGain = fconfig.getValue("FTracker.AuxIrisCamDigitalGain",80);
	int AuxIrisCamDataPedestal = fconfig.getValue("FTracker.AuxIrisCamDataPedestal",0);

	int MainIrisCamExposureTime = fconfig.getValue("FTracker.MainIrisCamExposureTime",8);
	int MainIrisCamDigitalGain = fconfig.getValue("FTracker.MainIrisCamDigitalGain",128);
	int MainIrisCamDataPedestal = fconfig.getValue("FTracker.MainIrisCamDataPedestal",0);

	int moveMotor = fconfig.getValue("FTracker.focusMoveCmd",140);		//add focusMoveCmd in faceConfig.ini
	//printf("moveMotor:::: %i\n", moveMotor);

	char cmd[512];

	// first turn off all cameras
	sprintf(cmd,"set_cam_mode(0x0,%d)",100);		//Turn on Alternate cameras
	port_com_send(cmd);


/*	//Homing
	printf("Re Homing\n");
	sprintf(cmd,"fx_home");
	port_com_send(cmd);

	//Move to pos
	sprintf(cmd,"fx_home | fx_abs(%i)",moveMotor);
	port_com_send(cmd);*/


	//Parsing Face LED commands - same as DoStartCmd()
	//sprintf(cmd, "psoc_write(2,%i) | psoc_write(1,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(3,%i)| psoc_write(6,%i)\n",faceLEDVolt, allLEDhighVoltEnable, faceLEDcurrentSet, faceLEDtrigger, faceLEDEnable, faceLEDmaxTime);
	sprintf(cmd,"psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,40) | psoc_write(4,4) | psoc_write(3,3)| psoc_write(6,4)");
	port_com_send(cmd);

	//New Changes
	sprintf(cmd, "psoc_write(9,%i)\n", 80);
	port_com_send(cmd);	// charge cap for max current 60 < range < 95

	port_com_send("wcr(0x1B,0x300C,1650)");		// required before flipping the incoming images
	port_com_send("wcr(0x1B,0x3040,0xC000)"); //Flipping of iris and AUX images

	//Parsing AUX cam settings commands - same as DoStartCmd()
	sprintf(cmd,"wcr(0x03,0x305e,12)|wcr(0x03,0x3012,12)");
	//sprintf(cmd, "wcr(0x03,0x3012,%i) | wcr(0x03,0x301e,%i) | wcr(0x03,0x305e,%i)\n",AuxIrisCamExposureTime, AuxIrisCamDataPedestal, AuxIrisCamDigitalGain);
	port_com_send(cmd);
	printf(cmd);
	//printf("wcr(0x03,0x3012,%i) | wcr(0x03,0x301e,%i) | wcr(0x03,0x305e,%i)\n",AuxIrisCamExposureTime, AuxIrisCamDataPedestal, AuxIrisCamDigitalGain);

	//Parsing Main cam settings commands - same as DoStartCmd()
	sprintf(cmd,"wcr(0x18,0x305e,16)|wcr(0x18,0x3012,8)");
	//sprintf(cmd, "wcr(0x18,0x3012,%i) | wcr(0x18,0x301e,%i) | wcr(0x18,0x305e,%i)\n",MainIrisCamExposureTime, MainIrisCamDataPedestal, MainIrisCamDigitalGain);
	port_com_send(cmd);
	printf(cmd);


	//Parsing Face cam settings commands - same as DoStartCmd()
	sprintf(cmd,"wcr(0x04,0x305e,25)|wcr(0x04,0x3012,6)");
	//sprintf(cmd, "wcr(0x18,0x3012,%i) | wcr(0x18,0x301e,%i) | wcr(0x18,0x305e,%i)\n",MainIrisCamExposureTime, MainIrisCamDataPedestal, MainIrisCamDigitalGain);
	port_com_send(cmd);
	printf(cmd);


	//printf("wcr(0x18,0x3012,%i) | wcr(0x18,0x301e,%i) | wcr(0x18,0x305e,%i)\n",MainIrisCamExposureTime, MainIrisCamDataPedestal, MainIrisCamDigitalGain);

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
	return dst;
}



extFocus::preProcessingImgF extFocus::preProcessingImg_focus(Mat &cropIm){
	int num = 0;
	//adjust brightness with other methods
	//cropIm.copyTo(cropImg);
	//BrightnessAndContrastAuto(cropIm,cropImg,0);
	//adjustBrightnessContrast_clahe(cropIm,cropImg,4);

	//adjusting brightness with Gamma correction
	Mat cropImg;
	double gamma = 0.5;
	adjustBrightnessContrast_GammaCorrection(cropIm,cropImg,gamma);

	Mat blurImg;

	//Tested GaussianBlur and medianBlur but these bluring methods washing out few vertical texture
	//cv::GaussianBlur(cropImg, blurImg,cv::Size(5,5),0,0,BORDER_DEFAULT);
	//cv::medianBlur(cropImg, blurImg,21);


	//bilateralFilter works best as it remove spatial noise by preserving the edges
	int b_kernal =2;
	bilateralFilter(cropImg,blurImg, b_kernal, b_kernal*2, b_kernal/2, BORDER_DEFAULT);


	if (focusDebug){
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

	if (focusDebug){
		sprintf(text, "Gray Img");
		cv::imshow(text,grayImg);
		//cv::imwrite(text,grayImg);
		cvWaitKey(0);
	}

	return {grayImg, cropImg};
}


void extFocus::adjustBrightnessContrast_GammaCorrection(Mat &src, Mat &out, double gamma){
	//gamma lesser then 1.0 will make the image darker (shift the histogram toward left) but high contrast zone will be clealry visible
	// but it will miss all the lower level of texture changes
	// gamma value greater then 1.0 will make the image brighter and high and low contrast changes will be clearly visible
	Mat lookUpTable(1,256,CV_8U);	//initiate a lookup table 1 - 256
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

    //to calculate grayscale histogram
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
        cv::Mat hist; //the grayscale histogram

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


	if (focusDebug){
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

	if (focusDebug){
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

	if (focusDebug){
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

	if (focusDebug){
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
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN, hScale,vScale,0,lineWidth);

	//printf("Mean:: %4.4f	std:: %4.4f	Var:: %4.4f\n", mean.val[0], sigma.val[0],sigma.val[0]*sigma.val[0]);
	//sprintf(textI1,"Mean: %0.2f std: %0.2f Var: %0.2f", mean.val[0], sigma.val[0],sigma.val[0]*sigma.val[0]);
	//sprintf(textI1,"Mean: %0.2f std: %0.2f", mean.val[0], sigma.val[0]);
	sprintf(textI1,"focus val mean: %0.2f    Var:: %0.2f ", float((mean.val[0]+0.005) * 100), float(((sigma.val[0]*sigma.val[0]) + 0.005) * 1000));
	//sprintf(textI1,"focus val mean: %4.4f    Var:: %4.4f ", float((mean.val[0]+0.0000) * 1), float((sigma.val[0]*sigma.val[0]) * 1));
	//focusScore << mean.val[0] * 100 << "," << sigma.val[0] << "," << sigma.val[0]*sigma.val[0] << endl;

	//cv::putText(grayImg,textI1,cvPoint(5,20), CV_FONT_HERSHEY_PLAIN,1.0,cvScalar(255,255,255),1,CV_AA);
	sprintf(text, "focus Measure in Rect");
	cv::namedWindow(text);
	cv::imwrite("imgCheck.bmp",cropImg);
	cv::imshow("imgCheck.bmp",cropImg);
	cv::rectangle(img, crop, Scalar(255, 0, 0), 1, 0);
	cv::putText(img,textI1,cvPoint(crop.x,crop.y - 30), CV_FONT_HERSHEY_PLAIN,1.0,cvScalar(255,0,0),1,CV_AA);
	cv::imshow(text,img);*/

}


extFocus::measureFocusOut extFocus::measureFocus(Mat img, int camID, int width, int height, int x1, int y1, int x2, int y2){

	if(img.empty())
		printf("Error reading image!\n");

	Mat cropImg, cropIm;
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
	}

	//Root mean square deviation of four corner values will detect crooked lens

	return out;

/*	CvFont font;
	double hScale=.1;
	double vScale=.1;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN, hScale,vScale,0,lineWidth);

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
	cv::putText(img,textI1,cvPoint(crop.x,crop.y - 10), CV_FONT_HERSHEY_PLAIN,1.0,cvScalar(255,0,0),1,CV_AA);
	cv::imshow(text,img);*/


}


bool extFocus::camControlFocus(Mat &img,int camID){
	//int w,h;
	char key;
	//bool quit = false;
/*
	VideoStream *vs;
	vs = new VideoStream(cam);
	vs->flush();
	vs->get(&w,&h,(char *)img.data);
	vs->get(&w,&h,(char *)img.data);
*/

	int height = 100;
	int width = 180;

	int x1,y1,x2,y2;	//co-ordinates for corner Rect



	measureFocusOut resultFocus;

	CvFont font;
	double hScale=.2;
	double vScale=.2;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN, hScale,vScale,0,lineWidth);

	//while(1){
	//vs->get(&w,&h,(char *)img.data);

	if (camID & 0x80 ?  1:0){
		x1 = 10, y1 = 50, x2 = 1020, y2 = 800;
		resultFocus = measureFocus(img, camID, width, height, x1, y1, x2, y2);
	}
	else{
		x1 = 30, y1 = 70, x2 = 860, y2 = 750;
		resultFocus = measureFocus(img, camID, width, height, x1, y1, x2, y2);
	}
	//resultFocus = measureFocus(img, vs->cam_id, width, height, x1, y1, x2, y2);

	if(camID == 8194){
		x1 = 450, y1 = 540, x2 = 540, y2 = 620;
		Mat rotMat = rotate90(img);
		//printf("Successfully rotate image\n");
		width = 65;
		height = 45;
		resultFocus = measureFocus(rotMat, camID, width, height, x1, y1, x2, y2);


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
		sprintf(extFocus::textI6,"Target Focus: %i", scaleRare);		//scaling data from 0 - 100
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

		cv::putText(rotMat,textI1,cvPoint(resultFocus.ROI1.x - 180,resultFocus.ROI1.y - 0), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(rotMat,textI2,cvPoint(resultFocus.ROI2.x + 140,resultFocus.ROI2.y - 0), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(rotMat,textI3,cvPoint(resultFocus.ROI3.x - 180,resultFocus.ROI3.y - 0), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(rotMat,textI4,cvPoint(resultFocus.ROI4.x + 140,resultFocus.ROI4.y - 0), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		//cv::putText(rotMat,textI5,cvPoint(resultFocus.ROI5.x - 50,resultFocus.ROI5.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(rotMat,textI6,cvPoint(img.rows/2.0, 50), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		//cv::putText(rotMat,textI7,cvPoint(img.rows/2.0, 100), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);

		cv::imshow(extFocus::text,rotMat);
		cv::moveWindow(extFocus::text, 20, 20);

		//for continuous streaming
		key = cv::waitKey(1);
		//For quit streaming
		if (key=='q'){
			char fName[50];
			sprintf(fName,"/home/root/data/calibration/%s_%d.bmp","FaceCam", numCount++);
			cv::imwrite(fName,rotMat);
			printf("saved %s\n",fName);
			numCount = 0;
			quit = true;
			destroyWindow(extFocus::text);

			return quit;
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

		return quit;
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
	sprintf(extFocus::textI6,"Target Focus: %i", multScaled);		//scaling data from 0 - 100
	//sprintf(extFocus::textI7,"Mult1: %i", mult);		//scaling data from 0 - 10,000

	//for laplacian
	//sprintf(textI1,"focus val mean: %0.2f    Var:: %0.2f ", float((matric.mean+0.005) * 100), float(((matric.sigma*matric.sigma) + 0.005) * 1000));


	sprintf(extFocus::text,"Focusing %s %s Camera ",camID & 0x01 ? "Left":"Right",camID & 0x80 ?  "AUX":"MAIN");
	cv::namedWindow(extFocus::text);

	//Uncomment the following two lines if brightness change effects need to check
	//cv::imwrite("imgCheck.bmp",resultFocus.brightAdjImg);
	//cv::imshow("imgCheck.bmp",resultFocus.brightAdjImg);

	cv::rectangle(img, resultFocus.ROI1, Scalar(255, 0, 0), 1, 0);
	cv::rectangle(img, resultFocus.ROI2, Scalar(255, 0, 0), 1, 0);
	cv::rectangle(img, resultFocus.ROI3, Scalar(255, 0, 0), 1, 0);
	cv::rectangle(img, resultFocus.ROI4, Scalar(255, 0, 0), 1, 0);
	cv::rectangle(img, resultFocus.ROI5, Scalar(255, 0, 0), 1, 0);

	cv::putText(img,textI1,cvPoint(resultFocus.ROI1.x,resultFocus.ROI1.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI2,cvPoint(resultFocus.ROI2.x,resultFocus.ROI2.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI3,cvPoint(resultFocus.ROI3.x,resultFocus.ROI3.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI4,cvPoint(resultFocus.ROI4.x,resultFocus.ROI4.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI5,cvPoint(resultFocus.ROI5.x,resultFocus.ROI5.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI6,cvPoint(img.cols/2.0, 50), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
	//cv::putText(img,textI7,cvPoint(img.cols/2.0, 100), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);

	cv::imshow(extFocus::text,img);
	cv::moveWindow(extFocus::text, 20, 20);

	//for continuous streaming
	key = cv::waitKey(1);
	//For quit streaming
	if (key=='q'){
		char fName[50];
		sprintf(fName,"/home/root/data/calibration/%s_%s_%d.bmp",camID & 0x01 ? "Left":"Right",camID & 0x80 ?  "AUX":"MAIN", numCount++);
		cv::imwrite(fName,img);
		printf("saved %s\n",fName);
		numCount = 0;
		quit = true;
		destroyWindow(extFocus::text);

		return quit;
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

	return quit;
	}

}


/*void camControlFocus(int cam){
	int w,h;
	char key;

	VideoStream *vs;
	vs = new VideoStream(cam);
	vs->flush();
	vs->get(&w,&h,(char *)img.data);
	vs->get(&w,&h,(char *)img.data);

	int height = 100;
	int width = 180;

	int x1,y1,x2,y2;	//co-ordinates for corner Rect



	measureFocusOut resultFocus;

	CvFont font;
	double hScale=.2;
	double vScale=.2;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN, hScale,vScale,0,lineWidth);

	while(1){
		vs->get(&w,&h,(char *)img.data);

		if (vs->cam_id & 0x80 ?  1:0){
			x1 = 5, y1 = 50, x2 = 1010, y2 = 800;
			resultFocus = measureFocus(img, vs->cam_id, width, height, x1, y1, x2, y2);
		}
		else{
			x1 = 30, y1 = 70, x2 = 860, y2 = 750;
			resultFocus = measureFocus(img, vs->cam_id, width, height, x1, y1, x2, y2);
		}
		//resultFocus = measureFocus(img, vs->cam_id, width, height, x1, y1, x2, y2);


		//for sobel
		sprintf(textI1,"focus: %0.0f", float((resultFocus.mean1+0.0005) * 100));		//scaling data from 0 - 100
		sprintf(textI2,"focus: %0.0f", float((resultFocus.mean2+0.0005) * 100));		//scaling data from 0 - 100
		sprintf(textI3,"focus: %0.0f", float((resultFocus.mean3+0.0005) * 100));		//scaling data from 0 - 100
		sprintf(textI4,"focus: %0.0f", float((resultFocus.mean4+0.0005) * 100));		//scaling data from 0 - 100
		sprintf(textI5,"focus: %0.0f", float((resultFocus.mean5+0.0005) * 100));		//scaling data from 0 - 100

		//for laplacian
		//sprintf(textI1,"focus val mean: %0.2f    Var:: %0.2f ", float((matric.mean+0.005) * 100), float(((matric.sigma*matric.sigma) + 0.005) * 1000));


		sprintf(text,"Focusing %s %s Camera ",vs->cam_id & 0x01 ? "Left":"Right",vs->cam_id & 0x80 ?  "AUX":"MAIN");
		cv::namedWindow(text);

		//Uncomment the following two lines if brightness change effects need to check
		//cv::imwrite("imgCheck.bmp",resultFocus.brightAdjImg);
		//cv::imshow("imgCheck.bmp",resultFocus.brightAdjImg);

		cv::rectangle(img, resultFocus.ROI1, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(img, resultFocus.ROI2, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(img, resultFocus.ROI3, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(img, resultFocus.ROI4, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(img, resultFocus.ROI5, Scalar(255, 0, 0), 1, 0);

		cv::putText(img,textI1,cvPoint(resultFocus.ROI1.x,resultFocus.ROI1.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(img,textI2,cvPoint(resultFocus.ROI2.x,resultFocus.ROI2.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(img,textI3,cvPoint(resultFocus.ROI3.x,resultFocus.ROI3.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(img,textI4,cvPoint(resultFocus.ROI4.x,resultFocus.ROI4.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(img,textI5,cvPoint(resultFocus.ROI5.x,resultFocus.ROI5.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);

		cv::imshow(text,img);

		//for continuous streaming
	    key = cv::waitKey(1);
	    //For quit streaming
		if (key=='q'){
			char fName[50];
			sprintf(fName,"/home/root/data/calibration/%s_%s_%d.bmp",vs->cam_id & 0x01 ? "Left":"Right",vs->cam_id & 0x80 ?  "AUX":"MAIN", numCount++);
			cv::imwrite(fName,img);
			printf("saved %s\n",fName);
			numCount = 0;
			destroyWindow(text);
			vs->flush();
			break;
		}
		//For saving images while streaming individual cameras
		if(key=='s')
		{
			char fName[50];
			sprintf(fName,"%s_%s_%d.bmp",vs->cam_id & 0x01 ? "Left":"Right",vs->cam_id & 0x80 ?  "AUX":"MAIN", numCount++);
			cv::imwrite(fName,img);
			printf("saved %s\n",fName);
		}

	}

	delete(vs);		// delete instances

}*/

/*void RunCamFocus(){

	char cmd[512];

	//Set AUX cameras
	sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);
	port_com_send(cmd);


	//Fetching images from Face camera
	int leftCam = 8192;
	int rightCam = 8193;

	camControlFocus(leftCam);
	camControlFocus(rightCam);

	//Set Main cameras
	sprintf(cmd,"set_cam_mode(0x07,%d)",FRAME_DELAY);
	port_com_send(cmd);
	usleep(1000);

	camControlFocus(leftCam);
	camControlFocus(rightCam);

}*/









/*


void DoStartCmd_focus(){

	FileConfiguration fconfig("/home/root/data/calibration/faceConfig.ini");

	int allLEDhighVoltEnable = fconfig.getValue("FTracker.allLEDhighVoltEnable",1);
	int faceLEDVolt = fconfig.getValue("FTracker.faceLEDVolt",30);
	int faceLEDcurrentSet = fconfig.getValue("FTracker.faceLEDcurrentSet",20);
	int faceLEDtrigger = fconfig.getValue("FTracker.faceLEDtrigger",4);
	int faceLEDEnable = fconfig.getValue("FTracker.faceLEDEnable",4);
	int faceLEDmaxTime = fconfig.getValue("FTracker.faceLEDmaxTime",4);


	int AuxIrisCamExposureTime = fconfig.getValue("FTracker.AuxIrisCamExposureTime",8);
	int AuxIrisCamDigitalGain = fconfig.getValue("FTracker.AuxIrisCamDigitalGain",80);
	int AuxIrisCamDataPedestal = fconfig.getValue("FTracker.AuxIrisCamDataPedestal",0);

	int MainIrisCamExposureTime = fconfig.getValue("FTracker.MainIrisCamExposureTime",8);
	int MainIrisCamDigitalGain = fconfig.getValue("FTracker.MainIrisCamDigitalGain",128);
	int MainIrisCamDataPedestal = fconfig.getValue("FTracker.MainIrisCamDataPedestal",0);

	int moveMotor = fconfig.getValue("FTracker.focusMoveCmd",140);

	char cmd[512];

	// first turn off all cameras
	sprintf(cmd,"set_cam_mode(0x0,%d)",100);		//Turn on Alternate cameras
	port_com_send(cmd);

	//Move to pos
	sprintf(cmd,"fx_home | fx_abs(%i)",moveMotor);
	port_com_send(cmd);

	//Parsing Face LED commands - same as DoStartCmd()
	sprintf(cmd, "psoc_write(2,%i) | psoc_write(1,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(3,%i)| psoc_write(6,%i)\n",faceLEDVolt, allLEDhighVoltEnable, faceLEDcurrentSet, faceLEDtrigger, faceLEDEnable, faceLEDmaxTime);
	//sprintf(cmd,"psoc_write(2,30) | psoc_write(1,1) | psoc_write(5,20) | psoc_write(4,4) | psoc_write(3,4)| psoc_write(6,4)");
	port_com_send(cmd);

	//Parsing AUX cam settings commands - same as DoStartCmd()
	//sprintf(cmd,"wcr(0x03,0x305e,90)|wcr(3,0x3012,24)");
	sprintf(cmd, "wcr(0x03,0x3012,%i) | wcr(0x03,0x301e,%i) | wcr(0x03,0x305e,%i)\n",AuxIrisCamExposureTime, AuxIrisCamDataPedestal, AuxIrisCamDigitalGain);
	port_com_send(cmd);
	printf("wcr(0x03,0x3012,%i) | wcr(0x03,0x301e,%i) | wcr(0x03,0x305e,%i)\n",AuxIrisCamExposureTime, AuxIrisCamDataPedestal, AuxIrisCamDigitalGain);

	//Parsing Main cam settings commands - same as DoStartCmd()
	//sprintf(cmd,"wcr(0x03,0x305e,90)|wcr(3,0x3012,24)");
	sprintf(cmd, "wcr(0x18,0x3012,%i) | wcr(0x18,0x301e,%i) | wcr(0x18,0x305e,%i)\n",MainIrisCamExposureTime, MainIrisCamDataPedestal, MainIrisCamDigitalGain);
	port_com_send(cmd);
	printf("wcr(0x18,0x3012,%i) | wcr(0x18,0x301e,%i) | wcr(0x18,0x305e,%i)\n",MainIrisCamExposureTime, MainIrisCamDataPedestal, MainIrisCamDigitalGain);

}


preProcessingImgF preProcessingImg_focus(Mat &cropIm){
	int num = 0;
	//adjust brightness with other methods
	//cropIm.copyTo(cropImg);
	//BrightnessAndContrastAuto(cropIm,cropImg,0);
	//adjustBrightnessContrast_clahe(cropIm,cropImg,4);

	//adjusting brightness with Gamma correction
	Mat cropImg;
	double gamma = 0.5;
	adjustBrightnessContrast_GammaCorrection(cropIm,cropImg,gamma);

	Mat blurImg;

	//Tested GaussianBlur and medianBlur but these bluring methods washing out few vertical texture
	//cv::GaussianBlur(cropImg, blurImg,cv::Size(5,5),0,0,BORDER_DEFAULT);
	//cv::medianBlur(cropImg, blurImg,21);


	//bilateralFilter works best as it remove spatial noise by preserving the edges
	int b_kernal =3;
	bilateralFilter(cropImg,blurImg, b_kernal, b_kernal*2, b_kernal/2, BORDER_DEFAULT);


	if (focusDebug){
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

	if (focusDebug){
		sprintf(text, "Gray Img");
		cv::imshow(text,grayImg);
		//cv::imwrite(text,grayImg);
		cvWaitKey(0);
	}

	return {grayImg, cropImg};
}


void adjustBrightnessContrast_GammaCorrection(Mat &src, Mat &out, double gamma){
	//gamma lesser then 1.0 will make the image darker (shift the histogram toward left) but high contrast zone will be clealry visible
	// but it will miss all the lower level of texture changes
	// gamma value greater then 1.0 will make the image brighter and high and low contrast changes will be clearly visible
	Mat lookUpTable(1,256,CV_8U);	//initiate a lookup table 1 - 256
	uchar* x = lookUpTable.ptr();

	for (int i = 0; i < 256; i++){
		//convert 1-256 into 0.0-1.0 and apply gamma like norm(I(x,y))^gamma
		//then change the conversion into 0-255 again
		x[i] = saturate_cast<uchar> (pow(i/255.0, gamma) * 255.0);
	}

	//transform src image based on lookUpTable vals
	cv::LUT(src, lookUpTable, out);
}


void adjustBrightnessContrast_clahe(Mat &src, Mat &dst, double clipLimit){

    // apply the CLAHE algorithm to the L channel
    cv::Ptr<cv::CLAHE> clahe = cv::createCLAHE();
    clahe->setClipLimit(clipLimit);
    clahe->apply(src, dst);
}


void BrightnessAndContrastAuto(const cv::Mat &src, cv::Mat &dst, float clipHistPercent)
{

    int histSize = 256;
    float alpha, beta;
    double minGray = 0, maxGray = 0;

    //to calculate grayscale histogram
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
        cv::Mat hist; //the grayscale histogram

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


focusMatric sobelBasedFocusMeasure(Mat &cropIm){
	int num = 0;
	//adjust brightness with other methods
	//cropIm.copyTo(cropImg);
	//BrightnessAndContrastAuto(cropIm,cropImg,0);
	//adjustBrightnessContrast_clahe(cropIm,cropImg,4);

	//adjusting brightness with Gamma correction
	Mat cropImg;
	double gamma = 0.5;
	adjustBrightnessContrast_GammaCorrection(cropIm,cropImg,gamma);

	Mat blurImg;

	//Tested GaussianBlur and medianBlur but these bluring methods washing out few vertical texture
	//cv::GaussianBlur(cropImg, blurImg,cv::Size(5,5),0,0,BORDER_DEFAULT);
	//cv::medianBlur(cropImg, blurImg,21);


	//bilateralFilter works best as it remove spatial noise by preserving the edges
	int b_kernal =3;
	bilateralFilter(cropImg,blurImg, b_kernal, b_kernal*2, b_kernal/2, BORDER_DEFAULT);


	if (focusDebug){
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

	if (focusDebug){
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



	//Dont need to look for y direction as we dont have any edges there
	//if we keep this activate, for un-focus images it will peak the noise as horizental texture
	x_order = 0;	//de-activate x directional feature
	y_order = 1;	//active y directional feature
	cv::Sobel(grayImg,y_gradImg,depth,x_order,y_order,kernal,scale,delta,BORDER_DEFAULT);
	cv::convertScaleAbs(y_gradImg,scaledImgY);	//cal absolute val and convert to 8-bit integers


	if (focusDebug){
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

	return {float(mean.val[0]),float(sigma.val[0])};
}


focusMatric laplacianBasedFocusMeasure(Mat &grayImg){
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

	if (focusDebug){
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

	CvFont font;
	double hScale=.1;
	double vScale=.1;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN, hScale,vScale,0,lineWidth);

	//printf("Mean:: %4.4f	std:: %4.4f	Var:: %4.4f\n", mean.val[0], sigma.val[0],sigma.val[0]*sigma.val[0]);
	//sprintf(textI1,"Mean: %0.2f std: %0.2f Var: %0.2f", mean.val[0], sigma.val[0],sigma.val[0]*sigma.val[0]);
	//sprintf(textI1,"Mean: %0.2f std: %0.2f", mean.val[0], sigma.val[0]);
	sprintf(textI1,"focus val mean: %0.2f    Var:: %0.2f ", float((mean.val[0]+0.005) * 100), float(((sigma.val[0]*sigma.val[0]) + 0.005) * 1000));
	//sprintf(textI1,"focus val mean: %4.4f    Var:: %4.4f ", float((mean.val[0]+0.0000) * 1), float((sigma.val[0]*sigma.val[0]) * 1));
	//focusScore << mean.val[0] * 100 << "," << sigma.val[0] << "," << sigma.val[0]*sigma.val[0] << endl;

	//cv::putText(grayImg,textI1,cvPoint(5,20), CV_FONT_HERSHEY_PLAIN,1.0,cvScalar(255,255,255),1,CV_AA);
	sprintf(text, "focus Measure in Rect");
	cv::namedWindow(text);
	cv::imwrite("imgCheck.bmp",cropImg);
	cv::imshow("imgCheck.bmp",cropImg);
	cv::rectangle(img, crop, Scalar(255, 0, 0), 1, 0);
	cv::putText(img,textI1,cvPoint(crop.x,crop.y - 30), CV_FONT_HERSHEY_PLAIN,1.0,cvScalar(255,0,0),1,CV_AA);
	cv::imshow(text,img);

}


measureFocusOut measureFocus(Mat img, int camID, int width, int height, int x1, int y1, int x2, int y2){

	if(img.empty())
		printf("Error reading image!\n");

	Mat cropImg, cropIm;
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
	}

	//Root mean square deviation of four corner values will detect crooked lens

	return out;

	CvFont font;
	double hScale=.1;
	double vScale=.1;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN, hScale,vScale,0,lineWidth);

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
	cv::putText(img,textI1,cvPoint(crop.x,crop.y - 10), CV_FONT_HERSHEY_PLAIN,1.0,cvScalar(255,0,0),1,CV_AA);
	cv::imshow(text,img);


}


bool camControlFocus(Mat &img,int camID){
	//int w,h;
	char key;
	//bool quit = false;

	VideoStream *vs;
	vs = new VideoStream(cam);
	vs->flush();
	vs->get(&w,&h,(char *)img.data);
	vs->get(&w,&h,(char *)img.data);


	int height = 100;
	int width = 180;

	int x1,y1,x2,y2;	//co-ordinates for corner Rect



	measureFocusOut resultFocus;

	CvFont font;
	double hScale=.2;
	double vScale=.2;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN, hScale,vScale,0,lineWidth);

	//while(1){
	//vs->get(&w,&h,(char *)img.data);

	if (camID & 0x80 ?  1:0){
		x1 = 5, y1 = 50, x2 = 1010, y2 = 800;
		resultFocus = measureFocus(img, camID, width, height, x1, y1, x2, y2);
	}
	else{
		x1 = 30, y1 = 70, x2 = 860, y2 = 750;
		resultFocus = measureFocus(img, camID, width, height, x1, y1, x2, y2);
	}
	//resultFocus = measureFocus(img, vs->cam_id, width, height, x1, y1, x2, y2);


	//for sobel
	sprintf(textI1,"focus: %0.0f", float((resultFocus.mean1+0.0005) * 100));		//scaling data from 0 - 100
	sprintf(textI2,"focus: %0.0f", float((resultFocus.mean2+0.0005) * 100));		//scaling data from 0 - 100
	sprintf(textI3,"focus: %0.0f", float((resultFocus.mean3+0.0005) * 100));		//scaling data from 0 - 100
	sprintf(textI4,"focus: %0.0f", float((resultFocus.mean4+0.0005) * 100));		//scaling data from 0 - 100
	sprintf(textI5,"focus: %0.0f", float((resultFocus.mean5+0.0005) * 100));		//scaling data from 0 - 100

	//for laplacian
	//sprintf(textI1,"focus val mean: %0.2f    Var:: %0.2f ", float((matric.mean+0.005) * 100), float(((matric.sigma*matric.sigma) + 0.005) * 1000));


	sprintf(text,"Focusing %s %s Camera ",camID & 0x01 ? "Left":"Right",camID & 0x80 ?  "AUX":"MAIN");
	cv::namedWindow(text);

	//Uncomment the following two lines if brightness change effects need to check
	//cv::imwrite("imgCheck.bmp",resultFocus.brightAdjImg);
	//cv::imshow("imgCheck.bmp",resultFocus.brightAdjImg);

	cv::rectangle(img, resultFocus.ROI1, Scalar(255, 0, 0), 1, 0);
	cv::rectangle(img, resultFocus.ROI2, Scalar(255, 0, 0), 1, 0);
	cv::rectangle(img, resultFocus.ROI3, Scalar(255, 0, 0), 1, 0);
	cv::rectangle(img, resultFocus.ROI4, Scalar(255, 0, 0), 1, 0);
	cv::rectangle(img, resultFocus.ROI5, Scalar(255, 0, 0), 1, 0);

	cv::putText(img,textI1,cvPoint(resultFocus.ROI1.x,resultFocus.ROI1.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI2,cvPoint(resultFocus.ROI2.x,resultFocus.ROI2.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI3,cvPoint(resultFocus.ROI3.x,resultFocus.ROI3.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI4,cvPoint(resultFocus.ROI4.x,resultFocus.ROI4.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
	cv::putText(img,textI5,cvPoint(resultFocus.ROI5.x,resultFocus.ROI5.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);

	cv::imshow(text,img);

	//for continuous streaming
	key = cv::waitKey(1);
	//For quit streaming
	if (key=='q'){
		char fName[50];
		sprintf(fName,"/home/root/data/calibration/%s_%s_%d.bmp",camID & 0x01 ? "Left":"Right",camID & 0x80 ?  "AUX":"MAIN", numCount++);
		cv::imwrite(fName,img);
		printf("saved %s\n",fName);
		numCount = 0;
		quit = true;
		destroyWindow(text);

		return quit;
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

	return quit;

}


void camControlFocus(int cam){
	int w,h;
	char key;

	VideoStream *vs;
	vs = new VideoStream(cam);
	vs->flush();
	vs->get(&w,&h,(char *)img.data);
	vs->get(&w,&h,(char *)img.data);

	int height = 100;
	int width = 180;

	int x1,y1,x2,y2;	//co-ordinates for corner Rect



	measureFocusOut resultFocus;

	CvFont font;
	double hScale=.2;
	double vScale=.2;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_PLAIN, hScale,vScale,0,lineWidth);

	while(1){
		vs->get(&w,&h,(char *)img.data);

		if (vs->cam_id & 0x80 ?  1:0){
			x1 = 5, y1 = 50, x2 = 1010, y2 = 800;
			resultFocus = measureFocus(img, vs->cam_id, width, height, x1, y1, x2, y2);
		}
		else{
			x1 = 30, y1 = 70, x2 = 860, y2 = 750;
			resultFocus = measureFocus(img, vs->cam_id, width, height, x1, y1, x2, y2);
		}
		//resultFocus = measureFocus(img, vs->cam_id, width, height, x1, y1, x2, y2);


		//for sobel
		sprintf(textI1,"focus: %0.0f", float((resultFocus.mean1+0.0005) * 100));		//scaling data from 0 - 100
		sprintf(textI2,"focus: %0.0f", float((resultFocus.mean2+0.0005) * 100));		//scaling data from 0 - 100
		sprintf(textI3,"focus: %0.0f", float((resultFocus.mean3+0.0005) * 100));		//scaling data from 0 - 100
		sprintf(textI4,"focus: %0.0f", float((resultFocus.mean4+0.0005) * 100));		//scaling data from 0 - 100
		sprintf(textI5,"focus: %0.0f", float((resultFocus.mean5+0.0005) * 100));		//scaling data from 0 - 100

		//for laplacian
		//sprintf(textI1,"focus val mean: %0.2f    Var:: %0.2f ", float((matric.mean+0.005) * 100), float(((matric.sigma*matric.sigma) + 0.005) * 1000));


		sprintf(text,"Focusing %s %s Camera ",vs->cam_id & 0x01 ? "Left":"Right",vs->cam_id & 0x80 ?  "AUX":"MAIN");
		cv::namedWindow(text);

		//Uncomment the following two lines if brightness change effects need to check
		//cv::imwrite("imgCheck.bmp",resultFocus.brightAdjImg);
		//cv::imshow("imgCheck.bmp",resultFocus.brightAdjImg);

		cv::rectangle(img, resultFocus.ROI1, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(img, resultFocus.ROI2, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(img, resultFocus.ROI3, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(img, resultFocus.ROI4, Scalar(255, 0, 0), 1, 0);
		cv::rectangle(img, resultFocus.ROI5, Scalar(255, 0, 0), 1, 0);

		cv::putText(img,textI1,cvPoint(resultFocus.ROI1.x,resultFocus.ROI1.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(img,textI2,cvPoint(resultFocus.ROI2.x,resultFocus.ROI2.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(img,textI3,cvPoint(resultFocus.ROI3.x,resultFocus.ROI3.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(img,textI4,cvPoint(resultFocus.ROI4.x,resultFocus.ROI4.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);
		cv::putText(img,textI5,cvPoint(resultFocus.ROI5.x,resultFocus.ROI5.y - 10), CV_FONT_HERSHEY_PLAIN,1.5,cvScalar(255,0,0),1,CV_AA);

		cv::imshow(text,img);

		//for continuous streaming
	    key = cv::waitKey(1);
	    //For quit streaming
		if (key=='q'){
			char fName[50];
			sprintf(fName,"/home/root/data/calibration/%s_%s_%d.bmp",vs->cam_id & 0x01 ? "Left":"Right",vs->cam_id & 0x80 ?  "AUX":"MAIN", numCount++);
			cv::imwrite(fName,img);
			printf("saved %s\n",fName);
			numCount = 0;
			destroyWindow(text);
			vs->flush();
			break;
		}
		//For saving images while streaming individual cameras
		if(key=='s')
		{
			char fName[50];
			sprintf(fName,"%s_%s_%d.bmp",vs->cam_id & 0x01 ? "Left":"Right",vs->cam_id & 0x80 ?  "AUX":"MAIN", numCount++);
			cv::imwrite(fName,img);
			printf("saved %s\n",fName);
		}

	}

	delete(vs);		// delete instances

}

void RunCamFocus(){

	char cmd[512];

	//Set AUX cameras
	sprintf(cmd,"set_cam_mode(0x87,%d)",FRAME_DELAY);
	port_com_send(cmd);


	//Fetching images from Face camera
	int leftCam = 8192;
	int rightCam = 8193;

	camControlFocus(leftCam);
	camControlFocus(rightCam);

	//Set Main cameras
	sprintf(cmd,"set_cam_mode(0x07,%d)",FRAME_DELAY);
	port_com_send(cmd);
	usleep(1000);

	camControlFocus(leftCam);
	camControlFocus(rightCam);

}





*/


