/*
 * LiquidLens.h
 *
 *  Created on: Mar 30, 2012
 *      Author: dhirvonen
 */

#ifndef LIQUIDLENS_H_
#define LIQUIDLENS_H_

#include "ImageProcessor.h"
#include "I2CBus.h"
#include "HThread.h"

class Semaphore;

class LiquidLens: public ImageHandler, public HThread {
public:
	LiquidLens(Configuration& conf);
	LiquidLens();
	virtual ~LiquidLens();
	bool Write(int reg, int value);
	bool SetPower(bool state);
	bool SetFocus(int focus);
	void Sweep();
	void Step();

	virtual int Handle(IplImage *frame);							// ImageHandler
	unsigned int MainLoop(); 			  					// HThread
	const char *getName() { return "LiquidLens"; }          // HTHread

protected:
	int m_Focus;
	int m_Power;
	Semaphore *m_pSem;
	std::vector<int> m_Steps;
};

#endif /* LIQUIDLENS_H_ */
