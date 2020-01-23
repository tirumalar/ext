/*
 * LogImageJSON.cpp
 *
 *  Created on: Jan 16, 2020
 *      Author: root
 */
#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include <random>
#include <stdarg.h>

#include <log4cxx/helpers/transcoder.h>
#include <log4cxx/mdc.h>
#include <LogImageJSON.h>


using namespace std;
using namespace log4cxx;



const char *log_format(const char *fmt, ...)
{
  va_list va;
  static char formatted[1024];
  va_start(va, fmt);
  vsnprintf(formatted, 1024, fmt, va);
  va_end(va);
  return formatted;
}



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




unsigned int LogImageRecordJSON::random_char()
{
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 255);
    return dis(gen);
}


std::string LogImageRecordJSON::base64(const char* data, int size)
{
	if (data == NULL || size <= 0)
	{
		return std::string("");
	}

	return Base64encode((const unsigned char *)data, (unsigned int)size);
}


static const std::string logiamge_base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";


std::string LogImageRecordJSON::Base64encode(unsigned char const* bytes_to_encode, unsigned int in_len)
{
  std::string ret;
  int i = 0;
  int j = 0;
  unsigned char char_array_3[3];
  unsigned char char_array_4[4];

  while (in_len--)
  {
    char_array_3[i++] = *(bytes_to_encode++);
    if (i == 3)
    {
      char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
      char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
      char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
      char_array_4[3] = char_array_3[2] & 0x3f;

      for(i = 0; (i <4) ; i++)
        ret += logiamge_base64_chars[char_array_4[i]];
      i = 0;
    }
  }

  if (i)
  {
    for(j = i; j < 3; j++)
      char_array_3[j] = '\0';

    char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
    char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
    char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
    char_array_4[3] = char_array_3[2] & 0x3f;

    for (j = 0; (j < i + 1); j++)
      ret += logiamge_base64_chars[char_array_4[j]];

    while((i++ < 3))
      ret += '=';

  }

  return ret;
}


std::string LogImageRecordJSON::generate_hex(const unsigned int len)
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
bool LogImageRecordJSON::SetImageData(uint8_t *pData, uint32_t width, uint32_t height)
{
	m_arImageData.empty();

	// Copy the image data internally
	m_arImageData.insert(m_arImageData.end(), pData, pData+(width*height));

	// Update all of the relevant record information
	m_nImageFormat = IMAGEFORMAT_MONO_RAW;

	// Update the length of our record...
	m_nImageWidth = width;
	m_nImageHeight = height;
	return true;
}


string LogImageRecordJSON::GetObjectAsJSON()
{
	json theObject;

	// Snapshot of processing duration
    auto t2 = std::chrono::high_resolution_clock::now();
    // floating-point duration: no duration_cast needed
    std::chrono::duration<double, std::milli>fp_ms = t2 - m_CreationTime;


	// Create all of the JSON elements for writing...
	theObject["Version"] = m_Version;
	theObject["FrameUUID"] = m_lFrameUUID;
	theObject["DateTime"] = m_sDateTime;
	theObject["TimeStampMS"] = m_lTimeStampMS;
	theObject["ProcessingDurationMS"] = fp_ms.count();
	theObject["DeviceType"] = m_nDeviceType;
	theObject["DeviceID"] = m_sCaptureDeviceID;
	theObject["FrameID"] = m_nFrameID;
	theObject["CameraID"] = m_nCamID;
	theObject["FaceIndex"] = m_nFaceIndex;
	theObject["HaarEyeCount"] = m_nNumOfHaarEyes;
	theObject["SpecularityCount"] = m_nNumofSpecularities;
	theObject["Discarded"] = m_bDiscarded;
	theObject["EyeLabel"] = m_nEyeLabel;
	theObject["ImageType"] = m_nImageType;
	theObject["ImageFormat"] = m_nImageFormat;
	theObject["ImageProperties"] = m_nImageProperties;
	theObject["ImageWidth"] = m_nImageWidth;
	theObject["ImageHeight"] = m_nImageHeight;
	theObject["BitDepth"] = m_nBitDepth;
	theObject["CaptureSession"] = m_sCaptureSession;
	theObject["Sharpness"] = m_fSharpness;

	// Populate the vector of custom eyedectectionpoints objects...
	json ptVector(m_arEyeDetectionPoints);
	theObject["EyeDetectionPoints"] = ptVector;
	json rectVector(m_arEyeDetectionCropRects);
	theObject["EyeDetectionCropRects"] = rectVector;

	// Segmentation values...
	theObject["IrisScore"] = m_irisScore;
	theObject["DarkScore"] = m_darkScore;
	theObject["SpecScore"] = m_specScore;
	theObject["PupilScore"] = m_pupilScore;
	theObject["ValidTemplateBits"] = m_validBitsInTemplate;
	theObject["EyelidCoverage"] = m_eyelidCoverage;
	theObject["UseableIrisArea"] = m_UseableIrisArea;
	theObject["Gaze.Z"] = m_gazeZ;
	theObject["PupilToIrisRatio"] = m_pupilToIrisRatio;
	theObject["IrisDiameter"] = m_IrisDiameter;
	theObject["PupilDiameter"] = m_PupilDiameter;
	theObject["SpecDiameter"] = m_SpecDiameter;

	theObject["SegmentationCheck"] = m_bSegmentationCheck;
	theObject["CorruptBitCountMask"] = m_corruptBitCountMask;
	theObject["IrisImagecentre_distX"] = m_irisImagecentre_distX;
	theObject["IrisImagecentre_distY"] = m_irisImagecentre_distY;

	theObject["PupilCircle"] = m_Pupil;
	theObject["IrisCircle"] = m_Iris;
	theObject["TemplatePipelineError"] = m_TemplatePipelineError;

	theObject["imageData"] = base64((char *)m_arImageData.data(), m_arImageData.size());

	return theObject.dump();
}

//Statics
LogImageRecordJSON *LogImageRecordJSON::put(const std::string& key, uint8_t *pData, uint32_t width, uint32_t height)
{
	char ptrString[16+3];

	// DMOTODO check for existence of this image type first?
	LogImageRecordJSON *pLogImage = new LogImageRecordJSON();
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
LogImageRecordJSON *LogImageRecordJSON::get(const std::string& key)
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
 				LogImageRecordJSON *pLogImage;

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


void LogImageRecordJSON::remove(const std::string& key)
{
	log4cxx::MDC::remove(key);
}









