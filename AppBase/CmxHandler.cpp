/*
 * CmxHandler.cpp
 *
 *  Created on: 18 Aug, 2009
 *      Author: akhil
 */

#include "CmxHandler.h"
#include "Configuration.h"
//#include "MatchProcessor.h"
#include "socket.h"
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <iostream>
#include "MessageExt.h"
#include <unistd.h>
//#include "DBReceive.h"
//#include "LEDConsolidator.h"
//#include "F2FDispatcher.h"
//#include "NwMatchManager.h"
//#include "Synchronization.h"
#include "SocketFactory.h"
#include "logging.h"
//#include "MT9P001FrameGrabber.h"
//#include "ImageProcessor.h"
//#include "MatchManagerInterface.h"

using namespace std;

extern "C" {
#include "file_manip.h"
#include "include/BobListener.h"
}
#include "NetworkUtilities.h"
//#include "NwMatcherSerialzer.h"
//#include "EyeDispatcher.h"

const char logger[30] = "CmxHandler";

CMXMESSAGETYPE Prev_mesg;
#define BUFLEN 1500
#define min(a,b)((a<b)?(a):(b))

enum PIM_BoardRGBLed{
	Black = 0,
	Blue = 1,
	Red = 2,
	Purple =3,
	Green = 4,
	Cyan = 5,
	yellow = 6,
	white = 7,
};

#ifdef HBOX_PG
#if 0
unsigned long SwapEndian(unsigned long input)
{
	unsigned char *ptr = ((unsigned char *)&input);
	std::swap(ptr[0], ptr[3]);
	std::swap(ptr[1], ptr[2]);
	return input;
}
#else
unsigned long SwapEndian(unsigned long input)
{
	unsigned long value = input;
	unsigned char *ptr = ((unsigned char *)&value);
	std::swap(ptr[0], ptr[3]);
	std::swap(ptr[1], ptr[2]);
	for(int i = 0; i < 4; i++)
	{
		unsigned char b = ptr[i];
		ptr[i] = (b * 0x0202020202ULL & 0x010884422010ULL) % 1023;
	}
	return value;
}
#endif

IplImage* CmxHandler::ConvertImageToOpenCV(FlyCapture2::Image* pImage)
{
    IplImage* cvImage = NULL;
    FlyCapture2::Image colorImage;
    bool bColor = true;
    CvSize mySize;
    mySize.height = pImage->GetRows();
    mySize.width = pImage->GetCols();
    bool bInitialized = false;

    switch ( pImage->GetPixelFormat() )
    {
        case PIXEL_FORMAT_MONO8:     cvImage = cvCreateImageHeader(mySize, 8, 1 );
                                     cvImage->depth = IPL_DEPTH_8U;
                                     cvImage->nChannels = 1;
                                     bColor = false;
                                     break;
        case PIXEL_FORMAT_411YUV8:   cvImage = cvCreateImageHeader(mySize, 8, 3 );
                                     cvImage->depth = IPL_DEPTH_8U;
                                     cvImage->nChannels = 3;
                                     break;
        case PIXEL_FORMAT_422YUV8:   cvImage = cvCreateImageHeader(mySize, 8, 3 );
                                     cvImage->depth = IPL_DEPTH_8U;
                                     cvImage->nChannels = 3;
                                     break;
        case PIXEL_FORMAT_444YUV8:   cvImage = cvCreateImageHeader(mySize, 8, 3 );
                                     cvImage->depth = IPL_DEPTH_8U;
                                     cvImage->nChannels = 3;
                                     break;
        case PIXEL_FORMAT_RGB8:      cvImage = cvCreateImageHeader(mySize, 8, 3 );
                                     cvImage->depth = IPL_DEPTH_8U;
                                     cvImage->nChannels = 3;
                                     break;
        case PIXEL_FORMAT_MONO16:    cvImage = cvCreateImageHeader(mySize, 16, 1 );
                                     cvImage->depth = IPL_DEPTH_16U;
                                     cvImage->nChannels = 1;
                                     bColor = false;
                                     break;
        case PIXEL_FORMAT_RGB16:     cvImage = cvCreateImageHeader(mySize, 16, 3 );
                                     cvImage->depth = IPL_DEPTH_16U;
                                     cvImage->nChannels = 3;
                                     break;
        case PIXEL_FORMAT_S_MONO16:  cvImage = cvCreateImageHeader(mySize, 16, 1 );
                                     cvImage->depth = IPL_DEPTH_16U;
                                     cvImage->nChannels = 1;
                                     bColor = false;
                                     break;
        case PIXEL_FORMAT_S_RGB16:   cvImage = cvCreateImageHeader(mySize, 16, 3 );
                                     cvImage->depth = IPL_DEPTH_16U;
                                     cvImage->nChannels = 3;
                                     break;
        case PIXEL_FORMAT_RAW8:      cvImage = cvCreateImageHeader(mySize, 8, 3 );
                                     cvImage->depth = IPL_DEPTH_8U;
                                     cvImage->nChannels = 3;
                                     break;
        case PIXEL_FORMAT_RAW16:     cvImage = cvCreateImageHeader(mySize, 8, 3 );
                                     cvImage->depth = IPL_DEPTH_8U;
                                     cvImage->nChannels = 3;
                                     break;
        case PIXEL_FORMAT_MONO12:    printf("Not supported by OpenCV");
                                     bColor = false;
                                     break;
        case PIXEL_FORMAT_RAW12:     printf("Not supported by OpenCV");
                                     break;
        case PIXEL_FORMAT_BGR:       cvImage = cvCreateImageHeader(mySize, 8, 3 );
                                     cvImage->depth = IPL_DEPTH_8U;
                                     cvImage->nChannels = 3;
                                     break;
        case PIXEL_FORMAT_BGRU:      cvImage = cvCreateImageHeader(mySize, 8, 4 );
                                     cvImage->depth = IPL_DEPTH_8U;
                                     cvImage->nChannels = 4;
                                     break;
        case PIXEL_FORMAT_RGBU:      cvImage = cvCreateImageHeader(mySize, 8, 4 );
                                     cvImage->depth = IPL_DEPTH_8U;
                                     cvImage->nChannels = 4;
                                     break;
        default: printf("Some error occurred...\n");
                 return NULL;
    }

    if(bColor) {
        if(!bInitialized)
        {
        	colorImage.SetData(new unsigned char[pImage->GetCols() * pImage->GetRows()*3], pImage->GetCols() * pImage->GetRows()*3);
            bInitialized = true;
        }

        pImage->Convert(PIXEL_FORMAT_BGR, &colorImage); //needs to be as BGR to be saved

        cvImage->width = colorImage.GetCols();
        cvImage->height = colorImage.GetRows();
        cvImage->widthStep = colorImage.GetStride();

        cvImage->origin = 0; //interleaved color channels

        cvImage->imageDataOrigin = (char*)colorImage.GetData(); //DataOrigin and Data same pointer, no ROI
        cvImage->imageData         = (char*)(colorImage.GetData());
        cvImage->widthStep      = colorImage.GetStride();
        cvImage->nSize = sizeof (IplImage);
        cvImage->imageSize = cvImage->height * cvImage->widthStep;
    }
    else
    {
        cvImage->imageDataOrigin = (char*)(pImage->GetData());
        cvImage->imageData         = (char*)(pImage->GetData());
        cvImage->widthStep         = pImage->GetStride();
        cvImage->nSize             = sizeof (IplImage);
        cvImage->imageSize         = cvImage->height * cvImage->widthStep;

        //at this point cvImage contains a valid IplImage
     }
    return cvImage;
}

void CmxHandler::PrintError(Error error)
{
	error.PrintErrorTrace();
}

void CmxHandler::PrintCameraInfo(CameraInfo *pCamInfo)
{
#if 0
    cout << endl;
   // cout << "*** CAMERA INFORMATION ***" << endl;
    cout << "Serial number - " << pCamInfo->serialNumber << endl;
    cout << "Camera model - " << pCamInfo->modelName << endl;

    cout << "Camera vendor - " << pCamInfo->vendorName << endl;
    cout << "Sensor - " << pCamInfo->sensorInfo << endl;
    cout << "Resolution - " << pCamInfo->sensorResolution << endl;
    cout << "Firmware version - " << pCamInfo->firmwareVersion << endl;
    cout << "Firmware build time - " << pCamInfo->firmwareBuildTime << endl
         << endl;
#endif
}

void CmxHandler::PrintFormat7Capabilities(Format7Info fmt7Info)
{
    cout << "Max image pixels: (" << fmt7Info.maxWidth << ", "
         << fmt7Info.maxHeight << ")" << endl;
    cout << "Image Unit size: (" << fmt7Info.imageHStepSize << ", "
         << fmt7Info.imageVStepSize << ")" << endl;
    cout << "Offset Unit size: (" << fmt7Info.offsetHStepSize << ", "
         << fmt7Info.offsetVStepSize << ")" << endl;
    cout << "Pixel format bit field: 0x" << fmt7Info.pixelFormatBitField << endl;
}

bool CmxHandler::CheckSoftwareTriggerPresence(FlyCapture2::Camera *pCam)
{
    const unsigned int k_triggerInq = 0x530;

    Error error;
    unsigned int regVal = 0;

    error = pCam->ReadRegister(k_triggerInq, &regVal);

    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return false;
    }

    if ((regVal & 0x10000) != 0x10000)
    {
        return false;
    }

    return true;
}

bool CmxHandler::PollForTriggerReady(FlyCapture2::Camera *pCam)
{
    const unsigned int k_softwareTrigger = 0x62C;
    Error error;
    unsigned int regVal = 0;

    do
    {
        error = pCam->ReadRegister(k_softwareTrigger, &regVal);
        if (error != PGRERROR_OK)
        {
            PrintError(error);
            return false;
        }

    } while ((regVal >> 31) != 0);

    return true;
}

bool CmxHandler::FireSoftwareTrigger(FlyCapture2::Camera *pCam)
{
    const unsigned int k_softwareTrigger = 0x62C;
    const unsigned int k_fireVal = 0x80000000;
    Error error;

    error = pCam->WriteRegister(k_softwareTrigger, k_fireVal);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return false;
    }

    return true;
}

IplImage* CmxHandler::transposeImage1(IplImage* image, int angle) {

    IplImage *rotated = cvCreateImage(cvSize(image->width,image->height),
        IPL_DEPTH_8U,image->nChannels);
    CvPoint2D32f center;
    float center_val = (float)((image->width)-1) / 2;
    center.x = (float)(image->width)/2;
    center.y = (float)(image->height)/2;
    CvMat *mapMatrix = cvCreateMat( 2, 3, CV_32FC1 );
    cv2DRotationMatrix(center, angle, 1.0, mapMatrix);
    cvWarpAffine(image, rotated, mapMatrix);
    cvReleaseMat(&mapMatrix);
	return rotated;
}

IplImage* RotateImage(IplImage* image, int angle) {

    IplImage *rotated = cvCreateImage(cvSize(image->width,image->height),
        IPL_DEPTH_8U,image->nChannels);
    CvPoint2D32f center;
    float center_val = (float)((image->width)-1) / 2;
    center.x = (float)(image->width)/2;
    center.y = (float)(image->height)/2;
    CvMat *mapMatrix = cvCreateMat( 2, 3, CV_32FC1 );
    cv2DRotationMatrix(center, angle, 1.0, mapMatrix);
    cvWarpAffine(image, rotated, mapMatrix);
    cvReleaseMat(&mapMatrix);
	return rotated;
}


void PrintErrorCamInit(Error error)
{
	error.PrintErrorTrace();
}

int CmxHandler::CameraDisconnect(int CameraIndex)
{

	FlyCapture2::Camera camera;
	FlyCapture2::Error error;
	FlyCapture2::BusManager busMgr;
	PGRGuid guid;
	error = busMgr.GetCameraFromIndex(CameraIndex, &guid);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	}

	// Stop capturing images
	error = camera.StopCapture();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}

	// Disconnect the camera
	error = camera.Disconnect();
	if (error != PGRERROR_OK)
	{
		PrintError(error);
		return -1;
	}
	return 0;
}

void CameraInitialize(int CameraIndex)
{
	FlyCapture2::Error error;
	FlyCapture2::Camera camera;
	FlyCapture2::BusManager busMgr;
	PGRGuid guid;
	error = busMgr.GetCameraFromIndex(CameraIndex, &guid);
	if (error != PGRERROR_OK)
	{
		PrintErrorCamInit(error);
	}
	// Connect the camera
	error = camera.Connect(&guid);
	if ( error != PGRERROR_OK )
	{
		PrintErrorCamInit(error);
	    std::cout << "Failed to connect to camera" << std::endl;
	}

	error = camera.StartCapture();
	if ( error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED )
	{
		PrintErrorCamInit(error);
	}
	else if ( error != PGRERROR_OK )
	{
		PrintErrorCamInit(error);
	    std::cout << "Failed to start image capture" << std::endl;
	}

	printf("CameraInitialise Successful for camera ID..%d\n", CameraIndex);
}
#if 0
IplImage* CmxHandler::GrabFramePG(int CameraIndex)
{
	FlyCapture2::Error error;
	FlyCapture2::Camera camera;
	FlyCapture2::BusManager busMgr;
	FlyCapture2::Image rawImage;

	PGRGuid guid;
		error = busMgr.GetCameraFromIndex(CameraIndex, &guid);
		if (error != PGRERROR_OK)
		{
			PrintErrorCamInit(error);
		}
		// Connect the camera
		error = camera.Connect(&guid);
		if ( error != PGRERROR_OK )
		{
			PrintErrorCamInit(error);
		    std::cout << "Failed to connect to camera" << std::endl;
		}

		error = camera.StartCapture();
		if ( error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED )
		{
			PrintErrorCamInit(error);
		}
		else if ( error != PGRERROR_OK )
		{
			PrintErrorCamInit(error);
		    std::cout << "Failed to start image capture" << std::endl;
		}

		printf("CameraInitialise Successful for camera ID..%d\n", CameraIndex);


	error = camera.RetrieveBuffer( &rawImage );
	if ( error != PGRERROR_OK )
	{
		std::cout << "capture error" << std::endl;
	}

	input_image = ConvertImageToOpenCV(&rawImage);

   	if(m_RotateInImage)
   	{
   		IplImage *m_warppedeyeCrop = transposeImage1(input_image, 90);
   		cvSaveImage("Test.pgm", m_warppedeyeCrop);
   		return(m_warppedeyeCrop);
   	}else
   	{
   		IplImage *dst = cvCreateImage(cvSize(m_width, m_height),IPL_DEPTH_8U,1);
   		cvCopyImage(input_image, dst);
   		cvSaveImage("Test_DST.pgm", dst);
   		return(dst);
   	}
}
#endif
#if 0 //working
 // New one
IplImage* CmxHandler::GrabFramePG(int CameraIndex)
{
	FlyCapture2::Error error;
    FlyCapture2::Camera camera;
    FlyCapture2::CameraInfo camInfo;
    FlyCapture2::BusManager busMgr;
    unsigned int numCameras;

//    error = busMgr.GetNumOfCameras(&numCameras);
//    	if (error != PGRERROR_OK)
//    	{
//    		PrintError(error);
//    	    //return -1;
//    	}
//
//    	// cout << "Number of cameras detected: " << numCameras << endl;
//
//    	if (numCameras < 1)
//    	{
//    		cout << "Insufficient number of cameras... exiting" << endl;
//    	    //return -1;
//    	}

    	PGRGuid guid;
    	error = busMgr.GetCameraFromIndex(CameraIndex, &guid);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    //return -1;
    	}

    // Connect the camera
    error = camera.Connect(&guid);
    if ( error != PGRERROR_OK )
    {
    	PrintError(error);
        std::cout << "Failed to connect to camera" << std::endl;
    }

//    // Get the camera info and print it out
//    error = camera.GetCameraInfo( &camInfo );
//    if ( error != PGRERROR_OK )
//    {
//    	PrintError(error);
//        std::cout << "Failed to get camera info from camera" << std::endl;
//    }
//
//	PrintCameraInfo(&camInfo);

	error = camera.StartCapture();
    if ( error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED )
    {
    	PrintError(error);
        std::cout << "Bandwidth exceeded" << std::endl;
        // return false;
    }
    else if ( error != PGRERROR_OK )
    {
    	PrintError(error);
        std::cout << "Failed to start image capture" << std::endl;
        // return false;
    }

    static int ImgCnt;
    // Get the image
    FlyCapture2::Image rawImage;
    error = camera.RetrieveBuffer( &rawImage );
    if ( error != PGRERROR_OK )
    {
    	std::cout << "capture error" << std::endl;
    }

    input_image = ConvertImageToOpenCV(&rawImage);
    	// sprintf(filename,"InputImage%d.pgm",ImgCnt++);
    	// cvSaveImage(filename, input_image);
    	if(m_RotateInImage)
    	{
    		IplImage *m_warppedeyeCrop = transposeImage1(input_image, 90);
    		cvSaveImage("Test.pgm", m_warppedeyeCrop);
    		// cvFlip(m_warppedeyeCrop, m_warppedeyeCrop, 1);
    		// cvSaveImage("Flip.pgm", m_warppedeyeCrop);
    		return(m_warppedeyeCrop);
    		//IplImage *m_rotatedImage = cvCreateImage(cvSize(m_height, m_width),IPL_DEPTH_8U,1);
    		//IplImage *eyemaskImage = cvLoadImage("PG.pgm", CV_LOAD_IMAGE_UNCHANGED);
    		//cvCopyImage(eyemaskImage, input_image);
    		//cvTranspose(input_image, m_rotatedImage);
    		//cvFlip(m_rotatedImage, m_rotatedImage, -1);
    		//return(m_rotatedImage);
    	}else
    	{
    		IplImage *dst = cvCreateImage(cvSize(m_width, m_height),IPL_DEPTH_8U,1);
    		cvCopyImage(input_image, dst);
    		return(dst);
    	}

}
#endif

#if 1

int CmxHandler::CameraPowerOn(int CameraSerialNo)
{
	FlyCapture2::Error error;
	FlyCapture2::Camera camera;
	FlyCapture2::CameraInfo camInfo;
	FlyCapture2::BusManager busMgr;
	unsigned int numCameras;
	const Mode fmt7Mode = MODE_0;
	const PixelFormat k_fmt7PixFmt = PIXEL_FORMAT_MONO8;
	bool m_AutomaticGain = false;
	bool m_AutomaticShutter = false;

	PGRGuid guid;
	error = busMgr.GetCameraFromSerialNumber(CameraSerialNo, &guid);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	    return -1;
	}

	// Connect the camera
	error = camera.Connect(&guid);
	if ( error != PGRERROR_OK )
	{
	    std::cout << "Failed to connect to camera" << std::endl;
	    return -1;
	}


    // Power on the camera
    const unsigned int k_cameraPower = 0x610;
    const unsigned int k_powerVal = 0x80000000;
    error = camera.WriteRegister(k_cameraPower, k_powerVal);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    const unsigned int millisecondsToSleep = 100;
    unsigned int regVal = 0;
    unsigned int retries = 10;

    // Wait for camera to complete power-up
    do
    {
        struct timespec nsDelay;
        nsDelay.tv_sec = 0;
        nsDelay.tv_nsec = (long)millisecondsToSleep * 1000000L;
        nanosleep(&nsDelay, NULL);

        error = camera.ReadRegister(k_cameraPower, &regVal);
        if (error == PGRERROR_TIMEOUT)
        {
            // ignore timeout errors, camera may not be responding to
            // register reads during power-up
        }
        else if (error != PGRERROR_OK)
        {
            PrintError(error);
            return -1;
        }

        retries--;
    } while ((regVal & k_powerVal) == 0 && retries > 0);

    // Check for timeout errors after retrying
    if (error == PGRERROR_TIMEOUT)
    {
        PrintError(error);
        return -1;
    }

    	// Power-up the camera (for cameras that support this feature)
    	//

    	//error = flycaptureSetCameraRegister( context, CAMERA_POWER, 0x80000000 );
    	//_HANDLE_ERROR( error, "flycaptureSetCameraRegister()" );

    	// Configure trigger pin as input and all others as output (1 == output/strobe, 0 == input)
    	unsigned int value1 = 0, value2 = 0;
    	camera.ReadRegister(PIO_DIRECTION, &value1);
    	camera.ReadRegister(TRIGGER_MODE, &value2);

    	//flycaptureGetCameraRegister( context, PIO_DIRECTION, &value1);
    	//flycaptureGetCameraRegister( context, TRIGGER_MODE, &value2);

    	unsigned int value = 0;
    	value |= ((1 << m_GPIOTriggerPin) ^ 0xf);
    	value = SwapEndian(value);
    	camera.WriteRegister(PIO_DIRECTION, value);

    	//flycaptureSetCameraRegister( context, PIO_DIRECTION, value);

    	// bits:
    	// 0 = trigger mode availability
    	// 6 = enable trigger
    	// 7 = default polarity of pin is active (low = 0, high = 1)
    	// [8..10] = decimal for trigger pin index
    	value = 0;
    	camera.ReadRegister(TRIGGER_MODE, &value);

    	//flycaptureGetCameraRegister( context, TRIGGER_MODE, &value);
    	value = SwapEndian(value);
    	value |= (1 << 6); // set enable trigger bit (6) to true
    	value |= (1 << 7); // set polarity bit (7) to active high
    	value &= ~(0x7 << 8); // set bits 8-10 to 0
    	value |= (m_GPIOTriggerPin << 8); // then take or of pin index shifted to 8...10
    	value = SwapEndian(value);
    	camera.WriteRegister( TRIGGER_MODE, value); // Configure camera to use GPIO[0-3] as external trigger

    //trigger settings:  trigger active state, polarity, state, source, mode
    FlyCapture2::TriggerMode triggerMode;
    triggerMode.onOff = true; //(m_Trigger >= 0);
    triggerMode.polarity = m_PulsePolarity;
    triggerMode.source = 0;
    triggerMode.mode = m_PointGreyMode;
    triggerMode.parameter = 0;
    error = camera.SetTriggerMode(&triggerMode, false);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

	//setting this shutter value only effects the exposure when running with trigger mode disabled.
	FlyCapture2::Property prop;
	prop.type = FlyCapture2::SHUTTER;
	prop.valueA = m_Shutter;
	prop.autoManualMode = m_AutomaticShutter;
	camera.SetProperty(&prop, false);

	//SetExposure(m_Shutter, m_AutomaticShutter); // ignored because of trigger mode
	prop.type = FlyCapture2::GAIN;
	//prop.valueA = m_Gain;
	prop.valueA = (unsigned int)m_Gain;
	prop.autoManualMode = m_AutomaticGain;
	camera.SetProperty(&prop, false);
	//SetGain(m_Gain, m_AutomaticGain);
	prop.type = FlyCapture2::BRIGHTNESS;
	prop.valueA = m_Brightness;
	camera.SetProperty(&prop);

    // Poll to ensure camera is ready
    bool retVal = PollForTriggerReady(&camera);
    if (!retVal)
    {
        cout << endl;
        cout << "Error polling for trigger ready!" << endl;
        return -1;
    }

    // Get the camera configuration
    FC2Config config;
    error = camera.GetConfiguration(&config);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Set the grab timeout to 5 seconds
    config.grabTimeout = 5000;

    // Set the camera configuration
    error = camera.SetConfiguration(&config);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Query for available Format 7 modes
    	Format7Info fmt7Info;
    	bool supported;
    	fmt7Info.mode = fmt7Mode;
    	error = camera.GetFormat7Info(&fmt7Info, &supported);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}

    	// PrintFormat7Capabilities(fmt7Info);
    	if ((k_fmt7PixFmt & fmt7Info.pixelFormatBitField) == 0)
    	{
    		// Pixel format not supported!
    	    cout << "Pixel format is not supported" << endl;
    	    // return -1;
    	}

    	Format7ImageSettings fmt7ImageSettings;
    	fmt7ImageSettings.mode = fmt7Mode;
    	fmt7ImageSettings.offsetX = 0;
    	fmt7ImageSettings.offsetY = 0;
    	fmt7ImageSettings.width = m_width;
    	fmt7ImageSettings.height = m_height;
    	fmt7ImageSettings.pixelFormat = k_fmt7PixFmt;
        bool valid;
        Format7PacketInfo fmt7PacketInfo;

        // Validate the settings to make sure that they are valid
    	error = camera.ValidateFormat7Settings(&fmt7ImageSettings, &valid, &fmt7PacketInfo);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}

    	if (!valid)
    	{
    		// Settings are not valid
    	    cout << "Format7 settings are not valid" << endl;
    	    // return -1;
    	}

    	// Set the settings to the camera
    	error = camera.SetFormat7Configuration(&fmt7ImageSettings, fmt7PacketInfo.recommendedBytesPerPacket);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}
#ifdef SOFTWARE_TRIGGER_CAMERA
    if (!CheckSoftwareTriggerPresence(&camera))
    {
        cout << "SOFT_ASYNC_TRIGGER not implemented on this camera! Stopping "
                "application"
             << endl;
        return -1;
    }
#else
    cout << "Trigger the camera by sending a trigger pulse to GPIO"
         << triggerMode.source << endl;

#endif


#ifdef SOFTWARE_TRIGGER_CAMERA
        // Check that the trigger is ready
        PollForTriggerReady(&camera);

        // Fire software trigger
        retVal = FireSoftwareTrigger(&camera);
        if (!retVal)
        {
            cout << endl;
            cout << "Error firing software trigger" << endl;
            //return -1;
        }
#endif
}


IplImage* CmxHandler::GrabFramePG(int CameraSerialNo)
{
	FlyCapture2::Error error;
	FlyCapture2::Camera camera;
	FlyCapture2::CameraInfo camInfo;
	FlyCapture2::BusManager busMgr;

	if(m_bStatus)
	{
		PGRGuid guid;
		error = busMgr.GetCameraFromSerialNumber(CameraSerialNo, &guid);
		if (error != PGRERROR_OK)
		{
			PrintError(error);
			//return -1;
		}

		// Connect the camera
		error = camera.Connect(&guid);
		if ( error != PGRERROR_OK )
		{
			std::cout << "Failed to connect to camera" << std::endl;
			//return false;
		}

		// Camera is ready, start capturing images
		error = camera.StartCapture();
		if ( error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED )
		{
			std::cout << "Bandwidth exceeded" << std::endl;
			//return false;
		}
		else if ( error != PGRERROR_OK )
		{
			std::cout << "Failed to start image capture" << std::endl;
			//return false;
		}

		// capture loop
		FlyCapture2::Image rawImage;
		error = camera.RetrieveBuffer( &rawImage );
		if ( error != PGRERROR_OK )
		{
			std::cout << "capture error" << std::endl;
		}

		input_image = ConvertImageToOpenCV(&rawImage);
		if(m_RotateInImage)
		{
			IplImage *m_warppedeyeCrop = transposeImage1(input_image, 90);
			return(m_warppedeyeCrop);
		}else
		{
			IplImage *dst = cvCreateImage(cvSize(m_width, m_height),IPL_DEPTH_8U,1);
			cvCopyImage(input_image, dst);
			return(dst);
		}
	}else
	{
		IplImage *empty = cvCreateImage(cvSize(m_width, m_height),IPL_DEPTH_8U,1);
		return(empty);
	}
}


IplImage* CmxHandler::GrabFramePGCAM0(int CameraIndex)
{
	FlyCapture2::Error error;
	FlyCapture2::Camera camera;
	FlyCapture2::CameraInfo camInfo;
	FlyCapture2::BusManager busMgr;
	unsigned int numCameras;
	const Mode fmt7Mode = MODE_0;
	const PixelFormat k_fmt7PixFmt = PIXEL_FORMAT_MONO8;
	bool m_AutomaticGain = false;
	bool m_AutomaticShutter = false;

	// printf("GrabFramePG  width...%d height...%d\n", m_width, m_height);
#if 0
	error = busMgr.GetNumOfCameras(&numCameras);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	    //return -1;
	}

	// cout << "Number of cameras detected: " << numCameras << endl;

	if (numCameras < 1)
	{
		cout << "Insufficient number of cameras... exiting" << endl;
	    //return -1;
	}
#endif

	PGRGuid guid;
	error = busMgr.GetCameraFromIndex(CameraIndex, &guid);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	    //return -1;
	}

	// Connect the camera
	error = camera.Connect(&guid);
	if ( error != PGRERROR_OK )
	{
	    std::cout << "Failed to connect to camera" << std::endl;
	    //return false;
	}
	static int firstEntry = 1;
	if(firstEntry){
    // Power on the camera
    const unsigned int k_cameraPower = 0x610;
    const unsigned int k_powerVal = 0x80000000;
    error = camera.WriteRegister(k_cameraPower, k_powerVal);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    const unsigned int millisecondsToSleep = 100;
    unsigned int regVal = 0;
    unsigned int retries = 10;

    // Wait for camera to complete power-up
    do
    {
        struct timespec nsDelay;
        nsDelay.tv_sec = 0;
        nsDelay.tv_nsec = (long)millisecondsToSleep * 1000000L;
        nanosleep(&nsDelay, NULL);

        error = camera.ReadRegister(k_cameraPower, &regVal);
        if (error == PGRERROR_TIMEOUT)
        {
            // ignore timeout errors, camera may not be responding to
            // register reads during power-up
        }
        else if (error != PGRERROR_OK)
        {
            PrintError(error);
            return -1;
        }

        retries--;
    } while ((regVal & k_powerVal) == 0 && retries > 0);

    // Check for timeout errors after retrying
    if (error == PGRERROR_TIMEOUT)
    {
        PrintError(error);
        return -1;
    }

#if 0
	// Get the camera info and print it out
	error = camera.GetCameraInfo( &camInfo );
	if ( error != PGRERROR_OK )
	{
	    std::cout << "Failed to get camera info from camera" << std::endl;
	    //return false;
	}

	PrintCameraInfo(&camInfo);
#endif

		// Power-up the camera (for cameras that support this feature)
    	//

    	//error = flycaptureSetCameraRegister( context, CAMERA_POWER, 0x80000000 );
    	//_HANDLE_ERROR( error, "flycaptureSetCameraRegister()" );

    	// Configure trigger pin as input and all others as output (1 == output/strobe, 0 == input)
    	unsigned int value1 = 0, value2 = 0;
    	camera.ReadRegister(PIO_DIRECTION, &value1);
    	camera.ReadRegister(TRIGGER_MODE, &value2);

    	//flycaptureGetCameraRegister( context, PIO_DIRECTION, &value1);
    	//flycaptureGetCameraRegister( context, TRIGGER_MODE, &value2);

    	unsigned int value = 0;
    	value |= ((1 << m_GPIOTriggerPin) ^ 0xf);
    	value = SwapEndian(value);
    	camera.WriteRegister(PIO_DIRECTION, value);

    	//flycaptureSetCameraRegister( context, PIO_DIRECTION, value);

    	// bits:
    	// 0 = trigger mode availability
    	// 6 = enable trigger
    	// 7 = default polarity of pin is active (low = 0, high = 1)
    	// [8..10] = decimal for trigger pin index
    	value = 0;
    	camera.ReadRegister(TRIGGER_MODE, &value);

    	//flycaptureGetCameraRegister( context, TRIGGER_MODE, &value);
    	value = SwapEndian(value);
    	value |= (1 << 6); // set enable trigger bit (6) to true
    	value |= (1 << 7); // set polarity bit (7) to active high
    	value &= ~(0x7 << 8); // set bits 8-10 to 0
    	value |= (m_GPIOTriggerPin << 8); // then take or of pin index shifted to 8...10
    	value = SwapEndian(value);
    	camera.WriteRegister( TRIGGER_MODE, value); // Configure camera to use GPIO[0-3] as external trigger

    //trigger settings:  trigger active state, polarity, state, source, mode
    FlyCapture2::TriggerMode triggerMode;
    triggerMode.onOff = true; //(m_Trigger >= 0);
    triggerMode.polarity = m_PulsePolarity;
    triggerMode.source = 0;
    triggerMode.mode = m_PointGreyMode;
    triggerMode.parameter = 0;
    error = camera.SetTriggerMode(&triggerMode, false);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

	//setting this shutter value only effects the exposure when running with trigger mode disabled.
	FlyCapture2::Property prop;
	prop.type = FlyCapture2::SHUTTER;
	prop.valueA = m_Shutter;
	prop.autoManualMode = m_AutomaticShutter;
	camera.SetProperty(&prop, false);

	//SetExposure(m_Shutter, m_AutomaticShutter); // ignored because of trigger mode
	prop.type = FlyCapture2::GAIN;
	//prop.valueA = m_Gain;
	prop.valueA = (unsigned int)m_Gain;
	prop.autoManualMode = m_AutomaticGain;
	camera.SetProperty(&prop, false);
	//SetGain(m_Gain, m_AutomaticGain);
	prop.type = FlyCapture2::BRIGHTNESS;
	prop.valueA = m_Brightness;
	camera.SetProperty(&prop);

    // Poll to ensure camera is ready
    bool retVal = PollForTriggerReady(&camera);
    if (!retVal)
    {
        cout << endl;
        cout << "Error polling for trigger ready!" << endl;
        return -1;
    }

    // Get the camera configuration
    FC2Config config;
    error = camera.GetConfiguration(&config);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Set the grab timeout to 5 seconds
    config.grabTimeout = 5000;

    // Set the camera configuration
    error = camera.SetConfiguration(&config);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Query for available Format 7 modes
    	Format7Info fmt7Info;
    	bool supported;
    	fmt7Info.mode = fmt7Mode;
    	error = camera.GetFormat7Info(&fmt7Info, &supported);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}

    	// PrintFormat7Capabilities(fmt7Info);
    	if ((k_fmt7PixFmt & fmt7Info.pixelFormatBitField) == 0)
    	{
    		// Pixel format not supported!
    	    cout << "Pixel format is not supported" << endl;
    	    // return -1;
    	}

    	Format7ImageSettings fmt7ImageSettings;
    	fmt7ImageSettings.mode = fmt7Mode;
    	fmt7ImageSettings.offsetX = 0;
    	fmt7ImageSettings.offsetY = 0;
    	fmt7ImageSettings.width = m_width;
    	fmt7ImageSettings.height = m_height;
    	fmt7ImageSettings.pixelFormat = k_fmt7PixFmt;
        bool valid;
        Format7PacketInfo fmt7PacketInfo;

        // Validate the settings to make sure that they are valid
    	error = camera.ValidateFormat7Settings(&fmt7ImageSettings, &valid, &fmt7PacketInfo);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}

    	if (!valid)
    	{
    		// Settings are not valid
    	    cout << "Format7 settings are not valid" << endl;
    	    // return -1;
    	}

    	// Set the settings to the camera
    	error = camera.SetFormat7Configuration(&fmt7ImageSettings, fmt7PacketInfo.recommendedBytesPerPacket);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}
#ifdef SOFTWARE_TRIGGER_CAMERA
    if (!CheckSoftwareTriggerPresence(&camera))
    {
        cout << "SOFT_ASYNC_TRIGGER not implemented on this camera! Stopping "
                "application"
             << endl;
        return -1;
    }
#else
    cout << "Trigger the camera by sending a trigger pulse to GPIO"
         << triggerMode.source << endl;

#endif


#ifdef SOFTWARE_TRIGGER_CAMERA
        // Check that the trigger is ready
        PollForTriggerReady(&camera);

        // Fire software trigger
        retVal = FireSoftwareTrigger(&camera);
        if (!retVal)
        {
            cout << endl;
            cout << "Error firing software trigger" << endl;
            //return -1;
        }
#endif
    	firstEntry = 0;
	} // firstEntry
    // Camera is ready, start capturing images
    error = camera.StartCapture();
    	if ( error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED )
    	{
    	    std::cout << "Bandwidth exceeded" << std::endl;
    	    //return false;
    	}
    	else if ( error != PGRERROR_OK )
    	{
    		std::cout << "Failed to start image capture" << std::endl;
    		//return false;
    	}


	// capture loop
	FlyCapture2::Image rawImage;
	error = camera.RetrieveBuffer( &rawImage );
	if ( error != PGRERROR_OK )
	{
		std::cout << "capture error" << std::endl;
	}

	input_image = ConvertImageToOpenCV(&rawImage);
	if(m_RotateInImage)
	{
		IplImage *m_warppedeyeCrop = transposeImage1(input_image, 90);
		// cvSaveImage("Test_90.pgm", m_warppedeyeCrop);
		return(m_warppedeyeCrop);
	}else
	{
		IplImage *dst = cvCreateImage(cvSize(m_width, m_height),IPL_DEPTH_8U,1);
		cvCopyImage(input_image, dst);
		return(dst);
	}
}

IplImage* CmxHandler::GrabFramePGCAM1(int CameraIndex)
{
	FlyCapture2::Error error;
	FlyCapture2::Camera camera;
	FlyCapture2::CameraInfo camInfo;
	FlyCapture2::BusManager busMgr;
	unsigned int numCameras;
	const Mode fmt7Mode = MODE_0;
	const PixelFormat k_fmt7PixFmt = PIXEL_FORMAT_MONO8;
	bool m_AutomaticGain = false;
	bool m_AutomaticShutter = false;

	// printf("GrabFramePG  width...%d height...%d\n", m_width, m_height);
#if 0
	error = busMgr.GetNumOfCameras(&numCameras);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	    //return -1;
	}

	// cout << "Number of cameras detected: " << numCameras << endl;

	if (numCameras < 1)
	{
		cout << "Insufficient number of cameras... exiting" << endl;
	    //return -1;
	}
#endif

	PGRGuid guid;
	error = busMgr.GetCameraFromIndex(CameraIndex, &guid);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	    //return -1;
	}

	// Connect the camera
	error = camera.Connect(&guid);
	if ( error != PGRERROR_OK )
	{
	    std::cout << "Failed to connect to camera" << std::endl;
	    //return false;
	}
	static int firstEntry = 1;
	if(firstEntry){
    // Power on the camera
    const unsigned int k_cameraPower = 0x610;
    const unsigned int k_powerVal = 0x80000000;
    error = camera.WriteRegister(k_cameraPower, k_powerVal);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    const unsigned int millisecondsToSleep = 100;
    unsigned int regVal = 0;
    unsigned int retries = 10;

    // Wait for camera to complete power-up
    do
    {
        struct timespec nsDelay;
        nsDelay.tv_sec = 0;
        nsDelay.tv_nsec = (long)millisecondsToSleep * 1000000L;
        nanosleep(&nsDelay, NULL);

        error = camera.ReadRegister(k_cameraPower, &regVal);
        if (error == PGRERROR_TIMEOUT)
        {
            // ignore timeout errors, camera may not be responding to
            // register reads during power-up
        }
        else if (error != PGRERROR_OK)
        {
            PrintError(error);
            return -1;
        }

        retries--;
    } while ((regVal & k_powerVal) == 0 && retries > 0);

    // Check for timeout errors after retrying
    if (error == PGRERROR_TIMEOUT)
    {
        PrintError(error);
        return -1;
    }

#if 0
	// Get the camera info and print it out
	error = camera.GetCameraInfo( &camInfo );
	if ( error != PGRERROR_OK )
	{
	    std::cout << "Failed to get camera info from camera" << std::endl;
	    //return false;
	}

	PrintCameraInfo(&camInfo);
#endif

		// Power-up the camera (for cameras that support this feature)
    	//

    	//error = flycaptureSetCameraRegister( context, CAMERA_POWER, 0x80000000 );
    	//_HANDLE_ERROR( error, "flycaptureSetCameraRegister()" );

    	// Configure trigger pin as input and all others as output (1 == output/strobe, 0 == input)
    	unsigned int value1 = 0, value2 = 0;
    	camera.ReadRegister(PIO_DIRECTION, &value1);
    	camera.ReadRegister(TRIGGER_MODE, &value2);

    	//flycaptureGetCameraRegister( context, PIO_DIRECTION, &value1);
    	//flycaptureGetCameraRegister( context, TRIGGER_MODE, &value2);

    	unsigned int value = 0;
    	value |= ((1 << m_GPIOTriggerPin) ^ 0xf);
    	value = SwapEndian(value);
    	camera.WriteRegister(PIO_DIRECTION, value);

    	//flycaptureSetCameraRegister( context, PIO_DIRECTION, value);

    	// bits:
    	// 0 = trigger mode availability
    	// 6 = enable trigger
    	// 7 = default polarity of pin is active (low = 0, high = 1)
    	// [8..10] = decimal for trigger pin index
    	value = 0;
    	camera.ReadRegister(TRIGGER_MODE, &value);

    	//flycaptureGetCameraRegister( context, TRIGGER_MODE, &value);
    	value = SwapEndian(value);
    	value |= (1 << 6); // set enable trigger bit (6) to true
    	value |= (1 << 7); // set polarity bit (7) to active high
    	value &= ~(0x7 << 8); // set bits 8-10 to 0
    	value |= (m_GPIOTriggerPin << 8); // then take or of pin index shifted to 8...10
    	value = SwapEndian(value);
    	camera.WriteRegister( TRIGGER_MODE, value); // Configure camera to use GPIO[0-3] as external trigger

    //trigger settings:  trigger active state, polarity, state, source, mode
    FlyCapture2::TriggerMode triggerMode;
    triggerMode.onOff = true; //(m_Trigger >= 0);
    triggerMode.polarity = m_PulsePolarity;
    triggerMode.source = 0;
    triggerMode.mode = m_PointGreyMode;
    triggerMode.parameter = 0;
    error = camera.SetTriggerMode(&triggerMode, false);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

	//setting this shutter value only effects the exposure when running with trigger mode disabled.
	FlyCapture2::Property prop;
	prop.type = FlyCapture2::SHUTTER;
	prop.valueA = m_Shutter;
	prop.autoManualMode = m_AutomaticShutter;
	camera.SetProperty(&prop, false);

	//SetExposure(m_Shutter, m_AutomaticShutter); // ignored because of trigger mode
	prop.type = FlyCapture2::GAIN;
	//prop.valueA = m_Gain;
	prop.valueA = (unsigned int)m_Gain;
	prop.autoManualMode = m_AutomaticGain;
	camera.SetProperty(&prop, false);
	//SetGain(m_Gain, m_AutomaticGain);
	prop.type = FlyCapture2::BRIGHTNESS;
	prop.valueA = m_Brightness;
	camera.SetProperty(&prop);

    // Poll to ensure camera is ready
    bool retVal = PollForTriggerReady(&camera);
    if (!retVal)
    {
        cout << endl;
        cout << "Error polling for trigger ready!" << endl;
        return -1;
    }

    // Get the camera configuration
    FC2Config config;
    error = camera.GetConfiguration(&config);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Set the grab timeout to 5 seconds
    config.grabTimeout = 5000;

    // Set the camera configuration
    error = camera.SetConfiguration(&config);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Query for available Format 7 modes
    	Format7Info fmt7Info;
    	bool supported;
    	fmt7Info.mode = fmt7Mode;
    	error = camera.GetFormat7Info(&fmt7Info, &supported);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}

    	// PrintFormat7Capabilities(fmt7Info);
    	if ((k_fmt7PixFmt & fmt7Info.pixelFormatBitField) == 0)
    	{
    		// Pixel format not supported!
    	    cout << "Pixel format is not supported" << endl;
    	    // return -1;
    	}

    	Format7ImageSettings fmt7ImageSettings;
    	fmt7ImageSettings.mode = fmt7Mode;
    	fmt7ImageSettings.offsetX = 0;
    	fmt7ImageSettings.offsetY = 0;
    	fmt7ImageSettings.width = m_width;
    	fmt7ImageSettings.height = m_height;
    	fmt7ImageSettings.pixelFormat = k_fmt7PixFmt;
        bool valid;
        Format7PacketInfo fmt7PacketInfo;

        // Validate the settings to make sure that they are valid
    	error = camera.ValidateFormat7Settings(&fmt7ImageSettings, &valid, &fmt7PacketInfo);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}

    	if (!valid)
    	{
    		// Settings are not valid
    	    cout << "Format7 settings are not valid" << endl;
    	    // return -1;
    	}

    	// Set the settings to the camera
    	error = camera.SetFormat7Configuration(&fmt7ImageSettings, fmt7PacketInfo.recommendedBytesPerPacket);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}
#ifdef SOFTWARE_TRIGGER_CAMERA
    if (!CheckSoftwareTriggerPresence(&camera))
    {
        cout << "SOFT_ASYNC_TRIGGER not implemented on this camera! Stopping "
                "application"
             << endl;
        return -1;
    }
#else
    cout << "Trigger the camera by sending a trigger pulse to GPIO"
         << triggerMode.source << endl;

#endif


#ifdef SOFTWARE_TRIGGER_CAMERA
        // Check that the trigger is ready
        PollForTriggerReady(&camera);

        // Fire software trigger
        retVal = FireSoftwareTrigger(&camera);
        if (!retVal)
        {
            cout << endl;
            cout << "Error firing software trigger" << endl;
            //return -1;
        }
#endif
    	firstEntry = 0;
	} // firstEntry
    // Camera is ready, start capturing images
    error = camera.StartCapture();
    	if ( error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED )
    	{
    	    std::cout << "Bandwidth exceeded" << std::endl;
    	    //return false;
    	}
    	else if ( error != PGRERROR_OK )
    	{
    		std::cout << "Failed to start image capture" << std::endl;
    		//return false;
    	}


	// capture loop
	FlyCapture2::Image rawImage;
	error = camera.RetrieveBuffer( &rawImage );
	if ( error != PGRERROR_OK )
	{
		std::cout << "capture error" << std::endl;
	}

	input_image = ConvertImageToOpenCV(&rawImage);
	if(m_RotateInImage)
	{
		IplImage *m_warppedeyeCrop = transposeImage1(input_image, 90);
		// cvSaveImage("Test_90.pgm", m_warppedeyeCrop);
		return(m_warppedeyeCrop);
	}else
	{
		IplImage *dst = cvCreateImage(cvSize(m_width, m_height),IPL_DEPTH_8U,1);
		cvCopyImage(input_image, dst);
		return(dst);
	}
}

IplImage* CmxHandler::GrabFramePGCAM2(int CameraIndex)
{
	FlyCapture2::Error error;
	FlyCapture2::Camera camera;
	FlyCapture2::CameraInfo camInfo;
	FlyCapture2::BusManager busMgr;
	unsigned int numCameras;
	const Mode fmt7Mode = MODE_0;
	const PixelFormat k_fmt7PixFmt = PIXEL_FORMAT_MONO8;
	bool m_AutomaticGain = false;
	bool m_AutomaticShutter = false;

	// printf("GrabFramePG  width...%d height...%d\n", m_width, m_height);
#if 0
	error = busMgr.GetNumOfCameras(&numCameras);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	    //return -1;
	}

	// cout << "Number of cameras detected: " << numCameras << endl;

	if (numCameras < 1)
	{
		cout << "Insufficient number of cameras... exiting" << endl;
	    //return -1;
	}
#endif

	PGRGuid guid;
	error = busMgr.GetCameraFromIndex(CameraIndex, &guid);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	    //return -1;
	}

	// Connect the camera
	error = camera.Connect(&guid);
	if ( error != PGRERROR_OK )
	{
	    std::cout << "Failed to connect to camera" << std::endl;
	    //return false;
	}
	static int firstEntry = 1;
	if(firstEntry){
    // Power on the camera
    const unsigned int k_cameraPower = 0x610;
    const unsigned int k_powerVal = 0x80000000;
    error = camera.WriteRegister(k_cameraPower, k_powerVal);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    const unsigned int millisecondsToSleep = 100;
    unsigned int regVal = 0;
    unsigned int retries = 10;

    // Wait for camera to complete power-up
    do
    {
        struct timespec nsDelay;
        nsDelay.tv_sec = 0;
        nsDelay.tv_nsec = (long)millisecondsToSleep * 1000000L;
        nanosleep(&nsDelay, NULL);

        error = camera.ReadRegister(k_cameraPower, &regVal);
        if (error == PGRERROR_TIMEOUT)
        {
            // ignore timeout errors, camera may not be responding to
            // register reads during power-up
        }
        else if (error != PGRERROR_OK)
        {
            PrintError(error);
            return -1;
        }

        retries--;
    } while ((regVal & k_powerVal) == 0 && retries > 0);

    // Check for timeout errors after retrying
    if (error == PGRERROR_TIMEOUT)
    {
        PrintError(error);
        return -1;
    }

#if 0
	// Get the camera info and print it out
	error = camera.GetCameraInfo( &camInfo );
	if ( error != PGRERROR_OK )
	{
	    std::cout << "Failed to get camera info from camera" << std::endl;
	    //return false;
	}

	PrintCameraInfo(&camInfo);
#endif

		// Power-up the camera (for cameras that support this feature)
    	//

    	//error = flycaptureSetCameraRegister( context, CAMERA_POWER, 0x80000000 );
    	//_HANDLE_ERROR( error, "flycaptureSetCameraRegister()" );

    	// Configure trigger pin as input and all others as output (1 == output/strobe, 0 == input)
    	unsigned int value1 = 0, value2 = 0;
    	camera.ReadRegister(PIO_DIRECTION, &value1);
    	camera.ReadRegister(TRIGGER_MODE, &value2);

    	//flycaptureGetCameraRegister( context, PIO_DIRECTION, &value1);
    	//flycaptureGetCameraRegister( context, TRIGGER_MODE, &value2);

    	unsigned int value = 0;
    	value |= ((1 << m_GPIOTriggerPin) ^ 0xf);
    	value = SwapEndian(value);
    	camera.WriteRegister(PIO_DIRECTION, value);

    	//flycaptureSetCameraRegister( context, PIO_DIRECTION, value);

    	// bits:
    	// 0 = trigger mode availability
    	// 6 = enable trigger
    	// 7 = default polarity of pin is active (low = 0, high = 1)
    	// [8..10] = decimal for trigger pin index
    	value = 0;
    	camera.ReadRegister(TRIGGER_MODE, &value);

    	//flycaptureGetCameraRegister( context, TRIGGER_MODE, &value);
    	value = SwapEndian(value);
    	value |= (1 << 6); // set enable trigger bit (6) to true
    	value |= (1 << 7); // set polarity bit (7) to active high
    	value &= ~(0x7 << 8); // set bits 8-10 to 0
    	value |= (m_GPIOTriggerPin << 8); // then take or of pin index shifted to 8...10
    	value = SwapEndian(value);
    	camera.WriteRegister( TRIGGER_MODE, value); // Configure camera to use GPIO[0-3] as external trigger

    //trigger settings:  trigger active state, polarity, state, source, mode
    FlyCapture2::TriggerMode triggerMode;
    triggerMode.onOff = true; //(m_Trigger >= 0);
    triggerMode.polarity = m_PulsePolarity;
    triggerMode.source = 0;
    triggerMode.mode = m_PointGreyMode;
    triggerMode.parameter = 0;
    error = camera.SetTriggerMode(&triggerMode, false);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

	//setting this shutter value only effects the exposure when running with trigger mode disabled.
	FlyCapture2::Property prop;
	prop.type = FlyCapture2::SHUTTER;
	prop.valueA = m_Shutter;
	prop.autoManualMode = m_AutomaticShutter;
	camera.SetProperty(&prop, false);

	//SetExposure(m_Shutter, m_AutomaticShutter); // ignored because of trigger mode
	prop.type = FlyCapture2::GAIN;
	//prop.valueA = m_Gain;
	prop.valueA = (unsigned int)m_Gain;
	prop.autoManualMode = m_AutomaticGain;
	camera.SetProperty(&prop, false);
	//SetGain(m_Gain, m_AutomaticGain);
	prop.type = FlyCapture2::BRIGHTNESS;
	prop.valueA = m_Brightness;
	camera.SetProperty(&prop);

    // Poll to ensure camera is ready
    bool retVal = PollForTriggerReady(&camera);
    if (!retVal)
    {
        cout << endl;
        cout << "Error polling for trigger ready!" << endl;
        return -1;
    }

    // Get the camera configuration
    FC2Config config;
    error = camera.GetConfiguration(&config);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Set the grab timeout to 5 seconds
    config.grabTimeout = 5000;

    // Set the camera configuration
    error = camera.SetConfiguration(&config);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Query for available Format 7 modes
    	Format7Info fmt7Info;
    	bool supported;
    	fmt7Info.mode = fmt7Mode;
    	error = camera.GetFormat7Info(&fmt7Info, &supported);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}

    	// PrintFormat7Capabilities(fmt7Info);
    	if ((k_fmt7PixFmt & fmt7Info.pixelFormatBitField) == 0)
    	{
    		// Pixel format not supported!
    	    cout << "Pixel format is not supported" << endl;
    	    // return -1;
    	}

    	Format7ImageSettings fmt7ImageSettings;
    	fmt7ImageSettings.mode = fmt7Mode;
    	fmt7ImageSettings.offsetX = 0;
    	fmt7ImageSettings.offsetY = 0;
    	fmt7ImageSettings.width = m_width;
    	fmt7ImageSettings.height = m_height;
    	fmt7ImageSettings.pixelFormat = k_fmt7PixFmt;
        bool valid;
        Format7PacketInfo fmt7PacketInfo;

        // Validate the settings to make sure that they are valid
    	error = camera.ValidateFormat7Settings(&fmt7ImageSettings, &valid, &fmt7PacketInfo);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}

    	if (!valid)
    	{
    		// Settings are not valid
    	    cout << "Format7 settings are not valid" << endl;
    	    // return -1;
    	}

    	// Set the settings to the camera
    	error = camera.SetFormat7Configuration(&fmt7ImageSettings, fmt7PacketInfo.recommendedBytesPerPacket);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}
#ifdef SOFTWARE_TRIGGER_CAMERA
    if (!CheckSoftwareTriggerPresence(&camera))
    {
        cout << "SOFT_ASYNC_TRIGGER not implemented on this camera! Stopping "
                "application"
             << endl;
        return -1;
    }
#else
    cout << "Trigger the camera by sending a trigger pulse to GPIO"
         << triggerMode.source << endl;

#endif


#ifdef SOFTWARE_TRIGGER_CAMERA
        // Check that the trigger is ready
        PollForTriggerReady(&camera);

        // Fire software trigger
        retVal = FireSoftwareTrigger(&camera);
        if (!retVal)
        {
            cout << endl;
            cout << "Error firing software trigger" << endl;
            //return -1;
        }
#endif
    	firstEntry = 0;
	} // firstEntry
    // Camera is ready, start capturing images
    error = camera.StartCapture();
    	if ( error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED )
    	{
    	    std::cout << "Bandwidth exceeded" << std::endl;
    	    //return false;
    	}
    	else if ( error != PGRERROR_OK )
    	{
    		std::cout << "Failed to start image capture" << std::endl;
    		//return false;
    	}


	// capture loop
	FlyCapture2::Image rawImage;
	error = camera.RetrieveBuffer( &rawImage );
	if ( error != PGRERROR_OK )
	{
		std::cout << "capture error" << std::endl;
		//return NULL;
	}

	input_image = ConvertImageToOpenCV(&rawImage);
	if(m_RotateInImage)
	{
		IplImage *m_warppedeyeCrop = transposeImage1(input_image, 90);
		// cvSaveImage("Test_90.pgm", m_warppedeyeCrop);
		return(m_warppedeyeCrop);
	}else
	{
		IplImage *dst = cvCreateImage(cvSize(m_width, m_height),IPL_DEPTH_8U,1);
		cvCopyImage(input_image, dst);
		return(dst);
	}
}

#else
#if 0
IplImage* CmxHandler::GrabFramePG(int CameraIndex)
{
	FlyCapture2::Error error;
	FlyCapture2::Camera camera;
	FlyCapture2::CameraInfo camInfo;
	FlyCapture2::BusManager busMgr;
	unsigned int numCameras;
	const Mode fmt7Mode = MODE_0;
	const PixelFormat k_fmt7PixFmt = PIXEL_FORMAT_MONO8;

	// printf("GrabFramePG  width...%d height...%d\n", m_width, m_height);

//	error = busMgr.GetNumOfCameras(&numCameras);
//	if (error != PGRERROR_OK)
//	{
//		PrintError(error);
//	    //return -1;
//	}
//
//	cout << "Number of cameras detected: " << numCameras << endl;
//
//	if (numCameras < 1)
//	{
//		cout << "Insufficient number of cameras... exiting" << endl;
//	    //return -1;
//	}

	PGRGuid guid;
	error = busMgr.GetCameraFromIndex(CameraIndex, &guid);
	if (error != PGRERROR_OK)
	{
		PrintError(error);
	    //return -1;
	}

#if 0

	if(m_RotateInImage)
	{
		IplImage *dst = cvCreateImage(cvSize(m_height,m_width),IPL_DEPTH_8U,1); //cvCloneImage(input_image);
	}else
	{
		IplImage *dst = cvCreateImage(cvSize(m_width, m_height),IPL_DEPTH_8U,1);
	}
#endif
	// char filename[100];
	// static int ImgCnt;

	// Connect the camera
	error = camera.Connect(&guid);
	if ( error != PGRERROR_OK )
	{
	    std::cout << "Failed to connect to camera" << std::endl;
	    //return false;
	}

#if 1 // Camera Trigger

    // Power on the camera
    const unsigned int k_cameraPower = 0x610;
    const unsigned int k_powerVal = 0x80000000;
    error = camera.WriteRegister(k_cameraPower, k_powerVal);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    const unsigned int millisecondsToSleep = 100;
    unsigned int regVal = 0;
    unsigned int retries = 10;

    // Wait for camera to complete power-up
    do
    {
        struct timespec nsDelay;
        nsDelay.tv_sec = 0;
        nsDelay.tv_nsec = (long)millisecondsToSleep * 1000000L;
        nanosleep(&nsDelay, NULL);

        error = camera.ReadRegister(k_cameraPower, &regVal);
        if (error == PGRERROR_TIMEOUT)
        {
            // ignore timeout errors, camera may not be responding to
            // register reads during power-up
        }
        else if (error != PGRERROR_OK)
        {
            PrintError(error);
            return -1;
        }

        retries--;
    } while ((regVal & k_powerVal) == 0 && retries > 0);

    // Check for timeout errors after retrying
    if (error == PGRERROR_TIMEOUT)
    {
        PrintError(error);
        return -1;
    }



#endif // Camera Trigger


	// Get the camera info and print it out
	error = camera.GetCameraInfo( &camInfo );
	if ( error != PGRERROR_OK )
	{
	    std::cout << "Failed to get camera info from camera" << std::endl;
	    //return false;
	}

	PrintCameraInfo(&camInfo);

#if 1

#ifndef SOFTWARE_TRIGGER_CAMERA
    // Check for external trigger support
    TriggerModeInfo triggerModeInfo;
    error = cam.GetTriggerModeInfo(&triggerModeInfo);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    if (triggerModeInfo.present != true)
    {
        cout << "Camera does not support external trigger! Exiting..." << endl;
        return -1;
    }
#endif

    // Get current trigger settings
    TriggerMode triggerMode;
    error = camera.GetTriggerMode(&triggerMode);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Set camera to trigger mode 0
    triggerMode.onOff = true;
    triggerMode.mode = 0;
    triggerMode.parameter = 0;

#ifdef SOFTWARE_TRIGGER_CAMERA
    // A source of 7 means software trigger
    triggerMode.source = 7;
#else
    // Triggering the camera externally using source 0.
    triggerMode.source = 0;
#endif

    error = camera.SetTriggerMode(&triggerMode);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Poll to ensure camera is ready
    bool retVal = PollForTriggerReady(&camera);
    if (!retVal)
    {
        cout << endl;
        cout << "Error polling for trigger ready!" << endl;
        return -1;
    }

    // Get the camera configuration
    FC2Config config;
    error = camera.GetConfiguration(&config);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Set the grab timeout to 5 seconds
    config.grabTimeout = 5000;

    // Set the camera configuration
    error = camera.SetConfiguration(&config);
    if (error != PGRERROR_OK)
    {
        PrintError(error);
        return -1;
    }

    // Query for available Format 7 modes
    	Format7Info fmt7Info;
    	bool supported;
    	fmt7Info.mode = fmt7Mode;
    	error = camera.GetFormat7Info(&fmt7Info, &supported);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}

    	// PrintFormat7Capabilities(fmt7Info);
    	if ((k_fmt7PixFmt & fmt7Info.pixelFormatBitField) == 0)
    	{
    		// Pixel format not supported!
    	    cout << "Pixel format is not supported" << endl;
    	    // return -1;
    	}

    	Format7ImageSettings fmt7ImageSettings;
    	fmt7ImageSettings.mode = fmt7Mode;
    	fmt7ImageSettings.offsetX = 0;
    	fmt7ImageSettings.offsetY = 0;
    	fmt7ImageSettings.width = m_width;
    	fmt7ImageSettings.height = m_height;
    	fmt7ImageSettings.pixelFormat = k_fmt7PixFmt;
        bool valid;
        Format7PacketInfo fmt7PacketInfo;

        // Validate the settings to make sure that they are valid
    	error = camera.ValidateFormat7Settings(&fmt7ImageSettings, &valid, &fmt7PacketInfo);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}

    	if (!valid)
    	{
    		// Settings are not valid
    	    cout << "Format7 settings are not valid" << endl;
    	    // return -1;
    	}

    	// Set the settings to the camera
    	error = camera.SetFormat7Configuration(&fmt7ImageSettings, fmt7PacketInfo.recommendedBytesPerPacket);
    	if (error != PGRERROR_OK)
    	{
    		PrintError(error);
    	    // return -1;
    	}

    // Camera is ready, start capturing images
    error = camera.StartCapture();
    	if ( error == PGRERROR_ISOCH_BANDWIDTH_EXCEEDED )
    	{
    	    std::cout << "Bandwidth exceeded" << std::endl;
    	    //return false;
    	}
    	else if ( error != PGRERROR_OK )
    	{
    		std::cout << "Failed to start image capture" << std::endl;
    		//return false;
    	}

#ifdef SOFTWARE_TRIGGER_CAMERA
    if (!CheckSoftwareTriggerPresence(&camera))
    {
        cout << "SOFT_ASYNC_TRIGGER not implemented on this camera! Stopping "
                "application"
             << endl;
        return -1;
    }
#else
    cout << "Trigger the camera by sending a trigger pulse to GPIO"
         << triggerMode.source << endl;

#endif
#endif



#ifdef SOFTWARE_TRIGGER_CAMERA
        // Check that the trigger is ready
        PollForTriggerReady(&camera);

        // Fire software trigger
        retVal = FireSoftwareTrigger(&camera);
        if (!retVal)
        {
            cout << endl;
            cout << "Error firing software trigger" << endl;
            //return -1;
        }
#endif


	// capture loop
	FlyCapture2::Image rawImage;
	error = camera.RetrieveBuffer( &rawImage );
	if ( error != PGRERROR_OK )
	{
		std::cout << "capture error" << std::endl;

	}

	// printf("rawImage.GetRows().....%d rawImage.GetCols().....%d\n", rawImage.GetRows(), rawImage.GetCols());
	input_image = ConvertImageToOpenCV(&rawImage);
	// sprintf(filename,"InputImage%d.pgm",ImgCnt++);
	// cvSaveImage(filename, input_image);
	if(m_RotateInImage)
	{
		IplImage *m_warppedeyeCrop = transposeImage1(input_image, 90);
		cvSaveImage("Test.pgm", m_warppedeyeCrop);
		// cvWaitKey(1);
		return(m_warppedeyeCrop);
		//IplImage *m_rotatedImage = cvCreateImage(cvSize(m_height, m_width),IPL_DEPTH_8U,1);
		//IplImage *eyemaskImage = cvLoadImage("PG.pgm", CV_LOAD_IMAGE_UNCHANGED);
		//cvCopyImage(eyemaskImage, input_image);
		//cvTranspose(input_image, m_rotatedImage);
		//cvFlip(m_rotatedImage, m_rotatedImage, -1);
		//return(m_rotatedImage);
	}else
	{
		IplImage *dst = cvCreateImage(cvSize(m_width, m_height),IPL_DEPTH_8U,1);
		cvCopyImage(input_image, dst);
		return(dst);
	}
}
#endif
#endif
#endif /* HBOX_PG */



#ifdef CMX_C1
CmxHandler::CmxHandler(Configuration& conf)
:m_debug(0)
,m_socketFactory(0)
#ifdef HBOX_PG
,input_image(0)
,m_width(1600)
,m_height(960)
,m_RotateInImage(false)
,m_PointGreyMode(0)
,m_GPIOTriggerPin(2)
,m_Trigger(1)
,m_PulsePolarity(1)
,m_Gain(6.0)
,m_Shutter(400)
,m_Brightness(0.0)
,m_bStatus(true)
,m_SerialNoCamera1(0), m_SerialNoCamera2(0), m_SerialNoCamera3(0)
#endif
{
	if (m_debug)
		EyelockLog(logger, TRACE, "CmxHandler::CmxHandler() start");

	m_debug = conf.getValue("GRITrigger.CmxDebug",true);
	m_waitPong = false;
	m_pingTime = 0;
	m_sock = 0;

	m_socketFactory = new SocketFactory(conf);

	//const char *cmxOIMAddr = "192.168.4.172:30"; // "169.254.1.10:8192");
	const char *cmxOIMAddr = "127.0.0.1:50"; // "169.254.1.10:8192");
	if(strcmp(cmxOIMAddr,"NONE")!= 0){
		m_resultDestAddr = HostAddress::MakeHost(cmxOIMAddr, eIPv4, false);
	}

	m_rcvdMsg = new BinMessage(4*1024*1023);
	if (m_rcvdMsg == NULL) {
		EyelockLog(logger, ERROR, "CmxHandler - malloc failed ");
	}

	int timeOutms = 200;	// 200 ms
	m_timeOutSend.tv_sec = timeOutms / 1000;
	m_timeOutSend.tv_usec = (timeOutms % 1000) * 1000;

	m_exposureTime = conf.getValue("Eyelock.ExposureTime",4);
	m_analogGain = conf.getValue("Eyelock.CmxAnalogGain",53);
	//Eyelock.ExposureTime
				//int exposuretime = get
#ifdef HBOX_PG
	m_width = conf.getValue("FrameSize.width",0);
	m_height = conf.getValue("FrameSize.height",0);

	m_RotateInImage = conf.getValue("GRI.PointGreyRotate",false);
	input_image = cvCreateImage(cvSize(m_width, m_height),IPL_DEPTH_8U,1);

    m_PointGreyMode = conf.getValue("GRI.PointGreyMode",0);
	m_GPIOTriggerPin = conf.getValue("GRI.GPIOTriggerPin",2);
	m_Trigger = conf.getValue("GRI.Trigger",1);
	m_PulsePolarity = conf.getValue("GRI.PulsePolarity",1);
	m_Gain = conf.getValue("GRI.Gain",6.0f);
	m_Shutter = conf.getValue("GRI.Shutter",400);
	m_Brightness = conf.getValue("GRI.Brightness",0.0f);
	m_SaveImages = conf.getValue("GRI.SaveCameraImages", 0 );
	m_SerialNoCamera1 = conf.getValue("Eyelock.PGSerialNoCamera1", 0 );
	m_SerialNoCamera2 = conf.getValue("Eyelock.PGSerialNoCamera2", 0 );
	m_SerialNoCamera3 = conf.getValue("Eyelock.PGSerialNoCamera3", 0 );
#endif
}

CmxHandler::~CmxHandler() {

	if (m_socketFactory)
		delete m_socketFactory;
#ifdef HBOX_PG
	if(input_image)
			cvReleaseImage(&input_image);
#endif

}


unsigned int CmxHandler::MainLoop() {

	if (m_debug)
		EyelockLog(logger, TRACE, "\n CmxHandler::MainLoop() start");
	printf("CmxHandler::MainLoop() start\n");

	std::string name = "CmxHandler::";

	// create a new thread to poll CMX OIM status
//	if (pthread_create (&statusThread, NULL, pingStatus, this)) {
//		EyelockLog(logger, ERROR, "MainLoop(): Error creating thread pingStatus");
//	}
	//CreateCMDTCPServer(30);
	// create a new thread to get Left Camera images
	if (pthread_create (&leftCThread, NULL, leftCServer, this)) {
		EyelockLog(logger, ERROR, "MainLoop(): Error creating thread leftCServer");
	}
#ifdef HBOX_PG
#if 0 //// Anita 21Feb
	// create a new thread to get Middle Camera images
	if (pthread_create (&faceThread, NULL, middleCamera, this)) {
			EyelockLog(logger, ERROR, "MainLoop(): Error creating thread faceServer");
	}
	// create a new thread to get Right Camera images
	if (pthread_create (&rightCThread, NULL, rightCamera, this)) {
		EyelockLog(logger, ERROR, "MainLoop(): Error creating thread rightCServer");
	}
#endif
#else
	// create a new thread to get Right Camera images
	if (pthread_create (&rightCThread, NULL, rightCServer, this)) {
		EyelockLog(logger, ERROR, "MainLoop(): Error creating thread rightCServer");
	}
	// create a new thread to get Face Camera images
	//if (pthread_create (&faceThread, NULL, faceServer, this)) {
	//	EyelockLog(logger, ERROR, "MainLoop(): Error creating thread faceServer");
	//}
#endif
#if 0
		sleep(5);
		unsigned char buf[256];
		buf[0] = CMX_INIT_CMD;
		HandleSendMsg((char *)buf);

		sleep(1);

		buf[0] = CMX_SEND_CMD;
		HandleSendMsg((char *)buf);
#endif
	printf("CmxHandler::MainLoop() End\n");
	return 0;
}


int CmxHandler::End()
{
	//m_QuitStatus.lock(); m_QuitStatus.set(true); m_QuitStatus.unlock();


	char buf[256];
		int len = 0;
		len = sprintf(buf, "b_on_time(1,0,\"\")\n");
					//len = sprintf(buf, "grab_send(1)\n");
					//len = sprintf(buf, "b_on_time(1,1000,grab_send(1))\n");

	//	SendMessage(buf, len);

		return 0;
	if (leftCThread){
		pthread_join (leftCThread, NULL);
		leftCThread = 0;
	}
	if (rightCThread){
		pthread_join (rightCThread, NULL);
		rightCThread = 0;
	}
	//if (faceThread){
	//	pthread_join (faceThread, NULL);
	//	faceThread = 0;
	//}

	EyelockLog(logger, DEBUG, "CmxHandler::End() => HThread::End()"); fflush(stdout);

}



void CmxHandler::SendMessage(char *outMsg, int len)
{
	if (m_debug)
		EyelockLog(logger, TRACE, "Send Message %d, len %d", outMsg[0], len);

	//printf("CMX SendMessage: sending %s \n",outMsg);
	if (!m_sock) {
		if (CreateCMDTCPServer(30) < 0) {
			return;
		}
	}
	printf("CMX;SendMessage:sending - %s\n",outMsg);
	if( send(m_sock , outMsg , strlen(outMsg) , 0) < 0)
	{
	      perror("Send failed");
	}
	//printf("CMX SendMessage: sent\n");
}

int m_prevR=0,m_prevG=0,m_prevB=0;
bool bSend=false;



void CmxHandler::HandleSendMsg(char *msg)
	unsigned char regs[1];	
	regs[0] = white;
#ifndef HBOX_PG
	if(m_debug)
		EyelockLog(logger, TRACE, "CmxHandler::HandleSendMsg() %d", msg[0]);
//	printf("CmxHandler::HandleSendMsg() %s \n", msg);
	//return;
	CMXMESSAGETYPE msgType = (CMXMESSAGETYPE)msg[0];
	char buf[512];
	int len = 0;
	memset(buf,0x00,512);
	if(msgType == CMX_MATCH)
	{
		printf("setting mes type to match\n");
		Prev_mesg = CMX_MATCH;
	}
	else if(Prev_mesg == CMX_MATCH)
	{
		Prev_mesg = msgType;
		printf("discarding a detect message after match\n");
		return;
	}
	switch(msgType){
		case CMX_LED_CMD:
			if((m_prevR!=msg[2]) || (m_prevG!=msg[3]) || (m_prevB!=msg[4]))
			{
				m_prevR=msg[2];
				m_prevG=msg[3];
				m_prevB=msg[4];
				bSend=true;
			}
			else
			{
				bSend=false;
			}
			if(bSend)
			{
				len = sprintf(buf, "fixed_set_rgb(%d,%d,%d)\n", msg[2], msg[3], msg[4]);	// set_rgb(r,g,b)
				printf("CMX: sending %s\n",buf);
				SendMessage(buf, len);
			if ((msg[2] == 0) && (msg[4] == 0)) {
				regs[0] = Green;
				//	len = sprintf(buf, "play_snd(0)\n");	// set_sound(1) // 1-PASS 2-FAIL 3-TAMPER
				//	SendMessage(buf, len);
			}
			if ((msg[3] == 0) && (msg[4] == 0)) {
				regs[0] = Red;
				//len = sprintf(buf, "play_snd(1)\n");	// set_sound(1) // 1-PASS 2-FAIL 3-TAMPER
				//SendMessage(buf, len);
			}
			if ((msg[2] == 0) && (msg[3] == 0)) {
				regs[0] = Blue;
				//len = sprintf(buf, "play_snd(1)\n");	// set_sound(1) // 1-PASS 2-FAIL 3-TAMPER
				//SendMessage(buf, len);
			}
			if ((msg[2] == 1) && (msg[3] == 1) && (msg[4] == 1)) {
				regs[0] = Cyan;
				//len = sprintf(buf, "play_snd(1)\n");	// set_sound(1) // 1-PASS 2-FAIL 3-TAMPER
				//SendMessage(buf, len);
			}

			BobSetData(regs, 1);
			BobSetCommand (BOB_COMMAND_SET_LED);

		} else {
				printf("SKIPPING AN LED COMMAND\n");
				len = sprintf(buf, "fixed_set_rgb(%d,%d,%d)\n", msg[2], msg[3], msg[4]);	// set_rgb(r,g,b)
				printf("CMX: NOT sending %s\n",buf);

			}
			break;
		case CMX_EYE_DETECT:
					len = sprintf(buf,"DETECT\n");	// set_rgb(r,g,b)

					printf("CMX: sending %s\n",buf);
					SendMessage(buf, len);
					break;
		case CMX_MATCH:
					len = sprintf(buf, "MATCH\n");	// set_rgb(r,g,b)

					printf("CMX: sending %s\n",buf);
					SendMessage(buf, len);
					break;
//CMX_MATCH_FAIL,CMX_NO_DETECT
		case CMX_MATCH_FAIL:
					len = sprintf(buf, "MATCH_FAIL\n");	// set_rgb(r,g,b)

					printf("CMX: sending %s\n",buf);
					SendMessage(buf, len);
					break;
		case CMX_NO_DETECT:
					len = sprintf(buf, "NO_EYES\n");	// set_rgb(r,g,b)

					printf("CMX: sending %s\n",buf);
					SendMessage(buf, len);
					break;

		case CMX_SOUND_CMD:
				printf("******** sending the sound file******** %d : %d : %d \n",msg[1],msg[2],msg[3]);
#if 1
				switch(msg[2])
				{
				case 1:
					//len = sprintf(buf, "play_snd(0)\n");	// set_sound(1) // 1-PASS 2-FAIL 3-TAMPER
					//SendMessage(buf, len);
					break;
				case 2:
				//	len = sprintf(buf, "play_snd(1)\n");	// set_sound(1) // 1-PASS 2-FAIL 3-TAMPER
				//	SendMessage(buf, len);
					break;
				default:
					break;
				}
#endif
		break;
		case CMX_PING_CMD:
			len = sprintf(buf, "status()\n");
			SendMessage(buf, len);
			break;
		case CMX_INIT_CMD:
#if 0
			len = sprintf(buf,"psoc_write(2,22) | psoc_write(1,1) | psoc_write(4,7) | psoc_write(3,0x31) | psoc_write(5,30)\n");
			SendMessage(buf, len);
			sleep(1);
			len = sprintf(buf, "wcr(7,0x3012,%d) | wcr(7,0x301e,0) | wcr(7,0x305e,0x50)\n", m_exposureTime); //| wcr(7,0x3040,0x8000)
			SendMessage(buf, len);
#endif
			system("/home/root/setupcmd.sh");
			break;
		case CMX_SEND_CMD:
			len = sprintf(buf, "b_on_time(1,200,\"grab_send(7) | tgl \")\n");
			//len = sprintf(buf, "grab_send(1)\n");
			//len = sprintf(buf, "b_on_time(1,1000,grab_send(1))\n");
			//b_on_time(1,300,"grab_send(3) | tgl ")
			//SendMessage(buf, len);  // for now running from a batch file
			break;
		default:
			EyelockLog(logger, ERROR, "Unknown MessageType %d ", msgType);;
			break;
	}
#endif
}

void *pingStatus(void *arg)
{
	EyelockLog(logger, TRACE, "pingStatus(): start!"); fflush(stdout);
	CmxHandler *me = (CmxHandler *) arg;
	// sleep(5);

    struct timeval te;
	while (!me->ShouldIQuit()) {
	    gettimeofday(&te, NULL); // get current time
	    unsigned long milliseconds = te.tv_sec*1000 + te.tv_usec/1000; // calculate milliseconds
	    unsigned long elapsed = milliseconds - me->m_pingTime;
	    // printf("milliseconds: %lld\n", milliseconds);

	    if ((me->m_waitPong == true) && elapsed > (unsigned long)WAIT_PONG_TIME) {
	    	EyelockLog(logger, ERROR, "Not received PONG message in %d ms!", WAIT_PONG_TIME); fflush(stdout);
	    	// No Response
	    }
	    else if (!me->m_waitPong && elapsed > SEND_PING_TIME) {
	    	// send ping message
	    	char buff[10];
	    	buff[0] = CMX_PING_CMD;
	    	buff[1] = 4;
	    	buff[2] = 'P';
	    	buff[3] = 'I';
	    	buff[4] = 'N';
	    	buff[5] = 'G';

	    	me->HandleSendMsg(buff);
	    	me->m_pingTime = milliseconds;
	    }


	}
}

bool CmxHandler::HandleReceiveMsg(Socket& client)
{

	if(1)
		return false; //Do not close socket
	else
		return true; //Close the socket
}

#ifdef HBOX_PG
void CmxHandler::CameraStatus(bool bStatus)
{
	m_bStatus = bStatus;
}
#endif

bool CmxHandler::HandleReceiveImage(char *buf, int length) {
#if 0
	if (m_debug)
		EyelockLog(logger, TRACE, "CmxHandler::HandleReceiveImage() start");
#endif
#if 0
	char filename[100];
	static int i;
	sprintf(filename,"image_%d.bin",i++);
	FILE *f = fopen(filename, "wb");
	fwrite(buf, length, 1, f);
#if 0
	FILE *f = fopen("/home/samplefile/image.bin", "rb");
		fseek(f, 0, SEEK_END);
		long fsize = ftell(f);
		fseek(f, 0, SEEK_SET);  //same as rewind(f);

		char *string1 = (char *)malloc(fsize + 1);
		fread(string1, fsize, 1, f);
		fclose(f);
		string1[fsize] = 0;
#endif
#endif
#if 1
	//	return true;
	if(buf)
    {
    	pImageProcessor->setLatestFrame_raw(buf);
    }
#else
	FILE *f = fopen("image_21.bin", "rb");
	if (f == NULL)
		printf("Image open filed\n");
	length = 1200*960;
	fread(buf, length, 1, f);
	fclose(f);
	pImageProcessor->setLatestFrame_raw(buf);
#endif
    return true;

}

int CmxHandler::CreateUDPServer(int port) {
	if (m_debug)
		EyelockLog(logger, TRACE, "CmxHandler::CreateUDPServer() start");

	int sock, length;
	struct sockaddr_in server;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		EyelockLog(logger, ERROR, "Opening socket");
		return sock;
	}
	length = sizeof(server);
	bzero(&server,length);
	server.sin_family=AF_INET;
	server.sin_addr.s_addr=INADDR_ANY;
	server.sin_port=htons(port);
	if (bind(sock,(struct sockaddr *)&server,length)<0) {
		EyelockLog(logger, ERROR, "binding error");
		return -1;
	}
	return sock;
}


int CmxHandler::CreateCMDTCPServer(int port) {
	if (m_debug)
		EyelockLog(logger, TRACE, "CmxHandler::CreateUDPServer() start");

	int sock, length;
	struct sockaddr_in server;

	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	if (m_sock < 0) {
		EyelockLog(logger, ERROR, "Opening socket");
		return m_sock;
	}
	length = sizeof(server);
	bzero(&server,length);
	server.sin_family=AF_INET;
//	server.sin_addr.s_addr=inet_addr("192.168.4.172");
//	server.sin_port=htons(30);
	printf("connecting to 127.0.0.1 at port 50\n");
	server.sin_addr.s_addr=inet_addr("127.0.0.1");
		server.sin_port=htons(50);


	if (connect(m_sock , (struct sockaddr *)&server , sizeof(server)) < 0)
	{
	      perror("connect failed. Error");
	      return -1;
	}
	return m_sock;
}



/************************************************************************/
#ifndef HBOX_PG
void *leftCServer1(void *arg)
{
        EyelockLog(logger, TRACE, "CmxHandler::rightCServer() start");

        CmxHandler *me = (CmxHandler *) arg;
        int length;
        int pckcnt=0;
        char buf[IMAGE_SIZE];
        char databuf[IMAGE_SIZE];
        int datalen = 0;
        short *pShort = (short *)buf;
        bool b_syncReceived = false;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(struct sockaddr_in);
        int leftCSock = me->CreateUDPServer(8192);
        int bytes_to_read = IMAGE_SIZE;
        if (leftCSock < 0) {
                EyelockLog(logger, ERROR, "Failed to create rightServer()");
                return NULL;
        }

        while (!me->ShouldIQuit())
        {

#if 1
            length = recvfrom(leftCSock, buf, min(1500,bytes_to_read), 0, (struct sockaddr *)&from, &fromlen);
            if (length < 0)
                EyelockLog(logger, ERROR, "recvfrom error in rightCServer()");
            else
            {
                if (me->m_debug)
 #if 1
                if (pShort[0] == 0x5555)
                {
                    // the first package
                	//printf("received the sync byte --- new frame\n");
                        datalen = 0;
                        memcpy(databuf, buf+2, length-2);
                        databuf[0] = 1;
                        datalen = length - 2;
                        b_syncReceived = true;
                       // printf("Sync\n");
                        pckcnt=1;
                }
                else if(b_syncReceived)
                {
                        length = (datalen+length <= IMAGE_SIZE-4) ? length : IMAGE_SIZE-4-datalen;
                        memcpy(databuf+datalen, buf, length);
                        datalen += length;
                        pckcnt++;
                }
         //       printf("leftCServer: received : total %d , in this packet %d of total 1152000 \n",datalen,length);
                if (datalen >= IMAGE_SIZE-5)
                {
                	//struct timeval te;
                	//gettimeofday(&te, NULL); // get current time
                	//long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds

                //	printf("image:rightserver %llu\n",milliseconds);
              		//FILE *fp = fopen("cmxRcvd.pgm","wb");
                	//fwrite(databuf, 1, datalen,fp);
                	//	fclose(fp);

                    me->HandleReceiveImage(databuf, datalen);

                    	datalen = 0;
                    	b_syncReceived=false;
                    	//printf("frame rcved 8193\n");

                      /*buf[0] = CMX_SEND_CMD;
                      me->HandleSendMsg(buf);
                      unsigned char cmdbuf[256];
                      cmdbuf[0] = CMX_SEND_CMD;
                      me->HandleSendMsg((char *)cmdbuf);*/
               }
#endif
          }
#endif
      }
      close(leftCSock);
}
#endif
/************************************************************************/
void *leftCServer(void *arg)
{
//	return;
        EyelockLog(logger, TRACE, "CmxHandler::leftCServer() start");



        CmxHandler *me = (CmxHandler *) arg;
        int length;
        int pckcnt=0;
        char buf[IMAGE_SIZE];
        char databuf[IMAGE_SIZE];
        int datalen = 0;
        int bytes_to_get=IMAGE_SIZE;
        short *pShort = (short *)buf;
        bool b_syncReceived = false;
        struct sockaddr_in from;
        int cam_id;
        socklen_t fromlen = sizeof(struct sockaddr_in);
        int bytes_to_read=IMAGE_SIZE;

        // me->HandleReceiveImage(databuf, datalen);

        int leftCSock = me->CreateUDPServer(8192);
        if (leftCSock < 0) {
                EyelockLog(logger, ERROR, "Failed to create leftC Server()");
                return NULL;
        }

       //  sleep(2);
        while (!me->ShouldIQuit())
        {

        	 length = recvfrom(leftCSock, buf, min(1500,bytes_to_read), 0, (struct sockaddr *)&from, &fromlen);
        	            if (length < 0)
        	                printf("recvfrom error in leftCServer()");
        	            else
        	            {
        	                if(!b_syncReceived && pShort[0] == 0x5555)
        	                {
        	                        datalen = 0;
        	                        memcpy(databuf, buf+2, length-2);
        	                        datalen = length - 2;
        	                        b_syncReceived = true;
        	                        pckcnt=1;
        	                        cam_id=databuf[2]&0xff;
        	                        //printf("vs->cam_id %02x\n", vs->cam_id);
        	                       // printf("Sync\n");
        	                }
        	                else if(b_syncReceived)
        	                {
        	                        length = (datalen+length <= IMAGE_SIZE-4) ? length : IMAGE_SIZE-4-datalen;
        	                        memcpy(databuf+datalen, buf, length);
        	                        datalen += length;
        	                        pckcnt++;
        	                }
        	                bytes_to_read-=  length;
        	                if(datalen >= IMAGE_SIZE-5)
        	                {
        	                    me->HandleReceiveImage(databuf, datalen);
        	                   // printf("Got image bytes to read = %d\n",bytes_to_read);
        	                   	datalen = 0;
        	                   	b_syncReceived=false;
        	                   	bytes_to_read=IMAGE_SIZE;
        	               }
        	               if(bytes_to_read<=0)
        	            	   bytes_to_read=IMAGE_SIZE;
        	            }
        }
        printf("closing socket...\n");
      close(leftCSock);
}

#ifdef HBOX_PG
void *middleCamera(void *arg)
{
	EyelockLog(logger, TRACE, "CmxHandler::MiddleCamera() start");

    CmxHandler *me = (CmxHandler *) arg;
    char filename[200];
    static int index;
    // int CamIndex = 1;
    int CamSerialNo = me->m_SerialNoCamera2; // 12120566; Eyelock.PGSerialNoCamera2
    static int firstEntry = 1;
    if(firstEntry){
       	me->CameraPowerOn(CamSerialNo);
       	firstEntry = 0;
    }
    while (!me->ShouldIQuit())
    {
    	IplImage *img = me->GrabFramePG(CamSerialNo);
    	// IplImage *img = me->GrabFramePGCAM1(CamIndex);
    	if(me->m_SaveImages){
    		sprintf(filename,"Cam_%d_%d.pgm",CamSerialNo, index++);
    		cvSaveImage(filename, img);
    	}
		me->HandleReceiveImage(img->imageData, img->imageSize);
		cvReleaseImage(&img);
    }
}

void *rightCamera(void *arg)
{
	EyelockLog(logger, TRACE, "CmxHandler::RightCamera() start");
	CmxHandler *me = (CmxHandler *) arg;
    char filename[200];
    static int Cnt;
    // int CamIndex = 2;
    int CamSerialNo = me->m_SerialNoCamera3; //12120579; PGSerialNoCamera3
    static int firstEntry = 1;
    if(firstEntry){
    	me->CameraPowerOn(CamSerialNo);
        firstEntry = 0;
    }

    while (!me->ShouldIQuit())
    {
    	IplImage *img = me->GrabFramePG(CamSerialNo);
    	// IplImage *img = me->GrabFramePGCAM2(CamIndex);
    	if(me->m_SaveImages){
    		sprintf(filename,"Cam_%d_%d.pgm",CamSerialNo, Cnt++);
    		cvSaveImage(filename, img);
    	}
		me->HandleReceiveImage(img->imageData, img->imageSize);
		cvReleaseImage(&img);
    }
}
#endif


void *rightCServer(void *arg)
{
    EyelockLog(logger, TRACE, "CmxHandler::leftCServer() start");



      CmxHandler *me = (CmxHandler *) arg;
      int length;
      int pckcnt=0;
      char buf[IMAGE_SIZE];
      char databuf[IMAGE_SIZE];
      int datalen = 0;
      int bytes_to_get=IMAGE_SIZE;
      short *pShort = (short *)buf;
      bool b_syncReceived = false;
      struct sockaddr_in from;
      int cam_id;
      socklen_t fromlen = sizeof(struct sockaddr_in);
      int bytes_to_read=IMAGE_SIZE;

      // me->HandleReceiveImage(databuf, datalen);

      int leftCSock = me->CreateUDPServer(8193);
      if (leftCSock < 0) {
              EyelockLog(logger, ERROR, "Failed to create leftC Server()");
              return NULL;
      }

     //  sleep(2);
      while (!me->ShouldIQuit())
      {

      	 length = recvfrom(leftCSock, buf, min(1500,bytes_to_read), 0, (struct sockaddr *)&from, &fromlen);
      	            if (length < 0)
      	                printf("recvfrom error in leftCServer()");
      	            else
      	            {
      	                if(!b_syncReceived && pShort[0] == 0x5555)
      	                {
      	                        datalen = 0;
      	                        memcpy(databuf, buf+2, length-2);
      	                        datalen = length - 2;
      	                        b_syncReceived = true;
      	                        pckcnt=1;
      	                        cam_id=databuf[2]&0xff;
      	                        //printf("vs->cam_id %02x\n", vs->cam_id);
      	                       // printf("Sync\n");
      	                }
      	                else if(b_syncReceived)
      	                {
      	                        length = (datalen+length <= IMAGE_SIZE-4) ? length : IMAGE_SIZE-4-datalen;
      	                        memcpy(databuf+datalen, buf, length);
      	                        datalen += length;
      	                        pckcnt++;
      	                }
      	                bytes_to_read-=  length;
      	                if(datalen >= IMAGE_SIZE-5)
      	                {
      	                    me->HandleReceiveImage(databuf, datalen);
      	                   // printf("Got image bytes to read = %d\n",bytes_to_read);
      	                   	datalen = 0;
      	                   	b_syncReceived=false;
      	                   	bytes_to_read=IMAGE_SIZE;
      	               }
      	               if(bytes_to_read<=0)
      	            	   bytes_to_read=IMAGE_SIZE;
      	            }
      }
      printf("closing socket...\n");
    close(leftCSock);
}

#ifndef HBOX_PG
void *rightCServer1(void *arg)
{
        EyelockLog(logger, TRACE, "CmxHandler::rightCServer() start");

        CmxHandler *me = (CmxHandler *) arg;
        int length;
        int pckcnt=0;
        char buf[IMAGE_SIZE];
        char databuf[IMAGE_SIZE];
        int datalen = 0;
        short *pShort = (short *)buf;
        bool b_syncReceived = false;
        struct sockaddr_in from;
        socklen_t fromlen = sizeof(struct sockaddr_in);
        int leftCSock = me->CreateUDPServer(8193);
        if (leftCSock < 0) {
                EyelockLog(logger, ERROR, "Failed to create rightServer()");
                return NULL;
        }

        while (!me->ShouldIQuit())
        {

#if 1
            length = recvfrom(leftCSock, buf, 1500, 0, (struct sockaddr *)&from, &fromlen);
            if (length < 0)
                EyelockLog(logger, ERROR, "recvfrom error in rightCServer()");
            else
            {
                if (me->m_debug)
 #if 1
                if (pShort[0] == 0x5555)
                {
                    // the first package
                	//printf("received the sync byte --- new frame\n");
                        datalen = 0;
                        memcpy(databuf, buf+2, length-2);
                        databuf[0] = 1;
                        datalen = length - 2;
                        b_syncReceived = true;
                       // printf("Sync\n");
                        pckcnt=1;
                }
                else if(b_syncReceived)
                {
                        length = (datalen+length <= IMAGE_SIZE-4) ? length : IMAGE_SIZE-4-datalen;
                        memcpy(databuf+datalen, buf, length);
                        datalen += length;
                        pckcnt++;
                }
         //       printf("leftCServer: received : total %d , in this packet %d of total 1152000 \n",datalen,length);
                if (datalen >= IMAGE_SIZE-5)
                {
                	//struct timeval te;
                	//gettimeofday(&te, NULL); // get current time
                	//long long milliseconds = te.tv_sec*1000LL + te.tv_usec/1000; // caculate milliseconds

                //	printf("image:rightserver %llu\n",milliseconds);
              		//FILE *fp = fopen("cmxRcvd.pgm","wb");
                	//fwrite(databuf, 1, datalen,fp);
                	//	fclose(fp);

                    me->HandleReceiveImage(databuf, datalen);

                    	datalen = 0;
                    	b_syncReceived=false;
                    	//printf("frame rcved 8193\n");

                      /*buf[0] = CMX_SEND_CMD;
                      me->HandleSendMsg(buf);
                      unsigned char cmdbuf[256];
                      cmdbuf[0] = CMX_SEND_CMD;
                      me->HandleSendMsg((char *)cmdbuf);*/
               }
#endif
          }
#endif
      }
      close(leftCSock);
}



void *faceServer(void *arg)
{
	EyelockLog(logger, TRACE, "CmxHandler::faceServer() start");

	CmxHandler *me = (CmxHandler *) arg;
	int length;
	char buf[IMAGE_SIZE];
	struct sockaddr_in from;
	socklen_t fromlen = sizeof(struct sockaddr_in);
	int faceSock = me->CreateUDPServer(8194);
	if (faceSock < 0) {
		EyelockLog(logger, ERROR, "Failed to create rightC Server()");
		return NULL;
	}

	while (!me->ShouldIQuit()) {

				int data_size=0;
				printf("waiting for image\n");
				while ( data_size <= 307210)
				{
					length = recvfrom(faceSock, buf+data_size, (IMAGE_SIZE - data_size), 0, (struct sockaddr *)&from, &fromlen);
					data_size = data_size + length;

				}
				if(data_size > 0)
				{
					printf("received image from left camera with size %d\n",data_size);
					me->HandleReceiveImage(buf, data_size);
					data_size = 0;
				}
				sleep(4);
			}
	close(faceSock);
}

#endif

#endif



