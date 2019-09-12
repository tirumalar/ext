

#include "stdio.h"
#include "stdlib.h"
#include "i2c_opencores.h"
#include "system.h"
#include "io.h"


//#include "touch.h"
//#include "WaveLib.h"
#include <sys/stat.h>
#include <fcntl.h>
#include "sound.h"

#include "includes.h"

#define DAC_ADDRESS 0x60
#define DAC_CONFIG   0x40 // command 010 vref 1 0 vrev 0 0 pd1 0 pd 0 0 gain is 0



#define BUF_SIZE 512
#define SOUND_BASE SOUND_0_BASE
void snd_enable(char en)
{
    int x;
	if (en)
		IOWR_32DIRECT(SOUND_BASE,0,0x40000000);
	else
		{
		IOWR_32DIRECT(SOUND_0_BASE,0,0xc0000000);
		while ((IORD_32DIRECT(SOUND_0_BASE,0)&0xffff0000)!=0)
		{
		//printf("state = %d",(IORD_32DIRECT(SOUND_0_BASE,0)>>16)&0xf);
		}
		while ((IORD_32DIRECT(SOUND_0_BASE,0)&0xffff0000)!=0);
		}


}


void snd_init(void)
    {
    int x;
    return;
	for (x=0;x <512;x++)
	      {
	      IOWR_32DIRECT(SOUND_0_BASE,0,(x*4)&0xffff);
	      OSTimeDly(1);
	      }
	for (x=0;x <512;x++)
		      {
		      IOWR_32DIRECT(SOUND_0_BASE,0,2048);
		      //OSTimeDly(1);
		      }

    }
static int vv=0;
int aud_play_array_low( short *aud_buff,int samples)
{
	int f = BUF_SIZE;
	int v;
	 while (samples)
	  {

	      while ( IORD_32DIRECT(SOUND_0_BASE,0)>(BUF_SIZE-10))
	        {

		  OSTimeDly(4);
	        }
	      v= (*aud_buff);
	      v=v/16 +2048;
	     // v= ((v&0xff)<<8) | ((v>>8)&0xff);
	    //  v=vv++;
	      if (vv>4095)
	    	  vv=0;
	      IOWR_32DIRECT(SOUND_0_BASE,0,v&0xffff);
	      aud_buff++;
	      samples--;
	  }
}


#define PREBUFF_SIZE 1024
short aud_pre_buff[PREBUFF_SIZE];
short prebuff=0;
int aud_play_array( short *aud_buff,int samples)
    {
    if ((prebuff==0) && samples<PREBUFF_SIZE && (IORD_32DIRECT(SOUND_0_BASE,0)==0))
	{
	memcpy(aud_pre_buff,aud_buff,samples*2);
	prebuff=samples;
	return;
	}
    else
	{
	if (prebuff)
	    aud_play_array_low(aud_pre_buff,prebuff);
	aud_play_array_low(aud_buff,samples);
	prebuff=0;
	}

    }

int x;
void dac_test(void)
{
  short v;
  int dat;
  unsigned int f;
  WAV_PLAY wp;

 // dac_configure();
  snd_enable(1);
//  snd_load_file("/mnt/fat_fs/tadasm.wav", &wp);
//  cur_play = &wp;
//  snd_play(&wp);
  while (0)
  	  {
	  snd_service();
	   if (wp.done)
		   snd_play(&wp);
  	  }

  while (1)
  {
	  int vlu[]={128,219,256,219,128,37,0,37};
	//  dac_write(247*sin(v*2*3.14/4096)+2047);
	  dat = v;// 247*sin(v*2*3.14/4096)+2047;
	  f = IORD_32DIRECT(SOUND_0_BASE,0);

      x=x+256;
     dat= vlu[(x>>8)&0x7]*2;
	// printf(" f= %xasdfasdf\n",f);
      while ( f>200)
        {
    	  f = IORD_32DIRECT(SOUND_0_BASE,0);
        }

      IOWR_32DIRECT(SOUND_0_BASE,0,dat&0xffff);

	  v = v+ 10;
	  v = v % 4096;
  }
}

#if 0
#include "ff.h"


snd_load_file(char *file, WAV_PLAY *wp)
{
	int resultF;
	FIL inputFd;
	char buf[BUF_SIZE];
  int numRead;
  int len;
  int offset;
  char *w_ptr;
  uint bread;
  if (strstr(file,"/mnt/fat_fs/"))
	  file = &file[strlen("/mnt/fat_fs/")];
  resultF = f_open(&inputFd, file, FA_READ);
  wp->len =0;
  if (resultF!=FR_OK)
  		{
  		printf("error cant open %s\n",file);
  		return 0;
  		}
  printf("opening sound\n");

  f_read(&inputFd, buf, BUF_SIZE,&bread);

  offset = Wave_GetWaveOffset( buf,BUF_SIZE);
  len =   Wave_GetDataByteSize( buf,BUF_SIZE);

  wp->dat = malloc(len*2+100);
  w_ptr =wp->dat;
  f_lseek(&inputFd,offset);

  while ((f_read(&inputFd, w_ptr, BUF_SIZE,&bread)) ==FR_OK)
      {
	  if (bread==0)
		  break;
	  w_ptr+=bread;
 	  printf(".");
      }
  wp->len =len/2;  // div 2 because of short samples
  wp->ptr = 0;
  wp->done = 0;
  f_close(&inputFd);
}

dac_configure(void)
{
	  I2C_init(I2C_OPENCORES_SOUND_BASE,ALT_CPU_FREQ,400000);
	 I2C_start(I2C_OPENCORES_SOUND_BASE,DAC_ADDRESS,0);
	 I2C_write(I2C_OPENCORES_SOUND_BASE,DAC_CONFIG,1);
}

dac_write(short val)
{
	unsigned char v;
	 I2C_start(I2C_OPENCORES_SOUND_BASE,DAC_ADDRESS,0);
	 v = (val>>8) & 0xf;
	 I2C_write(I2C_OPENCORES_SOUND_BASE,v,0);
	 v = val&0xff;
	 I2C_write(I2C_OPENCORES_SOUND_BASE,v,1);
}
*/

#define AUD_FIFO_MAX 500
WAV_PLAY *cur_play=0;
short *lpt,v;
char  snd_service(void)
{
	int x;
	int bytes_to_write;

	int fifo_fill;

	if (cur_play==0)
		return 0;
	if(cur_play->done)
		return 0;
	fifo_fill = IORD_32DIRECT(SOUND_0_BASE,0)&0xffff;
	bytes_to_write = AUD_FIFO_MAX -fifo_fill;
	if (bytes_to_write<100)
		return 1;

	if (cur_play->len-cur_play->ptr < bytes_to_write)
		bytes_to_write = cur_play->len-cur_play->ptr;

	lpt=&cur_play->dat[cur_play->ptr];
	for(x =0; x < bytes_to_write; x++)
	{
		v = (*lpt++)/32+2048;
		//  v = v+ 500;
		//  v = v % 4096;
		IOWR_32DIRECT(SOUND_0_BASE,0,(v&0xffff));
	}
	//printf("%d %d %d\n",cur_play->ptr,cur_play->len,bytes_to_write);
	cur_play->ptr+=bytes_to_write;
	if (cur_play->ptr==cur_play->len)
	{
		cur_play->done =1;
	}
	return 1;
}


char snd_play( WAV_PLAY *wp)
{
	if (cur_play)
		if(!cur_play->done)
			return -1;
	wp->ptr =0;
	wp->done =0;
	cur_play=wp;
	return 0;
}


#endif
