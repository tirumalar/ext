// PupilSegmentation.cpp : Defines the entry point for the console application.
//

//#include "stdafx.h"
#include <math.h>       /* sin */
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#include <string>
#include <time.h>
#include "pupilsegmentation.h"
//#include "ImagePGM.h"

#define PI 3.14159265

int compare (const void * a, const void * b)
{
	return ( *(float*)a - *(float*)b );
}
void candidateSearch(float *firstDiff, int length, int *candidateX, int candidateLength, int winSize = 3)

{
	int candidateId = 0;
	for (int i = winSize; i < winSize + candidateLength/2  ; i++ )
	{
		float max = -99999;     
		int maxIndex = 0;
		for (int j = i;  j < length/2;  j++)
		{
			if (firstDiff[j] > max )
			{  
				max = firstDiff[j];
				maxIndex = j;
			}
		}
		candidateX[candidateId++] = maxIndex;
		firstDiff[maxIndex] = -99999;
	}

	for (int i = length/2 ; i < length/2 + candidateLength/2; i++ )
	{
		float min = 99999;     
		int minIndex = 0;
		for (int j = i;  j < length-winSize;  j++)
		{
			if (firstDiff[j] < min)
			{  
				min = firstDiff[j];
				minIndex = j;
			}
		}
		candidateX[candidateId++] = minIndex;
		firstDiff[minIndex] = 99999;
	}
}

float standard_deviation(int *lineX, float *lineData, int start, int end)
{
	float mean=0.0, sum_deviation=0.0;
	int n = end - start +1;

	for(int  i=start; i<= end; i++)
		mean+=lineData[i];

	mean=mean/n;
	for(int i=start; i <= end;i++)
		sum_deviation+=(lineData[i]-mean)*(lineData[i]-mean);

	return sqrt(sum_deviation/(n-1));           
}

int findZeroCross(float *lineData, float *secondDiff, int length, int *lineX, int candidateX, float givenCir, int winSize, float *std, bool flag )
{
	int startCandidate = candidateX-7;
	int endCandidate = candidateX+7;
	float minimum = 99999;
	int minCand =  15;
	int candidateCir=0;
	if (flag)
	{
		if (startCandidate < winSize)
			startCandidate = winSize;
		if  (endCandidate > length/2)
			endCandidate =  length/2;

		for (int i=startCandidate; i < endCandidate; i++)
			if  (secondDiff[i] == 0 || ((secondDiff[i]) < 0 ) && (secondDiff[i+1] > 0))
			{
				candidateCir = i;
				if (abs(lineX[candidateCir] - givenCir) < minimum)
				{
					minCand = candidateCir;
					minimum = abs(lineX[candidateCir] - givenCir);
				}
			}
	}
	else{
		if (startCandidate < length/2)
			startCandidate = length/2;
		if  (endCandidate > length-winSize)
			endCandidate =  length-winSize;

		for (int i=startCandidate; i < endCandidate; i++)
			if  (secondDiff[i] == 0 || ((secondDiff[i]) > 0 ) && (secondDiff[i+1] < 0))
			{
				candidateCir = i;
				if (abs(lineX[candidateCir] - givenCir) < minimum)
				{
					minCand = candidateCir;
					minimum = abs(lineX[candidateCir] - givenCir);
				}
			}
	}
	std[0]=standard_deviation(lineX, lineData, startCandidate, endCandidate);
	return minCand;

}
/*int main(char* argv[])
{

	FILE *fw = fopen("D:/EyeLockRnD/SegmentationRnD/syntheticBadSegemnetedImages/coutput.csv", "w");
	fprintf(fw,"ImageNo, Score, Time\n");
	std::ifstream file("D:/EyeLockRnD/SegmentationRnD/inputWholeProblems1.csv");
	std::string str; 
	while (std::getline(file, str))
	{
		clock_t start = clock();
		
		std::string inputImage = "D:/EyeLockRnD/SegmentationRnD/syntheticBadSegemnetedImages/origImage" + str + ".pgm";
		std::string inputCircle = "D:/EyeLockRnD/SegmentationRnD/syntheticBadSegemnetedImages/origImage" + str + ".pgm.txt";


		int w, h;
		unsigned char *data = 0;
		int status = ReadPGM5(inputImage.c_str(), &data, &w, &h);
		if(w == 640 && h == 480 && status >=0) printf("Image Read successeful\n");		
		else  printf("Error: Incorrect Format or Unable to Read Image %s\n", str.c_str());

		FILE *fp;
		float pupilX, pupilY, pupilR, irisX, irisY, irisR;

		if( (fp = fopen(inputCircle.c_str(), "r+")) == NULL)
			printf("No such file\n");
		if (fp == NULL)
			printf("Error Reading File\n");


		fscanf(fp,"%f %f %f %f %f %f", &irisX, &irisY, &irisR, &pupilX, &pupilY, &pupilR);
		fclose(fp);
		
		int yLimMin = (int)(pupilY - pupilR*sin (30*PI/180));
		int yLimMax = (int)(pupilY + pupilR*sin (50*PI/180));

		float leftError[640]= {0};
		float rightError[640] = {0};
		int errorCountLeft = 0, errorCountRight = 0;


		for (int y = yLimMin-1; y < yLimMax; y++)
		{
			float X =  sqrt((pupilR + 30)* (pupilR + 30) - (y-pupilY)*(y-pupilY));
			int xLimMin =  (int)(pupilX - X);
			int xLimMax =  (int)(pupilX + X);


			X =  sqrt((pupilR)* (pupilR) - (y-pupilY)*(y-pupilY));

			float givenCir3 = ((float)pupilX - X);
			float givenCir4 = ((float)pupilX + X);
			//printf("");
			int winSize = 3;
			int dataSize =  (xLimMax - xLimMin +1) + 2*winSize;


			float *lineData;
			int *lineX;
			lineData = (float *) malloc(dataSize*sizeof(float));
			lineX = (int *) malloc(dataSize*sizeof(int));

			for (int i = 0; i < dataSize ; i++ )
			{
				int x = (xLimMin - winSize) + i;
				lineX[i] = x;
				
				//float average =  (int)data[w*y+x];
				float average	= (float)(data[w*(y-1)+(x-1)] + data[w*y+(x-1)] + data[w*(y+1)+(x-1)] + 	// 1/9*[ 1  1  1
					data[w*(y-1)+x] + data[w*y+x] +data[w*(y+1)+x] + 	                                    //		 1  1  1
					data[w*(y-1)+(x+1)] + data[w*y+(x+1)] + data[w*(y+1)+(x+1)])/9;							//		 1  1  1]
				lineData[i] = average;

			}

			float *lineFirstDiff;
			float *lineSecondDiff;
			lineFirstDiff = (float *) malloc(dataSize*sizeof(float));
			lineSecondDiff = (float *) malloc(dataSize*sizeof(float));

			for (int i = winSize; i < dataSize - winSize  ; i++ )
			{
				lineFirstDiff[i] =   (lineData[i-3] + lineData[i-2] + lineData[i-1] - lineData[i+1] - lineData[i+2] - lineData[i+3]);            //[1  1  1  0 -1 -1 -1]
				lineSecondDiff[i] =	 10*(lineData[i-3] + lineData[i-2] + lineData[i-1] - 6*lineData[i] + lineData[i+1] + lineData[i+2] + lineData[i+3]); //[1 1 1 -6 1 1 1]
			}


			int *candidateX;
			int MaximaSize = 5;
			int candidateLength = 2*MaximaSize;
			candidateX = (int *) malloc(candidateLength*sizeof(int));
			candidateSearch(lineFirstDiff, dataSize, candidateX, candidateLength, winSize);

			float *candidateStd = 0;
			candidateStd = (float*) malloc(1*sizeof(float));

			float maxStd= -999;
			float minimumError = 9999;
			int predictedCir3 = givenCir3 + 15;

			for (int i = 0; i < candidateLength/2; i++)
			{   
				int minCand = findZeroCross(lineData, lineSecondDiff, dataSize, lineX, candidateX[i], givenCir3, winSize, candidateStd, true);
				float candError = abs(givenCir3-(lineX[minCand]));
				if (candidateStd[0] > maxStd && candError < minimumError && candidateStd[0] > 3.0)
				{	
					minimumError = candError;
					maxStd = candidateStd[0];
					predictedCir3 = lineX[minCand];
				}
			}

			maxStd= -9999;
			minimumError = 9999;
			int predictedCir4 = givenCir4 + 15;

			for (int i = candidateLength/2; i < candidateLength; i++)
			{   
				int minCand = findZeroCross(lineData, lineSecondDiff, dataSize, lineX, candidateX[i], givenCir4, winSize, candidateStd, false);
				float candError = abs(givenCir4-(lineX[minCand]));
				if (candidateStd[0] > maxStd && candError < minimumError && candidateStd[0] > 3.0)
				{	
					minimumError = candError;
					maxStd = candidateStd[0];
					predictedCir4 = lineX[minCand];
				}
			}


			bool specularity = false;
			for (int i=winSize; i < dataSize/2; i++)
				if (lineData[i] > 190) 
				{
					specularity = true; break;
				}

				if (!specularity)
					leftError[errorCountLeft++] = abs(givenCir3 - predictedCir3);

				specularity = false;
				for (int i = dataSize/2; i < dataSize - winSize; i++)
					if (lineData[i] > 190) 
					{
						specularity = true; break;
					}

					if (!specularity)
						rightError[errorCountRight++] = abs(givenCir4 - predictedCir4);


					free(candidateX);
					free(lineFirstDiff);
					free(lineSecondDiff);
					free(lineX);
					
		}

	
		qsort (leftError, errorCountLeft, sizeof(float), compare);
		qsort (rightError, errorCountRight, sizeof(float), compare);
		printf("Input Image: %s , Left Median: %f, Right Median: %f\n", str.c_str(), leftError[errorCountLeft/2], rightError[errorCountRight/2]);

		float normErrorLeft = leftError[errorCountLeft/2]/pupilR;
		float normErrorRight = rightError[errorCountRight/2]/pupilR;

		clock_t end = clock();
		double time = (double) (end-start) / CLOCKS_PER_SEC * 1000.0;
		printf("Input Image: %s,  Total Error:  %f\n", str.c_str(), normErrorLeft > normErrorRight? normErrorLeft: normErrorRight);
		fprintf(fw,"%s, %0.4f, %0.4f\n", str.c_str(), normErrorLeft > normErrorRight? normErrorLeft: normErrorRight, time);
		free(data);

	}
	fclose(fw);
	return 0;
}

*/
int segmentation(const char *filename)
{

	FILE *fw = fopen("./data/syntheticBadSegemnetedImages/coutput.csv", "w");
	fprintf(fw,"ImageNo, Score, Time\n");
	std::ifstream file(filename); //"D:/EyeLockRnD/SegmentationRnD/inputWholeProblems1.csv"
	std::string str; 
	int count=0;
	while (std::getline(file, str))
	{
		clock_t start = clock();
		
		std::string inputImage = "D:/EyeLockRnD/SegmentationRnD/syntheticBadSegemnetedImages/origImage" + str + ".pgm";
		std::string inputCircle = "D:/EyeLockRnD/SegmentationRnD/syntheticBadSegemnetedImages/origImage" + str + ".pgm.txt";


		int w, h;
		unsigned char *data = 0;
		//int status = ReadPGM5(inputImage.c_str(), &data, &w, &h);
		int status = 0;
		if(w == 640 && h == 480 && status >=0) printf("Image Read successeful\n");		
		else  printf("Error: Incorrect Format or Unable to Read Image %s\n", str.c_str());

		FILE *fp;
		float pupilX, pupilY, pupilR, irisX, irisY, irisR;

		if( (fp = fopen(inputCircle.c_str(), "r+")) == NULL)
			printf("No such file\n");
		if (fp == NULL)
			printf("Error Reading File\n");


		fscanf(fp,"%f %f %f %f %f %f", &irisX, &irisY, &irisR, &pupilX, &pupilY, &pupilR);
		fclose(fp);
		
		int yLimMin = (int)(pupilY - pupilR*sin (30*PI/180));
		int yLimMax = (int)(pupilY + pupilR*sin (50*PI/180));

		float leftError[640]= {0};
		float rightError[640] = {0};
		int errorCountLeft = 0, errorCountRight = 0;


		for (int y = yLimMin-1; y < yLimMax; y++)
		{
			float X =  sqrt((pupilR + 30)* (pupilR + 30) - (y-pupilY)*(y-pupilY));
			int xLimMin =  (int)(pupilX - X);
			int xLimMax =  (int)(pupilX + X);


			X =  sqrt((pupilR)* (pupilR) - (y-pupilY)*(y-pupilY));

			float givenCir3 = ((float)pupilX - X);
			float givenCir4 = ((float)pupilX + X);
			//printf("");
			int winSize = 3;
			int dataSize =  (xLimMax - xLimMin +1) + 2*winSize;


			float *lineData;
			int *lineX;
			lineData = (float *) malloc(dataSize*sizeof(float));
			lineX = (int *) malloc(dataSize*sizeof(int));

			for (int i = 0; i < dataSize ; i++ )
			{
				int x = (xLimMin - winSize) + i;
				lineX[i] = x;
				
				//float average =  (int)data[w*y+x];
				float average	= (float)(data[w*(y-1)+(x-1)] + data[w*y+(x-1)] + data[w*(y+1)+(x-1)] + 	// 1/9*[ 1  1  1
					data[w*(y-1)+x] + data[w*y+x] +data[w*(y+1)+x] + 	                                    //		 1  1  1
					data[w*(y-1)+(x+1)] + data[w*y+(x+1)] + data[w*(y+1)+(x+1)])/9;							//		 1  1  1]
				lineData[i] = average;

			}

			float *lineFirstDiff;
			float *lineSecondDiff;
			lineFirstDiff = (float *) malloc(dataSize*sizeof(float));
			lineSecondDiff = (float *) malloc(dataSize*sizeof(float));

			for (int i = winSize; i < dataSize - winSize  ; i++ )
			{
				lineFirstDiff[i] =   (lineData[i-3] + lineData[i-2] + lineData[i-1] - lineData[i+1] - lineData[i+2] - lineData[i+3]);            //[1  1  1  0 -1 -1 -1]
				lineSecondDiff[i] =	 10*(lineData[i-3] + lineData[i-2] + lineData[i-1] - 6*lineData[i] + lineData[i+1] + lineData[i+2] + lineData[i+3]); //[1 1 1 -6 1 1 1]
			}


			int *candidateX;
			int MaximaSize = 5;
			int candidateLength = 2*MaximaSize;
			candidateX = (int *) malloc(candidateLength*sizeof(int));
			candidateSearch(lineFirstDiff, dataSize, candidateX, candidateLength, winSize);

			float *candidateStd = 0;
			candidateStd = (float*) malloc(1*sizeof(float));

			float maxStd= -999;
			float minimumError = 9999;
			int predictedCir3 = givenCir3 + 15;

			for (int i = 0; i < candidateLength/2; i++)
			{   
				int minCand = findZeroCross(lineData, lineSecondDiff, dataSize, lineX, candidateX[i], givenCir3, winSize, candidateStd, true);
				float candError = abs(givenCir3-(lineX[minCand]));
				if (candidateStd[0] > maxStd && candError < minimumError && candidateStd[0] > 3.0)
				{	
					minimumError = candError;
					maxStd = candidateStd[0];
					predictedCir3 = lineX[minCand];
				}
			}

			maxStd= -9999;
			minimumError = 9999;
			int predictedCir4 = givenCir4 + 15;

			for (int i = candidateLength/2; i < candidateLength; i++)
			{   
				int minCand = findZeroCross(lineData, lineSecondDiff, dataSize, lineX, candidateX[i], givenCir4, winSize, candidateStd, false);
				float candError = abs(givenCir4-(lineX[minCand]));
				if (candidateStd[0] > maxStd && candError < minimumError && candidateStd[0] > 3.0)
				{	
					minimumError = candError;
					maxStd = candidateStd[0];
					predictedCir4 = lineX[minCand];
				}
			}


			bool specularity = false;
			for (int i=winSize; i < dataSize/2; i++)
				if (lineData[i] > 190) 
				{
					specularity = true; break;
				}

				if (!specularity)
					leftError[errorCountLeft++] = abs(givenCir3 - predictedCir3);

				specularity = false;
				for (int i = dataSize/2; i < dataSize - winSize; i++)
					if (lineData[i] > 190) 
					{
						specularity = true; break;
					}

					if (!specularity)
						rightError[errorCountRight++] = abs(givenCir4 - predictedCir4);


					free(candidateX);
					free(lineFirstDiff);
					free(lineSecondDiff);
					free(lineX);
					
		}

	
		qsort (leftError, errorCountLeft, sizeof(float), compare);
		qsort (rightError, errorCountRight, sizeof(float), compare);
		printf("Input Image: %s , Left Median: %f, Right Median: %f\n", str.c_str(), leftError[errorCountLeft/2], rightError[errorCountRight/2]);

		float normErrorLeft = leftError[errorCountLeft/2]/pupilR;
		float normErrorRight = rightError[errorCountRight/2]/pupilR;

		clock_t end = clock();
		double time = (double) (end-start) / CLOCKS_PER_SEC * 1000.0;
		printf("Input Image: %s,  Total Error:  %f\n", str.c_str(), normErrorLeft > normErrorRight? normErrorLeft: normErrorRight);
		fprintf(fw,"%s, %0.4f, %0.4f\n", str.c_str(), normErrorLeft > normErrorRight? normErrorLeft: normErrorRight, time);
		free(data);
		count++;

	}
	fclose(fw);
	return count;
}


int segmentation(const char *inputImage, const char *inputCircle,  const char *label)
{

	

	
	
		clock_t start = clock();
		
		

		int w, h;
		unsigned char *data = 0;
		int status = 0;//ReadPGM5(inputImage, &data, &w, &h);
		if(w == 640 && h == 480 && status >=0) printf("Image Read successeful\n");		
		else  {printf("Error: Incorrect Format or Unable to Read Image %s\n", inputImage); return -99;}

		FILE *fp;
		float pupilX, pupilY, pupilR, irisX, irisY, irisR;

		if( (fp = fopen(inputCircle, "r+")) == NULL)
		{printf("%s No such file\n", inputCircle); return -99;}
		if (fp == NULL)
		{printf("Error Reading File\n");  return -99;}


		fscanf(fp,"%f %f %f %f %f %f", &irisX, &irisY, &irisR, &pupilX, &pupilY, &pupilR);
		fclose(fp);
		
		int yLimMin = (int)(pupilY - pupilR*sin (30*PI/180));
		int yLimMax = (int)(pupilY + pupilR*sin (50*PI/180));

		float leftError[640]= {0};
		float rightError[640] = {0};
		int errorCountLeft = 0, errorCountRight = 0;


		for (int y = yLimMin-1; y < yLimMax; y++)
		{
			float X =  sqrt((pupilR + 30)* (pupilR + 30) - (y-pupilY)*(y-pupilY));
			int xLimMin =  (int)(pupilX - X);
			int xLimMax =  (int)(pupilX + X);


			X =  sqrt((pupilR)* (pupilR) - (y-pupilY)*(y-pupilY));

			float givenCir3 = ((float)pupilX - X);
			float givenCir4 = ((float)pupilX + X);
			//printf("");
			int winSize = 3;
			int dataSize =  (xLimMax - xLimMin +1) + 2*winSize;


			float *lineData;
			int *lineX;
			lineData = (float *) malloc(dataSize*sizeof(float));
			lineX = (int *) malloc(dataSize*sizeof(int));

			for (int i = 0; i < dataSize ; i++ )
			{
				int x = (xLimMin - winSize) + i;
				lineX[i] = x;
				
				//float average =  (int)data[w*y+x];
				float average	= (float)(data[w*(y-1)+(x-1)] + data[w*y+(x-1)] + data[w*(y+1)+(x-1)] + 	// 1/9*[ 1  1  1
					data[w*(y-1)+x] + data[w*y+x] +data[w*(y+1)+x] + 	                                    //		 1  1  1
					data[w*(y-1)+(x+1)] + data[w*y+(x+1)] + data[w*(y+1)+(x+1)])/9;							//		 1  1  1]
				lineData[i] = average;

			}

			float *lineFirstDiff;
			float *lineSecondDiff;
			lineFirstDiff = (float *) malloc(dataSize*sizeof(float));
			lineSecondDiff = (float *) malloc(dataSize*sizeof(float));

			for (int i = winSize; i < dataSize - winSize  ; i++ )
			{
				lineFirstDiff[i] =   (lineData[i-3] + lineData[i-2] + lineData[i-1] - lineData[i+1] - lineData[i+2] - lineData[i+3]);            //[1  1  1  0 -1 -1 -1]
				lineSecondDiff[i] =	 10*(lineData[i-3] + lineData[i-2] + lineData[i-1] - 6*lineData[i] + lineData[i+1] + lineData[i+2] + lineData[i+3]); //[1 1 1 -6 1 1 1]
			}


			int *candidateX;
			int MaximaSize = 5;
			int candidateLength = 2*MaximaSize;
			candidateX = (int *) malloc(candidateLength*sizeof(int));
			candidateSearch(lineFirstDiff, dataSize, candidateX, candidateLength, winSize);

			float *candidateStd = 0;
			candidateStd = (float*) malloc(1*sizeof(float));

			float maxStd= -999;
			float minimumError = 9999;
			int predictedCir3 = givenCir3 + 15;

			for (int i = 0; i < candidateLength/2; i++)
			{   
				int minCand = findZeroCross(lineData, lineSecondDiff, dataSize, lineX, candidateX[i], givenCir3, winSize, candidateStd, true);
				float candError = abs(givenCir3-(lineX[minCand]));
				if (candidateStd[0] > maxStd && candError < minimumError && candidateStd[0] > 3.0)
				{	
					minimumError = candError;
					maxStd = candidateStd[0];
					predictedCir3 = lineX[minCand];
				}
			}

			maxStd= -9999;
			minimumError = 9999;
			int predictedCir4 = givenCir4 + 15;

			for (int i = candidateLength/2; i < candidateLength; i++)
			{   
				int minCand = findZeroCross(lineData, lineSecondDiff, dataSize, lineX, candidateX[i], givenCir4, winSize, candidateStd, false);
				float candError = abs(givenCir4-(lineX[minCand]));
				if (candidateStd[0] > maxStd && candError < minimumError && candidateStd[0] > 3.0)
				{	
					minimumError = candError;
					maxStd = candidateStd[0];
					predictedCir4 = lineX[minCand];
				}
			}


			bool specularity = false;
			for (int i=winSize; i < dataSize/2; i++)
				if (lineData[i] > 190) 
				{
					specularity = true; break;
				}

				if (!specularity)
					leftError[errorCountLeft++] = abs(givenCir3 - predictedCir3);

				specularity = false;
				for (int i = dataSize/2; i < dataSize - winSize; i++)
					if (lineData[i] > 190) 
					{
						specularity = true; break;
					}

					if (!specularity)
						rightError[errorCountRight++] = abs(givenCir4 - predictedCir4);


					free(candidateX);
					free(lineFirstDiff);
					free(lineSecondDiff);
					free(lineX);
					
		}

	
		qsort (leftError, errorCountLeft, sizeof(float), compare);
		qsort (rightError, errorCountRight, sizeof(float), compare);
		//printf("Input Image: %s , Left Median: %f, Right Median: %f\n", str.c_str(), leftError[errorCountLeft/2], rightError[errorCountRight/2]);

		float normErrorLeft = leftError[errorCountLeft/2]/pupilR;
		float normErrorRight = rightError[errorCountRight/2]/pupilR;

		clock_t end = clock();
		double time = (double) (end-start) / CLOCKS_PER_SEC * 1000.0;
		float score = normErrorLeft > normErrorRight? normErrorLeft: normErrorRight;
		printf("Input Image: %s,  Total Error:  %f, Time: %f\n", inputImage, score, time);
		free(data);
		

	

		return score < 0.13 ? 1:0 ;
}


int segmentation(unsigned char *data, int w, int h, float pupilX, float pupilY, float pupilR, float irisX, float irisY, float irisR)
{

    	clock_t start = clock();
		int yLimMin = (int)(pupilY - pupilR*sin (30*PI/180));
		int yLimMax = (int)(pupilY + pupilR*sin (50*PI/180));

		float leftError[640]= {0};
		float rightError[640] = {0};
		int errorCountLeft = 0, errorCountRight = 0;


		for (int y = yLimMin-1; y < yLimMax; y++)
		{
			float X =  sqrt((pupilR + 30)* (pupilR + 30) - (y-pupilY)*(y-pupilY));
			int xLimMin =  (int)(pupilX - X);
			int xLimMax =  (int)(pupilX + X);


			X =  sqrt((pupilR)* (pupilR) - (y-pupilY)*(y-pupilY));

			float givenCir3 = ((float)pupilX - X);
			float givenCir4 = ((float)pupilX + X);
			//printf("");
			int winSize = 3;
			int dataSize =  (xLimMax - xLimMin +1) + 2*winSize;


			float *lineData;
			int *lineX;
			lineData = (float *) malloc(dataSize*sizeof(float));
			lineX = (int *) malloc(dataSize*sizeof(int));

			for (int i = 0; i < dataSize ; i++ )
			{
				int x = (xLimMin - winSize) + i;
				lineX[i] = x;
				
				//float average =  (int)data[w*y+x];
				float average	= (float)(data[w*(y-1)+(x-1)] + data[w*y+(x-1)] + data[w*(y+1)+(x-1)] + 	// 1/9*[ 1  1  1
					data[w*(y-1)+x] + data[w*y+x] +data[w*(y+1)+x] + 	                                    //		 1  1  1
					data[w*(y-1)+(x+1)] + data[w*y+(x+1)] + data[w*(y+1)+(x+1)])/9;							//		 1  1  1]
				lineData[i] = average;

			}

			float *lineFirstDiff;
			float *lineSecondDiff;
			lineFirstDiff = (float *) malloc(dataSize*sizeof(float));
			lineSecondDiff = (float *) malloc(dataSize*sizeof(float));

			for (int i = winSize; i < dataSize - winSize  ; i++ )
			{
				lineFirstDiff[i] =   (lineData[i-3] + lineData[i-2] + lineData[i-1] - lineData[i+1] - lineData[i+2] - lineData[i+3]);            //[1  1  1  0 -1 -1 -1]
				lineSecondDiff[i] =	 10*(lineData[i-3] + lineData[i-2] + lineData[i-1] - 6*lineData[i] + lineData[i+1] + lineData[i+2] + lineData[i+3]); //[1 1 1 -6 1 1 1]
			}


			int *candidateX;
			int MaximaSize = 5;
			int candidateLength = 2*MaximaSize;
			candidateX = (int *) malloc(candidateLength*sizeof(int));
			candidateSearch(lineFirstDiff, dataSize, candidateX, candidateLength, winSize);

			float *candidateStd = 0;
			candidateStd = (float*) malloc(1*sizeof(float));

			float maxStd= -999;
			float minimumError = 9999;
			int predictedCir3 = givenCir3 + 15;

			for (int i = 0; i < candidateLength/2; i++)
			{   
				int minCand = findZeroCross(lineData, lineSecondDiff, dataSize, lineX, candidateX[i], givenCir3, winSize, candidateStd, true);
				float candError = abs(givenCir3-(lineX[minCand]));
				if (candidateStd[0] > maxStd && candError < minimumError && candidateStd[0] > 3.0)
				{	
					minimumError = candError;
					maxStd = candidateStd[0];
					predictedCir3 = lineX[minCand];
				}
			}

			maxStd= -9999;
			minimumError = 9999;
			int predictedCir4 = givenCir4 + 15;

			for (int i = candidateLength/2; i < candidateLength; i++)
			{   
				int minCand = findZeroCross(lineData, lineSecondDiff, dataSize, lineX, candidateX[i], givenCir4, winSize, candidateStd, false);
				float candError = abs(givenCir4-(lineX[minCand]));
				if (candidateStd[0] > maxStd && candError < minimumError && candidateStd[0] > 3.0)
				{	
					minimumError = candError;
					maxStd = candidateStd[0];
					predictedCir4 = lineX[minCand];
				}
			}


			bool specularity = false;
			for (int i=winSize; i < dataSize/2; i++)
				if (lineData[i] > 190) 
				{
					specularity = true; break;
				}

				if (!specularity)
					leftError[errorCountLeft++] = abs(givenCir3 - predictedCir3);

				specularity = false;
				for (int i = dataSize/2; i < dataSize - winSize; i++)
					if (lineData[i] > 190) 
					{
						specularity = true; break;
					}

					if (!specularity)
						rightError[errorCountRight++] = abs(givenCir4 - predictedCir4);


					free(candidateX);
					free(lineFirstDiff);
					free(lineSecondDiff);
					free(lineX);
					free(lineData);
					free(candidateStd);

					
		}

	
		qsort (leftError, errorCountLeft, sizeof(float), compare);
		qsort (rightError, errorCountRight, sizeof(float), compare);
		//printf("Input Image: %s , Left Median: %f, Right Median: %f\n", str.c_str(), leftError[errorCountLeft/2], rightError[errorCountRight/2]);

		float normErrorLeft = leftError[errorCountLeft/2]/pupilR;
		float normErrorRight = rightError[errorCountRight/2]/pupilR;

		clock_t end = clock();
		double time = (double) (end-start) / CLOCKS_PER_SEC * 1000.0;
		float score = normErrorLeft > normErrorRight? normErrorLeft: normErrorRight;
		// printf("Total Error:  %f, Time: %f\n",  score, time);
		//free(data);
		return score < 0.275 ? 1:0 ;
}
