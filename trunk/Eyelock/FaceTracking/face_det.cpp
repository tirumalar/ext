#include "opencv2/objdetect/objdetect.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

#include "logging.h"

using namespace std;
using namespace cv;

const char logger[30] = "facedetect";

/** Function Headers */
int face_init();
int FindEyeLocation(Mat frame, Point &eyes, float &eye_size, Rect &face);

/** Global variables */
String face_cascade_name =
		"/home/root/data/haarcascades/haarcascade_frontalface_alt.xml";
CascadeClassifier face_cascade;

String window_name = "Capture - Face detection";

/** Face initialization */
int face_init() {
	EyelockLog(logger, TRACE, "face_init");
	Mat frame;

	//-- 1. Load the cascades
	if (!face_cascade.load(face_cascade_name)) {
		EyelockLog(logger, ERROR, "face --(!)Error loading face cascade\n");
		printf("face --(!)Error loading face cascade\n");
		return -1;
	};

	return 0;
}

#define EYES_MULT 0.3		// previous val is 0.3
/** Find face and eye locaiton */
int FindEyeLocation(Mat frame, Point &eyes, float &eye_size, Rect &face) {
	EyelockLog(logger, TRACE, "FindEyeLocation");
	std::vector<Rect> faces;
	Mat frame_gray;
	int64 start = cv::getTickCount();
	int Neighbors = 2;
	vector<int> reject_levels;
	vector<double> level_weights;

	int t = clock();

	frame.copyTo(frame_gray);

	/*    //Mohammad commented it out becuase hist eqlaization creating noise around
	 equalizeHist( frame, frame_gray );*/

	frame_gray = frame;

	//-- Detect faces
	//CascadeClassifier::detectMultiScale(const Mat& image, vector<Rect>& objects, double scaleFactor=1.1,
	//int minNeighbors=3, int flags=0, Size minSize=Size(), Size maxSize=Size())
	//scaleFactor – Parameter specifying how much the image size is reduced at each image scale.
	//Suppose, the scale factor is 1.03, it means we're using a small step for resizing, i.e. reduce size by 3 %
	while (1) {
		//face_cascade.detectMultiScale( frame, faces, reject_levels, level_weights, 1.1, Neighbors, 0|CASCADE_SCALE_IMAGE, Size(15, 15) , Size(75, 75),true);
		face_cascade.detectMultiScale(frame, faces, 1.1, Neighbors,
				0 | CASCADE_SCALE_IMAGE, Size(15, 15), Size(75, 75));
		break;
	}

/*	//Drawing rectangle on a face
	for (size_t i = 0; i < faces.size(); i++) {
		rectangle(frame, Point(faces[i].x, faces[i].y),
				Point(faces[i].x + faces[i].width,
						faces[i].y + faces[i].height), Scalar(255, 255, 255),
				1);
	}*/

	//If the detectMultiScale finds multiple face the function will return 0, otherwise 1
	cv::Rect roi;
	if (faces.size() == 1)	//if it finds only 1 face
			{
		eyes.x = faces[0].x + faces[0].width / 2;//x is equal to the 1st point of the face and rect + half face width
		eyes.y = faces[0].y + (float) faces[0].height * EYES_MULT;//y is equal to the 2nd point of the face rect + 30% of face height

		eye_size = faces[0].width;		// eye_size is equal ot face rext width

		//printf("face x = %d  face y = %d face width = %d  face height = %d reject %f levels %d\n",faces[0].x, faces[0].y, faces[0].width, faces[0].height,level_weights[0],reject_levels[0]);

		face.x = faces[0].x;
		face.y = faces[0].y;
		face.height = faces[0].height;
		face.width= faces[0].width;

		// printf("FindEyeLocation face x = %d  face y = %d face width = %d  face height = %d\n", face.x, face.y, face.width, face.height);
		//printf("Face detect tooc %3.3f\n",
		//		(float) (clock() - t) / CLOCKS_PER_SEC );
		return 1;
	}

	return 0;
}

