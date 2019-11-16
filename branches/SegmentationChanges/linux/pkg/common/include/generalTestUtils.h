#pragma once

#include <tut/tut.hpp>
#include <cxcore.h>
#include <highgui.h>
#include "apr_strings.h"
#include "apr_file_io.h"

#include <string>
#include <fstream>
#include <stdio.h>


template<class T>
void compareImage(IplImage *out, IplImage *outexp, CvRect roi, T scale, T tolerance,const char *msg)
{
	tut::ensure("Image dimensions must be within roi", roi.width <= out->width);
	tut::ensure("Image dimensions must be within roi", roi.height <= out->height);

	for(int i=0;i<roi.height;i++)
	{
		T *o   = (T *) (out->imageData + (i+roi.y)* out->widthStep) + roi.x; 
		T *oi   = (T *) (outexp->imageData + (i+roi.y)* outexp->widthStep) + roi.x; 
		for(int j=0;j< roi.width;j++)
			tut::ensure_distance(msg, o[j], (T) (scale*oi[j]), tolerance);
	}
}
template<class T>
void compareImage(IplImage *out, const char *outexp, int widthStep, CvRect roi, T scale, T tolerance,const char *msg)
{
	tut::ensure("Image dimensions must be within roi", roi.width <= out->width);
	tut::ensure("Image dimensions must be within roi", roi.height <= out->height);
	tut::ensure("widthstep must be greater than roi.width", widthStep > roi.width);

	for(int i=0;i<roi.height;i++)
	{
		const T *o   = (const T *) (out->imageData + (i+roi.y)* out->widthStep) + roi.x; 
		const T *oi   = (const T *) (outexp + (i+roi.y) * widthStep) + roi.x; 
		for(int j=0;j< roi.width;j++)
			tut::ensure_distance(msg, o[j], (T) (scale*oi[j]), tolerance);
	}
}
template<class T>
void compareImage(const char *out, int widthStep, const char *outexp, int widthExpStep, CvRect roi, T scale, T tolerance,const char *msg)
{
	tut::ensure("widthstep must be greater than roi.width", widthStep > roi.width);
	tut::ensure("widthstep must be greater than roi.width", widthExpStep > roi.width);

	for(int i=0;i<roi.height;i++)
	{
		const T *o   = (const T *) (out + (i+roi.y)* widthStep) + roi.x; 
		const T *oi   = (const T *) (outexp + (i+roi.y) * widthExpStep) + roi.x; 
		for(int j=0;j< roi.width;j++)
			tut::ensure_distance(msg, o[j], (T) (scale*oi[j]), tolerance);
	}
}
template<class T>
void compareRaw(const char *out, const char *outexp, apr_size_t length, T scale, T tolerance,const char *msg)
{
	const T *outT = (const T *) out;
	const T *outExpT = (const T *) outexp;

	for(int i=0;i<length;i++)
		tut::ensure_distance(msg, outT[i], (T) (scale*outExpT[i]), tolerance);
}

static void compareRaw8U(const char *out, char *outexp, apr_size_t length, const char *msg, int tolerance=1){
	compareRaw<unsigned char>(out, outexp, length, (unsigned char)(1),(unsigned char)(tolerance),msg);
}

static void compareImage8U(IplImage *out, IplImage *outexp, CvRect roi,const char *msg,int tolerance=1){
	compareImage<unsigned char>(out,outexp,roi,(unsigned char)(1),(unsigned char)(tolerance),msg);
}
static void compareImage16U(IplImage *out, IplImage *outexp, CvRect roi,const char *msg,int tolerance=1){
	compareImage<unsigned short>(out,outexp,roi,(unsigned short)(1),(unsigned short)(tolerance),msg);
}
static void compareImage16S(IplImage *out, IplImage *outexp, CvRect roi,const char *msg,int tolerance=1){
	compareImage<short>(out,outexp,roi,(short)(1),(short)(tolerance),msg);
}
static void compareImage32F(IplImage *out, IplImage *outexp, CvRect roi, const char *msg, float tolerance=1.0, float scale=1.0){
	compareImage<float>(out,outexp,roi,scale,tolerance,msg);
}


static void compareImage8U(IplImage *out, const char *outexp, int widthStep, CvRect roi,const char *msg,int tolerance=1){
	compareImage<unsigned char>(out,outexp,widthStep,roi,(unsigned char)(1),(unsigned char)(tolerance),msg);
}
static void compareImage16U(IplImage *out, const char *outexp, int widthStep, CvRect roi,const char *msg,int tolerance=1){
	compareImage<unsigned short>(out,outexp,widthStep,roi,(unsigned short)(1),(unsigned short)(tolerance),msg);
}
static void compareImage16S(IplImage *out, const char *outexp, int widthStep, CvRect roi,const char *msg,int tolerance=1){
	compareImage<short>(out,outexp,widthStep,roi,(short)(1),(short)(tolerance),msg);
}
static void compareImage32F(IplImage *out, const char *outexp, int widthStep, CvRect roi, const char *msg, float tolerance=1.0, float scale=1.0){
	compareImage<float>(out,outexp,widthStep,roi,scale,tolerance,msg);
}

static void compareImage8U(const char *out, int widthStep, const char *outexp, int widthExpStep, CvRect roi,const char *msg,int tolerance=1){
	compareImage<unsigned char>(out,widthStep,outexp,widthExpStep,roi,(unsigned char)(1),(unsigned char)(tolerance),msg);
}
static void compareImage16U(const char *out, int widthStep, const char *outexp, int widthExpStep, CvRect roi,const char *msg,int tolerance=1){
	compareImage<unsigned short>(out,widthStep,outexp,widthExpStep,roi,(unsigned short)(1),(unsigned short)(tolerance),msg);
}
static void compareImage16S(const char *out, int widthStep, const char *outexp, int widthExpStep, CvRect roi,const char *msg,int tolerance=1){
	compareImage<short>(out,widthStep,outexp,widthExpStep,roi,(short)(1),(short)(tolerance),msg);
}
static void compareImage32F(const char *out, int widthStep, const char *outexp, int widthExpStep, CvRect roi, const char *msg, float tolerance=1.0, float scale=1.0){
	compareImage<float>(out,widthStep,outexp,widthExpStep,roi,scale,tolerance,msg);
}

static bool doesFileExist(const char *fileName, apr_pool_t *mempool)
{
	apr_file_t *ft = 0;
	apr_status_t rv=apr_file_open(&ft, fileName, APR_READ, APR_OS_DEFAULT, mempool);
	bool success = (rv == APR_SUCCESS && ft != 0);

	if(ft)	apr_file_close(ft);

	return success;
}

static apr_size_t readFile(const char *fileName, apr_pool_t *mempool, char **data)
{
	apr_file_t *ft = 0;
	apr_status_t rv = apr_file_open(&ft, fileName, APR_READ|APR_BINARY, APR_OS_DEFAULT, mempool);
	tut::ensure("File must exist", (rv == APR_SUCCESS && ft != 0));

	apr_finfo_t finfo;
	apr_file_info_get(&finfo, APR_FINFO_SIZE, ft);
	
	apr_size_t fileLength = (apr_size_t) finfo.size;

	*data = (char *) apr_palloc(mempool, fileLength);
	apr_size_t nBytes = fileLength;

	rv = apr_file_read(ft, *data, &nBytes);

	tut::ensure("File must get read", rv == APR_SUCCESS && nBytes == fileLength);

	if(ft)	apr_file_close(ft);

	return nBytes;
}

static void checkFileExists(const char *fileName, apr_pool_t *mempool, bool bExists)
{
	bool fileExists = doesFileExist(fileName, mempool);

	if(bExists)
		tut::ensure("file must exists", fileExists);
	else
		tut::ensure("file must not exist", !fileExists);
}

static void SaveOrCompare8U(IplImage *out, const char *outExpFileName, int loadFlags, CvRect roi, const char *msg, int tolerance=1)
{
	apr_pool_t *mempool = 0;
	
	apr_status_t rv = apr_pool_create(&mempool, 0);
	tut::ensure_equals("Pool creation must succeed", rv, APR_SUCCESS);

	if(doesFileExist(outExpFileName, mempool))
	{
		IplImage *expImg = cvLoadImage(outExpFileName, loadFlags);
		try 
		{
			compareImage8U(out, expImg, roi, msg, tolerance);
		}
		catch(tut::failure &f)
		{
			char *outStr = apr_pstrdup(mempool, outExpFileName);
			char *dotptr = strrchr(outStr, '.');	
			if(dotptr)
			{
				dotptr[0] = 0;
				char *outStr2 = apr_pstrcat(mempool, outStr, "_t.", dotptr+1, 0);
				cvSaveImage(outStr2, out);
			}
			apr_pool_destroy(mempool);
			cvReleaseImage(&expImg);
			throw(f);
		}
	}
	else
	{
		cvSaveImage(outExpFileName, out);
	}

	apr_pool_destroy(mempool);
}

static void saveDataToFile(const char *fileName, const char *data, apr_pool_t *mempool, apr_size_t length)
{
	apr_file_t *ft = 0;
	apr_status_t rv=apr_file_open(&ft, fileName, APR_WRITE|APR_CREATE|APR_BINARY|APR_TRUNCATE, APR_OS_DEFAULT, mempool);
	tut::ensure("Must be able to create file", (rv == APR_SUCCESS && ft != 0));

	apr_size_t bytesWritten = length;

	rv = apr_file_write(ft, data, &bytesWritten);

	tut::ensure("Must be able to write to file", (rv == APR_SUCCESS && bytesWritten == length));

	apr_file_close(ft);
}

static void SaveOrCompareRaw8U(const char *out, const char *outExpFileName, apr_size_t length, const char *msg, int tolerance=1)
{
	apr_pool_t *mempool = 0;
	
	apr_status_t rv = apr_pool_create(&mempool, 0);
	tut::ensure_equals("Pool creation must succeed", rv, APR_SUCCESS);

	if(doesFileExist(outExpFileName, mempool))
	{
		char *data = 0;
		apr_size_t rlen = readFile(outExpFileName, mempool, &data);
		tut::ensure_equals("Length must match", length, rlen);

		try 
		{
			compareRaw8U(out, data, length, msg, tolerance);
		}
		catch(tut::failure &f)
		{
			apr_pool_destroy(mempool);
			throw(f);
		}
	}
	else
	{
		saveDataToFile(outExpFileName, out, mempool, length);
	}

	apr_pool_destroy(mempool);
}

static void SaveOrCompareToFile(const char *outFile, const char *outExpFileName, const char *msg)
{
	apr_pool_t *mempool = 0;
	
	apr_status_t rv = apr_pool_create(&mempool, 0);
	tut::ensure_equals("Pool creation must succeed", rv, APR_SUCCESS);

	if(doesFileExist(outExpFileName, mempool))
	{
		try 
		{		
			tut::ensure(msg, doesFileExist(outFile, mempool));
			char *out = 0;
			apr_size_t length = readFile(outFile, mempool, &out);

			char *data = 0;
			apr_size_t rlen = readFile(outExpFileName, mempool, &data);
			tut::ensure_equals("Length must match", length, rlen);

			compareRaw8U(out, data, length, msg, 1);
		}
		catch(tut::failure &f)
		{
			apr_pool_destroy(mempool);
			throw(f);
		}
	}
	else
	{
		apr_file_rename(outFile, outExpFileName, mempool);
	}

	apr_pool_destroy(mempool);

}


static unsigned char xtod(char c) {
	if (c >= '0' && c <= '9')
		return c - '0';
	if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	return c = 0; // not Hex digit
}

static bool loadIris(int id, unsigned char *buff,string& expResp) {
	char fName[100];
	sprintf(fName, "data/I%d.txt", id);
	printf("Loading iris file: %s\n",fName);
	std::ifstream myfile(fName);
	std::string line;
	int lineCnt=0;
	if (myfile.is_open()) {
		while (!myfile.eof()) {
			std::getline(myfile, line);
			if(lineCnt==2)
			{
				expResp=line;
				break;
			}
			unsigned int b = 0;
			for (; b < line.size(); b++) {
				if (line[b] == 'x' || line[b] == 'X') {
					b++;
					break;
				}
			}
			for (int i = 0; b < line.size() && i < 1280; b = b + 2, i++) {
				buff[i] = xtod(line[b]);
				buff[i] = buff[i] << 4;
				buff[i] = buff[i] | xtod(line[b + 1]);
			}
			buff += 1280;
			lineCnt++;
		}
		myfile.close();
		return true;
	} else
		return false;
}

static void loaddb(const char *dbFile, unsigned char **outBuf, unsigned int *size)
{
	std::ifstream myfile;
	myfile.open(dbFile, ifstream::binary|ifstream::in);
	*outBuf = (unsigned char *)malloc(10*1024*1024);	// we dont have file bigger than 10M
	myfile.read((char *)*outBuf,10*1024*1024);
	*size=myfile.gcount();
}
