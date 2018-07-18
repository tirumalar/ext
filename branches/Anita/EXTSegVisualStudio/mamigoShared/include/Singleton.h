/*
 * Singleton.h
 *
 *  Created on: Apr 21, 2012
 *      Author: mamigo
 */

#ifndef SINGLETON_H_
#define SINGLETON_H_

// http://en.wikipedia.org/wiki/Singleton_pattern#C.2B.2B
template<typename T> class Singleton
{
public:
/**
* Get the instance of the object
* @return Reference to the object
*/
	static T& instance()
	{
		static T theSingleInstance;  // assumes T has a protected default constructor
		return theSingleInstance;
	}
/**
* Initialize the Singleton object
*/
	virtual void init() {}
/**
* Terminate the Singleton object
*/
	virtual void term() {}
};

#endif /* SINGLETON_H_ */
