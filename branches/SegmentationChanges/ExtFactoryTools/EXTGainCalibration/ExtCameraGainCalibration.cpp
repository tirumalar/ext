/*
 * CameraCalibrationGain.cpp
 *
 *  Created on: Apr 10, 2019
 *      Author: root
 */

#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <pthread.h>
#include <unistd.h>
#include <chrono>
#include <sys/time.h>
#include <stdlib.h>
#include <math.h>
#include <cmath>

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>
#include <opencv2/photo/photo.hpp>
#include "portcom.h"

#include "Synchronization.h"
#include "pstream.h"
#include "FileConfiguration.h"
#include "Configuration.h"
#include "logging.h"

//#include "extFocus.h"
//#include "geomtericCalibration.h"

using namespace cv;
//using namespace std::chrono;
using namespace std;


char cmd[512];
char msg[60];

VideoStream *vsLeft;
VideoStream *vsRight;
VideoStream *vsFace;

const int imageWidth = 1200;	//even though the AR0135 sensor is 1280 x 960 !
const int imageHeight = 960;

Mat leftImage = Mat(Size(imageWidth, imageHeight), CV_8U);
Mat rightImage = Mat(Size(imageWidth, imageHeight), CV_8U);
Mat faceImage = Mat(Size(imageWidth, imageHeight), CV_8U);
Mat clImage = Mat(Size(imageWidth, imageHeight), CV_8U);			//for finding camera levels
Mat msgImage = Mat(450, 1800, CV_8U);								//for showing messages

string msgLine1 = "";			//for showing messages
string msgLine2 = "";
string msgLine3 = "";
string msgLine4 = "";
string msgLine5 = "";

int problemCode = 0;

int defaultGain = 32;
int defaultExposure = 15;
int refLevelAux = 220;			//Desired level on image roi for Aux cameras
int refLevelMain = 110;			//Desired level on image roi for Main cameras


void msleep(int sleepTime)				//Pauses for sleepTime in msec
{
	usleep(1000*sleepTime);
}

void releaseImages()
{
	leftImage.release();
	rightImage.release();
	faceImage.release();
	msgImage.release();
}

void deleteVideostreams()
{
	delete(vsLeft);
	delete(vsRight);
	delete(vsFace);
}

void setcamerasMain()					//Activates Main cameras
//Even though the camera addresses are 1 and 2 for Aux and 8 and 16 for Aux, Main is set with code 7 and Aux is set with code 135 (0x87).

{
	sprintf(cmd,"set_cam_mode(0x07,100)\n");
	port_com_send(cmd);
	usleep(100);
}

void setcamerasAux()
{
	sprintf(cmd,"set_cam_mode(0x87,100)\n");
	port_com_send(cmd);
	usleep(100);
}

void showLeftImage()						//Displays an image from the left camera
{
	int w,h;
	vsLeft->flush();
	vsLeft->get(&w, &h, (char*)leftImage.data);
	imshow("leftWindow", leftImage);
}

void showRightImage()
{
	int w,h;
	vsRight->get(&w, &h, (char*)rightImage.data);
	imshow("rightWindow", rightImage);
}

void showFaceImage()
{
	int w,h;
	vsFace->get(&w, &h, (char*)faceImage.data);
	imshow("faceWindow", faceImage);
}

void showMainCameras()			//Displays live images from both Main cameras until a key is pressed
{
	int key;
	setcamerasMain();
	printf("Showing Main cameras - press a key to finish \n");

	do	{	showLeftImage();
			showRightImage();
			key=cvWaitKey(100);
		} 	while(key==-1);
}

void showAuxCameras()			//Displays live images from both Aux cameras until a key is pressed
{
	int key;
	setcamerasAux();
	printf("Showing Aux cameras - press a key to finish \n");

	do	{	showLeftImage();
			showRightImage();
			key=cvWaitKey(100);
		} 	while(key==-1);
}

void showFaceCamera()
{
	int key;
	printf("Showing Face camera - press a key to finish \n");

	do	{	showFaceImage();
			key=cvWaitKey(100);
		} 	while(key==-1);
}

void saveFaceImage()
{
	imwrite("FaceImage.png",faceImage);
}

void getLeftImage()				//Returns an image from the left camera
{
	int w,h;
	vsLeft->get(&w, &h, (char*)leftImage.data);

}

void getRightImage()				//Returns an image from the right camera
{
	int w,h;
	vsRight->get(&w, &h, (char*)rightImage.data);

}

void getFaceImage()
{
	int w,h;
	vsFace->get(&w, &h, (char*)faceImage.data);
}

void showMessage()

//Draws a window with up to five lines of text, ~60 characters per line.

{
	int fontFace = FONT_HERSHEY_COMPLEX;
	double fontScale = 1.5;
	int thickness = 2;

	msgImage = 0;

	putText(msgImage, msgLine1, Point(50,75), fontFace, fontScale, Scalar::all(255), thickness, 8);
	putText(msgImage, msgLine2, Point(50,150), fontFace, fontScale, Scalar::all(255), thickness, 8);
	putText(msgImage, msgLine3, Point(50,225), fontFace, fontScale, Scalar::all(255), thickness, 8);
	putText(msgImage, msgLine4, Point(50,300), fontFace, fontScale, Scalar::all(255), thickness, 8);
	putText(msgImage, msgLine5, Point(50,375), fontFace, fontScale, Scalar::all(255), thickness, 8);

	imshow("Message", msgImage);
//	waitKey(0);
}

void driveMotor(int dist)

// Drives the motor through (approximately) the number of motorUnits in the face camera
// (about 10 pixels/motorUnit)
// Then wait about 300msec for motor to settle.

{
	sprintf(cmd, "fx_rel(%i)\n",dist);
	port_com_send(cmd);
//	printf(cmd);
	msleep(300);
}

void driveHome()
{
	sprintf(cmd, "fx_home() \n");
		port_com_send(cmd);
		printf(cmd);
		msleep(300);
}

int RunSystemCmdCal(const char *ptr){
	int status = system(ptr);
	return status;
}

void saveValues(int gainML,int expML,int gainMR,int expMR,int gainAL,int expAL,int gainAR,int expAR)

//Saves gain and exposure values to .ini file.

{
	printf("%d %d %d %d %d %d %d %d\n", gainML, expML, gainMR, expMR, gainAL, expAL, gainAR, expAR);

	printf("Saving gain and exposure values to /home/root/data/calibration/faceConfig.ini \n");

	FileConfiguration faceConfig("/home/root/data/calibration/faceConfig.ini");
	std::stringstream ssI;
	std::string ssO;

	// AUX Camera Left
	ssI.clear();
	ssI << gainAL;
	ssI >> ssO;
	faceConfig.setValue("FTracker.LeftAuxIrisCamDigitalGain", ssO.c_str());

	ssI.clear();
	ssI << expAL;
	ssI >> ssO;
	faceConfig.setValue("FTracker.LeftAuxIrisCamExposureTime",ssO.c_str());

	// AUX Camera Right
	ssI.clear();
	ssI << gainAR;
	ssI >> ssO;
	faceConfig.setValue("FTracker.RightAuxIrisCamDigitalGain",ssO.c_str());

	ssI.clear();
	ssI << expAR;
	ssI >> ssO;
	faceConfig.setValue("FTracker.RightAuxIrisCamExposureTime",ssO.c_str());

	// Main Camera Left
	ssI.clear();
	ssI << gainML;
	ssI >> ssO;
	faceConfig.setValue("FTracker.LeftMainIrisCamDigitalGain",ssO.c_str());

	ssI.clear();
	ssI << expML;
	ssI >> ssO;
	faceConfig.setValue("FTracker.LeftMainIrisCamExposureTime",ssO.c_str());

	// Main Camera Right
	ssI.clear();
	ssI << gainMR;
	ssI >> ssO;
	faceConfig.setValue("FTracker.RightMainIrisCamDigitalGain",ssO.c_str());

	ssI.clear();
	ssI << expMR;
	ssI >> ssO;
	faceConfig.setValue("FTracker.RightMainIrisCamExposureTime",ssO.c_str());

	faceConfig.writeIni("/home/root/data/calibration/faceConfig.ini");

	printf("Finish writing all the values in faceConfig.ini\n");

	// Copy the faceConfig.ini file to /home/root folder to rename and upload to ftp
	system("cp data/calibration/faceConfig.ini .");

	// Uploading the file to OIM ftp
	char buf[200];
	sprintf(buf, "%s", "wput -B -t 2 -q faceConfig.ini ftp://guest:guest@192.168.4.172");
	fflush(stdout);
	RunSystemCmdCal(buf);
	printf("Uploading of faceConfig.ini File to OIM ftp is Successful");

	// Upload the files to FTP for backup
	int Deviceid;
	cout << "Enter the OIM device number" << endl;
	cin >> Deviceid;

	char newfaceConfigFName[100];
	// Rename faceConfig.ini file with DeviceId
	sprintf(newfaceConfigFName, "faceConfig.ini.%d", Deviceid);

	// printf("faceConfig file ....%s\n", newfaceConfigFileName);
	if(rename("faceConfig.ini", newfaceConfigFName) == 0)
		printf("faceConfig.ini file is renamed to %s\n", newfaceConfigFName);
	else
		perror("Error renaming faceConfig.ini file" );

	char FTPDetails[250];
	sprintf(FTPDetails, "wput -B -t 2 -q %s ftp://mamigotesters:'C3$W4$gr3+'@107.22.234.115/EXTCalibrationFiles/", newfaceConfigFName);
	fflush(stdout);
	RunSystemCmdCal(FTPDetails);
	printf("Upload of %s to ftp is Successful\n", newfaceConfigFName);

}



int dstToTarget()

//Returns the row number in the Face image that is centered on the target.

{
//	printf("starting dstToTarget \n");

	//We will need a face image and an array of 1200 char to store pixel levels, and another array to track the target.

	int w, h;
	int ribbon[imageWidth];
	int ribbonHeight = 40;					//Width of target search bar, in pixels.




 	getFaceImage();
 	getFaceImage();
 	getFaceImage();
 	getFaceImage();

	//Assume the target is centered along the x axis (i.e. horizontally for a portrait image).
	//Consider a ribbon 1" wide (40 pixels) along the image y axis.
	//At 28", there are about 1.4 pixels/mm.
	//ribbon[] is an array of 1200 points with the average values of a 1" wide strip along the center of the image

//	printf("starting ribbon \n");

	for (int iw=0;iw<imageWidth;iw++)
	{
		double level = 0;

		for (int ih=(imageHeight-ribbonHeight)/2; ih<(imageHeight+ribbonHeight)/2; ih++)
		{
			level = level + (int)faceImage.at<uchar>(ih,iw);
		}
		ribbon[iw] = level / ribbonHeight;		//Average level of the row.
//		printf("%i   %f   %i \n",iw,level,ribbon[iw]);
	}



	//Look for a pattern that is bright for 118 pixels surrounded by dark on both sides for 32 pixels.

	int frameWidth = 33;							//Width of black frame in pixels
	int	paperWidth = 178;							//Size of bright area in pixels
	int patternWidth = 2*frameWidth + paperWidth;
	float pattern[patternWidth];
	float patternMatch[imageWidth - patternWidth];
	float darkValue = -0.5f * paperWidth / frameWidth;

//	printf("Paper width = %i \n Frame width = %i \n Dark value =  %f \n",paperWidth,frameWidth,darkValue);

	for (int i=0;i<frameWidth;i++)
	{
		pattern[i]=darkValue;
//		printf("%i %f \n",i,pattern[i]);
	}
	for (int i=frameWidth+1;i<=frameWidth+paperWidth;i++)
	{
		pattern[i]=1;
//		printf("%i %f \n",i,pattern[i]);
	}
	for (int i=frameWidth+paperWidth+1;i<patternWidth;i++)
	{
		pattern[i]=darkValue;
//		printf("%i %f \n",i,pattern[i]);
	}


	for (int i = 0;i<imageWidth-patternWidth;i++)

	{
		patternMatch[i]=0.0;
		for(int ip = 0; ip<patternWidth;ip++)
		{
			patternMatch[i] = patternMatch[i] + 1.0f * ribbon[i+ip] * pattern[ip];		//patternMatch has a peak at the location of the target.
		}
//		printf("%i   %f \n",i, patternMatch[i]);
	}

	//Look for a peak in patternMatch

	float bright = 0.0f;
	int brightPeak;

	for (int i = 0; i < imageWidth-patternWidth; i++)
	{
		if (patternMatch[i] > bright)
		{
			bright = patternMatch[i];
			brightPeak = i+patternWidth/2;
		}
	}

//	printf("bright spot = %i \n",brightPeak);


	//	The following lines display images with a rectangle indicating the ROI where image levels are read.
	int x1 = brightPeak - patternWidth/2;
	int y1 = (imageHeight-ribbonHeight)/2;
	int x2 = brightPeak + patternWidth/2;
	int y2 = (imageHeight+ribbonHeight)/2;
	Point pp1(x1,y1);
	Point pp2(x2,y2);


	rectangle(faceImage,pp1,pp2,255);
	imshow("faceWindow", faceImage);
//	printf("Press a key to continue \n");
	cvWaitKey(500);

//	printf("finished dstToTarget \n");

	//Return the center of this region.
	return brightPeak;

}


void pointAtTarget()

	//Drives the camera board motor to point the face camera at the gain calibration target.

{
	int dstToGo;						//distance to move in pixels
	float pixelsPerMotor = -4.6;		//face camera pixels per motor unit
	int dstVariance = 10;				//Allowed position error in face camera pixels  (About 14 mm)
	int motorDrive;
	int targetOffset = 30;				//Desired offset of target position from center
	int targetPosition = imageWidth/2 + targetOffset;
	int driveAttempts = 0;
	int driveAttemptLimit = 10;			//Number of tries before driving is considered a failure


	printf("Centering target \n");
	msgLine2 = "Centering calibration target.";
	showMessage();

	sprintf(cmd, "wcr(4,0x305e,32)\n");		//Face camera gain = 1x
	port_com_send(cmd);
//	printf(cmd);
	usleep(100);

	dstToGo = targetPosition - dstToTarget();			//Get the target position along the horizontal axis in pixels.
	printf("distance to go = %i \n",dstToGo);

	while (abs(dstToGo) > dstVariance)
	{
		driveAttempts++;
		if (driveAttempts > driveAttemptLimit)			//Drive motor does not respond correctly!
		{
			msgLine1 = "CALIBRATION FAILED!";
			msgLine2 = "Drive motor or Face camera failure.";
			msgLine3 = "Reject this EXT.";
			msgLine4 = "Hit any key to end Calibration.";
			showMessage();
			printf("EXT rejected for drive motor failure");
			printf("Problem code 1");
			waitKey(0);
			problemCode = 1;
			return;
		}
		motorDrive = dstToGo / pixelsPerMotor;
		if (abs(motorDrive) < 5)
		{
			if (motorDrive > 0)
			{
				motorDrive = 5;
			}
			else
			{
				motorDrive = -5;
			}
		}
		driveMotor(motorDrive);
		msleep(500);
//		printf("drive motor %i \n",motorDrive);
		dstToGo = targetPosition - dstToTarget();
//		printf("distance to go = %i \n",dstToGo);
	}

	printf("Pointed at target (%i drives) \n", driveAttempts);

}

int getCameraLevel(int cam)

// Returns the average level within a region of interest
// cam = 1 - left aux
// cam = 2 - right aux
// cam = 8 - left main
// cam = 16 - right main

{
//	printf("starting getCameraLevel for camera %i \n",cam);

	int h,w,pxl,lvl,xL,xR,yL,yR;
	double pixel,level;
	int imageCount = 10;	//Number of images to be averaged.
	int roiWidth = 200;		//roi is a rectangle in the image where the level is measured.
	int roiHeight = 200;
	int roiOffset;			//roi offset is the distance (in pixels) of the roi offset from center, needed to compensate for camera position offset.

	//Set the proper offset to the roi
	switch(cam)
	{
		case 1:
			roiOffset = 100;
			break;
		case 2:
			roiOffset = -100;
			break;
		case 8:
			roiOffset = 185;
			break;
		case 16:
			roiOffset = -185;
			break;
	}

	//For displaying the roi
	xL = (imageWidth-roiWidth)/2+roiOffset;
	xR = (imageWidth+roiWidth)/2+roiOffset;
	yL = (imageHeight-roiHeight)/2;
	yR = (imageHeight+roiHeight)/2;

	Point pt1(xL,yL);
	Point pt2(xR,yR);

	level=0;

	//Get an image (clImage) from the correct camera and show it with the roi
	switch(cam)
		{
		case 1:
//				printf("Aux Left cam = %i \n",cam);
			setcamerasAux();
			vsLeft->flush();
			vsLeft->get(&w, &h, (char*)clImage.data);
			vsLeft->get(&w, &h, (char*)clImage.data);
			vsLeft->get(&w, &h, (char*)clImage.data);
			clImage.copyTo(leftImage);
			rectangle(leftImage,pt1,pt2,0);
			imshow("leftWindow", leftImage);
			break;
		case 2:
//				printf("Aux Right cam = %i \n",cam);
			setcamerasAux();
			vsRight->flush();
			vsRight->get(&w, &h, (char*)clImage.data);
			vsRight->get(&w, &h, (char*)clImage.data);
			vsRight->get(&w, &h, (char*)clImage.data);
			clImage.copyTo(rightImage);
			rectangle(rightImage,pt1,pt2,0);
			imshow("rightWindow", rightImage);
			break;

		case 8:
//				printf("Main Left cam = %i \n",cam);
			setcamerasMain();
			vsLeft->flush();
			vsLeft->get(&w, &h, (char*)clImage.data);
			vsLeft->get(&w, &h, (char*)clImage.data);
			vsLeft->get(&w, &h, (char*)clImage.data);
			clImage.copyTo(leftImage);
			rectangle(leftImage,pt1,pt2,0);
			imshow("leftWindow", leftImage);
			break;


		case 16:
//				printf("Main Right cam = %i \n",cam);
			setcamerasMain();
			vsRight->flush();
			vsRight->get(&w, &h, (char*)clImage.data);
			vsRight->get(&w, &h, (char*)clImage.data);
			vsRight->get(&w, &h, (char*)clImage.data);
			clImage.copyTo(rightImage);
			rectangle(rightImage,pt1,pt2,0);
			imshow("rightWindow", rightImage);
			break;

		}
//	printf("Camera %i - Press a key to continue \n",cam);
	cvWaitKey(300);		//Pause to show the image with the rectangle over the roi


	//Measure the average image level over the roi, averaging over many images
	for (int i=0;i<imageCount;i++)
	{

		//Get a fresh image
		switch(cam)
			{
			case 1:
				vsLeft->get(&w, &h, (char*)clImage.data);
				break;
			case 2:
				vsRight->get(&w, &h, (char*)clImage.data);
				break;
			case 8:
				vsLeft->get(&w, &h, (char*)clImage.data);
				break;
			case 16:
				vsRight->get(&w, &h, (char*)clImage.data);
				break;
			}

		//Add up the pixel levels in the roi.
		pixel=0;
		for(int ix=xL; ix<xR; ix++)
		{
			for(int iy=yL; iy<yR; iy++)
			{
				pxl = (int)clImage.at<uchar>(ix,iy);
				pixel += pxl;
			}
		}
		//Divide by the number of pixels to get the average level for this image, and add to that of previous images.
		level = level + pixel/(roiWidth*roiHeight);

	}

	//Divide by the number of images to get the overall average level.
	lvl = level/imageCount;

	return lvl;

}


void calibrateGainAndExposure(int gain[4], int exp[4])

//Returns new gain and exposure settings for all four iris cameras (Aux Left, Aux Right, Main Left, Main Right).
//It calls LeftCameraLevel and RightCameraLevel functions to get the level in a central ROI, averaged over many images.
//This level is compared to a desired level (refLevel).
//If, as is usual, this is lower than desired, then the gain is increased to compensate.
//If the level is too high, then the exposure is reduced to compensate, and the gain is left at the default value.

{

	int level, refLevel, newGain, newExposure;
	int levelTolerance = 10;		//Acceptable deviation from reference level
	int maxGain = 225;				//Reject EXT if required gain is excessive
	int minExposure = 8;			//Reject if images are MUCH too bright
	int cam;						//1 = Main Left, 2 = Main Right, (4=Face), 8 = Aux Left, 16 = Aux right


	printf("Setting gain and exposure \n");
	sprintf(cmd, "wcr(27,0x305e,%i)\n",defaultGain);		//Set all iris camera gains to 32 (1x)
	port_com_send(cmd);
//	printf(cmd);
	usleep(1000);

	sprintf(cmd, "wcr(27,0x3012,%i)\n",defaultExposure);	//Set all iris camera exposures to 15 (~0.8 msec)
	port_com_send(cmd);
//	printf(cmd);
	usleep(1000);


	for (int icam = 1; icam <= 4; icam++)	//Do the four iris cameras
	{
		switch(icam)
		{
		case 1:
			cam = 1;
			refLevel = refLevelAux;
			msgLine3 = "Calibrating Left Aux camera.  ";
			break;
		case 2:
			cam = 2;
			refLevel = refLevelAux;
			msgLine3 = "Calibrating Right Aux camera.  ";
			break;
		case 3:
			cam = 8;
			refLevel = refLevelMain;
			msgLine3 = "Calibrating Left Main camera.  ";
			break;
		case 4:
			cam = 16;
			refLevel = refLevelMain;
			msgLine3 = "Calibrating Right Main camera.  ";
			break;
		}

		showMessage();
		level =  getCameraLevel(cam);

//		printf("Cam %i   level %i   refLevel  %i \n",cam,level,refLevel);

		int oldGain = defaultGain;
		int oldExposure = defaultExposure;


		while(abs(level - refLevel) > levelTolerance)
		{

			if (level <= refLevel)
			{
				newGain = oldGain * refLevel / level;		//If level is too low, increase the gain to correct it.
				newExposure = oldExposure;
			}
			else
			{
				newGain = oldGain;
				newExposure = oldExposure * refLevel / level;	//If level is too high, reduce exposure to correct it.
			}

			if (newGain > 255)
			{
				newGain = 255;
			}

			if (newExposure > 20)
			{
				newExposure = 20;
			}


			sprintf(cmd, "wcr(%i,0x305e,%i)\n",cam,newGain);				//Updates gain setting on Cameras 1
			port_com_send(cmd);

			sprintf(cmd, "wcr(%i,0x3012,%i)\n",cam,newExposure);			//Updates exposure setting on Camera 1
			port_com_send(cmd);
			msleep(100);

//			printf("New Gain = %i, New Exposure = %i \n", newGain, newExposure);

			level =  getCameraLevel(cam);
//			printf("Cam %i   level %i   refLevel  %i \n",cam,level,refLevel);

			oldGain = newGain;
			oldExposure = newExposure;

		}


		//Reject if excessive gain is required.
		if (newGain > maxGain)
		{
			msgLine1 = "CALIBRATION FAILED!";
			sprintf(msg,"Camera %i requires excessive gain (%i)",cam,newGain);
			msgLine2 = msg;
			msgLine3 = "Reject this EXT.";
			msgLine4 = "Hit any key to end Calibration.";
			showMessage();
			waitKey(0);
			problemCode = 2;
			printf("EXT rejected for excessive gain (%i) on camera %i. \n",newGain,cam);
			printf("Problem code 2 \n");
			return;
		}

		//Reject if exposure is insufficient.
		if (newExposure < minExposure)
		{
			msgLine1 = "CALIBRATION FAILED!";
			sprintf(msg,"Camera %i requires insufficient exposure (%i)",cam,newExposure);
			msgLine2 = msg;
			msgLine3 = "Reject this EXT.";
			msgLine4 = "Hit any key to end Calibration.";
			showMessage();
			waitKey(0);
			problemCode = 2;
			printf("EXT rejected for insufficient exposure (%i) on camera %i. \n",newExposure,cam);
			printf("Problem code 3 \n");
			return;
		}


		gain[icam] = newGain;
		exp[icam] = newExposure;
		printf("Camera %i   level = %i   New Gain = %i   New Exposure = %i \n",cam,level,newGain,newExposure);

	}

/*				//For debugging
	msgLine1 = "Calibration complete.";
	msgLine2 = "Showing Aux cameras";
	msgLine3 = "Hit any key to show Main cameras.";
	showMessage();
	showAuxCameras();


	msgLine1 = "Calibration complete.";
	msgLine2 = "Showing Main cameras";
	msgLine3 = "Hit any key to save values.";
	showMessage();
	showMainCameras();
*/

}


void startCameras()
{
	//Sets up the cameras and starts videostreams.

	printf("Running startCameras \n");

	sprintf(cmd, "wcr(31,0x3012,15)\n");	//Exposure = 0.8 msec  (Iris and Face cameras)
	port_com_send(cmd);
//	printf(cmd);
	usleep(100);

	sprintf(cmd, "wcr(24,0x305e,80)\n");		//Main camera gain = 2.5x
	port_com_send(cmd);
//	printf(cmd);
	usleep(100);

	sprintf(cmd, "wcr(3,0x305e,128)\n");	//Aux camera gain = 4x
	port_com_send(cmd);
//	printf(cmd);
	usleep(100);

	sprintf(cmd, "wcr(4,0x305e,32)\n");	//Face camera gain = 1x
	port_com_send(cmd);
//	printf(cmd);
	usleep(100);
}

void startLEDs()

//Sets parameters to normal values for all iris cameras.
{

	printf("Running StartLEDs \n");

	sprintf(cmd, "psoc_write(2,37)\n");		//HV = 37V.  This is the voltage on the capacitor bank that provides current to the LEDs.
	port_com_send(cmd);
//	printf(cmd);
	usleep(100);

	sprintf(cmd, "psoc_write(5,40)\n");		//LED current = 4A
	port_com_send(cmd);
//	printf(cmd);
	usleep(100);

	sprintf(cmd, "psoc_write(6,8)\n");		//Pulse limit = 1 msec
	port_com_send(cmd);
//	printf(cmd);
	usleep(100);

	sprintf(cmd, "psoc_write(9,85)\n");		//Power PWM = 66%.  This limits the current to the capacitor bank.
	port_com_send(cmd);
//	printf(cmd);
	usleep(100);

	sprintf(cmd, "psoc_write(1,1)\n");		//HV on
	port_com_send(cmd);
//	printf(cmd);
	usleep(100);

	sprintf(cmd, "psoc_write(3,49)\n");		//LEDs alternate
	port_com_send(cmd);
//	printf(cmd);
	usleep(100);

}


void test()
{
	printf("Saving gain and exposure values to /home/root/data/calibration/faceConfigTest.ini \n");

		FileConfiguration faceConfig("/home/root/data/calibration/faceConfigTest.ini");
		stringstream ssI;
		string ssO;
		int output = 50;

		int test = faceConfig.getValue("FTracker.NewLine",0);
		printf("test = %i \n",test);

		faceConfig.setValue("FTracker.NewLine","56");
		test = faceConfig.getValue("FTracker.NewLine",0);
				printf("test = %i \n",test);



//		ssI.clear();
//		ssI << output;
//		ssI >> ssO;
//		faceConfig.setValue("FTracker.LeftMainIrisCamDigitalGain",ssO.c_str());
}


int main(int argc, char **argv)
{
	//	EyelockLog(logger, TRACE, "Inside main function");

	FileConfiguration faceConfig("/home/root/data/calibration/faceConfig.ini");

	bool bDoAESEncryption = faceConfig.getValue("FTracker.AESEncrypt", false);

	int gain[4], exp[4];
	int gML, eML, gMR, eMR, gAL, eAL, gAR, eAR;			//Gain and exposure values to be saved to .ini file
	int smallWidth = imageWidth/2;
	int smallHeight = imageHeight/2;

	namedWindow("Message",CV_WINDOW_AUTOSIZE);
	moveWindow("Message",100,585 );

	printf("Running Gain Calibration \n");
	msgLine1 = "Running Gain Calibration.  ";
	msgLine2 = "(If the system stalls there is a communication problem.";
	msgLine3 = "Reject the EXT and restart the Gain Calibration App.)";
	showMessage();
	waitKey(100);

	int rtn = portcom_start(bDoAESEncryption);

	printf("portcom_start = %i \n",rtn);
	msgLine2 = "Communicating with EXT.  ";
	msgLine3 = "";
	showMessage();
	printf("problemCode = %i \n",problemCode);

	if (problemCode == 0)
	{
		startLEDs();
		startCameras();
		msgLine2 = "LEDs and Cameras started.  ";

		setcamerasMain();

		vsLeft = new VideoStream(8192, false);			//Start left iris camera stream
		vsLeft->flush();
		msleep(100);

		vsRight = new VideoStream(8193, false);		//Start right iris camera stream
		vsRight->flush();
		msleep(100);

		vsFace = new VideoStream(8194, false);			//Start face camera stream, to find the target.
		vsFace->flush();
		msleep(100);

		namedWindow("leftWindow",CV_WINDOW_NORMAL);
		moveWindow("leftWindow",60,60);
		resizeWindow("leftWindow",smallWidth,smallHeight);//Windows for displaying images

		namedWindow("rightWindow",CV_WINDOW_NORMAL);
		moveWindow("rightWindow",690,60);
		resizeWindow("rightWindow",smallWidth,smallHeight);

		namedWindow("faceWindow",CV_WINDOW_NORMAL);
		moveWindow("faceWindow",1320,60);
		resizeWindow("faceWindow",smallWidth,smallHeight);


	}

//	driveHome();  //for debugging

	pointAtTarget();						//Recursive drive that locates target and centers it
	if (problemCode == 0)
	{
		msgLine2 = "Looking at target.  ";
		showMessage();

		calibrateGainAndExposure(gain, exp);	//Gain and exposure for proper image levels, for all iris cameras

	}

	if (problemCode == 0)
	{

		msgLine2 = "Saving values.";
		msgLine3 = "";
		showMessage();

		gAL = gain[1];			//Gain for Aux Left camera
		eAL = exp[1];			//Exposure for Aux Left camera
		gAR = gain[2];
		eAR = exp[2];
		gML = gain[3];
		eML = exp[3];
		gMR = gain[4];
		eMR = exp[4];

		saveValues(gML, eML, gMR, eMR, gAL, eAL, gAR, eAR);		//Save the values in the .ini file
		// saveValues(100, 101, 20, 21, 40, 41, 50, 45);		//Save the values in the .ini file
		// saveValues(200, 201, 202, 203, 204, 205, 206, 207);

		msgLine1 = "Calibration complete.";
		msgLine2 = "Hit any key, then disconnect the EXT.";
		showMessage();
		waitKey(0);
	}

	printf("Calibration completed");

	releaseImages();
	deleteVideostreams();
	destroyAllWindows();

	return 0;


}






