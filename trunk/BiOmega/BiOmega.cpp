#include "EyeSegmentationInterface.h"
#include "BiOmega.h"
#include "IrisSelectServer.h"


extern "C" {
	#include "test_fw.h"
	#include "file_manip.h"
}

char *BiOmega::m_softwareVersion = "BiOmega 1.41 Build 3480";

BiOmega::BiOmega(int w, int h, int scale): m_pEyeSegmentInterface(new EyeSegmentationInterface()), m_pIrisMatchInterface(0){
	if(m_pEyeSegmentInterface)
	{
		m_pEyeSegmentInterface->init(scale,w,h);
//22 Jun2011		m_pEyeSegmentInterface->EnableEyelidSegmentation(true);
		// modifying the default to accommodate the huge eye displacements in the ICE database
		m_pEyeSegmentInterface->SetEyeLocationSearchArea(240, 180, 160, 120);

		m_pIrisMatchInterface = new IrisMatchInterface(
			m_pEyeSegmentInterface->GetFeatureLength(),
			m_pEyeSegmentInterface->GetFeatureNumRows(),
			m_pEyeSegmentInterface->GetFeatureByteSize());

		if(m_pIrisMatchInterface)	m_pIrisMatchInterface->init();
	}
}

void BiOmega::SetDoRawScore(bool raw)
{
	if(m_pIrisMatchInterface)
		m_pIrisMatchInterface->SetDoRawScore(raw);
}

void BiOmega::SetNominalCommonBits(int bits)
{
	if(m_pIrisMatchInterface)
		m_pIrisMatchInterface->SetNominalCommonBits(bits);
}
void BiOmega::SetMinCommonBits(int commonBits)
{
	if(m_pIrisMatchInterface)
		m_pIrisMatchInterface->SetMinCommonBits(commonBits);
}

void BiOmega::SetEnableEyelidSegmentation(bool val){
	if(m_pEyeSegmentInterface)
		m_pEyeSegmentInterface->EnableEyelidSegmentation(val);
}

void BiOmega::SetUpperEyelidCenterandRadius(CvPoint cenPt,float rad ){
	if(m_pEyeSegmentInterface)
		m_pEyeSegmentInterface->SetUpperEyelidCenterandRadius(cenPt,rad);
}

void BiOmega::SetLowerEyelidCenterandRadius(CvPoint cenPt,float rad ){
	if(m_pEyeSegmentInterface)
		m_pEyeSegmentInterface->SetLowerEyelidCenterandRadius(cenPt,rad);
}

void BiOmega::SetLUT(int pupilmin5,int pupilmax64,int cirmin5,int cirmax255){
	if(m_pIrisMatchInterface) m_pEyeSegmentInterface->SetLUT(pupilmin5, pupilmax64,cirmin5,cirmax255);
}

int BiOmega::GetFeatureLength() const
{
	return m_pEyeSegmentInterface->GetFeatureLength()*2;
}
BiOmega::~BiOmega()
{
	if(m_pEyeSegmentInterface)
	{
		m_pEyeSegmentInterface->term();
		delete m_pEyeSegmentInterface; m_pEyeSegmentInterface = 0;
	}

	if(m_pIrisMatchInterface)
	{
		m_pIrisMatchInterface->term();
		delete m_pIrisMatchInterface; m_pIrisMatchInterface = 0;
	}
}
const char *BiOmega::GetVersion() const
{
	return m_softwareVersion;
}

void BiOmega::GetEyeLocationSearchArea(int& xo, int& yo, int& w, int& h)
{
	m_pEyeSegmentInterface->GetEyeLocationSearchArea(xo,yo,w,h);
}


bool BiOmega::SetEyeLocationSearchArea(int xo, int yo, int w, int h)
{
	if(xo >=0 && yo >= 0 && (xo + w <= 640) && (yo + h <=480) && w >= 40 && h >= 40)
	{
		m_pEyeSegmentInterface->SetEyeLocationSearchArea(xo, yo, w, h);
		return true;
	}
	return false;
}

bool BiOmega::SetIrisRadiusSearchRange(int min, int max)
{
	if(min < max && min >= 50 && max <= 175)
	{
		m_pEyeSegmentInterface->SetIrisRadiusSearchRange(min, max);
		return true;
	}

	return false;
}
bool BiOmega::SetPupilRadiusSearchRange(int min, int max)
{
	if(min < max && min >=10 && max <= 100)
	{
		m_pEyeSegmentInterface->SetPupilRadiusSearchRange(min, max);
		return true;
	}

	return false;
}

bool BiOmega::SetPupilAngleSearchRange(int min, int max)
{
	if(min < max)
	{
		m_pEyeSegmentInterface->SetPupilAngleSearchRange(min, max);
		return true;
	}

	return false;
}
bool BiOmega::GetPupilAngleSearchRange(float& min, float& max)
{
	m_pEyeSegmentInterface->GetPupilAngleSearchRange(min, max);
		return true;
}

bool BiOmega::GetIrisCode(unsigned char *imageBuffer, int w, int h, int stride, char *irisCode,IrisPupilCircles *pCircles, float *robustFetaureVariance )
{

	// printf("entering BiOmega::GetIrisCode\n");
	bool rc=m_pEyeSegmentInterface->GetIrisCode(imageBuffer, w, h, stride, (unsigned char *) irisCode, (unsigned char *) irisCode+m_pEyeSegmentInterface->GetFeatureLength(),pCircles);

	bool status;

	// If Bad Segementation fix
	if(rc){



		if(robustFetaureVariance)
//		if (status)
		{
			// printf("Before GetRobustFeatureVariances\n");

			//m_pEyeSegmentInterface->GetRobustFeatureVariances(robustFetaureVariance);
			// printf("After GetRobustFeatureVariances\n");
		}
	}else{
		printf("Bad Segmentation\n");
	}
	return rc;

}
bool BiOmega::GetDefaultMaskCode(unsigned char *irisCode)
{
	return m_pEyeSegmentInterface->GetDefaultMaskCode(irisCode, irisCode + m_pEyeSegmentInterface->GetFeatureLength());
}
int BiOmega::GetMaxCorruptBitsPercAllowed()
{
	return m_pEyeSegmentInterface->GetMaxCorruptBitsPercAllowed();
}
void BiOmega::SetMaxCorruptBitsPercAllowed(int perc)
{
	m_pEyeSegmentInterface->SetMaxCorruptBitsPercAllowed(perc);
}

float BiOmega::GetCorruptBitsPerc(){
	return m_pEyeSegmentInterface->GetCorruptBitsPerc();
}


std::pair<int, float> BiOmega::MatchIris(unsigned char *imageBuffer, int w, int h, int stride, char *irisCodeDatabase, int numberOfEyes,IrisPupilCircles *pCircles,CvPoint2D32f *pRefCentre,CvPoint2D32f *pVar,int specrad)
{
	char *l_refIrisCode = (char *) malloc(GetFeatureLength());
	XTIME_OP("GetIrisCode",
			bool status = GetIrisCode(imageBuffer, w, h, stride, l_refIrisCode,pCircles)
	);
	std::pair<int, float> best_score = std::make_pair(-1, 1);

	unsigned char *refIrisCode = (unsigned char *) l_refIrisCode;
	unsigned char *refMaskCode = (unsigned char *) refIrisCode + GetFeatureLength()/2;

	for(int i=0;i<numberOfEyes;i++)
	{
		unsigned char *insIrisCode = (unsigned char *) irisCodeDatabase + i*(GetFeatureLength());
		unsigned char *insMaskCode = insIrisCode + GetFeatureLength()/2;

		std::pair<int, float> score = m_pIrisMatchInterface->match_pair(refIrisCode, refMaskCode, insIrisCode, insMaskCode);

		if(score.second < best_score.second)
		{
			best_score = score;
			best_score.first = i;
		}
	}

	//If not NULL then only compute refined centre for Spoof
	if(pRefCentre)
	{
		CvPoint2D32f RefCentre;
		CvPoint2D32f Var;
		IplImage **pyr = m_pEyeSegmentInterface->GetImagePyramid();
	    IrisSelectServer::ComputeSpecularityMetrics(pyr[1],RefCentre,Var,specrad);
	    //Make the co-ordinaates to level 0 i.e, rescale it by 2
	    RefCentre.x = RefCentre.x*2;
	    RefCentre.y = RefCentre.y*2;
	    *pRefCentre = RefCentre;
	    if(pVar){
	    	*pVar = Var;
	    }
	}
	free(l_refIrisCode);

	return best_score;
}


std::pair<int, float> BiOmega::MatchIrisCode(char * IrisCode ,char *irisCodeDatabase, int numberOfEyes){
	std::pair<int, float> best_score = std::make_pair(-1, 1);

	unsigned char *refIrisCode = (unsigned char *) IrisCode;
	unsigned char *refMaskCode = (unsigned char *) refIrisCode + GetFeatureLength()/2;

	for(int i=0;i<numberOfEyes;i++)
	{
		unsigned char *insIrisCode = (unsigned char *) irisCodeDatabase + i*(GetFeatureLength());
		unsigned char *insMaskCode = insIrisCode + GetFeatureLength()/2;

		std::pair<int, float> score = m_pIrisMatchInterface->match_pair(refIrisCode, refMaskCode, insIrisCode, insMaskCode);

		if(score.second < best_score.second)
		{
			best_score = score;
			best_score.first = i;
		}
	}
	return best_score;
}

std::pair<int, float> BiOmega::MatchIrisCodeSingle(char * IrisCode ,char *irisCodeDatabase,unsigned int maskval){

	unsigned char *refIrisCode = (unsigned char *) IrisCode;
	unsigned char *refMaskCode = (unsigned char *) refIrisCode + GetFeatureLength()/2;

	unsigned char *insIrisCode = (unsigned char *) irisCodeDatabase ;
	unsigned char *insMaskCode = insIrisCode + GetFeatureLength()/2;

	std::pair<int, float> score = m_pIrisMatchInterface->match_pair(refIrisCode, refMaskCode, insIrisCode, insMaskCode,maskval);
	return score;
}

bool BiOmega::ProcessIris(unsigned char *imageBuffer, int w, int h, int stride,char *l_refIrisCode,IrisPupilCircles *pCircles,CvPoint2D32f *pRefCentre,CvPoint2D32f *pVar,int specrad)
{
	bool status;
	XTIME_OP("GetIris",
	status = GetIrisCode(imageBuffer, w, h, stride, l_refIrisCode,pCircles)
	);
	if(pRefCentre)
	{
		CvPoint2D32f RefCentre,Var;
		IplImage **pyr = m_pEyeSegmentInterface->GetImagePyramid();
	    IrisSelectServer::ComputeSpecularityMetrics(pyr[1],RefCentre,Var,specrad);
	    //Make the co-ordinates to level 0 i.e, rescale it by 2
	    RefCentre.x = RefCentre.x*2;
	    RefCentre.y = RefCentre.y*2;
	    *pRefCentre = RefCentre;
	    if(pVar){
	    	*pVar = Var;
	    }
	}
	return status;
}
