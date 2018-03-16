/*
 * FileConfigurationTest.h
 *
 *  Created on: Apr 24, 2017
 *      Author: builder
 */

#ifndef FILECONFIGURATIONTEST_H_
#define FILECONFIGURATIONTEST_H_

#include "gtest/gtest.h"

#include "FileConfiguration.h"

#define FILECONFIGURATIONTEST_CSTR_LENGTH 50

class FileConfigurationTest : public ::testing::Test
{
protected:


	FileConfigurationTest()
	{
	// You can do set-up work for each test here.
	}

	virtual ~FileConfigurationTest()
	{
	// You can do clean-up work that doesn't throw exceptions here.
	}

	// If the constructor and destructor are not enough for setting up
	// and cleaning up each test, you can define the following methods:

	virtual void SetUp()
	{
	// Code here will be called immediately after the constructor (right
	// before each test).
	}

	virtual void TearDown()
	{
	// Code here will be called immediately after each test (right
	// before the destructor).
	}

	virtual void CreateTestIniFile(const std::string& savePath);

protected:

	static const std::string parString;
	static const std::string parStringKey;

	static const char parCStr[FILECONFIGURATIONTEST_CSTR_LENGTH];
	static const std::string parCStrKey;

	static const char parCh;
	static const std::string parChKey;

	static const short parShort;
	static const std::string parShortKey;

	static const int parInt;
	static const std::string parIntKey;

	static const unsigned int parUInt;
	static const std::string parUIntKey;

	static const float parFloat;
	static const std::string parFloatKey;

	static const double parDouble;
	static const std::string parDoubleKey;

	static const bool parBool;
	static const std::string parBoolKey;

// Objects declared here can be used by all tests in the test case for EyelockConfigurationTest.
};



#endif /* FILECONFIGURATIONTEST_H_ */
