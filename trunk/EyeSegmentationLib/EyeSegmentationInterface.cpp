#include "EyeSegmentServer.h"
#include "EyeSegmentationInterface.h"

#include <opencv/cxcore.h>
#include "opencv/highgui.h"
#include "EyeFeatureServer.h"
#include "EyeMatchServer.h"
#include "useful.h"
#include "pupilsegmentation.h"

#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
//#include "FFTVarSpoofDetector.h"

// #define PROFILE

#ifdef PROFILE
#include <windows.h>
LARGE_INTEGER profiler_freq;
FILE * TimeInfo;
#define PROFILE_START(affine) LARGE_INTEGER start_time##affine; QueryPerformanceCounter(&start_time##affine);
#define PROFILE_END(affine)\
	LARGE_INTEGER end_time##affine;\
	static float total_time##affine = 0.0;\
	static int myCounter##affine=0;\
	myCounter##affine++;\
	QueryPerformanceCounter(&end_time##affine);\
	float time##affine =(float)(end_time##affine.QuadPart-start_time##affine.QuadPart)/profiler_freq.QuadPart*1000;\
	total_time##affine +=time##affine;\
	printf("\n%s = %fmS Total=%fmS iter = %d",#affine,time##affine, total_time##affine,myCounter##affine);\
	fprintf(TimeInfo,"\n%s = %fmS Total=%fmS iter = %d",#affine,time##affine, total_time##affine,myCounter##affine);
#else
#define PROFILE_START(affine)
#define PROFILE_END(affine)
#endif

extern "C" {
	#include "test_fw.h"
	#include "file_manip.h"
}
#ifdef __BFIN__
#include <bfin_sram.h>
#endif

void EyeSegmentationInterface::init(int scale, int w, int h)
{
	#ifdef PROFILE
	QueryPerformanceFrequency(&profiler_freq);
	TimeInfo = fopen("c:/profile_casia.txt","w");
	#endif

	m_pEyeSegmentServer = new EyeSegmentServer(w,h);
	m_pEyeFeatureServer = new EyeFeatureServer(8, (scale == 0)? 4:6);
	// Initialize Aus Segmentation code
	size_t flat_iris_height = 64;
	size_t flat_iris_width = 480;

	size_t template_height = 8;
	size_t template_width = 160;
	int base_scale = 6;

	/*	printf("Min Iris %d %d \n", m_AusIrisfind_min_Iris_Diameter, m_AusIrisfind_max_Iris_Diameter);
	printf("Min Pupil %d %d \n", m_AusIrisfind_min_pupil_Diameter, m_AusIrisfind_max_pupil_Diameter);
	printf("Min Pupil %d %d \n", m_AusIrisfind_min_spec_Diameter, m_AusIrisfind_max_spec_Diameter);


	m_AusIrisfind_min_Iris_Diameter = 60;
	m_AusIrisfind_max_Iris_Diameter = 180;
	m_AusIrisfind_min_pupil_Diameter = 16;
	m_AusIrisfind_max_pupil_Diameter = 60;
	m_AusIrisfind_min_spec_Diameter = 8;
	m_AusIrisfind_max_spec_Diameter = 20;
	*/

	m_AusSegment = new AusSegment(base_scale, m_AusIrisfind_EyeCropWidth, m_AusIrisfind_EyeCropWidth,flat_iris_width, flat_iris_height,
					template_width, template_height,
					m_AusIrisfind_min_Iris_Diameter, m_AusIrisfind_max_Iris_Diameter,
					m_AusIrisfind_min_pupil_Diameter,m_AusIrisfind_max_pupil_Diameter,
					m_AusIrisfind_min_spec_Diameter, m_AusIrisfind_max_spec_Diameter);


}
void EyeSegmentationInterface::SetLUT(int pupilmin5,int pupilmax64,int cirmin5,int cirmax255){
	if(m_pEyeSegmentServer)m_pEyeSegmentServer->compute_LUT_sqrtminmax(pupilmin5,pupilmax64,cirmin5,cirmax255);
}
int EyeSegmentationInterface::GetFeatureLength() const
{
	return m_pEyeFeatureServer->GetFeatureLength();
}
int EyeSegmentationInterface::GetFeatureByteSize() const
{
	return m_pEyeFeatureServer->GetFeatureByteSize();
}
int EyeSegmentationInterface::GetFeatureNumRows() const
{
	return m_pEyeFeatureServer->GetFeatureNumRows();
}
void EyeSegmentationInterface::term()
{
	if(m_pEyeSegmentServer) delete m_pEyeSegmentServer; m_pEyeSegmentServer = 0;
	if(m_pEyeFeatureServer) delete m_pEyeFeatureServer; m_pEyeFeatureServer = 0;
	if(m_AusSegment) delete m_AusSegment; m_AusSegment = 0;
}

EyeSegmentationInterface::EyeSegmentationInterface():m_index(0),m_maxCorruptBitsPercAllowed(70),m_bEnableAusSeg(false)
{
//	m_rng = cvRNG();
	m_eso = new EyeSegmentationOutput;
	m_flatMask = cvCreateImage(cvSize(480, 64), IPL_DEPTH_8U, 1);
	m_flatMaskIntegral = cvCreateImage(cvSize(480+1, 64+1), IPL_DEPTH_32S, 1);
	cvSetZero(m_flatMask);
}

EyeSegmentationInterface::~EyeSegmentationInterface()
{
	delete m_eso;
	cvReleaseImage(&m_flatMask);
	cvReleaseImage(&m_flatMaskIntegral);

}
void EyeSegmentationInterface::SetEyeLocationSearchArea(int xo, int yo, int w, int h)
{
	m_pEyeSegmentServer->SetEyeLocationSearchArea(cvRect(xo, yo, w, h));
}

void EyeSegmentationInterface::GetEyeLocationSearchArea(int& xo, int& yo, int& w, int& h)
{
	m_pEyeSegmentServer->GetEyeLocationSearchArea(xo, yo, w, h);
}

void EyeSegmentationInterface::SetIrisRadiusSearchRange(int min, int max)
{
	m_pEyeSegmentServer->SetIrisRadiusSearchRange(min, max);
}
void EyeSegmentationInterface::SetPupilRadiusSearchRange(int min, int max)
{
	m_pEyeSegmentServer->SetPupilRadiusSearchRange(min, max);
}
void EyeSegmentationInterface::SetPupilAngleSearchRange(int min, int max)
{
	m_pEyeSegmentServer->SetPupilAngleSearchRange(min, max);
}

void EyeSegmentationInterface::GetPupilAngleSearchRange(float& min, float& max)
{
	m_pEyeSegmentServer->GetPupilAngleSearchRange(min, max);
}

void EyeSegmentationInterface::SetPupilSearchAngles(float minAngle, float maxAngle)
{
	return m_pEyeSegmentServer->SetPupilSearchAngles(minAngle, maxAngle);
}

void EyeSegmentationInterface::SetFeatureNormalization(bool normalize)
{
	return m_pEyeFeatureServer->SetFeatureNormalization(normalize);
}

void EyeSegmentationInterface::SetSpecularityMaskLevel(int level)
{
	m_pEyeSegmentServer->SetSpecularityMaskLevel(level);
}

void EyeSegmentationInterface::SetUpperEyelidCenterandRadius(CvPoint cenPt,float rad ){
	m_pEyeSegmentServer->SetUpperEyelidCenterandRadius(cenPt,rad);
}

void EyeSegmentationInterface::SetLowerEyelidCenterandRadius(CvPoint cenPt,float rad ){
	m_pEyeSegmentServer->SetLowerEyelidCenterandRadius(cenPt,rad);
}

void EyeSegmentationInterface::EnableEyelidSegmentation(bool enable)
{
	m_pEyeSegmentServer->EnableEyelidSegmentation(enable);
}

void EyeSegmentationInterface::EnableAusSegmentation(bool bEnableAusSeg)
{
	m_bEnableAusSeg = bEnableAusSeg;
}
void EyeSegmentationInterface::EnableEyeQualityAssessment(bool enable)
{
	m_pEyeSegmentServer->EnableEyeQualityAssessment(enable);
}
bool EyeSegmentationInterface::GetEnableEyeQualityAssessment()
{
	return m_pEyeSegmentServer->GetEnableEyeQualityAssessment();
}

void EyeSegmentationInterface::SetCoarseSearchSampling(double sampling)
{
	m_pEyeSegmentServer->SetCoarseSearchSampling(sampling);
}
void EyeSegmentationInterface::SetFineSearchSampling(double sampling)
{
	m_pEyeSegmentServer->SetFineSearchSampling(sampling);
}
void EyeSegmentationInterface::SetEyelidSearchSampling(double sampling)
{
	m_pEyeSegmentServer->SetEyelidSearchSampling(sampling);
}
void EyeSegmentationInterface::EnableImprovedCostMetric(bool enable)
{
	m_pEyeSegmentServer->EnableImprovedCostMetric(enable);
}
void EyeSegmentationInterface::SetDefaultEyelidAngles(float upperEyelidAngle, float lowerEyelidAngle)
{
	m_pEyeSegmentServer->SetDefaultEyelidAngles(upperEyelidAngle, lowerEyelidAngle);
}

int EyeSegmentationInterface::GetMaxCorruptBitsPercAllowed()
{
	return m_maxCorruptBitsPercAllowed;
}
void EyeSegmentationInterface::SetMaxCorruptBitsPercAllowed(int perc)
{
	m_maxCorruptBitsPercAllowed=perc;
}

bool EyeSegmentationInterface::IsEyelidPresent()
{
	return m_pEyeSegmentServer->IsEyelidPresent();
}

IrisPupilCircles EyeSegmentationInterface::Segment(unsigned char *imageBuffer, int w, int h, int stride)
{
	printf("*********entering in eye segmentation *************\n");
	IplImage *image = cvCreateImageHeader(cvSize(w, h), IPL_DEPTH_8U, 1);

	cvSetData(image, imageBuffer, stride);

	// call the necessary functions;

	PROFILE_START(PROCESS)
	EyeSegmentationOutput out=m_pEyeSegmentServer->Process(image, m_index++);
	memcpy(m_eso, &(out), sizeof(EyeSegmentationOutput));
	PROFILE_END(PROCESS)

	IrisPupilCircles Circles;

	Circles.ip.x = m_eso->ip.x;
	Circles.ip.y = m_eso->ip.y;
	Circles.ip.r = m_eso->ip.z;

	Circles.pp.x = m_eso->pp.x;
	Circles.pp.y = m_eso->pp.y;
	Circles.pp.r = m_eso->pp.z;

	cvReleaseImageHeader(&image);

	printf("*********exiting eye segmentation *************\n");
	return(Circles);
}
bool EyeSegmentationInterface::GetRandomIrisCode(unsigned char *imageBuffer, int w, int h, int stride, unsigned char *Iriscode, unsigned char *Maskcode)
{
	printf("*********entering GetRandomIrisCode *************\n");
	IplImage *flatIris = cvCreateImageHeader(cvSize(w,h), IPL_DEPTH_8U, 1);
	cvSetData(flatIris, imageBuffer, stride);

	cvSetZero(m_flatMask);

	m_pEyeFeatureServer->ExtractFeatures(flatIris, m_flatMask, Iriscode, Maskcode);

	m_eso->flatMask = m_flatMask;
	cvReleaseImageHeader(&flatIris);

	return true;
}

#include <opencv/cv.h>

bool EyeSegmentationInterface::GetDefaultMaskCode(unsigned char *IrisCode, unsigned char *Maskcode)
{
	printf("*********entering GetDefaultMaskCode *************\n");
	cvSetZero(m_eso->flatMask);
	CvScalar color = cvRealScalar(255);

	float lowerRad, upperRad;
	CvPoint lowerCenPt, upperCenPt;

	m_pEyeSegmentServer->GetLowerEyelidCenterandRadius(lowerCenPt, lowerRad);
	cvCircle(m_eso->flatMask, lowerCenPt, cvRound(lowerRad), color, CV_FILLED);
	m_pEyeSegmentServer->GetUpperEyelidCenterandRadius(upperCenPt, upperRad);
	cvCircle(m_eso->flatMask, upperCenPt, cvRound(upperRad), color, CV_FILLED);
	m_pEyeFeatureServer->ExtractFeatures(m_eso->flatIris, m_eso->flatMask, IrisCode, Maskcode);
	memset(IrisCode, 0, m_pEyeFeatureServer->GetFeatureLength()); // zero out the iris code
	return false;
}

bool EyeSegmentationInterface::GetIrisCode2(unsigned char *flatIris, unsigned char *flatMask, int w, int h, int stride, unsigned char *Iriscode, unsigned char *Maskcode)
{
	printf("*********entering GetIrisCode2 *************\n");
	IplImage *flatIrisImage = cvCreateImageHeader(cvSize(w, h), IPL_DEPTH_8U, 1);
	cvSetData(flatIrisImage, flatIris, stride);

	IplImage *flatMaskImage = cvCreateImageHeader(cvSize(w, h), IPL_DEPTH_8U, 1);
	cvSetData(flatMaskImage, flatMask, stride);

	PROFILE_START(EXTRACTFEAT)
	m_pEyeFeatureServer->ExtractFeatures(flatIrisImage, flatMaskImage, Iriscode, Maskcode);
	PROFILE_END(EXTRACTFEAT)

	double corruptBitcount = m_pEyeFeatureServer->checkBitCorruption(Maskcode);
	corruptBitcount /= m_pEyeFeatureServer->GetFeatureLength() << 3;		// << 3 consequence of making iris code a byte
	corruptBitcount *=100;

	if(corruptBitcount > m_maxCorruptBitsPercAllowed)
		printf("Too many Bits Corrupted\n");

	cvReleaseImageHeader(&flatIrisImage);
	cvReleaseImageHeader(&flatMaskImage);

	return (corruptBitcount <= m_maxCorruptBitsPercAllowed);
}


_IplImage** EyeSegmentationInterface::GetImagePyramid(){
	return m_pEyeSegmentServer->GetImagePyramid();
}

void EyeSegmentationInterface::GetRobustFeatureVariances(float *var)
{
	for(int i=0; i<8; i++)
	{
		var[i]=m_pEyeFeatureServer->GetRobustFeatureVariance(i);
	}
}

#define MAX_LINE_LEN 256

int LoadEyelockConfigINIFile(){
	static FILE *fp;
	char line[MAX_LINE_LEN + 1] ;
	char *token; char *Value;
	int saveSegImages = 0;
	// char *saveseg = (char *)calloc(10,sizeof(char));
	fp = fopen("Eyelock.ini", "rb");
	if(fp == NULL) {
		printf("Can't open Eyelock.ini\n");
		//return -1;
	}else{
		while( fgets( line, MAX_LINE_LEN, fp ) != NULL )
		{
			token = strtok( line, "\t =\n\r" ) ;
			if( token != NULL && token[0] != '#' )
			{
				Value = strtok( NULL, "\t =\n\r" ) ;
				if(strcmp(token,"Eyelock.SaveSegmentedImages") == 0){
					if(strcmp(Value,"true") == 0)
						saveSegImages = 1;
					else
						saveSegImages = 0;
				}
			}
		}
		fclose(fp);
	}
	return saveSegImages;
}

bool EyeSegmentationInterface::GetIrisCode(unsigned char *imageBuffer, int w, int h, int stride, unsigned char *Iriscode, unsigned char *Maskcode, IrisPupilCircles *pCircles)
{
	if(m_bEnableAusSeg){
		bool bSegresult = false;
		size_t eyecrop_width = m_AusIrisfind_EyeCropWidth;
		size_t eyecrop_height = m_AusIrisfind_EyeCropHeight;
		printf("m_AusIrisfind_EyeCropWidth:%d m_AusIrisfind_EyeCropHeight:%d\n", m_AusIrisfind_EyeCropWidth, m_AusIrisfind_EyeCropHeight);
		if(eyecrop_width == 640 && eyecrop_height == 480){
			bSegresult = m_AusSegment->m_Iris->GenerateFlatIris(imageBuffer,(unsigned char*)m_AusSegment->m_flatIris->imageData,(unsigned char*)m_AusSegment->m_flatMask->imageData, eyecrop_width, eyecrop_height);
		}else{
			CvRect rect = {160, 120, 320, 240};
			memcpy(m_AusSegment->EyeCropHeader_640_480->imageData, imageBuffer,(640*480));
			// cvSetData(m_AusSegment->EyeCropHeader_640_480, imageBuffer, 640);
			cvSetImageROI(m_AusSegment->EyeCropHeader_640_480, rect);
			cvCopy(m_AusSegment->EyeCropHeader_640_480,m_AusSegment->EyeCropHeader_320_240);
			cvResetImageROI(m_AusSegment->EyeCropHeader_640_480);
			m_AusSegment->m_Iris->GenerateFlatIris((unsigned char*)m_AusSegment->EyeCropHeader_320_240->imageData,(unsigned char*)m_AusSegment->m_flatIris->imageData,(unsigned char*)m_AusSegment->m_flatMask->imageData, eyecrop_width, eyecrop_height);
			/*cv::Mat mateye = cv::cvarrToMat(m_AusSegment->EyeCropHeader_320_240);
			cv::imshow("Test",mateye);
			cvWaitKey(1);*/
			bSegresult = m_AusSegment->m_Iris->GenerateFlatIris((unsigned char*)m_AusSegment->EyeCropHeader_320_240->imageData,(unsigned char*)m_AusSegment->m_flatIris->imageData,(unsigned char*)m_AusSegment->m_flatMask->imageData, eyecrop_width, eyecrop_height);
		}
		if(bSegresult)
		{
			bool bTemplateResult = m_AusSegment->m_Encode->EncodeFlatIris((unsigned char*)m_AusSegment->m_flatIris->imageData,(unsigned char*)m_AusSegment->m_flatMask->imageData, Iriscode, Maskcode);
			if(bTemplateResult)
				return true;
			else
				return false;
		}
	}else{
		////printf("*********entering GetIrisCode *************\n");
		m_corruptBitcountPerc = 0.0f;
		IplImage *image = cvCreateImageHeader(cvSize(w, h), IPL_DEPTH_8U, 1);

		cvSetData(image, imageBuffer, stride);

		PROFILE_START(PROCESSIMAGE)
		memcpy(m_eso, &(m_pEyeSegmentServer->Process(image, m_index++)), sizeof(EyeSegmentationOutput));

		// printf("After Process m_eso->ip.x value %f\n", m_eso->ip.x);
		PROFILE_END(PROCESSIMAGE)
	
		CvScalar color = cvRealScalar(255);
	#if 0 //Anita
		if( m_eso->ip.x > 0 )
		{
			EyeSegmentationOutput tmp1 = *m_eso;

			draw( image, tmp1.pp, color );
			draw( image, tmp1.ip, color );
			cvSaveImage("segmented_Image.pgm",image);
		}
	#endif
	#if 0
		if( m_eso->ip.x > 0 )
		{
			draw( image, m_eso->pp, color );
			draw( image, m_eso->ip, color );
		}
		char fName[100];
		sprintf(fName,"eye_%d.pgm",m_index-1);
		cvSaveImage(fName,image);

	#endif



		//cvReleaseImageHeader(&image);

		memset( Iriscode, 0, m_pEyeFeatureServer->GetFeatureLength() );
		memset( Maskcode, -1, m_pEyeFeatureServer->GetFeatureLength() );
	#ifdef SEG
		printf("m_eso->ip.x...%d", m_eso->ip.x);
	#endif

		if( m_eso->ip.x < 0 )
		{
			cvSet( m_eso->flatIris, cvScalar(0) );
			cvSet( m_eso->flatMask, cvScalar(0) );
			return false;
		}

		// printf("Before pCircles\n");

		if(pCircles)
		{
			pCircles->ip.x = m_eso->ip.x;
			pCircles->ip.y = m_eso->ip.y;
			pCircles->ip.r = m_eso->ip.z;

			pCircles->pp.x = m_eso->pp.x;
			pCircles->pp.y = m_eso->pp.y;
			pCircles->pp.r = m_eso->pp.z;
		}




		// TO BE REMOVED _ JUST FOR TEST - IMPORTANT
	//	cvRandArr(&m_rng, m_eso->flatIris, CV_RAND_UNI, cvRealScalar(0), cvRealScalar(256));


		//SAVE_NAME_INDX(m_eso->flatIris,"flatIris",m_index);
		//SAVE_NAME_INDX(m_eso->flatMask,"flatMask",m_index);
		// printf("Before Extract Features\n");
		PROFILE_START(EXTRACTFEAT)
		XTIME_OP("ExtractFeatures",
		m_pEyeFeatureServer->ExtractFeatures(m_eso->flatIris, m_eso->flatMask, Iriscode, Maskcode)
		);
		PROFILE_END(EXTRACTFEAT)
		// printf("After Extract Features\n");
		/*char fName[100];
		sprintf(fName,"Iris_%d.pgm",m_index-1);
		cvSaveImage(fName,m_eso->flatIris);
		sprintf(fName,"Mask_%d.pgm",m_index-1);
		cvSaveImage(fName,m_eso->flatMask);*/


		PROFILE_START(GOODSEG)

		double corruptBitcount = m_pEyeFeatureServer->checkBitCorruption(Maskcode);
		corruptBitcount /= m_pEyeFeatureServer->GetFeatureLength() << 3;		// << 3 consequence of making iris code a byte
		corruptBitcount *=100;

		m_corruptBitcountPerc = (float)corruptBitcount;
		//Validation for Annular region
		int AnnularCheck = 0;
		float cdist = sqrt((m_eso->ip.x - m_eso->pp.x)*(m_eso->ip.x - m_eso->pp.x) + (m_eso->ip.y - m_eso->pp.y)*(m_eso->ip.y - m_eso->pp.y));
		if((m_eso->pp.z + cdist) > m_eso->ip.z - 10 || cdist > 30 )
			AnnularCheck = 0; //Failed
		else
			AnnularCheck = 1;

		CvPoint2D32f SegmentationCheck = m_pEyeSegmentServer->m_SegmentationCheck;
		PROFILE_END(GOODSEG)

		if(corruptBitcount > m_maxCorruptBitsPercAllowed)
			printf("Too many Bits Corrupted\n");

		char name[100];
		static int segmented_count;
		//char filename[100];
		//FILE *fp;
		// static int i;
		int SaveSegImages = LoadEyelockConfigINIFile();
		bool segresult = false;

		if(corruptBitcount <= m_maxCorruptBitsPercAllowed && (AnnularCheck==1)) // && Getiseye() ) // Original Check
		{
			if( m_eso->ip.x > 0 )
			{

				EyeSegmentationOutput tmp1 = *m_eso;
				// segmentation(unsigned char *data, int w, int h, float pupilX, float pupilY, float pupilR, float irisX, float irisY, float irisR)
				// bool status = (bool)segmentation((unsigned char*)imageBuffer, 640,480, m_eso->pp.x, m_eso->pp.y, m_eso->pp.z, m_eso->ip.x, m_eso->ip.y, m_eso->ip.z);

				double centerError = sqrt(abs((int)pCircles->ip.x-(int)pCircles->pp.x)*abs((int)pCircles->ip.x-(int)pCircles->pp.x) + abs((int)pCircles->ip.y-(int)pCircles->pp.y)*abs((int)pCircles->ip.y-(int)pCircles->pp.y));
				double imageCenterError = sqrt(abs(320-(int)pCircles->pp.x)*abs(320-(int)pCircles->pp.x) + abs(240-(int)pCircles->pp.y)*abs(240-(int)pCircles->pp.y));

				if ((centerError < 8) && (imageCenterError < 25) && (pCircles->ip.r/pCircles->pp.r > 2))
					segresult = true;
				else
				   segresult = false;

				int fontFace = CV_FONT_HERSHEY_SIMPLEX;
				double fontScale = 1;
				if(SaveSegImages){
					if (segresult)
					{
						draw( image, tmp1.pp, color );
						draw( image, tmp1.ip, color );
						sprintf(name, "Good_segmented_image_%d.bmp",segmented_count++);
						cv::Mat mateye = cv::cvarrToMat(image);
						std::ostringstream ssCoInfo;
						ssCoInfo << "pp=" << tmp1.pp.z << "ip" << tmp1.ip.z;
						putText(mateye, ssCoInfo.str().c_str(), cvPoint(30,30), fontFace, fontScale, cvScalar(0,122,122), 3, 4);
						imwrite(name, mateye);
						mateye.release();
						// cvSaveImage(name,image);
						//sprintf(filename,"text_%d.txt",segmented_count++);
						//fp = fopen(filename, "wb");
						//fprintf(fp, "%f %f %f %f %f %f %f", m_eso->pp.x, m_eso->pp.y, m_eso->pp.z, m_eso->ip.x, m_eso->ip.y, m_eso->ip.z);
						//fclose(fp);
						// cvSaveImage("segmented_Image.pgm",image);
					}
					else
					{
						draw( image, tmp1.pp, color );
						draw( image, tmp1.ip, color );
						sprintf(name, "Bad_segmented_image_%d.bmp",segmented_count++);
						cv::Mat mateye = cv::cvarrToMat(image);
						std::ostringstream ssCoInfo;
						ssCoInfo << "pp=" << tmp1.pp.z << "ip=" << tmp1.ip.z;
						putText(mateye, ssCoInfo.str().c_str(), cvPoint(30,30), fontFace, fontScale, cvScalar(0,122,122), 3, 4);
						imwrite(name, mateye);
						mateye.release();
						// cvSaveImage(name,image);
						//sprintf(filename,"text_%d.txt",segmented_count++);
						//fp = fopen(filename, "wb");
						//fprintf(fp, "%f %f %f %f %f %f %f", m_eso->pp.x, m_eso->pp.y, m_eso->pp.z, m_eso->ip.x, m_eso->ip.y, m_eso->ip.z);
						//fclose(fp);
					}
				}
			}
			cvReleaseImageHeader(&image);
			return segresult; // Iris and Segmentation is OK
		}
		else{
			printf("Inside GetIrisCode bad Seg\n");
			cvReleaseImageHeader(&image);
			return segresult; // Iris or Segmentation is NOT OK
		}
	}

}
#if 0
EyeSegmentationOutput EyeSegmentationInterface::GetFlatIrisMask(unsigned char *imageBuffer, int w, int h, int stride, unsigned char *Iriscode, unsigned char *Maskcode, IrisPupilCircles *pCircles)
{
	EyeSegmentationOutput EyeSegOut;
	bool status;
	////printf("*********entering GetIrisCode *************\n");
	m_corruptBitcountPerc = 0.0f;
	IplImage *image = cvCreateImageHeader(cvSize(w, h), IPL_DEPTH_8U, 1);

	cvSetData(image, imageBuffer, stride);

	PROFILE_START(PROCESSIMAGE)
	memcpy(m_eso, &(m_pEyeSegmentServer->Process(image, m_index++)), sizeof(EyeSegmentationOutput));

	// printf("After Process m_eso->ip.x value %f\n", m_eso->ip.x);
	PROFILE_END(PROCESSIMAGE)

	CvScalar color = cvRealScalar(255);
#if 0 //Anita
	if( m_eso->ip.x > 0 )
	{
		EyeSegmentationOutput tmp1 = *m_eso;

		draw( image, tmp1.pp, color );
		draw( image, tmp1.ip, color );
		cvSaveImage("segmented_Image.pgm",image);
	}
#endif
#if 0
	if( m_eso->ip.x > 0 )
	{
		draw( image, m_eso->pp, color );
		draw( image, m_eso->ip, color );
	}
	char fName[100];
	sprintf(fName,"eye_%d.pgm",m_index-1);
	cvSaveImage(fName,image);

#endif



	//cvReleaseImageHeader(&image);

	memset( Iriscode, 0, m_pEyeFeatureServer->GetFeatureLength() );
	memset( Maskcode, -1, m_pEyeFeatureServer->GetFeatureLength() );
#ifdef SEG
	printf("m_eso->ip.x...%d", m_eso->ip.x);
#endif

	if( m_eso->ip.x < 0 )
	{
		cvSet( m_eso->flatIris, cvScalar(0) );
        cvSet( m_eso->flatMask, cvScalar(0) );
		// return false;
	}

	// printf("Before pCircles\n");

	if(pCircles)
	{
		pCircles->ip.x = m_eso->ip.x;
		pCircles->ip.y = m_eso->ip.y;
		pCircles->ip.r = m_eso->ip.z;

		pCircles->pp.x = m_eso->pp.x;
		pCircles->pp.y = m_eso->pp.y;
		pCircles->pp.r = m_eso->pp.z;
    }




	// TO BE REMOVED _ JUST FOR TEST - IMPORTANT
//	cvRandArr(&m_rng, m_eso->flatIris, CV_RAND_UNI, cvRealScalar(0), cvRealScalar(256));


	//SAVE_NAME_INDX(m_eso->flatIris,"flatIris",m_index);
	//SAVE_NAME_INDX(m_eso->flatMask,"flatMask",m_index);
	// printf("Before Extract Features\n");
	PROFILE_START(EXTRACTFEAT)
	XTIME_OP("ExtractFeatures",
	m_pEyeFeatureServer->ExtractFeatures(m_eso->flatIris, m_eso->flatMask, Iriscode, Maskcode)
	);
	PROFILE_END(EXTRACTFEAT)
	// printf("After Extract Features\n");
	/*char fName[100];
	sprintf(fName,"Iris_%d.pgm",m_index-1);
	cvSaveImage(fName,m_eso->flatIris);
	sprintf(fName,"Mask_%d.pgm",m_index-1);
	cvSaveImage(fName,m_eso->flatMask);*/


	PROFILE_START(GOODSEG)

	double corruptBitcount = m_pEyeFeatureServer->checkBitCorruption(Maskcode);
	corruptBitcount /= m_pEyeFeatureServer->GetFeatureLength() << 3;		// << 3 consequence of making iris code a byte
	corruptBitcount *=100;

	m_corruptBitcountPerc = (float)corruptBitcount;
	//Validation for Annular region
	int AnnularCheck = 0;
	float cdist = sqrt((m_eso->ip.x - m_eso->pp.x)*(m_eso->ip.x - m_eso->pp.x) + (m_eso->ip.y - m_eso->pp.y)*(m_eso->ip.y - m_eso->pp.y));
	if((m_eso->pp.z + cdist) > m_eso->ip.z - 10 || cdist > 30 )
		AnnularCheck = 0; //Failed
	else
		AnnularCheck = 1;

	CvPoint2D32f SegmentationCheck = m_pEyeSegmentServer->m_SegmentationCheck;
	PROFILE_END(GOODSEG)

	if(corruptBitcount > m_maxCorruptBitsPercAllowed)
		printf("Too many Bits Corrupted\n");

	char name[100];
	static int segmented_count;
	//char filename[100];
	//FILE *fp;
	// static int i;
	int SaveSegImages = LoadEyelockConfigINIFile();

	if(SaveSegImages){
		EyeSegmentationOutput tmp1 = *m_eso;
		draw( image, tmp1.pp, color );
		draw( image, tmp1.ip, color );
		sprintf(name, "Good_segmented_image_%d.bmp",segmented_count++);
		cv::Mat mateye = cv::cvarrToMat(image);
		imwrite(name, mateye);
		mateye.release();
	}
#if 1
	if(corruptBitcount <= m_maxCorruptBitsPercAllowed && (AnnularCheck==1)) // && Getiseye() )
	{
		if( m_eso->ip.x > 0 )
		{
			memcpy(&EyeSegOut, m_eso, sizeof(EyeSegmentationOutput));
		}
		cvReleaseImageHeader(&image);

		return EyeSegOut;
	}
#else
	memcpy(&EyeSegOut, m_eso, sizeof(EyeSegmentationOutput));
	cvReleaseImageHeader(&image);
	return EyeSegOut;
#endif

}
#endif

bool EyeSegmentationInterface::Getiseye()
{
	return m_pEyeSegmentServer->Getiseye();
}
void EyeSegmentationInterface::GetFlatIris( unsigned char *flatIris, int w, int h, int stride)
{
	if(w == m_eso->flatIris->width && h == m_eso->flatIris->height)
	{
		IplImage *header = cvCreateImageHeader(cvSize(w, h), IPL_DEPTH_8U, 1);
		cvSetData(header, flatIris, stride);

		cvCopy( m_eso->flatIris, header );

		cvReleaseImageHeader(&header);
	}
}
void EyeSegmentationInterface::GetFlatMask( unsigned char *flatMask, int w, int h, int stride )
{
	if(w == m_eso->flatMask->width && h == m_eso->flatMask->height)
	{
		IplImage *header = cvCreateImageHeader(cvSize(w, h), IPL_DEPTH_8U, 1);
		cvSetData(header, flatMask, stride);

		cvCopy( m_eso->flatMask, header );
		cvReleaseImageHeader(&header);
	}
}
void EyeSegmentationInterface::GetSpecularityMask( unsigned char *Mask, int w, int h, int stride )
{
	IplImage *image = cvCreateImageHeader(cvSize(w, h), IPL_DEPTH_8U, 1);
	cvSetData(image, Mask, stride);

	cvCopy( m_eso->specMask, image );
}
int EyeSegmentationInterface::checkBitCorruption( unsigned char *tag )
{
	printf("*********entering checkBitCorruption *************\n");
	return m_pEyeFeatureServer->checkBitCorruption( tag );
}
int EyeSegmentationInterface::GetFeatureVariances(float *var)
{
	return m_pEyeFeatureServer->GetFeatureVariances(var);
}

IrisMatchInterface::IrisMatchInterface(int featureLength, int numRows, int byteSize,int shift):
m_pEyeMatchServer(0), m_featureLength(featureLength), m_numRows(numRows), m_byteSize(byteSize), m_shift(shift)
{
}

IrisMatchInterface::~IrisMatchInterface()
{
}

void IrisMatchInterface::init()
{
	m_pEyeMatchServer = new EyeMatchServer(m_featureLength, m_numRows, m_byteSize,m_shift);
}
void IrisMatchInterface::term()
{
	if(m_pEyeMatchServer) delete m_pEyeMatchServer; m_pEyeMatchServer=0;
}

void IrisMatchInterface::SetDoRawScore(bool raw)
{
	m_pEyeMatchServer->SetDoRawScore(raw);
}

void IrisMatchInterface::SetMinCommonBits(int commonBits)
{
	if(m_pEyeMatchServer)
		m_pEyeMatchServer->SetMinCommonBits(commonBits);
	else printf("Could not set min Common Bits\n");

}

std::pair<int, float> IrisMatchInterface::match_pair(unsigned char *Iriscode1, unsigned char *Maskcode1,unsigned char *Iriscode2, unsigned char *Maskcode2,unsigned int maskval)
{
	PROFILE_START(MATCH_FEATURES)
	float score;

			score = (float) m_pEyeMatchServer->Match(Iriscode1, Maskcode1, Iriscode2, Maskcode2,maskval);


	PROFILE_END(MATCH_FEATURES)
	return std::pair<int, float>(0,score);
}

std::pair<int, float> IrisMatchInterface::Match(unsigned char *Iriscode2, unsigned char *Maskcode2, int featureMask){
	float score;
	score = (float) m_pEyeMatchServer->Match(Iriscode2, Maskcode2, featureMask);
	return std::pair<int, float>(0,score);
}

std::pair<int, float> IrisMatchInterface::MatchDBNewOpt(unsigned char *database, int numCodes,bool greedy,float hammingscore,int featureMask, unsigned char *shiftedCode,unsigned char *shiftedMask){
	return m_pEyeMatchServer->MatchDBNewOpt(database,numCodes,greedy,hammingscore,featureMask,shiftedCode,shiftedMask);
}

std::pair<int, float> IrisMatchInterface::MatchDBNewCompress(unsigned char *database, int numCodes,bool greedy,float hammingscore,int featureMask, unsigned char *shiftedCode,unsigned char *shiftedMask){
	return m_pEyeMatchServer->MatchDBNewCompress(database,numCodes,greedy,hammingscore,featureMask,shiftedCode,shiftedMask);
}

void IrisMatchInterface::MakeShiftsForIris(unsigned char *f1ptr, unsigned char *m1ptr, unsigned char **shiftedCode,unsigned char **shiftedMask){
	return m_pEyeMatchServer->MakeShiftsForIris(f1ptr,m1ptr,shiftedCode,shiftedMask);
}


std::pair<int, float> IrisMatchInterface::MatchDBRotation(unsigned char *f1ptr,unsigned char *m1ptr, unsigned char *database, int numCodes,bool greedy,float hammingscore,int featureMask) {
	return m_pEyeMatchServer->MatchDBRotation(f1ptr,m1ptr,database,numCodes,greedy,hammingscore,featureMask);

}
void IrisMatchInterface::MakeShifts(unsigned char *Iriscode1, unsigned char *Maskcode1){
	m_pEyeMatchServer->MakeShifts(Iriscode1, Maskcode1);
}
void IrisMatchInterface::MakeShifts(unsigned char *Iriscode1, unsigned char *Maskcode1,unsigned int mask){
	m_pEyeMatchServer->MakeShifts(Iriscode1, Maskcode1,mask);
}


std::pair<int, float> IrisMatchInterface::MatchWithShifts(unsigned char *Iriscode2, unsigned char *Maskcode2,unsigned int maskval)
{
	float score;
	score = (float) m_pEyeMatchServer->MatchWithShifts(Iriscode2, Maskcode2,maskval);
	return std::pair<int, float>(0,score);
}
std::pair<int, int> IrisMatchInterface::match_pairNumDen(unsigned char *Iriscode1, unsigned char *Maskcode1,unsigned char *Iriscode2, unsigned char *Maskcode2, unsigned int maskval)
{
	return m_pEyeMatchServer->MatchNumDen(Iriscode1, Maskcode1, Iriscode2, Maskcode2, maskval);
}


std::pair<int, int> IrisMatchInterface::get_best_match_score()
{
	CvPoint pt = m_pEyeMatchServer->GetBestMatchScore();
	return std::pair<int, int>(pt.x, pt.y);
}

void IrisMatchInterface::SetShiftLeft(int left)
{
	m_pEyeMatchServer->SetShiftLeft(left);
}

void IrisMatchInterface::SetShiftRight(int right)
{
	m_pEyeMatchServer->SetShiftRight(right);
}
//
//float *IrisMatchInterface::GetHammingDistances()
//{
//	return m_pEyeMatchServer->GetHammingDistances();
//}

int IrisMatchInterface::GetNumberOfRotations()
{
	return m_pEyeMatchServer->GetNumberOfRotations();
}

void IrisMatchInterface::SetNominalCommonBits(int bits)
{
	m_pEyeMatchServer->SetNominalCommonBits(bits);
}
void IrisMatchInterface::SetFeatureScale(int scale)
{
	if(scale == 0)
		m_pEyeMatchServer->SetNominalCommonBits(5100.0);
	else
		m_pEyeMatchServer->SetNominalCommonBits(4100.0);
}

void IrisMatchInterface::SetFeatureScaleNew(int scale, float scaleFactor)
{
	if(scale == 6)
		m_pEyeMatchServer->SetNominalCommonBits(scaleFactor * 4100.0 * m_featureLength/1280);
	else
		m_pEyeMatchServer->SetNominalCommonBits(scaleFactor * 4100.0 * m_featureLength/1280);
}


int IrisMatchInterface::GetCoarseIrisCode(unsigned char *irisCode, unsigned char *maskCode, int length, unsigned char *coarseIrisCode,unsigned char *coarseMaskCode)
{

#ifdef __BFIN__
	GetCoarseIrisCodeAsm(irisCode,coarseIrisCode,length);
	GetCoarseIrisCodeAsm(maskCode,coarseMaskCode,length);
	return length >> 2;
#else
	length = length >> 2;
	unsigned char *ic = irisCode;
	unsigned char *mc = maskCode;
	// assumes that the row length of the iris code is divisble by 4
	for (int i = 0; i < length; i++, ic += 4, mc += 4) {
		coarseIrisCode[i] = (ic[0] & 0xC0) | ((ic[1] & 0xC0) >> 2) | ((ic[2] & 0xC0) >> 4) | ((ic[3] & 0xC0) >> 6);
		coarseMaskCode[i] = (mc[0] & 0xC0) | ((mc[1] & 0xC0) >> 2) | ((mc[2] & 0xC0) >> 4) | ((mc[3] & 0xC0) >> 6);
	}
	return length;
#endif
}

int IrisMatchInterface::GetCoarseIrisCodeFromCompactDB(unsigned char *irisCode, unsigned char *maskCode, int length, unsigned char *coarseIrisCode,unsigned char *coarseMaskCode)
{

#ifdef __BFIN__
	GetCoarseIrisCodeFromCompactDBAsm(irisCode,coarseIrisCode,length);
	GetCoarseIrisCodeFromCompactDBAsm(maskCode,coarseMaskCode,length);
	return length >> 2;
#else
	length = length >> 2;
	unsigned char *ic = irisCode;
	unsigned char *mc = maskCode;
	// assumes that the row length of the iris code is divisble by 4
	for (int i = 0; i < length; i++, ic += 2, mc += 2) {
		coarseIrisCode[i] = ((ic[0] & 0x0C)<<4) | ((ic[0] & 0xC0) >> 2) | (ic[1] & 0x0C) | ((ic[1] & 0xC0) >> 6);
		coarseMaskCode[i] = ((mc[0] & 0x0C)<<4) | ((mc[0] & 0xC0) >> 2) | (mc[1] & 0x0C) | ((mc[1] & 0xC0) >> 6);
	}
	return length;
#endif
}

AusSegment::AusSegment(int base_scale, size_t eyecrop_width, size_t eyecrop_height,
		size_t flat_iris_width, size_t flat_iris_height,
		size_t template_width, size_t template_height,
		int MinIrisDiameter, int MaxIrisDiameter,
		int MinPupilDiameter, int MaxPupilDiameter,
		int MinSpecDiameter, int MaxSpecDiameter)
{

	EyeCropHeader_640_480 = cvCreateImage(cvSize(640, 480), IPL_DEPTH_8U, 1);
	EyeCropHeader_320_240 = cvCreateImage(cvSize(320, 240), IPL_DEPTH_8U, 1);
	m_flatIris = cvCreateImage(cvSize(480, 64), IPL_DEPTH_8U, 1);
	m_flatMask = cvCreateImage(cvSize(480, 64), IPL_DEPTH_8U, 1);

	m_Iris = new AusIris(eyecrop_width, eyecrop_height, flat_iris_width,
				  flat_iris_height);

	m_Encode = new Encode(base_scale, flat_iris_width, flat_iris_height,
					   template_width, template_height);

	m_Iris->AusIris_init(eyecrop_width, eyecrop_height,
			MinIrisDiameter, MaxIrisDiameter,
			MinPupilDiameter, MaxPupilDiameter,
			MinSpecDiameter, MaxSpecDiameter);
}

AusSegment::~AusSegment()
{
	if(EyeCropHeader_640_480)
		cvReleaseImage(&EyeCropHeader_640_480);

	if(EyeCropHeader_320_240)
		cvReleaseImage(&EyeCropHeader_320_240);

	if(m_flatIris)
		cvReleaseImage(&m_flatIris);

	if(m_flatMask)
		cvReleaseImage(&m_flatMask);

	if(m_Iris){
		delete [] m_Iris;
		m_Iris = 0;
	}

	if(m_Encode){
		delete [] m_Encode;
		m_Encode = 0;
	}

}

int AusSegment::GenerateFlatIris(uint8_t* eyecrop, uint8_t* flat_iris,
                               uint8_t* partial_mask, size_t eyecrop_width, size_t eyecrop_height) {
  return m_Iris->GenerateFlatIris(eyecrop, flat_iris, partial_mask, eyecrop_width, eyecrop_height);
}

int AusSegment::GenerateEncodedTemplate(uint8_t* flat_iris, uint8_t* partial_mask,
                                      uint8_t* template_encode,
                                      uint8_t* template_mask) {
  return m_Encode->EncodeFlatIris(flat_iris, partial_mask, template_encode, template_mask);
}

void EyeSegmentationInterface::SetAusIrisfind_Iris_Diameter(unsigned short int MinIrisDiameter, unsigned short int MaxIrisDiameter)
{
	m_AusIrisfind_min_Iris_Diameter = MinIrisDiameter;
	m_AusIrisfind_max_Iris_Diameter = MaxIrisDiameter;
}

void EyeSegmentationInterface::SetAusIrisfind_Pupil_Diameter(unsigned short int MinPupilDiameter, unsigned short int MaxPupilDiameter)
{
	m_AusIrisfind_min_pupil_Diameter = MinPupilDiameter;
	m_AusIrisfind_max_pupil_Diameter = MaxPupilDiameter;  // 70, was 60 (faster), but missed eyes too close to camera
}

void EyeSegmentationInterface::SetAusIrisfind_Spec_Diameter(unsigned short int MinSpecDiameter, unsigned short int MaxSpecDiameter)
{
	m_AusIrisfind_min_spec_Diameter = MinSpecDiameter;
	m_AusIrisfind_max_spec_Diameter = MaxSpecDiameter;
}


void EyeSegmentationInterface::SetAusIrisfind_EyeCorpSize(int Width, int Height)
{
	m_AusIrisfind_EyeCropWidth = Width;
	m_AusIrisfind_EyeCropHeight = Height;
}

