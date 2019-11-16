#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include "ISOBiometric.h"

#include "logging.h"


const char logger[] = "ISOBiometric";

using namespace std;


class TimeFormatter
{
public:
	static std::string format(struct tm *input)
	{
		if (input == NULL)
		{
			return std::string("");
		}

		char buffer[21];
		//2011-03-08T05:25:00Z
		strftime(buffer, 21, "%FT%TZ", input);
		return std::string(buffer);
	}
};


// Store the passed in images data to a vector...
bool BDBIrisRecord::SetImageData(uint8_t *pData, uint32_t width, uint32_t height, ISO_EYELABELTYPE theEye, ISO_IMAGEFORMAT theFormat)
{
	m_arImageData.empty();

	return ConvertFrom8bitGreyscaleToFormat(pData, width, height, theEye, theFormat);
}



bool BDBIrisRecord::ConvertFrom8bitGreyscaleToFormat(uint8_t *pData, uint32_t width, uint32_t height, ISO_EYELABELTYPE theEye, ISO_IMAGEFORMAT theFormat)
{
	m_EyeLabel = theEye;

	switch (theFormat)
	{
		case IMAGEFORMAT_MONO_RAW:
		{
			// This is no-op...  copy the image data internally
			m_arImageData.insert(m_arImageData.end(), pData, pData+(width*height));

			// Update all of the relevant record information
			m_ImageFormat = IMAGEFORMAT_MONO_RAW;

			// Update the length of our record...
			m_nRecordLength += (width*height);				// This is the length of the actual data

			return true;
		}


		case IMAGEFORMAT_MONO_JPEG2000:
		{
			//Call OPENJPEG code
			m_ImageFormat = IMAGEFORMAT_MONO_JPEG2000;

			int nQuality = 100;

			if (NULL != m_pConf)
				nQuality = m_pConf->getValue("Eyelock.J2KImageQuality", 100);

			return ImageConvert::Convert8bitGreyscaleToJPEG2K(pData, width, height, nQuality, m_arImageData);
		}


		case IMAGEFORMAT_MONO_PNG:
		{
			m_ImageFormat = IMAGEFORMAT_MONO_PNG;

			return ImageConvert::Convert8bitGreyscaleToPNG(pData, width, height, m_arImageData);
		}
	}

	return false;
}




vector<uint8_t> BDBIrisRecord::ConvertFromFormatTo8bitGreyscale(vector<uint8_t>arData, uint8_t nImageFormat)
{
	switch (nImageFormat)
	{
		case IMAGEFORMAT_MONO_RAW:
		{
			// This is no-op...
			m_ImageFormat = IMAGEFORMAT_MONO_RAW;
			return arData;
		}

		case IMAGEFORMAT_MONO_JPEG2000:
			//return ImageConvert::Convert8bitGreyscaleToJPEG2K(pData, width, height, 100, m_arImageData);

		case IMAGEFORMAT_MONO_PNG:
			// Not sure where this is coming from yet... maybe we'll write it.
			break;
	}
}


//////////////////////////////////////////////////////////////////
// IrisBiometric Class

uint8_t *ISOBiometric::SaveToMemFile()
{
	// Saves the biometric data structures to a memory block...
	// Allocate the block based on our total size...
	if (NULL != m_pMemFileData)
		free(m_pMemFileData);

	m_pMemFileData = new uint8_t[m_IrisHeader.GetTotalBiometricSize()]; // Check Anita

	// Caller is repsonsible for freeing the block
	OMemStream memfile((char *)m_pMemFileData, m_IrisHeader.GetTotalBiometricSize());

	// Serialize the whole mess into memory...
	m_IrisHeader.write(memfile);

	return m_pMemFileData;
}


vector<uint8_t> ISOBiometric::SaveToMemFileCopy()
{
	vector<uint8_t> theData;

	uint8_t *pData = SaveToMemFile();

	theData.insert(theData.end(), pData, pData+m_IrisHeader.GetTotalBiometricSize());

	return theData;
}




//Write out biometric out to disk...
int ISOBiometric::SaveToFile(const char *pszFilePath)
{
	//Create a filestream...
	ofstream file(pszFilePath, ios::out | ios::binary | ios::trunc);

	// Write the header which also write out all of the BDBIrisRecords...
	m_IrisHeader.write(file);

	file.flush();
	file.close();

	return 0;
}


// Loads an empty Biometric object with data from a memory block
int ISOBiometric::LoadFromMemFile(uint8_t *pMemFileData, uint32_t dwSize)	// de-serialize a file into our ISOBiometric class
{
	IMemStream memfile((char *)pMemFileData, dwSize);

	// De-serialize it...
	serialize::read(memfile, m_IrisHeader);
}

// int Load(unsigned char *pISOData);			// de-serialize an in memory object into our ISOBiometric class
int ISOBiometric::LoadFromFile(const char *pszFilePath)	// de-serialize a file into our ISOBiometric class
{
	//Create a filestream...
	ifstream file(pszFilePath, ios::in | ios::binary);

	// read all of the BDB
	serialize::read(file, m_IrisHeader);

	file.close();

	return 0;
}





