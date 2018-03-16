

#include <stdlib.h>
#include <math.h>
#include <stdio.h>

#include <pthread.h>
#include <unistd.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/photo/photo.hpp>
#include <stdlib.h>
//#include <conio.h>

//#include "dynamixel_sdk.h"                                  // Uses Dynamixel SDK library

// Control table address
#define ADDR_PRO_TORQUE_ENABLE          24                 // Control table address is different in different Dynamixel model
#define ADDR_PRO_GOAL_POSITION          30
#define ADDR_PRO_PRESENT_POSITION       37
#define ADDR_PRO_GOAL_TORQUE            35
#define ADDR_PRO_MOVING                 49
#define ADDR_PRO_GOAL_VELOCITY          32
#define ADDR_PRO_CURR_VELOCITY          39

// Protocol version
#define PROTOCOL_VERSION                2.0                 // See which protocol version is used in the Dynamixel

// Default setting
#define DXL_ID                          1                   // Dynamixel ID: 1
#define BAUDRATE                        1000000
#define DEVICENAME                      "COM4"      // Check which port is being used on your controller
#define TORQUE_VLAUE                    1023
#define VELOCITY_VLAUE                  30
// ex) Windows: "COM1"   Linux: "/dev/ttyUSB0"

#define TORQUE_ENABLE                   1                   // Value for enabling the torque
#define TORQUE_DISABLE                  0                   // Value for disabling the torque
#define DXL_MINIMUM_POSITION_VALUE      150             // Dynamixel will rotate between this value
#define DXL_MAXIMUM_POSITION_VALUE      300             // and this value (note that the Dynamixel would not move when the position value is out of movable range. Check e-manual about the range of the Dynamixel you use.)
#define DXL_MOVING_STATUS_THRESHOLD     10                  // Dynamixel moving status threshold

#define ESC_ASCII_VALUE                 0x1b

#define CENTER_POS 240
#define MIN_POS 150
#define MAX_POS 300


//#include "VideoCapture.h"
#include "facetracker.h"
#include <opencv/cv.h>

//Comm files

extern int vid_stream_start(int port);
extern int vid_stream_get(int *win,int *hin, char *wbuffer);
extern int portcom_start();
extern void port_com_send(char *cmd);
extern void  *init_tunnel(void *arg);

#define WIDTH 1200
#define HEIGHT 960

//
class TickMeterX
{
public:
    TickMeterX();
    void start();
    void stop();

    int64 getTimeTicks() const;
    double getTimeMicro() const;
    double getTimeMilli() const;
    double getTimeSec()   const;
    int64 getCounter() const;

    void reset();
private:
    int64 counter;
    int64 sumTime;
    int64 startTime;
};

TickMeterX::TickMeterX()
{
    reset();
}
int64 TickMeterX::getTimeTicks() const
{
    return sumTime;
}
double TickMeterX::getTimeMicro() const
{
    return  getTimeMilli()*1e3;
}
double TickMeterX::getTimeMilli() const
{
    return getTimeSec()*1e3;
}
double TickMeterX::getTimeSec() const
{
    return (double)getTimeTicks()/cv::getTickFrequency();
}
int64 TickMeterX::getCounter() const
{
    return counter;
}
void TickMeterX::reset()
{
    startTime = 0;
    sumTime = 0;
    counter = 0;
}

void TickMeterX::start()
{
    startTime = cv::getTickCount();
}
void TickMeterX::stop()
{
    int64 time = cv::getTickCount();
    if ( startTime == 0 )
        return;
    ++counter;
    sumTime += ( time - startTime );
    startTime = 0;
}


int kbhit(void)
{
#ifndef __linux__
	struct termios oldt, newt;
	int ch;
	int oldf;

	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);

	ch = getchar();

	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);

	if (ch != EOF)
	{
		ungetc(ch, stdin);
		return 1;
	}

	return 0;
#elif defined(_WIN32) || defined(_WIN64)
	return _kbhit();
#endif
}

Mat enhanceImage(IplImage *inImage)
{
	cv::Mat grayscale_image = cv::cvarrToMat(inImage);
	// Mat_<uchar>  grayscale_image = Mat(inImage, true);
	//grayscale_image = grayscale_image + 50;
	//Mat_<uchar>  equalizeOut;
	//equalizeHist(grayscale_image, grayscale_image);
	return grayscale_image;
}

void MoveTo_R(int v)
{
	char buff[100];
	if (v<MIN_POS) v= MIN_POS;
	if (v>MAX_POS) v=MAX_POS;
	sprintf(buff,"fx_abs(%d)\n",v);
	printf("Sending move: %s\n",buff);
	port_com_send(buff);
}

int Rahman_main (int argc, char **argv)
{

	portcom_start();

	int newGoalPosition = 240;
	//Initial Position
	MoveTo_R(newGoalPosition);


	/*******Parameters**********/
	bool webcam = true;
	bool showGui = true, showTracking = true, skipOn = true;
	int skipFrameMax = 0, zoomOutFactor = 1;



	int  prevSuccessfullyMovedPos = 240, lastSearchPositionIndex = 0;
	bool isMotorMoved = false, keepSearching = false, FailedIteration = false;
	//int FailedCamerPosition[4] = {(196/8), ((196/4)+ 196/8) , ((196*2/4)+ 196/8), ((196*3/4)+ 196/8)};
	int FailedCamerPosition[2] = {5,160};
	int stablePersonPosition = 0;



	/*********Camera Initialization **************/
	VideoCapture  video_capture;
	Mat captured_image;

	int allreadySkipped = 0;
	long int totalFailedFrame = 0;

	//Comm code
	pthread_t threadId;
	pthread_create(&threadId,NULL,init_tunnel,NULL);
	vid_stream_start(atoi(argv[1]));


	/*************Initializations***************/
	FaceTracker trackerObj(FaceTracker::get_arguments(argc, argv), 0.1, 0.1);
	trackerObj.setCxCy(640, 480);

	/*******************Finished Initialization ********/

	TickMeterX tm;
	double fps=0;

	int exit_state=0;
	while (exit_state!=1)
	{  

		//if (showGui)
		//guiObj->init();
		if (webcam)
		{

			if (skipOn)
			{
				if (allreadySkipped >= skipFrameMax)
				{
					Mat source = Mat(Size(WIDTH,HEIGHT), CV_8U);;
					//Mihir
					//printf("Frame processed\n");
					//
					//video_capture >> source;
					int w,h;
					vid_stream_get(&w,&h,(char *)source.data);
					for (int i=0; i < zoomOutFactor; i++)
					{
						pyrDown( source, captured_image, Size( source.cols/2, source.rows/2));
						pyrDown( captured_image, captured_image, Size( captured_image.cols/2, captured_image.rows/2));
						//cv::resize(source, captured_image, cv::Size(), 0.15, 0.15, INTER_NEAREST);
					}
					//Rotating the image 
					//Point2f src_center(source.cols/2.0F, source.rows/2.0F);
					//Mat rot_mat = getRotationMatrix2D(src_center, -90, 1.0);
					//warpAffine(source, captured_image, rot_mat, source.size());
					//flip(captured_image, captured_image, 1);
					allreadySkipped = 0;
					//printf("down   cols:%d rows:%d \n",captured_image.cols,captured_image.rows);
					source.release();
				}
				else 
				{
					allreadySkipped++;
					//continue;
				}

			}
			else
			{

				pyrDown( captured_image, captured_image, Size( captured_image.cols/2, captured_image.rows/2));
				//flip(captured_image, captured_image, 1);
			}
		}

		tm.stop();
		tm.start();


		if(tm.getCounter() > 0)
		{
			fps = (double)tm.getCounter()/tm.getTimeSec();
		}


		std::pair<double, std::pair<double, double>> distance;


		if (allreadySkipped == 0)
		{
			//printf("Final   cols:%d rows:%d \n",captured_image.cols,captured_image.rows);
			//Old
			//distance = trackerObj.getDistance(captured_image, webcam, showTracking);
			distance = trackerObj.getDistance(captured_image, webcam, showTracking, fps);
			//printf("FPS ; %0.2f\n",fps);
			//imshow("test",captured_image);
			//cvWaitKey(1);

		}
		else 
		{
			distance.first = -1;
			distance.second.first = -1;
			distance.second.second = -1;
		}


		if (distance.first < 0) totalFailedFrame++;
		else totalFailedFrame = 0;


		if (showGui)
		{
			std::pair<double, double> estimatedPos;
			if ( allreadySkipped == 0)
			{	
				double distance3D = distance.first;
				estimatedPos = distance.second;
				//guiObj->update(distance.first, estimatedPos, true);
				//guiObj->showGUI();



				/************ For Motor COntrol*************/

				if (distance.first > 0)
				{

					int faceX = (int)(estimatedPos.first*100);
					int faceY = (int)(estimatedPos.second*100);
					int distanceT = (int) distance3D*5;
					double physicalDistance =  (0.00756*distance3D*distance3D + 0.187* distance3D + 5.067);
					double scale = distanceT/5;
					if (scale > 50)
						scale = 50.0;

					printf("\n scale:%d Y:%d",scale,faceY);
					double newFaceX = (faceX - 50)*scale;
					double newFaceY = (faceY - 50)*scale;
					int centerLEDx  = 10;
					int centerLEDy  = 10;
					int LEDOffsetx =  ((int)(newFaceX/250)) ;
					int LEDOffsety=  (int)(newFaceY/250);
					keepSearching = false;


#if 1

				if ((LEDOffsety > 2 || LEDOffsety < -2))
				{

					//printf("LEDOffsety: %d !!!\n", LEDOffsety);

					int motorControlGain = 10;    //Mihir-increment at which motor will move
					int newOffset = -1*LEDOffsety*motorControlGain;
					int newGoalPosition;
					if ( prevSuccessfullyMovedPos < DXL_MAXIMUM_POSITION_VALUE)
					newGoalPosition = prevSuccessfullyMovedPos - newOffset;
					else continue;

					isMotorMoved = true;
					prevSuccessfullyMovedPos = newGoalPosition;

					if (newGoalPosition < DXL_MINIMUM_POSITION_VALUE)
					{  
					newGoalPosition = DXL_MINIMUM_POSITION_VALUE;
					isMotorMoved = false;
					}
					if (newGoalPosition > DXL_MAXIMUM_POSITION_VALUE)
					{
					newGoalPosition = DXL_MAXIMUM_POSITION_VALUE;
					isMotorMoved = false;
					}
					MoveTo_R(newGoalPosition);
				}

			}


			else if (totalFailedFrame > 50)
			{
				int newGoalPosition = 240;
					//Initial Position
					MoveTo_R(newGoalPosition);

				/*FailedIteration = true;
				totalFailedFrame = 5;
				// Write New goal position
				int newGoalPosition;
				//Old
				if (!keepSearching)
				{
				if ( prevSuccessfullyMovedPos < 196/4)
				lastSearchPositionIndex = 0;
				else if ( prevSuccessfullyMovedPos >= 196/4 && prevSuccessfullyMovedPos < 196*2/4 )
				lastSearchPositionIndex = 1;
				else if ( prevSuccessfullyMovedPos >= 196*2/4 && prevSuccessfullyMovedPos < 196*3/4 )
				lastSearchPositionIndex = 2;
				else if ( prevSuccessfullyMovedPos >= 196*3/4 && prevSuccessfullyMovedPos < 196 )
				lastSearchPositionIndex = 3;
				newGoalPosition = FailedCamerPosition[lastSearchPositionIndex];
				keepSearching = true;
				lastSearchPositionIndex = 0;
				}
				if (!keepSearching)
				{faceX
					if ( prevSuccessfullyMovedPos < 196/2)
						lastSearchPositionIndex = 0;
					else if ( prevSuccessfullyMovedPos >= 196/2 && prevSuccessfullyMovedPos < 196 )
						lastSearchPositionIndex = 1;
					newGoalPosition = FailedCamerPosition[lastSearchPositionIndex];
					keepSearching = true;
					lastSearchPositionIndex = 0;
				}

				else 
				{
					newGoalPosition = FailedCamerPosition[lastSearchPositionIndex];
					if (lastSearchPositionIndex >= 1)
						lastSearchPositionIndex = 0;
					else 
						lastSearchPositionIndex++;


				}



				keepSearching = true;*/

				/* Rahman Start Commenting

				dxl_comm_result = packetHandler->write2ByteTxRx(portHandler, DXL_ID, ADDR_PRO_GOAL_POSITION, newGoalPosition, &dxl_error);


				//cvWaitKey(500);
				if (dxl_comm_result != COMM_SUCCESS)
				{
				packetHandler->printTxRxResult(dxl_comm_result);

				}
				else if (dxl_error != 0)
				{
				packetHandler->printRxPacketError(dxl_error);
				}
				else  printf("Write New Goal: %03d Successfully!, Failed 15\n", newGoalPosition);

				Rahman END Commenting*/
				//totalFailedFrame = 16;


				//OLD
				//uint8_t moving_status = 0;
				//do
				//{
				//	// Read present position
				//	dxl_comm_result = packetHandler->read4ByteTxRx(portHandler, DXL_ID, ADDR_PRO_PRESENT_POSITION, (uint32_t*)&dxl_present_position, &dxl_error);
				//	if (dxl_comm_result != COMM_SUCCESS)
				//	{
				//		packetHandler->printTxRxResult(dxl_comm_result);
				//	}
				//	else if (dxl_error != 0)
				//	{
				//		packetHandler->printRxPacketError(dxl_error);
				//	}

				//	printf("[Moving: ID:%03d] GoalPos:%03d  PresPos:%03d, Failed 15\n", DXL_ID, newGoalPosition, dxl_present_position);
				//	
				//	dxl_comm_result = packetHandler->read1ByteTxRx(portHandler, DXL_ID, ADDR_PRO_MOVING, &moving_status, &dxl_error);

				//}while((abs(newGoalPosition - dxl_present_position) > DXL_MOVING_STATUS_THRESHOLD) && moving_status==1);

			}	

		}

#endif





		else{		
			//estimatedPos= facekf->estimatefacePosition(distance.second, false, false, 640, 480, 1);
			//guiObj->update(-1, estimatedPos, false);
		}

	}
	//} //Comment it For Rahman's use case  (Uncomment for Gary's usecase)


	// Finalize COM
}
return 0;
}
