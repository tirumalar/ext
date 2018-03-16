#include "videoframe.h"
extern "C"{
#include "file_manip.h"
}

CSampleFrame::CSampleFrame(Image8u *frame){
	m_Binning = 0;
	m_Frame=frame;
	m_Pyramid= new Pyramid8u();
	m_Pyramid->Reset(frame);
	m_pEyeCenterPoints= new EyeCenterPointList();
	m_NumberOfHaarEyes=0;
	m_NumberOfSpecularityEyes=0;
	m_ObjectDistance=0.0f;
	m_FocusDistance=0.0f;

}

CSampleFrame::CSampleFrame(){
	m_Binning = 0;
	m_Frame=0;
	m_Pyramid= new Pyramid8u();
	m_pEyeCenterPoints= new EyeCenterPointList();
	m_NumberOfHaarEyes=0;
	m_NumberOfSpecularityEyes=0;
	m_ObjectDistance=0.0f;
	m_FocusDistance=0.0f;
}

CSampleFrame::~CSampleFrame(){
	if(m_Pyramid) delete m_Pyramid;
	if(m_pEyeCenterPoints) delete m_pEyeCenterPoints;
}
/*
 * Reset it to a new image
 */
void CSampleFrame::SetImage(Image8u *frame) {
	m_Frame=frame;
	XTIME_OP("Pyramid",
	m_Pyramid->Reset(frame,m_Binning)
	);
	m_pEyeCenterPoints->clear();
	m_NumberOfHaarEyes=0;
	m_NumberOfSpecularityEyes=0;
	m_ObjectDistance=0.0f;
	m_FocusDistance=0.0f;
}

void CSampleFrame::GetCroppedEye(int index, IplImage *dest, int& left, int & top){
	assert(index>=0);
	assert(index<m_NumberOfHaarEyes);
	cvSetZero(dest);
	CEyeCenterPoint& eye=GetEyeCenterPointList()->at(index);
	CvPoint pt = cvPoint(eye.m_nCenterPointX,eye.m_nCenterPointY);
	if(m_Binning == 1){
		pt.y = pt.y>>1;
	}
	if(m_Binning == 2){
		pt.x = pt.x>>1;
	}
	m_Frame->CopyROIInto(dest,pt,left,top);
}
void CSampleFrame::getDims(int level, int& width, int& height)
{
	Image8u * img=m_Pyramid->GetLevel(level);
	assert(img!=0);
	width=img->GetWidth();
	height=img->GetHeight();
}
void CSampleFrame::saveImage(const char* prefix, int idx)
{
	if(m_Frame->GetData()->depth ==8)
		savefile_OfSize_asPGM_index((unsigned char *)m_Frame->GetData()->imageData,m_Frame->GetWidth(),m_Frame->GetHeight(),prefix,idx);
	else
		savefile_OfSize_asPGM16_index((unsigned char *)m_Frame->GetData()->imageData,m_Frame->GetWidth(),m_Frame->GetHeight(),prefix,idx);
}

void CSampleFrame::saveImage(const char* prefix, int idx,int level)
{
	if(m_Frame->GetData()->depth ==8){
		Image8u *img=m_Pyramid->GetLevel(level);
		if(img->GetData()->imageData!= NULL){
			savefile_OfSize_asPGM_index((unsigned char *)img->GetData()->imageData,img->GetWidth(),img->GetHeight(),prefix,idx);
		}
	}
}
