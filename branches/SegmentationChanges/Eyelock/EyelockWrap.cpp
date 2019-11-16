/*
 * EyelockWrap.c
 *
 *  Created on: Apr 20, 2011
 *      Author: developer1
 */

#include "EyelockWrap.h"
#include "EyelockAPI.h"
#include <utility>
#include "logging.h"
extern "C"{
#include "file_manip.h"
}

static EyelockAPI *Eyelock=NULL;
static ConfAPI *Conf=NULL;

void ConfInit(char *str)
{
	Conf = new ConfAPI(str);
}

void ConfExit()
{
	if(Conf)
		delete Conf;
	Conf = NULL;
}

void EyelockInit(char *str)
{
	if(Eyelock)
		LOGI("EYELOCK FOUND PRE INITIALIZED Memory Leak \n");
	Eyelock = new EyelockAPI(str);
}

char* GetString(char* key)
{
	if(Conf)
	{
		char* ptr = Conf->GetString(key);
		return ptr;
	}
	else
	{
		return 0;
	}
}

int GetInt(char* key)
{
	if(Conf)
	{
		int val = Conf->GetInt(key);
		return val;
	}
	else
	{
		return 0;
	}
}

int GetConfParam(const char *name)
{
	return (Eyelock) ? Eyelock->GetConfParam(name) : -1;
}

void GetLEDSpot(int *xy)
{
	if(Eyelock)
		Eyelock->GetLEDSpot(xy);
}

int AppendDB(char *ptr1,char* ptr2,char* fname, char* file)
{
	return (Eyelock != NULL) ? Eyelock->AppendDB(ptr1,ptr2,fname,file) : -1;
}

void EyelockProcess(char *img,int* indx,float* score)
{
	if(Eyelock)
	{
		int idx;
		float scr;
		Eyelock->Process(img,&idx,&scr);
		*indx = idx;
		*score = scr;
	}
	return;
}

void SetDoSaveDetections(int save)
{
	if(Eyelock)
		Eyelock->SetDoSaveDetections(save);
}

void EnableEyelidMask(int value)
{
	if(Eyelock)
		Eyelock->EnableEyelidMask(value);
}

void EyelockExit()
{
	if(Eyelock)
		delete Eyelock;
	Eyelock = NULL;
}

void EyelockDbUpdate(char* dbptr,char* key)
{
	if(Eyelock)
		Eyelock->pMatchProcessor->UpdateDB(dbptr,key);
}
void EyelockReloadDB()
{
	if(Eyelock)
		Eyelock->pMatchProcessor->ReloadDB();
}

char *LoadFile(char *name, int size, int idx)
{
	if(Eyelock)
	{
		char *buff= Eyelock->getBuffer(idx);
		int w=0;
		int h=0;
		ReadPGM5(name,(unsigned char *)buff,&w,&h,size);
		if(w*h!=size) LOGI("Bad size");
		return buff;
	}
	else
	{
		return 0;
	}
}

// API for EyelockJ

unsigned char *GetEnrollmentEye(int index)
{
	LOGI("GetEnrollmentEye =>");
	return (Eyelock !=  NULL) ? Eyelock->GetEnrollmentEye(index) : 0;
}

int DoMatch(float *score, char **personName )
{
	return (Eyelock != NULL) ? Eyelock->DoMatch(score, personName) : -1;
}

int DoTestMatch(unsigned char *inscode, float *score, char **personName)
{
	return (Eyelock != NULL) ? Eyelock->DoMatch(inscode, score, personName) : -1;
}

int FindEyes(char *ptr)
{
	return (Eyelock != NULL) ? Eyelock->FindEyes(ptr) : -1;
}

void SaveEyes(int idx)
{
	if(Eyelock)
		Eyelock->SaveEyes(idx);
}

void FlushAllEyes()
{
	if(Eyelock)
		Eyelock->FlushAll();
}

int SaveBestPairOfEyes(const char *personName)
{
	return (Eyelock != NULL) ? Eyelock->SaveBestPairOfEyes(personName) : -1;
}

int StoreBestPairOfEyes(const char *personName)
{
	return (Eyelock != NULL) ? Eyelock->StoreBestPairOfEyes(personName) : -1;
}

int GetIrisCode(unsigned char *imageBuffer, char *Iriscode, float *robustFeatures)
{
	return (Eyelock != NULL) ? Eyelock->GetIrisCode(imageBuffer, Iriscode,robustFeatures) : -1;
}

float MatchPair(unsigned char *ins, unsigned char *ref)
{
	if(Eyelock != NULL)
	{
		std::pair<int, float> score = Eyelock->pMatchProcessor->GetbioInstance()->MatchIrisCodeSingle((char *)ins,(char *)ref);
		return score.second;
	}
	else
	{
		return 1.0;
	}
}

//horizontal (flip=0),
//vertical (flip=1)
//both(flip=-1) axises:



#if 0

void Flip(IplImage* src,int flip){
	char buff[100]={0};
	sprintf(buff,"/mnt/sdcard/Eyelock/Inp_%d.pgm",flip);
	savefile_OfSize_asPGM((unsigned char*)src->imageData,src->width,src->height,buff);

	TIME_OP("cvFlip",
		cvFlip(src,NULL,flip)
	);
	sprintf(buff,"/mnt/sdcard/Eyelock/Out_%d.pgm",flip);
	savefile_OfSize_asPGM((unsigned char*)src->imageData,src->width,src->height,buff);
}

void FlipFrame(unsigned char *ptr){
	int w = 2048;
	int h = 1536;

	IplImage *src = cvCreateImage(cvSize(w,h),IPL_DEPTH_8U,1);
	memcpy(src->imageData,ptr,src->imageSize);
	Flip(src,0);
	memcpy(src->imageData,ptr,src->imageSize);
	Flip(src,1);
	memcpy(src->imageData,ptr,src->imageSize);
	Flip(src,-1);
	cvReleaseImage(&src);
}

int main(void){
	int w= 1024,h=600;
	int indx;
	float score;
	EyelockInit("/data/Eyelock.ini");
	char buffL[2560]={0};
	char buffR[2560]={0};

	buffL[0] = 1;
	buffL[1280] = 2;
	buffR[0] = 3;
	buffR[1280] = 4;

	AppendDB((char*)buffL,(char*)buffR,"Akhil Kumar","/data/Akhil.bin");
	AppendDB((char*)buffL,(char*)buffR,"Madhav S S","/data/Akhil.bin");

//	char *ptr = (char*)malloc(w*h);
//	int ret = ReadPGM5("/data/Image.pgm",(unsigned char *)ptr,&w,&h,w*h);
//	printf("EyelockProcess->entering\n");
//  EyelockProcess(ptr,&indx,&score);
//  printf("EyelockProcess->exiting\n");
//	EyelockExit();
//  free(ptr);
	printf("EXIT\n");
}
#endif


