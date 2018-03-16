/*
 * LiquidLens.cpp
 *
 *  Created on: Mar 30, 2012
 *      Author: dhirvonen
 */

#include "LiquidLens.h"
#include "Synchronization.h"
#include "Parsing.h"
#include <iostream>

#define LENS_WRITE_ADDR_8BIT    0xec
#define LENS_WRITE_ADDR_7BIT    (LENS_WRITE_ADDR_8BIT >> 1)
#define LENS_POWER_MODE         0x00
#define LENS_VOLTAGE_OUT        0x01

LiquidLens::LiquidLens(Configuration& conf) : m_Focus(0), m_Power(0), m_pSem(new Semaphore(0))
{
	const char *data = conf.getValue("MAX14515A.Steps", "0");

	if(data != NULL)
	{
		std::string comma = ",";
		std::string input(data);
		std::vector<std::string> steps = tokenize(input, comma);
		for(int i = 0; i < steps.size(); i++)
		{
			int step = atoi(steps[i].c_str());
			m_Steps.push_back(step);
			printf("step: %d\n", step); fflush(stdout);
		}
	}

	SetPower(true); // turn on the lens
}

LiquidLens::LiquidLens()
{

}

LiquidLens::~LiquidLens()
{
	if(m_pSem)
	{
		delete m_pSem;
	}
}

bool LiquidLens::Write(int reg, int value)
{
	int status = 0;
	ScopeLock lock(I2CBusNanoAPI::instance().GetLock());
	status = I2CBusNanoAPI::instance().Assign(LENS_WRITE_ADDR_7BIT);
	if(0 > status)
	{
		fprintf(stderr, "Failed to assign liquid lens address on I2C bus\n"); fflush(stdout);
		return false;
	}

	status = I2CBusNanoAPI::instance().Write( reg, value );
	if(status)
	{
		fprintf(stderr, "Failed to write register %d = %d\n", reg, value); fflush(stdout);
		return false;
	}

	return true;
}

bool LiquidLens::SetPower(bool state)
{
	bool status = false;
	int value = state ? 1 : 0;
	if(Write(LENS_POWER_MODE, value))
	{
		m_Power = value;
		status = true;
	}
	return status;
}

bool LiquidLens::SetFocus(int focus)
{
	bool status = false;
	if(Write(LENS_VOLTAGE_OUT, focus))
	{
		m_Focus = focus;
		status = true;
	}
	return status;
}

void LiquidLens::Sweep()
{
	for(int i = 0; i < m_Steps.size(); i++)
	{
		m_pSem->Wait(); // Always wait for next frame to change focus
		printf("SetFocus(%d)?\n", m_Steps[i]); fflush(stdout);
		SetFocus( m_Steps[i] );
	}
}

void LiquidLens::Step()
{
	m_pSem->Post();
}

unsigned int LiquidLens::MainLoop()
{
	std::string name = "LiquidLens::";
	try {
		while (!ShouldIQuit())
		{
			Sweep();
			Frequency();
		}
	} catch (std::exception& ex) {
		std::cerr << name << ex.what() << std::endl;
		std::cerr << name << "exiting thread" << std::endl;
	} catch (const char *msg) {
		std::cerr << name << msg << std::endl;
		std::cerr << name << "exiting thread" << std::endl;
	} catch (...) {
		std::cerr << name << "Unknown exception! exiting thread" << std::endl;
	}

	return 0;
}

int LiquidLens::Handle(IplImage *frame)							// ImageHandler
{
	// slave process sticks eye into enrollment queue
	Step();
}


