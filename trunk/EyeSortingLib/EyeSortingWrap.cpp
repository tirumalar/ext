#if defined(NO_SORTWRAP)
#include<string>
#ifdef __WIN32
#include <Windows.h>
#endif
#include "EyeSegmentationInterface.h"
#include "EyeSorting.h"
#include"EyeSortingWrap.h"
#include "IrisSelector.h"
#include <opencv/highgui.h>
#include <stdexcept>

EyeSortingWrap::EyeSortingWrap(int FeatureScale, int maxSpecularityValue, int EarlyTimeoutMS, int GlobalTimeoutMS) : m_UserIris((IrisCodeMask *)0, (IrisCodeMask *)0), m_LogEnable(false)
,m_deviceType(MYRIS), m_focusBenchMark(220),m_imgQltyBenchMark(220)
{
	
		m_pEyeSegmentationInterface = new EyeSegmentationInterface(); 
		try		
		{
			m_pEyeSegmentationInterface->init(FeatureScale);
		}
		catch (std::exception ex)
		{ 
			const char *pmessage = ex.what();
			
		}
		m_pEyeSegmentationInterface->SetFeatureNormalization(false);

		m_pIrisMatchInterface = new IrisMatchInterface(m_pEyeSegmentationInterface->GetFeatureLength(),
														m_pEyeSegmentationInterface->GetFeatureNumRows(),
														m_pEyeSegmentationInterface->GetFeatureByteSize());
		int weights[8] = {1,1,1,1,0,0,0,0};

		int scale_weights[32];
		for(int i = 0; i < 4; i++)
		{
			for(int j = 0; j < 8; j++)
			{
				scale_weights[i*8+j] = weights[j];
			}
		}

		int weightSum = 0;
		for(int i = 0; i < 8; i++)
		{
			weightSum += (weights[i]);
		}
		
		int featureMask = 0xffffffff;

		float commonBitScale = float(weightSum) / 8;
#if 0 //Anita
		m_pIrisMatchInterface->init(featureMask, scale_weights);
		m_pIrisMatchInterface->SetFeatureScale(1, commonBitScale);
#else
		m_pIrisMatchInterface->init();
		m_pIrisMatchInterface->SetFeatureScaleNew(1, commonBitScale);
#endif
     	m_pEyeSorting = new EnrollmentServer(FeatureScale, m_pIrisMatchInterface, maxSpecularityValue, (m_deviceType==MYRIS));
		
		m_pEyeSorting->SetEyeSegmentationInterface(m_pEyeSegmentationInterface);
		m_pEyeSorting->SetEarlyTimeout(EarlyTimeoutMS);
		m_pEyeSorting->SetGlobalTimeout(GlobalTimeoutMS);
		
}


void EyeSortingWrap::SetImgQltyBenchMark(int val){
    m_imgQltyBenchMark = val;
}
void EyeSortingWrap::SetFocusBenchMark(int val){
    m_focusBenchMark = val;
}

double EyeSortingWrap::GetLaplacianScore(unsigned char *frame, int Width, int Height)
{
	return m_pEyeSorting->CalculateLaplacianScore(frame, Width, Height);
}

EyeSortingWrapperFeedback EyeSortingWrap::GetEyeSortingFeedback()
{
	EyeSortingWrapperFeedback *pFeedback = new EyeSortingWrapperFeedback;

	pFeedback->calibration = m_pEyeSorting->m_Feedback->calibration;
	pFeedback->hardOcclusion = m_pEyeSorting->m_Feedback->hardOcclusion;
	pFeedback->softOcclusion = m_pEyeSorting->m_Feedback->softOcclusion;
	pFeedback->irisPupilRatio = m_pEyeSorting->m_Feedback->irisPupilRatio;

	return *pFeedback;
}


void EyeSortingWrap::SetQualRatioMax(float g)
{
	m_pEyeSorting->SetQualRatioMax(g);
}
void EyeSortingWrap::SetQualThreshScore (float g)
{
	m_pEyeSorting->SetQualThreshScore(g);
}
void EyeSortingWrap::EnableQualityBasedRejection(bool bEnable)
{
	m_pEyeSorting->EnableQualityBasedRejection(bEnable);
}
void EyeSortingWrap:: SetSpoofParams(bool enableSpoof, float threshX, float threshY, int SpoofPairDepth)
{
	m_pEyeSorting->SetSpoofParams( enableSpoof,  threshX, threshY, SpoofPairDepth);

}
void EyeSortingWrap::SetPupilSearchAngles(float minAngle, float maxAngle)
{
	m_pEyeSegmentationInterface->SetPupilSearchAngles(minAngle,maxAngle);
}

bool EyeSortingWrap::SetEyeLocationSearchArea(int xo, int yo, int w, int h)
{
	if(xo >=0 && yo >= 0 && (xo + w <= 640) && (yo + h <=480) && w >= 40 && h >= 40)
	{
		m_pEyeSegmentationInterface->SetEyeLocationSearchArea(xo, yo, w, h);
		return true;
	}
	return false;
}

bool EyeSortingWrap::SetIrisRadiusSearchRange(int min, int max)
{
	if(min < max && min >= 50 && max <= 175)
	{
		m_pEyeSegmentationInterface->SetIrisRadiusSearchRange(min, max);
		return true;
	}

	return false;
}
bool EyeSortingWrap::SetPupilRadiusSearchRange(int min, int max)
{
	if(min < max && min >=10 && max <= 100)
	{
		m_pEyeSegmentationInterface->SetPupilRadiusSearchRange(min, max);
		return true;
	}

	return false;
}

void EyeSortingWrap::SetSpecularityMaskLevel(int level)
{  
	m_pEyeSegmentationInterface->SetSpecularityMaskLevel(level);

}
void EyeSortingWrap::SetIrisDiameterThresholds(int minIrisDiameter, int maxIrisDiameter)
{
	m_pEyeSorting->SetIrisDiameterThresholds(minIrisDiameter, maxIrisDiameter);
}
void EyeSortingWrap::SetHaloScoreTopPoints(int noOfPixelsToConsider, float topPixelsPercentage, int intensityThreshBP, int HaloThresh )
{   
	m_pEyeSorting ->SetHaloScoreTopPoints(noOfPixelsToConsider,topPixelsPercentage,intensityThreshBP,HaloThresh);
}
void EyeSortingWrap:: SetEyeQualityClassifierWeights(float haloRankWeight, float fvRankWeight, float cbMaskRankWeight)
 {
	 m_pEyeSorting->SetEyeQualityClassifierWeights(haloRankWeight,fvRankWeight,cbMaskRankWeight );
  }
void EyeSortingWrap::SetOldHaloRankWeight(float oldHaloRankWeight)
 {
	 m_pEyeSorting->SetOldHaloRankWeight(oldHaloRankWeight);
 }
void EyeSortingWrap::BeginSorting(long time_ms)
{
	m_pEyeSorting->Begin((unsigned long)time_ms);

	if(m_UserIris.first) {delete m_UserIris.first; m_UserIris.first = (IrisCodeMask *)0;}
	if(m_UserIris.second) {delete m_UserIris.second; m_UserIris.second = (IrisCodeMask *)0;}
}

void EyeSortingWrap::SetEyeSortingLogEnable(bool EnableLog)
{
	m_LogEnable = EnableLog;
	m_pEyeSorting->SetEyeSortingLogEnable(EnableLog);
}

std::pair<unsigned char*,unsigned char*> EyeSortingWrap::GetBestSortedPairOfEye(void){
	std::pair<unsigned char*,unsigned char*> ret;
	ret.first = NULL;
	ret.second = NULL;
	std::pair<Iris *, Iris *> result=m_pEyeSorting->m_BestPairOfEyes;
	if(result.first != NULL){
		ret.first = (unsigned char*)result.first->GetImage();
	}
	if(result.second != NULL){
		ret.second = (unsigned char*)result.second->GetImage();
	}
	return ret;
}


std::pair<IrisCodeMask *, IrisCodeMask *> EyeSortingWrap::GetSortedPairEyes(void)
{
	 
	 std::pair<Iris *, Iris *> result=m_pEyeSorting->m_BestPairOfEyes;
	 int FeatureLength = m_pEyeSegmentationInterface->GetFeatureLength() ;
	 if(result.first)
	 { 
		
		 if(m_UserIris.first) {delete m_UserIris.first; m_UserIris.first= (IrisCodeMask *)0;}
         m_UserIris.first = new IrisCodeMask(FeatureLength); 
		 //m_UserIris.first->IrisCode = new unsigned char[FeatureLength] ;
		 //m_UserIris.first->IrisMask = new unsigned char[FeatureLength] ;
		 //m_UserIris.first->m_allocate= true;
//		 m_UserIris.first->Diameter= (1-(float)(abs(result.first->GetDiameter()-220)/(float)220))*100;
//		 m_UserIris.first->ImageQuality=100-((result.first->GetCorruptBit_Mask()/(float)(FeatureLength<<3))*100);
//		 m_UserIris.first->Focus = 220- result.first->GetOldHaloScore();
         int imgQty =m_imgQltyBenchMark- result.first->GetOldHaloScore();
         m_UserIris.first->ImageQuality = imgQty<0?0:(imgQty>100?100:imgQty);
         m_UserIris.first->Focus= (1-(float)(abs(result.first->GetDiameter()-m_focusBenchMark)/(float)m_focusBenchMark))*100;
         m_UserIris.first->Visibility=100-((result.first->GetCorruptBit_Mask()/(float)(FeatureLength<<3))*100);
         m_UserIris.first->FeatureLength = FeatureLength;
		 memcpy(m_UserIris.first->IrisCode, result.first->GetCode(), FeatureLength);
		 memcpy(m_UserIris.first->IrisMask, result.first->GetMask(), FeatureLength);
		 // if(m_LogEnable)
		 // {
              // CvSize size = cvSize(result.first->GetWidth(), result.first->GetHeight()); 
              // int depth  = IPL_DEPTH_8U; 
              // int channels = 1; 
              // IplImage *frame =  cvCreateImageHeader(size, depth,channels); 
			  // frame->imageData=(char *)result.first->GetImage();
			  // cvNamedWindow("FirstEye");
              // cvShowImage("FirstEye", frame);
			  // cvWaitKey(0);
              // cvReleaseImage(&frame);
		 // }
	 }
	 if(result.second)
	 {  if(m_UserIris.second) {delete m_UserIris.second;m_UserIris.second = (IrisCodeMask *)0; }
	    m_UserIris.second = new IrisCodeMask(FeatureLength); 
		//m_UserIris.second->IrisCode = new unsigned char[FeatureLength] ;
		//m_UserIris.second->IrisMask = new unsigned char[FeatureLength] ;
		//m_UserIris.second->m_allocate= true;
//		m_UserIris.second->Diameter= (1-(abs(result.second->GetDiameter()-220)/220))*100;
//		m_UserIris.second->ImageQuality=100-((result.second->GetCorruptBit_Mask()/(float)(FeatureLength<<3))*100);
//		m_UserIris.second->Focus = 220- result.second->GetOldHaloScore();
        int imgQty =m_imgQltyBenchMark- result.second->GetOldHaloScore();
        m_UserIris.second->ImageQuality =imgQty<0?0:(imgQty>100?100:imgQty);
        m_UserIris.second->Focus= (1-(abs(result.second->GetDiameter()-m_focusBenchMark)/m_focusBenchMark))*100;
        m_UserIris.second->Visibility=100-((result.second->GetCorruptBit_Mask()/(float)(FeatureLength<<3))*100);
         
		m_UserIris.second->FeatureLength = FeatureLength;
		memcpy(m_UserIris.second->IrisCode, result.second->GetCode(), FeatureLength);
		memcpy(m_UserIris.second->IrisMask, result.second->GetMask(), FeatureLength);
		// if(m_LogEnable)
		 // {
			 // CvSize size = cvSize(result.second->GetWidth(), result.first->GetHeight()); 
              // int depth  = IPL_DEPTH_8U; 
              // int channels = 1; 
              // IplImage *frame =  cvCreateImageHeader(size, depth,channels); 
			  // frame->imageData=(char *)result.second->GetImage();
			  // cvNamedWindow("SecondEye");
              // cvShowImage("SecondEye", frame);
			  // cvWaitKey(0);
              // cvReleaseImage(&frame);
		 // }
	 } 
	 return m_UserIris;
}


// DMO - IrisDetail is a new class defined alongside class Iris to provide additional information to clients...
// A simple cast is all that is required...  this avoids naming convention ambiguity.  See the header...
std::pair<IrisDetail *, IrisDetail *> EyeSortingWrap::GetSortedPairEyesDetail(void)
{
	 std::pair<IrisDetail *, IrisDetail *> result;
	 
	 result.first = (IrisDetail *)m_pEyeSorting->m_BestPairOfEyes.first;
	 result.second = (IrisDetail *)m_pEyeSorting->m_BestPairOfEyes.second;

	 return result;
}

#ifdef EYE_SIDE
bool EyeSortingWrap::GetBestPairOfEyes(unsigned char *image, int id, int cameraId, int frameId, int imageId, int numberOfImages, float scale, int x, int y, int width, int height, int score, int maxValue, long time, float haloScore, int side, float laplacianScore)
{
	 bool ret = true;
	 bool firstUpdated;
     bool secondUpdated;
	 std::pair<Iris *, Iris *> result((Iris *)0, (Iris *)0);
     Iris *piris = NULL;
     if(m_deviceType==MYRIS)
    	 ret=  m_pEyeSorting->GetBestPairOfEyes(image,id, cameraId,frameId,imageId,numberOfImages,1,x,y, width,height,score,maxValue,0,240,(unsigned long)time,result,piris,firstUpdated,secondUpdated,-3000,true, side, laplacianScore);
     else
    	 ret=  m_pEyeSorting->GetBestPairOfEyes(image,id, cameraId,frameId,imageId,numberOfImages,1,x,y, width,height,score,maxValue,0,240,(unsigned long)time,result,piris,firstUpdated,secondUpdated,haloScore,true, side, laplacianScore);
   return ret;
}
#else
bool EyeSortingWrap::GetBestPairOfEyes(unsigned char *image, int id, int cameraId, int frameId, int imageId, int numberOfImages, float scale, int x, int y, int width, int height, int score, int maxValue, long time, float haloScore, float laplacianScore)
{
	 bool ret = true;
	 bool firstUpdated;
     bool secondUpdated;
	 std::pair<Iris *, Iris *> result((Iris *)0, (Iris *)0);
     Iris *piris = NULL;
     if(m_deviceType==MYRIS)
    	 ret=  m_pEyeSorting->GetBestPairOfEyes(image,id, cameraId,frameId,imageId,numberOfImages,1,x,y, width,height,score,maxValue,0,240,(unsigned long)time,result,piris,firstUpdated,secondUpdated,-3000,true, 0, laplacianScore);
     else
    	 ret=  m_pEyeSorting->GetBestPairOfEyes(image,id, cameraId,frameId,imageId,numberOfImages,1,x,y, width,height,score,maxValue,0,240,(unsigned long)time,result,piris,firstUpdated,secondUpdated,haloScore,true, 0, laplacianScore);
   return ret;
}
#endif

bool EyeSortingWrap::GetBestPairOfEyes(unsigned char *image, uint64_t time, std::pair<Iris *, Iris *> &output, Iris *&iris)
{
   return m_pEyeSorting->GetBestPairOfEyes(image, time, output, iris);
}


std::vector< std::vector< ::Iris *> * >& EyeSortingWrap::GetRankedEyeClusters()
{
   return m_pEyeSorting->GetRankedEyeClusters();
}


bool EyeSortingWrap::ResizeFrame(unsigned char *input, int inwidth, int inheight, int instride, unsigned char *output, int outwidth, int outheight)
{
	return m_pEyeSorting ->ResizeFrame(input,inwidth,inheight,instride,output,outwidth,outheight);
}
bool EyeSortingWrap::ScaleFrame(unsigned char *input, unsigned char *output, int width, int height, int widthStep, float ratio)
{
	return m_pEyeSorting->ScaleFrame(input,output,width,height,widthStep,ratio);
}

EyeSortingWrap::~EyeSortingWrap()
{
	if (m_pEyeSegmentationInterface!= NULL)
	{
		m_pEyeSegmentationInterface->term();
		#ifdef __WIN32
		OutputDebugStringA(" delete m_pEyeSegmentationInterface");
		#endif
		delete m_pEyeSegmentationInterface;
	}
	if (m_pIrisMatchInterface!=NULL)
	{   
		#ifdef __WIN32
		OutputDebugStringA("delete m_pIrisMatchInterface");
		#endif
		m_pIrisMatchInterface->term();
		#ifdef __WIN32
		OutputDebugStringA("delete m_pIrisMatchInterface");
		#endif
		delete m_pIrisMatchInterface;
	}
	if(m_pEyeSorting!= NULL)
	{
	    m_pEyeSorting->Clear();
		#ifdef __WIN32
		OutputDebugStringA("clear m_pEyeSorting");
	    #endif	    
	    delete m_pEyeSorting; 
	}

	if(m_UserIris.first) {delete m_UserIris.first; m_UserIris.first= (IrisCodeMask *)0;}
	if(m_UserIris.second) {delete m_UserIris.second; m_UserIris.second= (IrisCodeMask *)0;}
}

void EyeSortingWrap::clearAllEyes()
{
	m_pEyeSorting->Clear();
}

EyeSegmentationInterface * EyeSortingWrap::GetEyeSegmentationInterface()
{
	return m_pEyeSegmentationInterface;
}
#endif
