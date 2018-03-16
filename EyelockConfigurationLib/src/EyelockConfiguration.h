/*
 * EyelockConfiguration.h
 *
 *  Created on: Apr 11, 2017
 *      Author: builder
 */

#ifndef EYELOCKCONFIGURATION_H_
#define EYELOCKCONFIGURATION_H_

#include <string>
#include <vector>
#include <map>
#include <typeinfo>

#include "FileConfiguration.h"
#include "MatchType.h" // requires to add NwHDMatcher, BiOmega, OpenCV etc. Move enum MatchMode to separate .h?
// required for:
// enum MatchMode{IRIS_ONLY=1, CARD_OR_IRIS, CARD_AND_IRIS, CARD_AND_IRIS_PIN_PASS, PIN_AND_IRIS, CARD_AND_PIN_AND_IRIS}; // copied from MatchMode.h

namespace EyelockConfigurationNS
{
	class EyelockConfiguration
	{
	public:

		EyelockConfiguration();
		EyelockConfiguration(std::string filepath);
		virtual ~EyelockConfiguration() {};

		void loadFromFile(std::string filepath);

		bool updateFromFile(const std::string& filepath);
		bool reset();
		bool save(std::string filepath = "");

		static const std::string eyelockIniFile;
		static const std::string eyelockDefaultIniFile;

	protected:
		std::string filename;

		void addDefaultsForRequired();

		int acquireLock();
		int releaseLock();
		int handleLock(short int lType);

		FileConfiguration conf;

		static const std::string tempIniFile;

	public:

		// UInt
		typedef bool (EyelockConfiguration::*SetUIntFuncPointer)(unsigned int);
		typedef unsigned int (EyelockConfiguration::*GetUIntFuncPointer)(void);

		class UIntParamInfo
		{
			public:
				UIntParamInfo(const std::string& _sdkParameterName, SetUIntFuncPointer _pSetter, GetUIntFuncPointer _pGetter) : sdkParameterName(_sdkParameterName), pSetter(_pSetter), pGetter(_pGetter) {};

				std::string sdkParameterName;

				SetUIntFuncPointer pSetter;
				GetUIntFuncPointer pGetter;

				bool operator==(const UIntParamInfo& compared) const
				{
					return sdkParameterName == compared.sdkParameterName;
				}
		};

		int getUIntParameter(const std::string& key, unsigned int& value);
		int setUIntParameter(const std::string& paramName, const unsigned int value);

		// float
		typedef bool (EyelockConfiguration::*SetFloatFuncPointer)(float);
		typedef float (EyelockConfiguration::*GetFloatFuncPointer)(void);

		class FloatParamInfo
		{
			public:
				FloatParamInfo(const std::string& _sdkParameterName, SetFloatFuncPointer _pSetter, GetFloatFuncPointer _pGetter) : sdkParameterName(_sdkParameterName), pSetter(_pSetter), pGetter(_pGetter) {};

				std::string sdkParameterName;

				SetFloatFuncPointer pSetter;
				GetFloatFuncPointer pGetter;

				bool operator==(const FloatParamInfo& compared) const
				{
					return sdkParameterName == compared.sdkParameterName;
				}
		};

		int getFloatParameter(const std::string& key, float& value);
		int setFloatParameter(const std::string& paramName, const float value);

		// bool
		typedef bool (EyelockConfiguration::*SetBoolFuncPointer)(bool);
		typedef bool (EyelockConfiguration::*GetBoolFuncPointer)(void);

		class BoolParamInfo
		{
			public:
				BoolParamInfo(const std::string& _sdkParameterName, SetBoolFuncPointer _pSetter, GetBoolFuncPointer _pGetter) : sdkParameterName(_sdkParameterName), pSetter(_pSetter), pGetter(_pGetter) {};

				std::string sdkParameterName;

				SetBoolFuncPointer pSetter;
				GetBoolFuncPointer pGetter;

				bool operator==(const BoolParamInfo& compared) const
				{
					return sdkParameterName == compared.sdkParameterName;
				}
		};

		int getBoolParameter(const std::string& key, bool& value);
		int setBoolParameter(const std::string& paramName, const bool value);

		// string
		typedef bool (EyelockConfiguration::*SetStrFuncPointer)(const std::string&);
		typedef std::string (EyelockConfiguration::*GetStrFuncPointer)(void);

		class StrParamInfo
		{
			public:
				StrParamInfo(const std::string& _sdkParameterName, SetStrFuncPointer _pSetter, GetStrFuncPointer _pGetter) : sdkParameterName(_sdkParameterName), pSetter(_pSetter), pGetter(_pGetter) {};

				std::string sdkParameterName;

				SetStrFuncPointer pSetter;
				GetStrFuncPointer pGetter;

				bool operator==(const StrParamInfo& compared) const
				{
					return sdkParameterName == compared.sdkParameterName;
				}
		};

		int getStrParameter(const std::string& key, std::string& value);
		int setStrParameter(const std::string& paramName, const std::string& value);

		// for backward compatibility with legacy SDK config parameters handling: API <string>, <string> (like "CONFIG_SPEAKERVOLUME", "12")
		int setParameter(const std::string& key, const std::string& value);
		int setParameter(int32_t intAlias, const std::string& value);
		int setParametersMap(const std::map<int32_t, std::string> & inputParametersMap);
		int getAllParameters(std::map<int32_t, std::string>&);

	protected:

		std::vector<UIntParamInfo> uintParameters;
		std::vector<FloatParamInfo> floatParameters;
		std::vector<BoolParamInfo> boolParameters;
		std::vector<StrParamInfo> strParameters;

		std::map<std::string, int32_t> intAliases;

		void setSdkAliases();
		void setLegacyAliases();

	public:

		// ************** Parameters ****************************************************
		float getGRI_AuthorizationToneVolume();
		bool setGRI_AuthorizationToneVolume(float newValue);
		void ensureGRI_AuthorizationToneVolume();

		float getGRI_LEDBrightness();
		bool setGRI_LEDBrightness(float newValue);
		void ensureGRI_LEDBrightness();

		float getGRI_TamperToneVolume();
		bool setGRI_TamperToneVolume(float newValue);
		void ensureGRI_TamperToneVolume();

		bool getEyelock_TamperSignalHighToLow();
		bool setEyelock_TamperSignalHighToLow(bool newValue);
		void ensureEyelock_TamperSignalHighToLow();

		// ***** wrappers for Eyelock.TamperSignalHighToLow parameter used by SDK *******
		bool getTamperSignalHigh();
		bool setTamperSignalHigh(bool newValue);
		void ensureTamperSignalHigh();
		// ****** ******

		bool getEyelock_TamperOutSignalHighToLow();
		bool setEyelock_TamperOutSignalHighToLow(bool newValue);
		void ensureEyelock_TamperOutSignalHighToLow();

		// ***** wrappers for Eyelock.TamperOutSignalHighToLow parameter used by SDK *******
		bool getTamperOutSignalHigh();
		bool setTamperOutSignalHigh(bool newValue);
		void ensureTamperOutSignalHigh();
		// ****** ******

		bool getEyelock_DualMatcherPolicy();
		bool setEyelock_DualMatcherPolicy(bool newValue);
		void ensureEyelock_DualMatcherPolicy();

		unsigned int getGRI_RepeatAuthorizationPeriod();
		bool setGRI_RepeatAuthorizationPeriod(unsigned int newValue);
		void ensureGRI_RepeatAuthorizationPeriod();

		bool getEyelock_EnableNegativeMatchTimeout();
		bool setEyelock_EnableNegativeMatchTimeout(bool newValue);
		void ensureEyelock_EnableNegativeMatchTimeout();

		unsigned int getEyelock_NegativeMatchTimeout();
		bool setEyelock_NegativeMatchTimeout(unsigned int newValue);
		void ensureEyelock_NegativeMatchTimeout();

		unsigned int getEyelock_NegativeMatchResetTimer();
		bool setEyelock_NegativeMatchResetTimer(unsigned int newValue);
		void ensureEyelock_NegativeMatchResetTimer();

		// ****** Matchers settings (including Network Matcher) ******
		bool setNetworkMatcherAddress(const std::string& host, const unsigned short port);
		bool setNetworkMatcherAddress(const std::string& endpoint);
		std::string getNetworkMatcherAddress();

		bool enableNetworkMatcher(bool newValue);
		bool isNetworkMatcherEnabled();

		int getGRI_HDMatcherID();
		int getGRI_HDMatcherCount();
		const char* getGRI_HDMatcher_2_Address();
		const char* getGRI_HDMatcher_2_Type();
		int getGRI_HDMatcher_2_BuffSize();

	protected:

		bool setGRI_HDMatcherID(int newValue);
		bool setGRI_HDMatcherCount(int newValue);
		bool setGRI_HDMatcher_2_Address(const char* newValue);
		bool setGRI_HDMatcher_2_Type(const char* newValue);
		bool setGRI_HDMatcher_2_BuffSize(int newValue);

		void ensureGRI_HDMatcherID();
		void ensureGRI_HDMatcherCount();
		void ensureGRI_HDMatcher_2_Address();
		void ensureGRI_HDMatcher_2_Type();
		void ensureGRI_HDMatcher_2_BuffSize();

		void ensureNetworkMatcher();
		// ****** ******

	public:

		// ****** Protocols ******
		enum Protocol { WIEGAND, HID, PAC, F2F, OSDP, UNKNOWN_PROTOCOL };

		Protocol strToProtocol(const char *protocolCStr);
		std::string protocolToStr(Protocol protocol);

		Protocol getProtocol();
		std::string getProtocolStr()
		{
			return protocolToStr(getProtocol());
		}

		bool setProtocol(Protocol newValue);
		bool setProtocol(const std::string& newValue)
		{
			return setProtocol(strToProtocol(newValue.c_str()));
		}

		void ensureProtocol();

		bool getGRITrigger_WeigandEnable();
		bool getGRITrigger_WeigandHidEnable();
		bool getGRITrigger_PACEnable();
		bool getGRITrigger_F2FEnable();
		bool getGRITrigger_OSDPEnable();

		unsigned int getEyelock_OSDPAddress();
		bool setEyelock_OSDPAddress(unsigned int newValue);
		void ensureEyelock_OSDPAddress();

		unsigned int getEyelock_OSDPBaudRate();
		bool setEyelock_OSDPBaudRate(unsigned int newValue);
		void ensureEyelock_OSDPBaudRate();

	protected:

		bool setGRITrigger_WeigandEnable(bool newValue);
		void ensureGRITrigger_WeigandEnable();

		bool setGRITrigger_WeigandHidEnable(bool newValue);
		void ensureGRITrigger_WeigandHidEnable();

		bool setGRITrigger_PACEnable(bool newValue);
		void ensureGRITrigger_PACEnable();

		bool setGRITrigger_F2FEnable(bool newValue);
		void ensureGRITrigger_F2FEnable();

		bool setGRITrigger_OSDPEnable(bool newValue);
		void ensureGRITrigger_OSDPEnable();

		// ******  ******

	public:

		bool getGRITrigger_DualAuthNLedControlledByACS();
		bool setGRITrigger_DualAuthNLedControlledByACS(bool newValue);
		void ensureGRITrigger_DualAuthNLedControlledByACS();

		// ******  Auth Scheme ******
		static unsigned int strToMatchMode(const char *matchModeCStr);
		static std::string matchModeToStr(MatchMode matchMode);

		MatchMode getAuthScheme();
		unsigned int getAuthSchemeUInt();
		std::string getAuthSchemeStr()
		{
			return matchModeToStr(getAuthScheme());
		}

		bool setAuthScheme(MatchMode newValue);
		bool setAuthScheme(const unsigned int newValue);
		bool setAuthScheme(const std::string& newValue);

		void ensureAuthScheme();

		void getDualAuthAndPassThrough(bool& dualAuth, bool& passThrough);

		bool getGRITrigger_DualAuthenticationMode();
		bool getGRITrigger_PassThroughMode();

	protected:

		bool setGRITrigger_DualAuthenticationMode(bool newValue);
		bool setGRITrigger_PassThroughMode(bool newValue);
		// ******  ******

	public:

		bool getGRITrigger_DualAuthenticationParity();
		bool setGRITrigger_DualAuthenticationParity(bool newValue);
		void ensureGRITrigger_DualAuthenticationParity();

		unsigned int getGRITrigger_DualAuthNCardMatchWaitIrisTime();
		bool setGRITrigger_DualAuthNCardMatchWaitIrisTime(unsigned int newValue);
		void ensureGRITrigger_DualAuthNCardMatchWaitIrisTime();

		unsigned int getEyelock_WaitPinTime();
		bool setEyelock_WaitPinTime(unsigned int newValue);
		void ensureEyelock_WaitPinTime();

		unsigned int getEyelock_PinBurstBits();
		bool setEyelock_PinBurstBits(unsigned int newValue);
		void ensureEyelock_PinBurstBits();

		bool getGRITrigger_EnableRelayWithSignal();
		bool setGRITrigger_EnableRelayWithSignal(bool newValue);
		void ensureGRITrigger_EnableRelayWithSignal();

		unsigned int getGRITrigger_RelayTimeInMS();
		bool setGRITrigger_RelayTimeInMS(unsigned int newValue);
		void ensureGRITrigger_RelayTimeInMS();

		unsigned int getGRITrigger_DenyRelayTimeInMS();
		bool setGRITrigger_DenyRelayTimeInMS(unsigned int newValue);
		void ensureGRITrigger_DenyRelayTimeInMS();

		bool getEyelock_TLSEnable();
		bool setEyelock_TLSEnable(bool newValue);
		void ensureEyelock_TLSEnable();

		// ***** WebConfig specific

		std::string getGRI_MatchResultDestAddr();
		bool setGRI_MatchResultDestAddr(const std::string& newValue);
		void ensureGRI_MatchResultDestAddr();

		std::string getGRI_MatchResultNwMsgFormat();
		bool setGRI_MatchResultNwMsgFormat(const std::string& newValue);
		void ensureGRI_MatchResultNwMsgFormat();

		std::string getGRI_NwDispatcherSecure();
		bool setGRI_NwDispatcherSecure(const std::string& newValue);
		void ensureGRI_NwDispatcherSecure();

		bool getGRITrigger_PortableTemplatesUseCustomKey();
		bool setGRITrigger_PortableTemplatesUseCustomKey(bool newValue);
		void ensureGRITrigger_PortableTemplatesUseCustomKey();

		bool getEyelock_SystemReadyDebug();
		bool setEyelock_SystemReadyDebug(bool newValue);
		void ensureEyelock_SystemReadyDebug();

		int getGRI_EyeConnecTimeoutmsec();
		bool setGRI_EyeConnecTimeoutmsec(int newValue);
		void ensureGRI_EyeConnecTimeoutmsec();

		int getGRI_EyeSendTimeoutmsec();
		bool setGRI_EyeSendTimeoutmsec(int newValue);
		void ensureGRI_EyeSendTimeoutmsec();

		bool getGRITrigger_MobileMode();
		bool setGRITrigger_MobileMode(bool newValue);
		void ensureGRITrigger_MobileMode();

		bool getGRITrigger_TransTOCMode();
		bool setGRITrigger_TransTOCMode(bool newValue);
		void ensureGRITrigger_TransTOCMode();

		int getGRITrigger_TOCCardExpiredTime();
		bool setGRITrigger_TOCCardExpiredTime(int newValue);
		void ensureGRITrigger_TOCCardExpiredTime();

		std::string getEyelock_ACPCardID();
		bool setEyelock_ACPCardID(const std::string& newValue);
		void ensureEyelock_ACPCardID();

		std::string getEyelock_ACPTestFacilityCode();
		bool setEyelock_ACPTestFacilityCode(const std::string& newValue);
		void ensureEyelock_ACPTestFacilityCode();

		std::string getEyelock_TamperNotifyAddr();
		bool setEyelock_TamperNotifyAddr(const std::string& newValue);
		void ensureEyelock_TamperNotifyAddr();

		std::string getEyelock_TamperNotifyMessage();
		bool setEyelock_TamperNotifyMessage(const std::string& newValue);
		void ensureEyelock_TamperNotifyMessage();

		int getEyelock_MaxTemplateCount();
		bool setEyelock_MaxTemplateCount(int newValue);
		void ensureEyelock_MaxTemplateCount();

		std::string getEyelock_SoftwareUpdateURL();
		bool setEyelock_SoftwareUpdateURL(const std::string& newValue);
		void ensureEyelock_SoftwareUpdateURL();

		std::string getEyelock_SoftwareUpdateDateCheck();
		bool setEyelock_SoftwareUpdateDateCheck(const std::string& newValue);
		void ensureEyelock_SoftwareUpdateDateCheck();

		int getGRI_AuthorizationToneDurationSeconds();
		bool setGRI_AuthorizationToneDurationSeconds(int newValue);
		void ensureGRI_AuthorizationToneDurationSeconds();

		std::string getGRI_InternetTimeAddr();
		bool setGRI_InternetTimeAddr(const std::string& newValue);
		void ensureGRI_InternetTimeAddr();

		bool getGRI_InternetTimeSync();
		bool setGRI_InternetTimeSync(bool newValue);
		void ensureGRI_InternetTimeSync();

		std::string getGRI_EyeDestAddr();
		bool setGRI_EyeDestAddr(const std::string& newValue);
		void ensureGRI_EyeDestAddr();

		bool getEyelock_EnablePopupHelp();
		bool setEyelock_EnablePopupHelp(bool newValue);
		void ensureEyelock_EnablePopupHelp();

		bool getEyelock_PopupHelpTriggerHover();
		bool setEyelock_PopupHelpTriggerHover(bool newValue);
		void ensureEyelock_PopupHelpTriggerHover();

		int getEyelock_PopupHelpDelay();
		bool setEyelock_PopupHelpDelay(int newValue);
		void ensureEyelock_PopupHelpDelay();

		bool getEyelock_AllowSiteAdminUpgrade();
		bool setEyelock_AllowSiteAdminUpgrade(bool newValue);
		void ensureEyelock_AllowSiteAdminUpgrade();

		// ******************************************************************************

	};

}

#endif /* EYELOCKCONFIGURATION_H_ */
