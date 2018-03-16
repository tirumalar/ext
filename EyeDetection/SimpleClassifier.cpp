#include <fstream>
//#include <assert.h>
#include "SimpleClassifier.h"

int SimpleClassifier::Apply(Image32s *im, double scale)
{
	if(parity == 1)
		return (GetOneFeature(im, scale)<thresh)?1:0;
	else
		return (GetOneFeature(im, scale)>=thresh)?1:0;
}

int SimpleClassifier::Apply(Image32s *im, IppiPoint off, double scale)
{
	if(parity == 1)
		return (GetOneFeature(im, off, scale)<thresh)?1:0;
	else
		return (GetOneFeature(im, off, scale)>=thresh)?1:0;
}

int SimpleClassifier::Apply(REAL value)
{
	if(parity == 1)
		return (value<thresh)?1:0;
	else
		return (value>=thresh)?1:0;
}

int SimpleClassifier::GetNumberOfHaarFeatures()
{
	switch(type)
	{
	case 0:
		return 2;
	case 1:
		return 2;
	case 2:
		return 2;
	case 3:
		return 2;
	case 4:
		return 3;
	}

	return 0;
}

void SimpleClassifier::GetFeatureInfo(SimpleROI *roi, REAL *featureWeight)
{
	switch(type)
	{
	case 0:

		/*
		A ---------------- D
		|                  | 
		B ---------------- E
		|                  | 
		C ---------------- F

		Type 0 feature,
		Feature value is BCEF-ABDE = (B+F-C-E)-(A+E-B-D) = D+F-A-C+2(B-E)
		Coordinates: x1,x2,x3 (vertical) and y1,y3 (horizontal), e.g. E=(x2,y3)

		*/

		// BCEF
		roi[0].x = y1;
		roi[0].y = x2;
		roi[0].w = y3 - y1;
		roi[0].h = x2 - x1;

		featureWeight[0] = 1.0;

		// ABDE
		roi[1].x = y1;
		roi[1].y = x1;
		roi[1].w = y3 - y1;
		roi[1].h = x2 - x1;

		featureWeight[1] = -1.0;
		
		break;
	case 1:

		/*
		A ------- B -------- C
		|         |          |
		D ------- E -------- F

		Type 1 feautre, 
		Feature value is  BCEF-ABDE = (B+F-C-E)-(A+E-B-D) = D+F-A-C+2(B-E)
		Coordinates: x1,x3 (vertical) and y1,y2,y3 (horizontal), e.g. E=(x3,y2)

		*/	

		// BCEF
		roi[0].x = y2;
		roi[0].y = x1;
		roi[0].w = y3 - y2;
		roi[0].h = x3 - x1;

		featureWeight[0] = 1.0;

		// ABDE
		roi[1].x = y1;
		roi[1].y = x1;
		roi[1].w = y2 - y1;
		roi[1].h = x3 - x1;

		featureWeight[1] = -1.0;

		break;
	case 2:

		/*
		A ---------------- E
		|                  | 
		B ---------------- F
		|                  | 
		C ---------------- G
		|                  | 
		D ---------------- H

		Type 2 feature,
		Feature value is CDGH+ABEF-2BCFG = ADEH-3BCFG = A+H-D-E+3(C+F-B-G)
		Coordinate: x1,x2,x3,x4 (vertical) and y1,y3 (horizontal), e.g. C=(x3,y1)	

		*/

		// ADEH
		roi[0].x = y1;
		roi[0].y = x1;
		roi[0].w = y3 - y1;
		roi[0].h = x4 - x1;
		featureWeight[0] = 1.0;

		// BCFG
		roi[1].x = y1;
		roi[1].y = x2;
		roi[1].w = y3 - y1;
		roi[1].h = x3 - x2;
		featureWeight[1] = -3.0;

		break;
	case 3:

		/*
		A ------- B ------ C -------- D
		|         |        |          |
		E ------- F ------ G -------- H

		Type 3 features,
		Feature value is CDGH+ABEF-2BCFG = ADEH-3BCFG = A+H-D-E +3(C+F-B-G)
		Coordinate: x1,x3 (vertical) and y1,y2,y3,y4 (horizontal), e.g. C=(x1,y3)	

		*/		

		// ADEH
		roi[0].x = y1;
		roi[0].y = x1;
		roi[0].w = y4 - y1;
		roi[0].h = x3 - x1;
		featureWeight[0] = 1.0;

		// BCFG
		roi[1].x = y2;
		roi[1].y = x1;
		roi[1].w = y3 - y2;
		roi[1].h = x3 - x1;
		featureWeight[1] = -3.0;

		break;
	case 4:

		/*
		A ------- B -------- C
		|         |          |
		D ------- E -------- F
		|         |          |
		G ------- H -------- I

		Type 4 features,
		Feature value is ABDE+EFHI-BCEF-DEGH 
		= ACGI - 2(BCEF) - 2(DEGH)
		= (A+E-B-D)+(E+I-H-F)+(E+G-D-H)+(E+C-B-F) 
		= 4E � 2(B+D-H-F)+A+C+G+I
		Coordinate: x1,x2,x3 (vertical) and y1,y2,y3 (horizontal), e.g. F=(x2,y3).

		*/

		// ACGI
		roi[0].x = y1;
		roi[0].y = x1;
		roi[0].w = y3 - y1;
		roi[0].h = x3 - x1;
		featureWeight[0] = 1.0;

		// BCEF
		roi[1].x = y2;
		roi[1].y = x1;
		roi[1].w = y3 - y2;
		roi[1].h = x2 - x1;
		featureWeight[1] = -2.0;

		// DEGH
		roi[2].x = y1;
		roi[2].y = x2;
		roi[2].w = y2 - y1;
		roi[2].h = x3 - x2;
		featureWeight[2] = -2.0;

		break;
	}
}

REAL SimpleClassifier::GetOneFeature(Image32s *im, IppiPoint off, double scale )
{
	Ipp32s f1;
	Ipp32s* data = &im->At(off.y,off.x);
	int stride = (im->GetStride())/sizeof(Ipp32s);

	switch(type)
	{
	case 0:
		f1 =   data[x1*stride+y3] - data[x1*stride+y1] + data[x3*stride+y3] - data[x3*stride+y1]
			 + 2*(data[x2*stride+y1] - data[x2*stride+y3]);
		break;
	case 1:
		f1 =   data[x3*stride+y1] + data[x3*stride+y3] - data[x1*stride+y1] - data[x1*stride+y3]
			 + 2*(data[x1*stride+y2] - data[x3*stride+y2]);
		break;
	case 2:
		f1 =   data[x1*stride+y1] -data[x1*stride+y3] + data[x4*stride+y3] - data[x4*stride+y1]
			 + 3*(data[x2*stride+y3] - data[x2*stride+y1] + data[x3*stride+y1] - data[x3*stride+y3]);
		break;
	case 3:
		f1 =   data[x1*stride+y1] - data[x1*stride+y4] + data[x3*stride+y4] - data[x3*stride+y1]
			 + 3*(data[x3*stride+y2] - data[x3*stride+y3] + data[x1*stride+y3] - data[x1*stride+y2] );
		break;
	case 4:
		f1 =   data[x1*stride+y1] + data[x1*stride+y3] + data[x3*stride+y1] + data[x3*stride+y3]
			 - 2*(data[x2*stride+y1] + data[x2*stride+y3] + data[x1*stride+y2] + data[x3*stride+y2])
			 + 4*data[x2*stride+y2];
		break;
	}
	return (REAL) (f1 * scale);
}

REAL SimpleClassifier::GetOneFeature(Image32s *im, double scale )
{
	Ipp32s f1;
	Ipp32s* data = &im->At(0,0);
	int stride = (im->GetStride())/sizeof(Ipp32s);

	switch(type)
	{
	case 0:
		f1 =   data[x1*stride+y3] - data[x1*stride+y1] + data[x3*stride+y3] - data[x3*stride+y1]
			 + 2*(data[x2*stride+y1] - data[x2*stride+y3]);
		break;
	case 1:
		f1 =   data[x3*stride+y1] + data[x3*stride+y3] - data[x1*stride+y1] - data[x1*stride+y3]
			 + 2*(data[x1*stride+y2] - data[x3*stride+y2]);
		break;
	case 2:
		f1 =   data[x1*stride+y1] -data[x1*stride+y3] + data[x4*stride+y3] - data[x4*stride+y1]
			 + 3*(data[x2*stride+y3] - data[x2*stride+y1] + data[x3*stride+y1] - data[x3*stride+y3]);
		break;
	case 3:
		f1 =   data[x1*stride+y1] - data[x1*stride+y4] + data[x3*stride+y4] - data[x3*stride+y1]
			 + 3*(data[x3*stride+y2] - data[x3*stride+y3] + data[x1*stride+y3] - data[x1*stride+y2] );
		break;
	case 4:
		f1 =   data[x1*stride+y1] + data[x1*stride+y3] + data[x3*stride+y1] + data[x3*stride+y3]
			 - 2*(data[x2*stride+y1] + data[x2*stride+y3] + data[x1*stride+y2] + data[x3*stride+y2])
			 + 4*data[x2*stride+y2];
		break;
	}
	return (REAL) (f1 * scale);
}

REAL SimpleClassifier::GetOneFeatureTranslation(Image32s *im,const int y)
{
	Ipp32s f1;
	Ipp32s* data = &im->At(0,0);
	int stride = im->GetStride()/sizeof(Ipp32s);

	switch(type)
	{
	case 0:
		f1 =   data[x1*stride+y+y3] - data[x1*stride+y+y1] + data[x3*stride+y+y3] - data[x3*stride+y+y1]
			 + 2*(data[x2*stride+y+y1] - data[x2*stride+y+y3]);
		break;
	case 1:
		f1 =   data[x3*stride+y+y1] + data[x3*stride+y+y3] - data[x1*stride+y+y1] - data[x1*stride+y+y3]
			 + 2*(data[x1*stride+y+y2] - data[x3*stride+y+y2]);
		break;
	case 2:
		f1 =   data[x1*stride+y+y1] - data[x1*stride+y+y3] + data[x4*stride+y+y3] - data[x4*stride+y+y1]
			 + 3*(data[x2*stride+y+y3] - data[x2*stride+y+y1] + data[x3*stride+y+y1]  - data[x3*stride+y+y3]);
		break;
	case 3:
		f1 =   data[x1*stride+y+y1] - data[x1*stride+y+y4] + data[x3*stride+y+y4] - data[x3*stride+y+y1]
			 + 3*(data[x3*stride+y+y2] - data[x3*stride+y+y3] + data[x1*stride+y+y3] - data[x1*stride+y+y2]);
		break;
	case 4:
		f1 =   data[x1*stride+y+y1] + data[x1*stride+y+y3] + data[x3*stride+y+y1] + data[x3*stride+y+y3]
			 - 2*(data[x2*stride+y+y1] + data[x2*stride+y+y3] + data[x1*stride+y+y2] + data[x3*stride+y+y2])
			 + 4*data[x2*stride+y+y2];
		break;
	default:
		break;
	}
	return (REAL) f1;
}

void SimpleClassifier::ReadFromFile(std::ifstream& f,int maxtype)
{
	f>>thresh>>parity>>type;
	f>>x1>>x2>>x3>>x4>>y1>>y2>>y3>>y4;
	f.ignore(256,'\n');
	assert(parity == 0 || parity == 1);
	assert(type>=0 && type<=maxtype);
}

void SimpleClassifier::WriteToFile(std::ofstream& f)
{
	f<<thresh<<" ";
	f<<parity<<" ";
	f<<type<<" ";
	f<<x1<<" ";
	f<<x2<<" ";
	f<<x3<<" ";
	f<<x4<<" ";
	f<<y1<<" ";
	f<<y2<<" ";
	f<<y3<<" ";
	f<<y4<<" ";
	f<<std::endl;
}

//Maddy added for Spoof
int SimpleClassifier::Apply(Image32s *im0, double scale0, Image32s *im1, double scale1)
{
	if(parity == 1)
		return (GetOneFeature(im0, scale0, im1, scale1)<thresh)?1:0;
	else
		return (GetOneFeature(im0, scale0, im1, scale1)>=thresh)?1:0;
}

REAL SimpleClassifier::GetOneFeature(Image32s *im0, double scale0, Image32s *im1, double scale1 )
{
	Ipp32s f1, f2;

	Ipp32s* data = (Ipp32s*)(im0->GetData())->imageData;//Maddy im0->GetData();
	Ipp32s* datb = (Ipp32s*)(im1->GetData())->imageData;//Maddy im1->GetData();
	int stride = (im0->GetStride())/sizeof(Ipp32s);

	switch(type)
	{
	case 0:
		f1 =   data[x1*stride+y3] - data[x1*stride+y1] + data[x3*stride+y3] - data[x3*stride+y1]
			 + 2*(data[x2*stride+y1] - data[x2*stride+y3]);
		f2 =   datb[x1*stride+y3] - datb[x1*stride+y1] + datb[x3*stride+y3] - datb[x3*stride+y1]
			 + 2*(datb[x2*stride+y1] - datb[x2*stride+y3]);
		break;
	case 1:
		f1 =   data[x3*stride+y1] + data[x3*stride+y3] - data[x1*stride+y1] - data[x1*stride+y3]
			 + 2*(data[x1*stride+y2] - data[x3*stride+y2]);
		f2 =   datb[x3*stride+y1] + datb[x3*stride+y3] - datb[x1*stride+y1] - datb[x1*stride+y3]
			 + 2*(datb[x1*stride+y2] - datb[x3*stride+y2]);
		break;
	case 2:
		f1 =   data[x1*stride+y1] -data[x1*stride+y3] + data[x4*stride+y3] - data[x4*stride+y1]
			 + 3*(data[x2*stride+y3] - data[x2*stride+y1] + data[x3*stride+y1] - data[x3*stride+y3]);
		f2 =   datb[x1*stride+y1] -datb[x1*stride+y3] + datb[x4*stride+y3] - datb[x4*stride+y1]
			 + 3*(datb[x2*stride+y3] - datb[x2*stride+y1] + datb[x3*stride+y1] - datb[x3*stride+y3]);
		break;
	case 3:
		f1 =   data[x1*stride+y1] - data[x1*stride+y4] + data[x3*stride+y4] - data[x3*stride+y1]
			 + 3*(data[x3*stride+y2] - data[x3*stride+y3] + data[x1*stride+y3] - data[x1*stride+y2] );
		f2 =   datb[x1*stride+y1] - datb[x1*stride+y4] + datb[x3*stride+y4] - datb[x3*stride+y1]
			 + 3*(datb[x3*stride+y2] - datb[x3*stride+y3] + datb[x1*stride+y3] - datb[x1*stride+y2] );
		break;
	case 4:
		f1 =   data[x1*stride+y1] + data[x1*stride+y3] + data[x3*stride+y1] + data[x3*stride+y3]
			 - 2*(data[x2*stride+y1] + data[x2*stride+y3] + data[x1*stride+y2] + data[x3*stride+y2])
			 + 4*data[x2*stride+y2];
		f2 =   datb[x1*stride+y1] + datb[x1*stride+y3] + datb[x3*stride+y1] + datb[x3*stride+y3]
			 - 2*(datb[x2*stride+y1] + datb[x2*stride+y3] + datb[x1*stride+y2] + datb[x3*stride+y2])
			 + 4*datb[x2*stride+y2];
		break;
	case 5:
		f1 =   data[x1*stride+y1] -data[x1*stride+y3] + data[x2*stride+y3] - data[x2*stride+y1]
			- (data[x3*stride+y1] -data[x3*stride+y3] + data[x4*stride+y3] - data[x4*stride+y1]);
		f2 =   datb[x1*stride+y1] -datb[x1*stride+y3] + datb[x2*stride+y3] - datb[x2*stride+y1]
			- (datb[x3*stride+y1] -datb[x3*stride+y3] + datb[x4*stride+y3] - datb[x4*stride+y1]);
		break;
	case 6:
		f1 =   data[x1*stride+y1] - data[x1*stride+y2] + data[x3*stride+y2] - data[x3*stride+y1]
			 -(data[x1*stride+y3] - data[x1*stride+y4] + data[x3*stride+y4] - data[x3*stride+y3] );
		f2 =   datb[x1*stride+y1] - datb[x1*stride+y2] + datb[x3*stride+y2] - datb[x3*stride+y1]
			 -(datb[x1*stride+y3] - datb[x1*stride+y4] + datb[x3*stride+y4] - datb[x3*stride+y3] );
		break;
	}
	return (REAL) (f1 * scale0 * f2 * scale1);
}
