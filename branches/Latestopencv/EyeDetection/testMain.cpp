#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <tut/tut.hpp>
#include <tut/tut_reporter.hpp>
#include "videoframe.h"
#include "EyeDetectAndMatchServer.h"
#include <cv.h>
#include <cxcore.h>
#include <highgui.h>
#ifdef __BFIN__
#include "HaarClassification.h"
#include "MIPLClassificationParams.h"
	extern "C"{
	#include "file_manip.h"
	}
#else
#define TIME_OP(m,o) \
	o;

#define XTIME_OP(m,o) \
	o;

#endif
namespace tut {

struct TestData {
	TestData() {
	}
	~TestData() {
	}
};

typedef test_group<TestData> tg;
typedef tg::object testobject;

namespace {
tut::tg test_group("Eye detection tests");
}
};
namespace tut {
	void checkResult(CSampleFrame *frame,int *result){
		int i=0;
		ensure_equals("NumberOfSpecularityEyes",frame->GetNumberOfSpecularityEyes(),result[i++]);
		ensure_equals("NumberOfHaarEyes",frame->GetNumberOfHaarEyes(),result[i++]);
		if(frame->GetNumberOfHaarEyes()==0) return;
		CEyeCenterPoint& eye=frame->GetEyeCenterPointList()->at(result[i++]);
		ensure("real eye should be haar",eye.m_IsHaarEye);
		ensure("real eye should be specular",eye.m_IsSpecularityEye);
		ensure_equals("LeftSpecularityX",eye.m_nLeftSpecularityX,result[i++]);
		ensure_equals("LeftSpecularityY",eye.m_nLeftSpecularityY,result[i++]);
		ensure_equals("RightSpecularityX",eye.m_nRightSpecularityX,result[i++]);
		ensure_equals("RightSpecularityY",eye.m_nRightSpecularityY,result[i++]);
	}
template<>
template<>
void testobject::test<1>() {
	set_test_name("end to end test with 4 images");
	const char *imgNames[]={
		"data/ir0_070703092824_890_match_Juan_Pablo_0_285599.pgm",
		"data/ir0_070703122457_390_match_David_0_268222.pgm",
		"data/ir1_070623155542_859_match_Ella_0_181898.pgm",
		"data/ir1_081027131431_984_match_Luciano.pgm"
	};
#ifdef __BFIN__ // since we are using skip in pyramid
	int results[4][8] = {
		{5,		2,	1,	344,	1076,	380,	1080},
		{61,	27,	1,	792,	876,	824,	876},
		{13,	2,	1,	1008,	504,	1048,	504},
		{2,		0,	0,	0,		0,		0,		0}
	};
#else
	int results[4][8] = {
		{2,		2,	1,	344,	1076,	380,	1080},
		{2,		2,	1,	792,	876,	824,	876},
		{4,		2,	1,	1008,	504,	1044,	504},
		{2,		0,	0,	0,		0,		0,		0}
	};
#endif
	EyeDetectAndMatchServer srv(2048,2048);
	srv.SetSingleSpecMode(false);
	CSampleFrame frame;
	frame.setScratch(srv.GetScratch());
	srv.LoadHaarClassifier("data/adaboostClassifier.txt");
	for(int i=0;i<3;i++){
		Image8u img(imgNames[i]);
		frame.SetImage(&img);

		XTIME_OP("Detect",
				srv.Detect(&frame)
		);

		checkResult(&frame,results[i]);
	}

}
#ifdef __BFIN__
	#include <bfin_sram.h>

template<>
template<>
void testobject::test<2>() {
	set_test_name("variance test with cropped image");
	IplImage *img_load=cvLoadImage("data/real_eye.pgm",CV_LOAD_IMAGE_UNCHANGED);

	IplImage *img=cvCreateImageHeader(cvSize(img_load->width, img_load->height),IPL_DEPTH_8U,1);
	IplImage *intImg=cvCreateImageHeader(cvSize(img->width+1, img->height+1),IPL_DEPTH_32S,1);
	IplImage *sqintImg=cvCreateImageHeader(cvSize(img->width+1, img->height+1),IPL_DEPTH_32S,1);
	IplImage *varImg=cvCreateImageHeader(cvSize(5,5),IPL_DEPTH_32S,1);
	int max_scratch=10*1024;
	char *scratch=(char *)sram_alloc(max_scratch, L1_DATA_SRAM);
	char *scrPos=scratch;

	img->imageData=scrPos;		scrPos+=img->imageSize;
	intImg->imageData=scrPos; 	scrPos+=intImg->imageSize;
	sqintImg->imageData=scrPos; scrPos+=sqintImg->imageSize;
	varImg->imageData=scrPos;	scrPos+=varImg->imageSize;
	int scratch_needed=(scrPos-scratch);
	scratch_needed+= (img->width+1)*8;	// needed for variance calc
	ensure("not enough scratch",max_scratch>scratch_needed);
	MAMIGO_Rect rect = {0,0,24,24};
	XTIME_OP("mipl_haar_scaled_variance_image in SDRAM",
		for(int i=0;i<100;i++) mipl_haar_scaled_variance(img_load,scrPos,&rect,4,intImg,sqintImg,varImg)
	);
	cvCopyImage(img_load,img);
	XTIME_OP("mipl_haar_scaled_variance_image L1_DATA_SRAM",
		for(int i=0;i<100;i++) mipl_haar_scaled_variance(img,scrPos,&rect,4,intImg,sqintImg,varImg)
	);

	cvReleaseImage(&img_load);
	cvReleaseImageHeader(&img);
	cvReleaseImageHeader(&intImg);
	cvReleaseImageHeader(&sqintImg);
	cvReleaseImageHeader(&varImg);
	sram_free(scratch);
}


template<>
template<>
void testobject::test<3>() {
	set_test_name("classifier load test");
	int max_features=0;
	unsigned int thresh=0;
	MIPLClassificationParams **features=MIPLClassificationParams::loadFromFile("data/adaboostClassifier.txt",max_features, thresh);
	ensure_equals("max_features",max_features,50);
	int exp_thresh=(int)(25.692215*(1<<24));
	ensure_equals("thresh",thresh,exp_thresh);
//	ensure_equals(features[0]->getParams()[4],12);
//	ensure_equals(features[0]->getParams()[5],18);
//	ensure_equals(features[0]->getParams()[6],24);
	for(int i=0;i<max_features;i++) delete features[i];
	free(features);
}

template<>
template<>
void testobject::test<4>() {
	set_test_name("feature extraction test");
	MIPLClassificationParams params(0,0,0,71.781052,9,15,21,-1,6,-1,22,-1);
	IplImage *img=cvLoadImage("data/real_eye.pgm",CV_LOAD_IMAGE_UNCHANGED);
	IplImage *intImg=cvCreateImageHeader(cvSize(img->width+1, img->height+1),IPL_DEPTH_32S,1);
	IplImage *sqintImg=cvCreateImageHeader(cvSize(img->width+1, img->height+1),IPL_DEPTH_32S,1);
	IplImage *varImg=cvCreateImageHeader(cvSize(5,5),IPL_DEPTH_32S,1);
	IplImage *featImg=cvCreateImageHeader(cvSize(5,5),IPL_DEPTH_32S,1);
	int max_scratch=10*1024;
	char *scratch=(char *)sram_alloc(max_scratch, L1_DATA_SRAM);
	char *scrPos=scratch;
	intImg->imageData=scratch; 	scrPos+=intImg->imageSize;
	sqintImg->imageData=scrPos; scrPos+=sqintImg->imageSize;
	varImg->imageData=scrPos;	scrPos+=varImg->imageSize;
	featImg->imageData=scrPos;	scrPos+=featImg->imageSize;
	int scratch_needed=(scrPos-scratch);
	scratch_needed+= (img->width+1)*8;	// needed for variance calc

	ensure("not enough scratch",max_scratch>scratch_needed);
	MAMIGO_Rect rect = {0,0,24,24};
	mipl_haar_scaled_variance(img,scrPos,&rect,4,intImg,sqintImg,varImg);
	//calling initialize with so that, actual timing can be dtermined
	params.initializeWith(intImg,featImg);

	int featureShift=0;
	XTIME_OP("doComputation",
		for(int i=0;i<100;i++) featureShift=params.doComputation(intImg,featImg)
	);
	ensure_equals("featureShift",featureShift,1);
	cvReleaseImage(&img);
	cvReleaseImageHeader(&intImg);
	cvReleaseImageHeader(&sqintImg);
	cvReleaseImageHeader(&varImg);
	cvReleaseImageHeader(&featImg);
	sram_free(scratch);
}

template<>
template<>
void testobject::test<5>() {
	set_test_name("double represnetation test");
	unsigned int mantissa=0;
	char exponent=0;
	bool isNeg;

	MIPLClassificationParams::representThresh(-71.781052,exponent,mantissa,isNeg);
	ensure_equals("mantissa -ve",mantissa,1350702052);
	ensure_equals("exponent -ve",exponent,18);
	ensure("isNeg -ve",isNeg);

	MIPLClassificationParams::representThresh(71.781052,exponent,mantissa,isNeg);
	ensure_equals("mantissa +ve",mantissa,1350702052);
	ensure_equals("exponent +ve",exponent,18);
	ensure_not("isNeg +ve",isNeg);

	MIPLClassificationParams::representThresh(0,exponent,mantissa,isNeg);
	ensure_equals("mantissa 0",mantissa,0);
	ensure_equals("exponent 0",exponent,31);
	ensure_not("isNeg 0",isNeg);

	MIPLClassificationParams::representThresh(1,exponent,mantissa,isNeg);
	ensure_equals("mantissa 1",mantissa,0x40000000);
	ensure_equals("exponent 1",exponent,30);
	ensure_not("isNeg 1",isNeg);

	MIPLClassificationParams::representThresh(0.889989,exponent,mantissa,isNeg);
	ensure_equals("mantissa fractional",mantissa,1700979750);
	ensure_equals("exponent 1",exponent,31);
	ensure_not("isNeg 1",isNeg);

	MIPLClassificationParams::representThresh(0.001,exponent,mantissa,isNeg);
	ensure_equals("mantissa fractional",mantissa,1125899906);
	ensure_equals("exponent 1",exponent,50);
	ensure_not("isNeg 1",isNeg);

}



template<>
template<>
void testobject::test<6>() {
	set_test_name("haar decision test");
	int good_score=(int)(3.054415 * (1<<24));
	MIPLClassificationParams params(good_score,0,0,71.781052,9,15,21,-1,6,-1,22,-1);
	IplImage *img=cvLoadImage("data/real_eye.pgm",CV_LOAD_IMAGE_UNCHANGED);
	IplImage *intImg=cvCreateImageHeader(cvSize(img->width+1, img->height+1),IPL_DEPTH_32S,1);
	IplImage *sqintImg=cvCreateImageHeader(cvSize(img->width+1, img->height+1),IPL_DEPTH_32S,1);
	IplImage *varImg=cvCreateImageHeader(cvSize(5,5),IPL_DEPTH_32S,1);
	IplImage *featImg=cvCreateImageHeader(cvSize(5,5),IPL_DEPTH_32S,1);
	int max_scratch=10*1024;
	char *scratch=(char *)sram_alloc(max_scratch, L1_DATA_SRAM);
	char *scrPos=scratch;
	intImg->imageData=scratch; 	scrPos+=intImg->imageSize;
	varImg->imageData=scrPos;	scrPos+=varImg->imageSize;
	featImg->imageData=scrPos;	scrPos+=featImg->imageSize;
	sqintImg->imageData=scrPos; scrPos+=sqintImg->imageSize;
	int scratch_needed=(scrPos-scratch);
	scratch_needed+= (img->width+1)*8;	// needed for variance calc

	ensure("not enough scratch",max_scratch>scratch_needed);
	MAMIGO_Rect rect = {0,0,24,24};
	mipl_haar_scaled_variance(img,scrPos,&rect,4,intImg,sqintImg,varImg);

	IplImage *out_score=cvCreateImageHeader(cvSize(5,5),IPL_DEPTH_32S,1);
	out_score->imageData=sqintImg->imageData;

	cvReleaseImageHeader(&sqintImg); // not needed any more

	cvSetZero(out_score);

	params.initializeWith(intImg,featImg);

	XTIME_OP("haarDecision",
	params.haarDecision(varImg,4,out_score)
	);

	cvReleaseImage(&img);
	cvReleaseImageHeader(&intImg);
	cvReleaseImageHeader(&out_score);
	cvReleaseImageHeader(&varImg);
	cvReleaseImageHeader(&featImg);
	sram_free(scratch);
}

IplImage *loadImg(int index, bool& result, char *name){
	sprintf(name,"data/eye%03d_0.pgm",index);
	IplImage *img=cvLoadImage(name,CV_LOAD_IMAGE_UNCHANGED);
	if(img){
		result=false;
		return img;
	}
	sprintf(name,"data/eye%03d_1.pgm",index);
	img=cvLoadImage(name,CV_LOAD_IMAGE_UNCHANGED);
	if(img){
		result=true;
		return img;
	}
	return 0;
}
template<>
template<>
void testobject::test<7>() {
	set_test_name("end to end test");
	HaarClassification haar;
	int max_scratch=10*1024;
	char *scratch=(char *)malloc(max_scratch);
	haar.init(scratch,max_scratch,cvSize(32,32),cvSize(5,5),"data/adaboostClassifier.txt");
	char name[100];
	bool bPass=true;
	for(int i=0;i<81;i++){
		bool expected=false;
		IplImage *img=loadImg(i,expected,name);
//		printf("===%s===\n",name);
		bool actual=haar.isHaarEye(img);
		if(expected!=actual){
			printf("%s failed expected %d actual %d\n",name,expected,actual);
			bPass=false;
		}

	}
	free(scratch);
	ensure("some failures",bPass);
}

template<>
template<>
void testobject::test<8>() {
	IplImage * dest=cvCreateImage(cvSize(640,480), IPL_DEPTH_8U,1);
	Image8u src(2048,2048);
	src.Fill(255);			//white

	cvSetZero(dest);		//black
	CvPoint c={1000,1000};
	src.CopyROIInto(dest,c);
	//SAVE_NAME_INDX(dest,"cropped",1);
	for(int i=0;i< 640*480 ;i++) ensure_equals("should be all white",(unsigned char)*(dest->imageData+i),(unsigned char )255);
	cvSetZero(dest);		//black
	c.x=c.y=10;
	src.CopyROIInto(dest,c);
	// only the right bottom should be white, left top should be black
	//SAVE_NAME_INDX(dest,"cropped",2);

	cvSetZero(dest);		//black
	c.x=c.y=1900;
	src.CopyROIInto(dest,c);
	// only the left top should be white, right bottom should be black
	//SAVE_NAME_INDX(dest,"cropped",3);

	cvReleaseImage(&dest);
}


template<>
template<>
void testobject::test<9>() {
	set_test_name("IsFrameAnEyeSingleSpec for 3 images");
	const char *imgNames[]={
		"data/ir0_070703092824_890_match_Juan_Pablo_0_285599.pgm",
		"data/ir0_070703122457_390_match_David_0_268222.pgm",
		"data/ir1_070623155542_859_match_Ella_0_181898.pgm",

		"data/ir0_070622145827_968_match_Ramsay_0_264785.pgm",
	    "data/ir0_070622145811_062_match_Ramsay_0_189005.pgm",
	    "data/ir0_070622145915_796_match_Ramsay_0_172097.pgm",
	    "data/ir0_070622145828_031_match_Ramsay_0_293570.pgm",

	    "data/ir0_070703092447_609_match_Hector_0_295544.pgm",
	    "data/ir0_070703092553_109_match_Hector_0_297360.pgm",
	    "data/ir0_070703092430_890_match_Hector_0_263755.pgm",
	    "data/ir0_070703092515_906_match_Hector_0_241233.pgm",
	    "data/ir0_070703092557_062_match_Hector_0_282786.pgm",

	    "data/ir0_070703122448_812_match_David_0_148324.pgm",
	    "data/ir0_070703125818_218_match_David_0_260623.pgm",
	    "data/ir0_070703122452_718_match_David_0_121470.pgm",

	    "data/ir1_070703094040_921_match_Christina_0_150582.pgm",
	    "data/ir0_070703092824_828_match_Juan Pablo_0_262877.pgm",
	    "data/ir1_070703091022_828_match_Keith_0_220393.pgm",
	};
	int expected_eyes[]={10,20,23};
	EyeDetectAndMatchServer srv(2048,2048);
	srv.SetSingleSpecMode(true);
	CSampleFrame frame;
	frame.setScratch(srv.GetScratch());
	srv.LoadHaarClassifier("data/adaboostClassifier.txt");
	for(int i=0;i<3;i++){
		Image8u img(imgNames[i]);
		frame.SetImage(&img);

		XTIME_OP("Detect",
				srv.Detect(&frame)
		);
		//printf("[%d]: no of spec points: %d\n",i,frame.GetNumberOfSpecularityEyes());
		ensure_equals("no of single spec eyes",frame.GetNumberOfSpecularityEyes(),expected_eyes[i]);
	}

}


template<>
template<>
void testobject::test<10>() {
	set_test_name("IsFrameAnEye for 3 images");
	const char *imgNames[]={
		"data/ir0_070703092824_890_match_Juan_Pablo_0_285599.pgm",
		"data/ir0_070703122457_390_match_David_0_268222.pgm",
		"data/ir1_070623155542_859_match_Ella_0_181898.pgm",

		"data/ir0_070622145827_968_match_Ramsay_0_264785.pgm",
	    "data/ir0_070622145811_062_match_Ramsay_0_189005.pgm",
	    "data/ir0_070622145915_796_match_Ramsay_0_172097.pgm",
	    "data/ir0_070622145828_031_match_Ramsay_0_293570.pgm",

	    "data/ir0_070703092447_609_match_Hector_0_295544.pgm",
	    "data/ir0_070703092553_109_match_Hector_0_297360.pgm",
	    "data/ir0_070703092430_890_match_Hector_0_263755.pgm",
	    "data/ir0_070703092515_906_match_Hector_0_241233.pgm",
	    "data/ir0_070703092557_062_match_Hector_0_282786.pgm",

	    "data/ir0_070703122448_812_match_David_0_148324.pgm",
	    "data/ir0_070703125818_218_match_David_0_260623.pgm",
	    "data/ir0_070703122452_718_match_David_0_121470.pgm",

	    "data/ir1_070703094040_921_match_Christina_0_150582.pgm",
	    "data/ir0_070703092824_828_match_Juan Pablo_0_262877.pgm",
	    "data/ir1_070703091022_828_match_Keith_0_220393.pgm",

	};
	int expected_eyes[]={10,20,23};
	EyeDetectAndMatchServer srv(2048,2048);
	srv.SetSingleSpecMode(false);
	CSampleFrame frame;
	frame.setScratch(srv.GetScratch());
	srv.LoadHaarClassifier("data/adaboostClassifier.txt");
	char fName[32];
	for(int i=1;i<2;i++){
		Image8u img(imgNames[i]);
		frame.SetImage(&img);

		XTIME_OP("Detect",
				srv.Detect(&frame)
		);
		sprintf(fName,"frame_%d.pgm",i);
		SaveIPLImage(frame.GetPyramid()->GetLevel(2),fName);
		printf("[%d]: no of spec points: %d\n",i,frame.GetNumberOfSpecularityEyes());
		//ensure_equals("no of single spec eyes",frame.GetNumberOfSpecularityEyes(),expected_eyes[i]);
	}

}


#endif


test_runner_singleton runner;
}

int main() {
	tut::reporter reporter;
	tut::runner.get().set_callback(&reporter);
	tut::runner.get().run_test("Eye detection tests",1);
//	tut::runner.get().run_test("Eye detection tests",10);
	return 0;
}

#ifdef TOBEDONE
/*
  Usage information
  */
void
usage(const char *prog_name)
{
  static char *helpmsg[] = {
    "\nOPTIONS\n",
    "-i filename	specifies input image\n",
    "-o filename	specifies output eye image index\n",
    "-d int			specifies number of specularities\n",
    "-s float		specifies scale - iris width / 18\n",
    NULL
  };
  char **msgptr = helpmsg;

  /* output the header first */
  fprintf(stderr,"================== %s\n",prog_name);
  fprintf(stderr,"Detect and crop eyes\n");
  fprintf(stderr,"\nUSAGE\n");
  fprintf(stderr,"      %s [options]\n",prog_name);

  /* output all the options */
  while(*msgptr!= (char *) NULL)
    (void) fputs(*msgptr++,stderr);

  fprintf(stderr, "\n");

}


int main(int argc, char *argv[])
{
	int arg = 0;
	int dualSpecularity = 2;
	double scale = 7.0;

	while(++arg < argc)
	{
		if( !strcmp(argv[arg], "-i"))
			strcpy(inputImage, argv[++arg]); // test file for list of Images
		else if( ! strcmp(argv[arg], "-o"))
			strcpy(outputImagePrefix, argv[++arg]); // test file for pair wise testing
		else if(! strcmp(argv[arg], "-s"))
			scale = (atof(argv[++arg])); // threshold for matching
		else if(! strcmp(argv[arg], "-d"))
			dualSpecularity = (atoi(argv[++arg])); // threshold for matching
		else if(! strcmp(argv[arg], "-h"))
		{
			usage("EyeSegmentation");
			exit(1);
		}

	}
}

#endif
