/*
 * file_manip.c
 *
 *  Created on: 23 Aug, 2008
 *      Author: akhil
 */
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#include "file_manip.h"

unsigned char * file_OfSize_readFromOffset_into(const char *fileName, int imagesize,int offset, unsigned char* buffer){
	int fd = open(fileName, O_RDONLY);
	if(offset != lseek(fd, offset,SEEK_SET)){
		printf("something troubling. Offset set to %d\n", offset );
	}
	read(fd, (void *)buffer, imagesize );
	close (fd);
	return buffer;

}
/* Scan whitespace and comments.
 * Pre- and post-condition: *c holds first unscanned character.
 */
void PPM_scan_ws(unsigned char *c, FILE *fp)
{
    /* Scan one comment line */
    if (*c=='#')
      while((*c = getc(fp)) != '\n')
        continue;
    /* Scan whitespace */
    while (isspace(*c)) {
        *c = getc(fp);
        /* Scan embedded comment lines */
        if (*c=='#')
          while ((*c = getc(fp)) != '\n')
            continue;
    }
}

int ReadPGM5WHandBits(const char *fname, int *w, int *h, int *bits)
{
    unsigned char c;
    int maxval;
    char buf[2];
    FILE *fp = NULL;
    fp = fopen(fname, "rb");

    if (!fp)
      goto close_fail1;

    fread(buf, 1, 2, fp);
    if (buf[0] != 'P'  || buf[1] != '5')
      goto close_fail1;

    *w = *h = 0;
    maxval = 0;

    c = getc(fp);

    /* Scan for image width */
    PPM_scan_ws(&c, fp);
    while (isdigit(c)) {
      *w = 10*(*w) + c - '0';
      c = getc(fp);
    }

    /* Scan for image height */
    PPM_scan_ws(&c, fp);
    while (isdigit(c)) {
      *h = 10*(*h) + c - '0';
      c = getc(fp);
    }

    /* Scan for maxval */
    PPM_scan_ws(&c, fp);
    while (isdigit(c)) {
      maxval = 10*maxval + c - '0';
      c = getc(fp);
    }
	*bits = 0;
    if(maxval == 255)
    	*bits = 8;
    if(maxval == 65535)
    	*bits = 16;

    if(fp)fclose(fp);

//    printf("%s::W H b %d %d %d \n",fname,*w,*h,*bits);
    return 0;
close_fail1:
    if(fp)fclose(fp);
    return -1;
}

int ReadPGM5(const char *fname, unsigned char *data, int *w, int *h, int bufferSize)
{
    unsigned char c;
    unsigned short *ptr;
    int maxval;
    int i,bytesz=1;
    int size;
    char buf[2];
    FILE *fp = NULL;
    fp = fopen(fname, "rb");

    if (!fp)
      goto close_fail;

    fread(buf, 1, 2, fp);
    if (buf[0] != 'P' || buf[1] != '5')
      goto close_fail;

    *w = *h = 0;
    maxval = 0;

    c = getc(fp);

    /* Scan for image width */
    PPM_scan_ws(&c, fp);
    while (isdigit(c)) {
      *w = 10*(*w) + c - '0';
      c = getc(fp);
    }

    /* Scan for image height */
    PPM_scan_ws(&c, fp);
    while (isdigit(c)) {
      *h = 10*(*h) + c - '0';
      c = getc(fp);
    }

    /* Scan for maxval */
    PPM_scan_ws(&c, fp);
    while (isdigit(c)) {
      maxval = 10*maxval + c - '0';
      c = getc(fp);
    }

    if (!isspace(c) || maxval <= 0)
      goto close_fail;

    if(maxval == 65535)
    	bytesz=2;

    size=(*w)*(*h)*bytesz;
    if (size>bufferSize)
      goto close_fail;


   	i=fread(data,1,size, fp);
    if( i!= size)
      goto corrupt_fail;

    if(bytesz == 2){
    	ptr = data;
    	for(i=0;i<size>>1;i++){
    		ptr[i]= (ptr[i]>>8)|(ptr[i]<<8);
    	}
    }

    fclose(fp);
    return(0);

    close_fail:    /* Invalid image header, or no file.  *data is NULL. */
    if (fp) fclose(fp);
    return -1;

    corrupt_fail:  /* Some image data was read.  *data is not NULL. */
    if(fp) fclose(fp);
    return -2;
}


unsigned char * file_OfSize_readFromOffset(const char *fileName, int imagesize,int offset){
	unsigned char  *buffer=(unsigned char *) malloc(imagesize);
	return file_OfSize_readFromOffset_into(fileName,imagesize,offset,buffer);
}
#define PGMHDR(width,height,header) sprintf(header,"P5\n%d %d\n255\n",width,height)


void savefile_OfSize_asPGM(unsigned char* image, short width, short height,
		const char *fileName) {
	char header[32];
	int imagesize=width*height;
	int fd, hsize = PGMHDR(width,height,header);
	if (hsize <= 8) {
				printf("error creating header from image file %s\n", fileName);
				return;
	}
	fd = creat(fileName, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd <= 2) {
		printf("error creating file %s\n", fileName);
		return;
	}
	if (hsize != write(fd, (void *) header, hsize)) {
		printf("error writing image header for %s\n", fileName);
	}
	lseek(fd, 0,SEEK_END);
	if (imagesize != write(fd, (void *) image, imagesize)) {
		printf("error writing file %s\n", fileName);
	}
	close (fd);
}

#define PGMHDR16(width,height,header) sprintf(header,"P5\n%d %d\n65535\n",width,height)

void savefile_OfSize_asPGM16(unsigned char* image, short width, short height,
		const char *fileName) {
	char header[32];
	int imagesize=width*height*2;
	int fd, hsize = PGMHDR16(width,height,header);
	if (hsize <= 8) {
				printf("error creating header from image file %s\n", fileName);
				return;
	}
	fd = creat(fileName, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if (fd <= 2) {
		printf("error creating file %s\n", fileName);
		return;
	}
	if (hsize != write(fd, (void *) header, hsize)) {
		printf("error writing image header for %s\n", fileName);
	}
	lseek(fd, 0,SEEK_END);
	if (imagesize != write(fd, (void *) image, imagesize)) {
		printf("error writing file %s\n", fileName);
	}
	close (fd);
}


void savefile_OfSize_as(unsigned char* image, int imagesize, const char *fileName){
	int fd = creat(fileName, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
	if(fd<=2) {
		printf("error creating file %s\n",fileName);
		return;
	}
	if (imagesize!=write(fd, (void *)image, imagesize ))
	{
		printf("error writing file %s\n",fileName);
	}
	close (fd);
}

void savefile_OfSize_asPGM_index(unsigned char* image, short width, short height, const char* prefix, int index){
	char fName[100];
	sprintf(fName,"%s_%03d.pgm",prefix,index);
	savefile_OfSize_asPGM(image,width,height,fName);
}

void savefile_OfSize_asPGM16_index(unsigned char* image, short width, short height, const char* prefix, int index){
	char fName[100];
	sprintf(fName,"%s_%03d.pgm",prefix,index);
	printf(" SAving @ %s\n",fName);
	savefile_OfSize_asPGM16(image,width,height,fName);
}

uint64_t tvdelta(struct timeval *t1, struct timeval *t2){
	uint64_t delta = (t1->tv_sec-t2->tv_sec);
	delta = delta*1000000;
	delta += (t1->tv_usec-t2->tv_usec);
	return delta;
}
//
//__int64 FileSize64( const char * szFileName )
//{
//  struct __stat64 fileStat;
//  int err = _stat64( szFileName, &fileStat );
//  if (0 != err) return 0;
//  return fileStat.st_size;
//}

int FileSize( const char * szFileName )
{
  struct stat fileStat;
  int err = stat( szFileName, &fileStat );
  if (0 != err) return 0;
  return fileStat.st_size;
}
