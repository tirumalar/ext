/*
 * ftp_fs.c
 *
 *  Created on: Dec 7, 2018
 *      Author: PTG
 */




//ik
#include "ff.h"
FIL Fil;			/* File object needed for each open file */


int f_getdir(char *txt)
    {
        FRESULT res;
        DIR dir;
        UINT i;
        static FILINFO fno;
        char temp[512];

        strcat(txt,"Opening \r\n");
        res = f_opendir(&dir, "/");                       /* Open the directory */
        if (res == FR_OK) {
            for (;;) {
                res = f_readdir(&dir, &fno);                   /* Read a directory item */
                if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
                if (fno.fattrib & AM_DIR) {                    /* It is a directory */

                } else {                                       /* It is a file. */
                    sprintf(temp,"%s%s %d\r\n", "/", fno.fname,(int )fno.fsize);
                    strcat(txt,temp);
                }
            }
            f_closedir(&dir);
        }

        return strlen(txt);

    }
void * f_vfopen(char *fname, char *mode)
    {
    FIL *f;
    BYTE md;
//	printf("Try File open %s %s\n",fname,mode);

    f=malloc(sizeof(FIL));
    if (f==0)
	return 0;
    if (strchr(mode,'r'))
	  md = FA_READ;
    if (strchr(mode,'w'))
	  md = FA_WRITE | FA_CREATE_ALWAYS;

    if (f_open (f, fname,  md)==FR_OK)
	{
//	printf("File open %s %x\n",fname,f);
	return (void *)f;
	}
    free(f);
    return 0;

    }
void  f_vfclose(void *fp)
    {
    f_close(fp);
    free(fp);
    }
int rv=0;
int f_vgetc( void * vfd)
    {
    unsigned char ch;
    UINT bw;

  //  printf("Getch file= %x \n",vfd);
   // if (rv++<10)
//	return rv+'a';
 //   return -1;
    if (( f_read ((FIL *)vfd, &ch, 1, &bw)==FR_OK) && (bw==1))
    	{
	return ch;
    	}
	//printf("Getch ret -1\n");
    return -1;
    }
int f_vfwrite(char * buf, unsigned size, unsigned items, void *fp)
{
    UINT bw;
//	printf("Try File write %x %d\n",fp,size);
    if ( f_write (fp, buf, size*items, &bw)==FR_OK)
	return bw;
    return 0;
}
int f_vunlink (char * fname)
    {
     if ( f_unlink (fname)==FR_OK)
	 return 0;
     return -1;
    }

int f_vfread(char * buf, unsigned size, unsigned items, void *fp)
{
    UINT bw;
//	printf("Try File read %x %d\n",fp,size);
if ( f_read (fp, buf, size*items, &bw)==FR_OK)
	{
//	printf("Read %d\n",bw);
	return bw;
	}
return 0;
}
