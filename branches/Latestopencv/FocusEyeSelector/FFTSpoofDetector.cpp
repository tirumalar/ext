#include "FFTSpoofDetector.h"
#include <stdlib.h>
#include <memory.h>
#include <stdio.h>
#include <math.h>
#include <cv.h>
#include <cxcore.h>
#include <stdio.h>
#include <highgui.h>

#define IRIS_SPECULARITY_COVARIANCE_EIGENVALUE_THRESHOLD	7.0

extern "C" {
	#include "test_fw.h"
	#include "file_manip.h"
}

#ifdef IPP
FFTSpoofDetector::FFTSpoofDetector(int width, int height, int level, int pw, int ph): 
m_height(height), m_width(width), m_pBuffer(0), m_fftImage(0), m_fftMagImage(0), m_fImage(0),
m_xOffset(12), m_yOffset(12), m_peakThreshold(10.0)
{
	// need to compute the order from the image dimensions
	// create an error if it is not power of 2.

	int ix = (int) floor(log(pw*1.0)/log(2.0));
	int iy = (int) floor(log(ph*1.0)/log(2.0));

	int bufSize = 0, bufSize2 = 0;
	ippiFFTInitAlloc_R_32f( &m_lowSpec, ix, iy, IPP_FFT_DIV_FWD_BY_N, ippAlgHintAccurate);
	ippiFFTGetBufSize_R_32f(m_lowSpec, &bufSize);
	ippiFindPeaks3x3GetBufferSize_32f_C1R(pw, &bufSize2);

	m_fImage = new Image32f(width >> level, height >> level);

	m_fftImage = new Image32f(pw, ph);
	m_fftMagImage = new Image32f(pw, ph);
	m_fftMagIntImage = new Image32f(pw, ph);
	
	ippiMomentInitAlloc_64f(&m_momentsState, ippAlgHintFast);

	m_pBuffer = (Ipp8u *) malloc((bufSize > bufSize2)? bufSize: bufSize2);
}

FFTSpoofDetector::~FFTSpoofDetector(void)
{
	ippiMomentFree_64f(m_momentsState);
	ippiFFTFree_R_32f( m_lowSpec );
	if(m_pBuffer)	free(m_pBuffer);	m_pBuffer= 0;

	if(m_fImage)	delete m_fImage; m_fImage = 0;
	if(m_fftImage)	delete m_fftImage; m_fftImage = 0;
	if(m_fftMagImage)	delete m_fftMagImage; m_fftMagImage = 0;
	if(m_fftMagIntImage)	delete m_fftMagIntImage; m_fftMagIntImage = 0;
}

void FFTSpoofDetector::SetPeakSearchOffsets(int w, int h, double threshold)
{
	m_xOffset = w;
	m_yOffset = h;
	m_peakThreshold = threshold;
}

int FFTSpoofDetector::check(Image8u *image)
{
	IppStatus status;
	int iw = m_fftImage->GetWidth();
	int ih = m_fftImage->GetHeight();

	IppiSize roiSize = {iw, ih};

	status = ippiConvert_8u32f_C1R(image->GetData(), image->GetStride(),
		m_fImage->GetData(), m_fImage->GetStride(), image->GetSize());

	Ipp32f *srcPtr = m_fImage->GetData() + (m_fImage->GetHeight()/2 - ih/2)*m_fImage->GetStride()/sizeof(Ipp32f) + m_fImage->GetWidth()/2 - iw/2;

	status = ippiFFTFwd_RToPack_32f_C1R(srcPtr, m_fImage->GetStride(),
		m_fftImage->GetData(), m_fftImage->GetStride(), m_lowSpec, m_pBuffer);
	
	status = ippiMagnitudePack_32f_C1R(m_fftImage->GetData(), m_fftImage->GetStride(),
		m_fftMagIntImage->GetData(), m_fftMagIntImage->GetStride(), roiSize);

	roiSize.height = roiSize.height >> 1;
	roiSize.width = roiSize.width >> 1;

	ippiCopy_32f_C1R(m_fftMagIntImage->GetData(), m_fftMagIntImage->GetStride(), m_fftMagImage->GetData()+(ih/2)*m_fftMagImage->GetStride()/sizeof(Ipp32f) + iw/2, m_fftMagImage->GetStride(), roiSize);
	ippiCopy_32f_C1R(m_fftMagIntImage->GetData()+(ih/2)*m_fftMagIntImage->GetStride()/sizeof(Ipp32f) + iw/2, m_fftMagIntImage->GetStride(), m_fftMagImage->GetData(), m_fftMagImage->GetStride(), roiSize);
	ippiCopy_32f_C1R(m_fftMagIntImage->GetData() + iw/2, m_fftMagIntImage->GetStride(), m_fftMagImage->GetData()+(ih/2)*m_fftMagImage->GetStride()/sizeof(Ipp32f), m_fftMagImage->GetStride(), roiSize);
	ippiCopy_32f_C1R(m_fftMagIntImage->GetData()+(ih/2)*m_fftMagIntImage->GetStride()/sizeof(Ipp32f), m_fftMagIntImage->GetStride(), m_fftMagImage->GetData()+ iw/2, m_fftMagImage->GetStride(), roiSize);


#if 0	// this can be activated for evaluating offline
	FILE *fp = fopen("ffttest1.txt", "wt");
	for(int i=0;i<ih;i++)
	{
		float *fptr = (float *) (m_fftMagImage->GetData() + i*m_fftMagImage->GetStride()/sizeof(Ipp32f));
		for(int j=0;j<iw;j++)
		{
			fprintf(fp, "%f ", *fptr++);
		}
		fprintf(fp, "\n");

	}
	fclose(fp);
#endif

	IppiSize roiSize1, roiSize2;
	Ipp64f sum1, sum2;

	roiSize1.width = 2*m_xOffset;
	roiSize1.height = 2*m_yOffset;

	status = ippiSum_32f_C1R(m_fftMagImage->GetData() + (ih/2 - m_yOffset)*m_fftMagImage->GetStride()/sizeof(Ipp32f) + (iw/2 - m_xOffset),
		m_fftMagIntImage->GetStride(), roiSize1, &sum1, ippAlgHintFast);

	roiSize2.width = 4*m_xOffset;
	roiSize2.height = 4*m_yOffset;

	status = ippiSum_32f_C1R(m_fftMagImage->GetData() + (ih/2 - 2*m_yOffset)*m_fftMagImage->GetStride()/sizeof(Ipp32f) + (iw/2 - 2*m_xOffset),
		m_fftMagIntImage->GetStride(), roiSize2, &sum2, ippAlgHintFast);

	Ipp64f mean = (sum2 - sum1)/(roiSize2.width*roiSize2.height - roiSize1.height*roiSize1.width);

	int maxPeakCount = 500;
	int peakCount = 0;
	// Now try to locate some maxima's around
	// presence of symmetric set of maxima's near high frequencies indicate some funky business

	roiSize.height = ih;
	roiSize.width = iw;

	IppiPoint *peaks = (IppiPoint *) calloc(maxPeakCount, sizeof(IppiPoint)); 

	Ipp32f threshold = (Ipp32f) (5*mean);

	status = ippiFindPeaks3x3_32f_C1R(m_fftMagImage->GetData(), m_fftMagImage->GetStride(), roiSize, threshold,
		peaks, maxPeakCount, &peakCount, ippiNormInf, 1, m_pBuffer);

	Ipp32f *magPtr = m_fftMagImage->GetData();

	int off = 2;
	int th = 1;
	roiSize1.height = roiSize1.width = off*2+1;
	roiSize2.height = roiSize2.width = (off+th)*2+1;

	int realPeakCount = 0;

	for(int i=0;i<peakCount;i++)
	{
		if(peaks[i].x == iw/2 || peaks[i].y == ih/2 || (abs(peaks[i].x - iw/2) < m_xOffset && abs(peaks[i].y - ih/2) < m_yOffset))
			continue;

		int index = (peaks[i].y) *m_fftMagImage->GetStride()/sizeof(Ipp32f) + peaks[i].x;

		status = ippiSum_32f_C1R(m_fftMagImage->GetData() + (peaks[i].y - off)*m_fftMagImage->GetStride()/sizeof(Ipp32f) + (peaks[i].x - off),
			m_fftMagIntImage->GetStride(), roiSize1, &sum1, ippAlgHintFast);

		status = ippiSum_32f_C1R(m_fftMagImage->GetData() + (peaks[i].y - off - th)*m_fftMagImage->GetStride()/sizeof(Ipp32f) + (peaks[i].x - off - th),
			m_fftMagIntImage->GetStride(), roiSize2, &sum2, ippAlgHintFast);

		mean = (sum2 - sum1)/(roiSize2.width*roiSize2.height - roiSize1.height*roiSize1.width);

		if(magPtr[index]/mean > m_peakThreshold)
		{
			realPeakCount++;
//			printf("(%d %d) = %f (%f %f)\n", peaks[i].x, peaks[i].y,  magPtr[index], (float) mean, (float) (magPtr[index]/mean)); 
		}
	}

	free(peaks);

	return realPeakCount;
}
#else

#define __USE_FAST_LOOKUP__ 1


unsigned short cossinTable[] = {0x6480,0x0059,0xD54D,0x0252,0x0388};

void twidfft2d_fr16(
  complex_fract16 w[],  /* pointer to complex twiddle array */
  int n                 /* FFT length */
  )
{
#ifdef __BFIN__
  int      i, idx;
  int      nquart = n/4;
#ifdef __USE_FAST_LOOKUP__
  fract16  val[100/*nquart*/+1]; //index starting at 1!
#else
  fract16  val;
#endif
  float    step;

  step = 1.0/(float)nquart;
  idx  = 0;

  // 1. Quadrant
  // Compute cosine and sine values for the range [0..PI/2)
  w[idx].re = 0x7fff;  //=cos(0)
  w[idx].im = 0x0;     //=sin(0)
  for(i = 1; i < nquart; i++)
  {
    idx++;
#ifdef __USE_FAST_LOOKUP__
    val[i] = (fract16) ((i*step) * 32767.0); //count up
    w[idx].re = cos_fr16(val[i],cossinTable);
    w[idx].im = -sin_fr16(val[i],cossinTable);
#else
    val = (fract16) ((i*step) * 32767.0); //count up
    w[idx].re = cos_fr16(val,cossinTable);
    w[idx].im = -sin_fr16(val,cossinTable);
#endif
  }

  // 2. Quadrant
  // Compute cosine values for the range [PI/2..PI)
  // Since sin( [PI/2..PI] ) a mirror image of sin( [0..PI/2] )
  // no need to compute sine again
  idx++;
  w[idx].re = 0x0;      //=cos(PI/2)
  w[idx].im = 0x8000;   //=-sin(PI/2);
  for(i = 1; i < nquart; i++)
  {
    idx++;
#ifdef __USE_FAST_LOOKUP__
    w[idx].re = -cos_fr16(val[nquart-i],cossinTable);
    w[idx].im = w[nquart-i].im;
#else
    val = (fract16) (((nquart-i)*step) * 32767.0); //count down
    w[idx].re = -cos_fr16(val,cossinTable);
    w[idx].im = w[nquart-i].im;
#endif
  }

  // 3. Quadrant
  // Compute sine values for the range [PI..3PI/2)
  // Since cos( [PI..3PI/2] ) a mirror image of cos( [PI/2..PI] )
  // no need to compute cosine again
  idx++;
  w[idx].re = 0x8000;   //=cos(PI)
  w[idx].im = 0x0;      //=-sin(PI)
  for(i = 1; i < nquart; i++)
  {
    idx++;
#ifdef __USE_FAST_LOOKUP__
    val[i] = (fract16) ((i*step) * -32768.0); //count up negative
    w[idx].re = w[2*nquart-i].re;
    w[idx].im = -sin_fr16(val[i],cossinTable);
#else
    val = (fract16) ((i*step) * -32768.0); //count up negative
    w[idx].re = w[2*nquart-i].re;
    w[idx].im = -sin_fr16(val,cossinTable);
#endif
  }

  // 4. Quadrant
  // Since sin( [3PI/2..2PI] ) a mirror image of sin( [PI..3PI/2] )
  // no need to compute sine again
  // Since cos( [3PI/2..2PI] ) a mirror image of cos( [0..PI/2] )
  // no need to compute cosine again
  idx++;
  w[idx].re = 0x0;      //=cos(3/2PI)
  w[idx].im = 0x7fff;   //=-sin(3/2PI)
  for(i = 1; i < nquart; i++)
  {
    idx++;
    w[idx].re = w[nquart-i].re;
    w[idx].im = w[3*nquart-i].im;
  }
#endif
}

FFTSpoofDetector::FFTSpoofDetector(int width, int height, int level, int pw, int ph):
m_height(height), m_width(width), m_fftImage(0), m_fftMagImage(0),m_Twiddle(0),
m_xOffset(12), m_yOffset(12), m_peakThreshold(75.0),m_PeakBuf(0),m_fftIn(0),m_fftOut(0),m_planFft(0)
{
	// need to compute the order from the image dimensions
	// create an error if it is not power of 2.
	int ix = (int) floor(log(pw*1.0)/log(2.0));
	int iy = (int) floor(log(ph*1.0)/log(2.0));

	m_fftImage = cvCreateImage(cvSize(pw, ph),IPL_DEPTH_16S,1);
	m_fftOutImage = cvCreateImage(cvSize(pw, ph),IPL_DEPTH_32S,1);
	m_fftMagImage = cvCreateImage(cvSize(pw, ph),IPL_DEPTH_32S,1);

	m_Twiddle = (complex_fract16*)calloc(4,pw);
	m_PeakBuf = (CvPoint*)m_fftImage->imageData;
	m_maxPeakCount = m_fftImage->imageSize/sizeof(CvPoint);

#ifdef __BFIN__
	twidfft2d_fr16(m_Twiddle,pw);
#endif

#ifdef __linux__
	int  N = pw * ph;
	m_fftIn  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
	m_fftOut  = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * N);
	m_planFft = fftw_plan_dft_2d(pw,ph, m_fftIn, m_fftOut, FFTW_FORWARD, FFTW_ESTIMATE);
#endif

}

static void Print(char *str,IplImage *ptr){
	printf("\n%s\n",str);
	for (int i = 0;i < ptr->height; i++){
		for (int j = 0; j < ptr->width; j++){
			printf(" %3d ",*(((unsigned int*)ptr->imageData)+ i* (ptr->widthStep>>2) +j));
		}
		printf("\n");
	}
}

void FFTSpoofDetector::PopulateData(IplImage* image,IplImage *out){
	/* Populate input data in row-major order */
//	printf("PopulateData\n");
	unsigned char *inptrbase= (unsigned char *)(image->imageData + (image->height/2 - out->height/2 )*image->widthStep + (image->width/2 - out->width/2 )) ;
	for(int i=0,k = 0;i<out->height;i++){
		unsigned char * inptr = inptrbase+i*image->widthStep;
		for(int j=0;j<out->width;j++,k++){
//			printf(" %3d ",*inptr);
			m_fftIn[k][0]= *inptr++;
			m_fftIn[k][1] = 0.;
		}
//		printf("\n");
	}
}

void FFTSpoofDetector::ComputeMagnitude(unsigned int * outptr1,int iw,int ih){
//	printf("ComputeMagnitude\n");
	for(int i=0;i<ih;i++){
		for(int j=0;j<iw;j++){
			int x = i*iw+j;
			*outptr1++ = (unsigned int)sqrt(m_fftOut[x][0]*m_fftOut[x][0]+m_fftOut[x][1]*m_fftOut[x][1]);
//			printf("%8d ",outptr1[-1]);
		}
//		printf("\n");
	}
}

FFTSpoofDetector::~FFTSpoofDetector(void){
	if(m_fftImage)	cvReleaseImage(&m_fftImage); m_fftImage = 0;
	if(m_fftOutImage)	cvReleaseImage(&m_fftOutImage); m_fftOutImage = 0;
	if(m_fftMagImage)	cvReleaseImage(&m_fftMagImage); m_fftMagImage = 0;
	if(m_Twiddle) free(m_Twiddle); m_Twiddle = 0;

	if(m_planFft) fftw_destroy_plan(m_planFft);
	if(m_fftIn) fftw_free(m_fftIn);
	if(m_fftOut) fftw_free(m_fftOut);
}

void FFTSpoofDetector::SetPeakSearchOffsets(int w, int h, double threshold)
{
	m_xOffset = w;
	m_yOffset = h;
	m_peakThreshold = threshold;
}

void FFTSpoofDetector::ComputeMagnitude(complex_fract16 *ptr ,unsigned int * outptr1,int iw,int ih){
	for(int i=0;i<ih;i++){
		for(int j=0;j<iw;j++){
			*outptr1++ = (ptr->im)*(ptr->im) + (ptr->re)*(ptr->re);
			ptr++;
		}
	}
}

void FFTSpoofDetector::Copy8to16(IplImage* image,IplImage *out){
	unsigned char *inptrbase= (unsigned char *)(image->imageData + (image->height/2 - out->height/2 )*image->widthStep + (image->width/2 - out->width/2 )) ;
	for(int i=0;i<out->height;i++){
		short* outptr = (short*)(out->imageData + i* out->widthStep);
		unsigned char * inptr = inptrbase+i*image->widthStep;
		for(int j=0;j<out->width;j++){
			short int a = *inptr++;
			*outptr++ = (a<<7);
		}
	}
}

void FFTSpoofDetector::SwapQuad(){
	int *inpA = (int *)m_fftOutImage->imageData;
	int *inpB = inpA + (m_fftOutImage->widthStep>>3);
	int *inpC = (int *)(m_fftOutImage->imageData + (m_fftOutImage->widthStep * (m_fftOutImage->height/2)));
	int *inpD = inpC + (m_fftOutImage->widthStep>>3);

	int *outA = (int *)m_fftMagImage->imageData;
	int *outB = outA + (m_fftMagImage->widthStep>>3);
	int *outC = (int *)(m_fftMagImage->imageData + (m_fftMagImage->widthStep * (m_fftMagImage->height/2)));
	int *outD = outC + (m_fftMagImage->widthStep>>3);

	int stepinint = m_fftMagImage->widthStep>>2;

	for(int j=0;j<m_fftMagImage->height>>1;j++){
		memcpy(outA+j*stepinint,inpD+j*stepinint,m_fftMagImage->widthStep>>1);
		memcpy(outB+j*stepinint,inpC+j*stepinint,m_fftMagImage->widthStep>>1);
		memcpy(outC+j*stepinint,inpB+j*stepinint,m_fftMagImage->widthStep>>1);
		memcpy(outD+j*stepinint,inpA+j*stepinint,m_fftMagImage->widthStep>>1);
	}
}

long int FFTSpoofDetector::ComputeSum(unsigned int *ptr, int widthstep,int width,int height){
	long int sum =0;
	for(int i=0;i<height;i++){
		unsigned int *out=ptr + i*(widthstep>>2);
		for(int j=0;j<width;j++){
			sum+= *out++;
		}
	}
	return sum;
}


int FFTSpoofDetector::check(IplImage *image){

	int iw = m_fftImage->width;
	int ih = m_fftImage->height;
#ifdef __BFIN__
	XTIME_OP("COPY",
		Copy8to16(image,m_fftImage);
	);

	XTIME_OP("FFT",
	rfft2d_fr16 ((const short int*)m_fftImage->imageData,(complex_fract16 *)m_fftMagImage->imageData,
			(complex_fract16 *)m_fftOutImage->imageData,(complex_fract16 *)m_Twiddle,1,iw,0,0);
	);

	complex_fract16 *ptr = (complex_fract16 *)m_fftOutImage->imageData;
	unsigned int *outptr1 = (unsigned int *)m_fftOutImage->imageData;
	XTIME_OP("MAG",
			ComputeMagnitude(ptr,outptr1,iw,ih)
	);
#endif

#ifdef __linux__
	PopulateData(image,m_fftImage);
	TIME_OP("FFT",
			fftw_execute(m_planFft)
	);
	unsigned int *outptr1 = (unsigned int *)m_fftOutImage->imageData;
	TIME_OP("MAG",
			ComputeMagnitude(outptr1,iw,ih)
	);
#endif

	XTIME_OP("SWAP",
		SwapQuad();
	);

	CvRect roiSize1, roiSize2;
	long int sum1=0, sum2=0;

	roiSize1.width = 2*m_xOffset;
	roiSize1.height = 2*m_yOffset;

	unsigned int *outptr2 = (unsigned int *)(m_fftMagImage->imageData + (ih/2 - m_yOffset)*m_fftMagImage->widthStep+ (iw/2 - m_xOffset)*4);
	XTIME_OP("SUM1",
		sum1 = ComputeSum(outptr2,m_fftMagImage->widthStep,roiSize1.width,roiSize1.height);
	);
//	printf("SUM1 %d\n",sum1);
	roiSize2.width = 4*m_xOffset;
	roiSize2.height = 4*m_yOffset;
	unsigned int *outptr = (unsigned int *)(m_fftMagImage->imageData + (ih/2 - 2*m_yOffset)*m_fftMagImage->widthStep + (iw/2 - 2*m_xOffset)*4);
	XTIME_OP("SUM2",
		sum2 = ComputeSum(outptr,m_fftMagImage->widthStep,roiSize2.width,roiSize2.height);
	);
//	printf("SUM2 %d\n",sum2);
	float mean = (sum2 - sum1)*1.0/(roiSize2.width*roiSize2.height - roiSize1.height*roiSize1.width);
//	printf("Mean %f\n",mean);
	int peakCount = 0;
	// Now try to locate some maxima's around
	// presence of symmetric set of maxima's near high frequencies indicate some funky business

	CvRect roiSize;
	roiSize.height = ih;
	roiSize.width = iw;

	unsigned int threshold = (unsigned int) (5*mean);

	int param[5]={0};
    param[0] = m_fftMagImage->width;//width
    param[1] = m_fftMagImage->height;//height
    param[2] = m_fftMagImage->widthStep;//inp widthstep
    param[3] = m_fftOutImage->widthStep;//out widthstep
    param[4] = (int)m_fftImage->imageData;

    //It will have the max buffer
	XTIME_OP("PEAK1",
		dilate_i((unsigned int *)m_fftMagImage->imageData,(unsigned int *)m_fftOutImage->imageData,(unsigned int *)param);
	);
	//Print("dilate_i",m_fftOutImage);
	XTIME_OP("PEAK2",
	{
		for(int i=3;i<m_fftMagImage->height-3;i++){
			unsigned int *inpbuf=(unsigned int *)(m_fftMagImage->imageData + i*(m_fftMagImage->widthStep));
			unsigned int *maxbuf=(unsigned int *)(m_fftOutImage->imageData + i*(m_fftOutImage->widthStep));
			inpbuf +=3;
			maxbuf +=3;
			for(int j=3;j<m_fftMagImage->width-3;j++){
				if(*inpbuf == *maxbuf){
					if(*maxbuf > threshold){
						m_PeakBuf[peakCount].x=j;
						m_PeakBuf[peakCount].y=i;
//						printf("(%d %d) = %d \n", m_PeakBuf[peakCount].x, m_PeakBuf[peakCount].y,*maxbuf);
						peakCount++;
					}
				}
				inpbuf++;
				maxbuf++;
			}
			if(peakCount > (m_maxPeakCount-m_fftMagImage->width)){
				printf("Exhausted the Peak Cnt buffer %d",peakCount);
				break;
			}
		}
	}
	);
//	printf("Peak Cnt %d\n",peakCount);
	unsigned int *magPtr = (unsigned int *)m_fftMagImage->imageData;
	int off = 2;
	int th = 1;
	roiSize1.height = roiSize1.width = off*2 + 1;
	roiSize2.height = roiSize2.width = (off+th)*2 + 1;
	int realPeakCount = 0;
	XTIME_OP("PEAKPROCESS",
	{
		for(int i=0;i<peakCount;i++){
			if(m_PeakBuf[i].x == iw/2 || m_PeakBuf[i].y == ih/2 || (abs(m_PeakBuf[i].x - iw/2) < m_xOffset && abs(m_PeakBuf[i].y - ih/2) < m_yOffset))
				continue;
			int index = (m_PeakBuf[i].y*m_fftMagImage->widthStep + m_PeakBuf[i].x*4)>>2;

			unsigned int *out1 = (unsigned int *)(m_fftMagImage->imageData + (m_PeakBuf[i].y - off)*m_fftMagImage->widthStep + (m_PeakBuf[i].x - off)*4);
			sum1 = ComputeSum(out1,m_fftMagImage->widthStep,roiSize1.width,roiSize1.height);
			unsigned int *out2 = (unsigned int *)(m_fftMagImage->imageData + (m_PeakBuf[i].y - off - th)*m_fftMagImage->widthStep + (m_PeakBuf[i].x - off - th)*4);
			sum2 = ComputeSum(out2,m_fftMagImage->widthStep,roiSize2.width,roiSize2.height);
			float mean = (sum2 - sum1)/(roiSize2.width*roiSize2.height - roiSize1.height*roiSize1.width);
			if(magPtr[index]/mean > m_peakThreshold){
				realPeakCount++;
	//			printf("(%d %d) = %d (%f %f)\n", m_PeakBuf[i].x, m_PeakBuf[i].y,magPtr[index],(float)mean,(float)(magPtr[index]/mean));
			}
		}
	}
	);
	return realPeakCount;
}


#endif
