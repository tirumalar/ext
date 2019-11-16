#ifndef _CV_INTERFACE_DEFS_H
#define _CV_INTERFACE_DEFS_H

#include "apr_time.h"
#include "apr_errno.h"
#include "MamigoBaseIPL_def.h"

enum {OBJECT_STATUS_IGNORE, OBJECT_STATUS_ACTIVE, OBJECT_STATUS_EXPIRED};

typedef struct _MAMIGO_Point
{
    int x;
    int y;
}
MAMIGO_Point;

#ifndef _MAMIGO_RECT_
#define _MAMIGO_RECT_
typedef struct
{
    int x;
    int y;
    int width;
    int height;
}
MAMIGO_Rect;
#endif

typedef struct _MAMIGO_MOTION_CCOMP_BLOB
{
	int num_pixels;
	MAMIGO_Rect roi;
	short id;
	char valid;
	char status;
	double Cx, Cy;
	apr_time_t startTime;
	apr_time_t lastReportTime;
}
MAMIGO_MOTION_CCOMP_BLOB;

typedef struct _MAMIGO_ROI
{
    int  coi; /* 0 - no COI (all channels are selected), 1 - 0th channel is selected ...*/
    int  xOffset;
    int  yOffset;
    int  width;
    int  height;
}
MAMIGO_ROI;

typedef struct _MAMIGO_Image
{
    int  nSize;         /* sizeof(IplImage) */
    int  ID;            /* version (=0)*/
    int  nChannels;     /* Most of OpenCV functions support 1,2,3 or 4 channels */
    int  alphaChannel;  /* ignored by OpenCV */
    int  depth;         /* pixel depth in bits: IPL_DEPTH_8U, IPL_DEPTH_8S, IPL_DEPTH_16S,
                           IPL_DEPTH_32S, IPL_DEPTH_32F and IPL_DEPTH_64F are supported */
    char colorModel[4]; /* ignored by OpenCV */
    char channelSeq[4]; /* ditto */
    int  dataOrder;     /* 0 - interleaved color channels, 1 - separate color channels.
                           cvCreateImage can only create interleaved images */
    int  origin;        /* 0 - top-left origin,
                           1 - bottom-left origin (Windows bitmaps style) */
    int  align;         /* Alignment of image rows (4 or 8).
                           OpenCV ignores it and uses widthStep instead */
    int  width;         /* image width in pixels */
    int  height;        /* image height in pixels */
    struct _MAMIGO_ROI *roi;/* image ROI. if NULL, the whole image is selected */
    struct _MAMIGO_Image *maskROI; /* must be NULL */
    void  *imageId;     /* ditto */
    void  *tileInfo; /* ditto */
    int  imageSize;     /* image data size in bytes
                           (==image->height*image->widthStep
                           in case of interleaved data)*/
    char *imageData;  /* pointer to aligned image data */
    int  widthStep;   /* size of aligned image row in bytes */
    int  BorderMode[4]; /* ignored by OpenCV */
    int  BorderConst[4]; /* ditto */
    char *imageDataOrigin; /* pointer to very origin of image data
                              (not necessarily aligned) -
                              needed for correct deallocation */
}
MAMIGO_Image;

typedef struct
{
	MAMIGO_MOTION_CCOMP_BLOB *blob;
	size_t num_blobs;
	MAMIGO_Image *mask;

} MAMIGO_MOTION_OUTPUT;

MAMIGO_BASE_IPL_DLL_EXPORT apr_status_t mipl_crop_image(MAMIGO_Image *src, MAMIGO_Image *dst, MAMIGO_Rect *roi);
MAMIGO_BASE_IPL_DLL_EXPORT MAMIGO_Image *mipl_read_image(const char *filename, int flags);

#endif	// _CV_INTERFACE_DEFS_H
