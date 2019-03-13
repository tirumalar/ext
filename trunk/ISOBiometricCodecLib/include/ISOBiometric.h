#ifndef ISOBIOMETRIC_H_
#define ISOBIOMETRIC_H_

//#include "logging.h"
#include "serialization.h"

#include <string>
#include <ctime>
#include <limits>
#include <vector>
#include <fstream>
#include <sstream>
#include <istream>
#include <ostream>
#include <streambuf>
#include "ImageFormatConverter.h"
#include "FileConfiguration.h"


using namespace std;


typedef unsigned char		uint8_t;
typedef unsigned short int	uint16_t;
#ifndef __uint32_t_defined
typedef unsigned int		uint32_t;
# define __uint32_t_defined
#endif


// Iris Biometric Record
class BDBIrisRecord : public serialize::I
{
public:
	enum CaptureDeviceType
	{
		UNKNOWN = 0,
		CMOS_CCD
	};

	typedef enum EyeLabelType
	{
		SUBJECT_EYE_LABEL_UNDEF = 0,
		SUBJECT_EYE_LABEL_RIGHT,
		SUBJECT_EYE_LABEL_LEFT
	}ISO_EYELABELTYPE;

	typedef enum ImageType
	{
		IMAGE_TYPE_UNCROPPED = 1,
		IMAGE_TYPE_VGA = 2,
		IMAGE_TYPE_CROPPED = 3,
		IMAGE_TYPE_CROPPED_AND_MASKED = 7
	}ISO_IMAGETYPE;

	typedef enum ImageFormat
	{
		IMAGEFORMAT_MONO_RAW = 2,
		IMAGEFORMAT_MONO_JPEG2000 = 10,
		IMAGEFORMAT_MONO_PNG = 14
	}ISO_IMAGEFORMAT;

	enum EyeRange
	{
		RANGE_UNASSIGNED = 0,
		RANGE_FAILED = 1,
		RANGE_OVERFLOW = std::numeric_limits<short>::max()
	};

	uint32_t m_nRecordLength; 		//Includes header
	uint8_t m_CreationTimeDate[9]; 	// ISO/IEC 1974-1 clause 12.3.2
	uint8_t m_DeviceType; //Need to check
	uint16_t m_nCaptureDeviceVendorID;
	uint16_t m_nCaptureDeviceTypeID;
	uint8_t m_QualityBlock;
	uint16_t m_nRepresentationNumber;  // which record is this?  starts at 1
	uint8_t m_EyeLabel;
	uint8_t m_ImageType;
	uint8_t m_ImageFormat;
	uint8_t m_ImageProperties;

	uint16_t m_nImageWidth;
	uint16_t m_nImageHeight;

	uint8_t m_BitDepth;
	uint16_t m_nIrisRange;
	uint16_t m_nEyeRollAngle;
	uint16_t m_nEyeRollAngleUncertainty;

	uint16_t m_nIrisCenterSmallestX;
	uint16_t m_nIrisCenterLargestX;
	uint16_t m_nIrisCenterSmallestY;
	uint16_t m_nIrisCenterLargestY;
	uint16_t m_nIrisDiameterSmallest;
	uint16_t m_nIrisDiameterLargest;

	uint32_t m_nBaseObjectLength =
				sizeof(m_nRecordLength) +
				sizeof(m_CreationTimeDate) +
				sizeof(m_DeviceType) +
				sizeof(m_nCaptureDeviceVendorID) +
				sizeof(m_nCaptureDeviceTypeID) +
				sizeof(m_QualityBlock) +
				sizeof(m_nRepresentationNumber) +
				sizeof(m_EyeLabel) +
				sizeof(m_ImageType) +
				sizeof(m_ImageFormat) +
				sizeof(m_ImageProperties) +

				sizeof(m_nImageWidth) +
				sizeof(m_nImageHeight) +

				sizeof(m_BitDepth) +
				sizeof(m_nIrisRange) +
				sizeof(m_nEyeRollAngle) +
				sizeof(m_nEyeRollAngleUncertainty) +

				sizeof(m_nIrisCenterSmallestX) +
				sizeof(m_nIrisCenterLargestX) +
				sizeof(m_nIrisCenterSmallestY) +
				sizeof(m_nIrisCenterLargestY) +
				sizeof(m_nIrisDiameterSmallest) +
				sizeof(m_nIrisDiameterLargest) +
				sizeof(uint32_t); // The four is for the vector image length, gets added in the vector serialize...

	vector<uint8_t> m_arImageData;
	Configuration *m_pConf;


	BDBIrisRecord() : m_nRecordLength(0), m_DeviceType(CMOS_CCD), m_nCaptureDeviceVendorID(0xFF), m_nCaptureDeviceTypeID(0), m_QualityBlock(0),
					m_nRepresentationNumber(0),
					m_EyeLabel(SUBJECT_EYE_LABEL_UNDEF),  m_ImageType(IMAGE_TYPE_VGA),
					m_ImageFormat(IMAGEFORMAT_MONO_RAW), m_ImageProperties(0), m_nImageWidth(640), m_nImageHeight(480), m_BitDepth(8),
					m_nIrisRange(RANGE_UNASSIGNED), m_nEyeRollAngle(0),  m_nEyeRollAngleUncertainty(0), m_nIrisCenterSmallestX(0),
					m_nIrisCenterLargestX(0), m_nIrisCenterSmallestY(0), m_nIrisCenterLargestY(0), m_nIrisDiameterSmallest(0),
					m_nIrisDiameterLargest(0) { m_nRecordLength = m_nBaseObjectLength; };
	virtual ~BDBIrisRecord() {};


	// Getters & setters...
	uint32_t GetTotalRecordSize() { return m_nRecordLength; }

	bool SetImageData(uint8_t *pData, uint32_t width, uint32_t height, ISO_EYELABELTYPE theEye, ISO_IMAGEFORMAT theFormat);
	vector<uint8_t>GetImageData() { return m_arImageData; }

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

	void SetRepresentationNumber(uint16_t theNumber) { m_nRepresentationNumber = theNumber; }
	void SetConf(Configuration *pConf) { m_pConf = pConf;}
	bool ConvertFrom8bitGreyscaleToFormat(uint8_t *pData, uint32_t width, uint32_t height, ISO_EYELABELTYPE theEye, ISO_IMAGEFORMAT theFormat);
	vector<uint8_t> ConvertFromFormatTo8bitGreyscale(vector<uint8_t>arData, uint8_t nImageFormat);



public:
	ostream& write(ostream& s)
	{
		m_nRecordLength = m_nBaseObjectLength + m_arImageData.size();

		serialize::write(s, m_nRecordLength);
		serialize::write(s, m_CreationTimeDate);
		serialize::write(s, m_DeviceType);
		serialize::write(s, m_nCaptureDeviceVendorID);
		serialize::write(s, m_nCaptureDeviceTypeID);
		serialize::write(s, m_QualityBlock);
		serialize::write(s, m_nRepresentationNumber);
		serialize::write(s, m_EyeLabel);
		serialize::write(s, m_ImageType);
		serialize::write(s, m_ImageFormat);
		serialize::write(s, m_ImageProperties);

		serialize::write(s, m_nImageWidth);
		serialize::write(s, m_nImageHeight);

		serialize::write(s, m_BitDepth);
		serialize::write(s, m_nIrisRange);
		serialize::write(s, m_nEyeRollAngle);
		serialize::write(s, m_nEyeRollAngleUncertainty);

		serialize::write(s, m_nIrisCenterSmallestX);
		serialize::write(s, m_nIrisCenterLargestX);
		serialize::write(s, m_nIrisCenterSmallestY);
		serialize::write(s, m_nIrisCenterLargestY);
		serialize::write(s, m_nIrisDiameterSmallest);
		serialize::write(s, m_nIrisDiameterLargest);

		serialize::write(s, m_arImageData);

		return s;
	}

	istream& read(istream& s)
	{
		serialize::read(s, m_nRecordLength);
		serialize::read(s, m_CreationTimeDate);
		serialize::read(s, m_DeviceType);
		serialize::read(s, m_nCaptureDeviceVendorID);
		serialize::read(s, m_nCaptureDeviceTypeID);
		serialize::read(s, m_QualityBlock);
		serialize::read(s, m_nRepresentationNumber);
		serialize::read(s, m_EyeLabel);
		serialize::read(s, m_ImageType);
		serialize::read(s, m_ImageFormat);
		serialize::read(s, m_ImageProperties);

		serialize::read(s, m_nImageWidth);
		serialize::read(s, m_nImageHeight);

		serialize::read(s, m_BitDepth);
		serialize::read(s, m_nIrisRange);
		serialize::read(s, m_nEyeRollAngle);
		serialize::read(s, m_nEyeRollAngleUncertainty);

		serialize::read(s, m_nIrisCenterSmallestX);
		serialize::read(s, m_nIrisCenterLargestX);
		serialize::read(s, m_nIrisCenterSmallestY);
		serialize::read(s, m_nIrisCenterLargestY);
		serialize::read(s, m_nIrisDiameterSmallest);
		serialize::read(s, m_nIrisDiameterLargest);

		serialize::read(s, m_arImageData);

		//m_arImageData = ConvertFromFormatToRAW(m_arImageData, m_ImageFormat);

		return s;
	}
};
#define BDBIris BDBIrisRecord


// General Iris Biometric header...
class BDBIrisHeader : public serialize::I
{
	enum EyeRepresentationType
	{
		UNKNOWN = 0,
		LEFT_OR_RIGHT,
		LEFT_AND_RIGHT
	};

	// Reversed to correct for endianess issues in the serialization routines...
	const uint8_t m_FormatId[4] {0x00, 0x52, 0x49, 0x49};
	const uint8_t m_Version[4] {0x00, 0x30, 0x32, 0x30};


	const uint8_t m_CertificationFlag {0x00};

private:
	uint8_t m_nEyeRepresentation;
	uint32_t m_nRecordLength; 				//Includes entire blob
	uint16_t m_nIrisRecordCount;			//Count of BDBIrisRecords in this blob
	std::vector<BDBIris> 	m_arBDBIris;	// An array of BDBIris objects holding the actual data and image


	const uint16_t m_nHeaderBaseSize = sizeof(m_FormatId) + sizeof(m_Version) + sizeof(m_CertificationFlag) +
										sizeof(m_nRecordLength) + sizeof(m_nEyeRepresentation) + sizeof(m_nIrisRecordCount);

public:
	BDBIrisHeader() : m_nRecordLength(0), m_nIrisRecordCount(0), m_nEyeRepresentation((uint8_t)UNKNOWN) { m_nRecordLength = m_nHeaderBaseSize; };
	virtual ~BDBIrisHeader() {};

	// There is no provision to remove added irises...  it would require re-evaluating the BDBIrisRecords and updating internal values...
	BDBIris *AddIris(Configuration *pConf)
	{
		BDBIris theIris;

		// Which number iris is this in our list?
		theIris.SetRepresentationNumber(m_arBDBIris.size()+1);
		theIris.SetCreationTime();
		theIris.SetConf(pConf);

		m_arBDBIris.push_back(theIris);

		// Update EyeRepresentation...  User is responsible for adding ONLY one left and/or one right!
		m_nEyeRepresentation = m_arBDBIris.size() == 1 ? LEFT_OR_RIGHT : LEFT_AND_RIGHT;

		// Return a pointer to our irisrecord...
		return &m_arBDBIris.back();
	}


	BDBIris *GetIrisAt(int nElement)
	{
		vector<uint8_t> empty;

		if (nElement > (int)(m_arBDBIris.size()-1))
			return NULL;
		else
			return &m_arBDBIris[nElement];
	}

	vector<uint8_t> GetIrisImageAt(int nElement)
	{
		vector<uint8_t> empty;

		if (nElement > (int)(m_arBDBIris.size()-1))
			return empty;
		else
			return m_arBDBIris[nElement].GetImageData();
	}


	uint16_t GetIrisCount() { return m_arBDBIris.size(); }


	uint32_t GetTotalBiometricSize(void)
	{
		uint32_t theSize =  m_nHeaderBaseSize;

		for (int nIndex = 0; nIndex < (int)m_arBDBIris.size(); nIndex++)
			theSize += m_arBDBIris[nIndex].GetTotalRecordSize();

		return theSize;
	}

	ostream& write(ostream& s)
	{
		serialize::write(s, m_FormatId);
		serialize::write(s, m_Version);

		// Calculate the total record length...
		// This header

		// All of our IRIS Records, we have to do this here because records may or may not have image data
		// at a given point in time...
		m_nRecordLength = m_nHeaderBaseSize;

		for (int nIndex = 0; nIndex < (int)m_arBDBIris.size(); nIndex++)
			m_nRecordLength += m_arBDBIris[nIndex].GetTotalRecordSize();

		serialize::write(s, m_nRecordLength);
		m_nIrisRecordCount = (uint16_t)m_arBDBIris.size();
		serialize::write(s, m_nIrisRecordCount); // write is picky about params
		serialize::write(s, m_CertificationFlag);
		serialize::write(s, m_nEyeRepresentation);

		for (int nIndex = 0; nIndex < (int)m_arBDBIris.size(); nIndex++)
			serialize::write(s, m_arBDBIris[nIndex]);
		return s;
	}

	istream& read(istream& s)
	{
		serialize::read(s, m_FormatId);
		serialize::read(s, m_Version);
		serialize::read(s, m_nRecordLength);
		serialize::read(s, m_nIrisRecordCount);
		serialize::read(s, m_CertificationFlag);
		serialize::read(s, m_nEyeRepresentation);

		serialize::read(s, m_arBDBIris);
		return s;
	}
};


class StandardBiometricHeader
{
	//Anything in here?
};
#define SBH StandardBiometricHeader



// This is the main class used for creating, reading and writing ISO formatted Biometric Iris data...
class ISOBiometric
{
public:
	ISOBiometric() { m_pMemFileData = NULL; };
	ISOBiometric(Configuration *pConf) { m_pMemFileData = NULL; m_pConf = pConf; };

	virtual ~ISOBiometric() { if (m_pMemFileData != NULL) delete m_pMemFileData; };

	//data
protected:
	BDBIrisHeader	m_IrisHeader;		// The one an only Iris header object.
	uint8_t 		*m_pMemFileData;	// Memory blob representation in ISO format
	Configuration 	*m_pConf;

	// members
public:
	uint32_t GetTotalBiometricSize() { return m_IrisHeader.GetTotalBiometricSize(); }
	int	GetIrisCount() { return m_IrisHeader.GetIrisCount(); }
	BDBIris *AddIrisBiometric() { return m_IrisHeader.AddIris(m_pConf); }		// Allocates and adds an empty iris biometric
	BDBIris *GetIrisAt(int nElement) { return m_IrisHeader.GetIrisAt(nElement); }
	vector<uint8_t> GetIrisImageAt(int nElement) { return m_IrisHeader.GetIrisImageAt(nElement); }


	// Return ptr to internally allocated data... get's cleanup up in ISOBIometri destructor
	uint8_t *SaveToMemFile();					// serialize our ISOBiometric to a std::stringstreambuffer
	// Returns a copy... data is persistent...
	vector<uint8_t> SaveToMemFileCopy();

	int SaveToFile(const char *pszFilePath);	// serialize our ISOBiometric to our m_pISOObject

	int LoadFromMemFile(uint8_t *pISOData, uint32_t dwSize);  // de-serialize an in memory object into our ISOBiometric class
	int LoadFromFile(const char *pszFilePath);	// de-serialize a file into our ISOBiometric class
};


// In memory stream support
class IMemBuf: public std::streambuf
{
public:
	IMemBuf(const char* base, size_t size)
	{
		char* p(const_cast<char*>(base));
		this->setg(p, p, p + size);
	}
};

class IMemStream: virtual IMemBuf, public std::istream
{
public:
	IMemStream(const char* mem, size_t size) :
		IMemBuf(mem, size),
		std::istream(static_cast<std::streambuf*>(this))
	{
	}
};

class OMemBuf: public std::streambuf
{
public:
	OMemBuf(char* base, size_t size)
	{
		this->setp(base, base + size);
	}
};

class OMemStream: virtual OMemBuf, public std::ostream
{
public:
	OMemStream(char* base, size_t size) :
		OMemBuf(base, size),
		std::ostream(static_cast<std::streambuf*>(this))
	{
	}
};


#endif /* ISOBIOMETRIC_H_ */

