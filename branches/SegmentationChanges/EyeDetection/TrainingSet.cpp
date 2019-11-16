#include "trainingset.h"

#ifdef __BFIN__
#include "GaussPyr.h"
TrainingSet::~TrainingSet()
{
	if(m_intImg != NULL)
	{
		cvReleaseImageHeader(&(m_intImg->m_Data));
		delete m_intImg;
		m_intImg = NULL;
	}

	if(m_sqintImg != NULL)
	{
		cvReleaseImageHeader(&(m_sqintImg->m_Data));
		delete m_sqintImg;
		m_sqintImg = NULL;
	}
}
void TrainingSet::ComputeIntegralImage(IplImage *img)
{
	assert(scratch);
	char *scr_pos=(char *)scratch;
	if(!m_intImg)
	{
		// allocate an m_intImg from scratch
		IplImage *intImageHeader=cvCreateImageHeader(cvSize(img->width+1, img->height+1),IPL_DEPTH_32S,1);
		intImageHeader->imageData=scr_pos;
		m_intImg = new Image32s(intImageHeader,false);
	}
	else if(m_intImg->GetHeight() != img->height+1 || m_intImg->GetWidth() != img->width+1)
	{
		assert(false);
	}
	scr_pos+=m_intImg->m_Data->imageSize;

	if(!m_sqintImg)
	{
		// allocate an m_sqintImg from scratch
		IplImage *intImageHeader=cvCreateImageHeader(cvSize(img->width+1, img->height+1),IPL_DEPTH_32S,1);
		intImageHeader->imageData=scr_pos;
		m_sqintImg = new Image32s(intImageHeader,false);
	}
	else if(m_sqintImg->GetHeight() != img->height+1 || m_sqintImg->GetWidth() != img->width+1)
	{
		assert(false);
	}

	scr_pos+=m_sqintImg->m_Data->imageSize;
	mipl_integral_with_square_image(img, m_intImg->GetData(), m_sqintImg->GetData(), scr_pos);
	m_data = m_intImg->GetData(); // + m_intImg->GetStride() + 1;
	m_stride = m_intImg->GetStride();
}

Ipp64f TrainingSet::GetInvVariance(Image8u *img, IppiRect rect)
{
	Ipp32s varVal = m_sqintImg->At(rect.y, rect.x) - m_sqintImg->At(rect.y+rect.height, rect.x)
		+ m_sqintImg->At(rect.y+rect.height, rect.x+rect.width) - m_sqintImg->At(rect.y, rect.x+rect.width);

	Ipp32s sumVal = m_intImg->At(rect.y, rect.x) - m_intImg->At(rect.y+rect.height, rect.x)
		+ m_intImg->At(rect.y+rect.height, rect.x+rect.width) - m_intImg->At(rect.y, rect.x+rect.width);

	Ipp64f invN = 1.0/(rect.width * rect.height);
	Ipp64f meanVal=sumVal*invN;
	return 1.0/sqrt(varVal*invN - meanVal*meanVal);
}
#else


TrainingSet::~TrainingSet()
	{
		if(m_intImg != NULL)
		{
			delete m_intImg;
			m_intImg = NULL;
		}


		if(m_sqintImg != NULL)
		{
			delete m_sqintImg;
			m_sqintImg = NULL;
		}
	}
void TrainingSet::ComputeIntegralImage(IplImage *img)
{
	if(!m_intImg)
	{
		m_intImg = new Image32s(img->width+1, img->height+1);
	}
	else if(m_intImg->GetHeight() != img->height+1 || m_intImg->GetWidth() != img->width+1)
	{
		delete m_intImg;

		m_intImg = new Image32s(img->width+1, img->height+1);
	}
	if(!m_sqintImg)
	{
		m_sqintImg = new Image64f(img->width+1, img->height+1);
	}
	else if(m_sqintImg->GetHeight() != img->height+1 || m_sqintImg->GetWidth() != img->width+1)
	{
		delete m_sqintImg;
		m_sqintImg = new Image64f(img->width+1, img->height+1);
	}
	cvIntegral(img, m_intImg->GetData(), m_sqintImg->GetData());
	m_data = m_intImg->GetData(); // + m_intImg->GetStride() + 1;
	m_stride = m_intImg->GetStride();
}

Ipp64f TrainingSet::GetInvVariance(Image8u *img, IppiRect rect)
{

	CvScalar mean, std_dev;

	cvSetImageROI(img->GetData(),rect);

	cvAvgSdv(img->GetData(), &mean, &std_dev);

	cvResetImageROI(img->GetData());

	return 1.0/std_dev.val[0];
}
#endif

void TrainingSet::ComputeIntegralImageOnly(IplImage *img)
{
	IppiSize roi = {img->width, img->height};

	if(!m_intImg)
	{
		m_intImg = new Image32s(img->width+1, img->height+1);
	}
//	IplImage *in,*out;
//	in = img->GetData();
//	out= m_intImg->GetData();
//Maddy	IppStatus status = ippiIntegral_8u32s_C1R(img->GetData(), img->GetStride(), m_intImg->GetData(), m_intImg->GetStride(),	roi, 0);
	cvIntegral(img,m_intImg->GetData());

//Maddy	ippiMean_StdDev_8u_C1R(img->GetData(), img->GetStride(), roi, &mean, &std_dev);
	CvScalar mean, std_dev;
//	cvSetImageROI(img->GetData(),rect);
	cvAvgSdv(img, &mean, &std_dev);
	m_variance = (std_dev.val[0] > 0.0)? 1.0/std_dev.val[0] : 0.0;
	//Maddym_variance = (std_dev > 0.0)? 1.0/std_dev : 0.0;

	m_data = m_intImg->GetData(); // + m_intImg->GetStride() + 1;
	m_stride = m_intImg->GetStride();
}

int TrainingSet::ReadFromFile(std::ifstream& f, short id, char cl)
{
	assert(false);
	return -1;
}


