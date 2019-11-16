#ifndef _TRAINING_SET_H
#define _TRAINING_SET_H

#include <fstream>
#include <string>
#include "Image.h"

class HBOX_API TrainingSet
{
public:
	TrainingSet(void *scr=0):m_data(NULL),m_intImg(NULL),m_sqintImg(NULL),scratch(scr) {}
	~TrainingSet();

	int ReadFromFile(std::ifstream& f, short id, char cl);
	short m_id;
	char m_cl;
	IplImage *m_data;
	int m_stride;
	Ipp64f m_variance;
	std::string m_fileName;

	Image32s *GetIntegralImage() { return m_intImg;}
	void ComputeIntegralImage(Image8u *img) { return ComputeIntegralImage(img->GetData()); }
	Ipp64f GetInvVariance(Image8u *img, IppiRect rect);
	void ComputeIntegralImageOnly(Image8u *img){ return ComputeIntegralImageOnly(img->GetData()); }
	void ComputeIntegralImageOnly(IplImage *img);
protected:
	void ComputeIntegralImage(IplImage *img);
	Image32s *m_intImg;
#ifdef __BFIN__
	Image32s *m_sqintImg;
#else
	Image64f *m_sqintImg;
#endif
	void *scratch;
};

#endif
