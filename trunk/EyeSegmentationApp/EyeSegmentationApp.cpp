#include <stdio.h>
#include <cstdlib>
#include "BiOmega.h"
#include <opencv/cv.h>
#include "opencv/cxcore.h"
#include "opencv/highgui.h"
#include "LaplacianBasedFocusDetector.h"
#include <stdio.h>
#include <fstream>
#if 0
#include <direct.h>
#include <Windows.h>
#endif
#include "pupilsegmentation.h"
#include <stdlib.h>
#include <fstream>
#include <string>
#if 0
#include "blob_detect.h"
#endif
#include <iostream>
#include <fstream>
#include <sstream>
#if 0
#include <conio.h>
#endif
#include "EyeDetectAndMatchServer.h"
#include "FileConfiguration.h"
#include "HDMatcher.h"
#include "MatchProcessor.h"
//#include <vector.h>

using namespace std;
using namespace cv;
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv/cv.h>
#include <opencv2/core.hpp>
#include <opencv2/imgcodecs.hpp>

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

void setImage(IplImage *dst, IplImage *m_src)
{
	//if(m_Imageformat){
		int m_fileIndex = 1, m_fileEyeIndex=0,m_maxFramesReset=10;
		//const char *m_Imageformat="./data/Good_255.pgm";
		char *m_Imageformat="./data/Good_255.pgm";

		char fpath[1024];
		sprintf(fpath,m_Imageformat,m_fileIndex,m_fileEyeIndex);

		int w=-1,h=-1,bits=-1;
		// int ret = ReadPGM5WHandBits(fpath,&w,&h,&bits);

//		printf("Reading %s %d %d %d %d\n",fpath,w,h,bits,ret);
		/*if(!m_src && (ret != -1)){
			m_src = cvCreateImage(cvSize(w,h),bits,1);
		}

		if(ret == -1){
			if(m_fileIndex >= m_maxFramesReset){
				m_fileIndex = -1;
				m_fileEyeIndex = 2;
			}
			if(m_src)
				cvSetZero(m_src);
		}else{
			ReadPGM5(fpath,(unsigned char *)m_src->imageData,&w,&h,m_src->imageSize);
		}*/

		if(m_src){
			for(int i =0;i<min(dst->height,m_src->height);i++){
				unsigned char *inp,*out;
				inp = ((unsigned char *)m_src->imageData) + i*(m_src->widthStep);
				//out = ((unsigned char *)dst->imageData) + (i+(dst->height>>1)-(m_src->height>>1))*(dst->widthStep) + (dst->widthStep>>1);
				out = ((unsigned char *)dst->imageData) + (i+(dst->height>>1)-(m_src->height>>1))*(dst->widthStep) + (dst->widthStep>>1);
				memcpy(out,inp,min(dst->width,m_src->width));
			}
		}
		//m_il0 = m_fileIndex&0x1;
		m_fileEyeIndex++;
		if(m_fileEyeIndex >1){
			m_fileEyeIndex=0;
			m_fileIndex++;
		}
	//}
}

EyeDetectAndMatchServer *m_pSrv;	
#define WIDTH 1200
#define HEIGHT 960
int m_detectLevel = 2;


void configureDetector(){







	m_pSrv = new EyeDetectAndMatchServer(WIDTH,HEIGHT,m_detectLevel,"Eyelock.log");
	m_pSrv->LoadHaarClassifier("/home/root/adaboostClassifier.txt");

	// set the specularity mode
	FileConfiguration pConf("/home/root/Eyelock.ini");
	m_pSrv->SetSingleSpecMode(pConf.getValue("GRI.SingleSpecMode",false));
	m_pSrv->SetDoHaar(pConf.getValue("GRI.DoHaar",true));
	m_pSrv->SetHaarEyeZoom(pConf.getValue("GRI.HaarEyeZoom",m_pSrv->GetHaarEyeZoom()));
	m_pSrv->SetHaarImageShifts(pConf.getValue("GRI.HaarImageShifts",m_pSrv->GetHaarImageShifts()));
	m_pSrv->SetHaarImageSampling(pConf.getValue("GRI.HaarImageSampling",m_pSrv->GetHaarImageSampling()));

	bool val = pConf.getValue("GRI.CovarianceTestForDetection",false);

	m_pSrv->SetCovTestForDetection(val?1:0);
	m_pSrv->SetSpecCovEigenThresh(pConf.getValue("GRI.SpecularityCovarianceEigenvalueThreshold",m_pSrv->GetSpecCovEigenThresh()));
	m_pSrv->SetSpecEccThresh(pConf.getValue("GRI.SpecularityEccentricityThreshold",m_pSrv->GetSpecEccThresh()));



	EyeDetectorServer *detector=m_pSrv->GetEyeDetector();
	detector->SetSpecularityMagnitude(pConf.getValue("GRI.EyeDetectionSpecularityMagnitude",15));
	int a = detector->GetSpecularitySize();

	detector->SetMaskRadius(pConf.getValue("GRI.EyeDetectionMaskRadius",10));
	detector->SetVarianceThresholdMin(pConf.getValue("GRI.EyeDetectionVarianceThresholdMin",1.5f));
	detector->SetVarianceThresholdMax(pConf.getValue("GRI.EyeDetectionVarianceThresholdMax",0.666f));
	detector->SetSeparation(pConf.getValue("GRI.EyeDetectionSeparation",36));
	detector->SetSearchX(pConf.getValue("GRI.EyeDetectionSearchX",15));
	detector->SetSearchY(pConf.getValue("GRI.EyeDetectionSearchY",10));
	detector->SetBoxX(pConf.getValue("GRI.EyeDetectionBoxX",detector->GetSpecularitySize()));
	detector->SetBoxY(pConf.getValue("GRI.EyeDetectionBoxY",detector->GetSpecularitySize()));
}

int detectEyes(IplImage* inputImage)
{	
	configureDetector();
	static int EyeCropCnt;
	// cvSaveImage("Test.pgm", inputImage);
		
	

	CSampleFrame frame;
	frame.setScratch(m_pSrv->GetScratch());
	static int i=0;
	Image8u img(inputImage,false);
	frame.SetImage(&img);
	bool result = false;
	
	result = m_pSrv->Detect(&frame, m_detectLevel);
	
	IplImage *m_eyeCrop = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
	// bool detect = m_pSrv->Detect(&m_sframe,m_eyeDetectionLevel);
	if(result)
	{		
		int	haarEyes = frame.GetNumberOfHaarEyes();
		printf("haarEyes......%d\n", haarEyes);

		EyeCenterPointList *pointList = (frame.GetEyeCenterPoints());
		EyeCenterPointList::iterator iter = pointList->begin();
		char outputFileName[200];
		sprintf(outputFileName,"EyeCropDE.pgm",EyeCropCnt++);
		for(int j=0; j<haarEyes; ++j)
		{
			int left,top;
			static int EyeCropCnt;
			frame.GetCroppedEye(j,m_eyeCrop,left,top);
			cv::Mat mateye = cv::cvarrToMat(m_eyeCrop);
			imwrite("EyeCropDE.pgm",mateye);
			// cvSaveImage(outputFileName,m_eyeCrop);
			// cvShowImage("EyecropImage",m_eyeCrop);
			// cvWaitKey(1);
		}
	}

	if(m_eyeCrop)
		cvReleaseImage(&m_eyeCrop);
	
	getchar(); 
}

int m_width = 640;
int m_height = 480;
IplImage *m_scaleDest;
IplImage *m_scaleSrcHeader;

int m_expectedIrisWidth = 200;
int m_actualIrisWidth = 120;

IplImage * ResizeFrame(int width, int height, unsigned char *frame) {

	float ratio = (m_expectedIrisWidth * 1.0) / m_actualIrisWidth;
	m_scaleDest = cvCreateImage(cvSize(m_width, m_height), IPL_DEPTH_8U, 1);

	cvSetZero(m_scaleDest);
	if (m_scaleSrcHeader != 0) {
	if (m_scaleSrcHeader->width != width || m_scaleSrcHeader->height
	!= height) {
	delete m_scaleSrcHeader;
	m_scaleSrcHeader = 0;
	}
	}
	if (m_scaleSrcHeader == 0)
	m_scaleSrcHeader = cvCreateImageHeader(cvSize(width, height),
	IPL_DEPTH_8U, 1);


	CvRect scaleROI;
	scaleROI.width = width * ratio;
	scaleROI.height = height * ratio;
	scaleROI.x = (640 - scaleROI.width) >> 1;
	scaleROI.y = (480 - scaleROI.height) >> 1;
	cvSetImageROI(m_scaleDest, scaleROI);

	scaleROI.width = 640 / ratio;
	scaleROI.height = 480 / ratio;
	scaleROI.x = (width - scaleROI.width) >> 1;
	scaleROI.y = (height - scaleROI.height) >> 1;
	cvSetImageROI(m_scaleSrcHeader, scaleROI);


	cvSetData(m_scaleSrcHeader, frame, m_scaleSrcHeader->width);
	cvResize(m_scaleSrcHeader, m_scaleDest);

	return m_scaleDest;
}
#define WIDTH 1200
#define HEIGHT 960

int main(int argc, char* argv[])
{
	string side = "right.pgm";
	bool enrollMent = false;
	/**********laplacian initialization**************/
	CvFont font;
	double hScale=.5;
	double vScale=.5;
	int    lineWidth=1;
	cvInitFont(&font,CV_FONT_HERSHEY_SIMPLEX|CV_FONT_ITALIC, hScale,vScale,0,lineWidth);
	std::string str;
	ofstream myfile;
	myfile.open ("example.txt");
	FileConfiguration pConf("/home/root/Eyelock.ini");
	float lapThresh = pConf.getValue("Eyelock.Laplacian_focus_Threshold", 0.15f);
	LaplacianBasedFocusDetector lp = LaplacianBasedFocusDetector(320, 240);

	int cw = 480; //GRI.cropWidth=480
	int ch = 640; //GRI.cropHeight=640
	CvPoint3D32f outputTemp;
	bool m_shouldRotate = false;

	/**********Laplacian Initialization End***************/

#if 1 //Anita
	/**********Segmentation Initialization start***************/

	int m_width, m_height;
	int scale =1;
	bool fine = false; //conf.getValue("GRI.enableFineScale",false);
	if(fine) scale = 0;
	m_width = 640;//conf.getValue("GRI.cropWidth", 640);
	m_height = 480;//conf.getValue("GRI.cropHeight", 480);
	int stride = 640;
	CvScalar color = cvRealScalar(255);
	float* robustFeatureVar;
	/**************Segmentation Initialization end*******************/

	/***********************Eye Detection**************************/
	configureDetector();

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
	std::ifstream IrisCodeList("/home/eyelock/BlainDB/input.txt");

	if (!enrollMent)
	{
		if(IrisCodeList.is_open()){

			while(IrisCodeList.getline(IrisCodeName,2560)){
				printf("%s\n", IrisCodeName);
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

	/********end Reading Iris codes*******************/

	/********Managing the input and output folder*******************/
	std::ifstream file;
	std:: string homeFolder;
	if(enrollMent)
	{
		string inputFileName = "C:/Outdoor_NXT/EXTImages_person_with_glasses/EyeCrop_PG_4_13_18/inputEnroll.txt";
		file = std::ifstream(inputFileName);
		int rep_loc1= inputFileName.find_last_of("/");
		inputFileName.replace(rep_loc1,inputFileName.length(),"");
		homeFolder	 = std::string(inputFileName);
	}
	else

	{
		// take the input directory eiether as eye crop or full frame images
		string inputFileName = "/home/eyelock/SortingData/input.txt";
		file = std::ifstream(inputFileName);
		int rep_loc1= inputFileName.find_last_of("/");
		inputFileName.replace(rep_loc1,inputFileName.length(),"");
		homeFolder	 = std::string(inputFileName);

	}

	//create output directory
	char temp[100]= "";
	sprintf(temp, "/outputs");
	std::string thresholdFolder =   homeFolder + std::string(temp);
	string comm = "mkdir -p ";
	string parseComm = comm + thresholdFolder;
	//printf("COMM:::::::::::::::::::::: %s\n", parseComm.c_str());

	int dir_err = system(parseComm.c_str());
	if (dir_err == -1)
	{
	    printf("Error creating directory!n");
	    exit(1);
	}


	//create EyeCrops directory
	std::string eyeCropFolder;
	eyeCropFolder =   thresholdFolder + "/EyeCrops";
	parseComm = comm + eyeCropFolder;
	//printf("COMM:::::::::::::::::::::: %s\n", parseComm.c_str());
	dir_err = system(parseComm.c_str());
	if (dir_err == -1)
	{
		printf("Error creating directory!n");
		exit(1);
	}

	//create badsegmentation directory
	std::string unfocusedFolder =   thresholdFolder + "/badsegmentation";
	parseComm = comm + unfocusedFolder;
	//printf("COMM:::::::::::::::::::::: %s\n", parseComm.c_str());
	dir_err = system(parseComm.c_str());
	if (dir_err == -1)
	{
		printf("Error creating directory!n");
		exit(1);
	}

	//create goodSegmentation directory
	std::string focusedFolder =   thresholdFolder + "/goodSegmentation";
	parseComm = comm + focusedFolder;
	//printf("COMM:::::::::::::::::::::: %s\n", parseComm.c_str());
	dir_err = system(parseComm.c_str());
	if (dir_err == -1)
	{
		printf("Error creating directory!n");
		exit(1);
	}


	unsigned char *m_IrisBuff=new unsigned char[2560];

	int totalImage = 0;
	int totalImagePassed = 0;
	int badImagesPassed = 0;
	int goodImagesPassed = 0;
	int matched = 0;
	int totalGlass = 0;

	//DetectGlass dg= DetectGlass();
#if 0
	BiOmega *m_bioInstance  = new BiOmega(m_width, m_height,scale);
	m_bioInstance->GetEyeSegmentationInterface()->SetIrisRadiusSearchRange(50, 180);
	m_bioInstance->GetEyeSegmentationInterface()->SetPupilRadiusSearchRange(12,48);
	m_bioInstance->set
#else
		BiOmega *m_bioInstance = new BiOmega(m_width, m_height,scale);
		int pupilmin5 = pConf.getValue("GRI.minPupilLutValue", 5);


		int pupilmax64 = pConf.getValue("GRI.maxPupilLutValue", 64);
		int cirmin5 = pConf.getValue("GRI.minCircleLutValue", 5);
		int cirmax255 = pConf.getValue("GRI.maxCircleLutValue", 255);

		m_bioInstance->SetLUT(pupilmin5,pupilmax64,cirmin5,cirmax255);

		bool eyelidseg = pConf.getValue("GRI.enableEyelidSegmentation", true);
		m_bioInstance->SetEnableEyelidSegmentation(eyelidseg);

		//set Upper eyelid center and radius
		CvPoint depPt ;
		float defRad ;
		depPt.x = pConf.getValue("GRI.upperEyelidCenterX", 360);
		depPt.y = pConf.getValue("GRI.upperEyelidCenterY", 190);
		defRad = pConf.getValue("GRI.upperEyelidRadius", 140.0f);
		m_bioInstance->SetUpperEyelidCenterandRadius(depPt,defRad);

		depPt.x = pConf.getValue("GRI.lowerEyelidCenterX", 130);
		depPt.y = pConf.getValue("GRI.lowerEyelidCenterY", 190);
		defRad = pConf.getValue("GRI.lowerEyelidRadius", 135.0f);
		m_bioInstance->SetLowerEyelidCenterandRadius(depPt,defRad);

		// Pupil Min and Max
		int minPupilDiameter = pConf.getValue("GRI.minPupilDiameter", 16);
		int maxPupilDiameter = pConf.getValue("GRI.maxPupilDiameter", 85);
		m_bioInstance->SetPupilRadiusSearchRange(minPupilDiameter, maxPupilDiameter);

		//Iris Min and Max
		int minIrisDiameter = pConf.getValue("GRI.minIrisDiameter", 70);
		int maxIrisDiameter = pConf.getValue("GRI.maxIrisDiameter", 145);
		m_bioInstance->SetIrisRadiusSearchRange(minIrisDiameter, maxIrisDiameter);

		int minPupilAngle = pConf.getValue("GRI.minPupilAngle", -60);
		int maxPupilAngle = pConf.getValue("GRI.maxPupilAngle",  90);
		m_bioInstance->SetPupilAngleSearchRange(minPupilAngle, maxPupilAngle);

		//Search Area ROI
		CvRect searchArea;
		m_bioInstance->GetEyeLocationSearchArea(searchArea.x, searchArea.y,
				searchArea.width, searchArea.height);
		searchArea.x = pConf.getValue("GRI.EyeLocationSearchArea.x", searchArea.x);
		searchArea.y = pConf.getValue("GRI.EyeLocationSearchArea.y", searchArea.y);
		searchArea.width = pConf.getValue("GRI.EyeLocationSearchArea.width",
				searchArea.width);
		searchArea.height = pConf.getValue("GRI.EyeLocationSearchArea.height",
				searchArea.height);
		bool fail = m_bioInstance->SetEyeLocationSearchArea(searchArea.x,
				searchArea.y, searchArea.width, searchArea.height);


		// max corrupt bits percentage
		m_bioInstance->SetMaxCorruptBitsPercAllowed(
				pConf.getValue("GRI.MaxCorruptBitPercentage"
						,m_bioInstance->GetMaxCorruptBitsPercAllowed()));
#endif
	ofstream saveMatchScore, imposterScore;
	string matchPath = homeFolder + "/matchScore.csv";
	string impMatchPath = homeFolder + "/imposterScore.csv";

	saveMatchScore.open(matchPath, ios::out | ios::app);
	imposterScore.open(impMatchPath, ios::out | ios::app);


	// FileConfiguration pConf("/home/root/Eyelock.ini");
	int size = pConf.getValue("GRI.HDMatcher.0.BuffSize",64000000);
	int id = pConf.getValue("GRI.HDMatcherID",0);
	bool useCoarseFine = pConf.getValue("GRI.useCoarseFineMatch",false);
	int featureMask = pConf.getValue("GRI.MatcherFeatureMask",255);

	// Anita
	int nominalCommonBits = pConf.getValue("GRI.Match.NominalCommonBits",4100);
	int minCommonBitsFine = pConf.getValue("GRI.Match.MinCommonBits",0);
	int minCommonBitsCoarse = pConf.getValue("GRI.Match.Coarse.MinCommonBits",(minCommonBitsFine)/4);
	int maxCorrBitPer = pConf.getValue("GRI.MaxCorruptBitPercentageEnrollment",70);
	bool compressedMatching = pConf.getValue("GRI.CompressedMatching",false);
	unsigned int maskcode = pConf.getValue("GRI.MatcherFeatureMask",255);
	unsigned int maskval = (maskcode<<24)|(maskcode<<16)|(maskcode<<8)|(maskcode);
	bool lowernibble = pConf.getValue("GRI.MatcherUseLowerNibble",true);

	bool greedy = pConf.getValue("GRI.GreedyMatch",false);
	float thresh = pConf.getValue("GRI.matchScoreThresh", 0.13f);
	float coarsethresh = pConf.getValue("GRI.matchCoarseThresh",0.35f);
	bool coarseFineMatch = pConf.getValue("GRI.useCoarseFineMatch",false);
	int shift = pConf.getValue("GRI.HDMatcherShift", 12);

	// printf("shift.....%d nominalCommonBits %d\n", shift, nominalCommonBits);

	HDMatcher *m_HDMatcherInstance = new HDMatcher(size, id, useCoarseFine, featureMask);
	m_HDMatcherInstance->m_numIris = 2;
	m_HDMatcherInstance->SetCommonBits(nominalCommonBits,minCommonBitsFine,minCommonBitsCoarse);
	m_HDMatcherInstance->SetMaxCorruptBitsPercAllowed(maxCorrBitPer*1.0f);
	m_HDMatcherInstance->SetMaskCode(maskval);
	m_HDMatcherInstance->SetlowerNibble(lowernibble);
	m_HDMatcherInstance->SetCompressedMatching(compressedMatching);
	m_HDMatcherInstance->StartMatchInterface(shift,greedy,thresh,coarsethresh,1280);
	DBAdapter * dbAdapter;
	unsigned char* coarseDB = NULL;
	// m_HDMatcherInstance->InitializeDb(dbAdapter,(unsigned char*)dataBasePtr, coarseDB);
	//m_HDMatcherInstance->Init();

	//m_HDMatcherInstance->CheckFromCorruptList(irisNum);



	/*************Read Full frame Images from input.txt******************/
	while (std::getline(file, str))
	{
		//printf("Inside while\n");

		//if (str.find("EyeCrop") == string::npos)
		//continue;

/*		printf("Before str processinf :::: %s\n", str.c_str());
		//Replace all forward slash with blackward slash
		for (int i = 0; i < str.length(); i++){
			if (str[i] == '\\')
				str[i] = '/';
		}*/

		//printf("After str processinf ::"
		//		":: %s\n", str.c_str());

		std::string newName = str;
		//Turn this macro on if you want to pass eyecrop images as a full frame image
#if 0
		IplImage *frame1;
		frame1 = cvLoadImage(str.c_str(), CV_LOAD_IMAGE_GRAYSCALE);
		if(!frame1)
			{
				printf("error reading images\n");
				getchar();
				return 0;
			} else {
				printf("Successfully load the image\n");
			}
#else
		IplImage *frame1 = cvCreateImage(cvSize(WIDTH,HEIGHT),IPL_DEPTH_8U,1);

		cv::Mat sendImg, image;
		image=imread(str.c_str(),0);
		sendImg = cv::Mat(cv::Size(WIDTH,HEIGHT), CV_8U);
		image.copyTo(sendImg(cv::Rect(0,0,image.cols, image.rows)));
		// cvConvertScale(sendImg, frame1, 1.0, 0);
		IplImage copy = sendImg;
		frame1 = &copy;
		//cv::imshow("Test", sendImg);
		// cvWaitKey(0);
#endif


		bool processFullImage;

		//check whther the input image is full frame or just eyecrop images
		if (frame1->width > 650)
			processFullImage= true;
		else processFullImage= false;

		printf("processFullImage >>> %s\n", processFullImage ? "true" : "false");

		/*********EyeDetection if fullframe as input*********/

		static int EyeCropCnt;
		CSampleFrame frame;
		frame.setScratch(m_pSrv->GetScratch());
		Image8u img(frame1,false);
		frame.SetImage(&img);
		bool resultDetect = true;


#if 1
		if(processFullImage){
			// detectEyes(frame1);
			resultDetect = m_pSrv->Detect(&frame, m_detectLevel);
		}
#endif
		if(resultDetect)
		{
			int	haarEyes;
			if(processFullImage)
				haarEyes= frame.GetNumberOfHaarEyes();
			else haarEyes = 1;
			printf("haarEyes......%d\n", haarEyes);

			EyeCenterPointList *pointList = (frame.GetEyeCenterPoints());
			EyeCenterPointList::iterator iter = pointList->begin();
			char outputFileName[200];
			for(int j=0; j<haarEyes; ++j)
			{
				IrisPupilCircles Circles;

				totalImage++;
				IplImage *m_centreEyecropEnroll = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 1);
				IplImage *m_rotatedEyecrop = cvCreateImage(cvSize(ch, cw), IPL_DEPTH_8U, 1);
				//printf("%s\n", str.c_str());
				CvRect rect = { 160, 120, 320, 240 };
				IplImage *cec = NULL;
				IplImage* im = 0;
				cec = m_centreEyecropEnroll;
				if(processFullImage)
				{
					im = cvCreateImage(cvSize(640,480),IPL_DEPTH_8U,1);
					int left,top;
					frame.GetCroppedEye(j,im,left,top);
					EyeCropCnt++;

				}
				else
					im = cvLoadImage(str.c_str(), CV_LOAD_IMAGE_GRAYSCALE);


				/****Start Processing the eyecrop*********/
				bool Glass = false;

#if 0
				// DetectGlass::detect_glass(im,im, 8,8,.05,2);//DetectGlass::detect_glass_effect(im,im, 8,8,0.01,2, 30);  //Mihir Glasses detection

				//Glass = dg.detect_glass_window((int)Glass);

				if (Glass)

				{
					totalGlass++;

					printf("\nGlass Detected\n");
					//printf("New Iris Radius: %d\n", m_bioInstance->GetEyeSegmentationInterface()->get);
					//DetectGlass::detect_glass_Effect(im,im, 8,8,0.01,2, 60);


				}
				else
				{	//m_bioInstance->GetEyeSegmentationInterface()->SetIrisRadiusSearchRange(70, 145);
					printf("\nNo Glass \n");
				}

#endif

#if 1
				IplImage *outImage1 = cvCreateImageHeader(cvSize(640,480), IPL_DEPTH_8U, 1);
				outImage1 = ResizeFrame(640, 480, im->imageData);

#endif

				//Actual segmnetation (rc) 0 for unsuccesful seg and  1 for successful seg
				int rc = m_bioInstance->GetIrisCode((unsigned char*)outImage1->imageData, m_width, m_height, m_width,(char*)m_IrisBuff,&Circles,robustFeatureVar);

				FILE *fp = fopen("App_iris_buff.bin","wb");
				fwrite(m_IrisBuff,1,2560,fp);
				fclose(fp);

			//int rc = m_bioInstance->GetIrisCode((unsigned char*)frame, m_width, m_height, m_width,(char*)m_IrisBuff,&Circles,robustFeatureVar);
				int segmentationOkPupil = -1;
				int segmentationOkIris= 1;

				printf("Corrupt Bit check rc >>>>> %d\n", rc);

				if (rc)
				{
					//if val is 0 then seg check is bad, if 1 then good
					//segmentationOkPupil = segmentation((unsigned char*)im->imageData, m_width, m_height, Circles.pp.x, Circles.pp.y, Circles.pp.r, Circles.ip.x, Circles.ip.y, Circles.ip.r, 0.275);
					//printf("Rahjmans segementation check:::: %d\n", segmentationOkPupil);
					//segmentationOkIris = segmentation((unsigned char*)im->imageData, m_width, m_height, Circles.ip.x, Circles.ip.y, Circles.ip.r, Circles.ip.x, Circles.ip.y, Circles.ip.r, 0.28);

					segmentationOkPupil = true;
					segmentationOkIris = true;
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
				}

				char comment[4024]="";
				//printf("Before processFullImage\n ");
#if 1
				if(processFullImage)
				{
					str = newName;
					int rep_loc= str.find(".pgm");
					str.replace(rep_loc, str.length(),"");
					char tempName[1024]="";
					sprintf(tempName, "%s_eyeId_%d", str.c_str(), j);
					str = std::string(tempName);

				}

				int rep_loc= str.find_last_of("/");
				str.replace(0,rep_loc+1,"");

				if(processFullImage) //Save Eyecrop to the Eyecrops folder
				{
					sprintf(outputFileName,"%s/%s.pgm",eyeCropFolder.c_str(), str.c_str());
					printf("%s\n", outputFileName);
					cv::Mat mateye = cv::cvarrToMat(im);
					imwrite("eyesegeye.pgm",mateye);
					// cvSaveImage(outputFileName,im);
				}
#endif

				//printf("Image name >>>>>> 		%s\n", str.c_str());
				printf("Laplacian Score >>> 			%0.3f\n", outputTemp.x);

				outputTemp.x = 1.0;
				if (outputTemp.x < 0/*lapThresh*/)
				{
					sprintf(comment, "Bad-lap=%0.3f", outputTemp.x);
					//cvPutText (im,comment,cvPoint(25,50), &font, cvScalar(0,0,0));
					cvPutText (im,comment,cvPoint(175,50), &font, cvScalar(255,255,255));
					sprintf(comment, "%s/BadLap%s.png",  unfocusedFolder.c_str(), str.c_str());
					//badImagesPassed++;
					printf("%s", comment);
					int var1, var2;
					// cvSaveImage(comment, im);
				}
				else
				{

					if (rc)
					{
						//printf("Before totalImagePassed\n");
						totalImagePassed++;
#if 0
						if (Glass)
							sprintf(comment,"%s", ""); //sprintf(comment,"%s", "");
						else
							sprintf(comment, "%s", ""); //sprintf(comment, "%s", "NoGlass");

						//cvPutText (im,comment,cvPoint(25,50), &font, cvScalar(0,0,0));
						cvPutText (im,comment,cvPoint(150,50), &font, cvScalar(255,255,255));

#endif
						//draw(im, Circles.pp.x, Circles.pp.y, Circles.pp.r, color);
						//draw(im, Circles.ip.x, Circles.ip.y, Circles.ip.r, color);

						//printf("segmentationOkPupil check >>> %s	segmentationOkIris check >>> %s\n", segmentationOkPupil ? "true" : "false", segmentationOkIris ? "true" : "false");
						if ((bool)segmentationOkPupil && (bool)segmentationOkIris)
						{
							//printf("Inside segmentationOkPupil\n");
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

								//printf("Test Match\n");
								// std::pair<int, float> result = m_HDMatcherInstance->MatchIrisCode(NULL, NULL, NULL);
								std::pair<int, float> result = m_HDMatcherInstance->MatchIrisCode((unsigned char*)m_IrisBuff ,(unsigned char*)dataBasePtr, coarseDB);
								string check;
								check = str;
								//printf("Before modification		%s		%s\n", str.c_str(), check.c_str());
								int rep_locc= check.find_last_of("_");
								check.replace(0,rep_locc+1,"");
								printf("%s			%s		%s\n", str.c_str(), check.c_str(), side.c_str());

								if (check.compare(side) == 0){
									printf("Authentic score ------------------		%0.5f\n", result.second);
									saveMatchScore << str.c_str() << "," << result.second << endl;
								} else{
									printf("Imposter score ------------------		%0.5f\n", result.second);
									imposterScore << str.c_str() << "," << result.second << endl;
								}

								sprintf(comment, "Good-Seg lapScore =%0.3f, Match score = %0.3f", outputTemp.x, result.second);
								//sprintf(comment, "Good_Segment");
								//cvPutText (im,comment,cvPoint(25,50), &font, cvScalar(0,0,0));
								cvPutText (im,comment,cvPoint(175,50), &font, cvScalar(255,255,255));
								//fprintf(fp, "%s, %d, %0.3f\n", str.c_str(), result.first, result.second);
								char temp[300]="";
								sprintf(temp, "%s, %d, %0.3f\n", str.c_str(), result.first, result.second);
								myfile << string(temp);// <<endl;
								sprintf(comment, "%s/%s_good.png",  focusedFolder.c_str(), str.c_str());
							}
							goodImagesPassed++;
						}

						else
						{
							//std::pair<int, float> result= m_bioInstance->MatchIrisCode((char *)m_IrisBuff ,dataBasePtr, irisNum);
							sprintf(comment, "Bad-Seg lapScore =%0.3f", outputTemp.x);
							//cvPutText (im,comment,cvPoint(25,50), &font, cvScalar(0,0,0));
							cvPutText (im,comment,cvPoint(175,50), &font, cvScalar(255,255,255));

							sprintf(comment, "%s/%s.png",  unfocusedFolder.c_str(), str.c_str());
							printf("%s", comment);
							badImagesPassed++;
						}


						//printf("%s", comment);
						int var1, var2;
						//cvSaveImage(comment, im);



					}
					else
					{

						sprintf(comment, "No-Seg lapScore =%0.3f", outputTemp.x);
						//cvPutText (im,comment,cvPoint(25,50), &font, cvScalar(0,0,0));
						cvPutText (im,comment,cvPoint(175,50), &font, cvScalar(255,255,255));
						//sprintf(comment, "%s/%s",  unfocusedFolder.c_str(), newName.c_str());
						//sprintf(comment, "%s_bad.png",  newName.c_str());
						sprintf(comment, "%s/NoSeg%s.png",  unfocusedFolder.c_str(), str.c_str());
						printf("%s", comment);
						badImagesPassed++;
						printf("%s", comment);
						int var1, var2;
						// cvSaveImage(comment, im);
					}
				}



				cvReleaseImage(&im);
				cvReleaseImage(&m_centreEyecropEnroll);
				cvReleaseImage(&m_rotatedEyecrop);
				//printf(	"\nTotal = %d, totalPassed= %d, totalGood= %d, TotalBad %d, TotalMatched= %d\n", totalImage, totalImagePassed,  goodImagesPassed, badImagesPassed, matched);
				//printf(" TotalGlassPos = %d, NOGlass = %d\n", totalGlass, totalImage - totalGlass);
			}
			// delete m_bioInstance;

#if 0
			if(m_bioInstance)
			{
				delete m_bioInstance;
				m_bioInstance = 0;
			}
#endif
		}
		cvReleaseImage(&frame1);

	}
	saveMatchScore.close();
	myfile.close();

#endif

	// delete[]m_IrisBuff;


}
