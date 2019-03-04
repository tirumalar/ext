/*
 * geomtericCalibration.cpp
 *
 *  Created on: Feb 20, 2019
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
#include "geomtericCalibration.h"
#include "logging.h"

#include <aruco.h>
#include <dictionary.h>

using namespace cv;
using namespace std;

const char logger[30] = "ExtSetUp";

geomtericCalibration::geomtericCalibration(char* filename)
:geoConfig(filename)
,m_allLEDhighVoltEnable(0)
,m_calibVolt(0)
,m_calibCurrent(0)
,m_calibTrigger(0)
,m_calibLEDEnable(0)
,m_calibLEDMaxTime(0)
,m_faceCamExposureTime(0)
,m_faceCamDigitalGain(0)
,m_faceCamDataPedestal(0)
,m_AuxIrisCamExposureTime(0)
,m_AuxIrisCamDigitalGain(0)
,m_AuxIrisCamDataPedestal(0)
,m_MainIrisCamExposureTime(0)
,m_MainIrisCamDigitalGain(0)
,m_MainIrisCamDataPedestal(0)
,m_thresholdVal(0)
,m_calDebug(false)
,m_calTwoPoint(false)
,rightCam(0)
,leftCam(0)
,faceCam(0)
,m_agc_val_cal(0)
,numCount(0)
{
	m_calDebug = geoConfig.getValue("FTracker.calDebug",false);
	m_calTwoPoint = geoConfig.getValue("FTracker.twoPointCalibration",true);

	m_allLEDhighVoltEnable = geoConfig.getValue("FTracker.allLEDhighVoltEnable",1);

	m_calibVolt = geoConfig.getValue("FTracker.calibVolt",30);
	m_calibCurrent = geoConfig.getValue("FTracker.calibCurrent",30);
	m_calibTrigger = geoConfig.getValue("FTracker.calibTrigger",1);
	m_calibLEDEnable = geoConfig.getValue("FTracker.calibLEDEnable",1);
	m_calibLEDMaxTime = geoConfig.getValue("FTracker.calibLEDMaxTime",4);

	m_faceCamExposureTime = geoConfig.getValue("FTracker.calibFaceCamExposureTime",2);
	m_faceCamDigitalGain = geoConfig.getValue("FTracker.calibFaceCamDigitalGain",48);
	m_faceCamDataPedestal = geoConfig.getValue("FTracker.calibFaceCamDataPedestal",0);

	m_AuxIrisCamExposureTime = geoConfig.getValue("FTracker.calibAuxIrisCamExposureTime",3);
	m_AuxIrisCamDigitalGain = geoConfig.getValue("FTracker.calibAuxIrisCamDigitalGain",64);
	m_AuxIrisCamDataPedestal = geoConfig.getValue("FTracker.calibAuxIrisCamDataPedestal",0);

	m_MainIrisCamExposureTime = geoConfig.getValue("FTracker.calibMainIrisCamExposureTime",3);
	m_MainIrisCamDigitalGain = geoConfig.getValue("FTracker.calibMainIrisCamDigitalGain",64);
	m_MainIrisCamDataPedestal = geoConfig.getValue("FTracker.calibMainIrisCamDataPedestal",0);

	rightCam = 8193;
	leftCam = 8192;
	faceCam = 8194;
}


geomtericCalibration::~geomtericCalibration(){
	smallImg.release();
	imgCopy.release();
	imgCopy1.release();
}


void geomtericCalibration::DoStartCmd_CamCal(){
	char cmd[512];

	//configuration of LEDs
	sprintf(cmd, "psoc_write(2,%i) | psoc_write(1,%i) | psoc_write(5,%i) | psoc_write(4,%i) | psoc_write(3,%i)| psoc_write(6,%i)\n", m_calibVolt, m_allLEDhighVoltEnable, m_calibCurrent, m_calibTrigger, m_calibLEDEnable, m_calibLEDMaxTime);
	port_com_send(cmd);

	//Setting up cap current
	port_com_send("psoc_write(9,80)");	// charge cap for max current 60 < range < 95
	port_com_send("wcr(0x1B,0x300C,1650)");		// required before flipping the incoming images
	port_com_send("wcr(0x1B,0x3040,0xC000)"); //Flipping of iris and AUX images


	//Face cameras configuration
	sprintf(cmd, "wcr(0x04,0x3012,%i) | wcr(0x04,0x301e,%i) | wcr(0x04,0x305e,%i)\n",m_faceCamExposureTime, m_faceCamDataPedestal, m_faceCamDigitalGain);
	port_com_send(cmd);

	//AUX cameras configuration
	sprintf(cmd, "wcr(0x03,0x3012,%i) | wcr(0x03,0x301e,%i) | wcr(0x03,0x305e,%i)\n",m_AuxIrisCamExposureTime, m_AuxIrisCamDataPedestal, m_AuxIrisCamDigitalGain);
	port_com_send(cmd);

	//Main Iris Cameras Configuration
	sprintf(cmd, "wcr(0x18,0x3012,%i) | wcr(0x18,0x301e,%i) | wcr(0x18,0x305e,%i)\n",m_MainIrisCamExposureTime, m_MainIrisCamDataPedestal, m_MainIrisCamDigitalGain);
	port_com_send(cmd);


	// setup up all pll values
	//following process will activate PLL for all cameras
	sprintf(cmd,"set_cam_mode(0x00,%d)",10);	//turn off the cameras before changing PLL
	cvWaitKey(100);								//Wait for 100 msec
	port_com_send("wcr(0x1f,0x302e,2) | wcr(0x1f,0x3030,44) | wcr(0x1f,0x302c,2) | wcr(0x1f,0x302a,6)");
	cvWaitKey(10);								//Wait for 10 msec

	//Turn on analog gain
	sprintf(cmd,"wcr(0x1b,0x30b0,%i)\n",((1&0x3)<<4) | 0X80);
	port_com_send(cmd);

	sprintf(cmd,"wcr(0x04,0x30b0,%i)\n",((1&0x3)<<4) | 0X80);
	port_com_send(cmd);

	//Leave the PLL always ON
	port_com_send("wcr(0x1f,0x301a,0x1998)");
}


Mat geomtericCalibration::rotate90(Mat src){
	//EyelockLog(logger, TRACE, "rotation90");
	Mat dst;
	transpose(src, dst);
	flip(dst,dst,0);

	src.release();
	return dst;
}



std::vector<aruco::Marker> geomtericCalibration::gridBooardMarker(Mat img, int cam, bool calDebug){
	char c;
	Mat DisImg(100, 900, CV_8UC3, Scalar(0,0,0));

	twoMaxPeak twoPeak = calThreshold(img.cols,img.rows,(unsigned char *)(img.data),50);
	m_thresholdVal = double(twoPeak.peak1 + twoPeak.peak2)/2.0;

	if (cam == 4){
		smallImg = rotate90(img);
		smallImg.copyTo(imgCopy);
	}
	else{
		img.copyTo(imgCopy);
	}

	aruco::MarkerDetector mDetector;
	aruco::MarkerDetector::MarkerCandidate mCandidate;
	mDetector.setDictionary("ARUCO_MIP_36h12");
	std::vector<aruco::Marker> markers;

	char buffer[512], cmd[500], cmd1[500], cmd2[500], cmd3[500];
	imgCopy.copyTo(imgCopy1);

	if(!img.empty()){
		if (cam == 4){
/*			no binary image needed for marker detection for face camera calibration
			as the field of view have other objects then marker the threshold doesnot work quiet well*/
			//equalizeHist( imgCopy, imgCopy );
			//threshold( imgCopy, imgCopy, 10, 255,THRESH_BINARY);
/*			printf("m_thresholdVal is 		%i\n", m_thresholdVal);
			threshold( imgCopy, imgCopy, m_thresholdVal, 255,THRESH_BINARY);*/
		}
		else{
			//printf("m_thresholdVal is :%i\n", m_thresholdVal);
			threshold( imgCopy, imgCopy, m_thresholdVal, 255,THRESH_BINARY);
		}
		markers = mDetector.detect(imgCopy);

		if (markers.size() < 2){
			if(1){
				sprintf(buffer, "%s_%s have un-focus Images and only %i markers detected", cam & 0x01 ? "Left":"Right", cam & 0x80 ?  "AUX":"MAIN", markers.size());
				//cv::imshow("Binary No or only 1 marker detected", imgCopy);
				cv::putText(imgCopy1,buffer,cvPoint(50,imgCopy1.cols/2), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
				cv::imshow(buffer, imgCopy1);
				cv::moveWindow(buffer, 20, 20);
/*				c=cvWaitKey();
				if (c=='q'){
					printf("Continue!\n");
					destroyWindow(buffer);
				}*/
				waitKey(3000);//3 seconds delay
				destroyWindow(buffer);
			}
			DisImg.release();
			return markers;
		}


		if(1){

			//Draw markers with IDs
			for(size_t i = 0; i < markers.size(); i++){
				markers[i].draw(imgCopy1,cv::Scalar(255,255,0), 3, true, false);
				markers[i].draw(imgCopy,cv::Scalar(255,255,0), 3, true, false);
			}



			if (cam == 4){
				sprintf(cmd1, "%i Markers detected in Face camera", markers.size());
				imshow(cmd1, imgCopy1);
				cv::moveWindow(cmd1,20,20);

				sprintf(cmd3,"Result of Face cameras");
				sprintf(cmd2,"Press 'q' to continue!");
				//sprintf(buffer,"Press 'CTL + c' to terminate!");
				cv::putText(DisImg,cmd1,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
				//cv::putText(DisImg,cmd2,cvPoint(10,100), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
				//cv::putText(DisImg,buffer,cvPoint(10,150), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(255,0,0),2,CV_AA);
				imshow(cmd3, DisImg);
				cv::moveWindow(cmd3,20,20);

				sprintf(cmd,"/home/root/data/calibration/detectedMarkerCam%i.bmp\n", cam);
				cv::imwrite(cmd,imgCopy1);
				printf("detected markers Face cam image can be found in the following dir:\n");
				printf(cmd);

				waitKey(3000);//3 seconds delay
				destroyWindow(cmd1);
				destroyWindow(cmd3);

/*				c=cvWaitKey();
				if (c=='q'){
					printf("Continue!\n");
					destroyWindow(cmd1);
				}*/
			}
			else{

				sprintf(cmd1, "%i Markers detected from %s_%s camera", markers.size(), cam & 0x01 ? "Left":"Right",cam & 0x80 ?  "AUX":"MAIN");
				imshow(cmd1, imgCopy1);
				cv::moveWindow(cmd1,20,20);

				sprintf(cmd3,"Result of Iris cameras");
				sprintf(cmd2,"Press 'q' to continue!");
				//sprintf(buffer,"Press 'CTL + c' to terminate!");
				cv::putText(DisImg,cmd1,cvPoint(10,50), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
				//cv::putText(DisImg,cmd2,cvPoint(10,100), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(0,0,255),2,CV_AA);
				//cv::putText(DisImg,buffer,cvPoint(10,150), CV_FONT_HERSHEY_COMPLEX,1,cvScalar(255,0,0),2,CV_AA);
				imshow(cmd3, DisImg);
				cv::moveWindow(cmd3,20,20);

				sprintf(cmd,"/home/root/data/calibration/detectedMarkerCam%i.bmp", cam);
				cv::imwrite(cmd,imgCopy1);
				printf("detected markers Iris cam image can be found in the following dir:\n");
				printf(cmd);

				waitKey(3000);//3 seconds delay
				destroyWindow(cmd1);
				destroyWindow(cmd3);

/*				c=cvWaitKey();
				if (c=='q'){
					destroyWindow(cmd1);
					destroyWindow(cmd3);
					//destroyWindow(cmd2);
				}*/

			}
		}

		DisImg.release();
		return markers;
	}
	else{
		printf("There is no Image to detect Aruco-markers!!!\n");
		DisImg.release();
		return markers;
		exit(EXIT_FAILURE);

	}
	DisImg.release();
	return markers;
}


geomtericCalibration::twoMaxPeak geomtericCalibration::calThreshold(int width, int height,unsigned char *dsty, int limit)
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



//Find rect points from detected ARUCO markers
vector<float> geomtericCalibration::calibratedRect(std::vector<aruco::Marker> markerIris, std::vector<aruco::Marker> markerFace, int row, int col){
	std::vector<cv::Point2f> pointsIris, pointsFace;
	vector<float> rectResult;
	int count = 0;

	//Search max distance between points and collect marker Id which has max distact between them
	cv::Point2f ptr1, ptr2;
	std::vector<int> targetID;
	float maxDis = 0.0;
	int id;
	float m_offset, constantX, constantY; //magnification offset



	//Sorting out IDs that support two point condition
	// two points need to have max Euclidean distance and diagonally a part from each other
	for(int i = 0; i < markerIris.size(); i++){

		ptr1 = markerIris[i].getCenter();

		for(int j = i + 1; j < markerIris.size(); j++){

			ptr2 = markerIris[j].getCenter();

			if (std::abs(float(ptr1.y - ptr2.y)) < 20 || std::abs(float(ptr1.x - ptr2.x)) < 20){
				continue;
			}
			else{
				float dis = sqrt(pow((ptr1.x - ptr2.x),2) + pow((ptr1.y - ptr2.y),2));

				if (dis > maxDis){
					maxDis = dis;
					if(targetID.empty())
					{
						targetID.push_back(markerIris[i].id);
						targetID.push_back(markerIris[j].id);
					}
					else{
						targetID.pop_back();
						targetID.pop_back();
						targetID.push_back(markerIris[i].id);
						targetID.push_back(markerIris[j].id);
					}
				}
				//cout << "Dist ::::::::: " << dis << endl;
			}
		}
	}



	//Search target ID's center in iris camera
	for (int i = 0; i < targetID.size(); i++){
		id = targetID[i];
		for (int j = 0; j < markerIris.size(); j++){
			int idIris = markerIris[j].id;
			if (id == idIris){
				pointsIris.push_back(markerIris[j].getCenter());
			}
		}
	}


	//Search target ID's center in Face camera
	for (int i = 0; i < targetID.size(); i++){
		int id = targetID[i];
		for (int j = 0; j < markerFace.size(); j++){
			int idFace = markerFace[j].id;
			if (id == idFace){
				pointsFace.push_back(markerFace[j].getCenter());
			}
		}
	}



	//check whether it was successfully detected atleast two target points from both camera
	if (pointsIris.size() <= 1 || pointsFace.size() <= 1){
		printf("Fail to detect two common aruco markers with maximum Diagonal "
				"Distance in Both iris and face camera!\n");
		printf("Running the calibration again----->>>>>\n\n\n\n\n");
		return rectResult;
		exit(EXIT_FAILURE);
	}

	//pointsExp.push_back();

	if (m_calDebug){
		cout << pointsIris.size() << "-------------" << pointsIris[0].x << "-------------" << pointsIris[0].y << endl;
		cout << pointsIris.size() << "-------------" << pointsIris[1].x << "-------------" << pointsIris[1].y << endl;
		cout << pointsFace.size() << "-------------" << pointsFace[0].x << "-------------" << pointsFace[0].y << endl;
		cout << pointsFace.size() << "-------------" << pointsFace[1].x << "-------------" << pointsFace[1].y << endl;
		cout << endl;
	}

	if (m_calTwoPoint){
		//calculate the zoom offset or slope
		float mx = abs((pointsFace[1].x - pointsFace[0].x) / (pointsIris[1].x - pointsIris[0].x));
		float my = abs((pointsFace[1].y - pointsFace[0].y) / (pointsIris[1].y - pointsIris[0].y));

		m_offset = (mx + my)/2.0; 		//average Magnification offset

		float constantX = pointsFace[1].x - (mx * pointsIris[1].x);
		float constantY = pointsFace[1].y - (my * pointsIris[1].y);

		//constant = (cx + cy)/2.0;

		if (m_calDebug){
			cout << "number of face point ::: " << pointsFace.size() << " Number of Iris points :::: " << pointsIris.size() << endl;
			cout << "mx::::: " << mx << "   my:::::" << my << "		m_offset::: "<< m_offset <<endl;
			cout << "cx::::: " << constantX << "   cy:::::" << constantY <<endl;
		}

	}
	else{

		// Here for calculating magnification offset--- It will use all the common co-ordinates between
		int cc = 0;
		float sumIx1=0, sumFx1=0,sumIx2=0, sumFx2=0,multIx1Fx1=0,multIx2Fx2=0,powIx1=0,powFx1=0,powIx2=0,powFx2=0;
		float slopeIFx, slopeIFy;
		Vec4f lineX, lineY;

		for(int i = 0; i < markerIris.size(); i++){
			for(int j = 0; j < markerFace.size(); j++){
				if (markerIris[i].id == markerFace[j].id){
					sumIx1 += markerIris[i].getCenter().x;
					sumFx1 += markerFace[j].getCenter().x;
					multIx1Fx1 += markerIris[i].getCenter().x * markerFace[j].getCenter().x;
					powIx1 += markerIris[i].getCenter().x * markerIris[i].getCenter().x;
					powFx1 += markerFace[j].getCenter().x * markerFace[j].getCenter().x;

					sumIx2 += markerIris[i].getCenter().y;
					sumFx2 += markerFace[j].getCenter().y;
					multIx2Fx2 += markerIris[i].getCenter().y * markerFace[j].getCenter().y;
					powIx2 += markerIris[i].getCenter().y * markerIris[i].getCenter().y;
					powFx2 += markerFace[j].getCenter().y * markerFace[j].getCenter().y;

					cc++;
					break;
				}
			}
		}


		slopeIFx = ( (cc*multIx1Fx1) - (sumIx1 * sumFx1) ) / ( (cc*powIx1) - (sumIx1*sumIx1) );
		slopeIFy = ( (cc*multIx2Fx2) - (sumIx2 * sumFx2) ) / ( (cc*powIx2) - (sumIx2*sumIx2) );


		constantX = ((sumFx1 * powIx1) - (sumIx1*multIx1Fx1)) / ((cc*powIx1) - (sumIx1*sumIx1));
		constantY = ((sumFx2 * powIx2) - (sumIx2*multIx2Fx2)) / ((cc*powIx2) - (sumIx2*sumIx2));

		m_offset = (slopeIFx + slopeIFy)/2.0; 		//average Magnification offset

		if (m_calDebug){
			cout << "slopeIFx::::: " << slopeIFx << "   slopeIFy:::::" << slopeIFy << "m_offset :::::::: "<< m_offset << endl;
			cout << "constantIFx::::: " << constantX << "   constantIFy:::::" << constantY << endl;
		}
	}

	//use average magnification offset for projecting co-ordinates
	float x1_offset = cvRound(pointsFace[0].x - (m_offset * pointsIris[0].x));
	float y1_offset = cvRound(pointsFace[0].y - (m_offset * pointsIris[0].y));

	float x2_offset = cvRound((m_offset * col) + x1_offset);
	float y2_offset = cvRound((m_offset * row) + y1_offset);


	if (m_calDebug){
	cout << x1_offset << "*********************" << x2_offset << endl;
	cout << y1_offset << "  **********************   " << y2_offset << endl;
	}


	cout << "successfully calculated co-orinates! \n" << endl;

	rectResult.push_back(x1_offset);
	rectResult.push_back(y1_offset);
	rectResult.push_back(x2_offset);
	rectResult.push_back(y2_offset);
	rectResult.push_back(m_offset);		//common slope for two euqation
	rectResult.push_back(constantX);	//constant for calculating x coordinates
	rectResult.push_back(constantY);	//constant for calculating y coordinates

	return rectResult;

}


cv::Point geomtericCalibration::processDigRectPoint1(vector<float> x){
	return cv::Point(x[0],x[1]);
}

cv::Point geomtericCalibration::processDigRectPoint2(vector<float> x){
	return cv::Point(x[2],x[3]);
}

int RunSystemCmdCal(const char *ptr){
	int status = system(ptr);
	return status;
}

void geomtericCalibration::calibDataWrite(cv::Rect auxRect, vector<float> rectRightAux,vector<float> rectLeftAux, vector<float> rectRightMain, vector<float> rectLeftMain)
{
	stringstream ssI;
	string ssO;

	ssI << auxRect.x;
	ssI >> ssO;
	geoConfig.setValue("FTracker.targetRectX",ssO.c_str());
	if(m_calDebug){
		printf("x:: %s\n", ssO.c_str());
	}


	ssI.clear();
	ssI << auxRect.y;
	ssI >> ssO;
	geoConfig.setValue("FTracker.targetRectY",ssO.c_str());
	if(m_calDebug){
		printf("y:: %s\n", ssO.c_str());
	}


	ssI.clear();
	ssI << auxRect.width;
	ssI >> ssO;
	geoConfig.setValue("FTracker.targetRectWidth",ssO.c_str());
	if(m_calDebug){
		printf("width:: %s\n", ssO.c_str());
	}


	ssI.clear();
	ssI << auxRect.height;
	ssI >> ssO;
	geoConfig.setValue("FTracker.targetRectHeight",ssO.c_str());
	if(m_calDebug){
		printf("height:: %s\n", ssO.c_str());
	}


	//Writing magnification offset
	ssI.clear();
	ssI << rectRightAux[4];
	ssI >> ssO;
	geoConfig.setValue("FTracker.magOffsetAuxRightCam",ssO.c_str());

	ssI.clear();
	ssI << rectLeftAux[4];
	ssI >> ssO;
	geoConfig.setValue("FTracker.magOffsetAuxLeftCam",ssO.c_str());

	ssI.clear();
	ssI << rectRightMain[4];
	ssI >> ssO;
	geoConfig.setValue("FTracker.magOffsetMainRightCam",ssO.c_str());

	ssI.clear();
	ssI << rectLeftMain[4];
	ssI >> ssO;
	geoConfig.setValue("FTracker.magOffsetMainLeftCam",ssO.c_str());


	//Writing reference Marker points
	ssI.clear();
	ssI << float(rectRightAux[5]);
	ssI >> ssO;
	geoConfig.setValue("FTracker.constantAuxRightCam_x",ssO.c_str());
	//printf("constantAuxRightCam_x:: %s		%3.3f\n", ssO.c_str(), rectRightAux[5]);

	ssI.clear();
	ssI << float(rectRightAux[6]);
	ssI >> ssO;
	geoConfig.setValue("FTracker.constantAuxRightCam_y",ssO.c_str());
	//printf("constantAuxRightCam_y:: %s		%3.3f\n", ssO.c_str(), rectRightAux[6]);


	ssI.clear();
	ssI << float(rectLeftAux[5]);
	ssI >> ssO;
	geoConfig.setValue("FTracker.constantAuxLeftCam_x",ssO.c_str());

	ssI.clear();
	ssI << float(rectLeftAux[6]);
	ssI >> ssO;
	geoConfig.setValue("FTracker.constantAuxLeftCam_y",ssO.c_str());

	ssI.clear();
	ssI << float(rectRightMain[5]);
	ssI >> ssO;
	geoConfig.setValue("FTracker.constantMainRightCam_x",ssO.c_str());

	ssI.clear();
	ssI << float(rectRightMain[6]);
	ssI >> ssO;
	geoConfig.setValue("FTracker.constantMainRightCam_y",ssO.c_str());

	ssI.clear();
	ssI << float(rectLeftMain[5]);
	ssI >> ssO;
	geoConfig.setValue("FTracker.constantMainLeftCam_x",ssO.c_str());

	ssI.clear();
	ssI << float(rectLeftMain[6]);
	ssI >> ssO;
	geoConfig.setValue("FTracker.constantMainLeftCam_y",ssO.c_str());


	geoConfig.writeIni("/home/root/data/calibration/faceConfig.ini");
	// Creating CalRect.ini and upload to OIM ftp
	char buf[200];
	std::ofstream outfile("/home/root/CalRect.ini");

	EyelockLog(logger, DEBUG, "Create CalRect.ini file in /home/root folder");
	outfile << "FTracker.targetRectX=" << auxRect.x << std::endl;
	outfile << "FTracker.targetRectY=" << auxRect.y << std::endl;
	outfile << "FTracker.targetRectWidth=" << auxRect.width << std::endl;
	outfile << "FTracker.targetRectHeight=" << auxRect.height << std::endl;
	outfile.close();

	EyelockLog(logger, DEBUG, "Finished writing Calibration Rect data to CalRect.ini file");

	// Format the OIM ftp drive
	EyelockLog(logger, DEBUG, "Format the ftp drive");
	port_com_send("f_formt(0)");

	// upload to OIM
	sprintf(buf, "%s", "wput -B -t 2 -q CalRect.ini ftp://guest:guest@192.168.4.172");
	EyelockLog(logger, DEBUG, "Create ftp upload system command");
	fflush(stdout);
	RunSystemCmdCal(buf);
	EyelockLog(logger, DEBUG, "Uploading of CalRect.ini File to OIM ftp is Successful");

	// Upload the files to FTP for backup
	int Deviceid;
	cout << "Enter the OIM device number" << endl;
	cin >> Deviceid;

	char newCalRectFileName[100];
	char newfaceConfigFileName[100];
	// Rename the CalRect.ini file with DeviceId entered
	sprintf(newCalRectFileName, "CalRect.ini.%d", Deviceid);
	if(rename("CalRect.ini", newCalRectFileName ) == 0)
		printf("CalRect.ini file is renamed to %s\n", newCalRectFileName);
	else
	   perror("Error renaming CalRect.ini file" );


	// Copy the faceConfig.ini file to /home/root folder to rename and upload to ftp
	system("cp data/calibration/faceConfig.ini .");

	// Rename faceConfig.ini file with DeviceId
	sprintf(newfaceConfigFileName, "faceConfig.ini.%d", Deviceid);

	// printf("faceConfig file ....%s\n", newfaceConfigFileName);
	if(rename("faceConfig.ini", newfaceConfigFileName) == 0)
		printf("faceConfig.ini file is renamed to %s\n", newfaceConfigFileName);
	else
		perror("Error renaming faceConfig.ini file" );

	char FTPDetails[200];
	sprintf(FTPDetails, "wput -B -t 2 -q %s ftp://mamigotesters:'C3$W4$gr3+'@107.22.234.115/EXTCalibrationFiles/", newCalRectFileName);
	fflush(stdout);
	RunSystemCmdCal(FTPDetails);
	printf("Upload of %s to ftp is Successful\n", newCalRectFileName);

	sprintf(FTPDetails, "wput -B -t 2 -q %s ftp://mamigotesters:'C3$W4$gr3+'@107.22.234.115/EXTCalibrationFiles/", newfaceConfigFileName);
	fflush(stdout);
	RunSystemCmdCal(FTPDetails);
	printf("Upload of %s to ftp is Successful\n", newfaceConfigFileName);

}


void geomtericCalibration::brightnessAdjust(Mat outImg, int cam, bool calDebug){
	float p, p_old;

	p = AGC(outImg.cols,outImg.rows,(unsigned char *)(outImg.data),50);

	char buff[512], buffX[512], v;
	int exposure_camID;

	float bThreshold = 0;
	if (cam == 4){
		m_agc_val_cal = 1;
		bThreshold = 9.00;
		exposure_camID = 4;
	}
	else if (cam == 129 || cam == 130){
		m_agc_val_cal = 1;
		bThreshold = 25.00;
		exposure_camID = 3;
	}
	else if(cam == 1 || cam == 2){
		m_agc_val_cal = 1;
		//bThreshold = 15.00;
		bThreshold = 12.00;
		exposure_camID = 24;
	}

	if (calDebug){
		if (cam == 4){
			sprintf(buffX, "Adjusting Brightness of Face camera");
			imshow(buffX, outImg);
			cv::moveWindow(buffX, 20, 20);
		}
		else{
			sprintf(buffX, "Adjusting Brightness of %s %s camera", cam & 0x01 ? "Left":"Right",cam & 0x80 ?  "AUX":"MAIN");
			imshow(buffX, outImg);
			cv::moveWindow(buffX, 20, 20);
		}
	}

	while(!(p >= bThreshold)){
		m_agc_val_cal++;
		sprintf(buff,"wcr(%d,0x3012,%d)",exposure_camID,m_agc_val_cal);
		port_com_send(buff);
		p_old = p;

		p = AGC(outImg.cols,outImg.rows,(unsigned char *)(outImg.data),50);
		//printf(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Percentile::: %3.3f Agc value = %d\n",p,m_agc_val_cal);

		if (calDebug){
			imshow(buffX, outImg);
			v = cvWaitKey();
			if(v == 'q')
				continue;

		}

		if(m_agc_val_cal > 26)
		{
			sprintf(buff,"wcr(%d,0x3012,%d)",exposure_camID,m_agc_val_cal + 1);
			port_com_send(buff);
			break;
		}


	}

	if (calDebug){
		destroyWindow(buffX);
	}
	printf("Brightness adjustment is completed!\n");

}

float geomtericCalibration::AGC(int width, int height,unsigned char *dsty, int limit)
{
	//Percentile and Average Calculation
	unsigned char *dy = (unsigned char *)dsty;
	double total = 0,Ptotal = 0,percentile = 0,hist[256]={0},average=0;
	int pix=0,i;
	int n = width * height;
	//int limit = 180;    // Lower limit for percentile calculation
	for (; n > 0; n--)
	{
		pix =(int) *dy;
		hist[pix]++;
		dy++;
		average=average+pix;
	}

	for(i=0;i<=255;i++)
	{
		float histValue = hist[i];
		total = total + (double)histValue;
		if(i>=limit)
			Ptotal = Ptotal + (double)histValue;
	}
	percentile = (Ptotal*100)/total;
	average=average/(width*height);

	return (float)percentile;
}


