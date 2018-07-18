//
// NumberParser.h
//
// $Id: //poco/1.4/Foundation/include/Poco/NumberParser.h#1 $
//
// Library: Foundation
// Package: Core
// Module:  NumberParser
//
// Definition of the NumberParser class.
//
// Copyright (c) 2004-2006, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#ifndef NUMBERPARSER_H_
#define NUMBERPARSER_H_

#include <string>

namespace EyelockConfigurationNS
{

class NumberParser
	/// The NumberParser class provides static methods
	/// for parsing numbers out of strings.
	///
{
public:

	static int parseInt(const std::string& s);
		/// Parses an integer value from the given string.
		/// Throws a runtime_error if the string does not hold a number.
	
	static bool tryParseInt(const std::string& s, int& value);
		/// Parses an integer value from the given string.
		/// Returns true if a valid integer has been found, false otherwise. 
		/// If parsing was not successful, value is undefined.

	static unsigned parseUnsigned(const std::string& s);
		/// Parses an unsigned integer value from the given string.
		/// Throws a runtime_error if the string does not hold a number.

	static bool tryParseUnsigned(const std::string& s, unsigned int& value);
		/// Parses an unsigned integer value from the given string.
		/// Returns true if a valid integer has been found, false otherwise. 
		/// If parsing was not successful, value is undefined.

	static float parseFloat(const std::string& s);
		/// Parses a float value in decimal floating point notation
		/// from the given string.
		/// Throws a runtime_error if the string does not hold a floating-point
		/// number in decimal notation.
		
	static bool tryParseFloat(const std::string& s, float& value);
		/// Parses a float value in decimal floating point notation
		/// from the given string.
		/// Returns true if a valid floating point number has been found,
		/// false otherwise.
		/// If parsing was not successful, value is undefined.

	static double parseDouble(const std::string& s);
		/// Parses a double value in decimal floating point notation
		/// from the given string.
		/// Throws a runtime_error if the string does not hold a floating-point
		/// number in decimal notation.

	static bool tryParseDouble(const std::string& s, double& value);
		/// Parses a double value in decimal floating point notation
		/// from the given string.
		/// Returns true if a valid floating point number has been found,
		/// false otherwise.
		/// If parsing was not successful, value is undefined.

	static bool parseBool(const std::string& s);
		/// Parses a bool value in decimal or string notation
		/// from the given string.
		/// Valid forms are: "0", "1", "true", "on", false", "yes", "no", "off".
		/// String forms are NOT case sensitive.
		/// Throws a runtime_error if the string does not hold a valid bool number

	static bool tryParseBool(const std::string& s, bool& value);
		/// Parses a bool value in decimal or string notation
		/// from the given string.
		/// Valid forms are: "0", "1", "true", "on", false", "yes", "no", "off".
		/// String forms are NOT case sensitive.
		/// Returns true if a valid bool number has been found,
		/// false otherwise.
		/// If parsing was not successful, value is undefined.
};


} // namespace EyelockConfigurationNS


#endif // NUMBERPARSER_H_
