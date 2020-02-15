/*
 * LogImageJSON.h
 *
 *  Created on: Jan 15, 2020
 *      Author: root
 */
#ifndef INCLUDE_LOGIMAGEJSON_H_
#define INCLUDE_LOGIMAGEJSON_H_

#include <string>
#include <ctime>
#include <limits>
#include <vector>
#include <fstream>
#include <sstream>
#include <istream>
#include <ostream>
#include <streambuf>
#include <chrono>
#include <ctime>
#include <ratio>
#include <log4cxx/log4cxx.h>
#include <log4cxx/logstring.h>


// single file json library
#include <single_include/nlohmann/json.hpp>


using namespace std;
using namespace log4cxx;
using json = nlohmann::json;


typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
#ifndef __uint32_t_defined
typedef unsigned int		uint32_t;
# define __uint32_t_defined
#endif



struct LogImagePoint
{
	public:
		int m_nPointX;
		int m_nPointY;
};

//json serialization
inline void to_json(json &j, const LogImagePoint &p)
{
	j["PointX"] = p.m_nPointX;
	j["PointY"] = p.m_nPointY;
}

inline void from_json(const json &j, LogImagePoint &p)
{
	p.m_nPointX = j.at("PointX").get<int>();
	p.m_nPointY = j.at("PointY").get<int>();
}


struct LogImageRect
{
	public:
		int m_nTop;
		int m_nLeft;
		int m_nWidth;
		int m_nHeight;
};

//json serialization
inline void to_json(json &j, const LogImageRect &r)
{
	j["RectTop"] = r.m_nTop;
	j["RectLeft"] = r.m_nLeft;
	j["RectWidth"] = r.m_nWidth;
	j["RectHeight"] = r.m_nHeight;
}

inline void from_json(const json &j, LogImageRect &r)
{
	r.m_nTop = j.at("RectTop").get<int>();
	r.m_nLeft = j.at("RectLeft").get<int>();
	r.m_nWidth = j.at("RectWidth").get<int>();
	r.m_nHeight = j.at("RectHeight").get<int>();
}


//LogImageCircle
struct LogImageCircle
{
	public:
		int m_nPointX;
		int m_nPointY;
		int m_nRadius;
};

//json serialization
inline void to_json(json &j, const LogImageCircle &c)
{
	j["PointX"] = c.m_nPointX;
	j["PointY"] = c.m_nPointY;
	j["Radius"] = c.m_nRadius;
}

inline void from_json(const json &j, LogImageCircle &c)
{
	c.m_nPointX = j.at("PointX").get<int>();
	c.m_nPointY = j.at("PointY").get<int>();
	c.m_nRadius = j.at("Radius").get<int>();
}





// LogImage Record
class LogImageRecordJSON
{
public:
	typedef enum LogImageType
	{
		FACE = 0,
		IRIS_FRAME,
		IRIS_CROP
	}LOGIMAGETYPE;

	typedef enum EyeLabelType
	{
		SUBJECT_EYE_LABEL_UNDEF = 0,
		SUBJECT_EYE_LABEL_LEFT,
		SUBJECT_EYE_LABEL_RIGHT
	}EYELABELTYPE;

	typedef enum LogImageFormat
	{
		IMAGEFORMAT_MONO_RAW = 2,
		IMAGEFORMAT_MONO_JPEG2000 = 10,
		IMAGEFORMAT_MONO_PNG = 14
	}LOGIMAGEFORMAT;

	typedef enum DeviceType
	{
		EYELOCK_NXT = 0,
		EYELOCK_EXT,
	}DEVICETYPE;




#if 0
	class LogImageCircle : public serializelog::I
	{
	};

	class LogImageRect : public serializelog::I
	{
	};

#endif

	private:

	// Object Header Information
	const float m_Version = 1.0;

	long	m_lFrameUUID;
	string 	m_sDateTime;
	long	m_lTimeStampMS;
	std::chrono::high_resolution_clock::time_point 	m_CreationTime; // Difference is total processing time for this object...
	int 	m_nDeviceType; 			//Need to check (EXT, NXT, etc)
	string  m_sCaptureDeviceID;	// DeviceID of the capturing device

	// Frame/Crop/Camera specifics
	string  m_sImageKey; // frame, crop, ???
	int		m_nFrameID;  			// which Frame is this?
	int		m_nCamID;
	int		m_nFaceIndex;			// Which face image does this correspond to
	int		m_nNumOfHaarEyes;
	int		m_nNumofSpecularities;

	bool 	m_bDiscarded;
	string  m_sDiscardedReason;

	int		m_nEyeLabel;
	LogImageType		m_nImageType;
	int		m_nImageFormat;
	int		m_nImageProperties;

	int		m_nImageWidth;
	int		m_nImageHeight;

	int		m_nBitDepth;

	std::string m_sCaptureSession;

	// EyeDetection values
	float m_fSharpness;

	vector<LogImagePoint> m_arEyeDetectionPoints;
	vector<LogImageRect> m_arEyeDetectionCropRects;

	// AusSegmentation Values
	int m_irisScore;
	int m_darkScore;
	int m_specScore;
	int m_pupilScore;
	int m_validBitsInTemplate;
	float m_eyelidCoverage;
	float m_UseableIrisArea;
	int	m_gazeZ;
	float m_pupilToIrisRatio;
	int m_IrisDiameter;
	int m_PupilDiameter;
	int m_SpecDiameter;
	bool m_bSegmentationCheck;
	int m_corruptBitCountMask;
	int m_irisImagecentre_distX;
	int m_irisImagecentre_distY;
	LogImageCircle	m_Pupil;
	LogImageCircle	m_Iris;
	int m_TemplatePipelineError;


	vector<uint8_t> m_arImageData;

	vector<string> m_arCustomMetadata;


private:
	uint8_t 		*m_pMemFileData;	// Memory byte[] representation for writing to disk...
	LogString		key;

public:
	static unsigned int random_char();
	static std::string generate_hex(const unsigned int len);


	float GetVersion() { return m_Version; }

	long GetFrameUUID() { return m_lFrameUUID; }
	void SetFrameUUID(long lFrameUUID) { m_lFrameUUID = lFrameUUID; }

	string GetCreateDateTime() { return m_sDateTime; }
	void SetCreateDateTime(string sDateTime) { m_sDateTime = sDateTime; }

	long GetTimeStamp() { return m_lTimeStampMS; }
	void SetTimeStamp(long lTimeStampMS) { m_lTimeStampMS = lTimeStampMS; }

	int GetDeviceType() { return m_nDeviceType; }
	void SetDeviceType(int nDeviceType) { m_nDeviceType = nDeviceType; }

	string GetDeviceID() { return m_sCaptureDeviceID; }
	void SetDeviceID(string sDeviceID) { m_sCaptureDeviceID = sDeviceID; }

	string GetImageKey(){ return m_sImageKey; }
	void SetImageKey(string sKey) { m_sImageKey = sKey; }

	int GetFrameID() { return m_nFrameID; }
	void SetFrameID(int nFrameID) { m_nFrameID = nFrameID; }

	int GetCameraID() { return m_nCamID; }
	void SetCameraID(int nCamID) { m_nCamID = nCamID; }

	int GetFaceIndex() { return m_nFaceIndex; }
	void SetFaceIndex(int nFaceIndex) { m_nFaceIndex = nFaceIndex; }

	int GetHaarEyesCount() { return m_nNumOfHaarEyes; }
	void SetHaarEyesCount(int nNumOfHaarEyes) { m_nNumOfHaarEyes = nNumOfHaarEyes; }

	int GetSpecularityCount() { return m_nNumofSpecularities; }
	void SetSpecularityCount(int nNumofSpecularities) { m_nNumofSpecularities = nNumofSpecularities; }

	bool GetDiscarded() { return m_bDiscarded; }
	void SetDiscarded(bool bDiscarded) { m_bDiscarded = bDiscarded; }

	string GetDiscaredReason() { return m_sDiscardedReason; }
	void SetDiscardedReason(string theReason) { m_sDiscardedReason = theReason; }

	int GetEyeLabel() { return m_nEyeLabel; }
	void SetEyeLabel(int nEyeLabel) { m_nEyeLabel = nEyeLabel; }

	LogImageType GetImageType() { return m_nImageType; }
	void SetImageType(LogImageType nImageType) { m_nImageType = nImageType; }

	int GetImageFormat() { return m_nImageFormat; }
	void SetImageFormat(int nImageFormat) { m_nImageFormat = nImageFormat; }

	int GetImageProperties() { return m_nImageProperties; }
	void SetImageProperties(int nImageProperties) { m_nImageProperties = nImageProperties; }

	int GetImageWidth() { return m_nImageWidth; }
	void SetImageWidth(int nImageWidth) { m_nImageWidth = nImageWidth; }

	int GetImageHeight() { return m_nImageHeight; }
	void SetImageHeight(int nImageHeight) { m_nImageHeight = nImageHeight; }

	int GetBitDepth() { return m_nBitDepth; }
	void SetBitDepth(int nBitDepth) { m_nBitDepth = nBitDepth; }

	string GetCaptureSession() { return m_sCaptureSession; }
	void SetCaptureSession(string sCaptureSession) { if (sCaptureSession.size() > 0) m_sCaptureSession = sCaptureSession; }

	// EyeDetection values
	float GetSharpness() { return m_fSharpness; }
	void SetSharpness(float fSharpness) { m_fSharpness = fSharpness; }

	// Segmentation values
	int GetIrisScore() { return m_irisScore; }
	void SetIrisScore(int nIrisScore) { m_irisScore = nIrisScore; }

	int GetDarkScore() { return m_darkScore; }
	void SetDarkScore(int nDarkScore) { m_darkScore = nDarkScore; }

	int GetSpecScore() { return m_specScore; }
	void SetSpecScore(int nSpecScore) { m_specScore = nSpecScore; }

	int GetPupilScore() { return m_pupilScore; }
	void SetPupilScore(int nPupilScore) { m_pupilScore = nPupilScore; }

	int GetValidBitsInTemplate() { return m_validBitsInTemplate; }
	void SetValidBitsInTemplate(int nBits) { m_validBitsInTemplate = nBits; }

	float GetEyelidCoverage() { return m_eyelidCoverage; }
	void SetEyelidCoverage(float nCoverage) { m_eyelidCoverage = nCoverage; }

	float GetUseableIrisArea() { return m_UseableIrisArea; }
	void SetUseableIrisArea(float fUseableIrisArea) { m_UseableIrisArea = fUseableIrisArea; }

	int GetGazeZ() { return m_gazeZ; }
	void SetGazeZ(int nGazeZ) { m_gazeZ = nGazeZ; }

	float GetPupilToIrisRatio() { return m_pupilToIrisRatio; }
	void SetPupilToIrisRatio(float nRatio) { m_pupilToIrisRatio = nRatio; }

	int GetIrisDiameter() { return m_IrisDiameter; }
	void SetIrisDiameter(int nIrisDiameter) { m_IrisDiameter = nIrisDiameter; }

	int GetPupilDiameter() { return m_PupilDiameter; }
	void SetPupilDiameter(int nPupilDiameter) { m_PupilDiameter = nPupilDiameter; }

	int GetSpecDiameter() { return m_SpecDiameter; }
	void SetSpecDiameter(int nSpecDiameter) { m_SpecDiameter= nSpecDiameter; }

	bool GetSegmentationCheck() { return m_bSegmentationCheck; }
	void SetSegmentationCheck(bool bSegmentationCheck) { m_bSegmentationCheck = bSegmentationCheck; }

	int GetCorruptBitCountMask() { return m_corruptBitCountMask; }
	void SetCorruptBitCountMask(int ncorruptBitCountMask) { m_corruptBitCountMask = ncorruptBitCountMask; }

	int GetIrisImagecentre_distX() { return m_irisImagecentre_distX; }
	void SetIrisImagecentre_distX(int nirisImagecentre_distX) { m_irisImagecentre_distX = nirisImagecentre_distX; }

	int GetIrisImagecentre_distY() { return m_irisImagecentre_distY; }
	void SetIrisImagecentre_distY(int nirisImagecentre_distY) { m_irisImagecentre_distY = nirisImagecentre_distY; }

	int GetTemplatePipelineError() { return m_TemplatePipelineError; }
	void SetTemplatePipelineError(int nError) { m_TemplatePipelineError = nError; }

	void AddSegPupilCircle(LogImageCircle theCircle) { m_Pupil.m_nPointX = theCircle.m_nPointX;
														m_Pupil.m_nPointY = theCircle.m_nPointY;
														m_Pupil.m_nRadius = theCircle.m_nRadius; }

	void AddSegIrisCircle(LogImageCircle theCircle) { m_Iris.m_nPointX = theCircle.m_nPointX;
														m_Iris.m_nPointY = theCircle.m_nPointY;
														m_Iris.m_nRadius = theCircle.m_nRadius; }


public:
	// Get time as string in constructor...
	LogImageRecordJSON() : 	m_lFrameUUID(0L), m_sDateTime(""), m_lTimeStampMS(0L), m_nDeviceType(0),	m_sCaptureDeviceID(""), m_sImageKey(""),  m_nFrameID(0),
							m_nCamID(0), m_nFaceIndex(0), m_nNumOfHaarEyes(0), m_nNumofSpecularities(0), m_bDiscarded(false), m_sDiscardedReason(""),
							m_nEyeLabel(SUBJECT_EYE_LABEL_UNDEF), m_nImageType(FACE), m_nImageFormat(IMAGEFORMAT_MONO_RAW),
							m_nImageProperties(0), m_nImageWidth(0), m_nImageHeight(0),	m_nBitDepth(8),	m_sCaptureSession("Default"),
							m_fSharpness(0.0), m_irisScore(0), m_darkScore(0), m_specScore(0), m_pupilScore(0), m_validBitsInTemplate(0),
							m_eyelidCoverage(0.0), m_UseableIrisArea(0.0), m_gazeZ(0), m_pupilToIrisRatio(0.0), m_IrisDiameter(0), m_PupilDiameter(0),
							m_SpecDiameter(0), m_bSegmentationCheck(false), m_corruptBitCountMask(0), m_irisImagecentre_distX(0),
							m_irisImagecentre_distY(0),	m_TemplatePipelineError(0)

	{
		auto m_StartTime = std::chrono::system_clock::now();
		std::time_t theTime = std::chrono::system_clock::to_time_t(m_StartTime);
		m_sDateTime = std::ctime(&theTime);

		// Start timer for processing duration
		m_CreationTime = std::chrono::high_resolution_clock::now();

		// TimeStamp for absolute sorting
		// Store millisecond timestamp for use in viewers for timeline ordering of events...
		auto Duration = std::chrono::system_clock::now().time_since_epoch();
		m_lTimeStampMS = std::chrono::duration_cast<std::chrono::milliseconds>(Duration).count();

		// Default to this frame's timestamp...
		SetFrameUUID(m_lTimeStampMS);

		// More defaults...
		m_Iris.m_nPointX = 0;
		m_Iris.m_nPointY = 0;
		m_Iris.m_nRadius = 0;

		m_Pupil.m_nPointX = 0;
		m_Pupil.m_nPointY = 0;
		m_Pupil.m_nRadius = 0;
	};


	virtual ~LogImageRecordJSON() {};


	// Static API's for adding, modifying and removing this object from the MDC...
	// Add an allocated LogImageRecord to the MDC
	  static LogImageRecordJSON *put(const std::string& key, uint8_t *pData, uint32_t width, uint32_t height);
	  /**
	  * Get the context identified by the <code>key</code> parameter.
	  *
	  *  <p>This method has no side effects.
	  *  @param key key.
	  *  @return value for key, empty if not set.
	  * */
	  static LogImageRecordJSON *get(const std::string& key);

	  /**
	  * Remove the the context identified by the <code>key</code>
	  * parameter.
	  *  @param key key.
	  * @return value if key had been set, empty if not.
	  */
	  static void remove(const std::string& key);



	bool SetImageData(uint8_t *pData, uint32_t width, uint32_t height);
	bool Convert8bitGreyscaleToPNG(uint8_t *pData, uint32_t width, uint32_t height, vector<uint8_t>&arOutImage);

	vector<uint8_t>GetImageData() { return m_arImageData; }

	void AddEyeDetectionPoint(LogImagePoint thePoint) { m_arEyeDetectionPoints.push_back(thePoint); }
	void AddEyeDetectionCropRect(LogImageRect theRect) { m_arEyeDetectionCropRects.push_back(theRect); }
	void AddCustomMetadata(string theMetadata) { m_arCustomMetadata.push_back(theMetadata); }

	string GetObjectAsJSON();


protected:
	std::string base64(const char* data, int size);
	std::string Base64encode(unsigned char const* bytes_to_encode, unsigned int in_len);

//	void SetConf(Configuration *pConf) { m_pConf = pConf;}
//	bool ConvertFrom8bitGreyscaleToFormat(uint8_t *pData, uint32_t width, uint32_t height, ISO_EYELABELTYPE theEye, ISO_IMAGEFORMAT theFormat);
//	vector<uint8_t> ConvertFromFormatTo8bitGreyscale(vector<uint8_t>arData, uint8_t nImageFormat);
};
#define LogImageJSON LogImageRecordJSON

#endif /* LOGIMAGEJSON_H_ */





