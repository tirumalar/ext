/*
 *  This small demo sends a simple sinusoidal wave to your speakers.
 */

#include "TonePlayer.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sched.h>
#include <errno.h>
#include <math.h>
#include <string>
#include <algorithm>

void TonePlayer::SetVolume(float volume)
{
}

int TonePlayer::Generate(snd_pcm_uframes_t offset, int count, double *_phase)
{
 return 0;
}

/*
 *   Underrun and suspend recovery
 */
 
int TonePlayer::XRunRecovery(snd_pcm_t *handle, int err)
{

    return 0;

}

int TonePlayer::SetHWParams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access)
{

  return 0;
}

int TonePlayer::SetSWParams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams)
{
    return 0;
}

TonePlayer::TonePlayer()
{

}

TonePlayer::~TonePlayer()
{
}

int TonePlayer::Init()
{

}

const char* TonePlayer::getName() { return "TonePlayer"; }

int TonePlayer::End()
{
return 0;
}

int TonePlayer::Play()
{
	return 0;
}

unsigned int TonePlayer::MainLoop()
{

  return 0;
}

int TonePlayer::PlaySound(int seconds)
{

}

int TonePlayer::Term()
{

}

#if 0
int main(int argc, char **argv)
{
  TonePlayer player;
  player.SetFrequency(200);
  player.SetDevice("default");
  player.Init();
  player.MainLoop();
  player.Term();
}
#endif
