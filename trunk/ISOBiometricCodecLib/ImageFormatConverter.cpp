#include <string>
#include <cstring>
#include <iostream>
#include <sstream>
#include <vector>
#include <algorithm>
#include <iterator>
#include "ImageFormatConverter.h"

#include "logging.h"
//#include "FileConfiguration.h"

const char logger[] = "ImageConverter";

using namespace std;



// Internal Implementation Format Conversions...
bool ImageConvert::Convert8bitGreyscaleToPNG(uint8_t *pData, uint32_t width, uint32_t height, vector<uint8_t>&arOutImage)
{
	IplImage *image = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	memcpy(image->imageData, pData, width*height);

	cv::Mat img = cv::cvarrToMat(image);

	vector<int> compression_params;

	compression_params.push_back(CV_IMWRITE_PNG_COMPRESSION);
	compression_params.push_back(0); // Uncompressed

	vector<uint8_t> theBuffer;

	cv::imencode(".png", img, theBuffer);

	arOutImage.clear();

	if (theBuffer.size() > 0)
		arOutImage.insert(arOutImage.end(), theBuffer.begin(), theBuffer.end());

	cvReleaseImage(&image);
	img.release();
}



bool ImageConvert::ConvertPNGTo8bitGreyscale(uint8_t *pData, uint32_t width, uint32_t height, vector<uint8_t>&arOutImage)
{
	// Not implemented yet...
	assert(false);
}


//Encode/Decode J2K
bool ImageConvert::Convert8bitGreyscaleToJPEG2K(uint8_t *pData, uint32_t width, uint32_t height, int j2kQuality, vector<uint8_t>&arOutImage)
{
    //Our output stream...
    opj_memory_stream outputBufferStream;

    //OpenJPEG Stuff
    OPJ_BOOL 			bSuccess;
    opj_cparameters_t	parameters;	// compression parameters.
    opj_stream_t 		*l_stream = 00;
    opj_codec_t			*l_codec = 00;
    opj_image_t			*image = NULL;


    //Check and set the quality for lossy.
    if (j2kQuality < 0)
        j2kQuality = 100;  //DMOTODO get from ini file //IniValue::GetInstance()->sscscpPtr->LossyQuality;//Use the default or dicom.ini value.

    if(j2kQuality > 100)
    	j2kQuality = 100;

    //Set encoding parameters values.
    opj_set_default_encoder_parameters(&parameters);

    parameters.tcp_mct = 0;
    parameters.decod_format = 1;
    parameters.cod_format = 0;
    parameters.tcp_numlayers = 1;
    parameters.cp_disto_alloc = 1;

    if (j2kQuality < 100)
    {
        parameters.tcp_rates[0] = 100.0f/j2kQuality;
        // Lossy (ICT) or lossless (RCT)?
        parameters.irreversible = 1;//ICT
    }
    else
    	parameters.tcp_rates[0] = 0;


	// Create our input image opj object with our grayscale data in it...
	image = rawtoimage(pData, &parameters, width, height);
	if (NULL == image)
	{
		printf("***[OpenJP2 compress]: Failed to create image object.\n");
		return false;
	}

	// Get a J2K compressor handle.
	l_codec = opj_create_compress(OPJ_CODEC_J2K);

	//Catch events using our callbacks and give a local context.
	opj_set_info_handler(l_codec, ImageConvert::info_callback, 00);
	opj_set_warning_handler(l_codec, ImageConvert::warning_callback, 00);
	opj_set_error_handler(l_codec, ImageConvert::error_callback, 00);

	// Setup the encoder parameters using the current image and user parameters.
	 if (!opj_setup_encoder(l_codec, &parameters, image))
	 {
		printf("failed to encode image: opj_setup_encoder\n");
		opj_destroy_codec(l_codec);
		opj_image_destroy(image);
		return false;
	 }

    // Create our output stream object..
	outputBufferStream.pData = (OPJ_UINT8 *)malloc(width*height);
	outputBufferStream.dataSize = width*height;
	outputBufferStream.offset = 0;

	// Open a byte stream for writing.
	if(!(l_stream = opj_stream_create_default_memory_stream(&outputBufferStream, OPJ_FALSE)))
	{
		printf("***[OpenJP2 compress]: Failed to allocate output stream memory.\n");
		opj_destroy_codec(l_codec);
		opj_image_destroy(image);
		free(outputBufferStream.pData);
		return false;
	}

	// After all of that fanfare, encode the image in one shot
	bSuccess = opj_start_compress(l_codec, image, l_stream);
	if (!bSuccess)
		printf("***[OpenJP2 compress]:OpenJpeg could not start encoding the image.\n");
	else
	{
		bSuccess = opj_encode(l_codec, l_stream);

		if (!bSuccess)
			printf("***[OpenJP2 compress]:OpenJpeg could not encode the image.\n");
		else
		{
			bSuccess = opj_end_compress(l_codec, l_stream);
			if (!bSuccess)
				printf("***[OpenJP2 compress]:OpenJpeg could not end encoding the image.\n");
		}
	}

	//Done with not reusables.
	opj_destroy_codec(l_codec);
	opj_stream_destroy(l_stream);
	opj_image_destroy(image);

	if (!bSuccess)
	{
		free(outputBufferStream.pData);
		return false;
	}
	else
	{
		// Copy it out to our internal data, conversion is complete..
		arOutImage.clear();
		arOutImage.insert(arOutImage.end(), outputBufferStream.pData, outputBufferStream.pData+outputBufferStream.offset);
		free(outputBufferStream.pData);
	}

	return true;
}


#if 0
/* This routine will take in a jpeg2000 image and convert it to little endian, uncompressed,
 *  RGB or grayscale.  It uses the openjpeg library 2.1. You can get it here:
 *  https://github.com/uclouvain/openjpeg
 *  JPEG200 can compress almost anything, with any size for each plane.  So that means any
 *  of the color spaces and formats can be in it.  The dicom standard removes the header
 *  and color box infromation and sends just the data stream.  The standard states only
 *  MONO1,2 PALETTE_COLOR YBR_RCT and YBR_ICT can be use.  For the color space and
 *  image format, I have to trust libopenjpeg.
 *  If I have made some mistakes (most likely) you can contact me bruce.barton
 *  (the mail symbol goes here) me.com.  Let me know where I can find a sample of
 *  the image that didn't work. */
/**
 * Supported codec
 */
//Set decoding parameter format to stream.
#define JP2_RFC3745_MAGIC "\x00\x00\x00\x0c\x6a\x50\x20\x20\x0d\x0a\x87\x0a"
#define JP2_MAGIC "\x0d\x0a\x87\x0a"
#define J2K_CODESTREAM_MAGIC "\xff\x4f\xff\x51"
OPJ_CODEC_FORMAT J2KStreamType(ImageData *imageDataPtr)
{
    if (memcmp((const void *)(imageDataPtr->inPtr), JP2_RFC3745_MAGIC, 12) == 0 ||
        memcmp((const void *)(imageDataPtr->inPtr), JP2_MAGIC, 4) == 0)
        return OPJ_CODEC_JP2;// JPEG 2000 compressed image data

    if (memcmp((const void *)(imageDataPtr->inPtr), J2K_CODESTREAM_MAGIC, 4) == 0)
        return OPJ_CODEC_J2K;//JPEG-2000 codestream
    return OPJ_CODEC_UNKNOWN;
}
bool ImageConvert::ConvertJPEG2KTo8bitGreyscaleTo(uint8_t *pData, uint32_t width, uint32_t height, int j2kQuality, vector<uint8_t>&arOutImage)
{
    ImageData                   imageData;
    clock_t                     starttime;
    BOOL                        moreImages, hasAlpha;
    UINT16			*out16_ptr;
    unsigned int                cnt;
    int		*jpc_out[4], mask;
    int				bytes_jpc, prec_jpc;
    UINT32			stream_len;
#if NATIVE_ENDIAN == BIG_ENDIAN //Big Endian like Apple power pc
    int		masked;
#endif //Big Endian
//Kind of Openjpeg Stuff (shound be)
	opj_memory_stream inputStream;
// OpenJPEG stuff.
    OPJ_CODEC_FORMAT        streamFormat;
	OPJ_BOOL bSuccess;
	opj_dparameters_t	parameters;	// decompression parameters.
    opj_image_t			*decompImage = NULL;
	opj_codec_t*		l_codec = 00;
	opj_stream_t		*l_stream = NULL;
	const char			*version = opj_version();

    IniValue *iniValuePtr = IniValue::GetInstance();

//If debug > 0, get start time.
    starttime = clock();
    if (iniValuePtr->sscscpPtr->DebugLevel > 0) SystemDebug.printf("OpenJP2 decompress started.\n");
// Get the image info.
    if (!imageData.GetImageData(pDDO, "OpenJP2 decompress", TRUE)) return FALSE;
//Set decoding parameters to default values, searching for the stream will set the decode type.
    opj_set_default_decoder_parameters(&parameters);
//Jpeg 2000 is strange, should be just a stream, but could have a header.
    while (TRUE)
    {
        if(!imageData.GetEncapsulatedDDO(NULL)) return FALSE;//Get the data
        //Set decoding parameter format to stream and see if it is real.
        if((streamFormat = J2KStreamType(&imageData)) != OPJ_CODEC_UNKNOWN)break;
    }
    if(!imageData.CreateOutputArray()) return FALSE;
//Init some variables for uninitialized warnings.
    out16_ptr = NULL;
    mask = 0;
    bytes_jpc = 1;
    prec_jpc = 8;
    stream_len = 0;
    hasAlpha =FALSE;
//Start the frames loop.
    moreImages = TRUE;
    while(moreImages)
    {
#ifdef NOVARAD_FIX
        if (((unsigned char *)(imageData.imageVR->Data))[imageData.imageVRLength -2] != 0xFF
            && ((unsigned char *)(imageData.imageVR->Data))[imageData.imageVRLength -1] != 0xD9)
        {
        //Put the end back on.
            if (!imageData.FixJpegEnd()) return FALSE;
        }
#endif
    //Get a decoder handle.
		l_codec = opj_create_decompress(streamFormat);//Bad openjpeg, can't be reused!
    //Catch events using our callbacks and give a local context.
		opj_set_info_handler(l_codec, info_callback, 00);
		opj_set_warning_handler(l_codec, warning_callback, 00);
		opj_set_error_handler(l_codec, error_callback, 00);
    //Setup the decoder decoding parameters using user parameters.
		if (!opj_setup_decoder(l_codec, &parameters))
		{
			opj_destroy_codec(l_codec);
			OperatorConsole.printf("***[OpenJP2 decompress]: failed to setup the decoder\n");
			return (FALSE);
		}
	//Set up the input buffer as a stream
		inputStream.pData = (OPJ_UINT8 *)imageData.inPtr;
		inputStream.dataSize = imageData.inEndPtr - imageData.inPtr;
		inputStream.offset = 0;
	//Open the memory as a stream.
		if (!(l_stream = opj_stream_create_default_memory_stream(&inputStream, OPJ_TRUE)))
		{
			OperatorConsole.printf("***[OpenJP2 compress]: Failed to allocate output stream memory.\n");
			opj_destroy_codec(l_codec);
			return (FALSE);
		}
	// Read the main header of the codestream, if necessary the JP2 boxes, and create decompImage.
		if (!opj_read_header(l_stream, l_codec, &decompImage)) {
			OperatorConsole.printf("***[OpenJP2 decompress]: failed to read the header\n");
			opj_stream_destroy(l_stream);
			opj_destroy_codec(l_codec);
			opj_image_destroy(decompImage);
			return (FALSE);
		}
		//Decode the stream and fill the image structure
		bSuccess = opj_decode(l_codec, l_stream, decompImage);
		if(!bSuccess) OperatorConsole.printf("***[OpenJP2 decompress]: Jpeg 2K code stream did not decode.\n");
		else
		{
			bSuccess = opj_end_decompress(l_codec, l_stream);
			if (!bSuccess) OperatorConsole.printf("***[OpenJP2 decompress]: Jpeg 2K code stream did not end decode.\n");
		}
	//Done with the input stream and the decoder.
		opj_stream_destroy(l_stream);
		opj_destroy_codec(l_codec);
		if (!bSuccess)// Clean up if errored occured.
		{
			opj_image_destroy(decompImage);
			return FALSE;
		}
    //Do this the first time only, big enough for all images.
        if(!imageData.currFrame)
        {//Make the buffer.
        //Check for color.
            switch (decompImage->numcomps)
            {
                case 1://Gray
                    if (imageData.components != 1) {
                        OperatorConsole.printf("***[OpenJP2 decompress]: DICOM jpeg component mishmatch error, DICOM: %d ,J2K: 1 \n",
                                               imageData.components);
                        opj_image_destroy(decompImage);
                        return ( FALSE );
                    }
                    break;
                case 2://GA
                    if (imageData.components > 2) {
                        OperatorConsole.printf("***[OpenJP2 decompress]: DICOM jpeg component mishmatch error, DICOM: %d ,J2K: 2 \n",
                                               imageData.components);
                        opj_image_destroy(decompImage);
                        return ( FALSE );
                    }
                    imageData.newComponents=1;
                    hasAlpha=TRUE;
                    break;
                case 3:
                    if (imageData.components != 3) {
                        OperatorConsole.printf("***[OpenJP2 decompress]: DICOM jpeg component mishmatch error, DICOM: %d ,J2K: 3 \n",
                                               imageData.components);
                        opj_image_destroy(decompImage);
                        return ( FALSE );
                    }
                    break;
                case 4://RGBA
                    if (imageData.components < 3 ) {
                        OperatorConsole.printf("***[OpenJP2 decompress]: DICOM jpeg component mishmatch error, DICOM: %d ,J2K: 4 \n",
                                               imageData.components);
                        opj_image_destroy(decompImage);
                        return ( FALSE );
                    }
                    imageData.newComponents=3;
                    hasAlpha=TRUE;
                    break;
                default:
                    OperatorConsole.printf("***[OpenJP2 decompress]: Strange number of colors, DICOM: %d ,J2K: %d \n",
                                           imageData.components , decompImage->numcomps);
                    opj_image_destroy(decompImage);
                    return ( FALSE );
                break;
            }
        //Get the total uncompressed length.
            prec_jpc = decompImage->comps[0].prec;//Need it at the end
            bytes_jpc = (prec_jpc +7) / 8;// Bytes or words.
            stream_len = (decompImage->comps[0].w * decompImage->comps[0].h);
            if(!imageData.CreateOutputBuffer(stream_len * bytes_jpc * imageData.newComponents * imageData.frames))
            {
                opj_image_destroy(decompImage);
                return ( FALSE );
            }
            out16_ptr = (UINT16 *)imageData.outBuffer;
        //The same for all images
            mask = (1 << prec_jpc) - 1;
        }//End of make the data buffer
    //Cleanup if a ICC profile buffer was made.
        if (decompImage->icc_profile_buf)
        {
#if defined(HAVE_LIBLCMS1) || defined(HAVE_LIBLCMS2)
            color_apply_icc_profile(decompImage);
#endif
            free(decompImage->icc_profile_buf);
            decompImage->icc_profile_buf = NULL;
            decompImage->icc_profile_len = 0;
        }
    //Get the data pointer(s)
        for ( cnt = 0; cnt < imageData.newComponents; cnt++ )
        {
            jpc_out[cnt] = decompImage->comps[cnt].data;
        }
    //Image copy loops, open JPEG outputs ints.
        switch (decompImage->numcomps)
        {
            case 1://Gray
            case 2://GA
                if(bytes_jpc == 2)
                {
                    for(cnt = 0; cnt < stream_len; cnt++)
                    {
#if NATIVE_ENDIAN == BIG_ENDIAN //Big Endian like Apple power pc
                        masked = (UINT16)(*jpc_out[0]++ & mask);
                        *(imageData.outPtr)++ = (unsigned char)(masked & 0xFF);
                        *(imageData.outPtr)++ = (unsigned char)((masked >> 8) & 0xFF);
#else //Little Endian
                        *out16_ptr++ = (UINT16)(*jpc_out[0]++ & mask);
#endif //Big Endian
                    }
                    break;
                }
            //bytes_jpc == 1
                for(cnt = 0; cnt < stream_len; cnt++)
                    *(imageData.outPtr)++ = (unsigned char)(*jpc_out[0]++ & mask);
                break;
            case 3://RGB
            case 4:
                for(cnt = 0; cnt < stream_len; cnt++)
                {
                    *(imageData.outPtr)++ = (unsigned char)(*jpc_out[0]++ & mask);
                    *(imageData.outPtr)++ = (unsigned char)(*jpc_out[1]++ & mask);
                    *(imageData.outPtr)++ = (unsigned char)(*jpc_out[2]++ & mask);
                }
                break;
            default:// Caught above, just for warnings.
                return  ( FALSE );
                break;
        }
    //Done with libopenjpeg for this loop.
        opj_image_destroy(decompImage);
    //Done with all of the frames.
        imageData.currFrame++;
        if(imageData.currFrame >= imageData.frames)break;
    //More images to read.
        while (TRUE)
        {
            if(!imageData.GetEncapsulatedDDO(NULL))
            {//Not good, can't find the next image!
                moreImages = FALSE;//Break the image loop.
                break;
            }
            if((streamFormat = J2KStreamType(&imageData)) != OPJ_CODEC_UNKNOWN)break;
        }
    // Loop back to open the memory as a stream.
    }//End of the frames loop
//Should we kill it and keep the uncompressed data?
    imageData.CheckEnoughFrames();
//Change the image vr to the bigger uncompressed and unencapsulated image.
    imageData.SwitchToOutputBuffer();
//Kill the alpha channel.
    if(hasAlpha==TRUE)imageData.ChangeComponents();
//The color stuff.
    if( imageData.newComponents > 1) imageData.SetColorToRGB(FALSE);
//Change the transfer syntax to LittleEndianExplict!
    pDDO->ChangeVR( 0x0002, 0x0010, "1.2.840.10008.1.2.1\0", TYPE_IU);
//If debug > 0, print decompress time.
    if (iniValuePtr->sscscpPtr->DebugLevel > 0) SystemDebug.printf("OpenJP2 %s decompress time %u milliseconds.\n",
                                           version, (clock() - starttime)/1000);
    return (TRUE);
}
#endif



// HELPERS

// Prepare 8 bit grayscale for input into J2K Encoder...
opj_image_t* ImageConvert::rawtoimage(uint8_t *pData, opj_cparameters_t *parameters, uint32_t width, uint32_t height)
{
    int subsampling_dx = parameters->subsampling_dx;
    int subsampling_dy = parameters->subsampling_dy;
    int i;

    opj_image_cmptparm_t cmptparm;
    opj_image_t * image = NULL;


    // Just one layer...
    memset(&cmptparm, 0x00, sizeof(opj_image_cmptparm_t));

	cmptparm.prec = (OPJ_UINT32)8;
	cmptparm.bpp = (OPJ_UINT32)8;
	cmptparm.sgnd = (OPJ_UINT32)0;
	cmptparm.dx = (OPJ_UINT32)subsampling_dx;
	cmptparm.dy = (OPJ_UINT32)subsampling_dy;
	cmptparm.w = (OPJ_UINT32)width;
	cmptparm.h = (OPJ_UINT32)height;

    /* create the image */
    image = opj_image_create((OPJ_UINT32)1, &cmptparm, OPJ_CLRSPC_GRAY);

    if (!image)
        return NULL;


    /* set image offset and reference grid */
    image->x0 = (OPJ_UINT32)parameters->image_offset_x0;
    image->y0 = (OPJ_UINT32)parameters->image_offset_y0;
    image->x1 = (OPJ_UINT32)parameters->image_offset_x0 + (OPJ_UINT32)(width - 1) * (OPJ_UINT32)subsampling_dx + 1;
    image->y1 = (OPJ_UINT32)parameters->image_offset_y0 + (OPJ_UINT32)(height - 1) * (OPJ_UINT32)subsampling_dy + 1;

	//Copy the image data...
	// The image data array is a 32bit value... we must copy eacy byte one by one...
	for (i = 0; i < width * height; i++)
		image->comps[0].data[i] = *(pData+i);


    return image;
}

/**
sample error debug callback expecting no client object
*/
void ImageConvert::error_callback(const char *msg, void *client_data)
{
    (void)client_data;
    fprintf(stdout, "[ERROR] %s", msg);
}
/**
sample warning debug callback expecting no client object
*/
void ImageConvert::warning_callback(const char *msg, void *client_data)
{
    (void)client_data;
    fprintf(stdout, "[WARNING] %s", msg);
}
/**
sample debug callback expecting no client object
*/
void ImageConvert::info_callback(const char *msg, void *client_data)
{
    (void)client_data;
    fprintf(stdout, "[INFO] %s", msg);
}


//This will read from our memory to the buffer.
OPJ_SIZE_T ImageConvert::opj_memory_stream_read(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data)

{
	opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;//Our data.

	OPJ_SIZE_T l_nb_bytes_read = p_nb_bytes;//Amount to move to buffer.

	//Check if the current offset is outside our data buffer.

	if (l_memory_stream->offset >= l_memory_stream->dataSize) return (OPJ_SIZE_T)-1;
	//Check if we are reading more than we have.
	if (p_nb_bytes > (l_memory_stream->dataSize - l_memory_stream->offset))
		l_nb_bytes_read = l_memory_stream->dataSize - l_memory_stream->offset;//Read all we have.

	//Copy the data to the internal buffer.
	memcpy(p_buffer, &(l_memory_stream->pData[l_memory_stream->offset]), l_nb_bytes_read);

	l_memory_stream->offset += l_nb_bytes_read;//Update the pointer to the new location.

	return l_nb_bytes_read;
}

//This will write from the buffer to our memory.
OPJ_SIZE_T ImageConvert::opj_memory_stream_write(void * p_buffer, OPJ_SIZE_T p_nb_bytes, void * p_user_data)

{
	opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;//Our data.

	OPJ_SIZE_T l_nb_bytes_write = p_nb_bytes;//Amount to move to buffer.

	//Check if the current offset is outside our data buffer.

	if (l_memory_stream->offset >= l_memory_stream->dataSize) return (OPJ_SIZE_T)-1;

	//Check if we are write more than we have space for.
	if (p_nb_bytes > (l_memory_stream->dataSize - l_memory_stream->offset))
		l_nb_bytes_write = l_memory_stream->dataSize - l_memory_stream->offset;//Write the remaining space.

	//Copy the data from the internal buffer.
	memcpy(&(l_memory_stream->pData[l_memory_stream->offset]), p_buffer, l_nb_bytes_write);

	l_memory_stream->offset += l_nb_bytes_write;//Update the pointer to the new location.

	return l_nb_bytes_write;
}

//Moves the pointer forward, but never more than we have.
OPJ_OFF_T ImageConvert::opj_memory_stream_skip(OPJ_OFF_T p_nb_bytes, void * p_user_data)

{
	opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;

	OPJ_SIZE_T l_nb_bytes;


	if (p_nb_bytes < 0) return -1;//No skipping backwards.

	l_nb_bytes = (OPJ_SIZE_T)p_nb_bytes;//Allowed because it is positive.

	// Do not allow jumping past the end.

	if (l_nb_bytes >l_memory_stream->dataSize - l_memory_stream->offset)
		l_nb_bytes = l_memory_stream->dataSize - l_memory_stream->offset;//Jump the max.

	//Make the jump.
	l_memory_stream->offset += l_nb_bytes;

	//Returm how far we jumped.
	return l_nb_bytes;
}

//Sets the pointer to anywhere in the memory.
OPJ_BOOL ImageConvert::opj_memory_stream_seek(OPJ_OFF_T p_nb_bytes, void * p_user_data)
{
	opj_memory_stream* l_memory_stream = (opj_memory_stream*)p_user_data;

	if (p_nb_bytes < 0) return OPJ_FALSE;//No before the buffer.

	if (p_nb_bytes >(OPJ_OFF_T)l_memory_stream->dataSize) return OPJ_FALSE;//No after the buffer.

	l_memory_stream->offset = (OPJ_SIZE_T)p_nb_bytes;//Move to new position.

	return OPJ_TRUE;
}

//The system needs a routine to do when finished, the name tells you what I want it to do.
void ImageConvert::opj_memory_stream_do_nothing(void * p_user_data)
{
	OPJ_ARG_NOT_USED(p_user_data);
}

//Create a stream to use memory as the input or output.
opj_stream_t* ImageConvert::opj_stream_create_default_memory_stream(opj_memory_stream* p_memoryStream, OPJ_BOOL p_is_read_stream)
{
	opj_stream_t* l_stream;

	if (!(l_stream = opj_stream_default_create(p_is_read_stream))) return (NULL);
	//Set how to work with the frame buffer.
	if(p_is_read_stream)
		opj_stream_set_read_function(l_stream, ImageConvert::opj_memory_stream_read);
	else
		opj_stream_set_write_function(l_stream, ImageConvert::opj_memory_stream_write);

	opj_stream_set_seek_function(l_stream, ImageConvert::opj_memory_stream_seek);
	opj_stream_set_skip_function(l_stream, ImageConvert::opj_memory_stream_skip);
	opj_stream_set_user_data(l_stream, p_memoryStream, ImageConvert::opj_memory_stream_do_nothing);

	opj_stream_set_user_data_length(l_stream, p_memoryStream->dataSize);

	return l_stream;
}

