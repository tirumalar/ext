#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>  
#include <time.h>  
#include <fstream>
#include "NanoFocusSpecularityMeasure.h"
using namespace std;
using namespace cv; 


int ismember(int BW2[][2],int te1,int te2,int s)
{
	int i,r=0;
	for(i=0;i<s;i++)
	{
		if(BW2[i][0]==te1 && BW2[i][1]==te2)
		{
			r=1;
			break;
		}
	}
	return(r);

}

int getdata_bwCPP(IplImage* img,int BW2[500][2])
{


	//Function to get Boundary and number of boundary pixels
	//BW2 -- Boundary pixels
	//size_bw -- Number of boundary pixels


	int s1=img->height;
	int s2=img->width;
	int bw[500][500]={0};
	int i,j;int mj;

	for(i=0; i<s1; i++)
    {
      for(j=0; j<s2; j++)
       {
		   mj = CV_IMAGE_ELEM( img, uchar, i, j);
		   if(mj==255)
			   bw[i][j]=1;
	   }
     }

	int BW1[500][2]={0},count=0,x,y,count1=0;
	for(i=1;i<s1-1;i++)
	{
		for(j=1;j<s2-1;j++)
		{
			if(bw[i][j]==1)
			{
				if(bw[i-1][j]==0 || bw[i][j-1]==0 || bw[i][j+1]==0 || bw[i+1][j]==0)
				{
					BW1[count][0]=i;
					BW1[count][1]=j;
					count=count+1;
				}
			}
		}
	}

	for(j=1;j<s2-1;j++)
	{
		for(i=1;i<s1-1;i++)
		{
			if(bw[i][j]==1)
			{
				if(count1==0)
				{
					x=i;y=j;
					count1++;
				}
			}
		}
	}
	int C=1,l=1,a=0,b=1;
	int x1=x,y1=y;
	BW2[0][0]=x;BW2[0][1]=y;
	

	while(l>0)
	{
		
		if(bw[x][y-1]==1 && a==ismember(BW2,x,y-1,500) && b==ismember(BW1,x,y-1,500))
		{
			
				y=y-1;x=x;
				BW2[C][0]=x;BW2[C][1]=y;
				C++;
			
			
		}
		else if(bw[x+1][y-1]==1 && a==ismember(BW2,x+1,y-1, 500) && b==ismember(BW1,x+1,y-1,500))
		{
			
				x=x+1;y=y-1;
				BW2[C][0]=x;BW2[C][1]=y;
				C++;
			
		}
		else if(bw[x+1][y]==1 && a==ismember(BW2,x+1,y,500) && b==ismember(BW1,x+1,y,500))
		{
			
				x=x+1;y=y;
				BW2[C][0]=x;BW2[C][1]=y;
				C++;
			
		}
		else if(bw[x+1][y+1]==1 && a==ismember(BW2,x+1,y+1,500) && b==ismember(BW1,x+1,y+1,500))
		{
			
				x=x+1;y=y+1;
				BW2[C][0]=x;BW2[C][1]=y;
				C++;
			
		}
		else if(bw[x][y+1]==1 && a==ismember(BW2,x,y+1,500) && b==ismember(BW1,x,y+1,500))
		{
			
				x=x;y=y+1;
				BW2[C][0]=x;BW2[C][1]=y;
				C++;
			
		}
		else if(bw[x-1][y+1]==1 && a==ismember(BW2,x-1,y+1,500) && b==ismember(BW1,x-1,y+1,500))
		{
			
				x=x-1;y=y+1;
				BW2[C][0]=x;BW2[C][1]=y;
				C++;
			
		}
		else if(bw[x-1][y]==1 && a==ismember(BW2,x-1,y,500) && b==ismember(BW1,x-1,y,500))
		{
			
				x=x-1;y=y;
				BW2[C][0]=x;BW2[C][1]=y;
				C++;
			
		}
		else if(bw[x-1][y-1]==1 && a==ismember(BW2,x-1,y-1,500) && b==ismember(BW1,x-1,y-1,500))
		{	
				x=x-1;y=y-1;
				BW2[C][0]=x;BW2[C][1]=y;
				C++;
			
		}
		else
		{
			l=0;
		}

		if((x==x1 && y==y1) || C>499)
		{
			l=0;
		}
	}
	return(C);
}

bool checkBoundary(IplImage* image)
{

	NanoFocusSpecularityMeasure *m_nanoFocusSpecularityMeasure = new NanoFocusSpecularityMeasure();
	m_nanoFocusSpecularityMeasure->SetSpecularityValue(255);
	
	
	float hscore1 = m_nanoFocusSpecularityMeasure->ComputeHaloScoreTopPointsPico(image,255,15,25.0,115,200);
	CvRect rectH = m_nanoFocusSpecularityMeasure->GetSpecularityROI();


	if (rectH.height < 5 || rectH.width < 5)   
	{	
		printf(" Small specularity  box found in Check Boundary...\n");
		return true;
	}

	else if (rectH.height > 50 || rectH.width > 50)
	{	
		printf(" Large specularity  box found in Check Boundary....\n");
		return true;

	}
	else 
	{	
		
		
		rectH.height = rectH.height+1;
		rectH.width = rectH.width+1;
		cvSetImageROI(image, rectH);
		IplImage* roiImg = cvCreateImage(cvGetSize(image), image->depth, image->nChannels);
		cvCopy(image, roiImg, NULL);
		cvResetImageROI(image);
		
		int  BW_data[500][2] = {0};

		int BW_size;
		BW_size = getdata_bwCPP(roiImg,BW_data);
		unsigned char** matrix = new unsigned char*[500];
		for(int i = 0; i < 500; i++)
				matrix[i] = new unsigned char[500];

		for (int bwIdx=0; bwIdx < BW_size;  bwIdx++)
		{
			int x = (int)BW_data[bwIdx][0];
			int y = (int)BW_data[bwIdx][1];
			matrix[x][y] = (unsigned char)255;
		}

		cvReleaseImage(&roiImg);
		for (int bwIdx=0; bwIdx <BW_size; bwIdx++)
		{

			int x = (int)BW_data[bwIdx][0];
			int y = (int)(int)BW_data[bwIdx][1];
			int sumWhite = 0;
			for (int i = x-1; i <= x+1; i++)
			{
				for (int j = y-1; j <= y+1; j++)
				{
					if (matrix[i][j] == 255)
						sumWhite++;
				}

			}
			if (sumWhite < 3 || sumWhite > 5)
			{
				
				for(int i = 0; i < 500; ++i)
						 delete [] matrix[i];
				delete [] matrix;
				return true;			
			}
		}
		for(int i = 0; i < 500; ++i)
			 delete [] matrix[i];
		delete [] matrix;
		return false;    
	}

}
