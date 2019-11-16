/*
 * ImageMsgWriter.cpp
 *
 *  Created on: 19 Feb, 2009
 *      Author: akhil
 */
#include <stdio.h>
#include "ImageMsgWriter.h"

ImageMsgWriter::ImageMsgWriter(Configuration *pConf){
	svrAddr	=pConf->getValue("GRI.serverAddress","192.168.10.2:8081");
	url		=pConf->getValue("GRI.url","/img");
	boundary=pConf->getValue("GRI.boundary","538ee63c1515");
	cameraID=pConf->getValue("GRI.cameraID","Unknown");
	//const char *hbMsgFmt=pConf->getValue("GRI.heartBeatMsgFormat","HEARTBEAT cameraId=\"%s\";");
	const char *hbMsgFmt="HEARTBEAT cameraId=\"%s\";";
	sprintf(hbMsg,hbMsgFmt,cameraID);
	hbMsgLen=strlen(hbMsg);

	footerLen=sprintf(footer,"\r\n\r\n----------------------------%s--\r\n",boundary);
}

void fillContentLength(char *buff, int total){
	// locate "-Length:"
	char *fillPtr=9+strstr(buff,"-Length:");
	//locate boundary definition
	char *contentPtr=10+strstr(fillPtr,"----------------------------");
	//locate first use of boundary
	contentPtr=strstr(contentPtr,"----------------------------");
	// determine the length
	int length=total-(contentPtr-buff);
	assert(length<9999999);	// we left a gap for 7 digits
	// now write it there
	char lenBuff[16];
	int size=sprintf(lenBuff,"%d",length);
	// can not use sprintf as it puts a null after string
	memcpy(fillPtr,lenBuff,size);
}


int ImageMsgWriter::write(char *outBuff, int maxSize, IplImage *img, int id, int cameraId, int frameId, int imageId, int nImages, int x, int y, float scale,int score,uint64_t currtimestamp,int ill0,
		int spoof,float areascore,float focusscore,float blc,float irx, float iry,int prev,float halo,int numbits)
{
	char *orgBuff=outBuff;
	static char	hdrFmt[]="POST %s HTTP/1.1\r\n"
		"User-Agent: curl/7.17.1 (i586-pc-mingw32msvc) libcurl/7.17.1 OpenSSL/0.9.8g zlib/1.2.3\r\n"
		"Host: %s\r\n"
		"Accept: */"
		"*\r\n"
		"Content-Length:        \r\n"
		"Expect: 100-continue\r\n"
		"Content-Type: multipart/form-data; boundary=----------------------------%s\r\n"
		"\r\n"
		"----------------------------%s\r\n"
		"Content-Disposition: form-data; name=\"description\"; filename=\"form_description.xml\"\r\n"
		"Content-Type: application/octet-stream\r\n"
		"\r\n"
		"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n"
		"<images>\r\n"
		"<image id=\"%d\" cameraId=\"%d\" frameId=\"%d\" imageId=\"%d\" numberOfImages=\"%d\" scale=\"%3.2f\" x=\"%d\" y=\"%d\" width=\"%d\""
		" height=\"%d\" score=\"%d\" maxValue=\"255\" timeStamp=\"%llu\" Il0=\"%d\" Spoof=\"%d\" ascore=\"%f\" fscore=\"%f\" blc=\"%f\" irx=\"%f\" iry=\"%f\" halo=\"%f\" prev=\"%d\" numbits=\"%d\" file_name=\"image_%d.bin\">\r\n"
		"</image>\r\n"
		"<iris diameter=\"120\"/>\r\n"
		"</images>\r\n"
		"\r\n"
		"----------------------------%s\r\n"
		"Content-Disposition: form-data; name=\"file_name\"; filename=\"image_%d.bin\"\r\n"
		"Content-Type: application/octet-stream\r\n"
		"\r\n";

	int count=sprintf(outBuff,hdrFmt,url,svrAddr,boundary,boundary,
			id,cameraId,frameId,imageId,nImages,scale,x,y,img->width,img->height,score,currtimestamp,ill0,spoof,areascore,focusscore,blc,irx,iry,halo,prev,numbits,
			id,boundary,id);
	outBuff+=count;

	count=count+img->imageSize+footerLen;
	assert(maxSize>count);

	memcpy(outBuff,img->imageData,img->imageSize);
	outBuff+=img->imageSize;

	memcpy(outBuff,footer,footerLen);

	fillContentLength(orgBuff, count);

	return count;
}

int ImageMsgWriter::write_bitalign(char *outBuff, int maxSize, IplImage *img, int id, int frameId, int imageId, int nImages, int x, int y, float scale)
{
	char *orgBuff=outBuff;
	static char	hdrFmt[]="POST %s HTTP/1.1\r\n"
		"User-Agent: curl/7.17.1 (i586-pc-mingw32msvc) libcurl/7.17.1 OpenSSL/0.9.8g zlib/1.2.3\r\n"
		"Host: %s\r\n"
		"Accept: */"
		"*\r\n"
		"Content-Length:        \r\n"
		"Expect: 100-continue\r\n"
		"Content-Type: multipart/form-data; boundary=----------------------------%s\r\n"
		"\r\n"
		"----------------------------%s\r\n"
		"Content-Disposition: form-data; name=\"description\"; filename=\"form_description.xml\"\r\n"
		"Content-Type: application/octet-stream\r\n"
		"\r\n"
		"<?xml version=\"1.0\" encoding=\"utf-8\" ?>\r\n"
		"<images>\r\n"
		"<image id=\"%d\" cameraId=\"%s\" frameId=\"%d\" imageId=\"%d\" numberOfImages=\"%d\" scale=\"%3.2f\" x=\"%d\" y=\"%d\" width=\"%d\" height=\"%d\" maxValue=\"255\" file_name=\"image_%d.bin\">\r\n"
		"</image>\r\n"
		"<iris diameter=\"120\"/>\r\n"
		"</images>\r\n"
		"\r\n"
		"----------------------------%s\r\n"
		"Content-Disposition: form-data; name=\"file_name\"; filename=\"image_%d.bin\"\r\n"
		"Content-Type: ";
	static char	hdrFmt1[]=
		"application/octet-stream\r\n"
		"\r\n";
	static char *filler="    ";

	static int hdr1size = strlen(hdrFmt1);

	int count=sprintf(outBuff,hdrFmt,url,svrAddr,boundary,boundary,
			id,cameraID,frameId,imageId,nImages,scale,x,y,img->width,img->height,
			id,boundary,id);
	outBuff+=count;

	int bitalign = ((int)outBuff+hdr1size)&0x3;
	if(bitalign != 0)
	{
		memcpy(outBuff,filler,4-bitalign);
		outBuff+= (4-bitalign); //Alignment spaces
		count +=(4-bitalign);
	}
	memcpy(outBuff,hdrFmt1,hdr1size);
	outBuff+=hdr1size;
	count+=hdr1size;

	count=count+img->imageSize+footerLen;
	assert(maxSize>count);

	memcpy(outBuff,img->imageData,img->imageSize);
	outBuff+=img->imageSize;

	memcpy(outBuff,footer,footerLen);

	fillContentLength(orgBuff, count);

	return count;
}
int ImageMsgWriter::writeHB(char *outBuff,int maxSize){
	assert(hbMsgLen<maxSize);
	memcpy(outBuff,hbMsg, hbMsgLen);
	return hbMsgLen;
}

