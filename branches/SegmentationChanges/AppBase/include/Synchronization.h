/*
 * Synchronization.h
 *
 *  Created on: Jan 31, 2012
 *      Author: dhirvonen
 */

#ifndef SYNCHRONIZATION_H_
#define SYNCHRONIZATION_H_

#include <pthread.h>
#include <vector>
#include <deque>
#include <Safe.h>

class Mutex;
class ScopeLock
{
public:
	ScopeLock(pthread_mutex_t &mutex);
	ScopeLock(Mutex &mutex);
	~ScopeLock();
protected:
	pthread_mutex_t &m_Lock;
};

template <class T>
class SafeLock
{
public:
	SafeLock(Safe<T> &src) : m_Object(src) { m_Object.lock(); }
    ~SafeLock() { m_Object.unlock(); }
protected:
    Safe<T> m_Object;
};

class Semaphore
{
public:
	Semaphore(unsigned int initial_count = 0);
	~Semaphore();
	int Wait();
	int Post();

private:
	int Init(unsigned int initial_count);
	unsigned int  count_; // Current count of the semaphore.
	unsigned long waiters_count_; // Number of threads that have called <sema_wait>.
	pthread_mutex_t lock_; // Serialize access to <count_> and <waiters_count_>.
	pthread_cond_t count_nonzero_;  // Condition variable that blocks the <count_> 0.
};

// Pthread binary semaphore implementation using a mutex + condition variable
// http://www.mathcs.emory.edu/~cheung/Courses/455/Syllabus/5c-pthreads/sync.html

class BinarySemaphore
{
public:
	BinarySemaphore();
	~BinarySemaphore();
	void Lock();
	void Unlock();
protected:

     pthread_cond_t  m_cv;    /* cond. variable - used to block threads */
     pthread_mutex_t m_mutex; /* mutex variable - used to prevents concurrent access to the variable "flag" */
     int m_flag;              /* Semaphore state: 0 = down, 1 = up */
};

class Mutex
{
public:
	friend class ScopeLock;
	Mutex();
	~Mutex();
	void Lock();
	void Unlock();
protected:
	pthread_mutex_t &Get() { return m_Mutex; }
	pthread_mutex_t m_Mutex;
};

template <class T> T GetSafeValue(Safe<T> &value)
{
	T tmp = 0;
	value.lock();
	tmp = value.get();
	value.unlock();
	return tmp;
}

template <class T> void SetSafeValue(Safe<T> &value, T arg)
{
	value.lock();
	value.set(arg);
	value.unlock();
}

// Basic producer consumer class using pthread condition variables.
// Opted for non-template version for ARM platform.



template <class T>
class RingBuffer
{
protected:
	pthread_cond_t  m_NotFull, m_NotNull;
	std::deque<T> m_Buffer;
	int m_Max;
public:
	pthread_mutex_t m_Lock;
	RingBuffer(int length) : m_Max(length)
	{
		pthread_mutex_init(&m_Lock, 0);
		pthread_cond_init(&m_NotNull, 0);
		pthread_cond_init(&m_NotFull, 0);
	}

	~RingBuffer()
	{
		pthread_mutex_destroy(&m_Lock);
		pthread_cond_destroy(&m_NotNull);
		pthread_cond_destroy(&m_NotFull);
	}

	bool Empty()
	{
		return m_Buffer.empty();
	}

	int Size(){
		return m_Buffer.size();
	}

	bool Full()
	{
		return m_Buffer.size() == m_Max;
	}

	void Clear()
	{
		m_Buffer.clear();
	}

	void Erase(int nIndex)
	{
		if(nIndex < Size())
			m_Buffer.erase(m_Buffer.begin() + nIndex);

	}

void Push(T &thing)
	{
		ScopeLock lock(m_Lock);
		while(m_Max == m_Buffer.size())
		{
			pthread_cond_wait( &m_NotFull, &m_Lock );
		}
		m_Buffer.push_back(thing);
		pthread_cond_signal( &m_NotNull );
	}

bool TryPush(T &thing)
	{
		bool status = false;
		ScopeLock lock(m_Lock);
		if(m_Max != m_Buffer.size())
		{
			status = true;
			m_Buffer.push_back( thing );
			pthread_cond_signal( &m_NotNull );
		}
		return status;
	}
T Pop() // wait and pop
	{
		ScopeLock lock(m_Lock);
		while(m_Buffer.empty())
		{
			pthread_cond_wait( &m_NotNull, &m_Lock );
		}
		T value = m_Buffer.front();
		m_Buffer.pop_front();
		pthread_cond_signal(&m_NotFull);
		return value;
	}
	bool TryPop(T &thing) // wait and pop
	{
		bool status = false;
		ScopeLock lock(m_Lock);
		if(m_Buffer.size())
		{
			status = true;
			thing = m_Buffer.front();
			m_Buffer.pop_front();
			pthread_cond_signal(&m_NotFull);
		}
		return status;
	}
	bool TryPopBack(T &thing) // wait and pop
			{
		bool status = false;
		ScopeLock lock(m_Lock);
		if (m_Buffer.size()) {
			status = true;
			thing = m_Buffer.back();
			m_Buffer.pop_back();
			pthread_cond_signal(&m_NotFull);
		}
		return status;
	}
T Peek() // wait and peek
	{
		ScopeLock lock(m_Lock);
		while(m_Buffer.empty())
		{
			pthread_cond_wait( &m_NotNull, &m_Lock );
		}
		T value = m_Buffer.front();
		pthread_cond_signal(&m_NotFull);
		return value;
	}

T at(int index) // Get element in the queue
	{
		ScopeLock lock(m_Lock);
		if(m_Buffer.empty())
			return (m_Buffer.front());

		int size = m_Buffer.size();
		if(index < size - 1){
			T value = m_Buffer.at(index);
			return value;
		}else{
			return (m_Buffer.front());
		}
	}
};


#endif /* SYNCHRONIZATION_H_ */
