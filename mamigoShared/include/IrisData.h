#ifndef IRISDATA_H_
#define IRISDATA_H_
#include "MessageExt.h"
#if 0
#include <cxtypes.h>
#else
#include <opencv/cv.h>
#include <opencv/cxcore.h>
#include <opencv2/core/types_c.h>
#endif
#include <string>

#define  ALIGN_BYTES 4

extern "C" {
#include "file_manip.h"
}


class IrisData: public Copyable {
public:
	IrisData(){
		m_extraFeaure = NULL;
		m_iris = new unsigned char[2560];
		memset(m_iris,0, 2560);
		m_ip.x=m_ip.y=m_ip.z = 0;
		m_pp.x=m_pp.y=m_pp.z = 0;
		m_illuminatorState = m_imageIndex = m_eyeIndex=0;
		m_specCentroid.x = m_specCentroid.y = 0;
		m_ts = 0;
		m_enqueTs = 0;
		m_segmentation = 0;
		m_radiusAndIrisCheck = 1;
		m_prevIndex= -1;
		m_areaScore= -1.0;
		m_focusScore = -1.0;
		m_halo= -1.0;
		m_blc = -1.0;
		m_cameraIndex=0;
	}

	IrisData(const IrisData& data)// IrisData a(b);
	{
		m_iris = new unsigned char[2560];
		*this = data;
		CopyFrom(data);
	}
	virtual void CopyFrom(const Copyable& other) {CopyFrom((const IrisData&) other);}

	void CopyFrom(const IrisData & data){
		m_specCentroid = data.m_specCentroid;
		m_ip = data.m_ip;
		m_pp = data.m_pp;
		m_areaScore = data.m_areaScore;
		m_focusScore = data.m_focusScore;
		m_halo = data.m_halo;
		m_blc = data.m_blc;

		m_illuminatorState = data.m_illuminatorState;
		m_imageIndex = data.m_imageIndex;
		m_eyeIndex = data.m_eyeIndex;
		m_cameraIndex=data.m_cameraIndex;
		m_camID = data.m_camID;
		m_prevIndex = data.m_prevIndex;
		memcpy(m_iris, data.m_iris, 2560);
		m_ts = data.m_ts;
		m_enqueTs = data.m_enqueTs;
		m_segmentation = data.m_segmentation;
		m_radiusAndIrisCheck = data.m_radiusAndIrisCheck;
		m_extraFeaure = NULL;
	}
	IrisData& operator = (const IrisData& data) // a=b;
	{
		CopyFrom(data);
		return *this;
	}

	virtual ~IrisData(){
		delete m_iris;
	}

	void setCameraIndex(int CameraIdx){
		m_cameraIndex = CameraIdx;
	}

	void setCamID(char *camId){ std::string t(camId);m_camID = t;}
	void setIlluminatorState(int state){m_illuminatorState = state;}
	void setSpecCentroid(float x, float y){m_specCentroid.x = x;m_specCentroid.y = y;}
	void setIrisCircle(float x, float y, float z){m_ip.x = x;m_ip.y = y;m_ip.z = z;}
	void setPupilCircle(float x, float y, float z){m_pp.x = x;m_pp.y = y;m_pp.z = z;}
	void setAreaScore(float a){m_areaScore = a;}
	void setFocusScore(float a){m_focusScore = a;}
	void setHalo(float a){m_halo = a;}
	void setBLC(float a){m_blc = a;}
	void setIrisBuffer(unsigned char *inp){ memcpy(m_iris,inp,2560);}
	void setEyeIndex(int eyeIdx){m_eyeIndex = eyeIdx;}
	void setFrameIndex(int imgIdx){m_imageIndex = imgIdx;}
	void setPrevIndex(int previndx){m_prevIndex= previndx;}
	void setTimeStamp(uint64_t ts){m_ts = ts;}
	void setEnqueTimeStamp(uint64_t ts){m_enqueTs = ts;}

	void setSegmentation(bool seg){ m_segmentation = seg?1:0;}
	void setIrisRadiusCheck(bool seg){ m_radiusAndIrisCheck = seg?1:0;}
		int getSizeofIrisData(){
		int sz = 2560 + sizeof(uint64_t) + sizeof(CvPoint3D32f)*2 + sizeof(CvPoint2D32f) + sizeof(float)*4 + sizeof(int)*6 +  strlen(m_camID.c_str()) + 1;
		return sz;
	}
	CvPoint2D32f getSpecCentroid(){ return m_specCentroid;}
	CvPoint3D32f getIrisCircle(){ return m_ip;}
	CvPoint3D32f getPupilCircle(){ return m_pp;}
	float getAreaScore(){return m_areaScore;}
	float getFocusScore(){return m_focusScore;}
	float getHalo(){return m_halo;}
	float getBLC(){return m_blc;}


	int getIlluminatorState(){ return m_illuminatorState;}
	int getFrameIndex(){ return m_imageIndex;}
	int getCameraIndex(){ return m_cameraIndex;}
	int getEyeIndex(){return m_eyeIndex;}
	int getPrevIndex(){return m_prevIndex;}
	unsigned char* getIris(){ return m_iris;}
	const char* getCamID(){ return m_camID.c_str();}

	uint64_t getTimeStamp(){return m_ts;}
	uint64_t getEnqueTimeStamp(){return m_enqueTs;}
	bool getSegmentation(){ return m_segmentation?true:false;}
	bool getIrisRadiusCheck(){ return m_radiusAndIrisCheck?true:false;}
	void PrintIRISData(bool logging=false){
//		printf("%s;%08d;%02d;%02d;%02d;%02d;",m_camID.c_str(),m_imageIndex,m_eyeIndex,m_prevIndex,m_illuminatorState,m_segmentation);
//		printf("%8.3f;%8.3f;%8.3f;%8.3f;%8.3f;%8.3f;%8.3f;%8.3f;\n",m_specCentroid.x,m_specCentroid.y,m_ip.x,m_ip.y,m_ip.z,m_pp.x,m_pp.y,m_pp.z);
		if(logging){
			struct timeval m_timer;
			gettimeofday(&m_timer, 0);
			TV_AS_USEC(m_timer,a);

			FILE *fp = fopen("dump.txt","a");
			fprintf(fp,"IRISDATA;%llu;%s;%05d;%02d;%02d;%02d;%02d;%6.2f;%6.2f;%6.2f;%6.2f;%6.2f;%6.2f;%6.2f;%6.2f;\n",a,m_camID.c_str(),m_imageIndex,m_eyeIndex,m_prevIndex,m_illuminatorState,m_segmentation,
					m_specCentroid.x,m_specCentroid.y,m_ip.x,m_ip.y,m_ip.z,m_pp.x,m_pp.y,m_pp.z);
			fclose(fp);
		}
	}
	void CopyFrom(const IrisData* data){
			m_specCentroid = data->m_specCentroid;
			m_ip = data->m_ip;
			m_pp = data->m_pp;
			m_areaScore = data->m_areaScore;
			m_focusScore = data->m_focusScore;
			m_halo = data->m_halo;
			m_blc = data->m_blc;

			m_illuminatorState = data->m_illuminatorState;
			m_imageIndex = data->m_imageIndex;
			m_eyeIndex = data->m_eyeIndex;
			m_prevIndex = data->m_prevIndex;
			m_cameraIndex=data->m_cameraIndex;
			//m_camID = data->m_camID;
			setCamID((char*)data->m_camID.c_str());
			memcpy(m_iris, data->m_iris, 2560);
			m_ts = data->m_ts;
			m_segmentation = data->m_segmentation;
			m_radiusAndIrisCheck = data->m_radiusAndIrisCheck;
			m_extraFeaure = data->m_extraFeaure;
			m_enqueTs = data->m_enqueTs;
		}
private:
	CvPoint2D32f m_specCentroid;
	CvPoint3D32f m_ip;
	CvPoint3D32f m_pp;
	float m_areaScore,m_focusScore,m_halo,m_blc;
	int m_illuminatorState;
	int m_imageIndex,m_eyeIndex,m_prevIndex;
	int m_cameraIndex;
	int m_segmentation;
	int m_radiusAndIrisCheck;
	uint64_t m_ts;
	uint64_t m_enqueTs;
	unsigned char* m_iris;
	void *m_extraFeaure;
	std::string m_camID;
};

#endif //IRISDATA_H_
