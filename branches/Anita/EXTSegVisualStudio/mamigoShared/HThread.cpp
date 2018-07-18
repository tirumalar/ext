#include "HThread.h"
#include "errno.h"
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>


bool HThread::m_FreqEnable=false;
HThread::HThread(void)
{
	m_timeelapsed=0;
	m_counter=0;
	m_isUnhealthy=false;
	m_hThread = 0;
	m_QuitStatus.set(false);
	m_timeOutuSec=200000; // 200 ms by default
	pthread_mutex_init(&data_avlbl_mutex,NULL);
	pthread_cond_init(&data_avlbl_cv,NULL);
}

HThread::~HThread(void)
{
	pthread_cond_destroy(&data_avlbl_cv);
	pthread_mutex_destroy(&data_avlbl_mutex);
}

bool HThread::ShouldIQuit()
{
	return m_QuitStatus.get();
}

void * __stdcall HThread::BeginProxy(void *arg)
{
	unsigned int status = 0;

	HThread *pHThread = (HThread *) arg;
	if(pHThread)
	{
		long int tid = syscall(SYS_gettid);
		printf("***************%s thread %u ********************* \n",pHThread->getName(),tid);
		status = pHThread->MainLoop();
	}

	return (void *)status;
}

int HThread::Begin()
{
	setHealthy();
	printf("***************Creating thread %s ********************* \n",getName());
	m_Error=pthread_create(&m_hThread, NULL, HThread::BeginProxy, (void *)(this));

	return m_Error;
}

int HThread::End()
{
	m_QuitStatus.lock();
	m_QuitStatus.set(true); // terminate current thread loop
	m_QuitStatus.unlock();

	// make sure we are not waiting for this condition
	pthread_cond_signal(&data_avlbl_cv);

	// wait for it to finish forever
	void *retVal;
	return pthread_join(m_hThread,&retVal);
}

// waits endlessly
bool HThread::waitForData() {
	bool bShouldWait=false;
	pthread_mutex_lock(&data_avlbl_mutex);
	bShouldWait=shouldWait();
	if(bShouldWait) pthread_cond_wait(&data_avlbl_cv,&data_avlbl_mutex);
	pthread_mutex_unlock(&data_avlbl_mutex);
	return bShouldWait;
}

// waits for pre-configured duration before waking up
bool HThread::waitForDataAndTimeOut() {
	setHealthy();
	bool bShouldWait=false;
	pthread_mutex_lock(&data_avlbl_mutex);
	bShouldWait=shouldWait();
	if(bShouldWait){
		gettimeofday(&m_timeuSec,0);
		m_timeuSec.tv_usec+=m_timeOutuSec;
		if(m_timeuSec.tv_usec>1000000){
			m_timeuSec.tv_sec+=1;
			m_timeuSec.tv_usec-=1000000;
		}
		m_timenSec.tv_sec=m_timeuSec.tv_sec;
		m_timenSec.tv_nsec=m_timeuSec.tv_usec*1000;
//		printf("%ld:z ",m_hThread);
		pthread_cond_timedwait(&data_avlbl_cv,&data_avlbl_mutex,&m_timenSec);
	}
	pthread_mutex_unlock(&data_avlbl_mutex);
	return bShouldWait;
}

/*
 * return false on timeout, true other wise does not check for condition
 */
bool HThread::waitForData(int secs ) {
	setHealthy(false,secs);
	pthread_mutex_lock(&data_avlbl_mutex);

	gettimeofday(&m_timeuSec,0);
	m_timenSec.tv_sec=m_timeuSec.tv_sec+secs;
	m_timenSec.tv_nsec=m_timeuSec.tv_usec*1000;

	int rc=pthread_cond_timedwait(&data_avlbl_cv,&data_avlbl_mutex,&m_timenSec);
	pthread_mutex_unlock(&data_avlbl_mutex);
	if(rc==ETIMEDOUT) return false;
	return true;
}

void HThread::dataAvailable(){
	pthread_mutex_lock(&data_avlbl_mutex);
	pthread_cond_signal(&data_avlbl_cv);
	pthread_mutex_unlock(&data_avlbl_mutex);
}

void HThread::setHealthy(bool bForceUnhealthy, int secsInFuture)
{
	if(bForceUnhealthy) {
		m_isUnhealthy=true;
		return;
	}
	gettimeofday(&m_timeuSec,0);
	uint64_t temp = (m_timeuSec.tv_sec+secsInFuture);
	temp = temp*1000000u;
	temp += m_timeuSec.tv_usec;
	m_lastSeenHealthy=temp/1000;

}
// called by another thread
bool HThread::IsHealthy(uint64_t curTimeMS){
	if(m_isUnhealthy) return false;
//	printf("%s :: %lu %lu %ld \n",getName(),curTimeMS,m_lastSeenHealthy,curTimeMS-m_lastSeenHealthy);
	if(m_lastSeenHealthy >curTimeMS) return true;
	if((curTimeMS-m_lastSeenHealthy)<10000) return true;
	return false;
}

void HThread::Frequency(){
	m_counter++;
	timeval t;
	gettimeofday(&t,0);
	uint64_t u = t.tv_sec; u = u*1000000;
	u += t.tv_usec;
	if(u - m_timeelapsed > 30000000){
		m_timeelapsed = u;
		if(m_FreqEnable){
			long int tid = syscall(SYS_gettid);
			printf("%s::%lu Freq::%d\n",getName(),tid,m_counter/30);
		}
		m_counter=0;
	}
}
