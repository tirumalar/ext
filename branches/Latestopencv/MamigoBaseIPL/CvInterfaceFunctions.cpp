#include <cv.h>
#include <stdio.h>

extern "C"{
	#include "file_manip.h"
}


#include "CvInterfaceDefs.h"

apr_status_t mipl_crop_image(MAMIGO_Image *src, MAMIGO_Image *dst, MAMIGO_Rect *roi)
{
	CvRect rect = cvRect(roi->x, roi->y, roi->width, roi->height);
	cvSetImageROI((IplImage *) src, rect);
	cvCopy((IplImage *) src, (IplImage *) dst);
	cvResetImageROI((IplImage *) src);

	return APR_SUCCESS;
}

MAMIGO_Image *mipl_read_image(const char *filename, int flags)
{
	int w,h,bits;
	ReadPGM5WHandBits(filename,&w,&h,&bits);
	IplImage *_curImage = cvCreateImage(cvSize(w,h),bits,1);
	int ret = ReadPGM5(filename,(unsigned char *)_curImage->imageData,&w,&h,_curImage->imageSize);
	if(ret){
		printf("Unable to Read file\n");
	}
	return (MAMIGO_Image *) _curImage;
}
