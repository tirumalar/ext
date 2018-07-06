#pragma once

/**
* @ingroup CoralFoundation
* @{
* @class Singleton
* @brief Template class to hold singleton objects
*/

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
/** @} */