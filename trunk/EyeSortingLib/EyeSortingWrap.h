#include <string>
#include <opencv/highgui.h>
class EyeSegmentationInterface;
class IrisMatchInterface;
class EnrollmentServer;
//class Iris;
//class IrisDetail;
#include "IrisSelector.h"

#define EYE_SIDE

enum DeviceType{
	Nano=0,
	MYRIS=1
};


struct EyeSortingWrapperFeedback
{
	bool softOcclusion;
	bool hardOcclusion;
	bool calibration;
	bool irisPupilRatio;
};

// DMO - Total HACK ALERT.  The Iris() class implmented within this library is created within the Global namespace
// There is ANOTHER Iris() class defined in MyrisDeviceNI_types.h.  So any client wanting to use the Iris() class
// from this library has to deal with ambiguous class errors.  To ease that pain, I've created the simple derived
// class below.  It's sole purpose is to wrap the underlying Iris() class changing its name so that clients can
// use it without encountering the naming conflict described above.
class IrisDetail : public Iris
{
public:
	IrisDetail(unsigned char *pData, unsigned char *irisCode) : Iris(pData, irisCode){};
	IrisDetail() : Iris(){};
	IrisDetail(const Iris & rhsIris) : Iris(rhsIris){};
};


class IrisCodeMask
{
private:
		void Clear()
		{
			if(IrisCode)delete []IrisCode;
			if(IrisMask)delete []IrisMask;
		}
public :
	IrisCodeMask(int featureLength)
		: FeatureLength(featureLength)
		, IrisCode(NULL)
		, IrisMask(NULL)
		, Diameter(0)
		, ImageQuality(0)
		, Focus(0)
        , Visibility(0)
	{
		IrisCode = new unsigned char[FeatureLength] ;
		IrisMask = new unsigned char[FeatureLength] ;
	}
	IrisCodeMask(const IrisCodeMask& ref)
	{
		FeatureLength = ref.FeatureLength;
		IrisCode = new unsigned char[FeatureLength] ;
		IrisMask = new unsigned char[FeatureLength] ;

		memcpy(IrisCode,ref.IrisCode,ref.FeatureLength);
		memcpy(IrisMask,ref.IrisMask,ref.FeatureLength);
	}

	const IrisCodeMask& operator=(const IrisCodeMask& ref)
	{
		if(&ref == this)
			return *this;

		if(FeatureLength > 0)
		{
			Clear();
		}

		FeatureLength = ref.FeatureLength;
		IrisCode = new unsigned char[FeatureLength] ;
		IrisMask = new unsigned char[FeatureLength] ;

		memcpy(IrisCode,ref.IrisCode,ref.FeatureLength);
		memcpy(IrisMask,ref.IrisMask,ref.FeatureLength);

		return *this;
	}

	unsigned char * IrisCode;
	unsigned char * IrisMask;
	int  Diameter;
	int ImageQuality;
	int Focus;
	int FeatureLength;
    int Visibility;
	//bool m_allocate;
	~IrisCodeMask()
	{
		Clear();
/*
		if(m_allocate)
		{
		  delete []IrisCode;
		  delete []IrisMask;
		}
*/
	}



};
class EyeSortingWrap
{
    private:
        int m_imgQltyBenchMark,m_focusBenchMark;
    public:
        
		int m_LocalTimeout ;
	    int m_GlobalTimeout ;
		bool m_LogEnable;
		std::pair<IrisCodeMask *, IrisCodeMask *> m_UserIris ;
		double GetLaplacianScore(unsigned char *frame, int Width, int Height);
        void SetQualRatioMax(float g); 
		void SetQualThreshScore (float g); 
		void EnableQualityBasedRejection(bool bEnable);
        void SetImgQltyBenchMark(int);
        void SetFocusBenchMark(int);
		void SetSpoofParams(bool enableSpoof, float threshX, float threshY, int SpoofPairDepth);
		void SetPupilSearchAngles(float minAngle, float maxAngle);
        void SetSpecularityMaskLevel(int level);
		void SetHaloScoreTopPoints(int noOfPixelsToConsider, float topPixelsPercentage, int intensityThreshBP, int HaloThresh );
		void SetEyeQualityClassifierWeights(float haloRankWeight, float fvRankWeight, float cbMaskRankWeight);
		void SetOldHaloRankWeight(float oldHaloRankWeight);
		void SetIrisDiameterThresholds(int minIrisDiameter, int maxIrisDiameter);
		void SetMinEyesInCluster(int minEyesInCluster);
	    void SetEyeSortingLogEnable(bool EnableLog);
	    void SetDeviceType(DeviceType deviceType){m_deviceType = deviceType;}
		void BeginSorting(long time_ms);
	    std::pair<IrisCodeMask *, IrisCodeMask *> GetSortedPairEyes(void);
		std::pair<IrisDetail *, IrisDetail *> GetSortedPairEyesDetail(void);
	    std::pair<unsigned char*,unsigned char*> GetBestSortedPairOfEye(void);
		//bool GetBestPairOfEyes(std::pair<Iris *, Iris *> &output,long time_cur);//eyelock::videoFrame Video_frame,std::pair<Iris *, Iris *> &output,long time_cur);
#ifdef EYE_SIDE
	    bool GetBestPairOfEyes(unsigned char *image, int id, int cameraId, int frameId, int imageId, int numberOfImages, float scale, int x, int y, int width, int height, int score, int maxValue, long time, float haloScore, int side, float laplacianScore = 0.0f);
#else
		bool GetBestPairOfEyes(unsigned char *image, int id, int cameraId, int frameId, int imageId, int numberOfImages, float scale, int x, int y, int width, int height, int score, int maxValue, long time, float haloScore, float laplacianScore = 0.0f);
#endif
		bool GetBestPairOfEyes(unsigned char *image, uint64_t time, std::pair< ::Iris *, ::Iris *> &output, ::Iris *&iris);
		std::vector< std::vector< ::Iris *> * >& GetRankedEyeClusters();
	    EyeSortingWrap(int FeatureScale, int maxSpecularityValue, int EarlyTimeoutMS, int GlobalTimeoutMS); 
		bool ResizeFrame(unsigned char *input, int inwidth, int inheight, int instride, unsigned char *output, int outwidth, int outheight);
        bool ScaleFrame(unsigned char *input, unsigned char *output, int width, int height, int widthStep, float ratio);
		EyeSortingWrapperFeedback GetEyeSortingFeedback();
		void clearAllEyes();
        ~EyeSortingWrap();
        EyeSegmentationInterface * GetEyeSegmentationInterface();
        private:
        EyeSegmentationInterface *m_pEyeSegmentationInterface;
		IrisMatchInterface *m_pIrisMatchInterface;
		EnrollmentServer *m_pEyeSorting;
		DeviceType m_deviceType;
};

