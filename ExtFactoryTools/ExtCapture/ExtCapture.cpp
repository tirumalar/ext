/*
 * Anita: This app captures frames from the device.
 *
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
#include "portcom.h"
#include "Synchronization.h"
#include "pstream.h"
#include "FileConfiguration.h"
#include "Configuration.h"
#include "logging.h"

using namespace cv;
using namespace std;

VideoStream *vs;
Mat outImg, smallImg;
int fileNum=0;
char temp[512];

int main(int argc, char **argv)
{
	char key;

	// Face.ini Parameters
	FileConfiguration fconfig("/home/root/data/calibration/Face.ini");
	bool bShowFaceTracking = fconfig.getValue("FTracker.ShowFaceTracking", false);

	// Eyelock.ini Parameters
	FileConfiguration EyelockConfig("/home/root/Eyelock.ini");
	int m_ImageWidth = EyelockConfig.getValue("FrameSize.width", 1200);
	int m_ImageHeight = EyelockConfig.getValue("FrameSize.height", 960);
	bool m_ImageAuthentication = EyelockConfig.getValue("Eyelock.ImageAuthentication", true);

	outImg = Mat(Size(m_ImageWidth, m_ImageHeight), CV_8U);
	if (argc<2){
		printf("Not enough Parameters: Enter the port no of image capture\n");
		exit (0);
	}

	// vid_stream_start
	vs= new VideoStream(atoi(argv[1]), m_ImageAuthentication);
	sprintf(temp, "Disp %d",atoi (argv[1]) );

	int w,h;
	unsigned int count = 0;
	int s_canId;
	while (1)
	{
		vs->get(&w,&h,(char *)outImg.data, false);
		s_canId=vs->cam_id;

		for(int idx = 0; idx < 10; idx++){
			char fileName[50];
			sprintf(fileName,"%d_%d.pgm",atoi(argv[1]), idx);
			cv::imwrite(fileName,outImg);
		}
/*
		//For testing image optimization (OFFset correction)
		Mat DiffImage = imread("white.pgm",CV_LOAD_IMAGE_GRAYSCALE);
		Mat dstImage;
		if (DiffImage.cols!=0){
			dstImage=outImg-DiffImage;
			cv::resize(dstImage, smallImg, cv::Size(), 1, 1, INTER_NEAREST); //Time debug
		}else
			cv::resize(outImg, smallImg, cv::Size(), 1, 1, INTER_NEAREST); //Time debug
*/
		//MeasureSnr();
		char text[20];
	 	sprintf(text,"CAM %2x %s",s_canId,s_canId&0x80 ?  "AUX":"MAIN" );
		putText(smallImg,text,Point(10,60), FONT_HERSHEY_SIMPLEX, 1.5,Scalar(255,255,255),2);
		putText(smallImg,text,Point(10+1,60+1), FONT_HERSHEY_SIMPLEX, 1.5,Scalar(0,0,0),2);
		//	cv::resize(outImg, smallImg, cv::Size(), 0.25, 0.25, INTER_NEAREST); //Time debug

		if(bShowFaceTracking){
		// 	cv::resize(outImg, smallImg, cv::Size(320/2, 240/2), (1 / 2), (1 / 2), INTER_NEAREST);	//py level 3
			imshow("FaceTracker", outImg);  //Time debug
		}
	    key = cv::waitKey(1);

	    //For quit streaming
		if (key=='q')
			break;

		//For saving images while streaming individual cameras
		if(key=='s')
		{
			char fName[50];
			sprintf(fName,"%d_%d.pgm",atoi(argv[1]),fileNum++);
			cv::imwrite(fName,outImg);
			printf("saved %s\n",fName);

		}
		count++;
	}
}

