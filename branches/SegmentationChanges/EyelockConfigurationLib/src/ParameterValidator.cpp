/*
 * ParameterValidator.cpp
 *
 *  Created on: Apr 20, 2017
 *      Author: builder
 */

#include <string>
#include <regex.h>

#include "ParameterValidator.h"


using namespace std;

namespace EyelockConfigurationNS
{

bool ParameterValidator::validateRegex(string value, string regexp)
{
	regex_t re;
	if (regcomp(&re, regexp.c_str(), REG_EXTENDED) != 0)
	{
		return false;
	}

	int status = regexec(&re, value.c_str(), (size_t) 0, NULL, 0);
	regfree(&re);
	return (status == 0) ? true: false;
}

} // namespace EyelockConfigurationNS
