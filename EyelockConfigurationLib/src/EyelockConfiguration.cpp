/*
 * EyelockConfiguration.cpp
 *
 *  Created on: Apr 11, 2017
 *      Author: builder
 */

#include <typeinfo>
#include <climits>
#include <map>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <fcntl.h>

#include "EyelockConfiguration.h"
#include "AbstractConfiguration.h"
#include "ParameterValidator.h"
#include "NumberParser.h"
#include "NumberFormatter.h"
#include "ELKNS.h"

#include "logging.h"

const char logger[30] = "EyelockConfiguration";

using namespace std;

namespace EyelockConfigurationNS
{
	const string EyelockConfiguration::eyelockIniFile = string("/home/root/Eyelock.ini");
	const string EyelockConfiguration::tempIniFile = string("/home/root/Eyelock.ini.temp");
	const string EyelockConfiguration::eyelockDefaultIniFile = string("/home/default/Eyelock.ini");

	EyelockConfiguration::EyelockConfiguration()
	{
		addDefaultsForRequired();

		setSdkAliases();
	}

	EyelockConfiguration::EyelockConfiguration(string filepath): filename(filepath)
	{
		loadFromFile(filename);

		setSdkAliases();
	}

	void EyelockConfiguration::loadFromFile(string filepath)
	{
		EyelockLog(logger, DEBUG, "Loading from file: %s", filepath.c_str());
		if (acquireLock())
		{
			conf.load(filename);
			releaseLock();
		}
		else
		{
			EyelockLog(logger, ERROR, "Failed to acquire lock for %s", filename.c_str());
		}

		addDefaultsForRequired();
	}

	void EyelockConfiguration::addDefaultsForRequired()
	{
		ensureGRI_AuthorizationToneVolume();
		ensureProtocol();
		ensureAuthScheme();
		ensureNetworkMatcher();

		// TODO: put appropriate methods for all required parameters
	}

	bool EyelockConfiguration::reset()
	{
		// TODO: replace ini-file with default?
		// don't forget about lock

		if (conf.load(eyelockDefaultIniFile))
		{
			addDefaultsForRequired();
			return true;
		}
		return false;
	}

	int EyelockConfiguration::handleLock(short int lType)
	{
		if (lType != F_UNLCK && lType != F_WRLCK)
		{
			return 0;
		}

		if (filename.empty())
		{
			return 0;
		}

		char lockName[100];
		sprintf(lockName, "%s.lock", filename.c_str());

		struct flock fl;

		fl.l_type = lType;
		fl.l_whence = SEEK_SET;
		fl.l_start = 0;
		fl.l_len = 1;

		int fdlock;
		if ((fdlock = open(lockName, O_WRONLY|O_CREAT, 0666)) == -1)
		{
			return 0;
		}

		if (fcntl(fdlock, F_SETLK, &fl) == -1)
		{
			return 0;
		}

		return 1;
	}

	int EyelockConfiguration::acquireLock()
	{
		return handleLock(F_WRLCK);
	}

	int EyelockConfiguration::releaseLock()
	{
		return handleLock(F_UNLCK);
	}


	void EyelockConfiguration::setSdkAliases()
	{
		floatParameters.push_back(FloatParamInfo("CONFIG_SPEAKERVOLUME", &EyelockConfiguration::setGRI_AuthorizationToneVolume, &EyelockConfiguration::getGRI_AuthorizationToneVolume));
		floatParameters.push_back(FloatParamInfo("CONFIG_LEDBRIGHTNESS", &EyelockConfiguration::setGRI_LEDBrightness, &EyelockConfiguration::getGRI_LEDBrightness));
		floatParameters.push_back(FloatParamInfo("CONFIG_TAMPERTONEVOLUME", &EyelockConfiguration::setGRI_TamperToneVolume, &EyelockConfiguration::getGRI_TamperToneVolume));

		uintParameters.push_back(UIntParamInfo("CONFIG_REPEATAUTHORIZATIONPERIOD", &EyelockConfiguration::setGRI_RepeatAuthorizationPeriod, &EyelockConfiguration::getGRI_RepeatAuthorizationPeriod));
		uintParameters.push_back(UIntParamInfo("CONFIG_OSDPADDRESS", &EyelockConfiguration::setEyelock_OSDPAddress, &EyelockConfiguration::getEyelock_OSDPAddress));
		uintParameters.push_back(UIntParamInfo("CONFIG_OSDPBAUDRATE", &EyelockConfiguration::setEyelock_OSDPBaudRate, &EyelockConfiguration::getEyelock_OSDPBaudRate));

		uintParameters.push_back(UIntParamInfo("CONFIG_NEGATIVEMATCHTIMEOUT", &EyelockConfiguration::setEyelock_NegativeMatchTimeout, &EyelockConfiguration::getEyelock_NegativeMatchTimeout));
		uintParameters.push_back(UIntParamInfo("CONFIG_NEGATIVEMATCHRESETTIMER", &EyelockConfiguration::setEyelock_NegativeMatchResetTimer, &EyelockConfiguration::getEyelock_NegativeMatchResetTimer));
		uintParameters.push_back(UIntParamInfo("CONFIG_GRANTRELAYTIME", &EyelockConfiguration::setGRITrigger_RelayTimeInMS, &EyelockConfiguration::getGRITrigger_RelayTimeInMS));
		uintParameters.push_back(UIntParamInfo("CONFIG_DENYRELAYTIME", &EyelockConfiguration::setGRITrigger_DenyRelayTimeInMS, &EyelockConfiguration::getGRITrigger_DenyRelayTimeInMS));
		uintParameters.push_back(UIntParamInfo("CONFIG_DUALAUTHNCARDMATCHWAITIRISTIME", &EyelockConfiguration::setGRITrigger_DualAuthNCardMatchWaitIrisTime, &EyelockConfiguration::getGRITrigger_DualAuthNCardMatchWaitIrisTime));
		uintParameters.push_back(UIntParamInfo("CONFIG_AUTH_SCHEME", &EyelockConfiguration::setAuthScheme, &EyelockConfiguration::getAuthSchemeUInt));
		uintParameters.push_back(UIntParamInfo("CONFIG_PINWAITTIME", &EyelockConfiguration::setEyelock_WaitPinTime, &EyelockConfiguration::getEyelock_WaitPinTime));
		uintParameters.push_back(UIntParamInfo("CONFIG_PINBURSTBITS", &EyelockConfiguration::setEyelock_PinBurstBits, &EyelockConfiguration::getEyelock_PinBurstBits));

		//floatParameters.push_back(FloatParamInfo("CONFIG", &EyelockConfiguration::set, &EyelockConfiguration::get));

		boolParameters.push_back(BoolParamInfo("CONFIG_TAMPERSIGNALHIGH", &EyelockConfiguration::setTamperSignalHigh, &EyelockConfiguration::getTamperSignalHigh));
		boolParameters.push_back(BoolParamInfo("CONFIG_DUALMATCHERPOLICY", &EyelockConfiguration::setEyelock_DualMatcherPolicy, &EyelockConfiguration::getEyelock_DualMatcherPolicy));
		boolParameters.push_back(BoolParamInfo("CONFIG_LEDCONTROLLEDBYACS", &EyelockConfiguration::setGRITrigger_DualAuthNLedControlledByACS, &EyelockConfiguration::getGRITrigger_DualAuthNLedControlledByACS));
		boolParameters.push_back(BoolParamInfo("CONFIG_DUALAUTHENTICATIONPARITY", &EyelockConfiguration::setGRITrigger_DualAuthenticationParity, &EyelockConfiguration::getGRITrigger_DualAuthenticationParity));
		boolParameters.push_back(BoolParamInfo("CONFIG_ENABLENEGATIVEMATCHTIMEOUT", &EyelockConfiguration::setEyelock_EnableNegativeMatchTimeout, &EyelockConfiguration::getEyelock_EnableNegativeMatchTimeout));
		boolParameters.push_back(BoolParamInfo("CONFIG_RELAYENABLE", &EyelockConfiguration::setGRITrigger_EnableRelayWithSignal, &EyelockConfiguration::getGRITrigger_EnableRelayWithSignal));
		boolParameters.push_back(BoolParamInfo("CONFIG_FORCETLS12", &EyelockConfiguration::setEyelock_TLSEnable, &EyelockConfiguration::getEyelock_TLSEnable));
		boolParameters.push_back(BoolParamInfo("CONFIG_TAMPEROUTSIGNALHIGH", &EyelockConfiguration::setTamperOutSignalHigh, &EyelockConfiguration::getTamperOutSignalHigh));

		boolParameters.push_back(BoolParamInfo("CONFIG_DUALAUTHENTICATIONMODE", &EyelockConfiguration::setGRITrigger_DualAuthenticationMode, &EyelockConfiguration::getGRITrigger_DualAuthenticationMode));
		boolParameters.push_back(BoolParamInfo("CONFIG_PASSTHROUGHMODE", &EyelockConfiguration::setGRITrigger_PassThroughMode, &EyelockConfiguration::getGRITrigger_PassThroughMode));

		boolParameters.push_back(BoolParamInfo("CONFIG_NWMATCHER_ENABLED", &EyelockConfiguration::enableNetworkMatcher, &EyelockConfiguration::isNetworkMatcherEnabled));
		strParameters.push_back(StrParamInfo("CONFIG_NWMATCHER_ADDRESS", &EyelockConfiguration::setNetworkMatcherAddress, &EyelockConfiguration::getNetworkMatcherAddress));

		strParameters.push_back(StrParamInfo("CONFIG_AUTH_SCHEME", &EyelockConfiguration::setAuthScheme, &EyelockConfiguration::getAuthSchemeStr));
		strParameters.push_back(StrParamInfo("CONFIG_PROTOCOL", &EyelockConfiguration::setProtocol, &EyelockConfiguration::getProtocolStr));

		setLegacyAliases();
	}

	void EyelockConfiguration::setLegacyAliases()
	{
		// aliases for legacy implementation

		intAliases.insert(pair<string, int32_t> ("CONFIG_SPEAKERVOLUME",CONFIG_SPEAKERVOLUME));
		intAliases.insert(pair<string, int32_t> ("CONFIG_LEDBRIGHTNESS",CONFIG_LEDBRIGHTNESS));
		intAliases.insert(pair<string, int32_t> ("CONFIG_TAMPERTONEVOLUME",CONFIG_TAMPERTONEVOLUME));

		intAliases.insert(pair<string, int32_t> ("CONFIG_REPEATAUTHORIZATIONPERIOD",CONFIG_REPEATAUTHORIZATIONPERIOD));
		intAliases.insert(pair<string, int32_t> ("CONFIG_OSDPADDRESS",CONFIG_OSDPADDRESS));
		intAliases.insert(pair<string, int32_t> ("CONFIG_OSDPBAUDRATE",CONFIG_OSDPBAUDRATE));

		intAliases.insert(pair<string, int32_t> ("CONFIG_NEGATIVEMATCHTIMEOUT",CONFIG_NEGATIVEMATCHTIMEOUT));
		intAliases.insert(pair<string, int32_t> ("CONFIG_NEGATIVEMATCHRESETTIMER",CONFIG_NEGATIVEMATCHRESETTIMER));
		intAliases.insert(pair<string, int32_t> ("CONFIG_GRANTRELAYTIME",CONFIG_GRANTRELAYTIME));
		intAliases.insert(pair<string, int32_t> ("CONFIG_DENYRELAYTIME",CONFIG_DENYRELAYTIME));
		intAliases.insert(pair<string, int32_t> ("CONFIG_DUALAUTHNCARDMATCHWAITIRISTIME",CONFIG_DUALAUTHNCARDMATCHWAITIRISTIME));

		intAliases.insert(pair<string, int32_t> ("CONFIG_TAMPERSIGNALHIGH",CONFIG_TAMPERSIGNALHIGH));
		intAliases.insert(pair<string, int32_t> ("CONFIG_DUALMATCHERPOLICY",CONFIG_DUALMATCHERPOLICY));
		intAliases.insert(pair<string, int32_t> ("CONFIG_LEDCONTROLLEDBYACS",CONFIG_LEDCONTROLLEDBYACS));
		intAliases.insert(pair<string, int32_t> ("CONFIG_DUALAUTHENTICATIONPARITY",CONFIG_DUALAUTHENTICATIONPARITY));
		intAliases.insert(pair<string, int32_t> ("CONFIG_ENABLENEGATIVEMATCHTIMEOUT",CONFIG_ENABLENEGATIVEMATCHTIMEOUT));
		intAliases.insert(pair<string, int32_t> ("CONFIG_RELAYENABLE",CONFIG_RELAYENABLE));
		intAliases.insert(pair<string, int32_t> ("CONFIG_FORCETLS12",CONFIG_FORCETLS12));
		intAliases.insert(pair<string, int32_t> ("CONFIG_TAMPEROUTSIGNALHIGH",CONFIG_TAMPEROUTSIGNALHIGH));

		intAliases.insert(pair<string, int32_t> ("CONFIG_DUALAUTHENTICATIONMODE",CONFIG_DUALAUTHENTICATIONMODE));
		intAliases.insert(pair<string, int32_t> ("CONFIG_PASSTHROUGHMODE",CONFIG_PASSTHROUGHMODE));

		intAliases.insert(pair<string, int32_t> ("CONFIG_HDMATCHER_ADDRESS",CONFIG_HDMATCHER_ADDRESS));

		intAliases.insert(pair<string, int32_t> ("CONFIG_PROTOCOL",CONFIG_PROTOCOL));
	}


	int EyelockConfiguration::setUIntParameter(const string& sdkParameterName, const unsigned int value)
	{
		EyelockLog(logger, TRACE, "Setting uint parameter: %s = %d", sdkParameterName.c_str(), value);
		vector<UIntParamInfo>::iterator iter = find(uintParameters.begin(), uintParameters.end(), UIntParamInfo(sdkParameterName, NULL, NULL));
		if (iter != uintParameters.end())
		{
			return ((*(this).*(iter->pSetter)) (value)) ? SUCCESS : INVALID_INPUT_DATA;
		}
		else
		{
			return NOT_SUPPORTED;
		}
	}

	int EyelockConfiguration::getUIntParameter(const string& sdkParameterName, unsigned int& value)
	{
		EyelockLog(logger, TRACE, "Getting uint parameter: %s", sdkParameterName.c_str());
		vector<UIntParamInfo>::iterator iter = find(uintParameters.begin(), uintParameters.end(), UIntParamInfo(sdkParameterName, NULL, NULL));
		if (iter != uintParameters.end())
		{
			value = (*(this).*(iter->pGetter)) ();
			return SUCCESS;
		}
		else
		{
			return NOT_SUPPORTED;
		}

		return 0;
	}


	int EyelockConfiguration::setFloatParameter(const string& sdkParameterName, const float value)
	{
		EyelockLog(logger, TRACE, "Setting float parameter: %s = %f", sdkParameterName.c_str(), value);
		vector<FloatParamInfo>::iterator iter = find(floatParameters.begin(), floatParameters.end(), FloatParamInfo(sdkParameterName, NULL, NULL));
		if (iter != floatParameters.end())
		{
			return ((*(this).*(iter->pSetter)) (value)) ? SUCCESS : INVALID_INPUT_DATA;
		}
		else
		{
			return NOT_SUPPORTED;
		}
	}

	int EyelockConfiguration::getFloatParameter(const string& sdkParameterName, float& value)
	{
		EyelockLog(logger, TRACE, "Getting float parameter: %s", sdkParameterName.c_str());
		vector<FloatParamInfo>::iterator iter = find(floatParameters.begin(), floatParameters.end(), FloatParamInfo(sdkParameterName, NULL, NULL));
		if (iter != floatParameters.end())
		{
			value = (*(this).*(iter->pGetter)) ();
			return SUCCESS;
		}
		else
		{
			return NOT_SUPPORTED;
		}

		return 0;
	}


	int EyelockConfiguration::setBoolParameter(const string& sdkParameterName, const bool value)
	{
		EyelockLog(logger, TRACE, "Setting bool parameter: %s = %d", sdkParameterName.c_str(), value);
		vector<BoolParamInfo>::iterator iter = find(boolParameters.begin(), boolParameters.end(), BoolParamInfo(sdkParameterName, NULL, NULL));
		if (iter != boolParameters.end())
		{
			return ((*(this).*(iter->pSetter)) (value)) ? SUCCESS : INVALID_INPUT_DATA;
		}
		else
		{
			return NOT_SUPPORTED;
		}
	}

	int EyelockConfiguration::getBoolParameter(const string& sdkParameterName, bool& value)
	{
		EyelockLog(logger, TRACE, "Getting bool parameter: %s", sdkParameterName.c_str());
		vector<BoolParamInfo>::iterator iter = find(boolParameters.begin(), boolParameters.end(), BoolParamInfo(sdkParameterName, NULL, NULL));
		if (iter != boolParameters.end())
		{
			value = (*(this).*(iter->pGetter)) ();
			return SUCCESS;
		}
		else
		{
			return NOT_SUPPORTED;
		}

		return 0;
	}


	int EyelockConfiguration::setStrParameter(const string& sdkParameterName, const string& value)
	{
		EyelockLog(logger, TRACE, "Setting string parameter: %s = %s", sdkParameterName.c_str(), value.c_str());
		vector<StrParamInfo>::iterator iter = find(strParameters.begin(), strParameters.end(), StrParamInfo(sdkParameterName, NULL, NULL));
		if (iter != strParameters.end())
		{
			return ((*(this).*(iter->pSetter)) (value)) ? SUCCESS : INVALID_INPUT_DATA;
		}
		else
		{
			return NOT_SUPPORTED;
		}
	}

	int EyelockConfiguration::getStrParameter(const string& sdkParameterName, string& value)
	{
		EyelockLog(logger, TRACE, "Getting string parameter: %s", sdkParameterName.c_str());
		vector<StrParamInfo>::iterator iter = find(strParameters.begin(), strParameters.end(), StrParamInfo(sdkParameterName, NULL, NULL));
		if (iter != strParameters.end())
		{
			value = (*(this).*(iter->pGetter)) ();
			return SUCCESS;
		}
		else
		{
			return NOT_SUPPORTED;
		}

		return 0;
	}


	int EyelockConfiguration::setParameter(const std::string& sdkParameterName, const std::string& value)
	{
		EyelockLog(logger, DEBUG, "Setting SDK parameter %s = %s", sdkParameterName.c_str(), value.c_str());
		vector<FloatParamInfo>::iterator iterFloat = find(floatParameters.begin(), floatParameters.end(), FloatParamInfo(sdkParameterName, NULL, NULL));
		if (iterFloat != floatParameters.end())
		{
			float convertedValue = 0.0;
			if (NumberParser::tryParseFloat(value, convertedValue))
			{
				return ((*(this).*(iterFloat->pSetter)) (convertedValue)) ? SUCCESS : INVALID_INPUT_DATA;
			}
			else
			{
				return INVALID_INPUT_DATA;
			}
		}

		vector<UIntParamInfo>::iterator iterUInt = find(uintParameters.begin(), uintParameters.end(), UIntParamInfo(sdkParameterName, NULL, NULL));
		if (iterUInt != uintParameters.end())
		{
			unsigned int convertedValue = 0;
			if (NumberParser::tryParseUnsigned(value, convertedValue))
			{
				return ((*(this).*(iterUInt->pSetter)) (convertedValue)) ? SUCCESS : INVALID_INPUT_DATA;
			}
			else
			{
				return INVALID_INPUT_DATA;
			}
		}

		vector<BoolParamInfo>::iterator iterBool = find(boolParameters.begin(), boolParameters.end(), BoolParamInfo(sdkParameterName, NULL, NULL));
		if (iterBool != boolParameters.end())
		{
			bool convertedValue = false;
			if (NumberParser::tryParseBool(value, convertedValue))
			{
				return ((*(this).*(iterBool->pSetter)) (convertedValue)) ? SUCCESS : INVALID_INPUT_DATA;
			}
			else
			{
				return INVALID_INPUT_DATA;
			}
		}

		vector<StrParamInfo>::iterator iterStr = find(strParameters.begin(), strParameters.end(), StrParamInfo(sdkParameterName, NULL, NULL));
		if (iterStr != strParameters.end())
		{
			return ((*(this).*(iterStr->pSetter)) (value)) ? SUCCESS : INVALID_INPUT_DATA;
		}

		if (sdkParameterName == "CONFIG_HDMATCHER_ADDRESS")
		{
			if (value == "FALSE" || value == "false")
			{
				return (enableNetworkMatcher(false)) ? SUCCESS : UNSUCCESSFUL;
			}
			else
			{
				if (setNetworkMatcherAddress(value))
				{
					if (enableNetworkMatcher(true))
					{
						return SUCCESS;
					}
				}
				return INVALID_INPUT_DATA;
			}
		}

		return NOT_SUPPORTED;
	}

	int EyelockConfiguration::setParameter(int32_t intAlias, const std::string& value)
	{
		EyelockLog(logger, DEBUG, "Setting SDK parameter: %d", intAlias);
		for (map<string, int32_t>::iterator intAliasIter = intAliases.begin(); intAliasIter != intAliases.end(); intAliasIter++)
		{
			if (intAliasIter->second == intAlias)
			{
				return setParameter(intAliasIter->first, value);
			}
		}
		return NOT_SUPPORTED;
	}

	int EyelockConfiguration::setParametersMap(const map<int32_t, string> & inputParametersMap)
	{
		EyelockLog(logger, DEBUG, "Setting SDK parameters map");
		map<int32_t, string>::const_iterator inpMapIter;
		for (inpMapIter = inputParametersMap.begin(); inpMapIter != inputParametersMap.end(); ++inpMapIter)
		{
			int setResult = setParameter(inpMapIter->first, inpMapIter->second);
			if (setResult != SUCCESS)
			{
				return setResult;
			}
		}

		if (!save())
		{
			return FILE_ACCESS_ERROR;
		}

		return SUCCESS;
	}

	int EyelockConfiguration::getAllParameters(map<int32_t, string> & outParametersMap)
	{
		EyelockLog(logger, DEBUG, "Getting SDK parameters map");
		for (vector<FloatParamInfo>::iterator typeIter = floatParameters.begin(); typeIter != floatParameters.end(); typeIter++)
		{
			map<string, int32_t>::const_iterator intAliasIter = intAliases.find(typeIter->sdkParameterName);
			if (intAliasIter != intAliases.end())
			{
				float value = (*(this).*(typeIter->pGetter)) ();
				string valueStr = NumberFormatter::format(value);
				outParametersMap.insert(pair<int32_t, std::string> ((int32_t)intAliasIter->second, valueStr));
			}
		}

		for (vector<UIntParamInfo>::iterator typeIter = uintParameters.begin(); typeIter != uintParameters.end(); typeIter++)
		{
			map<string, int32_t>::const_iterator intAliasIter = intAliases.find(typeIter->sdkParameterName);
			if (intAliasIter != intAliases.end())
			{
				unsigned int value = (*(this).*(typeIter->pGetter)) ();
				string valueStr = NumberFormatter::format(value);
				outParametersMap.insert(pair<int32_t, std::string> ((int32_t)intAliasIter->second, valueStr));
			}
		}

		for (vector<BoolParamInfo>::iterator typeIter = boolParameters.begin(); typeIter != boolParameters.end(); typeIter++)
		{
			map<string, int32_t>::const_iterator intAliasIter = intAliases.find(typeIter->sdkParameterName);
			if (intAliasIter != intAliases.end())
			{
				bool value = (*(this).*(typeIter->pGetter)) ();
				string valueStr = NumberFormatter::format(value);
				outParametersMap.insert(pair<int32_t, std::string> ((int32_t)intAliasIter->second, valueStr));
			}
		}

		for (vector<StrParamInfo>::iterator typeIter = strParameters.begin(); typeIter != strParameters.end(); typeIter++)
		{
			map<string, int32_t>::const_iterator intAliasIter = intAliases.find(typeIter->sdkParameterName);
			if (intAliasIter != intAliases.end())
			{
				string value = (*(this).*(typeIter->pGetter)) ();
				outParametersMap.insert(pair<int32_t, std::string> ((int32_t)intAliasIter->second, value));
			}
		}

		string nwMatcherValue = (isNetworkMatcherEnabled()) ? getNetworkMatcherAddress() : "false";
		outParametersMap.insert(pair<int32_t, std::string> ((int32_t)CONFIG_HDMATCHER_ADDRESS, nwMatcherValue));

		bool passThrough, dualAuth;
		getDualAuthAndPassThrough(passThrough, dualAuth);
		outParametersMap.insert(pair<int32_t, std::string> ((int32_t)CONFIG_DUALAUTHENTICATIONMODE, NumberFormatter::format(dualAuth)));
		outParametersMap.insert(pair<int32_t, std::string> ((int32_t)CONFIG_PASSTHROUGHMODE, NumberFormatter::format(passThrough)));

		return NOT_SUPPORTED;
	}


	// ************** Parameters ****************************************************

	// GRI.AuthorizationToneVolume: float, default 40; min 0; max 100; required.
	const float GRI_AuthorizationToneVolumeDefault = 40.0f;

	float EyelockConfiguration::getGRI_AuthorizationToneVolume()
	{
		return conf.getFloat(string("GRI.AuthorizationToneVolume"), GRI_AuthorizationToneVolumeDefault);
	}

	bool EyelockConfiguration::setGRI_AuthorizationToneVolume(float newValue)
	{
		if (!ParameterValidator::validateRange(newValue, 0.0f, 100.0f)) // MIN, MAX
		{
			return false;
		}

		conf.setValue("GRI.AuthorizationToneVolume", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRI_AuthorizationToneVolume()
	{
		if (!conf.has(string("GRI.AuthorizationToneVolume")))
		{
			conf.setValue("GRI.AuthorizationToneVolume", GRI_AuthorizationToneVolumeDefault);
		}
	}

	const float GRI_LEDBrightnessDefault = 80.0f;
	const int GRI_LEDBrightnessMaximumDefault = 128;
	float EyelockConfiguration::getGRI_LEDBrightness()
	{
		int LEDBrightnessMaximum = conf.getInt(string("GRI.LEDBrightnessMaximum"), GRI_LEDBrightnessMaximumDefault);
		
		return conf.getFloat(string("GRI.LEDBrightness"), GRI_LEDBrightnessDefault)/LEDBrightnessMaximum * 100;
	}

	bool EyelockConfiguration::setGRI_LEDBrightness(float newValue)
	{
		if (!ParameterValidator::validateRange(newValue, 0.0f, 100.0f)) // MIN, MAX
		{
			return false;
		}
		
		int LEDBrightnessMaximum = conf.getInt(string("GRI.LEDBrightnessMaximum"), GRI_LEDBrightnessMaximumDefault);

		conf.setValue("GRI.LEDBrightness", newValue/100 * LEDBrightnessMaximum);

		return true;
	}

	void EyelockConfiguration::ensureGRI_LEDBrightness()
	{
		if (!conf.has(string("GRI.LEDBrightness")))
		{
			conf.setValue("GRI.LEDBrightness", GRI_LEDBrightnessDefault);
		}
	}

	const float GRI_TamperToneVolumeDefault = 50.0f;
	float EyelockConfiguration::getGRI_TamperToneVolume()
	{
		return conf.getFloat(string("GRI.TamperToneVolume"), GRI_TamperToneVolumeDefault);
	}

	bool EyelockConfiguration::setGRI_TamperToneVolume(float newValue)
	{
		if (!ParameterValidator::validateRange(newValue, 0.0f, 100.0f))
		{
			return false;
		}

		conf.setValue("GRI.TamperToneVolume", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRI_TamperToneVolume()
	{
		if (!conf.has(string("GRI.TamperToneVolume")))
		{
			conf.setValue("GRI.TamperToneVolume", GRI_TamperToneVolumeDefault);
		}
	}

	const bool Eyelock_TamperSignalHighToLowDefault = true;
	bool EyelockConfiguration::getEyelock_TamperSignalHighToLow()
	{
		return conf.getBool(string("Eyelock.TamperSignalHighToLow"), Eyelock_TamperSignalHighToLowDefault);
	}

	bool EyelockConfiguration::setEyelock_TamperSignalHighToLow(bool newValue)
	{

		conf.setValue("Eyelock.TamperSignalHighToLow", newValue);

		return true;
	}

	void EyelockConfiguration::ensureEyelock_TamperSignalHighToLow()
	{
		if (!conf.has(string("Eyelock.TamperSignalHighToLow")))
		{
			conf.setValue("Eyelock.TamperSignalHighToLow", Eyelock_TamperSignalHighToLowDefault);
		}
	}

	// ***** wrappers for Eyelock.TamperSignalHighToLow parameter used by SDK *******
	bool EyelockConfiguration::getTamperSignalHigh()
	{
		return !getEyelock_TamperSignalHighToLow();
	}

	bool EyelockConfiguration::setTamperSignalHigh(bool newValue)
	{
		return setEyelock_TamperSignalHighToLow(!newValue);
	}

	void EyelockConfiguration::ensureTamperSignalHigh()
	{
		ensureEyelock_TamperSignalHighToLow();
	}
	// ****** ******

	const bool Eyelock_TamperOutSignalHighToLowDefault = false;
	bool EyelockConfiguration::getEyelock_TamperOutSignalHighToLow()
	{
		return conf.getBool(string("Eyelock.TamperOutSignalHighToLow"), Eyelock_TamperOutSignalHighToLowDefault);
	}

	bool EyelockConfiguration::setEyelock_TamperOutSignalHighToLow(bool newValue)
	{

		conf.setValue("Eyelock.TamperOutSignalHighToLow", newValue);

		return true;
	}

	void EyelockConfiguration::ensureEyelock_TamperOutSignalHighToLow()
	{
		if (!conf.has(string("Eyelock.TamperOutSignalHighToLow")))
		{
			conf.setValue("Eyelock.TamperOutSignalHighToLow", Eyelock_TamperOutSignalHighToLowDefault);
		}
	}

	// ***** wrappers for Eyelock.TamperOutSignalHighToLow parameter used by SDK *******
	bool EyelockConfiguration::getTamperOutSignalHigh()
	{
		return !getEyelock_TamperOutSignalHighToLow();
	}

	bool EyelockConfiguration::setTamperOutSignalHigh(bool newValue)
	{
		return setEyelock_TamperOutSignalHighToLow(!newValue);
	}

	void EyelockConfiguration::ensureTamperOutSignalHigh()
	{
		ensureEyelock_TamperOutSignalHighToLow();
	}
	// ****** ******

	const bool Eyelock_DualMatcherPolicyDefault = false;
	bool EyelockConfiguration::getEyelock_DualMatcherPolicy()
	{
		return conf.getBool(string("Eyelock.DualMatcherPolicy"), Eyelock_DualMatcherPolicyDefault);
	}

	bool EyelockConfiguration::setEyelock_DualMatcherPolicy(bool newValue)
	{

		conf.setValue("Eyelock.DualMatcherPolicy", newValue);

		return true;
	}

	void EyelockConfiguration::ensureEyelock_DualMatcherPolicy()
	{
		if (!conf.has(string("Eyelock.DualMatcherPolicy")))
		{
			conf.setValue("Eyelock.DualMatcherPolicy", Eyelock_DualMatcherPolicyDefault);
		}
	}

	const unsigned int GRI_RepeatAuthorizationPeriodDefault = 4000;
	// seconds are accepted, ms are stored
	unsigned int EyelockConfiguration::getGRI_RepeatAuthorizationPeriod()
	{
		return conf.getInt(string("GRI.RepeatAuthorizationPeriod"), GRI_RepeatAuthorizationPeriodDefault);
	}

	bool EyelockConfiguration::setGRI_RepeatAuthorizationPeriod(unsigned int newValue)
	{
		if (!ParameterValidator::validateRange<unsigned int>( newValue, 2000, 60000))
		{
			return false;
		}

		conf.setValue("GRI.RepeatAuthorizationPeriod", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRI_RepeatAuthorizationPeriod()
	{
		if (!conf.has(string("GRI.RepeatAuthorizationPeriod")))
		{
			conf.setValue("GRI.RepeatAuthorizationPeriod", GRI_RepeatAuthorizationPeriodDefault);
		}
	}

	const bool Eyelock_EnableNegativeMatchTimeoutDefault = true;
	// bool is accepted, but default Eyelock.ini stores 1 or 0 for some reason
	bool EyelockConfiguration::getEyelock_EnableNegativeMatchTimeout()
	{
		return conf.getBool(string("Eyelock.EnableNegativeMatchTimeout"), false);
	}

	bool EyelockConfiguration::setEyelock_EnableNegativeMatchTimeout(bool newValue)
	{
		int storedValue = newValue ? 1 : 0;
		conf.setValue("Eyelock.EnableNegativeMatchTimeout", storedValue);

		return true;
	}

	void EyelockConfiguration::ensureEyelock_EnableNegativeMatchTimeout()
	{
		if (!conf.has(string("Eyelock.EnableNegativeMatchTimeout")))
		{
			int storedValue = Eyelock_EnableNegativeMatchTimeoutDefault ? 1 : 0;
			conf.setValue("Eyelock.EnableNegativeMatchTimeout", storedValue);
		}
	}

	const unsigned int Eyelock_NegativeMatchTimeoutDefault = 6000;
	// seconds are accepted, ms are stored
	// in WebConfig float values are accepted, but in Eyelock app it's int
	unsigned int EyelockConfiguration::getEyelock_NegativeMatchTimeout()
	{
		return conf.getUInt(string("Eyelock.NegativeMatchTimeout"), Eyelock_NegativeMatchTimeoutDefault);
	}

	bool EyelockConfiguration::setEyelock_NegativeMatchTimeout(unsigned int newValue)
	{
		if (!ParameterValidator::validateRange<unsigned int>(newValue, 2000, 60000))
		{
			return false;
		}

		conf.setValue("Eyelock.NegativeMatchTimeout", newValue);

		return true;
	}

	void EyelockConfiguration::ensureEyelock_NegativeMatchTimeout()
	{
		if (!conf.has(string("Eyelock.NegativeMatchTimeout")))
		{
			conf.setValue("Eyelock.NegativeMatchTimeout", Eyelock_NegativeMatchTimeoutDefault);
		}
	}

	const unsigned int Eyelock_NegativeMatchResetTimerDefault = 4;
	// seconds are accepted
	// in WebConfig float values are accepted, but in Eyelock app: int m_iWaitTime
	unsigned int EyelockConfiguration::getEyelock_NegativeMatchResetTimer()
	{
		return conf.getUInt(string("Eyelock.NegativeMatchResetTimer"), Eyelock_NegativeMatchResetTimerDefault);
	}

	bool EyelockConfiguration::setEyelock_NegativeMatchResetTimer(unsigned int newValue)
	{
		if (!ParameterValidator::validateRange<unsigned int>(newValue, 2, 60))
		{
			return false;
		}

		conf.setValue("Eyelock.NegativeMatchResetTimer", newValue);

		return true;
	}

	void EyelockConfiguration::ensureEyelock_NegativeMatchResetTimer()
	{
		if (!conf.has(string("Eyelock.NegativeMatchResetTimer")))
		{
			conf.setValue("Eyelock.NegativeMatchResetTimer", Eyelock_NegativeMatchResetTimerDefault);
		}
	}


	// ****** Matchers settings (including Network Matcher) ******
	bool EyelockConfiguration::setNetworkMatcherAddress(const std::string& host, const unsigned short port)
	{
		stringstream ss;
		ss << host << ":" << port;

		return setNetworkMatcherAddress(ss.str());
	}

	bool EyelockConfiguration::setNetworkMatcherAddress(const std::string& newValue)
	{
		string endpointRegex = "^((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]):(6553[0-5]|655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4][0-9]{3}|[1-5][0-9]{4}|[1-9][0-9]{0,3})$";

		if (!ParameterValidator::validateRegex(newValue, endpointRegex))
		{
			return false;
		}

		conf.setValue("HighLevel.NetworkMatcherAddress", newValue);

		return true;
	}

	string EyelockConfiguration::getNetworkMatcherAddress()
	{
		if (isNetworkMatcherEnabled())
		{
			return conf.getStr(string("GRI.HDMatcher.1.Address"), "");
		}

		return conf.getStr(string("HighLevel.NetworkMatcherAddress"), "");
	}

	void EyelockConfiguration::ensureNetworkMatcher()
	{
		if (!conf.has(string("HighLevel.NetworkMatcherAddress")))
		{
			conf.setValue("HighLevel.NetworkMatcherAddress", "");
		}

		if (!conf.has(string("HighLevel.NetworkMatcherEnabled")))
		{
			conf.setValue("HighLevel.NetworkMatcherEnabled", false);
		}

		ensureGRI_HDMatcherID();
		ensureGRI_HDMatcherCount();
	}


	bool EyelockConfiguration::enableNetworkMatcher(bool newValue)
	{
		bool result = false;
		if (newValue)
		{
			string nwmEndpoint = conf.getStr("HighLevel.NetworkMatcherAddress", "");
			if (nwmEndpoint.empty())
			{
				EyelockLog(logger, ERROR, "NW Matcher address is empty");
			}
			else
			{
				if (setGRI_HDMatcher_1_Address(nwmEndpoint.c_str()))
				{
					conf.setValue("GRI.HDMatcherID", 1);
					conf.setValue("GRI.HDMatcherCount", 2);
					conf.setValue("GRI.HDMatcher.1.Type", "PCMATCHER");
					conf.setValue("GRI.HDMatcher.1.BuffSize", 0);

					conf.setValue("HighLevel.NetworkMatcherEnabled", true);

					result = true;
				}
			}
		}
		else
		{
			conf.setValue("GRI.HDMatcherID", 0);
			conf.setValue("GRI.HDMatcherCount", 1);

			conf.remove("GRI.HDMatcher.1.Address");
			conf.remove("GRI.HDMatcher.1.Type");
			conf.remove("GRI.HDMatcher.1.BuffSize");

			conf.setValue("HighLevel.NetworkMatcherEnabled", false);

			result = true;
		}

		return result;
	}

	bool EyelockConfiguration::isNetworkMatcherEnabled()
	{
		//check is based on keys used by FW classes instead of HighLevel keys for more reliability
		return (conf.getInt("GRI.HDMatcherID") == 1 && conf.getInt("GRI.HDMatcherCount") == 2 && conf.has("GRI.HDMatcher.1.Address"));
	}

	const int GRI_HDMatcherIDDefault = 0;
	int EyelockConfiguration::getGRI_HDMatcherID()
	{
		return conf.getInt(string("GRI.HDMatcherID"), GRI_HDMatcherIDDefault);
	}

	const int GRI_HDMatcherCountDefault = 1;
	int EyelockConfiguration::getGRI_HDMatcherCount()
	{
		return conf.getInt(string("GRI.HDMatcherCount"), GRI_HDMatcherCountDefault);
	}

	const char* EyelockConfiguration::getGRI_HDMatcher_1_Type()
	{
		return conf.getCStr(string("GRI.HDMatcher.1.Type"), "");
	}

	const int GRI_HDMatcher_1_BuffSizeDefault = 1;
	int EyelockConfiguration::getGRI_HDMatcher_1_BuffSize()
	{
		return conf.getInt(string("GRI.HDMatcher.1.BuffSize"), GRI_HDMatcher_1_BuffSizeDefault);
	}


	bool EyelockConfiguration::setGRI_HDMatcherID(int newValue)
	{
		if (!ParameterValidator::validateRange(newValue, 0, 9))
		{
			return false;
		}

		conf.setValue("GRI.HDMatcherID", newValue);

		return true;
	}

	bool EyelockConfiguration::setGRI_HDMatcherCount(int newValue)
	{
		if (!ParameterValidator::validateRange(newValue, 0, 10))
		{
			return false;
		}

		conf.setValue("GRI.HDMatcherCount", newValue);

		return true;
	}

	bool EyelockConfiguration::setGRI_HDMatcher_1_Type(const char* newValue)
	{
		// TODO: validate (however, it is protected)

		conf.setValue("GRI.HDMatcher.1.Type", newValue);

		return true;
	}

	bool EyelockConfiguration::setGRI_HDMatcher_1_BuffSize(int newValue)
	{
		if (!ParameterValidator::validateRange(newValue, 0, INT_MAX))
		{
			return false;
		}

		conf.setValue("GRI.HDMatcher.1.BuffSize", newValue);

		return true;
	}


	void EyelockConfiguration::ensureGRI_HDMatcherID()
	{
		if (!conf.has(string("GRI.HDMatcherID")))
		{
			conf.setValue("GRI.HDMatcherID", GRI_HDMatcherIDDefault);
		}
	}

	void EyelockConfiguration::ensureGRI_HDMatcherCount()
	{
		if (!conf.has(string("GRI.HDMatcherCount")))
		{
			conf.setValue("GRI.HDMatcherCount", GRI_HDMatcherCountDefault);
		}
	}

	void EyelockConfiguration::ensureGRI_HDMatcher_1_Type()
	{
		// not required
	}

	void EyelockConfiguration::ensureGRI_HDMatcher_1_BuffSize()
	{
		// not required
	}

	const char* EyelockConfiguration::getGRI_HDMatcher_1_Address()
	{
		return conf.getCStr(string("GRI.HDMatcher.1.Address"), "");
	}

	bool EyelockConfiguration::setGRI_HDMatcher_1_Address(const char* newValue)
	{
		string endpointRegex = "^((25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9])[.]){3}(25[0-5]|2[0-4][0-9]|1[0-9][0-9]|[1-9][0-9]|[0-9]):(6553[0-5]|655[0-2][0-9]|65[0-4][0-9]{2}|6[0-4][0-9]{3}|[1-5][0-9]{4}|[1-9][0-9]{0,3})$";

		if (!ParameterValidator::validateRegex(newValue, endpointRegex))
		{
			EyelockLog(logger, ERROR, "Invalid value for NW Matcher address: %s", newValue);
			return false;
		}

		conf.setValue("GRI.HDMatcher.1.Address", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRI_HDMatcher_1_Address()
	{
		// not required
	}

	// ****** ******


	EyelockConfiguration::Protocol EyelockConfiguration::strToProtocol(const char *protocolCStr)
	{
		if (!strncmp(protocolCStr, "OSDP", 4))
		{
			return OSDP;
		}
		else if (!strncmp(protocolCStr, "WIEGAND", 7))
		{
			return WIEGAND;
		}
		else if (!strncmp(protocolCStr, "HID", 3))
		{
			return HID;
		}
		else if (!strncmp(protocolCStr, "PAC", 3))
		{
			return PAC;
		}
		else if (!strncmp(protocolCStr, "F2F", 3))
		{
			return F2F;
		}
		else
		{
			return UNKNOWN_PROTOCOL;
		}
	}

	string EyelockConfiguration::protocolToStr(EyelockConfiguration::Protocol protocol)
	{
		if (protocol == OSDP)
		{
			return "OSDP";
		}
		else if (protocol == WIEGAND)
		{
			return "WIEGAND";
		}
		else if (protocol == HID)
		{
			return "HID";
		}
		else if (protocol == PAC)
		{
			return "PAC";
		}
		else if (protocol == F2F)
		{
			return "F2F";
		}
		else
		{
			return "UNKNOWN_PROTOCOL";
		}

	}

	EyelockConfiguration::Protocol EyelockConfiguration::getProtocol()
	{
		if (conf.getBool("GRITrigger.OSDPEnable"))
		{
			return OSDP;
		}
		else if (conf.getBool("GRITrigger.WeigandEnable"))
		{
			return WIEGAND;
		}
		else if (conf.getBool("GRITrigger.WeigandHidEnable"))
		{
			return HID;
		}
		else if (conf.getBool("GRITrigger.PACEnable"))
		{
			return PAC;
		}
		else if (conf.getBool("GRITrigger.F2FEnable"))
		{
			return F2F;
		}
		else
		{
			return UNKNOWN_PROTOCOL;
		}
	}

	bool EyelockConfiguration::setProtocol(EyelockConfiguration::Protocol value)
	{
		if (value == OSDP)
		{
			conf.setValue("GRITrigger.OSDPEnable", true);
			//conf.setValue("GRITrigger.OSDPInputEnable", true); // commented in WebConfig
			conf.setValue("GRITrigger.WeigandEnable", true); // TODO: investigate why in WebConfig we have true // segfault if false
			conf.setValue("GRITrigger.WeigandHidEnable", false);
			conf.setValue("GRITrigger.PACEnable", false);
			conf.setValue("GRITrigger.F2FEnable", false);

			conf.setValue("HighLevel.Protocol", "OSDP");
		}
		else if (value == WIEGAND)
		{
			conf.setValue("GRITrigger.OSDPEnable", false);
			//conf.setValue("GRITrigger.OSDPInputEnable", false); // commented in WebConfig
			conf.setValue("GRITrigger.WeigandEnable", true);
			conf.setValue("GRITrigger.WeigandHidEnable", false);
			conf.setValue("GRITrigger.PACEnable", false);
			conf.setValue("GRITrigger.F2FEnable", false);

			conf.setValue("HighLevel.Protocol", "WIEGAND");
		}
		else if (value == HID)
		{
			conf.setValue("GRITrigger.OSDPEnable", false);
			//conf.setValue("GRITrigger.OSDPInputEnable", false); // commented in WebConfig
			conf.setValue("GRITrigger.WeigandEnable", false);
			conf.setValue("GRITrigger.WeigandHidEnable", true);
			conf.setValue("GRITrigger.PACEnable", false);
			conf.setValue("GRITrigger.F2FEnable", false);

			conf.setValue("HighLevel.Protocol", "HID");
		}
		else if (value == PAC)
		{
			conf.setValue("GRITrigger.OSDPEnable", false);
			//conf.setValue("GRITrigger.OSDPInputEnable", false); // commented in WebConfig
			conf.setValue("GRITrigger.WeigandEnable", false);
			conf.setValue("GRITrigger.WeigandHidEnable", false);
			conf.setValue("GRITrigger.PACEnable", true);
			conf.setValue("GRITrigger.F2FEnable", false);

			conf.setValue("HighLevel.Protocol", "PAC");
		}
		else if (value == F2F)
		{
			conf.setValue("GRITrigger.OSDPEnable", false);
			//conf.setValue("GRITrigger.OSDPInputEnable", false); // commented in WebConfig
			conf.setValue("GRITrigger.WeigandEnable", false);
			conf.setValue("GRITrigger.WeigandHidEnable", false);
			conf.setValue("GRITrigger.PACEnable", false);
			conf.setValue("GRITrigger.F2FEnable", true);

			conf.setValue("HighLevel.Protocol", "F2F");
		}
		else
		{
			return false;
		}

		return true;
	}

	void EyelockConfiguration::ensureProtocol()
	{
		ensureGRITrigger_OSDPEnable();
		ensureGRITrigger_WeigandEnable();
		ensureGRITrigger_WeigandHidEnable();
		ensureGRITrigger_PACEnable();
		ensureGRITrigger_F2FEnable();

		if (!conf.has(string("HighLevel.Protocol")))
		{
			conf.setValue("HighLevel.Protocol", "WIEGAND");
		}
	}

	const bool GRITrigger_WeigandEnableDefault = true;
	bool EyelockConfiguration::getGRITrigger_WeigandEnable()
	{
		return conf.getBool(string("GRITrigger.WeigandEnable"), GRITrigger_WeigandEnableDefault);
	}

	bool EyelockConfiguration::setGRITrigger_WeigandEnable(bool newValue)
	{

		conf.setValue("GRITrigger.WeigandEnable", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_WeigandEnable()
	{
		if (!conf.has(string("GRITrigger.WeigandEnable")))
		{
			conf.setValue("GRITrigger.WeigandEnable", GRITrigger_WeigandEnableDefault);
		}
	}

	const bool GRITrigger_WeigandHidEnableDefault = false;
	bool EyelockConfiguration::getGRITrigger_WeigandHidEnable()
	{
		return conf.getBool(string("GRITrigger.WeigandHidEnable"), GRITrigger_WeigandHidEnableDefault);
	}

	bool EyelockConfiguration::setGRITrigger_WeigandHidEnable(bool newValue)
	{

		conf.setValue("GRITrigger.WeigandHidEnable", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_WeigandHidEnable()
	{
		if (!conf.has(string("GRITrigger.WeigandHidEnable")))
		{
			conf.setValue("GRITrigger.WeigandHidEnable", GRITrigger_WeigandHidEnableDefault);
		}
	}

	const bool GRITrigger_PACEnableDefault = false;
	bool EyelockConfiguration::getGRITrigger_PACEnable()
	{
		return conf.getBool(string("GRITrigger.PACEnable"), GRITrigger_PACEnableDefault);
	}

	bool EyelockConfiguration::setGRITrigger_PACEnable(bool newValue)
	{

		conf.setValue("GRITrigger.PACEnable", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_PACEnable()
	{
		if (!conf.has(string("GRITrigger.PACEnable")))
		{
			conf.setValue("GRITrigger.PACEnable", GRITrigger_PACEnableDefault);
		}
	}

	const bool GRITrigger_F2FEnableDefault = false;
	bool EyelockConfiguration::getGRITrigger_F2FEnable()
	{
		return conf.getBool(string("GRITrigger.F2FEnable"), GRITrigger_F2FEnableDefault);
	}

	bool EyelockConfiguration::setGRITrigger_F2FEnable(bool newValue)
	{

		conf.setValue("GRITrigger.F2FEnable", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_F2FEnable()
	{
		if (!conf.has(string("GRITrigger.F2FEnable")))
		{
			conf.setValue("GRITrigger.F2FEnable", GRITrigger_F2FEnableDefault);
		}
	}

	const bool GRITrigger_OSDPEnableDefault = false;
	bool EyelockConfiguration::getGRITrigger_OSDPEnable()
	{
		return conf.getBool(string("GRITrigger.OSDPEnable"), GRITrigger_OSDPEnableDefault);
	}

	bool EyelockConfiguration::setGRITrigger_OSDPEnable(bool newValue)
	{

		conf.setValue("GRITrigger.OSDPEnable", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_OSDPEnable()
	{
		if (!conf.has(string("GRITrigger.OSDPEnable")))
		{
			conf.setValue("GRITrigger.OSDPEnable", GRITrigger_OSDPEnableDefault);
		}
	}

	const unsigned int Eyelock_OSDPAddressDefault = 0;
	unsigned int EyelockConfiguration::getEyelock_OSDPAddress()
	{
		return conf.getInt(string("Eyelock.OSDPAddress"), Eyelock_OSDPAddressDefault);
	}

	bool EyelockConfiguration::setEyelock_OSDPAddress(unsigned int newValue)
	{
		if (!ParameterValidator::validateRange<unsigned int>(newValue, 0, 127))
		{
			return false;
		}

		conf.setValue("Eyelock.OSDPAddress", newValue);

		return true;
	}

	void EyelockConfiguration::ensureEyelock_OSDPAddress()
	{
		if (!conf.has(string("Eyelock.OSDPAddress")))
		{
			conf.setValue("Eyelock.OSDPAddress", Eyelock_OSDPAddressDefault);
		}
	}

	const unsigned int Eyelock_OSDPBaudRateDefault = 9600;
	unsigned int EyelockConfiguration::getEyelock_OSDPBaudRate()
	{
		return conf.getInt(string("Eyelock.OSDPBaudRate"), Eyelock_OSDPBaudRateDefault);
	}

	bool EyelockConfiguration::setEyelock_OSDPBaudRate(unsigned int newValue)
	{
		if (newValue != 9600 && newValue != 19200 && newValue != 38400 && newValue != 115200 && newValue != 0) // 0 is added for backward compatibility
		{
			return false;
		}

		newValue = (newValue == 0) ? Eyelock_OSDPBaudRateDefault : newValue; // treating 0 as default

		conf.setValue("Eyelock.OSDPBaudRate", newValue);

		return true;
	}

	void EyelockConfiguration::ensureEyelock_OSDPBaudRate()
	{
		if (!conf.has(string("Eyelock.OSDPBaudRate")))
		{
			conf.setValue("Eyelock.OSDPBaudRate", Eyelock_OSDPBaudRateDefault);
		}
	}

	const bool GRITrigger_DualAuthNLedControlledByACSDefault = false;
	bool EyelockConfiguration::getGRITrigger_DualAuthNLedControlledByACS()
	{
		return conf.getBool(string("GRITrigger.DualAuthNLedControlledByACS"), GRITrigger_DualAuthNLedControlledByACSDefault);
	}

	bool EyelockConfiguration::setGRITrigger_DualAuthNLedControlledByACS(bool newValue)
	{

		conf.setValue("GRITrigger.DualAuthNLedControlledByACS", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_DualAuthNLedControlledByACS()
	{
		if (!conf.has(string("GRITrigger.DualAuthNLedControlledByACS")))
		{
			conf.setValue("GRITrigger.DualAuthNLedControlledByACS", GRITrigger_DualAuthNLedControlledByACSDefault);
		}
	}


	unsigned int EyelockConfiguration::strToMatchMode(const char *matchModeCStr)
	{
		if (matchModeCStr == NULL)
		{
			return 0;
		}

		string matchModeStr(matchModeCStr);

		if (!matchModeStr.compare("IRIS_ONLY"))
		{
			return IRIS_ONLY;
		}
		else if (!matchModeStr.compare("IRIS_OR_CARD"))
		{
			return CARD_OR_IRIS;
		}
		else if (!matchModeStr.compare("IRIS_AND_CARD"))
		{
			return CARD_AND_IRIS;
		}
		else if (!matchModeStr.compare("IRIS_AND_CARD_PIN_PASS"))
		{
			return CARD_AND_IRIS_PIN_PASS;
		}
		else if (!matchModeStr.compare("IRIS_AND_PIN"))
		{
			return PIN_AND_IRIS;
		}
		else if (!matchModeStr.compare("IRIS_AND_CARD_AND_PIN"))
		{
			return CARD_AND_PIN_AND_IRIS;
		}
		else if (!matchModeStr.compare("IRIS_AND_PIN_DURESS"))
		{
			return PIN_AND_IRIS_DURESS;
		}
		else if (!matchModeStr.compare("IRIS_AND_CARD_AND_PIN_DURESS"))
		{
			return CARD_AND_PIN_AND_IRIS_DURESS;
		}
		else
		{
			return 0;
		}
	}

	string EyelockConfiguration::matchModeToStr(MatchMode matchMode)
	{
		if (matchMode == IRIS_ONLY)
		{
			return "IRIS_ONLY";
		}
		else if (matchMode == CARD_OR_IRIS)
		{
			return "IRIS_OR_CARD";
		}
		else if (matchMode == CARD_AND_IRIS)
		{
			return "IRIS_AND_CARD";
		}
		else if (matchMode == CARD_AND_IRIS_PIN_PASS)
		{
			return "IRIS_AND_CARD_PIN_PASS";
		}
		else if (matchMode == PIN_AND_IRIS)
		{
			return "IRIS_AND_PIN";
		}
		else if (matchMode == CARD_AND_PIN_AND_IRIS)
		{
			return "IRIS_AND_CARD_AND_PIN";
		}
		else if (matchMode == PIN_AND_IRIS_DURESS)
		{
			return "IRIS_AND_PIN_DURESS";
		}
		else if (matchMode == CARD_AND_PIN_AND_IRIS_DURESS)
		{
			return "IRIS_AND_CARD_AND_PIN_DURESS";
		}
		else
		{
			return "UNKNOWN_SCHEME";
		}
	}

	const MatchMode Eyelock_AuthenticationModeDefault = IRIS_ONLY;
	// 0 is returned by default in F2FDispacther, but no such value in MatchMode enum and also 0 is processed in exactly the same way as IRIS_ONLY=1

	MatchMode EyelockConfiguration::getAuthScheme()
	{
		return (MatchMode) getAuthSchemeUInt();
	}

	unsigned int EyelockConfiguration::getAuthSchemeUInt()
	{
		if (conf.has(string("Eyelock.AuthenticationMode")))
		{
			return conf.getUInt(string("Eyelock.AuthenticationMode"), Eyelock_AuthenticationModeDefault);
		}
		else
		{
			bool isDualAuth = conf.getBool(string("GRITrigger.DualAuthenticationMode"), false);
			bool isPassThrough = conf.getBool(string("GRITrigger.PassThroughMode"), false);

			if (!isDualAuth && !isPassThrough)
			{
				return IRIS_ONLY;
			}
			else if (isDualAuth && !isPassThrough)
			{
				return CARD_AND_IRIS;
			}
			else if (!isDualAuth && isPassThrough)
			{
				return CARD_OR_IRIS;
			}
			else
			{
				// this must not be the case
				// if 0 is returned by the SDK, this means that user should set the correct value
				return 0;
			}
		}
	}

	bool EyelockConfiguration::setAuthScheme(MatchMode newValue)
	{
		conf.setValue("Eyelock.AuthenticationMode", (unsigned int) newValue);
		if (newValue == CARD_OR_IRIS)
		{
			conf.setValue("GRITrigger.DualAuthenticationMode", false);
			conf.setValue("GRITrigger.PassThrough", true);
		}
		else if (newValue == CARD_AND_IRIS)
		{
			conf.setValue("GRITrigger.DualAuthenticationMode", true);
			conf.setValue("GRITrigger.PassThrough", false);
		}
		else // if (newValue == IRIS_ONLY || newValue > CARD_AND_IRIS)
		{
			conf.setValue("GRITrigger.DualAuthenticationMode", false);
			conf.setValue("GRITrigger.PassThrough", false);
		}
		return true;
	}

	bool EyelockConfiguration::setAuthScheme(unsigned int newValue)
	{
		if (newValue == 0 || newValue > CARD_AND_PIN_AND_IRIS_DURESS) // Should be updated every time new value added to enum
		{
			return false;
		}

		return setAuthScheme((MatchMode) newValue);
	}

	bool EyelockConfiguration::setAuthScheme(const string& newValue)
	{
		unsigned int temp = strToMatchMode(newValue.c_str());
		if (temp > 0)
		{
			return setAuthScheme(temp);
		}
		return false;
	}

	void EyelockConfiguration::getDualAuthAndPassThrough(bool& dualAuth, bool& passThrough)
	{
		MatchMode authMode = getAuthScheme();
		//IRIS_ONLY=1, CARD_OR_IRIS, CARD_AND_IRIS, CARD_AND_IRIS_PIN_PASS, PIN_AND_IRIS, CARD_AND_PIN_AND_IRIS, PIN_AND_IRIS_DURESS, CARD_AND_PIN_AND_IRIS_DURESS

		if (authMode == CARD_OR_IRIS)
		{
			dualAuth = false;
			passThrough = true;
		}
		else if (authMode == CARD_AND_IRIS)
		{
			dualAuth = true;
			passThrough = false;
		}
		else //if (authMode == IRIS_ONLY || authMode > CARD_AND_IRIS)
		{
			dualAuth = false;
			passThrough = false;
		}
	}

	bool EyelockConfiguration::getGRITrigger_DualAuthenticationMode()
	{
		bool dualAuth, passThrough;
		getDualAuthAndPassThrough(dualAuth, passThrough);
		return dualAuth;
	}

	bool EyelockConfiguration::getGRITrigger_PassThroughMode()
	{
		bool dualAuth, passThrough;
		getDualAuthAndPassThrough(dualAuth, passThrough);
		return passThrough;
	}

	bool EyelockConfiguration::setGRITrigger_DualAuthenticationMode(bool newDualAuth)
	{
		bool passThrough = conf.getBool(string("GRITrigger.PassThrough"), false);
		conf.setValue("GRITrigger.DualAuthenticationMode", newDualAuth);

		if (!newDualAuth && !passThrough)
		{
			return setAuthScheme(IRIS_ONLY);
		}
		else if (newDualAuth && !passThrough)
		{
			return setAuthScheme(CARD_AND_IRIS);
		}
		else if (!newDualAuth && passThrough)
		{
			return setAuthScheme(CARD_OR_IRIS);
		}
		else //if (newDualAuth && passThrough)
		{
			conf.setValue("GRITrigger.PassThrough", false);
			return setAuthScheme(CARD_AND_IRIS);
		}
	}

	bool EyelockConfiguration::setGRITrigger_PassThroughMode(bool newPassThrough)
	{
		bool dualAuth = conf.getBool(string("GRITrigger.DualAuthenticationMode"), false);
		conf.setValue("GRITrigger.PassThrough", newPassThrough);

		if (!newPassThrough && !dualAuth)
		{
			return setAuthScheme(IRIS_ONLY);
		}
		else if (newPassThrough && !dualAuth)
		{
			return setAuthScheme(CARD_OR_IRIS);
		}
		else if (!newPassThrough && dualAuth)
		{
			return setAuthScheme(CARD_AND_IRIS);
		}
		else //if (newPassThroughValue && dualAuth)
		{
			conf.setValue("GRITrigger.DualAuthenticationMode", false);
			return setAuthScheme(CARD_OR_IRIS);
		}
	}

	void EyelockConfiguration::ensureAuthScheme()
	{
		if (!conf.has(string("Eyelock.AuthenticationMode")))
		{
			conf.setValue("Eyelock.AuthenticationMode", Eyelock_AuthenticationModeDefault);
		}
	}


	const bool GRITrigger_DualAuthenticationParityDefault = true;
	// seems to be redundant
	bool EyelockConfiguration::getGRITrigger_DualAuthenticationParity()
	{
		return conf.getBool(string("GRITrigger.DualAuthenticationParity"), GRITrigger_DualAuthenticationParityDefault);
	}

	bool EyelockConfiguration::setGRITrigger_DualAuthenticationParity(bool newValue)
	{

		conf.setValue("GRITrigger.DualAuthenticationParity", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_DualAuthenticationParity()
	{
		if (!conf.has(string("GRITrigger.DualAuthenticationParity")))
		{
			conf.setValue("GRITrigger.DualAuthenticationParity", GRITrigger_DualAuthenticationParityDefault);
		}
	}

	const unsigned int GRITrigger_DualAuthNCardMatchWaitIrisTimeDefault = 10000;
	// seconds are accepted, ms are stored
	// in WebConfig float values are accepted, but in Eyelock app it's int
	unsigned int EyelockConfiguration::getGRITrigger_DualAuthNCardMatchWaitIrisTime()
	{
		return conf.getUInt(string("GRITrigger.DualAuthNCardMatchWaitIrisTime"), GRITrigger_DualAuthNCardMatchWaitIrisTimeDefault);
	}

	bool EyelockConfiguration::setGRITrigger_DualAuthNCardMatchWaitIrisTime(unsigned int newValue)
	{
		if (!ParameterValidator::validateRange<unsigned int>(newValue, 2000, 60000))
		{
			return false;
		}

		conf.setValue("GRITrigger.DualAuthNCardMatchWaitIrisTime", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_DualAuthNCardMatchWaitIrisTime()
	{
		if (!conf.has(string("GRITrigger.DualAuthNCardMatchWaitIrisTime")))
		{
			conf.setValue("GRITrigger.DualAuthNCardMatchWaitIrisTime", GRITrigger_DualAuthNCardMatchWaitIrisTimeDefault);
		}
	}

	const unsigned int Eyelock_WaitPinTimeDefault = 10000;
	// seconds are accepted, ms are stored
	// in WebConfig float values are accepted, but in Eyelock app it's int
	unsigned int EyelockConfiguration::getEyelock_WaitPinTime()
	{
		return conf.getUInt(string("Eyelock.WaitPinTime"), Eyelock_WaitPinTimeDefault);
	}

	bool EyelockConfiguration::setEyelock_WaitPinTime(unsigned int newValue)
	{
		if (!ParameterValidator::validateRange<unsigned int>(newValue, 1000, 60000))
		{
			return false;
		}

		conf.setValue("Eyelock.WaitPinTime", newValue);

		return true;
	}

	void EyelockConfiguration::ensureEyelock_WaitPinTime()
	{
		if (!conf.has(string("Eyelock.WaitPinTime")))
		{
			conf.setValue("Eyelock.WaitPinTime", Eyelock_WaitPinTimeDefault);
		}
	}

	const unsigned int Eyelock_PinBurstBitsDefault = 4;
	unsigned int EyelockConfiguration::getEyelock_PinBurstBits()
	{
		return conf.getUInt(string("Eyelock.PinBurstBits"), Eyelock_PinBurstBitsDefault);
	}

	bool EyelockConfiguration::setEyelock_PinBurstBits(unsigned int newValue)
	{
		if (newValue != 4 && newValue != 8)
		{
			return false;
		}

		conf.setValue("Eyelock.PinBurstBits", newValue);

		return true;
	}

	void EyelockConfiguration::ensureEyelock_PinBurstBits()
	{
		if (!conf.has(string("Eyelock.PinBurstBits")))
		{
			conf.setValue("Eyelock.PinBurstBits", Eyelock_PinBurstBitsDefault);
		}
	}

	const bool GRITrigger_EnableRelayWithSignalDefault = false;
	bool EyelockConfiguration::getGRITrigger_EnableRelayWithSignal()
	{
		return conf.getBool(string("GRITrigger.EnableRelayWithSignal"), GRITrigger_EnableRelayWithSignalDefault);
	}

	bool EyelockConfiguration::setGRITrigger_EnableRelayWithSignal(bool newValue)
	{
		conf.setValue("GRITrigger.EnableRelayWithSignal", newValue);
		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_EnableRelayWithSignal()
	{
		if (!conf.has(string("GRITrigger.EnableRelayWithSignal")))
		{
			conf.setValue("GRITrigger.EnableRelayWithSignal", GRITrigger_EnableRelayWithSignalDefault);
		}
	}

	const unsigned int GRITrigger_RelayTimeInMSDefault = 3000;
	// seconds are accepted, ms are stored
	// in WebConfig float values are accepted, but in Eyelock app it's int
	unsigned int EyelockConfiguration::getGRITrigger_RelayTimeInMS()
	{
		return conf.getUInt(string("GRITrigger.RelayTimeInMS"), GRITrigger_RelayTimeInMSDefault);
	}

	bool EyelockConfiguration::setGRITrigger_RelayTimeInMS(unsigned int newValue)
	{
		if (!ParameterValidator::validateRange<unsigned int>(newValue, 0, 10000))
		{
			return false;
		}

		conf.setValue("GRITrigger.RelayTimeInMS", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_RelayTimeInMS()
	{
		if (!conf.has(string("GRITrigger.RelayTimeInMS")))
		{
			conf.setValue("GRITrigger.RelayTimeInMS", GRITrigger_RelayTimeInMSDefault);
		}
	}

	const unsigned int GRITrigger_DenyRelayTimeInMSDefault = 5000;
	// seconds are accepted, ms are stored
	// in WebConfig float values are accepted, but in Eyelock app it's int
	unsigned int EyelockConfiguration::getGRITrigger_DenyRelayTimeInMS()
	{
		return conf.getUInt(string("GRITrigger.DenyRelayTimeInMS"), GRITrigger_DenyRelayTimeInMSDefault);
	}

	bool EyelockConfiguration::setGRITrigger_DenyRelayTimeInMS(unsigned int newValue)
	{
		if (!ParameterValidator::validateRange<unsigned int>(newValue, 0, 10000))
		{
			return false;
		}

		conf.setValue("GRITrigger.DenyRelayTimeInMS", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_DenyRelayTimeInMS()
	{
		if (!conf.has(string("GRITrigger.DenyRelayTimeInMS")))
		{
			conf.setValue("GRITrigger.DenyRelayTimeInMS", GRITrigger_DenyRelayTimeInMSDefault);
		}
	}

	const bool Eyelock_TLSEnableDefault = true;
	bool EyelockConfiguration::getEyelock_TLSEnable()
	{
		return conf.getBool(string("Eyelock.TLSEnable"), Eyelock_TLSEnableDefault);
	}

	bool EyelockConfiguration::setEyelock_TLSEnable(bool newValue)
	{
		if (newValue)
		{
			conf.setValue("Eyelock.Cipher", "TLSv1.2:!eNULL");
		}
		else
		{
			conf.setValue("Eyelock.Cipher", "TLSv1.2:TLSv1:SSLv3");
		}
		conf.setValue("Eyelock.TLSEnable", newValue);
		return true;
	}

	void EyelockConfiguration::ensureEyelock_TLSEnable()
	{
		if (!conf.has(string("Eyelock.TLSEnable")))
		{
			conf.setValue("Eyelock.TLSEnable", Eyelock_TLSEnableDefault);
		}
	}


	// ***** WebConfig specific

	const string GRI_MatchResultDestAddrDefault = "";
	string EyelockConfiguration::getGRI_MatchResultDestAddr()
	{
		return conf.getStr(string("GRI.MatchResultDestAddr"), GRI_MatchResultDestAddrDefault);
	}

	bool EyelockConfiguration::setGRI_MatchResultDestAddr(const string& newValue)
	{
		conf.setValue("GRI.MatchResultDestAddr", newValue);
		return true;
	}

	void EyelockConfiguration::ensureGRI_MatchResultDestAddr()
	{
		if (!conf.has(string("GRI.MatchResultDestAddr")))
		{
			conf.setValue("GRI.MatchResultDestAddr", GRI_MatchResultDestAddrDefault);
		}
	}

	const string GRI_MatchResultNwMsgFormatDefault = "Matched:%d;Score:%0.4f;Time:%llu;ID:";
	string EyelockConfiguration::getGRI_MatchResultNwMsgFormat()
	{
		return conf.getStr(string("GRI.MatchResultNwMsgFormat"), GRI_MatchResultDestAddrDefault);
	}

	bool EyelockConfiguration::setGRI_MatchResultNwMsgFormat(const string& newValue)
	{
		conf.setValue("GRI.MatchResultNwMsgFormat", newValue);
		return true;
	}

	void EyelockConfiguration::ensureGRI_MatchResultNwMsgFormat()
	{
		if (!conf.has(string("GRI.MatchResultNwMsgFormat")))
		{
			conf.setValue("GRI.MatchResultNwMsgFormat", GRI_MatchResultNwMsgFormatDefault);
		}
	}


	const string GRI_NwDispatcherSecureDefault = "secure";
	string EyelockConfiguration::getGRI_NwDispatcherSecure()
	{
		return conf.getStr(string("GRI.NwDispatcherSecure"), GRI_NwDispatcherSecureDefault);
	}

	bool EyelockConfiguration::setGRI_NwDispatcherSecure(const string& newValue)
	{
		conf.setValue("GRI.NwDispatcherSecure", newValue);
		return true;
	}

	void EyelockConfiguration::ensureGRI_NwDispatcherSecure()
	{
		if (!conf.has(string("GRI.NwDispatcherSecure")))
		{
			conf.setValue("GRI.NwDispatcherSecure", GRI_NwDispatcherSecureDefault);
		}
	}

	const bool GRITrigger_PortableTemplatesUseCustomKeyDefault = false;
	bool EyelockConfiguration::getGRITrigger_PortableTemplatesUseCustomKey()
	{
		return conf.getBool(string("GRITrigger.PortableTemplatesUseCustomKey"), GRITrigger_PortableTemplatesUseCustomKeyDefault);
	}

	bool EyelockConfiguration::setGRITrigger_PortableTemplatesUseCustomKey(bool newValue)
	{
		conf.setValue("GRITrigger.PortableTemplatesUseCustomKey", newValue);
		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_PortableTemplatesUseCustomKey()
	{
		if (!conf.has(string("GRITrigger.PortableTemplatesUseCustomKey")))
		{
			conf.setValue("GRITrigger.PortableTemplatesUseCustomKey", GRITrigger_PortableTemplatesUseCustomKeyDefault);
		}
	}

	const bool Eyelock_SystemReadyDebugDefault = false;
	bool EyelockConfiguration::getEyelock_SystemReadyDebug()
	{
		return conf.getBool(string("Eyelock.SystemReadyDebug"), Eyelock_SystemReadyDebugDefault);
	}

	bool EyelockConfiguration::setEyelock_SystemReadyDebug(bool newValue)
	{
		conf.setValue("Eyelock.SystemReadyDebug", newValue);
		return true;
	}

	void EyelockConfiguration::ensureEyelock_SystemReadyDebug()
	{
		if (!conf.has(string("Eyelock.SystemReadyDebug")))
		{
			conf.setValue("Eyelock.SystemReadyDebug", Eyelock_SystemReadyDebugDefault);
		}
	}

	// TODO: other debug flags


	const int GRI_EyeConnecTimeoutmsecDefault = 4000;
	int EyelockConfiguration::getGRI_EyeConnecTimeoutmsec()
	{
		return conf.getInt(string("GRI.EyeConnecTimeoutmsec"), GRI_EyeConnecTimeoutmsecDefault);
	}

	bool EyelockConfiguration::setGRI_EyeConnecTimeoutmsec(int newValue)
	{
		if (!ParameterValidator::validateRange(newValue, 0, 60000))
		{
			return false;
		}

		conf.setValue("GRI.EyeConnecTimeoutmsec", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRI_EyeConnecTimeoutmsec()
	{
		if (!conf.has(string("GRI.EyeConnecTimeoutmsec")))
		{
			conf.setValue("GRI.EyeConnecTimeoutmsec", GRI_EyeConnecTimeoutmsecDefault);
		}
	}


	const int GRI_EyeSendTimeoutmsecDefault = 5000;
	int EyelockConfiguration::getGRI_EyeSendTimeoutmsec()
	{
		return conf.getInt(string("GRI.EyeSendTimeoutmsec"), GRI_EyeSendTimeoutmsecDefault);
	}

	bool EyelockConfiguration::setGRI_EyeSendTimeoutmsec(int newValue)
	{
		if (!ParameterValidator::validateRange(newValue, 0, 60000))
		{
			return false;
		}

		conf.setValue("GRI.EyeSendTimeoutmsec", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRI_EyeSendTimeoutmsec()
	{
		if (!conf.has(string("GRI.EyeSendTimeoutmsec")))
		{
			conf.setValue("GRI.EyeSendTimeoutmsec", GRI_EyeSendTimeoutmsecDefault);
		}
	}


	const bool GRITrigger_MobileModeDefault = false;
	bool EyelockConfiguration::getGRITrigger_MobileMode()
	{
		return conf.getBool(string("GRITrigger.MobileMode"), GRITrigger_MobileModeDefault);
	}

	bool EyelockConfiguration::setGRITrigger_MobileMode(bool newValue)
	{
		conf.setValue("GRITrigger.MobileMode", newValue);
		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_MobileMode()
	{
		if (!conf.has(string("GRITrigger.MobileMode")))
		{
			conf.setValue("GRITrigger.MobileMode", GRITrigger_MobileModeDefault);
		}
	}

	const bool GRITrigger_TransTOCModeDefault = false;
	bool EyelockConfiguration::getGRITrigger_TransTOCMode()
	{
		return conf.getBool(string("GRITrigger.TransTOCMode"), GRITrigger_TransTOCModeDefault);
	}

	bool EyelockConfiguration::setGRITrigger_TransTOCMode(bool newValue)
	{
		conf.setValue("GRITrigger.TransTOCMode", newValue);
		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_TransTOCMode()
	{
		if (!conf.has(string("GRITrigger.TransTOCMode")))
		{
			conf.setValue("GRITrigger.TransTOCMode", GRITrigger_TransTOCModeDefault);
		}
	}

	const int GRITrigger_TOCCardExpiredTimeDefault = 60;
	int EyelockConfiguration::getGRITrigger_TOCCardExpiredTime()
	{
		return conf.getInt(string("GRITrigger.TOCCardExpiredTime"), GRITrigger_TOCCardExpiredTimeDefault);
	}

	bool EyelockConfiguration::setGRITrigger_TOCCardExpiredTime(int newValue)
	{
		if (!ParameterValidator::validateRange(newValue, 0, 60000))
		{
			return false;
		}

		conf.setValue("GRITrigger.TOCCardExpiredTime", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRITrigger_TOCCardExpiredTime()
	{
		if (!conf.has(string("GRITrigger.TOCCardExpiredTime")))
		{
			conf.setValue("GRITrigger.TOCCardExpiredTime", GRITrigger_TOCCardExpiredTimeDefault);
		}
	}


	const string Eyelock_ACPCardIDDefault = "";
	string EyelockConfiguration::getEyelock_ACPCardID()
	{
		return conf.getStr(string("Eyelock.ACPCardID"), Eyelock_ACPCardIDDefault);
	}

	bool EyelockConfiguration::setEyelock_ACPCardID(const string& newValue)
	{
		conf.setValue("Eyelock.ACPCardID", newValue);
		return true;
	}

	void EyelockConfiguration::ensureEyelock_ACPCardID()
	{
		if (!conf.has(string("Eyelock.ACPCardID")))
		{
			conf.setValue("Eyelock.ACPCardID", Eyelock_ACPCardIDDefault);
		}
	}

	const string Eyelock_ACPTestFacilityCodeDefault = "";
	string EyelockConfiguration::getEyelock_ACPTestFacilityCode()
	{
		return conf.getStr(string("Eyelock.ACPTestFacilityCode"), Eyelock_ACPTestFacilityCodeDefault);
	}

	bool EyelockConfiguration::setEyelock_ACPTestFacilityCode(const string& newValue)
	{
		conf.setValue("Eyelock.ACPTestFacilityCode", newValue);
		return true;
	}

	void EyelockConfiguration::ensureEyelock_ACPTestFacilityCode()
	{
		if (!conf.has(string("Eyelock.ACPTestFacilityCode")))
		{
			conf.setValue("Eyelock.ACPTestFacilityCode", Eyelock_ACPTestFacilityCodeDefault);
		}
	}

	const string Eyelock_TamperNotifyAddrDefault = "";
	string EyelockConfiguration::getEyelock_TamperNotifyAddr()
	{
		return conf.getStr(string("Eyelock.TamperNotifyAddr"), Eyelock_TamperNotifyAddrDefault);
	}

	bool EyelockConfiguration::setEyelock_TamperNotifyAddr(const string& newValue)
	{
		conf.setValue("Eyelock.TamperNotifyAddr", newValue);
		return true;
	}

	void EyelockConfiguration::ensureEyelock_TamperNotifyAddr()
	{
		if (!conf.has(string("Eyelock.TamperNotifyAddr")))
		{
			conf.setValue("Eyelock.TamperNotifyAddr", Eyelock_TamperNotifyAddrDefault);
		}
	}

	const string Eyelock_TamperNotifyMessageDefault = "";
	string EyelockConfiguration::getEyelock_TamperNotifyMessage()
	{
		return conf.getStr(string("Eyelock.TamperNotifyMessage"), Eyelock_TamperNotifyMessageDefault);
	}

	bool EyelockConfiguration::setEyelock_TamperNotifyMessage(const string& newValue)
	{
		conf.setValue("Eyelock.TamperNotifyMessage", newValue);
		return true;
	}

	void EyelockConfiguration::ensureEyelock_TamperNotifyMessage()
	{
		if (!conf.has(string("Eyelock.TamperNotifyMessage")))
		{
			conf.setValue("Eyelock.TamperNotifyMessage", Eyelock_TamperNotifyMessageDefault);
		}
	}


	const int Eyelock_MaxTemplateCountDefault = 20000;
	int EyelockConfiguration::getEyelock_MaxTemplateCount()
	{
		return conf.getInt(string("Eyelock.MaxTemplateCount"), Eyelock_MaxTemplateCountDefault);
	}

	bool EyelockConfiguration::setEyelock_MaxTemplateCount(int newValue)
	{
		if (!ParameterValidator::validateRange(newValue, 0, 60000))
		{
			return false;
		}

		conf.setValue("Eyelock.MaxTemplateCount", newValue);

		return true;
	}

	void EyelockConfiguration::ensureEyelock_MaxTemplateCount()
	{
		if (!conf.has(string("Eyelock.MaxTemplateCount")))
		{
			conf.setValue("Eyelock.MaxTemplateCount", Eyelock_MaxTemplateCountDefault);
		}
	}

	const string Eyelock_SoftwareUpdateURLDefault = "https://eyelock.com/updates/nanonxtversioninfo.xml";
	string EyelockConfiguration::getEyelock_SoftwareUpdateURL()
	{
		return conf.getStr(string("Eyelock.SoftwareUpdateURL"), Eyelock_SoftwareUpdateURLDefault);
	}

	bool EyelockConfiguration::setEyelock_SoftwareUpdateURL(const string& newValue)
	{
		conf.setValue("Eyelock.SoftwareUpdateURL", newValue);
		return true;
	}

	void EyelockConfiguration::ensureEyelock_SoftwareUpdateURL()
	{
		if (!conf.has(string("Eyelock.SoftwareUpdateURL")))
		{
			conf.setValue("Eyelock.SoftwareUpdateURL", Eyelock_SoftwareUpdateURLDefault);
		}
	}

	const string Eyelock_SoftwareUpdateDateCheckDefault = "Never";
	string EyelockConfiguration::getEyelock_SoftwareUpdateDateCheck()
	{
		return conf.getStr(string("Eyelock.SoftwareUpdateDateCheck"), Eyelock_SoftwareUpdateDateCheckDefault);
	}

	bool EyelockConfiguration::setEyelock_SoftwareUpdateDateCheck(const string& newValue)
	{
		conf.setValue("Eyelock.SoftwareUpdateDateCheck", newValue);
		return true;
	}

	void EyelockConfiguration::ensureEyelock_SoftwareUpdateDateCheck()
	{
		if (!conf.has(string("Eyelock.SoftwareUpdateDateCheck")))
		{
			conf.setValue("Eyelock.SoftwareUpdateDateCheck", Eyelock_SoftwareUpdateDateCheckDefault);
		}
	}

	// TODO: update date Nano + Bob

	const int GRI_AuthorizationToneDurationSecondsDefault = 4;
	int EyelockConfiguration::getGRI_AuthorizationToneDurationSeconds()
	{
		return conf.getInt(string("GRI.AuthorizationToneDurationSeconds"), GRI_AuthorizationToneDurationSecondsDefault);
	}

	bool EyelockConfiguration::setGRI_AuthorizationToneDurationSeconds(int newValue)
	{
		if (!ParameterValidator::validateRange(newValue, 0, 60))
		{
			return false;
		}

		conf.setValue("GRI.AuthorizationToneDurationSeconds", newValue);

		return true;
	}

	void EyelockConfiguration::ensureGRI_AuthorizationToneDurationSeconds()
	{
		if (!conf.has(string("GRI.AuthorizationToneDurationSeconds")))
		{
			conf.setValue("GRI.AuthorizationToneDurationSeconds", GRI_AuthorizationToneDurationSecondsDefault);
		}
	}


	const string GRI_InternetTimeAddrDefault = "time.nist.gov";
	string EyelockConfiguration::getGRI_InternetTimeAddr()
	{
		return conf.getStr(string("GRI.InternetTimeAddr"), GRI_InternetTimeAddrDefault);
	}

	bool EyelockConfiguration::setGRI_InternetTimeAddr(const string& newValue)
	{
		conf.setValue("GRI.InternetTimeAddr", newValue);
		return true;
	}

	void EyelockConfiguration::ensureGRI_InternetTimeAddr()
	{
		if (!conf.has(string("GRI.InternetTimeAddr")))
		{
			conf.setValue("GRI.InternetTimeAddr", GRI_InternetTimeAddrDefault);
		}
	}

	const bool GRI_InternetTimeSyncDefault = true;
	bool EyelockConfiguration::getGRI_InternetTimeSync()
	{
		return conf.getBool(string("GRI.InternetTimeSync"), GRI_InternetTimeSyncDefault);
	}

	bool EyelockConfiguration::setGRI_InternetTimeSync(bool newValue)
	{
		conf.setValue("GRI.InternetTimeSync", newValue);
		return true;
	}

	void EyelockConfiguration::ensureGRI_InternetTimeSync()
	{
		if (!conf.has(string("GRI.InternetTimeSync")))
		{
			conf.setValue("GRI.InternetTimeSync", GRI_InternetTimeSyncDefault);
		}
	}

	const string GRI_EyeDestAddrDefault = "";
	string EyelockConfiguration::getGRI_EyeDestAddr()
	{
		return conf.getStr(string("GRI.EyeDestAddr"), GRI_EyeDestAddrDefault);
	}

	bool EyelockConfiguration::setGRI_EyeDestAddr(const string& newValue)
	{
		conf.setValue("GRI.EyeDestAddr", newValue);
		return true;
	}

	void EyelockConfiguration::ensureGRI_EyeDestAddr()
	{
		if (!conf.has(string("GRI.EyeDestAddr")))
		{
			conf.setValue("GRI.EyeDestAddr", GRI_EyeDestAddrDefault);
		}
	}

	const bool Eyelock_EnablePopupHelpDefault = true;
	bool EyelockConfiguration::getEyelock_EnablePopupHelp()
	{
		return conf.getBool(string("Eyelock.EnablePopupHelp"), Eyelock_EnablePopupHelpDefault);
	}

	bool EyelockConfiguration::setEyelock_EnablePopupHelp(bool newValue)
	{
		conf.setValue("Eyelock.EnablePopupHelp", newValue);
		return true;
	}

	void EyelockConfiguration::ensureEyelock_EnablePopupHelp()
	{
		if (!conf.has(string("Eyelock.EnablePopupHelp")))
		{
			conf.setValue("Eyelock.EnablePopupHelp", Eyelock_EnablePopupHelpDefault);
		}
	}

	const bool Eyelock_PopupHelpTriggerHoverDefault = true;
	bool EyelockConfiguration::getEyelock_PopupHelpTriggerHover()
	{
		return conf.getBool(string("Eyelock.PopupHelpTriggerHover"), Eyelock_PopupHelpTriggerHoverDefault);
	}

	bool EyelockConfiguration::setEyelock_PopupHelpTriggerHover(bool newValue)
	{
		int storedValue = (newValue) ? 1 : 0;
		conf.setValue("Eyelock.PopupHelpTriggerHover", storedValue);
		return true;
	}

	void EyelockConfiguration::ensureEyelock_PopupHelpTriggerHover()
	{
		if (!conf.has(string("Eyelock.PopupHelpTriggerHover")))
		{
			conf.setValue("Eyelock.PopupHelpTriggerHover", Eyelock_PopupHelpTriggerHoverDefault);
		}
	}

	const int Eyelock_PopupHelpDelay = 1;
	int EyelockConfiguration::getEyelock_PopupHelpDelay()
	{
		return conf.getInt(string("Eyelock.PopupHelpDelay"), Eyelock_PopupHelpDelay)/1000;
	}

	bool EyelockConfiguration::setEyelock_PopupHelpDelay(int newValue)
	{
		if (!ParameterValidator::validateRange(newValue, 0, 60))
		{
			return false;
		}

		conf.setValue("Eyelock.PopupHelpDelay", newValue*1000);

		return true;
	}

	void EyelockConfiguration::ensureEyelock_PopupHelpDelay()
	{
		if (!conf.has(string("Eyelock.PopupHelpDelay")))
		{
			conf.setValue("Eyelock.PopupHelpDelay", Eyelock_PopupHelpDelay);
		}
	}

	const bool Eyelock_AllowSiteAdminUpgradeDefault = false;
	bool EyelockConfiguration::getEyelock_AllowSiteAdminUpgrade()
	{
		return conf.getBool(string("Eyelock.AllowSiteAdminUpgrade"), Eyelock_AllowSiteAdminUpgradeDefault);
	}

	bool EyelockConfiguration::setEyelock_AllowSiteAdminUpgrade(bool newValue)
	{
		conf.setValue("Eyelock.AllowSiteAdminUpgrade", newValue);
		return true;
	}

	void EyelockConfiguration::ensureEyelock_AllowSiteAdminUpgrade()
	{
		if (!conf.has(string("Eyelock.AllowSiteAdminUpgrade")))
		{
			conf.setValue("Eyelock.AllowSiteAdminUpgrade", Eyelock_AllowSiteAdminUpgradeDefault);
		}
	}

	// ******************************************************************************

	bool EyelockConfiguration::updateFromFile(const string& filepath)
	{
		EyelockLog(logger, DEBUG, "Updating from file: %s", filepath.c_str());

		if (!FileConfiguration::checkFile(filepath))
		{
			return false;
		}

		FileConfiguration updateConf(filepath);

		AbstractConfiguration::Keys keys;
		updateConf.keys(keys);

		// TODO: more efficient

		AbstractConfiguration::KeysCIter itLoop = keys.begin();
		AbstractConfiguration::KeysCIter it;
		while (itLoop != keys.end())
		{
			it = itLoop++;

			if (*it == "GRI.AuthorizationToneVolume")
			{
				if (setGRI_AuthorizationToneVolume(updateConf.getInt("GRI.AuthorizationToneVolume")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRI.LEDBrightness")
			{
				if (setGRI_LEDBrightness(updateConf.getInt("GRI.LEDBrightness")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRI.TamperToneVolume")
			{
				if (setGRI_TamperToneVolume(updateConf.getInt("GRI.TamperToneVolume")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.TamperSignalHighToLow")
			{
				if (setEyelock_TamperSignalHighToLow(updateConf.getBool("Eyelock.TamperSignalHighToLow")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.TamperOutSignalHighToLow")
			{
				if (setEyelock_TamperOutSignalHighToLow(updateConf.getBool("Eyelock.TamperOutSignalHighToLow")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.DualMatcherPolicy")
			{
				if (setEyelock_DualMatcherPolicy(updateConf.getBool("Eyelock.DualMatcherPolicy")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.EnableNegativeMatchTimeout")
			{
				if (setEyelock_EnableNegativeMatchTimeout(updateConf.getBool("Eyelock.EnableNegativeMatchTimeout")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.NegativeMatchTimeout")
			{
				if (setEyelock_NegativeMatchTimeout(updateConf.getInt("Eyelock.NegativeMatchTimeout")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.NegativeMatchResetTimer")
			{
				if (setEyelock_NegativeMatchResetTimer(updateConf.getInt("Eyelock.NegativeMatchResetTimer")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRI.HDMatcher.1.Address")
			{
				if (setGRI_HDMatcher_1_Address(updateConf.getCStr("GRI.HDMatcher.1.Address")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.WeigandEnable")
			{
				if (setGRITrigger_WeigandEnable(updateConf.getBool("GRITrigger.WeigandEnable")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.WeigandHidEnable")
			{
				if (setGRITrigger_WeigandHidEnable(updateConf.getBool("GRITrigger.WeigandHidEnable")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.PACEnable")
			{
				if (setGRITrigger_PACEnable(updateConf.getBool("GRITrigger.PACEnable")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.F2FEnable")
			{
				if (setGRITrigger_F2FEnable(updateConf.getBool("GRITrigger.F2FEnable")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.OSDPEnable")
			{
				if (setGRITrigger_OSDPEnable(updateConf.getBool("GRITrigger.OSDPEnable")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.OSDPAddress")
			{
				if (setEyelock_OSDPAddress(updateConf.getInt("Eyelock.OSDPAddress")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.OSDPBaudRate")
			{
				if (setEyelock_OSDPBaudRate(updateConf.getInt("Eyelock.OSDPBaudRate")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.DualAuthNLedControlledByACS")
			{
				if (setGRITrigger_DualAuthNLedControlledByACS(updateConf.getBool("GRITrigger.DualAuthNLedControlledByACS")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.DualAuthenticationParity")
			{
				if (setGRITrigger_DualAuthenticationParity(updateConf.getBool("GRITrigger.DualAuthenticationParity")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.DualAuthNCardMatchWaitIrisTime")
			{
				if (setGRITrigger_DualAuthNCardMatchWaitIrisTime(updateConf.getInt("GRITrigger.DualAuthNCardMatchWaitIrisTime")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.WaitPinTime")
			{
				if (setEyelock_WaitPinTime(updateConf.getInt("Eyelock.WaitPinTime")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.PinBurstBits")
			{
				if (setEyelock_PinBurstBits(updateConf.getInt("Eyelock.PinBurstBits")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.EnableRelayWithSignal")
			{
				if (setGRITrigger_EnableRelayWithSignal(updateConf.getBool("GRITrigger.EnableRelayWithSignal")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.RelayTimeInMS")
			{
				if (setGRITrigger_RelayTimeInMS(updateConf.getInt("GRITrigger.RelayTimeInMS")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.DenyRelayTimeInMS")
			{
				if (setGRITrigger_DenyRelayTimeInMS(updateConf.getInt("GRITrigger.DenyRelayTimeInMS")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.TLSEnable")
			{
				if (setEyelock_TLSEnable(updateConf.getBool("Eyelock.TLSEnable")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.AuthenticationMode")
			{
				unsigned int matchMode = updateConf.getUInt("Eyelock.AuthenticationMode");
				if (setAuthScheme(matchMode))
				{
					updateConf.remove(*it);
				}
			}

			// additional "indirect" parameters
			else if (*it == "HighLevel.Protocol")
			{
				Protocol prot = strToProtocol(updateConf.getStr("HighLevel.Protocol").c_str());
				if (setProtocol(prot))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "HighLevel.NetworkMatcherAddress")
			{
				if (setNetworkMatcherAddress(updateConf.getStr("HighLevel.NetworkMatcherAddress")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "HighLevel.NetworkMatcherEnabled")
			{
				if (enableNetworkMatcher(updateConf.getBool("HighLevel.NetworkMatcherEnabled")))
				{
					updateConf.remove(*it);
				}
			}

			// ***** WebConfig specific

			else if (*it == "GRI.MatchResultDestAddr")
			{
				if (setGRI_MatchResultDestAddr(updateConf.getStr("GRI.MatchResultDestAddr")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRI.MatchResultNwMsgFormat")
			{
				if (setGRI_MatchResultNwMsgFormat(updateConf.getStr("GRI.MatchResultNwMsgFormat")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRI.NwDispatcherSecure")
			{
				if (setGRI_NwDispatcherSecure(updateConf.getStr("GRI.NwDispatcherSecure")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.PortableTemplatesUseCustomKey")
			{
				if (setGRITrigger_PortableTemplatesUseCustomKey(updateConf.getBool("GRITrigger.PortableTemplatesUseCustomKey")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.SystemReadyDebug")
			{
				if (setEyelock_SystemReadyDebug(updateConf.getBool("Eyelock.SystemReadyDebug")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRI.EyeConnecTimeoutmsec")
			{
				if (setGRI_EyeConnecTimeoutmsec(updateConf.getInt("GRI.EyeConnecTimeoutmsec")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRI.EyeSendTimeoutmsec")
			{
				if (setGRI_EyeSendTimeoutmsec(updateConf.getInt("GRI.EyeSendTimeoutmsec")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.MobileMode")
			{
				if (setGRITrigger_MobileMode(updateConf.getBool("GRITrigger.MobileMode")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.TransTOCMode")
			{
				if (setGRITrigger_TransTOCMode(updateConf.getBool("GRITrigger.TransTOCMode")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRITrigger.TOCCardExpiredTime")
			{
				if (setGRITrigger_TOCCardExpiredTime(updateConf.getInt("GRITrigger.TOCCardExpiredTime")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.ACPCardID")
			{
				if (setEyelock_ACPCardID(updateConf.getStr("Eyelock.ACPCardID")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.ACPTestFacilityCode")
			{
				if (setEyelock_ACPTestFacilityCode(updateConf.getStr("Eyelock.ACPTestFacilityCode")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.TamperNotifyAddr")
			{
				if (setEyelock_TamperNotifyAddr(updateConf.getStr("Eyelock.TamperNotifyAddr")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.TamperNotifyMessage")
			{
				if (setEyelock_TamperNotifyMessage(updateConf.getStr("Eyelock.TamperNotifyMessage")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.MaxTemplateCount")
			{
				if (setEyelock_MaxTemplateCount(updateConf.getInt("Eyelock.MaxTemplateCount")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.SoftwareUpdateURL")
			{
				if (setEyelock_SoftwareUpdateURL(updateConf.getStr("Eyelock.SoftwareUpdateURL")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.SoftwareUpdateDateCheck")
			{
				if (setEyelock_SoftwareUpdateDateCheck(updateConf.getStr("Eyelock.SoftwareUpdateDateCheck")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRI.AuthorizationToneDurationSeconds")
			{
				if (setGRI_AuthorizationToneDurationSeconds(updateConf.getInt("GRI.AuthorizationToneDurationSeconds")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRI.InternetTimeAddr")
			{
				if (setGRI_InternetTimeAddr(updateConf.getStr("GRI.InternetTimeAddr")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRI.InternetTimeSync")
			{
				if (setGRI_InternetTimeSync(updateConf.getBool("GRI.InternetTimeSync")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "GRI.EyeDestAddr")
			{
				if (setGRI_EyeDestAddr(updateConf.getStr("GRI.EyeDestAddr")))
				{
					updateConf.remove(*it);
				}
			}

			// WebConfig specific
			// TODO: check and remove
			else if (*it == "Eyelock.EnablePopupHelp")
			{
				if (setEyelock_EnablePopupHelp(updateConf.getBool("Eyelock.EnablePopupHelp")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.PopupHelpTriggerHover")
			{
				if (setEyelock_PopupHelpTriggerHover(updateConf.getBool("Eyelock.PopupHelpTriggerHover")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.PopupHelpDelay")
			{
				if (setEyelock_PopupHelpDelay(updateConf.getInt("Eyelock.PopupHelpDelay")))
				{
					updateConf.remove(*it);
				}
			}
			else if (*it == "Eyelock.AllowSiteAdminUpgrade")
			{
				if (setEyelock_AllowSiteAdminUpgrade(updateConf.getBool("Eyelock.AllowSiteAdminUpgrade")))
				{
					updateConf.remove(*it);
				}
			}
		}

		updateConf.save(filepath);

		return true;
	}

	bool EyelockConfiguration::save(string filepath)
	{
		EyelockLog(logger, DEBUG, "Saving configuration");
		addDefaultsForRequired();
		if (filepath.empty())
		{
			filepath = filename;
		}

		if (filename.empty())
		{
			return false;
		}

		bool updateResult = false;

		if (acquireLock())
		{
			EyelockLog(logger, DEBUG, "Saving configuration to %s", filepath.c_str());
			updateResult = conf.update(filepath);
			releaseLock();
		}
		else
		{
			EyelockLog(logger, ERROR, "Failed to acquire lock for %s", filepath.c_str());
		}

		return updateResult;
	}



	// ******************************************************************************
	// SDK specific


	// ******************************************************************************


} // namespace EyelockConfigurationNS


