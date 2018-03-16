#include <stdio.h>
#include <stdlib.h>
#include "SpecularityBasedSpoofDetector.h"
#include "FFTSpoofDetector.h"
#include <cxcore.h>
//#include "Image.h"
#include <highgui.h>
#include <string>
#include <cv.h>

extern "C" {
#include "file_manip.h"
}
/*
-id C:\HDemo\mini_spoof_data\spoof\dspoof1 -i C:\HDemo\mini_spoof_data\spoof\dspoof1_l.txt
-id C:\HDemo\mini_spoof_data\spoof\dspoof1 -i C:\HDemo\mini_spoof_data\spoof\dspoof1_r.txt
-id C:\HDemo\mini_spoof_data\spoof\dspoof2 -i C:\HDemo\mini_spoof_data\spoof\dspoof2_l.txt
-id C:\HDemo\mini_spoof_data\spoof\dspoof2 -i C:\HDemo\mini_spoof_data\spoof\dspoof2_r.txt
-id C:\HDemo\mini_spoof_data\spoof\dspoof3 -i C:\HDemo\mini_spoof_data\spoof\dspoof3_l.txt
-id C:\HDemo\mini_spoof_data\spoof\dspoof3 -i C:\HDemo\mini_spoof_data\spoof\dspoof3_r.txt
-id C:\HDemo\mini_spoof_data\spoof\dspoof4 -i C:\HDemo\mini_spoof_data\spoof\dspoof4_l.txt
-id C:\HDemo\mini_spoof_data\spoof\dspoof4 -i C:\HDemo\mini_spoof_data\spoof\dspoof4_r.txt
-id C:\HDemo\mini_spoof_data\spoof\dspoof5 -i C:\HDemo\mini_spoof_data\spoof\dspoof5_l.txt
-id C:\HDemo\mini_spoof_data\spoof\dspoof5 -i C:\HDemo\mini_spoof_data\spoof\dspoof5_r.txt

-id C:\HDemo\mini_spoof_data\spoof\bspoof6 -i C:\HDemo\mini_spoof_data\spoof\bspoof6_r.txt
-id C:\HDemo\mini_spoof_data\spoof\bspoof6 -i C:\HDemo\mini_spoof_data\spoof\bspoof6_l.txt

-id C:\HDemo\mini_spoof_data\spoof\lb1 -i C:\HDemo\mini_spoof_data\spoof\lb1_l.txt
-id C:\HDemo\mini_spoof_data\spoof\lb1 -i C:\HDemo\mini_spoof_data\spoof\lb1_r.txt
-id C:\HDemo\mini_spoof_data\spoof\lb2 -i C:\HDemo\mini_spoof_data\spoof\lb2_l.txt
-id C:\HDemo\mini_spoof_data\spoof\lb2 -i C:\HDemo\mini_spoof_data\spoof\lb2_r.txt
-id C:\HDemo\mini_spoof_data\spoof\lb3 -i C:\HDemo\mini_spoof_data\spoof\lb3_l.txt
-id C:\HDemo\mini_spoof_data\spoof\lb3 -i C:\HDemo\mini_spoof_data\spoof\lb3_r.txt
-id C:\HDemo\mini_spoof_data\spoof\lb4 -i C:\HDemo\mini_spoof_data\spoof\lb4_l.txt
-id C:\HDemo\mini_spoof_data\spoof\lb4 -i C:\HDemo\mini_spoof_data\spoof\lb4_r.txt
-id C:\HDemo\mini_spoof_data\spoof\lb5 -i C:\HDemo\mini_spoof_data\spoof\lb5_l.txt
-id C:\HDemo\mini_spoof_data\spoof\lb5 -i C:\HDemo\mini_spoof_data\spoof\lb5_r.txt


-id C:\HDemo\mini_spoof_data\real\d1 -i C:\HDemo\mini_spoof_data\real\d1_l.txt
-id C:\HDemo\mini_spoof_data\real\d1 -i C:\HDemo\mini_spoof_data\real\d1_r.txt
-id C:\HDemo\mini_spoof_data\real\d2 -i C:\HDemo\mini_spoof_data\real\d2_l.txt
-id C:\HDemo\mini_spoof_data\real\d2 -i C:\HDemo\mini_spoof_data\real\d2_r.txt
-id C:\HDemo\mini_spoof_data\real\d3 -i C:\HDemo\mini_spoof_data\real\d3_l.txt
-id C:\HDemo\mini_spoof_data\real\d3 -i C:\HDemo\mini_spoof_data\real\d3_r.txt
-id C:\HDemo\mini_spoof_data\real\d4 -i C:\HDemo\mini_spoof_data\real\d4_l.txt
-id C:\HDemo\mini_spoof_data\real\d4 -i C:\HDemo\mini_spoof_data\real\d4_r.txt
-id C:\HDemo\mini_spoof_data\real\d5 -i C:\HDemo\mini_spoof_data\real\d5_l.txt
-id C:\HDemo\mini_spoof_data\real\d5 -i C:\HDemo\mini_spoof_data\real\d5_r.txt

-id C:\HDemo\mini_spoof_data\real\m1 -i C:\HDemo\mini_spoof_data\real\m1_l.txt
-id C:\HDemo\mini_spoof_data\real\m1 -i C:\HDemo\mini_spoof_data\real\m1_r.txt
-id C:\HDemo\mini_spoof_data\real\m2 -i C:\HDemo\mini_spoof_data\real\m2_l.txt
-id C:\HDemo\mini_spoof_data\real\m2 -i C:\HDemo\mini_spoof_data\real\m2_r.txt
-id C:\HDemo\mini_spoof_data\real\m3 -i C:\HDemo\mini_spoof_data\real\m3_l.txt
-id C:\HDemo\mini_spoof_data\real\m3 -i C:\HDemo\mini_spoof_data\real\m3_r.txt
-id C:\HDemo\mini_spoof_data\real\m4 -i C:\HDemo\mini_spoof_data\real\m4_l.txt
-id C:\HDemo\mini_spoof_data\real\m4 -i C:\HDemo\mini_spoof_data\real\m4_r.txt
-id C:\HDemo\mini_spoof_data\real\m5 -i C:\HDemo\mini_spoof_data\real\m5_l.txt
-id C:\HDemo\mini_spoof_data\real\m5 -i C:\HDemo\mini_spoof_data\real\m5_r.txt

-id C:\HDemo\mini_spoof_data\spoof\fake_detects -i C:\HDemo\mini_spoof_data\spoof\fake_detects.txt
*/

std::string trim(std::string str)
{
	std::string whitespaces (" \t\f\v\n\r");
	size_t found = str.find_last_not_of(whitespaces);
	if (found != std::string::npos)
		str.erase(found+1);
	else
		str.clear();            // str is all whitespace

	return str;
}
#if 0
int main(int argc, char *argv[])
{
	IplImage *data = 0;
	int w = 640; int h = 480;
	char line[1001], directory[200];
	int arg = 0;

	while(++arg < argc)
	{
		if( !strcmp(argv[arg], "-d"))
			strcpy(directory, argv[++arg]); // test file for list of Images
	}

	FILE *fp = fopen(argv[argc-1], "rt");
	if(fp== NULL)
	{
		printf("File: %s not found\n", argv[1]);
		return 0;
	}

	FFTSpoofDetector *srServer = new FFTSpoofDetector();

	while(fgets(line,1000, fp))
	{
		char filename[200];
		sscanf(line, "%s", filename);
		sprintf(line, "%s/%s", directory,filename);

		IplImage *data = cvLoadImage(line, CV_LOAD_IMAGE_GRAYSCALE);


		//
		int rv;
		TIME_OP("TotalTime",
		{	rv = srServer->check(data);
		}
		);
		printf("Result %s = %d\n", filename, rv);
		cvReleaseImage(&data);
		//return 0;
	}

	delete srServer;

	printf("A;; done\n");

	return 1;
}

#endif


#if 1
int main(int argc, char *argv[])
{
	char inputDirectory[200];
	char inputSequence[100];
	int arg=0;

	while(++arg < argc)
	{
		if( !strcmp(argv[arg], "-d"))
		{
			strcpy(inputDirectory, argv[++arg]); // test file for list of Images
		}
		else if( !strcmp(argv[arg], "-i"))
		{
			strcpy(inputSequence, argv[++arg]); // test file for list of Images
		}
	}

	SpecularityBasedSpoofDetector *server = 0;

	FILE *fp = fopen(inputSequence, "rb");
	char line[100];

	bool result = false;

	while(fgets(line, 100, fp) != NULL)
	{
		std::string lineStr = trim(std::string(line));
		if(lineStr.empty())	continue;

		char fileName[300];

		sprintf(fileName, "%s/%s", inputDirectory, lineStr.c_str());
		printf("Processing .... %s\n", fileName);

		IplImage *input = cvLoadImage(fileName, CV_LOAD_IMAGE_GRAYSCALE);

		if(input)
		{
			if(server == 0)
			{
				server = new SpecularityBasedSpoofDetector(input->width, input->height);
				server->set_min_max_threshold(0, 10.0);
			}

			result = server->check2(input->imageData, input->width, input->height, input->widthStep);

			cvReleaseImage(&input);
		}
	}

	printf("Real = %d\n", (int) result);

	if(server) delete server; server = 0;

}
#endif









#ifdef COMMENT
#include <stdio.h>
#include <stdlib.h>
#include "IrisSelectServer.h"
#include <cxcore.h>
#include <cv.h>
#include <highgui.h>

/*

-id data/real -i image%3.3d.pgm -od data/real -x 0 10
-id data/fake -i image%3.3d.pgm -od data/fake -x 0 18
-id data/EyeSwipeMini/LeftCamera -i eye_%d.pgm -od data/EyeSwipeMini/LeftCamera/output -x 24 148
-id C:\HDemo\eyeswipe-mini-pass\keith1 -i image%3.3d.pgm -od C:\HDemo\eyeswipe-mini-pass\keith1 -x 0 22
-id C:\HDemo\eyeswipe-mini-pass\fidel2 -i image%3.3d.pgm -od C:\HDemo\eyeswipe-mini-pass\fidel2 -x 0 12
-id C:\HDemo\eyeswipe-mini-pass\david1 -i image%3.3d.pgm -od C:\HDemo\eyeswipe-mini-pass\david1 -x 0 40
-id C:\HDemo\eyeswipe-mini-pass\slow -i image%3.3d.pgm -od C:\HDemo\eyeswipe-mini-pass\slow -x 0 29
-id C:\HDemo\eyeswipe-mini-pass\keith4-slow -i image%3.3d.pgm -od C:\HDemo\eyeswipe-mini-pass\keith4-slow -x 0 32

*/


void main(int argc, char *argv[])
{
	char inputDirectory[200];
	char outputDirectory[200];
	char inputSequence[100];
	int startIdx, endIdx;
	int arg=0;

	while(++arg < argc)
	{
		if( !strcmp(argv[arg], "-id"))
		{
			strcpy(inputDirectory, argv[++arg]); // test file for list of Images
		}
		else if( !strcmp(argv[arg], "-i"))
		{
			strcpy(inputSequence, argv[++arg]); // test file for list of Images
		}		
		else if( ! strcmp(argv[arg], "-x"))
		{
			startIdx = atoi(argv[++arg]); // test file for pair wise testing
			endIdx = atoi(argv[++arg]); // test file for pair wise testing
		}
		else if( !strcmp(argv[arg], "-od"))
		{
			strcpy(outputDirectory, argv[++arg]);
		}
	}

	IrisSelectServer *server = 0;
	IplImage *pyrImage = 0;
	
	float *pv[3];
	for(int i=0;i<3;i++)
		pv[i] = (float *) calloc(10, sizeof(float));

	for(int i=startIdx;i<=endIdx;i++)
	{
		char fileName[300];
		char interName[100];

		sprintf(interName, inputSequence, i);

		sprintf(fileName, "%s/%s", inputDirectory, interName);
		printf("Processing .... %s\n", fileName);

		IplImage *input = cvLoadImage(fileName, CV_LOAD_IMAGE_GRAYSCALE);

		if(input)
		{
			if(i == startIdx)
			{
				server = new IrisSelectServer(input->width/2, input->height/2);
				pyrImage = cvCreateImage(cvSize(input->width/2, input->height/2), IPL_DEPTH_8U, 1);
			}

			cvPyrDown( input, pyrImage, CV_GAUSSIAN_5x5 );

			server->ComputeFeatureVector(pyrImage, cvRect(input->width/4 - 32, input->height/4 - 32, 64, 64), pv[2]);
			
			CvPoint2D32f pt, var;

			server->ComputeSpecularityMetrics(pyrImage, pt, var, 14);
			printf("%f %f %f\n", pt.x*2, pt.y*2, var.y/var.x);

			cvReleaseImage(&input);

			float *temp = pv[2];
			pv[2] = pv[1];
			pv[1] = pv[0];
			pv[0] = temp;
		}
	}

	for(int i=0;i<3;i++)
		free(pv[i]);

	if(server) delete server; server = 0;
	if(pyrImage) cvReleaseImage(&pyrImage);
	
}
#endif
#if 0
int main(int argc, char *argv[])
{
	char inputDirectory[200];
	char outputDirectory[200];
	char inputSequence[100];
	int startIdx, endIdx;
	int arg=0;

	while(++arg < argc)
	{
		if( !strcmp(argv[arg], "-id"))
		{
			strcpy(inputDirectory, argv[++arg]); // test file for list of Images
		}
		else if( !strcmp(argv[arg], "-i"))
		{
			strcpy(inputSequence, argv[++arg]); // test file for list of Images
		}		
		else if( ! strcmp(argv[arg], "-x"))
		{
			startIdx = atoi(argv[++arg]); // test file for pair wise testing
			endIdx = atoi(argv[++arg]); // test file for pair wise testing
		}
		else if( !strcmp(argv[arg], "-od"))
		{
			strcpy(outputDirectory, argv[++arg]);
		}
	}

	IrisSelectServer *server = 0;
	IplImage *pyrImage = 0;
	
	float *pv[3];
	for(int i=0;i<3;i++)
		pv[i] = (float *) calloc(10, sizeof(float));

	for(int i=startIdx;i<=endIdx;i++)
	{
		char fileName[300];
		char interName[100];

		sprintf(interName, inputSequence, i);

		sprintf(fileName, "%s/%s", inputDirectory, interName);
		printf("\nProcessing .... %s\n", fileName);

		IplImage *input = cvLoadImage(fileName, CV_LOAD_IMAGE_GRAYSCALE);

		if(input)
		{
			if(i == startIdx)
			{
				server = new IrisSelectServer(input->width/2, input->height/2);
				pyrImage = cvCreateImage(cvSize(input->width/2, input->height/2), IPL_DEPTH_8U, 1);
			}

			cvPyrDown( input, pyrImage, CV_GAUSSIAN_5x5 );

			server->ComputeFeatureVector(pyrImage, cvRect(input->width/4 - 32, input->height/4 - 32, 64, 64), pv[0]);
			printf("%f %f %f %f\n", pv[0][1], pv[0][2], pv[0][3]);

			server->ComputeFeatureVector(pyrImage, cvRect(input->width/4 - 16, input->height/4 - 16, 32, 32), pv[1]);
			printf("%f %f %f %f\n", pv[1][1], pv[1][2], pv[1][3]);
			
			printf("%f %f %f %f\n", pv[0][1]/pv[1][1], pv[0][2]/pv[1][2], pv[0][3]/pv[1][3]);
			cvReleaseImage(&input);

		}
	}

	for(int i=0;i<3;i++)
		free(pv[i]);

	if(server) delete server; server = 0;
	if(pyrImage) cvReleaseImage(&pyrImage);
	return 0
}
#endif


