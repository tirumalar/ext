#ifndef LOGIMAGE_H_
#define LOGIMAGE_H_

#include <string>
#include <ctime>
#include <limits>
#include <vector>
#include <fstream>
#include <sstream>
#include <istream>
#include <ostream>
#include <streambuf>
#include <log4cxx/log4cxx.h>
#include <log4cxx/logstring.h>
#include <serializelog.h>
// single file json library
#include <single_include/nlohmann/json.hpp>



using namespace std;
using namespace log4cxx;



typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
#ifndef __uint32_t_defined
typedef unsigned int		uint32_t;
# define __uint32_t_defined
#endif





// LogImage Record
class LogImageRecord : public serializelog::I
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


	//
	class LogImagePoint : public serializelog::I
	{
		public:
			LogImagePoint() : m_nPointX(0), m_nPointY(0) {};
			virtual ~LogImagePoint() {};

			uint16_t m_nPointX;
			uint16_t m_nPointY;


		public:
			static uint16_t GetSize() { return sizeof(LogImagePoint::m_nPointX)+sizeof(LogImagePoint::m_nPointY); }

			ostream& write(ostream& s)
			{
				serializelog::write(s, m_nPointX);
				serializelog::write(s, m_nPointY);
			}

			istream& read(istream& s)
			{
				serializelog::read(s, m_nPointX);
				serializelog::read(s, m_nPointY);
			}
	};

	class LogImageCircle : public serializelog::I
	{
	};

	class LogImageRect : public serializelog::I
	{
	};


	uint32_t m_nRecordLength; 		//Includes header
	const uint8_t m_Version[4] {0x00, 0x30, 0x32, 0x30};

	uint8_t m_CreationTimeDate[9]; 	// ISO/IEC 1974-1 clause 12.3.2
	uint8_t m_DeviceType; 			//Need to check (EXT, NXT, etc)
	uint16_t m_nCaptureDeviceID;	// DeviceID of the capturing device

	LogImageType m_LogImageType;
	uint16_t m_nFrameID;  			// which Frame is this?
	uint16_t m_nCamID;
	uint16_t m_nFaceIndex;			// Which face image does this correspond to
	uint16_t m_nNumOfHaarEyes;
	uint16_t m_nNumofSpecularities;

	uint8_t m_Discarded;
	uint8_t m_EyeLabel;
	uint8_t m_ImageType;
	uint8_t m_ImageFormat;
	uint8_t m_ImageProperties;

	uint16_t m_nImageWidth;
	uint16_t m_nImageHeight;

	uint8_t m_BitDepth;

	uint8_t m_CaputureSession[8+1];

	// EyeDetection values
	float m_fSharpness;

	// Segmentation values
	uint16_t m_nIrisCenterSmallestX;
	uint16_t m_nIrisCenterLargestX;
	uint16_t m_nIrisCenterSmallestY;
	uint16_t m_nIrisCenterLargestY;
	uint16_t m_nIrisDiameterSmallest;
	uint16_t m_nIrisDiameterLargest;

	uint32_t m_nBaseObjectLength =
				sizeof(m_nRecordLength) +
				sizeof(m_Version) +
				sizeof(m_CreationTimeDate) +
				sizeof(m_DeviceType) +
				sizeof(m_nCaptureDeviceID) +
				sizeof(m_nFrameID) +
				sizeof(m_nCamID) +
				sizeof(m_nFaceIndex) +
				sizeof(m_nNumOfHaarEyes) +
				sizeof(m_nNumofSpecularities) +
				sizeof(m_Discarded) +
				sizeof(m_EyeLabel) +
				sizeof(m_ImageType) +
				sizeof(m_ImageFormat) +
				sizeof(m_ImageProperties) +
				sizeof(m_nImageWidth) +
				sizeof(m_nImageHeight) +
				sizeof(m_BitDepth) +
				sizeof(m_CaputureSession) +
				sizeof(m_fSharpness) +
				sizeof(m_nIrisCenterSmallestX) +
				sizeof(m_nIrisCenterLargestX) +
				sizeof(m_nIrisCenterSmallestY) +
				sizeof(m_nIrisCenterLargestY) +
				sizeof(m_nIrisDiameterSmallest) +
				sizeof(m_nIrisDiameterLargest) +
				sizeof(uint32_t); // The four is for the vector image length, gets added in the vector serialize...


	vector<LogImagePoint> m_arEyeDetectionPoints;
	vector<uint8_t> m_arImageData;
	//Configuration *m_pConf;

private:
	uint8_t 		*m_pMemFileData;	// Memory byte[] representation for writing to disk...
	LogString		key;

public:
	static unsigned int random_char();
	static std::string generate_hex(const unsigned int len);


public:
	LogImageRecord() : m_nRecordLength(0), m_DeviceType(EYELOCK_EXT), m_nCaptureDeviceID(0), m_nFrameID(0), m_nCamID(0), m_nFaceIndex(0),
						m_nNumOfHaarEyes(0), m_nNumofSpecularities(0), m_Discarded(0), m_EyeLabel(SUBJECT_EYE_LABEL_UNDEF),  m_ImageType(IRIS_FRAME),
					m_ImageFormat(IMAGEFORMAT_MONO_RAW), m_ImageProperties(0), m_nImageWidth(640), m_nImageHeight(480), m_BitDepth(8), m_fSharpness(0.0)
					 { m_pMemFileData = NULL; m_nRecordLength = m_nBaseObjectLength; SetCreationTime(); }
	virtual ~LogImageRecord() {};


	// Static API's for adding, modifying and removing this object from the MDC...
	// Add an allocated LogImageRecord to the MDC
	  static LogImageRecord *put(const std::string& key, uint8_t *pData, uint32_t width, uint32_t height);
	  /**
	  * Get the context identified by the <code>key</code> parameter.
	  *
	  *  <p>This method has no side effects.
	  *  @param key key.
	  *  @return value for key, empty if not set.
	  * */
	  static LogImageRecord *get(const std::string& key);

	  /**
	  * Remove the the context identified by the <code>key</code>
	  * parameter.
	  *  @param key key.
	  * @return value if key had been set, empty if not.
	  */
	  static void remove(const std::string& key);


	// Getters & setters...
	// This size calculation here is only used for creating a mem stream of the object.  It's not used by serialization but must be accurate
	// If any variables are added to the class, this function must be updated
	uint32_t GetTotalRecordSize() { return m_nRecordLength + //This is base size + total for imagedata...
											m_arEyeDetectionPoints.size()*LogImagePoint::GetSize()+sizeof(uint32_t); // Points + 4 bytes for vector count
									}

	bool SetImageData(uint8_t *pData, uint32_t width, uint32_t height);
	vector<uint8_t>GetImageData() { return m_arImageData; }

	void AddEyeDetectionPoint(LogImagePoint thePoint) { m_arEyeDetectionPoints.push_back(thePoint); }

	void SetCreationTime(void)
	{
		time_t curr_time;
		curr_time = time(NULL);

		tm *tm_gmt = gmtime(&curr_time);

		uint16_t nYear = tm_gmt->tm_year + 1900;

		m_CreationTimeDate[8] = (uint8_t)((nYear >> 8) & 0x00FF);
		m_CreationTimeDate[7] = (uint8_t)(nYear & 0x00FF);
		m_CreationTimeDate[6] = (uint8_t)tm_gmt->tm_mon;
		m_CreationTimeDate[5] = (uint8_t)tm_gmt->tm_mday;
		m_CreationTimeDate[4] = (uint8_t)tm_gmt->tm_hour;
		m_CreationTimeDate[3] = (uint8_t)tm_gmt->tm_min;
		m_CreationTimeDate[2] = (uint8_t)tm_gmt->tm_sec;
		m_CreationTimeDate[1] = (uint8_t)0xFF;
		m_CreationTimeDate[0] = (uint8_t)0xFF;
	}

	// Return ptr to internally allocated data... get's cleanup up in ISOBIometri destructor
	uint8_t *SaveToMemFile();					// serialize our ISOBiometric to a std::stringstreambuffer
	// Returns a copy... data is persistent...
	vector<uint8_t> SaveToMemFileCopy();

	int SaveToFile(const char *pszFilePath);	// serialize our ISOBiometric to our m_pISOObject
#if 0
	int LoadFromMemFile(uint8_t *pImageRecord, uint32_t dwSize);  // de-serialize an in memory object into our LogImageRecord class
	int LoadFromFile(const char *pszFilePath);	// de-serialize a file into our LogImageRecord class
#endif
//	void SetConf(Configuration *pConf) { m_pConf = pConf;}
//	bool ConvertFrom8bitGreyscaleToFormat(uint8_t *pData, uint32_t width, uint32_t height, ISO_EYELABELTYPE theEye, ISO_IMAGEFORMAT theFormat);
//	vector<uint8_t> ConvertFromFormatTo8bitGreyscale(vector<uint8_t>arData, uint8_t nImageFormat);

	ostream& write(ostream& s)
	{
		m_nRecordLength = m_nBaseObjectLength + m_arImageData.size();

		serializelog::write(s, m_nRecordLength);
		serializelog::write(s, m_Version);
		serializelog::write(s, m_CreationTimeDate);
		serializelog::write(s, m_DeviceType);
		serializelog::write(s, m_nCaptureDeviceID);
		serializelog::write(s, m_nFrameID);  			// which Frame is this?
		serializelog::write(s, m_nCamID);
		serializelog::write(s, m_nFaceIndex);
		serializelog::write(s, m_nNumOfHaarEyes);
		serializelog::write(s, m_nNumofSpecularities);
		serializelog::write(s, m_Discarded);
		serializelog::write(s, m_EyeLabel);
		serializelog::write(s, m_ImageType);
		serializelog::write(s, m_ImageFormat);
		serializelog::write(s, m_ImageProperties);

		serializelog::write(s, m_nImageWidth);
		serializelog::write(s, m_nImageHeight);

		serializelog::write(s, m_BitDepth);

		serializelog::write(s, m_CaputureSession);

		serializelog::write(s, m_fSharpness);

		serializelog::write(s, m_nIrisCenterSmallestX);
		serializelog::write(s, m_nIrisCenterLargestX);
		serializelog::write(s, m_nIrisCenterSmallestY);
		serializelog::write(s, m_nIrisCenterLargestY);
		serializelog::write(s, m_nIrisDiameterSmallest);
		serializelog::write(s, m_nIrisDiameterLargest);

		serializelog::write(s, m_arEyeDetectionPoints);

		serializelog::write(s, m_arImageData);

		return s;
	}

	istream& read(istream& s)
	{
		serializelog::read(s, m_nRecordLength);
		serializelog::read(s, m_CreationTimeDate);
		serializelog::read(s, m_DeviceType);
		serializelog::read(s, m_nCaptureDeviceID);
		serializelog::read(s, m_nFrameID);
		serializelog::read(s, m_nCamID);
		serializelog::read(s, m_nFaceIndex);
		serializelog::read(s, m_nNumOfHaarEyes);
		serializelog::read(s, m_nNumofSpecularities);
		serializelog::read(s, m_Discarded);
		serializelog::read(s, m_EyeLabel);
		serializelog::read(s, m_ImageType);
		serializelog::read(s, m_ImageFormat);
		serializelog::read(s, m_ImageProperties);

		serializelog::read(s, m_nImageWidth);
		serializelog::read(s, m_nImageHeight);

		serializelog::read(s, m_BitDepth);

		serializelog::read(s, m_CaputureSession);

		serializelog::read(s, m_fSharpness);

		serializelog::read(s, m_nIrisCenterSmallestX);
		serializelog::read(s, m_nIrisCenterLargestX);
		serializelog::read(s, m_nIrisCenterSmallestY);
		serializelog::read(s, m_nIrisCenterLargestY);
		serializelog::read(s, m_nIrisDiameterSmallest);
		serializelog::read(s, m_nIrisDiameterLargest);

		serializelog::read(s, m_arEyeDetectionPoints);

		serializelog::read(s, m_arImageData);

		//m_arImageData = ConvertFromFormatToRAW(m_arImageData, m_ImageFormat);

		return s;
	}
};
#define LogImage LogImageRecord





#endif /* LOGIMAGE_H_ */

