/*
 * ImageFormatConverter.h
 *
 *  Created on: Feb 22, 2019
 *      Author: root
 */

#ifndef INCLUDE_IMAGEFORMATCONVERTER_H_
#define INCLUDE_IMAGEFORMATCONVERTER_H_

#include <string>
#include <ctime>
#include <limits>
#include <vector>
#include <fstream>
#include <sstream>
#include <istream>
#include <ostream>
#include <streambuf>

// From OpenJPEG Source
#include "openjpeg.h"
#include "convert.h"


using namespace std;


// Iris Biometric Record
class ImageConvert
{
public:
	ImageConvert()	{};
	virtual ~ImageConvert() {};

	typedef struct
	{
		OPJ_UINT8* pData; //Our data.
		OPJ_SIZE_T dataSize; //How big is our data.
		OPJ_SIZE_T offset; //Where are we currently in our data.
	}opj_memory_stream;


	// Public Members for conversion...
	static bool Convert8bitGreyscaleToJPEG2K(uint8_t *pData, uint32_t width, uint32_t height, int j2kQuality, vector<uint8_t>&arOutImage);
	static bool ConvertJPEG2KTo8bitGreyscale(uint8_t *pData, uint32_t width, uint32_t height, int j2kQuality, vector<uint8_t>&arOutImage);

	static bool Convert8bitGreyscaleToPNG(uint8_t *pData, uint32_t width, uint32_t height, vector<uint8_t>&arOutImage);
	static bool ConvertPNGTo8bitGreyscale(uint8_t *pData, uint32_t width, uint32_t height, vector<uint8_t>&arOutImage);


	// Members...
protected:
	// J2K Conversion helpers...
	static opj_image_t* rawtoimage(uint8_t *pData, opj_cparameters_t *parameters, uint32_t width, uint32_t height);

	static OPJ_SIZE_T opj_memory_stream_read(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data);
	static OPJ_SIZE_T opj_memory_stream_write(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data);
	static OPJ_OFF_T opj_memory_stream_skip(OPJ_OFF_T p_nb_bytes, void * p_user_data);
	static OPJ_BOOL opj_memory_stream_seek(OPJ_OFF_T p_nb_bytes, void * p_user_data);
	static void opj_memory_stream_do_nothing(void * p_user_data);
	static opj_stream_t* opj_stream_create_default_memory_stream(opj_memory_stream* p_memoryStream, OPJ_BOOL p_is_read_stream);

	static void error_callback(const char *msg, void *client_data);
	static void warning_callback(const char *msg, void *client_data);
	static void info_callback(const char *msg, void *client_data);
};



#endif /* INCLUDE_IMAGEFORMATCONVERTER_H_ */
