/*
 * SpeakServer.cpp
 *
 *  Created on: Dec 20, 2011
 *      Author: dhirvonen
 */



#include "SpeakServer.h"
#include <stdlib.h>
#include <string.h>

#if HAS_ESPEAK
#include <espeak/speak_lib.h>
#endif

SpeakServer::SpeakServer()
{
#if HAS_ESPEAK
#error
	espeak_Initialize(AUDIO_OUTPUT_SYNCH_PLAYBACK, 30000, NULL, 0);
	espeak_SetParameter(espeakRATE, 170, 0);   // words per minute [80..370] dflt=170
	espeak_SetParameter(espeakPITCH, 50, 0);   // pitch adjustment [0..99]
	espeak_SetParameter(espeakWORDGAP, 10, 0);  // world gap in ms between words
	espeak_SetVoiceByName("en+f1");
#endif
}

SpeakServer::~SpeakServer()
{
#if HAS_ESPEAK
	espeak_Terminate();
#endif
}

void SpeakServer::speak(char* word)
{
#if HAS_ESPEAK
	espeak_Synth((char*)word, strlen(word)+1, 0, POS_CHARACTER, 0, espeakCHARS_AUTO, NULL, NULL);
#endif
}

void SpeakServer::setRate(const int value)
{
#if HAS_ESPEAK
	espeak_SetParameter(espeakRATE, value, 0);
#endif
}

void SpeakServer::setPitch(const int value)
{
#if HAS_ESPEAK
	espeak_SetParameter(espeakPITCH, value, 0);
#endif
}

void SpeakServer::setRange(const int value)
{
#if HAS_ESPEAK
	espeak_SetParameter(espeakRANGE, value, 0);
#endif
}

void SpeakServer::setLanguage(const char* language)
{
#if HAS_ESPEAK
	espeak_SetVoiceByName(language);
#endif
}

