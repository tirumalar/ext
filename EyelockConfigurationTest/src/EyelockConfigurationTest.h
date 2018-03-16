/*
 * EyelockConfigurationTest.h
 *
 *  Created on: Apr 12, 2017
 *      Author: builder
 */

#ifndef EYELOCKCONFIGURATIONTEST_H_
#define EYELOCKCONFIGURATIONTEST_H_

#include "gtest/gtest.h"

class EyelockConfigurationTest : public ::testing::Test
{
protected:
	EyelockConfigurationTest() {};

	virtual ~EyelockConfigurationTest()
	{
	// You can do clean-up work that doesn't throw exceptions here.
	}

	virtual void SetUp();
	virtual void TearDown() {};

	static const std::string tempEyelockIni;

	// Objects declared here can be used by all tests in the test case for EyelockConfigurationTest.
};

#endif /* EYELOCKCONFIGURATIONTEST_H_ */
