//
// AbstractConfiguration.h
//
// $Id: //poco/1.4/Util/include/Poco/Util/AbstractConfiguration.h#2 $
//
// Library: Util
// Package: Configuration
// Module:  AbstractConfiguration
//
// Definition of the AbstractConfiguration class.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#ifndef ABSTRACTCONFIGURATION_H_
#define ABSTRACTCONFIGURATION_H_

#include <vector>
#include <string>
#include <utility>

#include "Synchronization.h"

namespace EyelockConfigurationNS
{

class AbstractConfiguration
{
public:
	typedef std::vector<std::string> Keys;
	typedef std::vector<std::string>::const_iterator KeysCIter;
	
	AbstractConfiguration() {};
		/// Creates the AbstractConfiguration.

	bool hasProperty(const std::string& key);
		/// Returns true if the property with the given key exists.

	bool hasOption(const std::string& key);
		/// Returns true if the property with the given key exists.
		///
		/// Same as hasProperty().

	bool has(const std::string& key);
		/// Returns true if the property with the given key exists.
		///
		/// Same as hasProperty().

		
	/// getters
	/// If a property with the given key exists, return the property's value,
	/// otherwise return the given default value.

	std::string getStr(const std::string& key, const std::string& defaultValue = "");

	const char* getCStr(const std::string& key, const char* defaultValue = "");

	char getChar(const std::string& key, char defaultValue = 0);
		/// Numbers starting with 0x are treated as hexadecimal.
		
	short getShort(const std::string& key, short defaultValue = 0);
		/// Numbers starting with 0x are treated as hexadecimal.

	int getInt(const std::string& key, int defaultValue = 0);
		/// Numbers starting with 0x are treated as hexadecimal.
		
	unsigned int getUInt(const std::string& key, unsigned int defaultValue = 0);
		/// Numbers starting with 0x are treated as hexadecimal.
		
	float getFloat(const std::string& key, float defaultValue = 0.0f);

	double getDouble(const std::string& key, double defaultValue = 0.0);
		
	bool getBool(const std::string& key, bool defaultValue = false);
		/// The following string values can be converted into a boolean:
		///   - numerical values: non zero becomes true, zero becomes false
		///   - strings: true, yes, on become true, false, no, off become false
		/// Case does not matter.


	/// setters
	/// Set the property with the given key to the given value.
	/// An already existing value for the key is overwritten.
		
	virtual void setValue(const std::string& key, const std::string& value);

	virtual void setValue(const std::string& key, const char* value);
		
	virtual void setValue(const std::string& key, char value);

	virtual void setValue(const std::string& key, short value);

	virtual void setValue(const std::string& key, int value);
		
	virtual void setValue(const std::string& key, unsigned int value);

	virtual void setValue(const std::string& key, float value);

	virtual void setValue(const std::string& key, double value);

	virtual void setValue(const std::string& key, bool value);
	

	void remove(const std::string& key);
		/// Removes the property with the given key.
		///
		/// Does nothing if the key does not exist.

	virtual void keys(Keys& range) = 0;
		/// Returns the names of all keys.
		///
		/// Must be overridden by subclasses.
	
protected:
	virtual bool getRaw(const std::string& key, std::string& value) = 0;
		/// If the property with the given key exists, stores the property's value
		/// in value and returns true. Otherwise, returns false.
		///
		/// Must be overridden by subclasses.

	virtual void setRaw(const std::string& key, const std::string& value) = 0;
		/// Sets the property with the given key to the given value.
		/// An already existing value for the key is overwritten.
		///
		/// Must be overridden by subclasses.
		
	virtual void removeRaw(const std::string& key);
		/// Removes the property with the given key.
		///
		/// Does nothing if the key does not exist.
	
	virtual ~AbstractConfiguration() {};

	Mutex m_lock;

private:
	AbstractConfiguration(const AbstractConfiguration&);
	AbstractConfiguration& operator = (const AbstractConfiguration&);
};


} // namespace EyelockConfigurationNS


#endif // ABSTRACTCONFIGURATION_H_
