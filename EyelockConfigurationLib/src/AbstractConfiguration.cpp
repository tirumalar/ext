//
// AbstractConfiguration.cpp
//
// $Id: //poco/1.4/Util/src/AbstractConfiguration.cpp#2 $
//
// Library: Util
// Package: Configuration
// Module:  AbstractConfiguration
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#include <string>
#include <stdexcept>

#include "AbstractConfiguration.h"
#include "NumberParser.h"
#include "NumberFormatter.h"
#include "Synchronization.h"

#include "logging.h"

const char logger[30] = "AbstractConfiguration";

using namespace std;

namespace EyelockConfigurationNS
{

bool AbstractConfiguration::hasProperty(const string& key)
{
	ScopeLock lock(m_lock);

	string value;
	return getRaw(key, value);
}


bool AbstractConfiguration::hasOption(const string& key)
{
	return hasProperty(key);
}


bool AbstractConfiguration::has(const string& key)
{
	return hasProperty(key);
}

	
string AbstractConfiguration::getStr(const string& key, const string& defaultValue)
{
	ScopeLock lock(m_lock);

	string value;
	if (getRaw(key, value))
		return value;
	else
		return defaultValue;
}

const char* AbstractConfiguration::getCStr(const string& key, const char* defaultValue)
{
	ScopeLock lock(m_lock);

	string value;
	if (getRaw(key, value))
		return value.c_str();
	else
		return defaultValue;
}

char AbstractConfiguration::getChar(const string& key, char defaultValue)
{
	ScopeLock lock(m_lock);

	string value;
	int ret = 0;
	if (getRaw(key, value))
	{
		if (NumberParser::tryParseInt(value, ret))
		{
			return (char)ret;
		}
	}

	return defaultValue;
}


short AbstractConfiguration::getShort(const string& key, short defaultValue)
{
	ScopeLock lock(m_lock);

	string value;
	int ret = 0;
	if (getRaw(key, value))
	{
		if (NumberParser::tryParseInt(value, ret))
		{
			return (short)ret;
		}
	}

	return defaultValue;
}


int AbstractConfiguration::getInt(const string& key, int defaultValue)
{
	ScopeLock lock(m_lock);

	string value;
	int ret = 0;
	if (getRaw(key, value))
	{
		if (NumberParser::tryParseInt(value, ret))
		{
			return ret;
		}
	}

	return defaultValue;
}


unsigned int AbstractConfiguration::getUInt(const string& key, unsigned int defaultValue)
{
	ScopeLock lock(m_lock);

	string value;
	unsigned int ret = 0;
	if (getRaw(key, value))
	{
		if (NumberParser::tryParseUnsigned(value, ret))
		{
			return ret;
		}
	}

	return defaultValue;
}


float AbstractConfiguration::getFloat(const string& key, float defaultValue)
{
	ScopeLock lock(m_lock);

	string value;
	float ret = 0;
	if (getRaw(key, value))
	{
		if (NumberParser::tryParseFloat(value, ret))
		{
			return ret;
		}
	}

	return defaultValue;
}


double AbstractConfiguration::getDouble(const string& key, double defaultValue)
{
	ScopeLock lock(m_lock);

	string value;
	double ret = 0;
	if (getRaw(key, value))
	{
		if (NumberParser::tryParseDouble(value, ret))
		{
			return ret;
		}
	}

	return defaultValue;
}


bool AbstractConfiguration::getBool(const string& key, bool defaultValue)
{
	ScopeLock lock(m_lock);

	string value;
	bool ret = 0;
	if (getRaw(key, value))
	{
		if (NumberParser::tryParseBool(value, ret))
		{
			return ret;
		}
	}

	return defaultValue;
}


void AbstractConfiguration::setValue(const string& key, const string& value)
{
	setRaw(key, value);
}


void AbstractConfiguration::setValue(const string& key, const char* value)
{
	setRaw(key, string(value));
}


void AbstractConfiguration::setValue(const string& key, char value)
{
	setRaw(key, NumberFormatter::format((int)value));
}


void AbstractConfiguration::setValue(const string& key, short value)
{
	setRaw(key, NumberFormatter::format((int)value));
}


void AbstractConfiguration::setValue(const string& key, int value)
{
	setRaw(key, NumberFormatter::format(value));
}

	
void AbstractConfiguration::setValue(const string& key, unsigned int value)
{
	setRaw(key, NumberFormatter::format(value));
}


void AbstractConfiguration::setValue(const string& key, float value)
{
	setRaw(key, NumberFormatter::format(value));
}


void AbstractConfiguration::setValue(const string& key, double value)
{
	setRaw(key, NumberFormatter::format(value));
}


void AbstractConfiguration::setValue(const string& key, bool value)
{
	setRaw(key, NumberFormatter::format(value));
}


void AbstractConfiguration::remove(const string& key)
{
	ScopeLock lock(m_lock);
	removeRaw(key);
}


void AbstractConfiguration::removeRaw(const string& key)
{
}


} // namespace EyelockConfigurationNS
