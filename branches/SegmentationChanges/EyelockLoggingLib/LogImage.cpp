#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <random>
#include <stdarg.h>

#include <LogImage.h>
#include <MemoryStream.h>

#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/mdc.h>


using namespace std;
using namespace log4cxx;



// This hack reference required to our class linked into our applications...
//EyelockBinaryAppender theAppender();



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




unsigned int LogImageRecord::random_char()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}

std::string LogImageRecord::generate_hex(const unsigned int len)
{
    std::stringstream ss;
    for (unsigned int i = 0; i < len; i++) {
        const auto rc = random_char();
        std::stringstream hexstream;
        hexstream << std::hex << rc;
        auto hex = hexstream.str();
        ss << (hex.length() < 2 ? '0' + hex : hex);
    }
    return ss.str();
}



// Store the passed in images data to a vector...
bool LogImageRecord::SetImageData(uint8_t *pData, uint32_t width, uint32_t height)
{
	m_arImageData.empty();

	// Copy the image data internally
	m_arImageData.insert(m_arImageData.end(), pData, pData+(width*height));

	// Update all of the relevant record information
	m_ImageFormat = IMAGEFORMAT_MONO_RAW;

	// Update the length of our record...
	m_nRecordLength += (width*height);				// This is the length of the actual data
	m_nImageWidth = width;
	m_nImageHeight = height;
	return true;
}


uint8_t *LogImageRecord::SaveToMemFile()
{
	// Saves the logimage data structures to a memory block...
	// Allocate the block based on our total size...
	if (NULL != m_pMemFileData)
		free(m_pMemFileData);

	m_pMemFileData = new uint8_t[GetTotalRecordSize()];

	// Caller is responsible for freeing the block
	OMemStream memfile((char *)m_pMemFileData, GetTotalRecordSize());

	// Serialize the whole mess into memory...
	write(memfile);

	return m_pMemFileData;
}


vector<uint8_t> LogImageRecord::SaveToMemFileCopy()
{
	vector<uint8_t> theData;

	uint8_t *pData = SaveToMemFile();

	theData.insert(theData.end(), pData, pData+GetTotalRecordSize());

	return theData;
}




//Write out logImage record out to disk...
int LogImageRecord::SaveToFile(const char *pszFilePath)
{
	//Create a filestream...
	ofstream file(pszFilePath, ios::out | ios::binary | ios::trunc);

	// Write the record
	write(file);

	file.flush();
	file.close();

	return 0;
}


//Statics
LogImageRecord *LogImageRecord::put(const std::string& key, uint8_t *pData, uint32_t width, uint32_t height)
{
	char ptrString[16+3];

	// DMOTODO check for existence of this image type first?
	LogImageRecord *pLogImage = new LogImageRecord();
	pLogImage->SetImageData(pData, width, height);

	// Now we have our object created and we have image data in it...
	// Create our UUID:LogImageRecordPtr value and put it into the MDC...
	std::string sValue = generate_hex(8);
	std::string delim(";");
	sValue += delim;
	sprintf(ptrString, "%p", pLogImage);
	sValue += ptrString;

	// This string value now had a GUID and a string representation of our pointer...
	// Simply place it in the MDC.
	log4cxx::MDC::put(key, sValue);

	return pLogImage;
}


// Return an actual ptr to the LogImageRecord object referenced by the key...
LogImageRecord *LogImageRecord::get(const std::string& key)
{
		std::string sValue = log4cxx::MDC::get(key);

		if (sValue.length())
		{
			 vector<string> result;

			 stringstream ss(sValue);
			 string item;

 			 while (std::getline(ss, item, ';'))
				result.push_back (item);

 			 if (result.size() > 1)
 			 {
 				LogImageRecord *pLogImage;

				if (sscanf(result[1].c_str(), "%p", &pLogImage) <= 0)
					pLogImage = NULL;

 				return pLogImage;
 			 }
 			 else
 				 return NULL;
		}
		else
			return NULL;
}


void LogImageRecord::remove(const std::string& key)
{
	log4cxx::MDC::remove(key);
}



#if 0
// Loads an empty Log Image object with data from a memory block
int LogImageRecord::LoadFromMemFile(uint8_t *pMemFileData, uint32_t dwSize)	// de-serialize a file into our LogImage class
{
	IMemStream memfile((char *)pMemFileData, dwSize);

	// De-serialize it...
	serialize::read(memfile);
}

// int Load(unsigned char *pISOData);			// de-serialize an in memory object into our ISOBiometric class
int ISOBiometric::LoadFromFile(const char *pszFilePath)	// de-serialize a file into our ISOBiometric class
{
	//Create a filestream...
	ifstream file(pszFilePath, ios::in | ios::binary);

	// read all of the BDB
	serialize::read(file);

	file.close();

	return 0;
}
#endif





