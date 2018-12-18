/*
 * AudioDispatcher.cpp
 *
 *  Created on: Dec 20, 2011
 *      Author: dhirvonen
 */

#include "AudioDispatcher.h"
#include "SpeakServer.h"
#include "TonePlayer.h"
#include "UtilityFunctions.h"
#include "logging.h"
#include <string.h>

const char logger[30] = "AudioDispatcher";

void SetAlsaMasterVolume(float volume)
{
	int vol = std::min(100, std::max(0,(int)volume));
	vol = vol > 0 ? ((int)(55 + 45.0*vol/100)):0;
#ifdef __ARM__
	EyelockLog(logger, DEBUG, "Audio Level %d",vol);
    char cmd[1024]={0};
    snprintf(cmd,1023,"amixer set 'DAC1 Digital Coarse' %d%%",vol);
    RunCmd(cmd);
    snprintf(cmd,1023,"amixer set 'DAC1 Digital Fine' %d%%",vol);
    RunCmd(cmd);
    snprintf(cmd,1023,"amixer set 'DAC2 Digital Coarse' %d%%",vol);
    RunCmd(cmd);
    snprintf(cmd,1023,"amixer set 'DAC2 Digital Fine' %d%%",vol);
    RunCmd(cmd);
    snprintf(cmd,1023,"amixer set 'DAC2 Analog' %d%%",vol);
    RunCmd(cmd);
    snprintf(cmd,1023,"amixer set 'DAC1 Analog' %d%%",vol);
    RunCmd(cmd);
#endif

}
AudioDispatcher::AudioDispatcher(Configuration &conf) : ResultDispatcher(conf)
{
	m_pSpeaker = new SpeakServer;

	m_volume = conf.getValue("GRI.AuthorizationToneVolume", 10.0f); /* Volume in % can be 0 to 100 */
	SetAlsaMasterVolume(m_volume);
	m_volumeTamper = conf.getValue("GRI.TamperToneVolume", 50.0f);
	m_seconds = conf.getValue("GRI.AuthorizationToneDurationSeconds", 1); /* DJH: NEW */
	m_successFrequency = conf.getValue("GRI.AuthorizationToneFrequency", 900); /* DJH: NEW */
	m_failFrequency = conf.getValue("GRI.AuthorizationToneFrequencyForFail", 500);
	m_cardFrequency = conf.getValue("GRI.AuthorizationToneFrequencyForCard", 500);
	m_authtoneFile = conf.getValue("Eyelock.AuthorizationToneFile", "/home/root/tones/auth.wav");
	m_pCmxHandler = NULL;
	m_EnableAudioMatch = conf.getValue("Eyelock.EnableAudioMatch", false);
}



int AudioDispatcher::Begin()
{
	EyelockLog(logger, DEBUG, "AudioDispatcher::Begin() => TonePlayer::Begin()");

	HThread::Begin();
}

int AudioDispatcher::End()
{
	EyelockLog(logger, DEBUG, "AudioDispatcher::End() => TonePlayer::End()");

	HThread::End();
}

AudioDispatcher::~AudioDispatcher()
{
	if(m_pSpeaker)delete m_pSpeaker;
}

void AudioDispatcher::process(MatchResult *msg)
{
	EyelockLog(logger, DEBUG, "AudioDispatcher match state %d ***", msg->getState()); fflush(stdout);

	if (msg->getState()==DUAL_AUTHN_CARD) return;
#if defined(HBOX_PG) || defined(CMX_C1)
// #ifdef HBOX_PG
	if (msg->getState()==FAILED || msg->getState()==CONFUSION)
		{

		}
		else if (msg->getState()==TAMPER)
		{
			EyelockLog(logger, DEBUG, "tamper state");
			if (m_volumeTamper)
			{
				//SetAlsaMasterVolume(m_volumeTamper);

				// RunSystemCmd_Audio("killall -KILL aplay; aplay /home/root/tones/tamper1.wav ");

				// sleep(2);
				//SetAlsaMasterVolume(m_volume);
			}
		}
		else if (msg->getState()==PASSED)
		{
			if(m_EnableAudioMatch){
				char buf[100];
				string name;
				name.assign(msg->getName());
				//m_pCmxHandler->CameraStatus(false);
				char *pch = strtok ((char*)name.c_str(),"|");
				sprintf(buf,"espeak \"welcome %s\" ",pch);
				// printf("name.cstr()........%s \n", name.c_str());
				// sprintf(buf,"espeak \"welcome %s\" &",name.c_str());
				// printf("Buf..%s", buf);
				EyelockLog(logger, DEBUG, "AudioDispather sound for match %s", buf); fflush(stdout);
				RunSystemCmd_Audio(buf);
				//m_pCmxHandler->CameraStatus(true);
			}
		}
	//return;
#endif
#ifndef CMX_C1
	if (msg->getState()==FAILED || msg->getState()==CONFUSION) {
		if (m_volume)
			RunSystemCmd_Audio("killall -KILL aplay;aplay /home/root/tones/rej.wav ");
	}
	else if (msg->getState()==TAMPER) {
		EyelockLog(logger, DEBUG, "tamper state");
		if (m_volumeTamper)
		{
			SetAlsaMasterVolume(m_volumeTamper);
			RunSystemCmd_Audio("killall -KILL aplay; aplay /home/root/tones/tamper1.wav ");
			// sleep(2);
			SetAlsaMasterVolume(m_volume);
		}
	}
	else if (msg->getState()==PASSED) {
		if (m_volume)
			RunSystemCmd_Audio("killall -KILL aplay;aplay /home/root/tones/auth.wav  ");
	}
	//RunSystemCmd("aplay /home/root/tones/auth.wav");
#else
#ifndef HBOX_PG
	//SetAlsaMasterVolume(0);	// disable NXT local sound
#endif	
	//printf("AudioDispathcer ******* sending CMX command\n");
	char buff[100];
	buff[0] = CMX_SOUND_CMD;	// 1-PASS 2-FAIL 3-TAMPER
	buff[1] = 2;	// message length

	if (msg->getState()==FAILED || msg->getState()==CONFUSION) {
		if (m_volume) {
			buff[2] = 2;
			buff[3] = m_volume;	// volume

		}
	}
	else if (msg->getState()==TAMPER) {
		EyelockLog(logger, DEBUG, "tamper state");
		if (m_volumeTamper)
		{
			buff[2] = 3;
			buff[3] = m_volumeTamper;	// volume

		}
	}
	else if (msg->getState()==PASSED) {
		if (m_volume) {
			buff[2] = 1;
			buff[3] = m_volume;	// volume
		}
	}

	if (m_pCmxHandler)
	{
		printf("HandleSendMsg message\n");

		if(msg->getState() == PASSED){
			m_pCmxHandler->m_Randomseed = m_pCmxHandler->GenerateSeed();
			// printf("Random seed for Pass...%d\n", m_pCmxHandler->m_Randomseed);
		}
		if (msg->getState()==FAILED || msg->getState()==CONFUSION) {
			m_pCmxHandler->m_Randomseed = m_pCmxHandler->GenerateSeed();
			// printf("Random seed for fail....%d\n", m_pCmxHandler->m_Randomseed);
		}
		m_pCmxHandler->HandleSendMsg(buff, m_pCmxHandler->m_Randomseed);
	}
#endif
}

int32_t AudioDispatcher::SetAudiolevel(float VolumeLevel)
{
	int32_t ret =1;
	VolumeLevel = std::min(100.0f, std::max(0.0f,VolumeLevel));
	SetAlsaMasterVolume(VolumeLevel);
	m_volume = VolumeLevel;
	bool fexist = FileExists(m_authtoneFile);
	if(fexist){
#ifndef HBOX_PG
		string cmd("aplay ");
		cmd.append(m_authtoneFile);
		RunSystemCmd(cmd.c_str());
#endif
		ret =0;
	}
	return ret;
}

double AudioDispatcher::GetAudiolevel(void)
{
	return (double)m_volume;
}

void AudioDispatcher::SetAudioDispatcher(AudioDispatcher *ptr)
{
	ptr = m_pAudioDispatcher;
}

