
#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"

#include <iostream>
#include <stdio.h>

using namespace std;
using namespace cv;

/** Function Headers */
void detectAndDisplay( Mat frame );

/** Global variables */
String face_cascade_name = "/home/root/data/haarcascades/haarcascade_frontalface_alt.xml";
String eyes_cascade_name = "/home/root/data/haarcascades/haarcascade_eye.xml";
CascadeClassifier face_cascade;
CascadeClassifier eyes_cascade;
String window_name = "Capture - Face detection";

/** @function main */
int face_init(  )
{
    Mat frame;

    //-- 1. Load the cascades
    if( !face_cascade.load( face_cascade_name ) ){ printf("face --(!)Error loading face cascade\n"); return -1; };
    //if( !eyes_cascade.load( eyes_cascade_name ) ){ printf("eye --(!)Error loading eyes cascade\n"); return -1; };

    //-- 2. Read the video stream
     return 0;
}

/** @function detectAndDisplay */
void detectAndDisplay( Mat frame )
{
    std::vector<Rect> faces;
    Mat frame_gray;
    int64 start = cv::getTickCount();

    frame.copyTo(frame_gray);

   // cvtColor( frame, frame_gray, COLOR_BGR2GRAY );

   // equalizeHist( frame_gray, frame_gray );

    //-- Detect faces
    face_cascade.detectMultiScale( frame_gray, faces, 1.1, 2, 0|CASCADE_SCALE_IMAGE, Size(30, 30) );

    for( size_t i = 0; i < faces.size(); i++ )
    {
        Point center( faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2 );
        ellipse( frame, center, Size( faces[i].width/2, faces[i].height/2), 0, 0, 360, Scalar( 255, 0, 255 ), 4, 8, 0 );

        Mat faceROI = frame_gray( faces[i] );
        std::vector<Rect> eyes;

        //-- In each face, detect eyes
        eyes_cascade.detectMultiScale( faceROI, eyes, 1.1, 2, 0 |CASCADE_SCALE_IMAGE, Size(30, 30) );

        for( size_t j = 0; j < eyes.size(); j++ )
        {
            Point eye_center( faces[i].x + eyes[j].x + eyes[j].width/2, faces[i].y + eyes[j].y + eyes[j].height/2 );
            int radius = cvRound( (eyes[j].width + eyes[j].height)*0.25 );
            circle( frame, eye_center, radius, Scalar( 255, 0, 0 ), 4, 8, 0 );
        }
    }

    //-- Show what you got
    //printf("Working121321321321321321321321321321321321321321321321321");
    imshow( window_name, frame );
}

int countFaces = 1;
char faceCrop[100];
int  FindEyeLocation( Mat frame , Point &eyes, float &eye_size)
{
    std::vector<Rect> faces;
    Mat frame_gray;
    int64 start = cv::getTickCount();
    int Neighbors =2;

	int t=clock();

    frame.copyTo(frame_gray);

    //Mohammad commented it out becuase hist eqlaization creating noise around
    //equalizeHist( frame, frame_gray );
    frame_gray = frame;

    //-- Detect faces
    while(1)
    {
    	//Mohammad
    	//CascadeClassifier::detectMultiScale(const Mat& image, vector<Rect>& objects, double scaleFactor=1.1,
    	//int minNeighbors=3, int flags=0, Size minSize=Size(), Size maxSize=Size())
    	//scaleFactor – Parameter specifying how much the image size is reduced at each image scale.
    	//Suppose, the scale factor is 1.03, it means we're using a small step for resizing, i.e. reduce size by 3 %
    	face_cascade.detectMultiScale( frame, faces, 1.1, Neighbors, 0|CASCADE_SCALE_IMAGE, Size(15, 15) , Size(75, 75));
   		break;
    }

    //Mohammad
    for( size_t i = 0; i < faces.size(); i++ )
    {
        //Point center( faces[i].x + faces[i].width/2, faces[i].y + faces[i].height/2 );
        //ellipse( frame, center, Size( faces[i].width/2, faces[i].height/2), 0, 0, 360, Scalar( 255, 0, 255 ), 2, 8, 0 );
        //rectangle(frame, Point(faces[i].x, faces[i].y),Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(255, 255, 255),2);
        rectangle(frame, Point(faces[i].x, faces[i].y),Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(255, 255, 255),1);
    }



#define EYES_MULT .3 // need to change
    cv::Rect roi;
    if (faces.size()==1)
    {

    	//Previous code
    	eyes.x= faces[0].x + faces[0].width/2;
    	eyes.y = faces[0].y + (float)faces[0].height*EYES_MULT;
    	printf("size = %d\n",faces[0].width);
    	eye_size = faces[0].width;
    	printf("face x = %d  face y = %d face width = %d  face height = %d \n",faces[0].x, faces[0].y, faces[0].width, faces[0].height);
    	Mat ROI(frame,Rect(faces[0].x, faces[0].y, faces[0].width, faces[0].height));
    	Mat cropImg;
    	ROI.copyTo(cropImg);
    	countFaces++;
    	faceCrop[100] = {0};

/*    	if (countFaces < 50){
    		sprintf(faceCrop,"y110_%iFace.bmp",countFaces);
    		imwrite(faceCrop, cropImg);
    	}
    	imwrite("y110Face.bmp", cropImg);*/
    	printf("Face detect tooc %3.3f\n",(float)(clock()-t)/CLOCKS_PER_SEC);
    	return 1;
    }

/*    Rect roi;
    roi.x = face[0].x;
    roi.y = face[0].y;
    roi.width = faces[0].x + faces[0].width;
    roi.height = faces[0].x + faces[0].height;


    Mat crop = frame(roi);
    imwrite("y.bmp", crop);*/
   return 0;
}


