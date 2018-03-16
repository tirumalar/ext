#include "pyramid.h"
#include "Image.h"
#include <cv.h>

#include "GaussPyr.h"
extern "C"{
	#include "file_manip.h"
	#include "EdgeImage_private.h"
}

PyrGauss::PyrGauss():scratch(0),scrImg(0)
{

}
PyrGauss::~PyrGauss()
{
	if(scrImg) cvReleaseImageHeader(&scrImg); scrImg=0;

}

#define GAUSSIAN3 1

#ifdef __BFIN__
int PyrGauss::PyrDown(Image8u* pSrc, Image8u* pDst)
{
	assert(scratch);
	XTIME_OP("reduce_gauss5",
	mipl_reduce_gauss5_b(pSrc->GetData(), pDst->GetData(),scratch)
	);
	return 0;
}
int PyrGauss::PyrDown4x(Image8u* pSrc, Image8u* pDst){
	assert(scratch);
	XTIME_OP("reduce_gauss5_4x",
			mipl_reduce_gauss5_4x_b(pSrc->GetData(), pDst->GetData(),scratch)
	);
	return 0;
}

int PyrGauss::HorzBinningDown4x(Image8u* pSrc, Image8u* pDst){
	assert(scratch);
	XTIME_OP("HorzBin_gauss5_4x",
			mipl_reduce_gauss5_HorzBinning(pSrc->GetData(), pDst->GetData(),scratch)
	);
	return 0;
}
int PyrGauss::VertBinningDown4x(Image8u* pSrc, Image8u* pDst){
	assert(scratch);
	XTIME_OP("VertBin_gauss5_4x",
			mipl_reduce_gauss5_VertBinning(pSrc->GetData(), pDst->GetData(),scratch)
	);
	return 0;
}

#else

int PyrGauss::PyrDown(Image8u* pSrc, Image8u* pDst)
{
	IplImage *in = pSrc->GetData();
	IplImage *out = pDst->GetData();
	int args[]={in->width,in->widthStep,in->height,out->widthStep,(int)scratch};

	assert((in->width>>2)<<2==in->width);//	in->width should be 4X
	assert((in->widthStep>>1)<<1==in->widthStep);//	in->width should be 2X
	assert((in->height>>1)<<1==in->height);//	in->height should be 2X
	reduce_gauss5_2x((unsigned char*)in->imageData,(unsigned char*)out->imageData,args);

//	cvPyrDown(pSrc->GetData(), pDst->GetData());
	return 0;
}
int PyrGauss::PyrDown4x(Image8u* pSrc, Image8u* pDst){

	assert(scratch);
	IplImage *in = pSrc->GetData();
	IplImage *out = pDst->GetData();

	if(!scrImg){
		scrImg=cvCreateImageHeader(cvSize(in->width>>2, in->height>>1),IPL_DEPTH_8U,1);
		cvSetData(scrImg,scratch,2*scrImg->width);
	}
	int args[]={in->width,in->widthStep,in->height,out->widthStep,(int)scratch,0};

	assert((in->width>>4)<<4==in->width);//	in->width should be 16X
	assert((in->widthStep>>2)<<2==in->widthStep);//	in->width should be 4X
	assert((in->height>>2)<<2==in->height);//	in->height should be 4X
	if(in->depth == out->depth){
	#if 1
		reduce_gauss5_4x((unsigned char*)in->imageData,(unsigned char*)out->imageData,args);
	#else
		mipl_reduce_gauss5_4x_b_noBorder(in,out,scrImg);
	#endif
	}
	else
	{	args[5]= 1;
		reduce_gauss5_16bit_4x_To_8bit((unsigned short*)in->imageData,(unsigned short*)out->imageData,args);
	}
	return 0;
}
#endif
int PyrGauss::PyrUp(Image8u* pSrc, Image8u* pDst)
{
	cvPyrUp(pSrc->GetData(), pDst->GetData());
	return 0;
}

IppiSize PyrGauss::GetDestSizeUp(Image8u* pSrc)
{
	IppiSize dstSize = { pSrc->GetSize().width*2, pSrc->GetSize().height*2 };
	return dstSize;
}

IppiSize PyrGauss::GetDestSizeDown(Image8u* pSrc)
{
	IppiSize dstSize = { pSrc->GetSize().width/2, pSrc->GetSize().height/2 };
	return dstSize;
}

//PyrGauss Pyramid8u::pyr_;

Pyramid8u::Pyramid8u()
{
	int i;
	for (i=0; i<MaxLevels; i++){
		pImages[i] = 0;
		pDoneLeveL[i]=false;
	}
	m_imageBinned=0;
}

Pyramid8u::~Pyramid8u()
{
	FreeAll_();
}

void Pyramid8u::FreeAll_()
{
	int i;
	for (i=1; i<MaxLevels; i++)
	{
		if (pImages[i] != 0)
		{
			delete pImages[i];
			pImages[i] = 0;
		}
	}
	if(m_imageBinned)
		delete m_imageBinned;
}
/*
 * special implemetation
 */
//void Pyramid8u::Reset(Image8u* pImage)
//{
//	pImages[0] = pImage;
//#ifdef __BFIN__
//	if(pImages[2]==0){
//		IppiSize dstSize = { pImages[0]->GetWidth()>>2, pImages[0]->GetHeight()>>2};
//		pImages[2] = new Image8u(dstSize, pImages[0]->GetChannelCount());
//	}
//	pyr_.PyrDown4x(pImages[0],pImages[2]);
//
//	if(pImages[3]==0){
//		IppiSize dstSize = { pImages[0]->GetWidth()>>3, pImages[0]->GetHeight()>>3};
//		pImages[3] = new Image8u(dstSize, pImages[0]->GetChannelCount());
//	}
//	pyr_.PyrDown(pImages[2],pImages[3]);
//
//#else
//	FreeAll_();
//#endif
//
//}

void Pyramid8u::Reset(Image8u* pImage,int binning)
{

	int i;
	for (i=0; i<MaxLevels; i++){
		pDoneLeveL[i]=false;
	}


	if(binning)
	{
		m_imageBinned=pImage;
	}
	else{

	}
	bool alloc = pImages[2]?true:false;
#ifdef __BFIN__
	if(binning == 1){
//		if(pImages[0]==0)
//			pImages[0]= new Image8u(pImage->GetWidth(),pImage->GetHeight()<<1,pImage->GetChannelCount());
		if(!alloc){
			IppiSize dstSize = { pImage->GetWidth()>>2, pImage->GetHeight()>>1};
			pImages[2] = new Image8u(dstSize, pImage->GetChannelCount());
		}
		//Means 2592*976->648*486
		pyr_.HorzBinningDown4x(pImage,pImages[2]);
		pDoneLeveL[2]=true;
//		cvSaveImage("Img0Hor.pgm",pImages[0]->GetData());
//		cvSaveImage("Img2Hor.pgm",pImages[2]->GetData());
	}
	else if(binning == 2){
//		if(pImages[0]==0)
//			pImages[0]= new Image8u(pImage->GetWidth()<<1,pImage->GetHeight(),pImage->GetChannelCount());
		if(pImages[2]==0){
			IppiSize dstSize = {pImage->GetWidth()>>1, pImage->GetHeight()>>2};
			pImages[2] = new Image8u(dstSize, pImage->GetChannelCount());
		}
		//Means 1296*1944->648*486
		pyr_.VertBinningDown4x(pImage,pImages[2]);
		pDoneLeveL[2]=true;
//		cvSaveImage("Img0Ver.pgm",pImages[0]->GetData());
//		cvSaveImage("Img2Ver.pgm",pImages[2]->GetData());
	}
	else
	{
		pImages[0] = pImage;
		pDoneLeveL[0]=true;
		if(pImages[2]==0){
			IppiSize dstSize = { pImages[0]->GetWidth()>>2, pImages[0]->GetHeight()>>2};
			pImages[2] = new Image8u(dstSize, pImages[0]->GetChannelCount());
		}
		pyr_.PyrDown4x(pImages[0],pImages[2]);
		pDoneLeveL[2]=true;
	}

	if(pImages[3]==0){
		IppiSize dstSize = { pImages[2]->GetWidth()>>1, pImages[2]->GetHeight()>>1};
		pImages[3] = new Image8u(dstSize, pImages[0]->GetChannelCount());
	}
	pyr_.PyrDown(pImages[2],pImages[3]);
	pDoneLeveL[3]=true;
	//cvSaveImage("Img3.pgm",pImages[3]->GetData());

#else

	#ifdef USINGOPENCVPYRAMID
		pImages[0] = pImage;
		pDoneLeveL[0]=true;
		if(pImages[1]==0){
			IppiSize dstSize = { pImages[0]->GetWidth()>>1, pImages[0]->GetHeight()>>1};
			pImages[1] = new Image8u(dstSize, pImages[0]->GetChannelCount());
		}
		TIME_OP("L0",
		pyr_.PyrDown(pImages[0],pImages[1])
		);
		pDoneLeveL[1]=true;
		if(pImages[2]==0){
			IppiSize dstSize = { pImages[1]->GetWidth()>>1, pImages[1]->GetHeight()>>1};
			pImages[2] = new Image8u(dstSize, pImages[0]->GetChannelCount());
		}
		TIME_OP("L1",
		pyr_.PyrDown(pImages[1],pImages[2])
		);
		pDoneLeveL[2]=true;
		if(pImages[3]==0){
			IppiSize dstSize = { pImages[2]->GetWidth()>>1, pImages[2]->GetHeight()>>1};
			pImages[3] = new Image8u(dstSize, pImages[0]->GetChannelCount());
		}
		TIME_OP("L2",
		pyr_.PyrDown(pImages[2],pImages[3])
		);
		pDoneLeveL[3]=true;
	#else

		pImages[0] = pImage;
		pDoneLeveL[0]=true;
		if(pImages[2]==0){
			IppiSize dstSize = { pImages[0]->GetWidth()>>2, pImages[0]->GetHeight()>>2};
			pImages[2] = new Image8u(dstSize, pImages[0]->GetChannelCount());
		}
		XTIME_OP("L0",
		pyr_.PyrDown4x(pImages[0],pImages[2])
		);
		pDoneLeveL[2]=true;

#if 0
		if(pImages[1]==0){
			IppiSize dstSize = { pImages[0]->GetWidth()>>1, pImages[0]->GetHeight()>>1};
			pImages[1] = new Image8u(dstSize, pImages[0]->GetChannelCount());
		}
		XTIME_OP("L1",
		pyr_.PyrDown(pImages[0],pImages[1])
		);
		pDoneLeveL[1]=true;
//		cvSaveImage("Img0.pgm",pImages[0]->GetData());
//		cvSaveImage("Img2.pgm",pImages[2]->GetData());
#endif

		if(pImages[3]==0){
			IppiSize dstSize = { pImages[2]->GetWidth()>>1, pImages[2]->GetHeight()>>1};
			pImages[3] = new Image8u(dstSize, pImages[0]->GetChannelCount());
		}
		XTIME_OP("L2",
		pyr_.PyrDown(pImages[2],pImages[3])
		);
		pDoneLeveL[3]=true;
//		for(int i=0;i<3;i++){
//			char fname[100];
//			sprintf(fname,"/data/OutImage%d.pgm",i);
//			IplImage *ptr = pImages[i]->GetData();
//			savefile_OfSize_asPGM((unsigned char*)ptr->imageData,ptr->width,ptr->height,fname);
//		}

	#endif
#endif

}

Image8u* Pyramid8u::GetLevel(int level)
{
	int i;
	if (level == 0) {
		return pImages[0];
	}
	if (level >= MaxLevels)
		level = MaxLevels-1;
	if((pImages[level]) && (pDoneLeveL[level]==true)) return pImages[level];
#ifdef __BFIN__
	if (level==2 && pImages[level]==0){
		IppiSize dstSize = { pImages[0]->GetWidth()>>2, pImages[0]->GetHeight()>>2};
		pImages[level] = new Image8u(dstSize, pImages[0]->GetChannelCount());
		pyr_.PyrDown4x(pImages[0],pImages[2]);
		pDoneLeveL[2]=true;
		return pImages[level];
	}
#endif
	for (i=1; i<=level; i++)
	{
		if (!pImages[i]){
			pImages[i] = new Image8u(pyr_.GetDestSizeDown(pImages[i-1]), pImages[i-1]->GetChannelCount());
			if ((pImages[i]->GetSize().width == 0) || (pImages[i]->GetSize().height == 0))
				return 0;
		}
		if(pDoneLeveL[i]==false){
			pyr_.PyrDown(pImages[i-1], pImages[i]);
			pDoneLeveL[i] = true;
		}
	}

	return pImages[level];
}


