/*
 * EnrollmentProcessor.h
 *
 *  Created on: Feb 15, 2012
 *      Author: dhirvonen
 */

#ifndef ENROLLMENTPROCESSOR_H_
#define ENROLLMENTPROCESSOR_H_

#include "GenericProcessor.h"

class EnrollmentServer;
class MatchProcessor;
class HTTPPOSTMsg;


class Timer
{
public:
	Timer() {}
	static uint64_t GetTimeInMilliseconds();
};

class EnrollmentProcessor: public GenericProcessor {
public:
	EnrollmentProcessor(Configuration& conf);
	virtual ~EnrollmentProcessor();

	void Enrollment(unsigned char *pImage);

	virtual const char *getName();
	virtual void postProcess();
	virtual void process(Copyable *msg);
	virtual Copyable *createNewQueueItem();
	void SetScaleRatio(float ratio) { m_ScaleRatio = ratio; }
	void Start(uint64_t time);


protected:

	unsigned char * GetMessageAsImage(HTTPPOSTMsg *pMsg);
	Safe<EnrollmentServer *> m_pEnrollmentServer;
	MatchProcessor *m_pMatchProcessor; /* Use this for convenient allocation of matcher */
	float m_ScaleRatio;
};

#endif /* ENROLLMENTPROCESSOR_H_ */
