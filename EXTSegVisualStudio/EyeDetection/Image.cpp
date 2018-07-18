#include "Image.h"
extern "C"{
	#include "file_manip.h"
}


void InitializeIPP(int threads)
{
//	ippSetNumThreads(1);
}

void SaveIPLImage(Image8u *img, char *fname)
{
	//cvSaveImage(fname, img->GetData());
	savefile_OfSize_asPGM((unsigned char *)(img->GetData()->imageData),img->GetData()->width,img->GetData()->height,fname);
}

IplImage *LoadIPLImage(const char *fname)
{
	//return cvLoadImage(fname, CV_LOAD_IMAGE_UNCHANGED);
	int w,h,bits;
	ReadPGM5WHandBits(fname,&w,&h,&bits);
	IplImage *_curImage = cvCreateImage(cvSize(w,h),bits,1);
	int ret = ReadPGM5(fname,(unsigned char *)_curImage->imageData,&w,&h,_curImage->imageSize);
	if(ret){
		printf("Unable to Read file\n");
	}
	return _curImage;
}
