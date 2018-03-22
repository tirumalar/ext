/*
 * Synchronization.cpp
 *
 *  Created on: Jan 31, 2012
 *      Author: dhirvonen
 */

#include "Synchronization.h"

ScopeLock::ScopeLock(Mutex &mutex) : m_Lock(mutex.Get())
{
	pthread_mutex_lock(&m_Lock);
}

ScopeLock::ScopeLock(pthread_mutex_t &mutex) : m_Lock(mutex)
{
	pthread_mutex_lock(&m_Lock);
}

ScopeLock::~ScopeLock()
{
	pthread_mutex_unlock(&m_Lock);
}

Semaphore::Semaphore(unsigned int initial_count)
{
	Init(initial_count);
}

Semaphore::~Semaphore()
{
	pthread_mutex_destroy(&lock_);
	pthread_cond_destroy(&count_nonzero_);
}

int Semaphore::Init(unsigned int initial_count)
{
	pthread_mutex_init(&lock_, NULL);
	pthread_cond_init(&count_nonzero_, NULL);
	count_ = initial_count;
	waiters_count_ = 0;

	return 0;
}

int Semaphore::Wait()
{
	pthread_mutex_lock (&lock_);   // Acquire mutex to enter critical section.
	waiters_count_++;// Keep track of the number of waiters so that <sema_post> works correctly.

	// Wait until the semaphore count is > 0, then atomically release
	// <lock_> and wait for <count_nonzero_> to be signaled.
	while (count_ == 0)
		pthread_cond_wait (&count_nonzero_, &lock_);

	// <lock_> is now held
	waiters_count_--; // Decrement the waiters count.
	count_--; // Decrement the semaphore's count.
	pthread_mutex_unlock (&lock_); // Release mutex to leave critical section.

	return 0;
}

int Semaphore::Post()
{
	pthread_mutex_lock (&lock_);

	// Always allow one thread to continue if it is waiting.
	if (waiters_count_ > 0)
		pthread_cond_signal (&count_nonzero_);

	// Increment the semaphore's count.
	count_++;

	pthread_mutex_unlock (&lock_);

	return 0;
}

Mutex::Mutex()
{
	pthread_mutex_init(&m_Mutex, NULL);
}
Mutex::~Mutex()
{
	pthread_mutex_destroy(&m_Mutex);
}
void Mutex::Lock()
{
	pthread_mutex_lock(&m_Mutex);
}
void Mutex::Unlock()
{
	pthread_mutex_unlock(&m_Mutex);
}

//////

BinarySemaphore::BinarySemaphore()
{
	/* Init mutex */
	pthread_mutex_init(&m_mutex, NULL);
	/* Init cond. variable */
	pthread_cond_init(&m_cv, NULL);
	/* Set flag value */
	m_flag = 1;
}

BinarySemaphore::~BinarySemaphore()
{
	pthread_mutex_destroy(&m_mutex);
	pthread_cond_destroy(&m_cv);
}

void BinarySemaphore::Lock()
{
	/* Try to get exclusive access to flag */
	pthread_mutex_lock(&m_mutex);

	/* Success - no other thread can get here unless the current thread unlock "mutex" */

	/* Examine the flag and wait until flag == 1 */
	while (m_flag == 0)
	{
		pthread_cond_wait(&m_cv, &m_mutex);
		/* When the current thread execute this
	       pthread_cond_wait() statement, the
	       current thread will be block on cv
	       and (atomically) unlocks mutex !!!
	       Unlocking mutex will let other thread
	       in to test flag.... */
	}

	/* -----------------------------------------
        If the program arrive here, we know that flag == 1 and this thread has now
		successfully pass the semaphore !!!
      ------------------------------------------- */
	m_flag = 0;  /* This will cause all other threads that executes a P() call to wait in the (above) while-loop !!! */

	/* Release exclusive access to flag */
	pthread_mutex_unlock(&m_mutex);
}

void BinarySemaphore::Unlock()
{
	/* Try to get exclusive access to flag */
	pthread_mutex_lock(&m_mutex);
	/* Update semaphore state to Up */
	m_flag = 1;
	pthread_cond_signal(&m_cv);
	/* This call may restart some thread that was blocked on cv (in the P() call)
	   if there was not thread blocked on cv, this operation does absolutely nothing...
	 */

	/* Release exclusive access to flag */
	pthread_mutex_unlock(&m_mutex);
}

//////


