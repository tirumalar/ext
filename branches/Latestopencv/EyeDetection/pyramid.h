#if !defined _INCLUDED_PYRAMID_H
#define _INCLUDED_PYRAMID_H

#include "Image.h"

class HBOX_API PyrGauss
{
	private:
	char *scratch;
	IplImage *scrImg;
	public:
		PyrGauss();
		~PyrGauss();

		int PyrUp(Image8u* pSrc, Image8u* pDst);
		int PyrDown(Image8u* pSrc, Image8u* pDst);
		int PyrDown4x(Image8u* pSrc, Image8u* pDst);
		int HorzBinningDown4x(Image8u* pSrc, Image8u* pDst);
		int VertBinningDown4x(Image8u* pSrc, Image8u* pDst);
		IppiSize GetDestSizeUp(Image8u* pSrc);
		IppiSize GetDestSizeDown(Image8u* pSrc);
		void setScratch(char *scr) {scratch=scr;}
};

class HBOX_API Pyramid8u
{
	public:
		enum { MaxLevels = 6};

		Pyramid8u();
		~Pyramid8u();
		Image8u* GetLevel(int level);
		void Reset(Image8u* pImage,int bin=0);
		void setScratch(char *scr) {pyr_.setScratch(scr);}
	private:
		/*static*/ PyrGauss pyr_;

		Image8u* pImages[MaxLevels];	//usual pyramid
		Image8u* m_imageBinned;			//special 0.5 level image
		bool pDoneLeveL[MaxLevels];
		void FreeAll_();
};

#endif
