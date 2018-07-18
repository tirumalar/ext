#include <fstream>
#include "Image.h"

typedef Ipp32f REAL;

struct SimpleROI
{
	int x, y, w, h;
};

class HBOX_API SimpleClassifier
{
public:
	REAL thresh;
	REAL error;
	int parity;
	int index;
	int type; // which type of feature?
	int x1,x2,x3,x4,y1,y2,y3,y4;

	int GetNumberOfHaarFeatures();
	void GetFeatureInfo(SimpleROI *roi, REAL *featureWeight);

	int GetParity() const { return parity; }
	REAL GetThreshold() const { return thresh; }

	inline REAL GetOneFeature(Image32s *im, double scale);
	inline REAL GetOneFeature(Image32s *im, IppiPoint off, double scale);
	inline REAL GetOneFeatureTranslation(Image32s *im,const int y);
	int Apply(REAL value);
	int Apply(Image32s *im, double scale);
	int Apply(Image32s *im, IppiPoint off, double scale);
	void WriteToFile(std::ofstream& f);
	void ReadFromFile(std::ifstream& f ,int maxtype=4);

//Maddy added for Spoof
	inline REAL GetOneFeature(Image32s *im0, double scale0, Image32s *im1, double scale1);
	int Apply(Image32s *im0, double scale0, Image32s *im1, double scale1);

};


