//
// NumberFormatter.h
//
// $Id: //poco/1.4/Foundation/include/Poco/NumberFormatter.h#1 $
//
// Library: Foundation
// Package: Core
// Module:  NumberFormatter
//
// Definition of the NumberFormatter class.
//
// Copyright (c) 2004-2008, Applied Informatics Software Engineering GmbH.
// and Contributors.
//
// SPDX-License-Identifier:	BSL-1.0
//


#ifndef NUMBERFORMATTER_H_
#define NUMBERFORMATTER_H_

#include <sstream>

namespace EyelockConfigurationNS
{

class NumberFormatter
{
public:

	template<typename T>
	static std::string format(T value)
	{
		std::stringstream ss;
		ss << value;
		if (ss.fail())
		{
			return "";
		}

		return ss.str();
	}

	static std::string format(bool value)
	{
		return (value) ? "true" : "false";
	}

};

} // namespace EyelockConfigurationNS


#endif // NUMBERFORMATTER_H_
