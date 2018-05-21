/*
 * TonePlayer.h
 *
 *  Created on: Jan 31, 2012
 *      Author: dhirvonen
 */

#ifndef TONEPLAYER_H_
#define TONEPLAYER_H_

// #include <alsa/asoundlib.h>
#include <string>
#include "HThread.h"
#include "Synchronization.h"

class TonePlayer : public HThread
{
public:

  TonePlayer();
  ~TonePlayer();

  int End();

  const char* getName();
  unsigned int MainLoop();

  int Init();
  int Term();
  //int Generate(snd_pcm_uframes_t offset, int count, double *_phase);
  int Play();
  void SetVolume(float volume);

  void SetSeconds(int seconds)
  {
    m_Seconds = seconds;
  }

  void SetDevice(const char *arg)
  {
   // device = strdup(arg);
  }

  void SetBufferTime(int time)
  {
    buffer_time = buffer_time < 1000 ? 1000 : buffer_time;
    buffer_time = buffer_time > 1000000 ? 1000000 : buffer_time;
  }
  void SetPeriod(int period)
  {
    period_time = period_time < 1000 ? 1000 : period_time;
    period_time = period_time > 1000000 ? 1000000 : period_time;
  }

  void SetFormat(const char *arg)
  {
#if 0
    for (int i = 0; i < SND_PCM_FORMAT_LAST; i++)
    {
      format = (snd_pcm_format_t)i;
      const char *format_name = (const char *)snd_pcm_format_name(format);
      if (format_name)
	if (!strcasecmp(format_name, arg))
	  break;
    }
    if (format == SND_PCM_FORMAT_LAST)
      format = SND_PCM_FORMAT_S16;
    if (!snd_pcm_format_linear(format) && !(format == SND_PCM_FORMAT_FLOAT_LE || format == SND_PCM_FORMAT_FLOAT_BE))
    {
      printf("Invalid (non-linear/float) format %s\n", optarg);
    }
#endif
  }

  void SetChannels(int channels)
  {
    channels = channels < 1 ? 1 : channels;
    channels = channels > 1024 ? 1024 : channels;
  }
  void SetRate(int rate)
  {
    rate = rate < 4000 ? 4000 : rate;
    rate = rate > 196000 ? 196000 : rate;
  }
  void SetFrequency(double freq)
  {
    m_freq = freq < 50 ? 50 : freq;
    m_freq = freq > 5000 ? 5000 : freq;
  }

protected:

  int PlaySound(int seconds);
 // int XRunRecovery(snd_pcm_t *handle, int err);
 // int SetHWParams(snd_pcm_t *handle, snd_pcm_hw_params_t *params, snd_pcm_access_t access);
  //int SetSWParams(snd_pcm_t *handle, snd_pcm_sw_params_t *swparams);

  Semaphore m_Sem;
  int m_Seconds;
  float m_Volume;
//  snd_pcm_channel_area_t *areas;
  signed short *samples;
//  snd_pcm_t *handle;
  int err;
 // snd_pcm_hw_params_t *hwparams;
 // snd_pcm_sw_params_t *swparams;
  unsigned int chn;
  std::string device;
 // snd_pcm_format_t format;
  double m_freq;
  unsigned int rate;
  unsigned int channels;
  unsigned int buffer_time;
  unsigned int period_time;
  int verbose;
  int resample;
  int period_event;
 //snd_pcm_sframes_t buffer_size;
  //snd_pcm_sframes_t period_size;
 // snd_output_t *output;
};

#endif /* TONEPLAYER_H_ */
