/*
 * FileConfigurationTest.cpp
 *
 *  Created on: Apr 24, 2017
 *      Author: builder
 */

#include <string>
#include <fstream>

#include "gtest/gtest.h"

#include "logging.h"

#include "FileConfiguration.h"
#include "FileConfigurationTest.h"

const char logger[30] = "FileConfigurationTest";

using namespace std;
using namespace EyelockConfigurationNS;

const std::string FileConfigurationTest::parString = "testString";
const std::string FileConfigurationTest::parStringKey = "Parameter.String";

const char FileConfigurationTest::parCStr[] = "testCString";
const std::string FileConfigurationTest::parCStrKey = "Parameter.CString";

const char FileConfigurationTest::parCh = 127;
const std::string FileConfigurationTest::parChKey = "Parameter.Char";

const short FileConfigurationTest::parShort = 32767;
const std::string FileConfigurationTest::parShortKey = "Parameter.Short";

const int FileConfigurationTest::parInt = 2147483647;
const std::string FileConfigurationTest::parIntKey = "Parameter.Int";

const unsigned int FileConfigurationTest::parUInt = 4294967;
const std::string FileConfigurationTest::parUIntKey = "Parameter.UInt";

const float FileConfigurationTest::parFloat = 333.333f;
const std::string FileConfigurationTest::parFloatKey = "Parameter.Float";

const double FileConfigurationTest::parDouble = 444.444;
const std::string FileConfigurationTest::parDoubleKey = "Parameter.Double";

const bool FileConfigurationTest::parBool = true;
const std::string FileConfigurationTest::parBoolKey = "Parameter.Bool";


void FileConfigurationTest::CreateTestIniFile(const string& savePath)
{
	EyelockLog(logger, TRACE, "Creating test ini-file");

	ifstream ifs(savePath.c_str());
	if (ifs.good())
	{
		ifs.close();
		remove(savePath.c_str());
	}

	FileConfiguration conf;

	conf.setValue(parStringKey.c_str(), parString);
	conf.setValue(parCStrKey.c_str(), parCStr);
	conf.setValue(parChKey.c_str(), parCh);
	conf.setValue(parShortKey.c_str(), parShort);
	conf.setValue(parIntKey.c_str(), parInt);
	conf.setValue(parUIntKey.c_str(), parUInt);
	conf.setValue(parFloatKey.c_str(), parFloat);
	conf.setValue(parDoubleKey.c_str(), parDouble);
	conf.setValue(parBoolKey.c_str(), parBool);

	conf.save(savePath);
}

TEST_F(FileConfigurationTest, CostructorTestStack)
{
	FileConfiguration conf;
	AbstractConfiguration::Keys keys;
	conf.keys(keys);

	EXPECT_TRUE(keys.empty()) << "Default constructor (on stack) produced non-empty configuration";
}

TEST_F(FileConfigurationTest, CostructorTestHeap)
{
	FileConfiguration *pConf = new FileConfiguration();
	AbstractConfiguration::Keys keys;
	pConf->keys(keys);

	EXPECT_TRUE(keys.empty()) << "Default constructor (on heap) produced non-empty configuration";

	delete pConf;
}


TEST_F(FileConfigurationTest, CheckFile)
{
	string testFile = "/home/EyelockConfiguratorTest/fileExistsTest";

	ofstream ofs(testFile.c_str());
	ASSERT_TRUE(ofs.good()) << "Cannot create test file";
	ofs << "test" << endl << endl << endl;
	ofs.close();

	EXPECT_TRUE(FileConfiguration::checkFile(testFile)) << "File existence check error";

	remove(testFile.c_str());
}


TEST_F(FileConfigurationTest, CreateInMemoryConfSaveToFile)
{
	string tempIniFile = "/home/EyelockConfiguratorTest/temp.ini";
	CreateTestIniFile(tempIniFile);

	FileConfiguration conf(tempIniFile);

	remove(tempIniFile.c_str());

	ASSERT_TRUE(conf.has(parStringKey)) << "String parameter not found";
	EXPECT_EQ(parString, conf.getStr(parStringKey.c_str(), string(""))) << "String parameter was retrieved incorrectly";

	ASSERT_TRUE(conf.has(parCStrKey)) << "CString parameter not found";
	const char* pCString = conf.getCStr(parCStrKey.c_str(), (const char*) NULL);
	int compareResult = strncmp(pCString, parCStr, FILECONFIGURATIONTEST_CSTR_LENGTH);
	EXPECT_EQ(0, compareResult) << "CString parameter was retrieved incorrectly";

	ASSERT_TRUE(conf.has(parChKey)) << "Char parameter not found";
	EXPECT_EQ(parCh, conf.getChar(parChKey.c_str(), 0)) << "Char parameter was retrieved incorrectly";

	ASSERT_TRUE(conf.has(parShortKey)) << "Short parameter not found";
	EXPECT_EQ(parShort, conf.getShort(parShortKey.c_str(), 0)) << "Short parameter was retrieved incorrectly";

	ASSERT_TRUE(conf.has(parIntKey)) << "Integer parameter not found";
	EXPECT_EQ(parInt, conf.getInt(parIntKey.c_str(), 0)) << "Integer parameter was retrieved incorrectly";

	ASSERT_TRUE(conf.has(parUIntKey)) << "Unsigned integer parameter not found";
	EXPECT_EQ(parUInt, conf.getUInt(parUIntKey.c_str(), 0)) << "Unsigned integer parameter was retrieved incorrectly";

	ASSERT_TRUE(conf.has(parFloatKey)) << "Float parameter not found";
	EXPECT_EQ(parFloat, conf.getFloat(parFloatKey.c_str(), 0.0f)) << "Float parameter was retrieved incorrectly";

	ASSERT_TRUE(conf.has(parDoubleKey)) << "Double parameter not found";
	EXPECT_EQ(parDouble, conf.getDouble(parDoubleKey.c_str(), 0.0)) << "Double parameter was retrieved incorrectly";

	ASSERT_TRUE(conf.has(parBoolKey)) << "Boolean parameter not found";
	EXPECT_EQ(parBool, conf.getBool(parBoolKey.c_str(), false)) << "Boolean parameter was retrieved incorrectly";
}


TEST_F(FileConfigurationTest, UpdateFile_ChangeExisting)
{
	string tempIniFile = "/home/EyelockConfiguratorTest/temp.ini";
	CreateTestIniFile(tempIniFile);

	FileConfiguration conf;

	// updating the existing values
	string changedString = "String_changed";
	conf.setValue(parStringKey.c_str(), changedString);

	const char changedCString[] = "CString_changed";
	conf.setValue(parCStrKey.c_str(), changedCString);

	char changedChar = 111;
	conf.setValue(parChKey.c_str(), changedChar);

	short changedShort = 1111;
	conf.setValue(parShortKey.c_str(), changedShort);

	int changedInt = -111;
	conf.setValue(parIntKey.c_str(), changedInt);

	unsigned int changedUInt = 11111;
	conf.setValue(parUIntKey.c_str(), changedUInt);

	float changedFloat = 0.1f;
	conf.setValue(parFloatKey.c_str(), changedFloat);

	double changedDouble = 0.001;
	conf.setValue(parDoubleKey.c_str(), changedDouble);

	bool changedBool = false;
	conf.setValue(parBoolKey.c_str(), changedBool);

	//tempIniFile="updated.ini";
	conf.save(tempIniFile);

	FileConfiguration conf_updated(tempIniFile);

	remove(tempIniFile.c_str());

	ASSERT_TRUE(conf_updated.has(parStringKey)) << "String parameter not found";
	EXPECT_EQ(changedString, conf_updated.getStr(parStringKey.c_str(), string(""))) << "String parameter was retrieved incorrectly";

	ASSERT_TRUE(conf_updated.has(parCStrKey)) << "CString parameter not found";
	const char* pCString = conf_updated.getCStr(parCStrKey.c_str(), (const char*) NULL);
	int compareResult = strncmp(pCString, changedCString, FILECONFIGURATIONTEST_CSTR_LENGTH);
	EXPECT_EQ(0, compareResult) << "CString parameter was retrieved incorrectly";

	ASSERT_TRUE(conf_updated.has(parChKey)) << "Char parameter not found";
	EXPECT_EQ(changedChar, conf_updated.getChar(parChKey.c_str(), 0)) << "Char parameter was retrieved incorrectly";

	ASSERT_TRUE(conf_updated.has(parShortKey)) << "Short parameter not found";
	EXPECT_EQ(changedShort, conf_updated.getShort(parShortKey.c_str(), 0)) << "Short parameter was retrieved incorrectly";

	ASSERT_TRUE(conf_updated.has(parIntKey)) << "Integer parameter not found";
	EXPECT_EQ(changedInt, conf_updated.getInt(parIntKey.c_str(), 0)) << "Integer parameter was retrieved incorrectly";

	ASSERT_TRUE(conf_updated.has(parUIntKey)) << "Unsigned integer parameter not found";
	EXPECT_EQ(changedUInt, conf_updated.getUInt(parUIntKey.c_str(), 0)) << "Unsigned integer parameter was retrieved incorrectly";

	ASSERT_TRUE(conf_updated.has(parFloatKey)) << "Float parameter not found";
	EXPECT_EQ(changedFloat, conf_updated.getFloat(parFloatKey.c_str(), 0.0f)) << "Float parameter was retrieved incorrectly";

	ASSERT_TRUE(conf_updated.has(parDoubleKey)) << "Double parameter not found";
	EXPECT_EQ(changedDouble, conf_updated.getDouble(parDoubleKey.c_str(), 0.0)) << "Double parameter was retrieved incorrectly";

	ASSERT_TRUE(conf_updated.has(parBoolKey)) << "Boolean parameter not found";
	EXPECT_EQ(changedBool, conf_updated.getBool(parBoolKey.c_str(), false)) << "Boolean parameter was retrieved incorrectly";
}


TEST_F(FileConfigurationTest, UpdateFile_AddNewRemoveExisting)
{
	string tempIniFile = "/home/EyelockConfiguratorTest/temp.ini";
	CreateTestIniFile(tempIniFile);

	FileConfiguration conf;

	// removing the existing key-value pairs and adding new
	string changedStringKey = parStringKey + ".New";
	string changedString = "String_new";
	conf.setValue(changedStringKey.c_str(), changedString);
	conf.remove(parString);

	const char changedCString[] = "CString_changed";
	string changedCStrKey = parCStrKey + ".New";
	conf.setValue(changedCStrKey.c_str(), changedCString);
	conf.remove(parCStrKey);

	char changedChar = 111;
	string changedChKey = parChKey + ".New";
	conf.setValue(changedChKey.c_str(), changedChar);
	conf.remove(parChKey);

	short changedShort = 1111;
	string changedShortKey = parShortKey + ".New";
	conf.setValue(changedShortKey.c_str(), changedShort);
	conf.remove(parShortKey);

	int changedInt = -111;
	string changedIntKey = parIntKey + ".New";
	conf.setValue(changedIntKey.c_str(), changedInt);
	conf.remove(parIntKey);

	unsigned int changedUInt = 11111;
	string changedUIntKey = parUIntKey + ".New";
	conf.setValue(changedUIntKey.c_str(), changedUInt);
	conf.remove(parUIntKey);

	float changedFloat = 0.1f;
	string changedFloatKey = parFloatKey + ".New";
	conf.setValue(changedFloatKey.c_str(), changedFloat);
	conf.remove(parFloatKey);

	double changedDouble = 0.001;
	string changedDoubleKey = parDoubleKey + ".New";
	conf.setValue(changedDoubleKey.c_str(), changedDouble);
	conf.remove(parDoubleKey);

	bool changedBool = false;
	string changedBoolKey = parBoolKey + ".New";
	conf.setValue(changedBoolKey.c_str(), changedBool);
	conf.remove(parBoolKey);

	//tempIniFile="updated.ini";
	conf.save(tempIniFile);

	FileConfiguration conf_updated(tempIniFile);

	remove(tempIniFile.c_str());

	ASSERT_TRUE(!conf_updated.has(parStringKey)) << "String parameter was not deleted";
	ASSERT_TRUE(conf_updated.has(changedStringKey)) << "New String parameter not found";
	EXPECT_EQ(changedString, conf_updated.getStr(changedStringKey.c_str(), string(""))) << "String parameter was retrieved incorrectly";

	ASSERT_TRUE(!conf_updated.has(parCStrKey)) << "CString parameter was not deleted";
	ASSERT_TRUE(conf_updated.has(changedCStrKey)) << "New CString parameter not found";
	const char* pCString = conf_updated.getCStr(changedCStrKey.c_str(), (const char*) NULL);
	int compareResult = strncmp(pCString, changedCString, FILECONFIGURATIONTEST_CSTR_LENGTH);
	EXPECT_EQ(0, compareResult) << "CString parameter was retrieved incorrectly";

	ASSERT_TRUE(!conf_updated.has(parChKey)) << "Char parameter was not deleted";
	ASSERT_TRUE(conf_updated.has(changedChKey)) << "New Char parameter not found";
	EXPECT_EQ(changedChar, conf_updated.getChar(changedChKey.c_str(), 0)) << "New Char parameter was retrieved incorrectly";

	ASSERT_TRUE(!conf_updated.has(parShortKey)) << "Short parameter was not deleted";
	ASSERT_TRUE(conf_updated.has(changedShortKey)) << "New Short parameter not found";
	EXPECT_EQ(changedShort, conf_updated.getShort(changedShortKey.c_str(), 0)) << "New Short parameter was retrieved incorrectly";

	ASSERT_TRUE(!conf_updated.has(parIntKey)) << "Integer parameter was not deleted";
	ASSERT_TRUE(conf_updated.has(changedIntKey)) << "New Integer parameter not found";
	EXPECT_EQ(changedInt, conf_updated.getInt(changedIntKey.c_str(), 0)) << "New Integer parameter was retrieved incorrectly";

	ASSERT_TRUE(!conf_updated.has(parUIntKey)) << "Unsigned integer parameter was not deleted";
	ASSERT_TRUE(conf_updated.has(changedUIntKey)) << "New Unsigned integer parameter not found";
	EXPECT_EQ(changedUInt, conf_updated.getUInt(changedUIntKey.c_str(), 0)) << "New Unsigned integer parameter was retrieved incorrectly";

	ASSERT_TRUE(!conf_updated.has(parFloatKey)) << "Float parameter was not deleted";
	ASSERT_TRUE(conf_updated.has(changedFloatKey)) << "New Float parameter not found";
	EXPECT_EQ(changedFloat, conf_updated.getFloat(changedFloatKey.c_str(), 0.0f)) << "New Float parameter was retrieved incorrectly";

	ASSERT_TRUE(!conf_updated.has(parDoubleKey)) << "Double parameter was not deleted";
	ASSERT_TRUE(conf_updated.has(changedDoubleKey)) << "New Double parameter not found";
	EXPECT_EQ(changedDouble, conf_updated.getDouble(changedDoubleKey.c_str(), 0.0)) << "New Double parameter was retrieved incorrectly";

	ASSERT_TRUE(!conf_updated.has(parBoolKey)) << "Boolean parameter was not deleted";
	ASSERT_TRUE(conf_updated.has(changedBoolKey)) << "New Boolean parameter not found";
	EXPECT_EQ(changedBool, conf_updated.getBool(changedBoolKey.c_str(), false)) << "New Boolean parameter was retrieved incorrectly";
}

