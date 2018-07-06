/*
 * EyelockConfiguratorTest.h
 *
 *  Created on: Apr 28, 2017
 *      Author: builder
 */

#ifndef EYELOCKCONFIGURATORTEST_H_
#define EYELOCKCONFIGURATORTEST_H_

#include "gtest/gtest.h"

namespace EyelockConfigurationNS {


class EyelockConfiguratorTest : public ::testing::Test
{
protected:
	EyelockConfiguratorTest() {};

	virtual ~EyelockConfiguratorTest()
	{
	// You can do clean-up work that doesn't throw exceptions here.
	}

	virtual void SetUp();
	virtual void TearDown() {};

	static const std::string tempEyelockIni;
	static const std::string eyelockIni;
	static const std::string eyelockConfiguratorBinaryName;

	bool runConfigurator();
	void assertNoNwm();
	void assertNwmIsSet(const std::string& endpoint);
	void assertWiegand();
	void assertOSDP();

	void assertIrisOnly();
	void assertIrisOrCard();
	void assertIrisAndCard();

	// Objects declared here can be used by all tests in the test case for EyelockConfigurationTest.
};
} /* namespace EyelockConfigurationNS */

#endif /* EYELOCKCONFIGURATORTEST_H_ */
