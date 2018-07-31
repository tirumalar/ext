/*
 * EyelockAPI.cpp
 *
 *  Created on: Apr 7, 2011
 *      Author: developer1
 */

#include "EyelockAPI.h"
#ifdef IRIS 
#include "IrisSelector.h"
#endif

#include "opencv/cv.h"
#include "opencv/cxcore.h"

extern "C"{
#include "file_manip.h"
}

#include "logging.h"

#define DO_COLOR_CONV 0
const char logger[30] = "EyelockAPI";

#if DO_COLOR_CONV

////////////// from OpenCV\otherlibs\highgui\cvcap_v4l.cpp //////////////

/*
 * Turn a YUV4:2:0 block into an RGB block
 *
 * Video4Linux seems to use the blue, green, red channel
 * order convention-- rgb[0] is blue, rgb[1] is green, rgb[2] is red.
 *
 * Color space conversion coefficients taken from the excellent
 * http://www.inforamp.net/~poynton/ColorFAQ.html
 * In his terminology, this is a CCIR 601.1 YCbCr -> RGB.
 * Y values are given for all 4 pixels, but the U (Pb)
 * and V (Pr) are assumed constant over the 2x2 block.
 *
 * To avoid floating point arithmetic, the color conversion
 * coefficients are scaled into 16.16 fixed-point integers.
 * They were determined as follows:
 *
 *  double brightness = 1.0;  (0->black; 1->full scale)
 *  double saturation = 1.0;  (0->greyscale; 1->full color)
 *  double fixScale = brightness * 256 * 256;
 *  int rvScale = (int)(1.402 * saturation * fixScale);
 *  int guScale = (int)(-0.344136 * saturation * fixScale);
 *  int gvScale = (int)(-0.714136 * saturation * fixScale);
 *  int buScale = (int)(1.772 * saturation * fixScale);
 *  int yScale = (int)(fixScale);
 */

/* LIMIT: convert a 16.16 fixed-point value to a byte, with clipping. */
#define LIMIT(x) ((x)>0xffffff?0xff: ((x)<=0xffff?0:((x)>>16)))

static inline void
move_420_block(int yTL, int yTR, int yBL, int yBR, int u, int v, int rowPixels, unsigned char * rgb)
{
    const int rvScale = 91881;
    const int guScale = -22553;
    const int gvScale = -46801;
    const int buScale = 116129;
    const int yScale  = 65536;
    int r, g, b;

    g = guScale * u + gvScale * v;
//  if (force_rgb) {
//      r = buScale * u;
//      b = rvScale * v;
//  } else {
        r = rvScale * v;
        b = buScale * u;
//  }

    yTL *= yScale; yTR *= yScale;
    yBL *= yScale; yBR *= yScale;

    /* Write out top two pixels */
    rgb[0] = LIMIT(b+yTL); rgb[1] = LIMIT(g+yTL);
    rgb[2] = LIMIT(r+yTL);

    rgb[3] = LIMIT(b+yTR); rgb[4] = LIMIT(g+yTR);
    rgb[5] = LIMIT(r+yTR);

    /* Skip down to next line to write out bottom two pixels */
    rgb += 3 * rowPixels;
    rgb[0] = LIMIT(b+yBL); rgb[1] = LIMIT(g+yBL);
    rgb[2] = LIMIT(r+yBL);

    rgb[3] = LIMIT(b+yBR); rgb[4] = LIMIT(g+yBR);
    rgb[5] = LIMIT(r+yBR);
}

static inline void
move_411_block(int yTL, int yTR, int yBL, int yBR, int u, int v, int rowPixels, unsigned char * rgb)
{
    const int rvScale = 91881;
    const int guScale = -22553;
    const int gvScale = -46801;
    const int buScale = 116129;
    const int yScale  = 65536;
    int r, g, b;

    g = guScale * u + gvScale * v;
//  if (force_rgb) {
//      r = buScale * u;
//      b = rvScale * v;
//  } else {
        r = rvScale * v;
        b = buScale * u;
//  }

    yTL *= yScale; yTR *= yScale;
    yBL *= yScale; yBR *= yScale;

    /* Write out top two first pixels */
    rgb[0] = LIMIT(b+yTL); rgb[1] = LIMIT(g+yTL);
    rgb[2] = LIMIT(r+yTL);

    rgb[3] = LIMIT(b+yTR); rgb[4] = LIMIT(g+yTR);
    rgb[5] = LIMIT(r+yTR);

    /* Write out top two last pixels */
    rgb += 6;
    rgb[0] = LIMIT(b+yBL); rgb[1] = LIMIT(g+yBL);
    rgb[2] = LIMIT(r+yBL);

    rgb[3] = LIMIT(b+yBR); rgb[4] = LIMIT(g+yBR);
    rgb[5] = LIMIT(r+yBR);
}

// Consider a YUV420P image of 8x2 pixels.
//
// A plane of Y values    A B C D E F G H
//                        I J K L M N O P
//
// A plane of U values    1   2   3   4
// A plane of V values    1   2   3   4 ....
//
// The U1/V1 samples correspond to the ABIJ pixels.
//     U2/V2 samples correspond to the CDKL pixels.
//
/* Converts from planar YUV420P to RGB24. */
static void
yuv420p_to_rgb24(int width, int height, unsigned char *pIn0, unsigned char *pOut0)
{
    const int numpix = width * height;
    const int bytes = 24 >> 3;
    int i, j, y00, y01, y10, y11, u, v;
    unsigned char *pY = pIn0;
    unsigned char *pU = pY + numpix;
    unsigned char *pV = pU + numpix / 4;
    unsigned char *pOut = pOut0;

    for (j = 0; j <= height - 2; j += 2) {
        for (i = 0; i <= width - 2; i += 2) {
            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + width);
            y11 = *(pY + width + 1);
            u = (*pU++) - 128;
            v = (*pV++) - 128;

            move_420_block(y00, y01, y10, y11, u, v, width, pOut);

            pY += 2;
            pOut += 2 * bytes;

        }
        pY += width;
        pOut += width * bytes;
    }
}

static void
yuv420_to_rgb24(int width, int height, unsigned char *pIn0, unsigned char *pOut0)
{
    const int numpix = width * height;
    const int bytes = 24 >> 3;
    int i, j, y00, y01, y10, y11, u, v;
    unsigned char *pY = pIn0;
    unsigned char *pV = pY + numpix;
    unsigned char *pU = pV + 1;

    unsigned char *pOut = pOut0;

    for (j = 0; j <= height - 2; j += 2) {
        for (i = 0; i <= width - 2; i += 2, pU+=2, pV+=2) {
            y00 = *pY;
            y01 = *(pY + 1);
            y10 = *(pY + width);
            y11 = *(pY + width + 1);
            u = (*pU) - 128;
            v = (*pV) - 128;

            move_420_block(y00, y01, y10, y11, u, v, width, pOut);

            pY += 2;
            pOut += 2 * bytes;

        }
        pY += width;
        pOut += width * bytes;
    }
}


typedef unsigned long ulong;
static int bwrite_endian(ulong x, int bytes, unsigned char *buf, int endian)
{
    unsigned char u1, u2, u3, u4;
    if (bytes==1) {
        u1 = x&0xFF;
        *buf++ = u1;
        return 1;
    }
    else if (bytes==2) {
        u1 = x&0xFF;
        u2 = (x>>8)&0xFF;
        if (endian) {
            *buf++ = u1;
            *buf++ = u2;
        }
        else {
            *buf++ = u2;
            *buf++ = u1;
        }
        return 2;
    }
    else if (bytes==4) {
        u1 = x&0xFF;
        u2 = (x>>8)&0xFF;
        u3 = (x>>16)&0xFF;
        u4 = (x>>24)&0xFF;
        if (endian) {
            *buf++ = u1;
            *buf++ = u2;
            *buf++ = u3;
            *buf++ = u4;
        }
        else {
            *buf++ = u4;
            *buf++ = u3;
            *buf++ = u2;
            *buf++ = u1;
        }
        return 4;
    }
    else {
        return -1;
    }
}


/* Save a 24-bit BMP file. */
int WriteBMP24(const char *fname, unsigned char * data, int w, int h, int stride, int channels)
{
    FILE *fp = fopen(fname, "wb");
    int Endian = 1;
    unsigned char buffer[1024];
    ulong fileSize;
    ulong imageSize;
    int x, y;

    if (!fp)
      return -1;

    imageSize = 3*w*h;
    fileSize = 14 + 40 + imageSize;

    /* Write header data */
    memset(buffer, 0x00, 14);
    buffer[0] = 66;
    buffer[1] = 77;
    bwrite_endian(fileSize, 4, buffer+2, Endian);
    /* Four bytes of zero data (reserved) */
    bwrite_endian(0, 4, buffer+6, Endian);
    /* Four bytes of data offset (= 14+40 = 54) */
    bwrite_endian(54, 4, buffer+10, Endian);
    fwrite(buffer, 1, 14, fp);

    /* Write info header */
    memset(buffer, 0x00, 40);
    bwrite_endian(40, 4, buffer+0, Endian);
    bwrite_endian(w, 4, buffer+4, Endian);
    bwrite_endian(h, 4, buffer+8, Endian);
    bwrite_endian(1, 2, buffer+12, Endian);  /* Number of planes */
    bwrite_endian(24, 2, buffer+14, Endian);  /* Bit count */
    bwrite_endian(0, 4, buffer+16, Endian);   /* No compression */
    bwrite_endian(imageSize, 4, buffer+20, Endian);
    /* Pixels per meter.  X and Y.  We don't care. Say 0x00010000. */
    bwrite_endian(0x10000, 4, buffer+24, Endian);
    bwrite_endian(0x10000, 4, buffer+28, Endian);
    /* Colors in the image. 2^^24 colors, all "important." */
    bwrite_endian(0x1000000, 4, buffer+32, Endian);
    bwrite_endian(0, 4, buffer+36, Endian);
    fwrite(buffer, 1, 40, fp);

    /* No color table. */

    /* Raster data. */
    memset(buffer, 0x00, 4);
    for (y = h-1; y >= 0; --y) {
		unsigned char *ptr = &data[y * stride];
        for (x = 0; x < w; x ++) {
			for(int i = 0; i < channels; i++) {
				fwrite(&ptr[(x*channels)+i], 1, 1, fp);
			}
        }
        if (x%4 != 0)
          fwrite(buffer, 1, x%4, fp);
    }

    fclose(fp);
    return(0);
}

#endif

/////////////////

ConfAPI::ConfAPI(char* filename):conf(filename){
}

ConfAPI::~ConfAPI(){

}

char* ConfAPI::GetString(char* key){
	char* ptr = (char*)conf.getValue(key,"NONE");
	return ptr;
}

int ConfAPI::GetInt(char* key){
	int val = conf.getValue(key,0);
	return val;
}

void Set(float *array, float x, float y, float tx, float ty)
{
  const int width = 4;
  memset(array, 0, sizeof(float) * width * width);
  array[0 * width + 0] = x;
  array[1 * width + 1] = y;
  array[2 * width + 2] = 1.0;
  array[3 * width + 3] = 1.0;
  array[0 * width + 3] = tx;
  array[1 * width + 3] = ty;
}

void ResizeFrame(unsigned char *input, int w1, int h1, int stride1, unsigned char *output, int w2, int h2, int stride2, float ratio)
{
  const float cx1 = float(w1)/2.f;
  const float cy1 = float(h1)/2.f;
  const float cx2 = float(w2)/2.f;
  const float cy2 = float(h2)/2.f;

  float d2c[16], c2b[16], b2a[16], d2b[16], d2a[16];
  Set(d2c, 1.0, 1.0, -cx2, -cy2);
  Set(b2a, 1.0, 1.0, +cx1, +cy1);
  Set(c2b, 1.0/ratio, 1.0/ratio, 0.0, 0.0);

  CvMat md2c = cvMat(4, 4, CV_32FC1, d2c);
  CvMat mc2b = cvMat(4, 4, CV_32FC1, c2b);
  CvMat mb2a = cvMat(4, 4, CV_32FC1, b2a);
  CvMat md2b = cvMat(4, 4, CV_32FC1, d2b);
  CvMat md2a = cvMat(4, 4, CV_32FC1, d2a);
  cvMatMul( &mc2b, &md2c, &md2b );
  cvMatMul( &mb2a, &md2b, &md2a );  // d2a = b2a * c2b * d2c

  float coeffs[6];
  coeffs[0] = d2a[0];
  coeffs[1] = d2a[1];
  coeffs[2] = d2a[3];
  coeffs[3] = d2a[4];
  coeffs[4] = d2a[5];
  coeffs[5] = d2a[7];

  CvMat t = cvMat(2, 3, CV_32FC1, coeffs);

  IplImage src;
  cvInitImageHeader(&src, cvSize(w1, h1), IPL_DEPTH_8U, 1);
  cvSetData(&src, input, stride1);

  IplImage dst;
  cvInitImageHeader(&dst, cvSize(w2, h2), IPL_DEPTH_8U, 1);
  cvSetData(&dst, output, stride2);

  cvWarpAffine( (CvArr *)&src, (CvArr *)&dst,  &t, CV_INTER_LINEAR+CV_WARP_INVERSE_MAP );
}


void ResizeFrame(unsigned char *input, unsigned char *output, int width, int height, int widthStep, float ratio)
{
	ResizeFrame(input, width, height, widthStep, output, width, height, widthStep, ratio);
}

#define printf LOGI

/////////////////////////////////////// PROCESS THREAD /////////////////////////////////////////////

#include "MessageExt.h"
#include "Safe.h"
#include <iostream>
#include "HThread.h"
#define EXPOSURE_TIME 2000

class MatchProcessServer : public HThread
{
public:

	MatchProcessServer()
	{

	}

	virtual const char* getName() { return "MatchProcessServer"; }

	virtual unsigned int MainLoop()
	{
		while(true)
		{

		}
		return 0;
	}

protected:


};

#include <vector>
#include <deque>

class ScopeLock2
{
public:
	ScopeLock2(pthread_mutex_t &mutex);
	~ScopeLock2();
protected:
	pthread_mutex_t &m_Lock;
};

ScopeLock2::ScopeLock2(pthread_mutex_t &mutex) : m_Lock(mutex) { pthread_mutex_lock(&m_Lock); }
ScopeLock2::~ScopeLock2() { pthread_mutex_unlock(&m_Lock);  }

// Basic producer consumer class using pthread condition variables.
// Opted for non-template version for ARM platform.
// Should use cicular_buffer instead of dequeu

class RingBuffer2
{
protected:
		pthread_cond_t  m_NotFull, m_NotNull;
		pthread_mutex_t m_Lock;
		std::deque<unsigned char *> m_Buffer;
		int m_Max;
public:
		RingBuffer2(int length);
		~RingBuffer2();
		bool Empty();
		bool Full();
		void Push(unsigned char * &thing);
		unsigned char * Pop();
};

RingBuffer2::RingBuffer2(int length) : m_Max(length)
{
	pthread_mutex_init(&m_Lock, 0);
	pthread_cond_init(&m_NotNull, 0);
	pthread_cond_init(&m_NotFull, 0);
}
RingBuffer2::~RingBuffer2()
{
	pthread_mutex_destroy(&m_Lock);
	pthread_cond_destroy(&m_NotNull);
	pthread_cond_destroy(&m_NotFull);
}
bool RingBuffer2::Empty()
{
	return m_Buffer.empty();
}
bool RingBuffer2::Full()
{
	return m_Buffer.size() == m_Max;
}
void RingBuffer2::Push(unsigned char * &thing)
{
	ScopeLock2 lock(m_Lock);
	while(m_Max == m_Buffer.size())
	{
		pthread_cond_wait( &m_NotFull, &m_Lock );
	}
	m_Buffer.push_back(thing);
	pthread_cond_signal( &m_NotNull );
}

unsigned char* RingBuffer2::Pop() // wait and pop
{
	ScopeLock2 lock(m_Lock);
	while(m_Buffer.empty())
	{
		pthread_cond_wait( &m_NotNull, &m_Lock );
	}
	unsigned char* value = m_Buffer.front();
	m_Buffer.pop_front();
	pthread_cond_signal(&m_NotFull);
	return value;
}

///// END THREAD /////////// END THREAD ////////////

//////////
// Scope based wrapper for lock on constructor and unlock on destructor.
// Provides a fail safe way to lock and unlock in the presence of exceptions or equivalent.
//////////

class FrameScopeLock
{
public:
	FrameScopeLock(SafeFrameMsg *pMsg) : m_pMsg(pMsg)
	{
		m_pMsg->lock();
	}
	~FrameScopeLock()
	{
		m_pMsg->unlock();
	}
protected:
	SafeFrameMsg *m_pMsg;
};

void EyelockAPI::GetLEDSpot(int *xy)
{
#if 1
	xy[0] = m_LEDX;
	xy[1] = m_LEDY;
#else
	int ledX = m_LEDX, ledY = m_LEDY;
	pImageProcessor->FixCoordinates(m_LEDX, m_LEDY, ledX, ledY, m_Width, m_Height); // convert to rotated and flipped coordinates
	xy[0] = ledX;
	xy[1] = ledY;
#endif
}

int EyelockAPI::GetConfParam(const char *name)
{
	LOGI("GetConfParam(%s)", name);
	int param = conf.getValue(name, 0);
	return conf.getValue(name, 0);
}

unsigned char *EyelockAPI::GetEnrollmentEye(int index)
{
	LOGI("EyelockAPI::GetEnrollmentEye(%d) =>", index);
	if(index < 0)
	{
		unsigned char *pImage = enrollmentEyes[2], *pLatest = 0;
#if 0
		// Debugging test pattern
		for(int y = 0; y < 480; y++)
		{
			for(int x = 0; x < 640; x++)
			{
				pImage[y * 640 + x] = ((x + y) % 255);
			}
		}
#else
		ELEyeMsg hMsg;
		int w = 0, h = 0;
		citerator<CircularAccess<SafeFrameMsg *> , SafeFrameMsg *> sendIter(outMsgQueue);
		SafeFrameMsg *msg = 0;

		long stamp = 0;

		for(int i=0;i<m_outQSize;i++)
		{
			msg = sendIter.curr();
			FrameScopeLock lock(msg);

			if (msg->isUpdated() && (msg->GetTime() >= stamp))
			{
				hMsg.CopyFrom(*msg);
				if (hMsg.getMsgType() == IMG_MSG)
				{
					stamp = msg->GetTime();
					hMsg.getImageDims(w, h);
					pLatest = (unsigned char *)hMsg.getImage();
				}
			}

			sendIter.next();
		}
#endif

		if(pLatest)
		{
			ResizeFrame((unsigned char *) pLatest, w, h, w,	pImage, 640, 480, 640, pMatchProcessor->getScaleRatio());
			int x = 0, y = 0;
			hMsg.getXY(x, y);
			int *pHeader = (int *)&pImage[0];

			int ledX = m_LEDX, ledY = m_LEDY;
			pImageProcessor->FixCoordinates(m_LEDX, m_LEDY, ledX, ledY, m_Width, m_Height); // convert to rotated and flipped coordinates

			pHeader[0] = (x + w/2);
			pHeader[1] = (y + h/2);
			pHeader[2] = ledX;
			pHeader[3] = ledY;
		}

		return pImage;
	}
	else
	{
		return enrollmentEyes[index];
	}
}


EyelockAPI::EyelockAPI(char* filename, FunctionalType fType):
		conf(filename),
		pImageProcessor(0),
		pMatchProcessor(0),
		m_DetectedMsg(20),
		m_MotionMsg(20),
		m_outQSize(5),
		m_Image(0),
		m_Debug(0),
		m_ImgCntr(0),
		mFunctType(fType),
		m_HDThreshold(0.3),
		m_DoEyelidMask(false),
		m_DoSaveDetections(false)
{
	m_LEDX = m_LEDY = 0;

	for(int i = 0; i < 3; i++)
	{
		enrollmentEyes[i] = new unsigned char[640 * 480];
	}

	for(int i = 0; i < 2; i++)
	{
		enrollmentIrisCodes[i] = new char[2560];
	}

	m_HasEnrollment = false;

	// Draw test pattern for debugging
	for(int y = 0; y < 480; y++)
	{
		for(int x = 0; x < 640; x++)
		{
			enrollmentEyes[0][y * 640 + x] = (x % 255);
			enrollmentEyes[1][y * 640 + x] = (y % 255);
			enrollmentEyes[2][y * 640 + x] = ((y + x) % 255);
		}
	}

	EyelockLog(logger, DEBUG, "EyelockAPI\n");
	const char *str = "NOTAPPLICABLE";

	HTTPPOSTMsg::init(str,str,str,str,str,str,str,str,str,str,str,str,str,str,str);

	m_Debug = conf.getValue("Eyelock.Debug",false);
	m_outQSize=conf.getValue("GRI.outQSize",m_outQSize);

    m_cw = conf.getValue("GRI.cropWidth", 384);
    m_ch = conf.getValue("GRI.cropHeight", 288);

	m_HDThreshold = conf.getValue("GRI.matchScoreThresh", 0.31f);
	m_HDThresholdRaw = conf.getValue("GRI.matchScoreThreshRaw", 0.205f);

	m_SaveImg = conf.getValue("Eyelock.SaveImages",0);
	EyelockLog(logger, DEBUG, "Saving Images %d \n",m_SaveImg);
#ifndef COMMENT

	if(fType==eImageProcessor || fType==eEyeLock)
		pImageProcessor = new EyeLockImageGrabber(&conf,outMsgQueue,m_DetectedMsg,m_MotionMsg);

	int bufSize = conf.getValue("GRI.sendBufferSize", pImageProcessor->defBuffSize);
	outMsgQueue(m_outQSize);

	for (int i = 0; i < m_outQSize; i++) {
		outMsgQueue[i] = new SafeFrameMsg(bufSize);
	}

	FlushAll();

	if(fType==eMatchProcessor || fType==eEyeLock)
		pMatchProcessor = new MatchProcessor(conf);
#endif
	m_Width = conf.getValue("FrameSize.width", 1024 );
	m_Height = conf.getValue("FrameSize.height", 1024 );

	m_InWidth = conf.getValue("InFrameSize.width", 2048);
	m_InHeight = conf.getValue("InFrameSize.height", 1536);

	m_RoiX = conf.getValue("FrameROI.x", 724);
	m_RoiY = conf.getValue("FrameROI.y", 512);

	m_LEDX = conf.getValue("LED.x", 0);
	m_LEDY = conf.getValue("LED.y", 0);

	m_ImageCrop = 0; // not needed for now
}

EyelockAPI::~EyelockAPI() {
#ifndef COMMENT
	if(pMatchProcessor){
		delete pMatchProcessor;
		pMatchProcessor=0;
	}

	if(pImageProcessor){
		delete pImageProcessor;
		pImageProcessor = 0;
	}
#endif
#ifndef COMMENT
	for (int i = 0; i < m_outQSize; i++) {
		delete outMsgQueue[i];
	}

	if(m_Image)
		cvReleaseImage(&m_Image);
	m_Image = NULL;
#endif

	if(m_ImageCrop)
	{
		delete [] m_ImageCrop;
		m_ImageCrop = 0;
	}

	for(int i = 0; i < 3; i++)
	{
		if(enrollmentEyes[i])
			delete [] enrollmentEyes[i];
	}

	for(int i = 0; i < 2; i++)
	{
		if(enrollmentIrisCodes[i])
			delete [] enrollmentIrisCodes[i];
	}
}

void EyelockAPI::SetDoSaveDetections(int save)
{
	m_DoSaveDetections = save;
}

void EyelockAPI::EnableEyelidMask(int value)
{
	ScopeLock2 lock(m_BioLock.Get());

	m_DoEyelidMask = value;
	if(m_DoEyelidMask)
	{
		pMatchProcessor->GetbioInstance()->SetDoRawScore(true);
		pMatchProcessor->GetbioInstance()->SetNominalCommonBits(4100.f/(2.f*2.f));
		pMatchProcessor->GetbioInstance()->SetMinCommonBits(400);
		pMatchProcessor->GetbioInstance()->SetEnableEyelidSegmentation(false); // use masks instead
	}
	else
	{
		pMatchProcessor->GetbioInstance()->SetDoRawScore(false); // use penalty term
		pMatchProcessor->GetbioInstance()->SetNominalCommonBits(4100.f/2.f);
		pMatchProcessor->GetbioInstance()->SetMinCommonBits(400);
		pMatchProcessor->GetbioInstance()->SetEnableEyelidSegmentation(true); // use automatic detection
	}
}

bool EyelockAPI::IsQueueFull(){
	citerator<CircularAccess<SafeFrameMsg *>, SafeFrameMsg *> sendIter(outMsgQueue);

	SafeFrameMsg *msg=0;
	bool bFound= false;

	for(int i=0;i<m_outQSize;i++)
	{
		msg=sendIter.curr();
		{
			FrameScopeLock lock(msg);
			//if any one updated was false means its not full
			if(!msg->isUpdated())
			{
				bFound = true;
			}
		}
		sendIter.next();
		if(bFound)break;
	}
	return (!bFound);
	// True means full
}

bool EyelockAPI::IsQueueEmpty(){
	citerator<CircularAccess<SafeFrameMsg *>, SafeFrameMsg *> sendIter(outMsgQueue);

	SafeFrameMsg *msg=0;
	bool bFound= false;

	for(int i=0;i<m_outQSize;i++)
	{
		msg=sendIter.curr();
		{
			FrameScopeLock lock(msg);
			//if any one updated was true means its not empty
			if(msg->isUpdated()){
				bFound = true;
			}
		}
		sendIter.next();
		if(bFound)break;
	}
	return (!bFound);
	// true means empty
}

bool EyelockAPI::FindBest(int& position){
	citerator<CircularAccess<SafeFrameMsg *>, SafeFrameMsg *> sendIter(outMsgQueue);
	SafeFrameMsg *msg=0;
	int score=0,minscore=-10;
	position = -1;

	bool doReload = false;

	for(int i=0;i<m_outQSize;i++)
	{
		msg=sendIter.curr();
		{
			FrameScopeLock lock(msg);
			if(msg->isUpdated())
			{
				ELEyeMsg hMsg ;
				hMsg.CopyFrom(*msg);
				if(hMsg.getMsgType() == IMG_MSG)
				{
					hMsg.getScore(score);
					if(score > minscore)
					{
						m_bestEye.CopyFrom(*msg);
						minscore = score;
						position = i;
					}
				}
				else
				{
					// database reload request !
					EyelockLog(logger, DEBUG, "NON IMG Msg\n");
					m_bestEye.CopyFrom(*msg); // DJH: Is CopyFrom okay for non-img message?
					minscore = 0;
					position = i;

					// Perform inline (EyelockAPI::FlushAll) to keep loop atomic and since we aren't using recursive mutex
					for(int j=0;j<m_outQSize;j++)
						outMsgQueue[j]->setUpdated(false);

					break;
				}
			}
		}
		sendIter.next();
	}

	return (minscore > (-10));
}

void EyelockAPI::FlushAll(void)
{
	for(int i=0;i<m_outQSize;i++)
	{
		SafeFrameMsg *msg = outMsgQueue[i];
		FrameScopeLock lock(msg);
		msg->setUpdated(false);
	}

#ifdef IRIS	
	// Free the enrollment eye buffer
	for (std::vector<Iris* >::iterator it= m_EnrollmentEyes.begin();it!=m_EnrollmentEyes.end();it++)
	{
		Iris *ir= (*it);
		free( ir->GetImage());
		free( ir->GetCode());
		delete ir;
	}
	m_EnrollmentEyes.clear();

	// Reset the enrollment eye pointers
	m_BestEnrollmentEyes.first = 0;
	m_BestEnrollmentEyes.second = 0;
#endif	
}

void EyelockAPI::CheckQueue(void){
	EyelockLog(logger, DEBUG, "Occupied positions =\n");
	for(int i=0;i<m_outQSize;i++)
	{
		SafeFrameMsg *msg = outMsgQueue[i];
		FrameScopeLock lock(msg);
		if(msg->isUpdated())
		{
			EyelockLog(logger, DEBUG, "%d ",i);
		}
	}
	EyelockLog(logger, DEBUG, "\n");
}

int EyelockAPI::CountEyes(void)
{
	int k=0;
	for(int i=0;i<m_outQSize;i++)
	{
		SafeFrameMsg *msg = outMsgQueue[i];
		FrameScopeLock lock(msg);
		if(msg->isUpdated())
		{
			k++;
		}
	}
	return k;
}

void EyelockAPI::SetUpdatedFalse(int positon)
{
	SafeFrameMsg *msg = outMsgQueue[positon];
	msg->setUpdated(false);
}

int EyelockAPI::FindEyes(char *ptr)
{
	// Crop the input buffer image
	if (pImageProcessor == 0)
	{
		LOGI("No image processor found ignoring\n");
		return 0;
	}

#if 0
	pImageProcessor->setLatestFrame_raw(ptr);
	IplImage *frame = pImageProcessor->GetFrame();
#else
	LOGI("Make image wrapper of size %d %d\n", m_Width, m_Height);
	IplImage image;
	cvInitImageHeader(&image, cvSize(m_Width, m_Height), IPL_DEPTH_8U, 1);
	cvSetData(&image, ptr + (m_RoiY * m_InWidth + m_RoiX), m_InWidth);
	IplImage *frame = &image;
#endif
	bool rc = pImageProcessor->ProcessImage(frame);

	int eyeCount = CountEyes();

#if 0
	if(eyeCount)
	{
		char buffer[1024];
		sprintf(buffer,"%s_%i.pgm","/mnt/sdcard/Eyelock/image_roi", rand()%1000);
		savefile_ROIOfSize_asPGM((unsigned char*)image.imageData,m_Width,m_Height,m_InWidth,buffer);
	}
#endif

	return eyeCount;
}

#define TESTENROLL 1

static void SaveCode(const char *filename, char *code)
{
	std::ofstream output(filename, std::ios::binary);
	if (output.good())
	{
		output.write(code, 2560);
	}
}

int EyelockAPI::SaveBestPairOfEyes(const char *personName)
{
	int rc=0;
#ifdef IRIS	
	ScopeLock2 lock(m_BioLock.Get());

	char buffer[1024];

	float rFeat[8];
	int status=0; //
	ELEyeMsg hMsg ;
	int w=0,h=0;
	citerator<CircularAccess<SafeFrameMsg *>, SafeFrameMsg *> sendIter(outMsgQueue);

	SafeFrameMsg *msg=0;
	for(int i=0;i<m_outQSize;i++)
	{
		msg=sendIter.curr();
		{
			FrameScopeLock lock(msg);
			if(msg->isUpdated())
			{
				hMsg.CopyFrom(*msg);
				hMsg.getImageDims(w,h);
				if(hMsg.getMsgType()==IMG_MSG)
				{
					unsigned char * code = (unsigned char *)malloc(2560);
					unsigned char * scaled = (unsigned char *)malloc(640*480);
					unsigned char * eyecrop=(unsigned char * )hMsg.getImage();

					memset(scaled, 1, 640*480);
					//for(int y = 0; y < 480; y++) for(int x = 0; x < 640; x++) scaled[y*640+x] = rand() % 255;
					ResizeFrame((unsigned char *)hMsg.getImage(), w, h, w, scaled, 640, 480, 640, pMatchProcessor->getScaleRatio());
					LOGI("Scaling done %d \n",i);

#if TESTENROLL
					if(m_DoSaveDetections)
					{
						sprintf(buffer,"%s_%04d.pgm","/mnt/sdcard/Eyelock/enroll",i);
						savefile_OfSize_asPGM(eyecrop,w,h,buffer);
						sprintf(buffer,"%s_%i.pgm","/mnt/sdcard/Eyelock/enroll_scaled",i);
						savefile_OfSize_asPGM(scaled,640,480,buffer);
					}
#endif

					msg->setUpdated(false); // free up this frame message

					if(GetIrisCode(scaled, (char *)code,rFeat))
					{
						Iris * iris=new Iris(scaled,code);
						iris->setFeatureVariances(rFeat);
						m_EnrollmentEyes.push_back(iris);
					}
					else
					{
						LOGI("Segmentation failed %d \n",i);
						EyelockLog(logger, ERROR,"Segmentation failed %d \n",i);
					}
				}
			}
		}
		sendIter.next();
	}

	IrisSelector is(pMatchProcessor->GetbioInstance()->GetMatchInterface());
	is.SetHDThreshold(0.2f);
	is.SetFeatureVarianceScaleIndex(0);
	std::pair< Iris *, Iris *>  eyePair=is.Select(m_EnrollmentEyes);

	// If we have a non-null result and it is better than any existing estimate, then update the enrollment
	if ((eyePair.first != 0) && (eyePair.first != m_BestEnrollmentEyes.first))
	{
		LOGI("Copy enrollment iris image\n");
		rc = 2;

		m_HasEnrollment = true;
		memcpy(enrollmentEyes[0], eyePair.first->GetImage(), 640 * 480);
		memcpy(enrollmentEyes[1], eyePair.first->GetImage(), 640 * 480);

		sprintf(buffer,"%s.pgm","/mnt/sdcard/Eyelock/enroll_best_scaled");
		savefile_OfSize_asPGM(eyePair.first->GetImage(),640,480,buffer);

		memcpy(enrollmentIrisCodes[0], eyePair.first->GetCode(), 2560);
		memcpy(enrollmentIrisCodes[1], eyePair.first->GetCode(), 2560);

		m_BestEnrollmentEyes.first = eyePair.first;
		m_BestEnrollmentEyes.second = eyePair.second;

		//SaveCode("/mnt/sdcard/eyelock/code0_prea.bin", (char *)eyePair.first->GetCode());
		//SaveCode("/mnt/sdcard/eyelock/code1_prea.bin", (char *)eyePair.first->GetCode());
		//SaveCode("/mnt/sdcard/eyelock/code0_preb.bin", (char *)enrollmentIrisCodes[0]);
		//SaveCode("/mnt/sdcard/eyelock/code1_preb.bin", (char *)enrollmentIrisCodes[1]);
	}

#if TESTENROLL
	if(m_DoSaveDetections)
	{
		std::vector< std::vector<Iris *> * > results = 	is.GetRankedEyeClusters();
		for(int i = 0; i < (results.size() < 2 ? results.size() : 2); i++)
		{
			LOGI("RankedEyeClusters List Size[%d]=> %d\n",i,results[i]->size());
			EyelockLog(logger, "RankedEyeClusters List Size[%d]=> %d\n",i,results[i]->size());
			for(int j = 0; j < results[i]->size(); j++)
			{
				sprintf(buffer, "/mnt/sdcard/Eyelock/enroll_%02d_r%02d.pgm", i, j);
				savefile_OfSize_asPGM( (*results[i])[j]->GetImage(), 640, 480, buffer);
			}
		}
	}
#endif

#endif	
	return rc;
}

// Input parameter of null indicates enrollment cancellation
int EyelockAPI::StoreBestPairOfEyes(const char *personName)
{
	int rc = 0;

	LOGI("StoreBestPairOfEyes()\n");

	if((personName != NULL) && (strlen(personName) > 0) && m_HasEnrollment)
	{
		LOGI("personName is (%s)\n", personName);
		EyelockLog(logger, INFO, "personName is (%s)\n", personName);
		{
			const char *fileName=conf.getValue("GRI.irisCodeDatabaseFile","./data/sqlite.db3");

			LOGI("Working on saving Eyes to db for %s into %s\n", personName, fileName);
			EyelockLog(logger, DEBUG, "Working on saving Eyes to db for %s into %s\n", personName, fileName);
			//SaveCode("/mnt/sdcard/eyelock/code0_post.bin", enrollmentIrisCodes[0]);
			//SaveCode("/mnt/sdcard/eyelock/code1_post.bin", enrollmentIrisCodes[1]);
			rc=AppendDB((char *)enrollmentIrisCodes[0] , (char *)enrollmentIrisCodes[1], (char *)personName, (char *)fileName);
		}
	}

	m_HasEnrollment = false;

	return rc;
}

#define TESTMATCH 1

int EyelockAPI::DoMatch(float *score, char **personName)
{
	ScopeLock2 lock(m_BioLock.Get());

	int len=0;
	int bits=0;

	char buffer[1024];
	unsigned char code[2560], mask[2560];
	ELEyeMsg hMsg ;
	int w = 0, h = 0;
	citerator<CircularAccess<SafeFrameMsg *>, SafeFrameMsg *> sendIter(outMsgQueue);
	SafeFrameMsg *msg=0;
	float thresh=pMatchProcessor->getScoreThresh();
	MatchResult *r=0;

	m_MatchResult.reset(); // set score to 1.0

	for(int i=0;(i<m_outQSize);i++)
	{
		msg=sendIter.curr();
		{
			FrameScopeLock lock(msg);
			if(msg->isUpdated())
			{
				hMsg.CopyFrom(*msg);
				hMsg.getImageDims(w,h);
				if(hMsg.getMsgType()==IMG_MSG)
				{
					unsigned char * eyecrop=(unsigned char * )hMsg.getImage();
#if TESTMATCH
					if(m_DoSaveDetections)
					{
						sprintf(buffer,"%s_%04d.pgm","/mnt/sdcard/Eyelock/eyecrop",i);
						savefile_OfSize_asPGM(eyecrop,w,h,buffer);
					}
#endif

					ResizeFrame(eyecrop,w,h,w,(unsigned char *)mBuffer3,640,480,640,pMatchProcessor->getScaleRatio());
					bool isEye=GetIrisCode((unsigned char *)mBuffer3 , (char *)code);

#if TESTMATCH
					if(m_DoSaveDetections)
					{
						sprintf(buffer,"%s_%04d.pgm","/mnt/sdcard/Eyelock/scaled",i);
						savefile_OfSize_asPGM((unsigned char *)mBuffer3,640,480,buffer);
					}
#endif
					if(isEye)
					{

						r=pMatchProcessor->GetMatchManager()->DoMatch(code);
						LOGI("match result => %s %f; ",r->getF2F(len, bits),r->getScore());
						EyelockLog(logger, INFO, "match result => %s %f; ",r->getF2F(len, bits),r->getScore());
						if(r->getScore() < m_MatchResult.getScore())
						{
							m_MatchResult.CopyFrom(*r);
						}
					}
					else
					{
						LOGI("Segmentation failed\n");
					}
				}
			}
		}
		sendIter.next();
	}

	LOGI("Flush the queue\n");

	// Flush the queue
	FlushAll();

	LOGI("Assign the best score and ID value\n");

	// now extract the results from this match
	const float threshold = m_DoEyelidMask ? m_HDThresholdRaw : m_HDThreshold;
	if(m_MatchResult.getKey() && (m_MatchResult.getScore() <= threshold))
	{
		*score=m_MatchResult.getScore();
		*personName=m_MatchResult.getF2F(len, bits);
		return m_MatchResult.getEyeIndex();
	}
	else
	{
		*score=1.0;
		*personName=0;
		return -1;
	}
}

int EyelockAPI::DoMatch(unsigned char *inspCode, float *score, char **personName)
{
	int len=0;
	int bits=0;

	MatchResult  *r=pMatchProcessor->GetMatchManager()->DoMatch(inspCode);
	*score=r->getScore();
	*personName=r->getF2F(len, bits);
	return r->getEyeIndex();
}

int EyelockAPI::GetIrisCode(unsigned char *imageBuffer, char *Iriscode, float *robustFeatures)
{
	int w=640;
	int h=480;
	int rc=pMatchProcessor->GetbioInstance()->GetIrisCode(imageBuffer, w, h, w, Iriscode,0,robustFeatures);

	return rc;
}

void EyelockAPI::SaveEyes(int frIndx)
{
//	CheckQueue();
	int cnt = 0;
	char buff[100] = { 0 };

	citerator<CircularAccess<SafeFrameMsg *>, SafeFrameMsg *> sendIter(outMsgQueue);
	SafeFrameMsg *msg=0;
	for(int i=0;i<m_outQSize;i++)
	{
		msg=sendIter.curr();
		{
			FrameScopeLock lock(msg);
			if(msg->isUpdated())
			{
				ELEyeMsg hMsg ;
				hMsg.CopyFrom(*msg);
				if(hMsg.getMsgType()==IMG_MSG)
				{
					sprintf(buff, "/mnt/sdcard/capture/Crop%d_%d.pgm", frIndx,cnt++);
					EyelockLog(logger, DEBUG, "Saving Eye as %s\n", buff);
					savefile_OfSize_asPGM((unsigned char*) hMsg.getImage(),m_cw, m_ch, buff);
				}
				else
				{
					EyelockLog(logger, DEBUG, "Non image msg:%d\n",	hMsg.getMsgType());
				}
			}
		}
		sendIter.next();
	}
}

void EyelockAPI::Process(char *ptr, int* indx, float *score)
{
	bool test= false;
	int cnt;
	*indx = -1;
	*score = 0.0f;
	char buff[100]={0};

#if 0
	pImageProcessor->setLatestFrame_raw(ptr);
	IplImage *frame = pImageProcessor->GetFrame();
#else
	LOGI("Make image wrapper of size %d %d\n", m_Width, m_Height);
	IplImage image;
	cvInitImageHeader(&image, cvSize(m_Width, m_Height), IPL_DEPTH_8U, 1);
	cvSetData(&image, ptr + (m_RoiY * m_InWidth + m_RoiX), m_InWidth);
	IplImage *frame = &image;
#endif
	bool rc = pImageProcessor->ProcessImage(frame);

	cnt=0;
	if(m_Debug)CheckQueue();

	bool matched = false;
	while(!IsQueueEmpty())
	{
		int pos =-1;
		if(FindBest(pos))
		{
			if(pMatchProcessor)
			{
				TIME_OP("MatchProcessor",
						pMatchProcessor->process(&m_bestEye)
				);
				if(m_SaveImg)
				{
					int w,h;
					pMatchProcessor->GetCropWH(&w,&h);

					sprintf(buff,"/mnt/sdcard/Capture/Inp%d_%d.pgm",m_ImgCntr-1,cnt++);
					EyelockLog(logger, DEBUG, "Saving Eyes as %s\n",buff );
					savefile_OfSize_asPGM((unsigned char*)m_bestEye.getImage(),w,h,buff);
				}

				if(pMatchProcessor->MatchStatus())
				{
					// Means matched
					matched = true;
					break;
				}
			}
		}
		if(pos>=0)SetUpdatedFalse(pos);
	}

	if(m_Debug)CheckQueue();

	if(matched)
	{
		MatchResult* mr = pMatchProcessor->GetMatchResult();
		*score = mr->getScore();
		*indx = mr->getEyeIndex();
		FlushAll();
	}
	return;
}


int EyelockAPI::AppendDB(char *ptr1,char* ptr2,char* fname, char* file){
	return pMatchProcessor->AppendDB(ptr1,ptr2,fname,file);
}



