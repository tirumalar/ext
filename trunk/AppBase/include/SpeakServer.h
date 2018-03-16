/*
 * SpeakServer.h
 *
 *  Created on: Dec 20, 2011
 *      Author: dhirvonen
 */

#ifndef SPEAKSERVER_H_
#define SPEAKSERVER_H_

#include <malloc.h>
#include <string>

class SpeakServer
{
	private:
	std::string outputfile;

	public:
	SpeakServer();
	~SpeakServer();
	void speak(char* word);
	void setRate(const int value);
	void setPitch(const int value);
	void setRange(const int value);
	void setLanguage(const char* language);
};

#endif /* SPEAKSERVER_H_ */




