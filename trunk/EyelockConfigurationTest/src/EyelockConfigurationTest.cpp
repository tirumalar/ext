/*
 * EyelockConfigurationTest.cpp
 *
 *  Created on: Apr 12, 2017
 *      Author: builder
 */

#include <string>
#include <vector>
#include <algorithm>
#include <fstream>
#include <climits>

#include "gtest/gtest.h"

#include "EyelockConfigurationTest.h"
#include "EyelockConfiguration.h"

using namespace std;
using namespace EyelockConfigurationNS;

const string EyelockConfigurationTest::tempEyelockIni = "temp.Eyelock.ini";

void EyelockConfigurationTest::SetUp()
{
	ifstream ifs(tempEyelockIni.c_str());
	if (ifs.good())
	{
		ifs.close();
		remove(tempEyelockIni.c_str());
	}
}

TEST_F(EyelockConfigurationTest, LoadDefaultFile)
{
	string defaultConfigFile = "/home/default/Eyelock.ini";

	EyelockConfiguration conf(defaultConfigFile);

	EXPECT_EQ((unsigned int)4000, conf.getGRI_RepeatAuthorizationPeriod());
}

TEST_F(EyelockConfigurationTest, UpdateValid)
{
	FileConfiguration conf;
	int AuthorizationToneVolumeNewValue = 50;
	bool DualMatcherPolicyNewValue = true;
	conf.setValue("GRI.AuthorizationToneVolume", AuthorizationToneVolumeNewValue);
	conf.setValue("Eyelock.DualMatcherPolicy", DualMatcherPolicyNewValue);

	conf.save(tempEyelockIni);

	string elkIniFile = "Eyelock.ini";
	EyelockConfiguration elkConf(elkIniFile);

	//EXPECT_EQ(40, elkConf.getGRI_AuthorizationToneVolume()) << "Initial value for AuthorizationToneVolume was set incorrectly";
	//EXPECT_EQ(false, elkConf.getEyelock_DualMatcherPolicy()) << "Initial value for DualMatcherPolicy was set incorrectly";

	elkConf.updateFromFile(tempEyelockIni);

	EXPECT_EQ(AuthorizationToneVolumeNewValue, elkConf.getGRI_AuthorizationToneVolume()) << "Failed to update value for AuthorizationToneVolume";
	EXPECT_EQ(DualMatcherPolicyNewValue, elkConf.getEyelock_DualMatcherPolicy()) << "Failed to update value for DualMatcherPolicy";

	FileConfiguration postUpdateConf(tempEyelockIni);
	AbstractConfiguration::Keys postUpdateKeys;

	postUpdateConf.keys(postUpdateKeys);
	EXPECT_TRUE(postUpdateKeys.empty()) << "Not all keys were deleted after successful update";
}


TEST_F(EyelockConfigurationTest, UpdateInvalidValueUnknownKey)
{
	FileConfiguration conf;
	int AuthorizationToneVolumeNewValue = INT_MAX;
	bool DualMatcherPolicyNewValue = true;
	conf.setValue("GRI.AuthorizationToneVolume", AuthorizationToneVolumeNewValue);
	conf.setValue("Eyelock.DualMatcherPolicy", DualMatcherPolicyNewValue);
	conf.setValue("Eyelock.Unknown", "testUnknown");

	conf.save(tempEyelockIni);

	string elkIniFile = "Eyelock.ini";
	EyelockConfiguration elkConf(elkIniFile);

	//EXPECT_EQ(40, elkConf.getGRI_AuthorizationToneVolume()) << "Initial value for AuthorizationToneVolume was set incorrectly";
	//EXPECT_EQ(false, elkConf.getEyelock_DualMatcherPolicy()) << "Initial value for DualMatcherPolicy was set incorrectly";

	elkConf.updateFromFile(tempEyelockIni);

	EXPECT_EQ(40, elkConf.getGRI_AuthorizationToneVolume()) << "Incorrect updated value for AuthorizationToneVolume";
	EXPECT_EQ(DualMatcherPolicyNewValue, elkConf.getEyelock_DualMatcherPolicy()) << "Failed to update value for DualMatcherPolicy";

	FileConfiguration postUpdateConf(tempEyelockIni);
	AbstractConfiguration::Keys postUpdateKeys;

	postUpdateConf.keys(postUpdateKeys);
	EXPECT_TRUE(find(postUpdateKeys.begin(), postUpdateKeys.end(), "Eyelock.DualMatcherPolicy") == postUpdateKeys.end()) << "Not all keys were deleted after successful update";
	EXPECT_TRUE(find(postUpdateKeys.begin(), postUpdateKeys.end(), "GRI.AuthorizationToneVolume") != postUpdateKeys.end()) << "Missing key with invalid value after unsuccessful update";
	EXPECT_TRUE(find(postUpdateKeys.begin(), postUpdateKeys.end(), "Eyelock.Unknown") != postUpdateKeys.end()) << "Missing unknown key after unsuccessful update";
}

TEST_F(EyelockConfigurationTest, Highlevel_NetworkMatcher)
{
	string nwmini = tempEyelockIni;
	ofstream fs(nwmini.c_str());
	if (fs.good())
	{
		fs << "# test network matcher setting" << endl;
	}
	fs.close();

	EyelockConfiguration elkConf(nwmini);

	ASSERT_TRUE(elkConf.getNetworkMatcherAddress().empty()) << "Incorrect default value for network matcher address";
	ASSERT_FALSE(elkConf.isNetworkMatcherEnabled()) << "Incorrect default value for network matcher enable";

	ASSERT_TRUE(elkConf.save()) << "Failed to save ini file";

	FileConfiguration fconf(nwmini);
	ASSERT_EQ(1, fconf.getInt("GRI.HDMatcherID"));
	ASSERT_EQ(2, fconf.getInt("GRI.HDMatcherCount"));
	ASSERT_FALSE(fconf.has("GRI.HDMatcher.2.Type"));
	ASSERT_FALSE(fconf.has("GRI.HDMatcher.2.BuffSize"));
	ASSERT_FALSE(fconf.has("GRI.HDMatcher.2.Address"));
	ASSERT_TRUE(fconf.getStr("HighLevel.NetworkMatcherAddress").empty());
	ASSERT_FALSE(fconf.getBool("HighLevel.NetworkMatcherEnabled"));

	string nwMatcher = "192.168.113.28:6565";

	ASSERT_FALSE(elkConf.enableNetworkMatcher(true)) << "Must not allow to enable NWM when destination is not set";

	ASSERT_TRUE(elkConf.setNetworkMatcherAddress(nwMatcher)) << "Failed to set valid network matcher";

	ASSERT_TRUE(elkConf.getNetworkMatcherAddress() == nwMatcher) << "Network matcher was set incorrectly";

	ASSERT_TRUE(elkConf.enableNetworkMatcher(true)) << "Must allow to enable NWM when destination is set";

	ASSERT_TRUE(elkConf.save()) << "Failed to save ini file";

	fconf.load(nwmini);
	ASSERT_EQ(2, fconf.getInt("GRI.HDMatcherID"));
	ASSERT_EQ(3, fconf.getInt("GRI.HDMatcherCount"));
	ASSERT_EQ("PCMATCHER", fconf.getStr("GRI.HDMatcher.2.Type"));
	ASSERT_EQ(0, fconf.getInt("GRI.HDMatcher.2.BuffSize"));
	ASSERT_EQ(nwMatcher, fconf.getStr("GRI.HDMatcher.2.Address"));

	ASSERT_EQ(nwMatcher, fconf.getStr("HighLevel.NetworkMatcherAddress"));
	ASSERT_TRUE(fconf.getBool("HighLevel.NetworkMatcherEnabled"));

	ASSERT_TRUE(elkConf.enableNetworkMatcher(false)) << "Failed to disable network matcher";

	ASSERT_TRUE(elkConf.save()) << "Failed to save ini file";

	ASSERT_FALSE(elkConf.getNetworkMatcherAddress().empty()) << "Network matcher endpoint was lost";

	fconf.load(nwmini);
	ASSERT_EQ(1, fconf.getInt("GRI.HDMatcherID"));
	ASSERT_EQ(2, fconf.getInt("GRI.HDMatcherCount"));
	ASSERT_FALSE(fconf.has("GRI.HDMatcher.2.Type"));
	ASSERT_FALSE(fconf.has("GRI.HDMatcher.2.BuffSize"));
	ASSERT_FALSE(fconf.has("GRI.HDMatcher.2.Address"));

	ASSERT_EQ(nwMatcher, fconf.getStr("HighLevel.NetworkMatcherAddress"));
	ASSERT_FALSE(fconf.getBool("HighLevel.NetworkMatcherEnabled"));
}

TEST_F(EyelockConfigurationTest, Highlevel_Protocol)
{
	ofstream fs(tempEyelockIni.c_str());
	if (fs.good())
	{
		fs << "# test protocol setting" << endl;
	}
	fs.close();

	EyelockConfiguration elkConf(tempEyelockIni);

	//EyelockConfiguration::Protocol cmpResult = EyelockConfiguration::UNKNOWN_PROTOCOL;
	ASSERT_EQ(EyelockConfiguration::WIEGAND, elkConf.getProtocol()) << "Incorrect default value for protocol";

	EyelockConfiguration::Protocol prot = EyelockConfiguration::PAC;
	ASSERT_TRUE(elkConf.setProtocol(prot)) << "Failed to set valid protocol";

	ASSERT_TRUE(elkConf.getProtocol() == prot) << "Protocol was set incorrectly";

	ASSERT_TRUE(elkConf.save()) << "Failed to save ini file";

	FileConfiguration fconf(tempEyelockIni);
	ASSERT_EQ(false, fconf.getBool("GRITrigger.OSDPEnable", true));
	ASSERT_EQ(false, fconf.getBool("GRITrigger.WeigandEnable", true));
	ASSERT_EQ(false, fconf.getBool("GRITrigger.WeigandHidEnable", true));
	ASSERT_EQ(true, fconf.getBool("GRITrigger.PACEnable"));
	ASSERT_EQ(false, fconf.getBool("GRITrigger.F2FEnable", true));


	prot = EyelockConfiguration::OSDP;
	ASSERT_TRUE(elkConf.setProtocol(prot)) << "Failed to set valid protocol";

	ASSERT_TRUE(elkConf.getProtocol() == prot) << "Protocol was set incorrectly";

	ASSERT_TRUE(elkConf.save()) << "Failed to save ini file";

	FileConfiguration fconf2(tempEyelockIni);
	ASSERT_EQ(true, fconf2.getBool("GRITrigger.OSDPEnable"));
	ASSERT_EQ(false, fconf2.getBool("GRITrigger.WeigandEnable", true));
	ASSERT_EQ(false, fconf2.getBool("GRITrigger.WeigandHidEnable", true));
	ASSERT_EQ(false, fconf2.getBool("GRITrigger.PACEnable", true));
	ASSERT_EQ(false, fconf2.getBool("GRITrigger.F2FEnable"));

}


TEST_F(EyelockConfigurationTest, Highlevel_AuthScheme)
{
	ofstream fs(tempEyelockIni.c_str());
	if (fs.good())
	{
		fs << "# test auth scheme setting" << endl;
	}
	fs.close();

	EyelockConfiguration elkConf(tempEyelockIni);

	ASSERT_EQ(EyelockConfiguration::IRIS_ONLY, elkConf.getAuthScheme()) << "Incorrect default value for AuthScheme";

	ASSERT_TRUE(elkConf.save()) << "Failed to save ini file";

	FileConfiguration fconf(tempEyelockIni);
	ASSERT_EQ(false, fconf.getBool("GRITrigger.DualAuthenticationMode", true));
	ASSERT_EQ(false, fconf.getBool("GRITrigger.PassThroughMode", true));

	EyelockConfiguration::AuthScheme sch = EyelockConfiguration::IRIS_OR_CARD;

	ASSERT_TRUE(elkConf.setAuthScheme(sch)) << "Failed to set valid AuthScheme";

	ASSERT_TRUE(elkConf.getAuthScheme() == sch) << "AuthScheme was set incorrectly";

	ASSERT_TRUE(elkConf.save()) << "Failed to save ini file";

	FileConfiguration fconf2(tempEyelockIni);
	ASSERT_EQ(false, fconf2.getBool("GRITrigger.DualAuthenticationMode", true));
	ASSERT_EQ(true, fconf2.getBool("GRITrigger.PassThroughMode", false));

	sch = EyelockConfiguration::IRIS_AND_CARD;

	ASSERT_TRUE(elkConf.setAuthScheme(sch)) << "Failed to set valid AuthScheme";

	ASSERT_TRUE(elkConf.getAuthScheme() == sch) << "AuthScheme was set incorrectly";

	ASSERT_TRUE(elkConf.save()) << "Failed to save ini file";

	FileConfiguration fconf3(tempEyelockIni);
	ASSERT_EQ(true, fconf3.getBool("GRITrigger.DualAuthenticationMode", false));
	ASSERT_EQ(true, fconf3.getBool("GRITrigger.PassThroughMode", false));

}
