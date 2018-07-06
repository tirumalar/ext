// MAIN HEADER FILE FOR CProfiler CLASS
//
// A small profiling class using the Win32 API
//
// v.1.0
// Written by Hernan Di Pietro (May 2003)
// mailto:hernandp@speedy.com.ar
///////////////////////////////////////////////////
// See Profiler.cpp for implementation
//////////////////////////////////////////////////

#ifndef __PROFILER_H__
#define __PROFILER_H__


#ifdef __linux

#include <apr_time.h>
#define __int64 apr_time_t
class CProfiler
{
public:
	
	CProfiler(void){}
	~CProfiler(void){}

	void ProfileStart(){
		m_StartCounter = apr_time_now();
	}
	
	apr_time_t	ProfileEnd  (){
		m_EndCounter = apr_time_now();
		m_ElapsedTime=(m_EndCounter-m_StartCounter);
		return m_ElapsedTime;
	}

	double SecsFromTicks(apr_time_t inp) {	return apr_time_sec((double)ticks); }
	double GetElapsedTime() { return SecsFromTicks(m_ElapsedTime);}

private:
	apr_time_t	m_StartCounter,m_EndCounter,m_ElapsedTime;		// ticks/sec resolution

};
#else 
#include <windows.h>
// enumeration for Debug Logging Output Types (or "styles")
enum LOGTYPE { LOGNONE, LOGTICKS, LOGSECS, LOGALL, LOGMSGBOX };		

class CProfiler
{
public:
	
	CProfiler(void){
		// constructor zeroes all LARGE_INTEGER data
		// members and gets the correct counter frequency
		// for the system.
		ZeroMemory(&m_QPFrequency, sizeof(m_QPFrequency));
		ZeroMemory(&m_ElapsedTime, sizeof(m_ElapsedTime));
		ZeroMemory(&m_StartCounter,sizeof(m_StartCounter));
		m_Retval = 0;
		
		m_Retval = QueryPerformanceFrequency(&m_QPFrequency);	// Query Frecuency
	}

	~CProfiler(void){}

	void ProfileStart(){
		m_Retval = QueryPerformanceCounter (&m_StartCounter);
	}
	
	__int64 ProfileEnd(){
		m_Retval = QueryPerformanceCounter (&m_EndCounter);
		m_ElapsedTime = (m_EndCounter.QuadPart  - m_StartCounter.QuadPart );
		return m_ElapsedTime;
	}

	double SecsFromTicks ( __int64 ticks){
		return static_cast<double>(ticks) / static_cast<double>(m_QPFrequency.QuadPart);
	}

	DWORD		GetLastRetVal(void);	// returns the last error, if any.
	double		GetElapsedTime() {return SecsFromTicks(m_ElapsedTime); }
		
private:
	
	LARGE_INTEGER	m_QPFrequency;		// ticks/sec resolution
	LARGE_INTEGER	m_StartCounter;		// start time
	LARGE_INTEGER	m_EndCounter;		// finish time
	__int64			m_ElapsedTime;		// elapsed time
	DWORD			m_Retval;			// return value for API functions
};
#endif


#endif // __PROFILER_H__ 
