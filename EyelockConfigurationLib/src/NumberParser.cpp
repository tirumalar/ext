//
// NumberParser.cpp from Poco was taken as example
//



#include "NumberParser.h"
#include <cstdio>
#include <cctype>
#include <stdlib.h>
#include <exception>
#include <stdexcept>
#include <algorithm>

#include <boost/lexical_cast.hpp>

#include "logging.h"

const char logger[30] = "NumberParser";

using namespace std;

using boost::lexical_cast;
using boost::bad_lexical_cast;

namespace EyelockConfigurationNS
{

int NumberParser::parseInt(const std::string& s)
{
	int result;
	if (tryParseInt(s, result))
		return result;
	else
		throw runtime_error("Not a valid integer");
}


bool NumberParser::tryParseInt(const std::string& s, int& value)
{
	if (s.empty())
	{
		return false;
	}

	//value = strtol(s.c_str(), NULL, base); // doesn't return an error if invalid value is passed

    try
    {
    	value = lexical_cast<int>(s);
    }
    catch (const bad_lexical_cast &)
    {
    	EyelockLog(logger, ERROR, "Failed to convert to int: %s", s.c_str());
    	return false;
    }

	return true;
}


unsigned NumberParser::parseUnsigned(const std::string& s)
{
	unsigned result;
	if (tryParseUnsigned(s, result))
		return result;
	else
		throw runtime_error("Not a valid unsigned integer");
}


bool NumberParser::tryParseUnsigned(const std::string& s, unsigned int& value)
{
	if (s.empty())
	{
		return false;
	}

	//value = strtoul(s.c_str(), NULL, base); // doesn't return an error if invalid value is passed

    try
    {
    	value = lexical_cast<unsigned int>(s);
    }
    catch (const bad_lexical_cast &)
    {
    	EyelockLog(logger, ERROR, "Failed to convert to unsigned int: %s", s.c_str());
    	return false;
    }

	return true;
}


float NumberParser::parseFloat(const std::string& s)
{
	float result;
	if (tryParseFloat(s, result))
		return result;
	else
		throw runtime_error("Not a valid floating-point number");
}


bool NumberParser::tryParseFloat(const std::string& s, float& value)
{
	if (s.empty())
	{
		return false;
	}

	// value = strtof(s.c_str(), NULL);  // doesn't return an error if invalid value is passed

    try
    {
    	value = lexical_cast<float>(s);
    }
    catch (const bad_lexical_cast &)
    {
    	EyelockLog(logger, ERROR, "Failed to convert to float: %s", s.c_str());
    	return false;
    }

	return true;
}

double NumberParser::parseDouble(const std::string& s)
{
	double result;
	if (tryParseDouble(s, result))
		return result;
	else
		throw runtime_error("Not a valid floating-point (double) number");
}

bool NumberParser::tryParseDouble(const std::string& s, double& value)
{
	if (s.empty())
	{
		return false;
	}

	// value = strtod(s.c_str(), NULL); // doesn't return an error if invalid value is passed

    try
    {
    	value = lexical_cast<double>(s);
    }
    catch (const bad_lexical_cast &)
    {
    	EyelockLog(logger, ERROR, "Failed to convert to double: %s", s.c_str());
    	return false;
    }

	return true;
}


bool NumberParser::parseBool(const std::string& s)
{
	bool result;
	if (tryParseBool(s, result))
		return result;
	else
		throw runtime_error("Not a valid boolean value");
}


bool NumberParser::tryParseBool(const std::string& s, bool& value)
{
	string temp(s);
	transform(temp.begin(), temp.end(), temp.begin(), ::tolower);

	if (temp.compare("1") == 0)
	{
		value = true;
		return true;
	}
	if (temp.compare("true") == 0)
	{
		value = true;
		return true;
	}
	else if (temp.compare("yes") == 0)
	{
		value = true;
		return true;
	}
	else if (temp.compare("on") == 0)
	{
		value = true;
		return true;
	}
	
	if (temp.compare("false") == 0)
	{
		value = false;
		return true;
	}
	if (temp.compare("0") == 0)
	{
		value = false;
		return true;
	}
	else if (temp.compare("no") == 0)
	{
		value = false;
		return true;
	}
	else if (temp.compare("off") == 0)
	{
		value = false;
		return true;
	}
	
	return false;
}

} // namespace EyelockConfigurationNS
