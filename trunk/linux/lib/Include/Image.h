#pragma once
#include <cxcore.h>
#include "HBox.h"
#include "CvInterfaceDefs.h"
#include <algorithm>
using std::max;
using std::min;


/*
 * Template specializations for for ipp image buffer allocators
 */

template <class T>
inline IplImage* ippiMalloc(T **pointer, int width, int height, int *stride)
{
	return 0;
}
template <> inline IplImage* ippiMalloc(Ipp8u **pointer, int width, int height, int *stride)
{
	IplImage *img = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 1);
	*pointer = (Ipp8u *) cvPtr2D(img, 0, 0);
	*stride = img->widthStep;

	return img;
}
template <> inline IplImage* ippiMalloc(Ipp16u **pointer, int width, int height, int *stride)
{
	IplImage *img = cvCreateImage(cvSize(width, height), IPL_DEPTH_16U, 1);
	*pointer = (Ipp16u *) cvPtr2D(img, 0, 0);
	*stride = img->widthStep;

	return img;
}
template <> inline IplImage* ippiMalloc(Ipp16s **pointer, int width, int height, int *stride)
{
	IplImage *img = cvCreateImage(cvSize(width, height), IPL_DEPTH_16S, 1);
	*pointer = (Ipp16s *) cvPtr2D(img, 0, 0);
	*stride = img->widthStep;

	return img;
}
template <> inline IplImage* ippiMalloc(Ipp32s **pointer, int width, int height, int *stride)
{
	IplImage *img = cvCreateImage(cvSize(width, height), IPL_DEPTH_32S, 1);
	*pointer = (Ipp32s *) cvPtr2D(img, 0, 0);
	*stride = img->widthStep;

	return img;
}
template <> inline IplImage* ippiMalloc(Ipp32f **pointer, int width, int height, int *stride)
{
	IplImage *img = cvCreateImage(cvSize(width, height), IPL_DEPTH_32F, 1);
	*pointer = (Ipp32f *) cvPtr2D(img, 0, 0);
	*stride = img->widthStep;

	return img;
}

/*
 * Ipp image buffer wrapper class
 */
IplImage *LoadIPLImage(const char *fname);

template <class T>
class Image
{
public:
	Image(const char *fname)
	{
		m_Data = LoadIPLImage(fname);
		m_RawData = (T *) m_Data->imageData;
		m_Bands = 1;
		m_Size.width = m_Data->width;
		m_Size.height = m_Data->height;
		m_ChannelCount = 1;
		m_Stride= m_Data->widthStep;
		m_bRelease=true;
	}

	Image(const IppiSize &extent, int channelCount) { Init(extent.width, extent.height);  }
	Image(int widthPixels, int heightPixels, int bands = 1) { Init(widthPixels, heightPixels, bands); }
	Image(const Image<T> &src)
	{
		Init(src.GetWidth(), src.GetHeight(), src.GetChannelCount());
		Copy(src);
	}
	Image(IplImage *data, bool bRelease=false):m_Data(0),m_bRelease(false){
		Init(data,bRelease);
	}
	Image():m_Data(0),m_bRelease(false),m_RawData(0){}
	/*
	 * Rebirth
	 */
	void Init(IplImage *data, bool bRelease=false){
		if(m_Data!=0 && m_bRelease) {
			cvReleaseImage( &m_Data );
			m_Data = 0;
			m_RawData = 0;
		}
		m_Data=data;
		m_bRelease=bRelease;
		m_Bands=m_Data->nChannels;
		m_Size.width=m_Data->width;
		m_Size.height=m_Data->height;
		m_Stride=m_Data->widthStep;
	}
	virtual ~Image(void) { if(m_Data!=0 && m_bRelease) cvReleaseImage( &m_Data ); m_Data = 0; m_RawData = 0;}
	virtual void Init(int widthPixels, int heightPixels, int bands = 1)
	{
		m_Bands = bands;
		m_Size.width = widthPixels;
		m_Size.height = heightPixels;
		m_Data = ippiMalloc(&m_RawData, widthPixels, heightPixels * bands, &m_Stride);
		m_bRelease=true;
	}
	void Fill(T value)
	{
		cvSet(m_Data, cvRealScalar(value));
	}
	void Copy(const Image<T> &img)
	{
		if(m_Size.height != img.GetHeight() || m_Size.width != img.GetWidth())
		{
			if(m_Data)
			{
				cvReleaseImage( &m_Data );
				Init(img.GetHeight(), img.GetWidth());
			}
		}

		cvCopy(m_Data, img.GetData());
	}

	const T &At(int y, int x) const { return ((T *) cvPtr2D(m_Data, y, x))[0]; }
	T &At(int y, int x) { return ((T *) cvPtr2D(m_Data, y, x))[0]; }

	int GetWidth() const { return m_Size.width; }
	int GetHeight() const { return m_Size.height; }
	int GetBands() const { return m_Bands; }

	IplImage *GetData() { return m_Data; }
	const IplImage *GetData() const { return m_Data; }

	int GetStride() const { return m_Stride; }
	IppiSize GetSize() const { return m_Size; }
	int GetChannelCount() const { return 1; }

	void CopyCenteredROIInto(IplImage *dest){
		int left=0,top=0;
		CopyROIInto(dest, cvPoint(GetWidth()>>1, GetHeight()>>1),left,top);
	}
	void copy16to8(IplImage *src, IplImage *dst,CvRect *sroi,CvRect *droi){
		for(int i=0;i<droi->height;i++){
			unsigned char *cptr = (unsigned char *)(dst->imageData + (i+droi->y)*dst->widthStep + droi->x);
			unsigned short *sptr = (unsigned short *)(src->imageData + (i+sroi->y)*src->widthStep + 2*sroi->x);
			for(int j=0;j<droi->width;j++){
				unsigned short a = *sptr++;
				*cptr++ = a>>4;
			}
		}
	}
	void CopyROIInto(IplImage *dest, CvPoint center, int& left, int& top){
		IplImage *src=GetData();
		CvRect srcROI;

		left=srcROI.x = center.x-(dest->width>>1);
		int right = min(srcROI.x+dest->width, src->width);
		srcROI.x =min(max(0,srcROI.x),src->width);	//clip
		srcROI.width=right-srcROI.x;

		top=srcROI.y = center.y-(dest->height>>1);
		int bottom = min(srcROI.y+dest->height, src->height);
		srcROI.y =min(max(0,srcROI.y),src->height); //clip
		srcROI.height=bottom-srcROI.y;

		CvMat subSrc;
		cvGetSubRect(src,&subSrc,srcROI);

		CvRect destROI=srcROI;
		// if any of x and y is zero, we may need different(0,0) in destination image
		if(srcROI.x==0) destROI.x=dest->width-srcROI.width;
		else destROI.x=0;

		if(srcROI.y==0) destROI.y=dest->height-srcROI.height;
		else destROI.y=0;

		CvMat subDest;
		cvGetSubRect(dest,&subDest,destROI);
//		printf("SRC [X Y W H]->[%d %d %d %d]\n",srcROI.x,srcROI.y,srcROI.width,srcROI.height);
//		printf("DST [X Y W H]->[%d %d %d %d]\n",destROI.x,destROI.y,destROI.width,destROI.height);

		if(src->depth == dest->depth)
			cvCopy(&subSrc,&subDest);
		else
		{
			if((src->depth == IPL_DEPTH_16U) && (dest->depth == IPL_DEPTH_8U)){
				copy16to8(src,dest,&srcROI,&destROI);
			}
		}
	}

	int m_ChannelCount;
	IppiSize m_Size;
	int m_Stride;
	int m_Bands;
	IplImage* m_Data;
	T *m_RawData;
protected:
	bool m_bRelease;
};

/*
 * Template instantiations for common IPP image buffer wrapper classes
 */

struct Ipp8u3
{
	unsigned char r, g, b;
};

typedef Image<Ipp8u> Image8u;
typedef Image<Ipp8s> Image8s;
typedef Image<Ipp16s> Image16s;
typedef Image<Ipp16u> Image16u;
typedef Image<Ipp8u3> Image8u3;
typedef Image<Ipp32f> Image32f;
typedef Image<Ipp32s> Image32s;
typedef Image<Ipp64f> Image64f;

void SaveIPLImage(Image8u *img, char *fname);
void HBOX_API InitializeIPP(int threads);
