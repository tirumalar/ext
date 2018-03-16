#ifndef ELKNS_SupportedConfigParams_H
#define ELKNS_SupportedConfigParams_H

typedef enum _ELKNS_SupportedConfigParams
{	/// Speaker Volume.
	CONFIG_SPEAKERVOLUME								= 1,
	/// Led Brightness.
	CONFIG_LEDBRIGHTNESS								= 2,
	/// Tamper Tone Volume.
	CONFIG_TAMPERTONEVOLUME								= 3,
	/// Card Reader Tamper Settings. Activate Tamper State on Signal High. TRUE/true/FALSE/false values are accepted.
	CONFIG_TAMPERSIGNALHIGH								= 4,
	/// Authentication. Matching: Use Both Eyes. TRUE/true/FALSE/false values are accepted.
	CONFIG_DUALMATCHERPOLICY							= 5,
	/// Repeat Authorization Period.
	CONFIG_REPEATAUTHORIZATIONPERIOD					= 6,
	/// Enable Negative Match Timeout. TRUE/true/FALSE/false values are accepted.
	CONFIG_ENABLENEGATIVEMATCHTIMEOUT					= 7,
	/// Negative Match Timeout.
	CONFIG_NEGATIVEMATCHTIMEOUT							= 8,
	/// Negative Match Reset Timer.
	CONFIG_NEGATIVEMATCHRESETTIMER						= 9,
	/// Network Matcher. Address and port (in format xxx.xxx.xxx.xxx:xxxx) to enable, otherwise disable.
	CONFIG_HDMATCHER_ADDRESS							= 10,
	/// Protocol. WIEGAND, OSDP, HID, PAC, F2F are accepted.
	CONFIG_PROTOCOL										= 11,
	/// OSDP Baud Rate.
	CONFIG_OSDPBAUDRATE									= 12,
	/// OSDP Address.
	CONFIG_OSDPADDRESS									= 13,
	/// Led Controlled by ACS. TRUE/true/FALSE/false values are accepted.
	CONFIG_LEDCONTROLLEDBYACS							= 14,
	/// Dual Authentication Mode. TRUE/true/FALSE/false values are accepted.
	CONFIG_DUALAUTHENTICATIONMODE						= 15,
	/// Dual Authentication Parity. TRUE/true/FALSE/false values are accepted.
	CONFIG_DUALAUTHENTICATIONPARITY						= 16,
	/// Dual Authentication Iris Wait Time
	CONFIG_DUALAUTHNCARDMATCHWAITIRISTIME				= 17,
	/// Relay Enable. TRUE/true/FALSE/false values are accepted.
	CONFIG_RELAYENABLE									= 18,
	/// Grant Relay Time.
	CONFIG_GRANTRELAYTIME								= 19,
	/// Deny Relay Time.
	CONFIG_DENYRELAYTIME								= 20,
	/// Force TLS v1.2.
	CONFIG_FORCETLS12									= 21,
	/// Tamper Output Settings. Signal High. TRUE/true/FALSE/false values are accepted.
	CONFIG_TAMPEROUTSIGNALHIGH							= 22,
	/// Pass through mode. TRUE/true/FALSE/false values are accepted.
	CONFIG_PASSTHROUGHMODE								= 23,

} ELKNS_SupportedConfigParams;

enum ELKNS_Status_T
{
	SUCCESS													= 0,
	UNSUCCESSFUL											= 1,
	NO_DEVICE_FOUND											= 2,
	CONNECTION_REFUSED										= 3,
	BUFFER_ALREADY_NULL										= 4,
	INVALID_INPUT_DATA										= 5,
	MEMORY_ALLOCATION_FAIL									= 6,
	FILE_DOES_NOT_EXIST										= 7,
	EYE_SEGMENTATION_FAILED									= 8,
	FILE_ACCESS_ERROR										= 9,
	DATABASE_ERROR											= 10,
	NO_RECORD_FOUND											= 11,
	INPUT_ERR												= 12,
	FLOW_ERR												= 13,
	DEVICE_WAS_TAMPERED										= 14,
	DEVICE_WAS_NOT_TAMPERED									= 15,
	ACCESS_DENIED											= 16,
	NO_DATA_FOUND											= 17,
	WRONG_GUID_FORMAT										= 18,
	WRONG_ACD_DATA_TYPE										= 19,
	CONNECTION_FLOW_ERROR									= 20,
	SLAVE_FILE_DOES_NOT_EXIST								= 21,
	SLAVE_UNSUCCESSFUL										= 22,
	SLAVE_CONNECTION_FLOW_ERROR								= 23,
	OLD_VERSION												= 24,
	FILE_CORRUPTED											= 25,
	NOT_IMPLEMENTED											= 26,
	UID_ALREADY_EXISTS										= 27,
	LICENSE_NOT_INITIALIZED									= 28,
	LICENSE_INVALID											= 29,
	SAMPLE_LICENSE_USE_EXCEEDED								= 30,
	NO_EYE_FOUND											= 31,
	UNSUCCESSFUL_ONLY_ONE_EYE_FOUND_FOR_DUAL_EYE_ENROLLMENT = 32,
	KEY_NOT_SET												= 33,
	INVALID_KEY												= 34,
	CUSTOM_KEY_ALREADY_USED									= 35,
	ACSTESTDATA_IS_SET										= 36,
	ACSTESTDATA_IS_NOT_SET									= 37,
	SOCKET_INIT_FAILED										= 38,
	DATABASE_CORRUPTED										= 39,
	NOT_SUPPORTED											= 40,


	//More to be added
};

#endif // ELKNS_SupportedConfigParams_H
