/*
 * ParameterValidator.h
 *
 *  Created on: Apr 20, 2017
 *      Author: builder
 */

#ifndef PARAMETERVALIDATOR_H_
#define PARAMETERVALIDATOR_H_

namespace EyelockConfigurationNS
{

	class ParameterValidator
	{
		public:
			ParameterValidator() {};
			virtual ~ParameterValidator() {};

			template<typename T>
			static bool validateRange(T value, T min, T max)
			{
				return !(value < min || value > max);
			}

			static bool validateRegex(std::string value, std::string regexp);
	};

} // namespace EyelockConfigurationNS

#endif /* PARAMETERVALIDATOR_H_ */
