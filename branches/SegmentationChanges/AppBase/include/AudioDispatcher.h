/*
 * AudioDispatcher.h
 *
 *  Created on: Dec 20, 2011
 *      Author: dhirvonen
 */

#ifndef AUDIODISPATCHER_H_
#define AUDIODISPATCHER_H_

#include "ResultDispatcher.h"
#include "CmxHandler.h"

class SpeakServer;
class TonePlayer;

class AudioDispatcher: public ResultDispatcher {
public:

	AudioDispatcher(Configuration &conf);
	virtual ~AudioDispatcher();
	const char *getName() { return "AudioDispatcher"; } // from HThread

	virtual int Begin();
	virtual int End();

	virtual void process(MatchResult *msg); // from ResultDispatcher
	virtual int32_t SetAudiolevel(float VolumeLevel);
	virtual double GetAudiolevel(void);
	virtual void SetAudioDispatcher(AudioDispatcher *ptr);
	SpeakServer * GetSpeakServer() { return m_pSpeaker; }
	TonePlayer * GetTonePlayer() { return m_pTonePlayer; }
	void SetCmxHandler(CmxHandler *pCmxHandler) {m_pCmxHandler = pCmxHandler;}

protected:

	SpeakServer *m_pSpeaker;
	TonePlayer *m_pTonePlayer;
	float m_volume;
	float m_volumeTamper;
	int m_seconds;
	int m_successFrequency;
	int m_failFrequency;
	int m_cardFrequency;
	bool m_EnableAudioMatch;
	AudioDispatcher *m_pAudioDispatcher;
	const char *m_authtoneFile;
	CmxHandler *m_pCmxHandler;
};

#endif /* AUDIODISPATCHER_H_ */
