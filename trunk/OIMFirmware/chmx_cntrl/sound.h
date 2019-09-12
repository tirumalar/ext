/*
 * sound.h
 *
 *  Created on: Apr 15, 2014
 *      Author: PTG
 */

#ifndef SOUND_H_
#define SOUND_H_


typedef struct
{
	short *dat;
	int   len;
	int   ptr;
	int   start;
	int   done;
}WAV_PLAY;
char snd_service(void);
snd_load_file(char *file, WAV_PLAY *wp);
char snd_play( WAV_PLAY *wp);
void snd_enable(char en);


#endif /* SOUND_H_ */
