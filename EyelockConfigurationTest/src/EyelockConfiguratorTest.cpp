/*
 * EyelockConfiguratorTest.cpp
 *
 *  Created on: Apr 28, 2017
 *      Author: builder
 */

#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <climits>

#include "gtest/gtest.h"

#include "logging.h"

#include "EyelockConfiguratorTest.h"
#include "EyelockConfiguration.h"

const char logger[30] = "EyelockConfiguratorTest";

using namespace std;
using namespace EyelockConfigurationNS;

const string EyelockConfiguratorTest::tempEyelockIni = "temp.Eyelock.ini";
const string EyelockConfiguratorTest::eyelockIni = "Eyelock.ini";
const string EyelockConfiguratorTest::eyelockConfiguratorBinaryName = "EyelockConfigurator";

void EyelockConfiguratorTest::SetUp()
{
	ifstream ifs(tempEyelockIni.c_str());
	if (ifs.good())
	{
		ifs.close();
		remove(tempEyelockIni.c_str());
	}

	string cmd = "cp /home/default/Eyelock.ini " + eyelockIni;
	system(cmd.c_str());
}


bool EyelockConfiguratorTest::runConfigurator()
{
	EyelockLog(logger, TRACE, "Running EyelockConfigurator");
	if (FileConfiguration::checkFile(eyelockConfiguratorBinaryName))
	{
		string cmd = "./" + eyelockConfiguratorBinaryName + " UPDATE " + tempEyelockIni;
		system(cmd.c_str());
		return true;
	}

	return false;
}

TEST_F(EyelockConfiguratorTest, LoadDefaultFile)
{
	EyelockConfiguration conf(eyelockIni);

	EXPECT_EQ((unsigned int)4000, conf.getGRI_RepeatAuthorizationPeriod());
}

TEST_F(EyelockConfiguratorTest, UpdateValid)
{
	FileConfiguration conf;
	int AuthorizationToneVolumeNewValue = 81;
	bool DualMatcherPolicyNewValue = true;
	conf.setValue("GRI.AuthorizationToneVolume", AuthorizationToneVolumeNewValue);
	conf.setValue("Eyelock.DualMatcherPolicy", DualMatcherPolicyNewValue);

	conf.save(tempEyelockIni);

	runConfigurator();

	FileConfiguration postUpdateConf(tempEyelockIni);
	AbstractConfiguration::Keys postUpdateKeys;
	postUpdateConf.keys(postUpdateKeys);
	EXPECT_TRUE(postUpdateKeys.empty()) << "Not all keys were deleted after successful update";

	FileConfiguration checkingConf(eyelockIni);
	EXPECT_EQ(AuthorizationToneVolumeNewValue, checkingConf.getInt("GRI.AuthorizationToneVolume")) << "Incorrect value for AuthorizationToneVolume in saved file";
	EXPECT_EQ(DualMatcherPolicyNewValue, checkingConf.getBool("Eyelock.DualMatcherPolicy")) << "Incorrect value for DualMatcherPolicy in saved file";
}


TEST_F(EyelockConfiguratorTest, UpdateInvalidValueUnknownKey)
{
	FileConfiguration conf;
	int AuthorizationToneVolumeNewValue = INT_MAX;
	bool DualMatcherPolicyNewValue = true;
	conf.setValue("GRI.AuthorizationToneVolume", AuthorizationToneVolumeNewValue);
	conf.setValue("Eyelock.DualMatcherPolicy", DualMatcherPolicyNewValue);
	conf.setValue("Eyelock.Unknown", "testUnknown");

	conf.save(tempEyelockIni);

	runConfigurator();

	FileConfiguration postUpdateConf(tempEyelockIni);
	AbstractConfiguration::Keys postUpdateKeys;

	postUpdateConf.keys(postUpdateKeys);
	EXPECT_TRUE(find(postUpdateKeys.begin(), postUpdateKeys.end(), "Eyelock.DualMatcherPolicy") == postUpdateKeys.end()) << "Not all keys were deleted after successful update";
	EXPECT_TRUE(find(postUpdateKeys.begin(), postUpdateKeys.end(), "GRI.AuthorizationToneVolume") != postUpdateKeys.end()) << "Missing key with invalid value after unsuccessful update";
	EXPECT_TRUE(find(postUpdateKeys.begin(), postUpdateKeys.end(), "Eyelock.Unknown") != postUpdateKeys.end()) << "Missing unknown key after unsuccessful update";

	FileConfiguration checkingConf(eyelockIni);
	EXPECT_EQ(40, checkingConf.getInt("GRI.AuthorizationToneVolume")) << "Incorrect value for AuthorizationToneVolume in saved file";
	EXPECT_EQ(DualMatcherPolicyNewValue, checkingConf.getBool("Eyelock.DualMatcherPolicy")) << "Incorrect value for DualMatcherPolicy in saved file";
	EXPECT_FALSE(checkingConf.has("Eyelock.Unknown")) << "Unsupported key in saved file";
}

void EyelockConfiguratorTest::assertNoNwm()
{
	FileConfiguration conf(eyelockIni);
	EXPECT_EQ(1, conf.getInt("GRI.HDMatcherID"));
	EXPECT_EQ(2, conf.getInt("GRI.HDMatcherCount"));
	EXPECT_FALSE(conf.has("GRI.HDMatcher.2.Type"));
	EXPECT_FALSE(conf.has("GRI.HDMatcher.2.BuffSize"));
	EXPECT_FALSE(conf.has("GRI.HDMatcher.2.Address"));

	EXPECT_FALSE(conf.getBool("HighLevel.NetworkMatcherEnabled", true));

	FileConfiguration tempconf(tempEyelockIni);
	EXPECT_TRUE(tempconf.has("HighLevel.NetworkMatcherEnabled"));
}

void EyelockConfiguratorTest::assertNwmIsSet(const string& endpoint)
{
	FileConfiguration conf(eyelockIni);
	EXPECT_EQ(2, conf.getInt("GRI.HDMatcherID"));
	EXPECT_EQ(3, conf.getInt("GRI.HDMatcherCount"));
	EXPECT_EQ("PCMATCHER", conf.getStr("GRI.HDMatcher.2.Type"));
	EXPECT_EQ(0, conf.getInt("GRI.HDMatcher.2.BuffSize"));
	EXPECT_EQ(endpoint, conf.getStr("GRI.HDMatcher.2.Address"));

	EXPECT_TRUE(conf.getBool("HighLevel.NetworkMatcherEnabled"));

	FileConfiguration tempconf(tempEyelockIni);
	EXPECT_FALSE(tempconf.has("HighLevel.NetworkMatcherEnabled"));
}

TEST_F(EyelockConfiguratorTest, Highlevel_NetworkMatcher)
{
	string nwmini = tempEyelockIni;
	ofstream fs(nwmini.c_str());
	if (fs.good())
	{
		fs << "# test network matcher setting" << endl;
	}
	fs.close();

	FileConfiguration fconf(tempEyelockIni);

	EyelockLog(logger, TRACE, "Setting invalid endpoint");
	fconf.setValue("HighLevel.NetworkMatcherAddress", "192.168.113.111.28:50000");
	fconf.setValue("HighLevel.NetworkMatcherEnabled", true);
	ASSERT_TRUE(fconf.save(tempEyelockIni));
	runConfigurator();
	assertNoNwm();

	EyelockLog(logger, TRACE, "Setting injection");
	fconf.setValue("HighLevel.NetworkMatcherAddress", "192.168.113.28:50000;INJECT=INJECT");
	fconf.setValue("HighLevel.NetworkMatcherEnabled", true);
	ASSERT_TRUE(fconf.save(tempEyelockIni));
	runConfigurator();
	assertNoNwm();

	EyelockLog(logger, TRACE, "Setting valid endpoint");
	string nwmValid = "192.168.113.28:50000";
	fconf.setValue("HighLevel.NetworkMatcherAddress", nwmValid);
	fconf.setValue("HighLevel.NetworkMatcherEnabled", true);
	ASSERT_TRUE(fconf.save(tempEyelockIni));
	runConfigurator();
	assertNwmIsSet(nwmValid);
}

void EyelockConfiguratorTest::assertWiegand()
{
	FileConfiguration conf(eyelockIni);
	EXPECT_EQ(false, conf.getBool("GRITrigger.OSDPEnable", true));
	EXPECT_EQ(true, conf.getBool("GRITrigger.WeigandEnable"));
	EXPECT_EQ(false, conf.getBool("GRITrigger.WeigandHidEnable", true));
	EXPECT_EQ(false, conf.getBool("GRITrigger.PACEnable", true));
	EXPECT_EQ(false, conf.getBool("GRITrigger.F2FEnable", true));
}

void EyelockConfiguratorTest::assertOSDP()
{
	FileConfiguration conf(eyelockIni);
	EXPECT_EQ(true, conf.getBool("GRITrigger.OSDPEnable"));
	EXPECT_EQ(false, conf.getBool("GRITrigger.WeigandEnable", true));
	EXPECT_EQ(false, conf.getBool("GRITrigger.WeigandHidEnable", true));
	EXPECT_EQ(false, conf.getBool("GRITrigger.PACEnable", true));
	EXPECT_EQ(false, conf.getBool("GRITrigger.F2FEnable", true));
}

TEST_F(EyelockConfiguratorTest, Highlevel_Protocol)
{
	ofstream fs(tempEyelockIni.c_str());
	if (fs.good())
	{
		fs << "# test protocol setting" << endl;
	}
	fs.close();

	EyelockLog(logger, TRACE, "Creating FileConfiguration instances");
	FileConfiguration fconf(tempEyelockIni);

	EyelockLog(logger, TRACE, "Setting OSDP protocol");
	fconf.setValue("HighLevel.Protocol", "OSDP");
	ASSERT_TRUE(fconf.save(tempEyelockIni));
	runConfigurator();
	assertOSDP();
	fconf.load(tempEyelockIni);
	ASSERT_FALSE(fconf.has("HighLevel.Protocol"));

	EyelockLog(logger, TRACE, "Setting WIEGAND protocol");
	fconf.setValue("HighLevel.Protocol", "WIEGAND");
	ASSERT_TRUE(fconf.save(tempEyelockIni));
	runConfigurator();
	assertWiegand();
	fconf.load(tempEyelockIni);
	ASSERT_FALSE(fconf.has("HighLevel.Protocol"));

	EyelockLog(logger, TRACE, "Setting invalid protocol");
	fconf.setValue("HighLevel.Protocol", "INVALID");
	ASSERT_TRUE(fconf.save(tempEyelockIni));
	runConfigurator();
	assertWiegand();
	fconf.load(tempEyelockIni);
	ASSERT_TRUE(fconf.has("HighLevel.Protocol"));
}

void EyelockConfiguratorTest::assertIrisOnly()
{
	EyelockLog(logger, TRACE, "Asserting IRIS_ONLY configuration");
	FileConfiguration conf(eyelockIni);
	EXPECT_EQ(false, conf.getBool("GRITrigger.DualAuthenticationMode", true));
	EXPECT_EQ(false, conf.getBool("GRITrigger.PassThroughMode", true));
}

void EyelockConfiguratorTest::assertIrisOrCard()
{
	EyelockLog(logger, TRACE, "Asserting IRIS_OR_CARD configuration");
	FileConfiguration conf(eyelockIni);
	EXPECT_EQ(false, conf.getBool("GRITrigger.DualAuthenticationMode", true));
	EXPECT_EQ(true, conf.getBool("GRITrigger.PassThroughMode", false));
}

void EyelockConfiguratorTest::assertIrisAndCard()
{
	EyelockLog(logger, TRACE, "Asserting IRIS_AND_CARD configuration");
	FileConfiguration conf(eyelockIni);
	EXPECT_EQ(true, conf.getBool("GRITrigger.DualAuthenticationMode", false));
	EXPECT_EQ(true, conf.getBool("GRITrigger.PassThroughMode", false));
}


TEST_F(EyelockConfiguratorTest, Highlevel_AuthScheme)
{
	ofstream fs(tempEyelockIni.c_str());
	if (fs.good())
	{
		fs << "# test auth scheme setting" << endl;
	}
	fs.close();

	FileConfiguration fconf(tempEyelockIni);

	// assertIrisOnly(); // no GRITrigger.PassThroughMode parameter in default Eyelock.ini
	EyelockLog(logger, TRACE, "Asserting no changes to default configuration");
	fconf.load(eyelockIni);
	EXPECT_EQ(false, fconf.getBool("GRITrigger.DualAuthenticationMode", true));
	ASSERT_FALSE(fconf.has("GRITrigger.PassThroughMode"));

	fconf.setValue("HighLevel.AuthScheme", "LABUDA");
	ASSERT_TRUE(fconf.save(tempEyelockIni));
	runConfigurator();

	EyelockLog(logger, TRACE, "Asserting no changes to default configuration after attempt to set invalid value");
	assertIrisOnly(); // GRITrigger.PassThroughMode parameter is set in "ensure_" method
	/*fconf.load(eyelockIni);
	EXPECT_EQ(false, fconf.getBool("GRITrigger.DualAuthenticationMode", true));
	ASSERT_FALSE(fconf.has("GRITrigger.PassThroughMode"));*/

	fconf.load(tempEyelockIni);
	ASSERT_TRUE(fconf.has("HighLevel.AuthScheme"));

	fconf.setValue("HighLevel.AuthScheme", "IRIS_OR_CARD");
	ASSERT_TRUE(fconf.save(tempEyelockIni));
	runConfigurator();
	assertIrisOrCard();
	fconf.load(tempEyelockIni);
	ASSERT_FALSE(fconf.has("HighLevel.AuthScheme"));

	fconf.setValue("HighLevel.AuthScheme", "IRIS_AND_CARD");
	ASSERT_TRUE(fconf.save(tempEyelockIni));
	runConfigurator();
	assertIrisAndCard();
	fconf.load(tempEyelockIni);
	ASSERT_FALSE(fconf.has("HighLevel.AuthScheme"));

	fconf.setValue("HighLevel.AuthScheme", "IRIS_ONLY");
	ASSERT_TRUE(fconf.save(tempEyelockIni));
	runConfigurator();
	assertIrisOnly();
	fconf.load(tempEyelockIni);
	ASSERT_FALSE(fconf.has("HighLevel.AuthScheme"));
}

