#include <stdio.h>
#include "BiOmega.h"
#include "cv.h"
#include "cxcore.h"
#include "highgui.h"
#include "LaplacianBasedFocusDetector.h"
#include <stdio.h>
#include <fstream>
#include <direct.h>
#include <Windows.h>
#include "pupilsegmentation.h"
#include <stdlib.h>
#include <fstream>
#include <string>
#include "blob_detect.h"
#include <fstream>
#include <sstream>
#include <conio.h>
#include "EyeDetectAndMatchServer.h"
//#include <vector.h>
using namespace std;

void saveImageSnippet(char imageName[], int height, int width, char* data) 
{
	int var1, var2; //unused variables
	char fName[200]="c:\\testimg\\";
	IplImage *img;
	sprintf(fName,"%s.bmp", imageName);  
	img = cvCreateImageHeader(cvSize(width, height),8/*depth*/,1);
	img->imageData=(char*)data;
	cvSaveImage(fName, img); //now it works fine
	cvReleaseImage(&img);   
}


void draw( IplImage* img1, float x, float y, float z, CvScalar color )
{
	CvPoint center = {(int)(x), (int)(y)};
	int radius = (int)(z);
	cvCircle( img1, center, radius, color, 1, 8, 0 );
}







int main(int argc, char* argv[])
{
	bool enrollMent = false;
	/**********laplacian initialization**************/
	CvFont font;
	double hScale=.5;
	double vScale=.5;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);
	
	//CreateDirectoryA(homeFolder.c_str(), NULL);


	// Anita EyeDetection
	EyeDetectAndMatchServer *m_pSrv;
	CSampleFrame m_sframe;
	int m_eyeDetectionLevel;
	bool detect = m_pSrv->Detect(&m_sframe,m_eyeDetectionLevel);

	std::string str; 
	ofstream myfile;
    myfile.open ("example.txt");

	LaplacianBasedFocusDetector lp = LaplacianBasedFocusDetector(320, 240);

	int cw = 480; //GRI.cropWidth=480
	int ch = 640; //GRI.cropHeight=640
	CvPoint3D32f outputTemp;
	bool m_shouldRotate = false;


	/**********Laplacian Initialization End***************/

	/**********Segmentation Initialization start***************/

	int m_width, m_height;
	int scale =1;
	bool fine = false; //conf.getValue("GRI.enableFineScale",false);
	if(fine) scale = 0;
	m_width = 640;//conf.getValue("GRI.cropWidth", 640);
	m_height = 480;//conf.getValue("GRI.cropHeight", 480);
	int stride = 640;



	IrisPupilCircles Circles;
	// unsigned char* m_IrisBuff;

	int minAngle = -90;
	int maxAngle = 90;
	printf("%d, %d", minAngle, maxAngle);
	// m_bioInstance = new BiOmega(m_width, m_height,scale, minAngle, maxAngle);
	CvScalar color = cvRealScalar(255);

	/**************Segmentation Initialization end*******************/

	/**************Read Iris Codes into Memory*******************/


	int MAX_TARGET_IRIS_COUNT = 300;
	char* IrisCode =  new char[2560];
	fstream MyBinFileRead;
	//std::vector<std::string> IrisFilesList;
	char IrisCodeName[256];

	//Reading the Iris's in dataBase
	char * dataBase = new char[2560 * MAX_TARGET_IRIS_COUNT];
	char * dataBasePtr = dataBase;
	int irisNum=0;
	std::ifstream IrisCodeList("C:/Outdoor_NXT/extData/database/input.txt");

	if (!enrollMent)
	{
		if(IrisCodeList.is_open()){

			while(IrisCodeList.getline(IrisCodeName,2560)){
				//IrisFilesList.push_back(IrisCodeName);
				MyBinFileRead.open(IrisCodeName,std::ios::in|std::ios::binary);
				if(MyBinFileRead.is_open())
				{
					MyBinFileRead.read(dataBase, 2560);
					dataBase+=2560;
					MyBinFileRead.close();

					if(++irisNum >= MAX_TARGET_IRIS_COUNT)
						break;
				}
				else 
				{
					printf("Error! %s not found !!!!", IrisCodeName);
					continue;
				}
			}
		}
		else 
		{
			printf("Error! Code List not found");
			//return 0;
		}
	}

	/***end Reading Iris codes

	/*************Read Images******************/
	std::ifstream file;
	if(enrollMent)
		file = std::ifstream("C:/Outdoor_NXT/EXTImages_person_with_glasses/EyeCrop_PG_4_13_18/inputEnroll.txt"); //"D:/EyeLockRnD/SegmentationRnD/inputWholeProblems1.csv";
	else
		file = std::ifstream("C:/Outdoor_NXT/extData/7_6_18_Debug/7_6_images/input.txt"); //"D:/EyeLockRnD/SegmentationRnD/inputWholeProblems1.csv";

	std:: string homeFolder = std::string("C:/Outdoor_NXT/extData/7_6_18_Debug/7_6_images");
	char temp[100]= "";
	sprintf(temp, "/MinMaxAngle");
	std::string thresholdFolder =   homeFolder + std::string(temp);
	CreateDirectoryA(thresholdFolder.c_str(), NULL);
	std::string unfocusedFolder =   thresholdFolder + "/badsegmentation";
	CreateDirectoryA(unfocusedFolder.c_str(), NULL);
	std::string focusedFolder =   thresholdFolder + "/goodSegmentation";
	CreateDirectoryA(focusedFolder.c_str(), NULL);
	unsigned char *m_IrisBuff=new unsigned char[2560];
	int totalImage = 0;
	int totalImagePassed = 0;
	int badImagesPassed = 0;
	int goodImagesPassed = 0;
	int matched = 0;
	int totalGlass = 0;

	DetectGlass dg= DetectGlass();


	BiOmega *m_bioInstance  = new BiOmega(m_width, m_height,scale);
	//m_bioInstance->GetEyeSegmentationInterface()->SetIrisRadiusSearchRange(50, 145);

				
	while (std::getline(file, str))
	{ 
		
		//if (str.find("EyeCrop") == string::npos)
			//continue;
		
		unsigned char *frame;
		float* robostFeatureVar;
		totalImage++;

		IplImage *m_centreEyecropEnroll = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 1);
		IplImage *m_rotatedEyecrop = cvCreateImage(cvSize(ch, cw), IPL_DEPTH_8U, 1);
		printf("%s\n", str.c_str());
		CvRect rect = { 160, 120, 320, 240 };
		IplImage *cec = NULL;
		IplImage* im = 0;
		//			IplImage* cec=0; 
		cec = m_centreEyecropEnroll;
		im=cvLoadImage(str.c_str(), 0);
		if(!im)
		{
			printf("error reading images\n");
			getchar();
			return 0;
		}
		//cvShowImage("Input", im);
		//m_bioInstance = new BiOmega(m_width, m_height,scale, minAngle, maxAngle);
		//Segmentation


		bool Glass = DetectGlass::detect_glass(im,im, 8,8,.05,2);//DetectGlass::detect_glass_effect(im,im, 8,8,0.01,2, 30);  //Mihir Glasses detection
		
		//Glass = dg.detect_glass_window((int)Glass);
		
		if (Glass)

		{   
			totalGlass++;
			//bool shouldPass = DetectGlass::detect_glass_Effect(im,im, 8,8,0.01,2, 60); 
			//if(!shouldPass)
			//	continue;
			//m_bioInstance->GetEyeSegmentationInterface()->SetIrisRadiusSearchRange(50, 145);
			printf("\nGlass Detected\n");
			//printf("New Iris Radius: %d\n", m_bioInstance->GetEyeSegmentationInterface()->get);
			//DetectGlass::detect_glass_Effect(im,im, 8,8,0.01,2, 60); 


		}
		else 
		{	//m_bioInstance->GetEyeSegmentationInterface()->SetIrisRadiusSearchRange(70, 145);
			printf("\nNo Glass \n");
		}
		



		int rc = m_bioInstance->GetIrisCode((unsigned char*)im->imageData, m_width, m_height, m_width,(char*)m_IrisBuff,&Circles,robostFeatureVar);
		int segmentationOkPupil = -1;
		int segmentationOkIris= 1;
		if (rc)
		{
			segmentationOkPupil = segmentation((unsigned char*)im->imageData, m_width, m_height, Circles.pp.x, Circles.pp.y, Circles.pp.r, Circles.ip.x, Circles.ip.y, Circles.ip.r, .18);
			//segmentationOkIris = segmentation((unsigned char*)im->imageData, m_width, m_height, Circles.ip.x, Circles.ip.y, Circles.ip.r, Circles.ip.x, Circles.ip.y, Circles.ip.r, 0.28);

		}
		//Laplacian
		CvPoint3D32f outputTemp;
		if (cec) {
			rect.x = MAX(0,(m_rotatedEyecrop->width-cec->width)/2);
			rect.y = MAX(0,(m_rotatedEyecrop->height-cec->height)/2);
			rect.width = cec->width;
			rect.height = cec->height;

			if (m_shouldRotate) {
				cvTranspose(im, m_rotatedEyecrop);
				cvFlip(m_rotatedEyecrop, m_rotatedEyecrop, 1);
				cvSetImageROI(m_rotatedEyecrop, rect);
				cvCopy(m_rotatedEyecrop, cec);
				cvResetImageROI(m_rotatedEyecrop);
			}
			else {
				cvSetImageROI(im, rect);
				cvCopy(im, cec);
				cvResetImageROI(im);
				cvFlip(cec, cec, 1);
			}

			outputTemp.x = -1.0f;
			outputTemp = lp.ComputeRegressionFocus(cec,
				255);
			//printf("\nLap during Enrollment:%0.3f\n", outputTemp.x); //For Enrollment Only
		}

		//std::pair<double, double> agcValues = AGC(im->width, im->height, (unsigned char *)im->imageData);
		//fprintf(fp, "%s, %0.3f,%0.3f, %0.3f\n", str.c_str(), outputTemp.x, agcValues.first, agcValues.second);
		char comment[4024]="";
		sprintf(comment, "Lap: %0.3f", outputTemp.x);
		cvPutText (im,comment,cvPoint(25,100), &font, cvScalar(0,0,0));
		cvPutText (im,comment,cvPoint(150,100), &font, cvScalar(255,255,255));

		std::string newName = str;
		int rep_loc= str.find("ug/");
		str.replace(0,rep_loc+3,"");
		//rep_loc= str.find("/");
		//str.replace(0,rep_loc+3,"");
		//str.replace(str.begin(), str.end(), '/', '_');
		str.replace(str.find("/"), 1, "_");
		//str.replace(str.find("/"), 1, "_");
		printf("%s", str.c_str());
		//std::string newName = str;
		if (outputTemp.x < .10)
		{
			printf("");
			sprintf(comment, "%s/%s.png",  unfocusedFolder.c_str(), str.c_str());
		}
		else
		{

			if (rc)
			{
				totalImagePassed++;

				if (Glass)
					sprintf(comment,"%s", ""); //sprintf(comment,"%s", "");
				else 
					sprintf(comment, "%s", "NoGlass"); //sprintf(comment, "%s", "NoGlass");

				cvPutText (im,comment,cvPoint(25,75), &font, cvScalar(0,0,0));
				cvPutText (im,comment,cvPoint(150,75), &font, cvScalar(255,255,255));


				draw(im, Circles.pp.x, Circles.pp.y, Circles.pp.r, color);
				draw(im, Circles.ip.x, Circles.ip.y, Circles.ip.r, color);
				if ((bool)segmentationOkPupil && (bool)segmentationOkIris)	
				{   
					if (enrollMent)
					{
						printf("Writing bin file\n");
						sprintf(comment, "C:/Outdoor_NXT/EXTImages_person_with_glasses/EyeCrop_PG_4_13_18/Enrolled_%d.bin", goodImagesPassed);
						std::string binFileName(comment);

						std::ofstream firstEye(binFileName,std::ios::binary|std::ios::trunc);
						firstEye<<std::string((char*)m_IrisBuff,2560);//<<std::string((char *)bestPairOfEyes.first->GetMask(),1280);



					}
					else
					{

						std::pair<int, float> result= m_bioInstance->MatchIrisCode((char *)m_IrisBuff ,dataBasePtr, irisNum);
						sprintf(comment, "Good Segment   MatchHD=%0.3f", result.second);
						//sprintf(comment, "Good_Segment");
						cvPutText (im,comment,cvPoint(25,25), &font, cvScalar(0,0,0));
						cvPutText (im,comment,cvPoint(25,50), &font, cvScalar(255,255,255));
						//fprintf(fp, "%s, %d, %0.3f\n", str.c_str(), result.first, result.second);
						char temp[300]="";
						sprintf(temp, "%s, %d, %0.3f\n", str.c_str(), result.first, result.second);
						myfile << string(temp);// <<endl;
						
						

						if (result.second < -0.3)
							{ 
								matched++;
								sprintf(comment, "%s_good.png",  newName.c_str());

						}
						else
							sprintf(comment, "%s/%s_good.png",  focusedFolder.c_str(), str.c_str());


					}

					//sprintf(comment, "%s_good.bmp",  str.c_str());
					//printf("%s", comment);
					goodImagesPassed++;


				}

				else
				{
					sprintf(comment, "Bad Segment");
					cvPutText (im,comment,cvPoint(50,50), &font, cvScalar(0,0,0));
					cvPutText (im,comment,cvPoint(150,50), &font, cvScalar(255,255,255));
					//sprintf(comment, "%s/%s",  unfocusedFolder.c_str(), newName.c_str());
					//sprintf(comment, "%s_bad.png",  newName.c_str());
					sprintf(comment, "%s/%s.png",  unfocusedFolder.c_str(), str.c_str());
					//printf("%s", comment);
					badImagesPassed++;
				}

				//sprintf(comment, "%s/ImageNo_%d.bmp", thresholdFolder.c_str(), totalImage);

				//sprintf(comment, "%s/%s.pgm", thresholdFolder.c_str(), newName.c_str());
				printf("%s", comment);
				int var1, var2;
				//cvShowImage("output", im);
				//cvWaitKey(50);
				//getchar();
				//saveImageSnippet(comment, im->width, im->height, im->imageData);
				cvSaveImage(comment, im);
				/*

				#if 0 
				// DetectGlass detectglassObj = DetectGlass(); //Window with size 10 

				for (int i = prevIrisListSize; i < currentIrisListSize; i++)
				{
				hasGlass = detectglassObj.detect_glass_window(m_EnrollmentEyesList[i]->get_hasGlass());
				if (hasGlass)
				break;		
				}	

				if (hasGlass)
				printf("Glass Detected !!!!!!!!!!!!!!!!!!\n");


				if(m_GlassDetection){
				bool Glass = DetectGlass::detect_glass(m_eyeCrop,3,3,0.04,2);  //Mihir Glasses detection
				//bool Glass = false;
				if(Glass == true)
				{
				G[0]=1;
				}
				G<<=1;
				}
				#endif
				*/


			}
		}


		//IplImage *eyeCrop = cvLoadImage("EyeCrop_PG_0.pgm",CV_LOAD_IMAGE_UNCHANGED);
		//cvShowImage("Input", eyeCrop);
		//cvWaitKey(10);
		//printf("Anita\n");


		//cvSaveImage(comment, im);
		cvReleaseImage(&im);
		cvReleaseImage(&m_centreEyecropEnroll);
		cvReleaseImage(&m_rotatedEyecrop);
		printf(	"\nTotal = %d, totalPassed= %d, totalGood= %d, TotalBad %d, TotalMatched= %d\n", totalImage, totalImagePassed,  goodImagesPassed, badImagesPassed, matched);
		printf(" TotalGlassPos = %d, NOGlass = %d\n", totalGlass, totalImage - totalGlass);
		// delete m_bioInstance; 

#if 0
		if(m_bioInstance)
		{
			delete m_bioInstance;
			m_bioInstance = 0;
		}
#endif
	}
	myfile.close();



	// delete[]m_IrisBuff; 


 }
