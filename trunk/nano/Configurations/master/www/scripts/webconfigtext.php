<?php
//trigger a build!
function defineStrings($HardwareType)
{
	$NXTText = "NanoNXT";
	if($HardwareType == '1')
		$NXTText = "NanoEXT";
    else if ($HardwareType == '2')
        $NXTText = "HBOX";
	
    switch($_SESSION['lang'])
    {
        case "en":
        case "us":
        default:
        //////////////////////////////////////////////////////
        // LOGIN PAGE
        //////////////////////////////////////////////////////
            define("TITLE_NANOLOGIN", $NXTText." Login");
            define("LOGIN_USERNAME", "Username");
            define("LOGIN_PASSWORD", "Password");
            define("LOGIN_LOGIN", "Login");
            define("LOGIN_FORGOTPASSWORD", "Forgot your password?");
            define("LOGIN_USER_GROUPTITLE", "User Login Level");

			//
			//License page
			//
			define("TITLE_LICENSE_KEY", "Nano License Key");
			define("LK_ENTER_LICENSE_KEY_HEADER", "ENTER LICENSE KEY");
			define("LK_ENTER_KEY_TEXT", "Please enter the License Key found on the<br>License Key Card included with the ".$NXTText." Package.");
			define("LK_DEFAULT_TEXT", "License Key...");
			define("LK_BTN_ACTIVATE_TEXT", "Activate");
			define("LK_INVALID_LICENSE", "Entered License was Invalid.");
			  define("SUBMIT_BUTTON", "Save");
			 define("CANCEL_BUTTON", "Cancel");
			define("DLGBTN_CONTINUE","Continue");
        //////////////////////////////////////////////////////
        // CONFIGURATION PAGE
        //////////////////////////////////////////////////////
            define("TITLE_NANOCONFIG", $NXTText." Configuration");
            define("TITLE_HELP", "Help Settings");
            define("TITLE_LOGOUT", "Logout");
			   define("HEADER_LICENSE", "License");
			
			
				 define ("NWMS_TEST_FAILED","NWMS test failed");
			define ("NWMS_TEST_SUCCESS","NWMS test success");
			define ("NWMS_SUCCESS_DETAIL","Network Matcher was found at the targeted address!");
				define ("NWMS_FAIL_DETAIL_NOTFOUND","Network Matcher was not found at the targeted address!");
				define ("NWMS_FAIL_DETAIL_INVALID","Test response was invalid. Check that nano is operational.");
			define("SESSION_TIMEOUT_WARNING", "Your session is going to expire");
			define("SESSION_TIMEOUT_CONTINUE", "Click Continue to stay logged in.");
			
            //////////////////////////////////////////////////////
            // HOME TAB STRINGS
            //////////////////////////////////////////////////////
            define ("HOME_TABLABEL", "Home");
            define ("HOME_HEADING", $NXTText." Information");
            define ("HOME_DEVICEINFO_GROUPTITLE", "Device Information");
            define ("HOME_DEVICEINFO_DEVICENAME", $NXTText." Name:");
            define ("HOME_DEVICEINFO_IPADDR", "IP Address:");
            define ("HOME_DEVICEINFO_IPV6ADDR", "IPv6 Address:");
			define ("HOME_DEVICEINFO_SERIAL", "Device ID:");
            define ("HOME_DEVICEINFO_MACADDR", "MAC Address:");
            define ("HOME_DEVICEINFO_LOCALTIME", $NXTText." Time Snapshot:");
            define ("HOME_DEVICEINFO_PREVIOUSUPDATEFAILED", "A previous attempt to update firmware on this ".$NXTText." failed.<br>The ".$NXTText." was reverted to the last known working state.<br>Update the firmware to clear this message.");

            define ("HOME_STORAGESTATS_GROUPTITLE", "Storage Statistics");
            define("HOME_STORAGESTATS_PT_GROUPTITLE", "Portable Template Decoder Information");
            define ("HOME_STORAGESTATS_NETWORKMATCHENABLED", "Network Matcher:");
            define ("HOME_STORAGESTATS_NETWORKMATCHADDRESS", "Network Matcher Addr:");
            define ("HOME_STORAGESTATS_TEMPLATESPACE", "Remaining Templates:");

            define ("HOME_PTINFO_BLEVERSION", "BLE Version:");
            define ("HOME_PTINFO_MAINFWVERSION", "Firmware Version:");
            define ("HOME_PTINFO_BOOTLOADERVERSION", "Boot Loader Version:");
            define ("HOME_PTINFO_HARDWAREVERSION", "Hardware Version:");
            define ("HOME_PTINFO_KEYPADVERSION", "Keypad Version:");
            define ("HOME_PTINFO_CONFIGURATION", "Configuration:");

            define ("HOME_SOFTWARE_GROUPTITLE", "Software Information");
            define ("HOME_SOFTWARE_APPVER", $NXTText." Firmware:");
            define ("HOME_SOFTWARE_BOBVER", "ICM Firmware Ver.: ");
            define ("HOME_HARDWARE_BOBVER", "ICM Hardware Ver.: ");
             define ("HOME_SOFTWARE_LINUXVER", "Linux OS Ver.: ");
             define ("HOME_HARDWARE_PSOCVER", "M.B. PSOC Ver.: ");
             define ("HOME_CAMERA_FPGAVER", "Camera FPGA Ver.: ");
             define ("HOME_CAMERA_PSOCVER", "Camera PSOC Ver.: ");
             define ("HOME_FIXED_BOARDVER", "Fixed Board Ver.: ");

            define ("HOME_SOFTWARE_NANOLABEL", $NXTText." Updated: ");
            define ("HOME_SOFTWARE_BOBLABEL", "ICM Updated: ");
			define ("HOME_SOFTWARE_FPGALABEL", "Camera FPGA Updated: ");
			define ("HOME_SOFTWARE_CAMERAPSOCLABEL", "Camera PSOC Updated: ");
			define ("HOME_SOFTWARE_FIXEDPSOCLABEL", "Fixed Board Updated: ");

       
            //////////////////////////////////////////////////////
            // NETWORK TAB STRINGS
            //////////////////////////////////////////////////////
            define ("NETWORK_TABLABEL", "Network");
            define ("NETWORK_HEADING", "Device Network Configuration");
            define ("NETWORK_DEVICENAME_GROUPTITLE", "Device Name");
            define ("NETWORK_DEVICENAME_LABEL", "Name:");
            define ("NETWORK_DHCP_GROUPTITLE", "Obtain an IP address automatically (DHCP)");
            define ("NETWORK_DHCP_SETTINGS", "DHCP Settings...");
			define("NETWORK_DNS_SERVER1", "DNS Server 1:");
			define("NETWORK_DNS_SERVER2", "DNS Server 2:");
                ///// ADV. DHCP SETTINGS DIALOG
                define ("NETWORK_DHCPSETTINGS_TITLE", "Advanced DHCP Settings");
                define ("NETWORK_DHCPSETTINGS_TIMEOUTS_GROUPLABEL", "Timeouts");
                define ("NETWORK_DHCPSETTINGS_TIMEOUTS_DHCPTIMEOUTLABEL", "DHCP Timeout:");
                define ("NETWORK_DHCPSETTINGS_TIMEOUTS_DHCPRETRIESLABEL", "DHCP Retries:");
                define ("NETWORK_DHCPSETTINGS_TIMEOUTS_DHCPRETRYDELAYLABEL", "Retry Delay:");
                define ("NETWORK_DHCPSETTINGS_OK", "OK");
            define ("NETWORK_STATICIP_GROUPLABEL", "Use the following Static IP Address");
            define ("NETWORK_STATICIP_DEVICEIPLABEL", "Device IP:");
            define ("NETWORK_STATICIP_BROADCASTNETWORKLABEL", "Broadcast Network:");
            define ("NETWORK_STATICIP_SUBNETMASKLABEL", "Subnet Mask:");
            define ("NETWORK_STATICIP_DEFAULTGATEWAYLABEL", "Default Gateway:");

            define ("NETWORK_IPV6_ENABLE", "Enable IPv6");
            define ("NETWORK_IPV6_CONFIGURATION", "IPv6 Configuration...");
                // IPV6 Pop-up
                define ("NETWORK_IPV6CONFIGURATION_TITLE", "IPv6 Configuration");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL", "General");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE", "DHCP Mode:");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_INFORMATION", "Stateless");

                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_NORMAL", "Normal");

                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_AUTO", "Auto");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_NONE", "Off");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_ADDRESS_PREFIX_LENGTH", "IPv6 Address/Subnet Prefix Length:");

                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DEFAULTGATEWAY", "Default Gateway:");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DNS_SERVER1", "DNS Server 1:");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DNS_SERVER2", "DNS Server 2:");
                define ("NETWORK_IPV6CONFIGURATION_STATUS", "Status");
                define ("NETWORK_IPV6CONFIGURATION_STATUS_LINK_LOCAL", "Link-local:");
                define ("NETWORK_IPV6CONFIGURATION_STATUS_GLOBAL", "Global:");
                define ("NETWORK_IPV6CONFIGURATION_STATUS_GLOBAL_STATELESS", "Global stateless:");
				define ("NETWORK_IPV6CONFIGURATION_STATUS_GLOBAL_STATELESS_TEMPORARY", "Global stateless temporary:");
                define ("NETWORK_IPV6CONFIGURATION_STATUS_NO_ADDRESSES", "Device currently has no IPv6 addresses assigned to it");

            define("SSL_PROTOCOL_LEGEND", "SSL Protocol");
            define("SSLPROTO_DEFAULT", "Legacy");
            define("SSLPROTO_TLS12", "TLS 1.2 (Only)");

            define ("NETWORK_CERTIFICATES_GROUPTITLE", "Certificates");
            define ("NETWORK_CACERT", "CA Certificate:");
            define ("NETWORK_CLIENTCERT", "Client Certificate:");
            define ("NETWORK_CLIENTPRIVATEKEY", "Client Private Key (and Certificate):");
            define ("NETWORK_CERTIFICATES_SETTINGS_GROUPTITLE", "Settings");
            define ("NETWORK_EAPOLVERSION", "EAPOL Version:");
            define ("NETWORK_EAPIDENTITY", "EAP Identity:");
            define ("NETWORK_PRIVTEKEYPWD", "Private Key Password:");

            define ("NETWORK_SETTINGS_ENABLEIEEE8021X", "Enable IEEE 802.1X");
            define ("NETWORK_SETTINGS_NOCERTIFICATE", "No Certificate File Uploaded");
            define ("NETWORK_SETTINGS_NOKEY", "No Private Key Uploaded");
            define ("NETWORK_802LOG_DOWNLOAD", "Download IEEE 802.1X Log...");
            
            define ("NETWORK_CHECKING_IP_ADDRESS_DUPLICATE", "Checking for IP address duplicates");
            define ("NETWORK_DUPLICATE_IP_ADDRESS", "IP address conflict");
            define ("NETWORK_IP_ADDRESS_IN_USE", "Specified device IP address conflict with another system on the network.");

            //////////////////////////////////////////////////////
            // DEVICE TAB STRINGS
            //////////////////////////////////////////////////////
            define ("DEVICE_TABLABEL", "Device");
            define ("DEVICE_HEADING", "Device Settings");
            define ("DEVICE_USERFEEDBACK_GROUPTITLE", "Feedback");
            define ("DEVICE_USERFEEDBACK_VOLUMELABEL", "Speaker Volume:");
            define ("DEVICE_USERFEEDBACK_FREQUENCYLABEL", "Tone Frequency (Hz):");
            define ("DEVICE_USERFEEDBACK_DURATIONLABEL", "Tone Duration:");
            define ("DEVICE_USERFEEDBACK_TAMPERVOLUMELABEL", "Tamper Tone Volume");
            define ("DEVICE_USERFEEDBACK_LEDBRIGHTNESSLABEL", "LED Brightness:");
            define ("DEVICE_USERFEEDBACK_LOCATEDEVICE", "Locate Device...");
            define ("DEVICE_TIMESETTINGS_GROUPTITLE", "Time Settings");
            define ("DEVICE_TIMESETTINGS_SERVERLABEL", "Time Server:");
            define ("DEVICE_TIMESETTINGS_NANOTIMELABEL", "nano Local Time:");
            define ("DEVICE_TIMESETTINGS_UPDATETIME", "Synchronize Now!");
            define ("DEVICE_TIMESETTINGS_SYNCHRONIZETIMELABEL", "Synchronize Daily");
    	    define ("DEVICE_TIMESETTINGS_UPDATELOCALTIME", "Synchronize With Host");
            define ("DEVICE_ACTIVITIES_GROUPTITLE", "Activities");
            define ("DEVICE_ACTIVITIES_FACTORYRESET", "Factory Reset");
		    define ("DEVICE_ACS_OSDPINSTALLMODE", "OSDP Installation Mode");
            define ("DEVICE_ACTIVITIES_REBOOTDEVICE", "Reboot Device");
            define ("DEVICE_EXTERNAL_GROUPTITLE", "HBOX Settings (EyeLock Support Only!)");
            define("DEVICE_EXTERNAL_WELCOMEMESSAGE", "Welcome Message:");
            define("DEVICE_EXTERNAL_LOCATION", "Location:");
            define("DEVICE_EXTERNAL_POSTTITLE", "Post Event URLs");
            define("DEVICE_EXTERNAL_DESTINATIONURL", "Main URL:");
            define("DEVICE_EXTERNAL_IRISURL", "Iris Post Endpoint:");
            define("DEVICE_EXTERNAL_ERRORURL", "Error Endpoint:");
            define("DEVICE_EXTERNAL_HEARTBEATURL", "HeartBeat Endpoint:");
            define("DEVICE_EXTERNAL_MAINTENANCEURL", "Maintenance Endpoint:");
            define("DEVICE_EXTERNAL_POSTSCHEMEURL", "POST Scheme:");
            define ("DEVICE_ADVSETTINGS_GROUPTITLE", "Advanced Device Settings");
            define ("DEVICE_ADVSETTINGS_LISTENINGPORTLABEL", "Network Listening Port:");
            define ("DEVICE_ADVSETTINGS_EYEDESTADDR", "Eye Destination Address:");
            define ("DEVICE_ADVSETTINGS_EYEDESTPORT", "Eye Destination Port:");
			  define ("DEVICE_ADVSETTINGS_EYECONNECTTIMEOUT", "Eye Connect Timeout:");
			  define ("DEVICE_ADVSETTINGS_EYESENDTIMEOUT", "Eye Send Timeout:");

			define ("DEVICE_TIMESERVERFAIL_PING", "Could not Ping time server.");
			define ("DEVICE_TIMESERVERFAIL_SYNC", "Could not synchronize with time server.");
				define("FACTORY_RESET_PROGRESS", "A Factory Reset is in progress.");
			define("FACTORY_RESET_LOGGEDOUT", "You were logged out for this process.");
			define("FACTORY_RESET_TWOMINS", "You may log in again after about 2 minutes.");
            //////////////////////////////////////////////////////
            // SECURITY TAB STRINGS
            //////////////////////////////////////////////////////
            define ("SECURITY_TABLABEL", "Security");
            define ("SECURITY_HEADING", "Security Settings");

            define ("SECURITY_PASSWORD_GROUPTITLE", "Password Reset");
            define ("SECURITY_PASSWORD_OLDPWDLABEL", "Old Password:");
            define ("SECURITY_PASSWORD_NEWPWDLABEL", "New Password:");
            define ("SECURITY_PASSWORD_CONFIRMPWDLABEL", "Confirm Password:");
            define ("SECURITY_PASSWORD_REMOVEPWDLABEL", "Clear Password");
            define ("SECURITY_PASSWORD_RESETPWD", "Update Password");

            define ("SECURITY_TAMPER_SETTINGS", "Tamper Settings");
            define ("SECURITY_TAMPER_SIGNALHIGH", "Activate Tamper State on Signal High");
            define ("SECURITY_TAMPER_SIGNALLOW", "Activate Tamper State on Signal Low");
            define ("SECURITY_TAMPER_NOTIFYADDRESS", "Notification Address:");
            define ("SECURITY_TAMPER_NOTIFYPORT", "Port:");
            define ("SECURITY_TAMPER_NOTIFYMESSAGE", "Tamper Message:");
			define ("SECURITY_TAMPER_MESSAGE_SETTINGS", "Tamper Message Settings");
	     
             define ("SECURITY_CARD_READER_INPUT_TAMPER_SETTINGS", "Card Reader Input Tamper Settings");
              define ("SECURITY_NXT_OUTPUT_TAMPER_SETTINGS", "Tamper Output Settings");
               define ("SECURITY_OUTPUT_TAMPER_SIGNALHIGH", "Signal High");
            define ("SECURITY_OUTPUT_TAMPER_SIGNALLOW", "Signal Low");
            //////////////////////////////////////////////////////
            // SOFTWARE TAB STRINGS
            //////////////////////////////////////////////////////
			define ("SOFTWARE_FIRMWARELINK", "Visit <a href=\"http://help.eyelock.com\">help.eyelock.com</a> for the latest firmware.");
            define ("SOFTWARE_TABLABEL", "Software");
            define ("SOFTWARE_HEADING", "Software/Firmware Details");
            define ("SOFTWARE_STATUS_GROUPTITLE", "Version/Update Status");
            define ("SOFTWARE_CHECKUPDATES_LABEL", "Most recent check for updates:");
            define ("SOFTWARE_UPDATEDETAILS_TITLE", "Software Update Details");
            define ("SOFTWARE_AVAILUPDATE_NANOLABEL", "New ".$NXTText." Version:");
            define ("SOFTWARE_AVAILUPDATE_BOBLABEL", "New ICM Version:");
            define ("SOFTWARE_INSTALLEDUPDATES_NANOLABEL", $NXTText." Update was installed:");
            define ("SOFTWARE_INSTALLEDUPDATES_BOBLABEL", "ICM Update was installed:");
            define ("SOFTWARE_UPDATE_ALLOWSITEADMIN", "Allow admin to Update Device");

			define ("VERSION_HEADER", "Version");
			define("RESTORE_POINTS_TIME_STAMP", "Restore Point Timestamp");
            define ("SOFTWARE_STATUS_UPDATESTATUS_FAILED", "Failed to Contact Update Server!");
            define ("SOFTWARE_STATUS_UPDATESTATUS_NEWVERSION", "A New ".$NXTText." Software Update is Available!");
            define ("SOFTWARE_STATUS_UPDATESTATUS_CURRENT", "The ".$NXTText." Software is Up to Date!");
            define ("SOFTWARE_STATUS_UPDATESTATUS_VERCORRUPT", "Version Files Appear to be Corrupted!");
            define ("SOFTWARE_STATUS_UPDATESTATUS_CHECKINTERNET", "Check Browser Internet Connection Status!");
            define ("SOFTWARE_STATUS_UPDATENOW", "Update Now!");
            define ("SOFTWARE_STATUS_LATER", "Update Later");
            define ("SOFTWARE_STATUS_MANUALNANO", "Local File...");
            define ("SOFTWARE_STATUS_MANUALBOB", "Local ICM File...");
            define ("SOFTWARE_STATUS_UPDATEDETAIL", "Details...");
            define ("SOFTWARE_MODE_NANOLABEL", $NXTText." Restore Points");
			define ("SOFTWARE_MODE_BOBLABEL", "ICM Board Restore Points");
            define ("SOFTWARE_MODE_DELETERESTOREPOINTS", "Deleting Restore Point(s)...");
            define ("SOFTWARE_MODE_RESTORERESTOREPOINT", "Restoring To Previous Version...");
            define ("SOFTWARE_RESTOREPOINTS_NONANO", "No ".$NXTText." Restore Points Available!");
            define ("SOFTWARE_RESTOREPOINTS_NOBOB", "No ICM Restore Points Available!");
            define ("SOFTWARE_RESTOREHEADER_SELECT", "Select");
            define ("SOFTWARE_RESTOREHEADER_RESTOREPOINTS", "Restore Points");
            define ("SOFTWARE_RESTORE_GROUPTITLE", "Restore Firmware");
            define ("SOFTWARE_RESTORE_RESTORENOW", "Restore Now...");
            define ("SOFTWARE_RESTORE_DELETERESTOREPOINTS", "Delete...");
			

            //////////////////////////////////////////////////////
            // AUTHENTICATION TAB STRINGS
            //////////////////////////////////////////////////////
            define ("AUTHENTICATION_TABLABEL", "Iris Processing");
            define ("AUTHENTICATION_HEADING", "Configure Iris Processing Details");
            define ("AUTHENTICATION_SETTINGS_IRISPROCESSINGMODE", "Iris Processing Mode:");
            define ("AUTHENTICATION_SETTINGS_ACCESSCONTROLMODE", "Access Control Authentication Mode");
            define ("AUTHENTICATION_SETTINGS_ACCESSCONTROLAUTHENTICATION", "Access Control Authentication");
            define ("AUTHENTICATION_SETTINGS_IRISCAPTUREMODE", "Iris Capture Mode");
            define ("AUTHENTICATION_SETTINGS_IRISCAPTURE", "Iris Capture");
            define ("DEVICE_EXTERNAL_POSTSCHEME", "Iris Post Scheme:");
            define ("AUTHENTICATION_SETTINGS_IRISCAPTURESETTINGS", "Iris Capture Settings");
            define ("AUTHENTICATION_SETTINGS_IRISCAPTURETIMEOUT", "Iris Capture Timeout:");
            define ("AUTHENTICATION_SETTINGS_IRISCAPTURERESETDELAY", "Iris Capture Reset Delay:");
            define ("AUTHENTICATION_SETTINGS_HTTPPOSTMSGFORMAT", "Http Post Message Format:");
            define ("AUTHENTICATION_SETTINGS_IRISIMAGEFORMAT", "Iris Image Format:");
            define ("AUTHENTICATION_SETTINGS_IRISIMAGEQUALITY", "Image Quality:");
            define ("AUTHENTICATION_SETTINGS_100LOSSLESS", "(100 = Lossless)");
            define ("AUTHENTICATION_MODE_GROUPTITLE", "Matching");
            define ("AUTHENTICATION_MODE_SINGLEEYELABEL", "Use Single Eye");
            define ("AUTHENTICATION_MODE_DUALEYELABEL", "Use Both Eyes");
            define ("AUTHENTICATION_SETTINGS_GROUPTITLE", "Settings");
            define ("AUTHENTICATION_SETTINGS_REPEATPERIODLABEL", "Repeat Authorization Period:");
            define ("AUTHENTICATION_SETTINGS_NEGMATCHTIMEOUTENABLEDLABEL", "Enable Negative Match Timeouts");
            define ("AUTHENTICATION_SETTINGS_LOITERPERIODLABEL", "Loiter Period:");
            define ("AUTHENTICATION_SETTINGS_NEGMATCHRESETLABEL", "Neg. Match Reset Timer:");
            define ("AUTHENTICATION_SETTINGS_DESTINATIONADDRESSLABEL", "Network Message Destination IP:");
            define ("AUTHENTICATION_SETTINGS_DESTINATIONPORTLABEL", "Port:");
            define ("AUTHENTICATION_SETTINGS_SECURENETWORDLABEL", "Secure Network Match Message");
            define ("AUTHENTICATION_SETTINGS_MSGFORMATLABEL", "Network Message Format:");
            define ("AUTHENTICATION_SETTINGS_SENDALLIMAGES", "Send All Images");

            //////////////////////////////////////////////////////f
            // DATABASE TAB STRINGS
            //////////////////////////////////////////////////////
            define ("DATABASE_TABLABEL", "Database");
            define ("DATABASE_HEADING", "Database Configuration Details");
            define ("DATABASE_TYPE", "Database Type: ");
            define ("DATABASE_SQLLITE", "SQLite");
            define ("DATABASE_BINARY", "Binary (Flat File)");
            define ("DATABASE_TYPE_GROUPTITLE", "Database Details");
            define ("DATABASE_TYPE_LOCALLABEL", "Local");
            define ("DATABASE_TYPE_NETMATCHLABEL", "Enable Network Matcher (NWMS)");
            define ("DATABASE_TYPE_NETMATCHADDRESSLABEL", "Network Matcher Address:");
			define ("TESTING_NETWORK_MATCHER", "Testing Network Matcher...");
            define ("DATABASE_TYPE_NETMATCHPORTLABEL", "Network Matcher Destination Port:");
            define ("DATABASE_STATISTICS_GROUPTITLE", "Database Statistics");
               define ("DATABASE_TOC_GROUPTITLE", "Portable Templates");
            define ("DATABASE_STATISTICS_TEMPLATESPACE", "Remaining Template Space:");
			define ("DATABASE_SECURECOMM_NETMATCHLABEL", "Secure Network Matcher");
            define("DATABASE_MOBILEMODE", "Mobile Mode:" );
            define ("TOOLTIP_TOC_MODE", "Portable template on Smartphone Application will use these modes: Walk Up, Tap to Send, Pin to Send</br><strong>Does not affect EV1 card system.</strong>");
            define ("TOOLTIP_TOC_IRIS_WAIT_TIME", "Time in seconds that the ".$NXTText." retains a portable template after it was presented.  Minimum 10 seconds, Maximum 600 seconds.");
              define("TOOLTIP_TOC_DEFAULT_KEY", "Use the default ".$NXTText." key for portable templates.");
             define("TOOLTIP_TOC_CUSTOM_KEY", "Use the uploaded key for portable templates.");
            define("TOOLTIP_UPLOAD_CERTIFICATE", "Upload a .pfx key for use with portable templates.  Provide the password for the key in the box to the left.");
            define("TOOLTIP_CURRENT_CERTIFICATE", "If this shows \"Upload key...\" please upload a key.");
            define("DATABASE_EXPIRATION", "Iris Expiration Time");
			
			define("DATABASE_CURRENT_KEY", "Current Key:");
			
			define("DATABASE_TOC_MODE_WALKUP","Walk-Up");
				define("DATABASE_TOC_MODE_TAPTOSEND","Tap-To-Send");
				define("DATABASE_TOC_MODE_PINTOSEND","Pin-To-Send");
            //////////////////////////////////////////////////////
            // ACS TAB STRINGS
            //////////////////////////////////////////////////////
            define ("ACP_TABLABEL", "ACS");
            define ("ACP_HEADING", "Access Control System (ACS)");
            define ("ACP_PROTOCOL_GROUPTITLE", "Access Control Protocol");
            define ("ACP_PROTOCOL_PROTOCOL", "Protocol:");
            define ("ACP_PROTOCOL_DUALAUTHENABLEDLABEL", "Dual Factor Authentication");
             define ("ACP_PROTOCOL_TEMPLATEONCARD", "Portable Templates");
             define ("ACP_PROTOCOL_TEMPLATEONCARDPASS", "Single Factor Authentication");
             
               define ("ACP_PROTOCOL_DUALAUTHPARITY", "Check Parity Bits");
            define ("ACP_PROTOCOL_DUALAUTHLEDENABLEDLABEL", "LED Controlled by ACS");
            define ("ACP_PROTOCOL_MATCHWAITIRISTIMELABEL", "Iris Wait Time:");
            define ("ACP_PROTOCOL_MATCHWAITPINTIMELABEL", "PIN Wait Time:");
            define ("ACP_PROTOCOL_PINBURSTBITS", "PIN Burst Bits:");
            define ("ACP_PROTOCOL_RELAYTIMELABEL", "Grant Relay Time:");
            define ("ACP_PROTOCOL_DENYRELAYTIMELABEL", "Deny Relay Time:");
            define ("ACP_PROTOCOL_DURESSRELAYTIMELABEL", "Duress Relay Time:");
            define ("ACP_PROTOCOL_ENABLERELAYTRIGGERSLABEL","Enable Relays");
            define ("ACP_PROTOCOL_NEGMATCHTIMEOUTLABEL", "Neg. Match Timeout:");

            define ("ACP_TEST_GROUPTITLE", "Denied Access and Test Data");
            define ("ACP_TEST_TESTBUTTON", "Test Now!");
            define ("ACP_TEST_CARDIDLABEL", "Card Number:");
            define ("ACP_TEST_FACILITYCODELABEL", "Facility Code:");
            define ("ACP_TEST_SENDINGMESSAGE", "Sending Test Message to ACS Panel...");
            define ("ACP_NETWORK_SECURECOMMLABEL", "Secured Communication");
            define ("ACP_TEST_TCPCONNECTIONFAILED", "Socket Connection Failure");
            define ("ACP_TEST_FAILED", "Failed to send ACS Test string to panel.");
            define ("ACP_TEST_CONNECTIONFAILED", "Unable to establish socket connection to firmware.");
	        define ("OSDP_INSTALL_MODE_SETTINGS", "Setting device to installation mode.");
            define ("AUTHENTICATION_SCHEME", "Authentication Scheme:");
            define ("ACP_DD_SINGLEFACTORIO", "Single Factor [Iris Only]");
            define ("ACP_DD_SINGLEFACTORIC", "Single Factor [Iris OR Card]");
            define ("ACP_DD_DUALFACTORIC", "Dual Factor [Iris AND Card]");
            define ("ACP_DD_DUALFACTORICPP", "Dual Factor [Iris AND Card (PIN Pass-Through)]");
            define ("ACP_DD_DUALFACTORPI", "Dual Factor [Iris AND PIN]");
            define ("ACP_DD_DUALFACTORIPID", "Dual Factor [Iris AND PIN (Duress)]");
            define ("ACP_DD_DUALFACTORICPI", "3 Factor [Iris, Card AND PIN]");
            define ("ACP_DD_DUALFACTORICPID", "3 Factor [Iris, Card AND PIN (Duress)]");
			
			define("PARITY_MASK_GROUP_TITLE", "Parity Masking");
			define("TOOLTIP_ACS_PARITY_MASK_DISABLED", "When checked, ".$NXTText." will check the parity bits from the reader during Dual Factor Authentication");
			define("TOOLTIP_ACS_PARITY_MASK_ENABLED", "When checked, ".$NXTText." will ignore parity bits from the reader during Dual Factor Authentication.");
			
			define("ACS_PARITY_MASK_DISABLED", "No Masking");
			define("ACS_PARITY_MASK_ENABLED", "Masking");
            //////////////////////////////////////////////////////
            // LOGS TAB STRINGS
            //////////////////////////////////////////////////////
			define ("LOG_NO_INFO", "No log information available!");
			define("MATCH_FAIL_NO_IRIS","Match Failure: No Iris Present");
			define("MATCH_FAIL_MISMATCH","Match Failure: Iris Mismatch");
			define("MATCH_FAIL_INVALID_CARD","Match Failure: Invalid Card");
            define("MATCH_SUCCESS","Match Success");
            define("MATCH_SUCCESSDURESS","Match Success (Duress)");
            define("MATCH_FAIL_INVALIDPIN","Match Failure: Invalid PIN");
            define("MATCH_FAIL_NOPIN","Match Failure: No PIN");

            define ("LOGS_TABLABEL", "Logs");
            define ("LOGS_HEADING", "Logs");
            define ("LOGHEADER_STATUS", "Status");
            define ("LOGHEADER_DATE", "Date/Time");
            define ("LOGHEADER_NAME", "Name");
            define ("LOGHEADER_CODE", "ACS Code");
            define ("LOGHEADER_MESSAGE", "Message");
            define ("LOGS_EVENTLOG_GROUPTITLE", "Event Log");
            define ("LOGS_EVENTLOG_REFRESHBUTTON", "Refresh!");
            define ("LOGS_EVENTLOG_DOWNLOAD", "Download Log...");
            define ("LOG_AUTOREFRESH_LABEL", "Auto Refresh:");
			define ("LOGS_TIMES_SHOWN_IN", "All times shown in ");
			define ("LOG_TYPE", "Log Level");
            //////////////////////////////////////////////////////
            // DUMP TAB STRINGS
            //////////////////////////////////////////////////////
            define ("DUMP_TABLABEL", "Dump");


            //////////////////////////////////////////////////////
            // HELP SETTINGS DIALOG STRINGS
            //////////////////////////////////////////////////////
            define ("DIALOG_HELPSETTINGS_TITLE", "Help Settings");
            define ("DIALOG_HELPSETTINGS_ENABLEHELP", "Enable Popup Help");
            define ("DIALOG_HELPSETTINGS_POPUPTRIGGERMODE", "Popup Trigger Mode:");
            define ("DIALOG_HELPSETTINGS_POPUPHOVER", "Hover Mouse");
            define ("DIALOG_HELPSETTINGS_POPUPCLICK", "Click Mouse");
            define ("DIALOG_HELPSETTINGS_POPUPDELAY", "Cursor Hover Delay:");


            //////////////////////////////////////////////////////
            // HELP DIALOG HELP STRINGS
            //////////////////////////////////////////////////////
            define ("TOOLTIP_HELPSETTINGS_ENABLEHELP", "<span style='font-size: 16px; font-style:bold'>Enable Popup Help</span>
                                                <ul style='padding-left: 1em'><li>Select this option to enable the <i>Popup Help System</i>.</li>
                                                <li>Deselect to completely disable the <i>Popup Help System</i>.</li></ul>");
            define ("TOOLTIP_HELPSETTINGS_POPUPMODE", "<span style='font-size: 16px; font-style:bold'>Popup Help Mode</span>
                                                <ul style='padding-left: 1em'><li>Select the method which is used to initiate <i>Popup Help System</i> popups.</li></ul>");
            define ("TOOLTIP_HELPSETTINGS_POPUPDELAY", "<span style='font-size: 16px; font-style:bold'>Cursor Hover Delay</span>
                                                <ul style='padding-left: 1em'><li>Specify the time in seconds that the mouse pointer must hover over a field before the <i>Popup Help System</i> window appears.</li>
                                                <li>Duration Range (seconds): 0 - 5.</li>
                                                <li>Default value: 1.0.</li></ul>");


            //////////////////////////////////////////////////////
            // COMMON STRINGS
            //////////////////////////////////////////////////////
            define ("TAMPER_ICON_ALTTEXT", "Tamper Alert!");

            /////////////////////////////////////////////////////
            // TOOLTIP STRINGS
            /////////////////////////////////////////////////////
            define ("TAMPER_TOOLTIP_TEXT", "Device Tamper Detected.");
            define ("ATOMIC_TOOLTIP_TEXT", "Eyelock application is running!");
            define ("POWERBUTTON_TOOLTIP_TEXT", "Eyelock application is not running!");
            define ("EYELOCK_APPLICATION_STATUS", "Eyelock Application Status");
            define ("EYELOCK_MASTER_STATUSTEXT", "Device Camera 1 Status:");
            define ("EYELOCK_SLAVE_STATUSTEXT", "Device Camera 2 Status:");
            define ("EYELOCK_STATUS_RUNNING", "Ok");
            define ("EYELOCK_STATUS_NOTRUNNING", "Failed");

            /////////////////////////////////////////////////////
            // ASSORTED STRINGS
            /////////////////////////////////////////////////////
            define ("SECONDS_LABEL", " Seconds");
            define ("MILLISECONDS_LABEL", " Milliseconds");
			define("MSG_UPDATING", "Updating");
            define ("DEFAULT_EMPTY_FIELD", "Optional");
            define ("REQUIRED_EMPTY_FIELD", "Required");
            define ("CHANGE_PASSWORD_OLD", "Old Password");
            define ("CHANGE_PASSWORD_NEW", "New Password");
            define ("CHANGE_PASSWORD_CONFIRM", "Confirm Password");
            define ("MSG_UNAVAILABLE", "Unavailable!");
            define ("MSG_USERHELLO", "Hi");
            define ("MSG_UNKNOWNUSER", "Unknown User");
            define ("MSG_ENABLED", "Enabled");
            define ("MSG_DISABLED", "Disabled");
            define ("MSG_NEVER", "Never!");
            define ("MSGBOX_INFORMATION_TITLE", "Information");
            define ("MSGBOX_INFORMATION_TITLESHORT", "Info");
            define ("MSGBOX_SUCCESS_TITLE", "Success");
            define ("MSGBOX_FAILED_TITLE", "Failed");
            define ("MSGBOX_TAMPERED_TITLE", "Tamper!");
            define ("MSGBOX_OKBUTTON", "OK");
            define ("MSGBOX_CANCELBUTTON", "Cancel");
            define ("MSGBOX_YESBUTTON", "Yes");
            define ("MSGBOX_NOBUTTON", "No");
            define ("SAVING_SETTINGS", "Saving Settings...");
            define ("SAVING_SETTINGSANDRESTART", "Saving Settings and Restarting...");
            define ("SAVING_FEWMOMENTS", "This may take a few moments...");
            define ("RELOADING_PAGE", "Reloading Page... Please Wait...");
            define ("REFRESHING_PAGE", "Refreshing Page... Please Wait...");
            define ("VALIDATION_FAILEDTITLE", "Validation Failed!");
            define ("VALIDATION_MESSAGE1", "Some fields do not contain valid information!");
            define ("VALIDATION_MESSAGE2", "Please check all tabs for invalid fields!");
            define ("CONNECTION_FAILEDTITLE", "Connection Problem!");
            define ("CONNECTION_MESSAGE1", "WebConfig was unable to connect to the device!");
            define ("CONNECTION_MESSAGE2", "Please check the device IP address and power!");
            define ("LOADINGLOG_DETAIL", "Loading Log Detail...  Please Wait...");
            define ("ALERT_IPINUSE", "The specified Static IP is already in use! Not all settings could be saved!");
            define ("ALERT_802CONFIG", "Unable to verify IEEE 802.1X configuation information.  Not all settings could be saved!  Check to ensure that you have uploaded all of the required Certificates/Keys!");
            define ("RESETPASSWORD_MESSAGETITLE", "Password Reset");
            define ("RESETTINGPASSWORD_MESSAGE", "Resetting Password. Please Wait...");
            define ("RESETPASSWORD_SUCCESS", "Successfully reset password!");
            define ("RESETPASSWORD_LOGOUT", "Logout to use your new password.");
            define ("RESETPASSWORD_FAIL", "Failed to reset password!");
	     define ("KEY_MANAGEMENT_GROUPTITLE", "Key Management Settings");
         define ("KEY_MANAGEMENT_DEFAULT", "Use Default Key");
          define ("KEY_MANAGEMENT_CUSTOM", "Use Custom Key");
			define("DATABASE_CURRENT_KEY_DEFAULT", "Default Key");
				define("DATABASE_CURRENT_KEY_CUSTOM", "Custom Key");
			define("DATABASE_PTUPLOAD", "Upload...");
			
	     define ("KEY_MANAGEMENT_BUTTON", "Download Key...");
	     define ("ADDKEY_DIALOG_TITLE", "Add New Encryption Key");
	     define ("ADDKEY_DIALOG_MESSAGE", "Enter details of encryption key below :");
	     define ("ADDKEY_DIALOG_CONTROLS", "<table style=\"width:100%\"><tr style=\"height:2px\" /> <tr><td>Host Name :</td> <td><input id=\"keyHostName\" type=\"text\" name=\"keyHostName\" style=\"width:120px\" onblur=\"checkHostName()\"></td> <td /></tr> <tr style=\"height:2px\" /> <tr><td>Validity Period : </td><td><input id=\"keyValidPeriod\" type=\"text\" name=\"keyValidity\" style=\"width:120px\" onblur=\"checkValidity()\"></td><td>in days(5 to 3650)</td></tr> <tr style=\"height:2px\" /></table>");						
	     define ("ADDING_NEW_KEY", "Adding New Key...");
	     define ("DELETEALLKEY_DIALOG_TITLE", "Delete All Host Keys");
	     define ("DELETEALLKEY_DIALOG_MESSAGE", "Trying to delete all host keys on the device.");
	     define ("DELETING_ALL_KEYS", "Deleting All Keys...");
 	     define ("DELETEKEY_DIALOG_TITLE", "Delete Host Key");
	     define ("DELETEKEY_DIALOG_MESSAGE", "Trying to delete host key on the device.");
    	     define ("DELETING_HOST_KEY", "Deleting Host Key...");
            define ("DOWNLOADING_KEY", "Downloading Key...");
	     define ("REGENERATEKEY_DIALOG_TITLE", "Regenerate ".$NXTText."  Key");
	     define ("REGENERATEKEY_DIALOG_MESSAGE", "Trying to regenerate ".$NXTText."  key. This will cause all previously downloaded Keys to be invalid.");
	     define ("REGENERATING_NANO_KEY", "Regenerating ".$NXTText."  Key...");
	     define ("STARTING_EYELOCK_APPLICATION", "Starting Eyelock Application...");
            define ("IDENTIFY_DEVICE_TITLE", "Indentifying Device");
            define ("IDENTIFY_DEVICE_MESSAGE", "Repeatedly Flashing Device LEDs...");
            define ("IDENTIFY_DEVICE_MESSAGE2", "Click Cancel To Abort.");
            define ("RESETTING_DEVICE_MESSAGE", "Resetting Device... Please Wait...");
            define ("REBOOTING_DEVICE_MESSAGE", "Rebooting Device... Please Wait...");
            define ("REBOOTING_DEVICE_MESSAGE2", "This may take a minute or two...");
            define ("WAITING_FOR_EYELOCK_RESTART", "Restarting Eyelock Application...  Please Wait...");
            define ("DEVICE_TIME_SYNCHRONIZING", "Synchronizing Device Time... Please Wait...");
            define ("DEVICE_TIME_SYNCHRONIZED", "The device time has been successfully set!");
            define ("DEVICE_TIME_SYNCFAILED", "Failed to set device time!");
            define ("FACTORYRESET_DEVICE", "Resetting Device to Factory Defaults...");
            define ("RESTORE_DEVICE", "Restoring Device from backup...");
            define ("RESTORE_DEVICE_TITLE", "Restore Device Firmware");
            define ("RESTORE_DEVICE_DELETETITLE", "Delete Restore Point");
            define ("RESTORE_DEVICE_DELETEMSG", "Deleting Restore Point(s) from Device...");
			define("RESTORE_FAILED_NOT_SUPPORTED", "Restore point failed to restore, version is no longer supported");
			define("RESTORE_FAILED_NO_FILE", "Restore point failed, could not find Nano restore point"); //should be impossible to get this one, but have to cover it anyway
            define ("AUTOMATIC_LOGOUT", "You will now be logged out!");
            define ("LOGOUT_MESSAGE", "Logging Out... Please Wait...");
            define ("REBOOT__DEVICE_TITLE", "Reboot Device");
            define ("REBOOT_DEVICE_WARNING", "Clicking Yes will reboot the device...");
            define ("MSG_AREYOUSURE", "Do you want to continue?");
            define ("FACTORY_RESET_TITLE", "Confirm Factory Reset");
			
			 define ("OSDPINSTALLMODE_TITLE", "Confirm setting OSDP Install Mode");
			
            define ("FACTORY_RESET_WARNING", "This action cannot be undone...");
            define ("FIRMWARE_UPDATE_NANOTITLE", "Processing ".$NXTText." Firmware Update...  Please Wait...");
			
			define ("PT_FIRMWARE_UPDATE_NANO_TITLE","Processing Portable Template Reader Firwmare Update... Please Wait...");
			  define ("PT_FIRMWARE_UPDATE_STATUS_UPLOAD","Uploading Portable Template Reader Firmware");
			  define ("PT_FIRMWARE_UPDATE_STATUS_UPDATING_BOB","Updating Portable Template Reader Firmware");

			  define ("PT_FIRMWARE_UPDATE_TITLE","Portable Template Reader Firmware Update");
			  define ("PT_FIRWMARE_UPDATE_SUCCESS","Successfully Updated Portable Template Reader Firmware.");
			  define ("PT_FIRWMARE_UPDATE_RELOAD","Click OK to refresh the page.");
			  define ("PT_FIRWMARE_UPDATE_ERROR_FAILED","Failed to update Portable Template Reader Firmware.");
			  define ("PT_FIRWMARE_UPDATE_FAILED","Failed to update Portable Template Reader Firmware.");
			
			
			define("PT_FIRMWARE_MANAGEMENT_TITLE", "Portable Template Reader Firmware Management");
			define("PT_FIRWMARE_UPDATE_WAITING", "Waiting for Portable Template Reader to restart...");
			
			
			
			define ("UPGRADE_NOT_ALLOWED", "Upgrade Failed, Upgrade version not supported");
            define ("FIRMWARE_UPDATESTATUS_UPLOAD", "Uploading Package To Device...");
            define ("FIRMWARE_UPDATESTATUS_DOWNLOAD", "Downloading Package From Server: ");
            define ("FIRMWARE_UPDATESTATUS_UNPACK", "Unpacking Files...");
            define ("FIRMWARE_UPDATESTATUS_VALIDATING", "Validating ".$NXTText." Image...");
            define ("FIRMWARE_UPDATESTATUS_COPYING", "Copying Required Files...");
            define ("FIRMWARE_UPDATESTATUS_RESTOREPOINT", "Creating ".$NXTText." Restore Point...");
            define ("FIRMWARE_UPDATESTATUS_UPDATING", "Performing ".$NXTText." Update...");
            define ("FIRMWARE_UPDATESTATUS_VALIDATINGBOB", "Validating ICM Image...");
            define ("FIRMWARE_UPDATESTATUS_RESTOREPOINTBOB", "Creating ICM Restore Point...");
            define ("FIRMWARE_UPDATESTATUS_UPDATINGBOB", "Performing ICM Update...");
            define ("FIRMWARE_UPDATESTATUS_COMPLETE", "Firmware Update Complete!");
            define ("FIRMWARE_UPDATESTATUS_RESTORESETTINGS", "Restoring Device Settings...");
            define ("FIRMWARE_UPDATE_TITLE", "Firmware Update Results");
            define ("FIRMWARE_UPDATE_FAILEDTITLE", "Firmware Update Failed");
	     define ("FIRMWARE_UPDATE_FAILEDMESSAGE", $NXTText."  firmware update failed!");
            define ("FIRMWARE_UPDATE_SUCCESS", "Firmware has been successfully updated!");
            define ("FIRMWARE_UPDATE_RELOAD", "Click OK to reboot the device...");
            define ("FIRMWARE_UPDATEERROR_BADFILETYPE", "Uploaded file is not a valid firmware package!");
            define ("FIRMWARE_UPDATEERROR_UNPACKFAILED", "Failed to unpack firmware package!  Package may be corrupted or device may be out of space.");
            define ("FIRMWARE_UPDATEERROR_VALIDATEFAILED", "Failed to validate package contents!  Contents may be corrupted.");
            define ("FIRMWARE_UPDATEERROR_RESTOREPOINTFAILED", "Failed to create restore point!  Device may be out of space.");
            define ("FIRMWARE_UPDATEERROR_INSTALLFAILED", "Failed to completely extract firmware on device!  Device may be out of space.");
            define ("FIRMWARE_UPDATEERROR_BOBINSTALLFAILED", "Failed to successfully install break out board firmware update!");
            define ("FIRMWARE_UPDATEERROR_DEVICERESTOREFAILED", "Failed to restore device settings!");
            define ("FIRMWARE_UPDATEERROR_SLAVECOPYFAILED", "Failed to copy files to slave device!  Device may be out of space.");
            define ("FIRMWARE_UPDATEERROR_SLAVEINSTALLFAILED", "Failed to upgrade slave device!");
            define ("FIRMWARE_UPDATEERROR_UNKNOWNFAILED", "Unknown failure while updating device!");
            define ("DATABASE_DETAILSUNAVAILABLE", "Details Unavailable!");
            define ("NANO_DEVICE_STATUSTITLE", $NXTText." Device Status");
            define ("NANO_DEVICE_CONNDOWN", "Unable to communicate. Please verify that the device is powered.");
            define ("NANO_DEVICE_RECONNECT", "Please check on device, then click Ok to check on status.");

            //Tooltip text
            define ("TOOLTIP_LOGIN_installer", "<b>Installer</b><br>The <b><i>Installer</i></b> user type allows full edit privileges.");
            define ("TOOLTIP_LOGIN_SITEADMIN", "<b>Admin</b><br>The <b><i>Admin</i></b> user type allows full read and limited write privileges.");
            define ("TOOLTIP_LOGIN_CHANGEPASSWORD", "Select the user type for which you wish to change the password.");

            define ("TOOLTIP_HOME_TAB", "<b><i>Home Tab</i></b> <br>Displays basic details of the ".$NXTText." . (Device Information, Software Information and Database Storage Statistics)");

            define ("TOOLTIP_NETWORK_TAB", "<b><i>Network Tab</i></b><br>Configuration of network parameters. (DHCP Settings, IP Address, Host Name etc.)");
            define ("TOOLTIP_NETWORK_NAME", "<span style='font-size: 16px; font-style:bold'>Device Name</span>
                                             <ul style='padding-left: 1em'><li>The <b><i>Name</i></b> field represents the hostname of the ".$NXTText." when running DHCP or Static IP on the network.</li>
                                             <li>A valid <i>Device Name</i> must contain only letters and numbers and be less than 64 characters in length.</li></ul>");
            define ("TOOLTIP_NETWORK_DHCP", "<span style='font-size: 16px; font-style:bold'>Obtain an IP address automatically (DHCP)</span>
                                             <ul style='padding-left: 1em'><li>This selection causes the ".$NXTText." to make itself available on the network using the <i>Device Name</i> through DHCP.</li>
                                             <li>When using DHCP, the ".$NXTText." can be reached on the network by entering the <i>Device Name</i> instead of an IP Address.</li>
                                             <li>Some networks are configured to use DHCP. If the network is not configured to use DHCP, Web Config can be accessed using a static IP.</li></ul>");
             define ("TOOLTIP_NETWORK_SSLPROTOCOL", "<span style='font-size: 16px; font-style:bold'>SSL Protocol</span>
                                             <ul style='padding-left: 1em'><li>Select the SSL Protocol that the ".$NXTText." uses for communications.</li>
                                             <li>Legacy:  Uses SSL 3.0 or TLS 1.2</li>
                                             <li>TLS 1.2 (Only):  Forces TLS 1.2</li></ul>");
            define ("TOOLTIP_NETWORK_ADVDHCPBUTTON", "<span style='font-size: 16px; font-style:bold'>DHCP Settings Button</span>
                                             <ul style='padding-left: 1em'><li>Click to access advanced DHCP settings.</li></ul>");
            define ("TOOLTIP_NETWORK_STATICIP", "<span style='font-size: 16px; font-style:bold'>Use the following static IP Address</span>
                                                <ul style='padding-left: 1em'><li>This option makes the ".$NXTText." available on the network using a specified static IP Address.</li>
                                                <li>The configured IP Address must not be in use elsewhere on the network.</li>
                                                <li>If the IP address is already in use, the ".$NXTText." will not be available on the network and the <i>Device Name</i> will not appear.</li></ul>");
            define ("TOOLTIP_NETWORK_DEVICEIP", "<span style='font-size: 16px; font-style:bold'>Device IP</span>
                                                <ul style='padding-left: 1em'><li>Enter the static IP address here.</li>
                                                <li>Consult with the local Network Administrator, if necessary, to determine existence of available IP Addresses.</li>
                                                <li>This setting only applies if <i>Use the following static IP Address</i> is selected above.</li></ul>");
            define ("TOOLTIP_NETWORK_BROADCASTNETWORK", "<span style='font-size: 16px; font-style:bold'>Broadcast Network</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Consult with the local Network Administrator, if necessary, to determine the proper value.</li>
                                                <li>This setting only applies if <i>Use the following static IP Address</i> is selected above.</li></ul>");
            define ("TOOLTIP_NETWORK_SUBNETMASK", "<span style='font-size: 16px; font-style:bold'>Subnet Mask</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Consult with the local Network Administrator if necessary to determine the proper value.</li>
                                                <li>This setting only applies if <i>Use the following static IP Address</i> is selected above.</li></ul>");
            define ("TOOLTIP_NETWORK_DEFAULTGATEWAY", "<span style='font-size: 16px; font-style:bold'>Default Gateway</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Consult with the local Network Administrator if necessary to determine the proper value.</li>
                                                <li>This setting only applies if <i>Use the following static IP Address</i> is selected above.</li></ul>");
 	        define ("TOOLTIP_NETWORK_DNS", "<span style='font-size: 16px; font-style:bold'>DNS(Domain Name System) Server</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Consult with the local Network Administrator if necessary to determine the proper value.</li>
                                                <li>This setting only applies if <i>Use the following static IP Address</i> is selected above.</li></ul>");
            define ("TOOLTIP_NETWORK_ENABLEIEEE8021X", "<span style='font-size: 16px; font-style:bold'>Enable IEEE 802.1X Security</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Consult with the local Network Administrator if necessary to determine the proper value.</li>
                                                <li>This setting enables the IEEE 802.1X network protocol on the ".$NXTText."</li></ul>");
            define ("TOOLTIP_NETWORK_CACERT", "<span style='font-size: 16px; font-style:bold'>CA Certificate</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use the Upload button to the right to browse and upload the CA Certificate file to the device.</li></ul>");
            define ("TOOLTIP_NETWORK_UPLOADCACERTIFICATE", "<span style='font-size: 16px; font-style:bold'>Upload CA Certificate</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use this button to browse and upload the CA Certificate file to the device.</li></ul>");
            define ("TOOLTIP_NETWORK_CLIENTCERT",  "<span style='font-size: 16px; font-style:bold'>Client Certificate</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use the Upload button to the right to browse and upload the Client Certificate file to the device.</li></ul>");
            define ("TOOLTIP_NETWORK_UPLOADCLIENTCERTIFICATE", "<span style='font-size: 16px; font-style:bold'>Upload Client Certificate</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use this button to browse and upload the Client Certificate file to the device.</li></ul>"); 
            define ("TOOLTIP_NETWORK_CLIENTPRIVATEKEY",  "<span style='font-size: 16px; font-style:bold'>Client Private Key</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use the Upload button to the right to browse and upload the Client Private Key file to the device.</li>
                                                <li>It is also acceptable to upload a combined client key and password (PEM) file.</li></ul>");
            define ("TOOLTIP_NETWORK_UPLOADPRIVATEKEY", "<span style='font-size: 16px; font-style:bold'>Upload Client Certificate</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use this button to browse and upload the Client Private Key file to the device.</li>
                                                <li>It is also acceptable to upload a combined client key and password (PEM) file.</li></ul>"); 
            define ("TOOLTIP_NETWORK_EAPOLVERSION", "<span style='font-size: 16px; font-style:bold'>EAPOL Version</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use the dropdown to specify the version of the EAPOL protocol to use.</li></ul>"); 
            define ("TOOLTIP_NETWORK_EAPIDENTITY", "<span style='font-size: 16px; font-style:bold'>EAP Identity</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Specify your EAP Indentity.</li></ul>"); 
            define ("TOOLTIP_NETWORK_PRIVTEKEYPWD",  "<span style='font-size: 16px; font-style:bold'>Private Key Pasword</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Specify your Private Key Password.</li></ul>"); 
            define ("TOOLTIP_NETWORK_DOWNLOADLOGBUTTON", "<span style='font-size: 16px; font-style:bold'>Download 802.1X Log Button</span>
                                                <ul style='padding-left: 1em'><li>Click to download the ".$NXTText." <i>IEEE 802.1X Log</i> as a text file.</li></ul>");


            //////////////////////////////////////////////////////
            // ADV. DHCP DIALOG HELP STRINGS
            //////////////////////////////////////////////////////
            define ("TOOLTIP_ADVDHCP_TIMEOUT", "<span style='font-size: 16px; font-style:bold'>DHCP Timeout</span>
                                                <ul style='padding-left: 1em'><li>Specifies the duration in seconds that the ".$NXTText." DHCP client will attempt to resolve the ".$NXTText." <i>hostname</i> before failing.</li>
                                                <li>Duration Range (seconds): 10 - 120.</li>
                                                <li>Default value: 10.</li></ul>");
            define ("TOOLTIP_ADVDHCP_RETRIES", "<span style='font-size: 16px; font-style:bold'>DHCP Retries</span>
                                                <ul style='padding-left: 1em'><li>Specifies the total number of times that the ".$NXTText." DHCP client will retry resolution of the ".$NXTText." <i>hostname</i> when failures occur.</li>
                                                <li>Retries (times) : 0 - 5</li>
                                                <li>Default value: 0.</li></ul>");
            define ("TOOLTIP_ADVDHCP_RETRYDELAY", "<span style='font-size: 16px; font-style:bold'>Retry Delay</span>
                                                <ul style='padding-left: 1em'><li>Specifies the delay in seconds between each DHCP client retry attempt.</li>
                                                <li>Duration Range (seconds): 0 - 60.</li>
                                                <li>Default value: 10.</li></ul>");

            define ("TOOLTIP_IPV6_ENABLE", "<span class='tooltip-header'>Enable IPv6</span>");
            define ("TOOLTIP_IPV6_CONFIGURATION", "<span class='tooltip-header'>Specify IPv6 Configuration Details</span>");
            define ("TOOLTIP_IPV6_DHCP_MODE", "<span class='tooltip-header'>IPv6 DHCP Mode:</span>
                                                <ul class='tooltip-list'>
                                                <li><b>Stateless</b> – DHCPv6 is used to set DNS servers and NTP servers etc, but not to set IPv6 addresses</li>
                                                <li><b>Stateful</b> - DHCPv6 is used to set IPv6 addresses as well as DNS servers etc.</li>
                                                <li><b>Auto</b> – The use of DHCPv6 is determined by the router advertisements</li>
                                                <li><b>Off</b> - DHCPv6 is disabled.</li>
                                                </ul>");
            define ("TOOLTIP_IPV6_ADDRESS", "<span class='tooltip-header'>IPv6 Network Address</span>");
            define ("TOOLTIP_IPV6_GATEWAY", "<span class='tooltip-header'>IPv6 Default Gateway Address</span>");
            define ("TOOLTIP_IPV6_DNS1", "<span class='tooltip-header'>IPv6 Pirmary DNS Server Address</span>");
            define ("TOOLTIP_IPV6_DNS2", "<span class='tooltip-header'>IPv6 Secondary DNS Server Address</span>");
			define ("TOOLTIP_IPV6_STATUS", "<span class='tooltip-header'>Current IPv6 addresses assigned to the EXT device</span>
									<ul class='tooltip-list'>
									<li><b>Global</b> – addresses that are globally valid.</li>
									<li><b>Link-local</b> - addresses that are valid only for communications within the network segment or the broadcast domain that the EXT device is connected to.</li>
			<li><b>Stateless</b> – addresses installed due to stateless address configuration.</li>
									<li><b>Temporary</b> - temporary random addresses (documented in RFC 4941).</li>
									</ul>");

            define ("TOOLTIP_DEVICE_TAB", "<b><i>Device Tab</i></b><br>Configuration of ".$NXTText."  device. (LED Brightness, Volume Control, Time Settings, Reboot and Factroy Reset)");
            define ("TOOLTIP_DEVICE_SPEAKERVOLUME", "<span style='font-size: 16px; font-style:bold'>Speaker Volume</span>
                                                <ul style='padding-left: 1em'><li>Adjust the volume of the ".$NXTText." speaker.</li>
                                                <li>Specify a value of '0' to MUTE the speaker.</li>
                                                <li>Volume Range: 0 - 100.</li>
                                                <li>Default value: 40.</li></ul>");
            define ("TOOLTIP_DEVICE_LEDBRIGHTNESS", "<span style='font-size: 16px; font-style:bold'>LED Brightness</span>
                                                <ul style='padding-left: 1em'><li>Adjust the ".$NXTText." LED Brightness level.</li>
                                                <li>Specify a value of '0' to turn off the LEDs.</li>
                                                <li>Brightness Range: 0 - 100.</li>
                                                <li>Default value: 20.</li></ul>");
            define ("TOOLTIP_DEVICE_TAMPERVOLUME", "<span style='font-size: 16px; font-style:bold'>Tamper Tone Volume</span>
                                                <ul style='padding-left: 1em'><li>Adjust the volume level of the <i>Tamper Alarm</i>.</li>
                                                <li>Specify a value of '0' to MUTE the alarm.</li>
                                                <li>Volume Range: 0 - 100.</li>
                                                <li>Default value: 10.</li></ul>");
            define ("TOOLTIP_DEVICE_TIMESERVER", "<span style='font-size: 16px; font-style:bold'>Internet Time Server</span>
                                                <ul style='padding-left: 1em'><li>The Web Address of the <i>Internet Time Server</i> from which to retrieve the curent time.</li>
                                                <li>Note that the ".$NXTText." <b><i>MUST</i></b> have access to the <i>Internet Time Server</i> for this function to work.</li>
                                                <li>Default value: time.nist.gov.</li></ul>");
            define ("TOOLTIP_DEVICE_SYNCHRONIZEDAILY", "<span style='font-size: 16px; font-style:bold'>Synchronize Time Daily</span>
                                                <ul style='padding-left: 1em'><li>This option enables the ".$NXTText." to synchronize with the specified <i>Internet Time Server</i> once per day.</li>
                                                <li>Default value: Enabled.</li></ul>");
            define ("TOOLTIP_DEVICE_LOCATEDEVICE", "<span style='font-size: 16px; font-style:bold'>Locate Device Button</span>
                                                <ul style='padding-left: 1em'><li>Click to flash the ".$NXTText." LEDs.</li>
                                                <li>The LEDs will continue to flash until the user cancels the operation.</li></ul>");
            define ("TOOLTIP_DEVICE_SYNCHRONIZENOW", "<span style='font-size: 16px; font-style:bold'>Synchronize Now! Button</span>
                                                <ul style='padding-left: 1em'><li>Click to immediately synchronize ".$NXTText." time against the specified <i>Internet Time Server</i>.</li></ul>");
	    define ("TOOLTIP_DEVICE_SYNCHRONIZEWITHHOST", "<span style='font-size: 16px; font-style:bold'>Synchronize With Host Button</span>
                                                <ul style='padding-left: 1em'><li>Click to immediately synchronize ".$NXTText." time with current host/system time.</li></ul>");
            define ("TOOLTIP_DEVICE_FACTORYRESET", "<span style='font-size: 16px; font-style:bold'>Factory Reset Button</span>
                                                <ul style='padding-left: 1em'><li>Click to reset ".$NXTText." to its <i>Factory</i> settings.</li>
                                                <li>When the ".$NXTText." is reset to its factory settings:</li><ol>
                                                <li>All settings are reset to default values.</li>
                                                <li>Network settings are reverted to default (DHCP).</li>
                                                <li>The database on the ".$NXTText." will be cleared</li>
                                                <li>Log files stored on the ".$NXTText." are cleared</li>
                                                <li>".$NXTText." reboots.</li>
                                                </ol></ul>");
			
			 define ("TOOLTIP_ACS_INSTALLATION_MODE", "<span style='font-size: 16px; font-style:bold'>Secure Mode OSDP Installation Mode</span>
                                                <ul style='padding-left: 1em'><li>Click to set ".$NXTText." into Secure Mode OSDP installation mode.</li>
                                                <li>When in this mode the ".$NXTText." can receive Secure Mode OSDP SCBK from the Control Panel. </li><ol>
                                                <li>This mode is required to use Secure Mode OSDP with a new Control Panel or during a new install.</li>
												<li>After confirming this option currently stored SCBK is deleted.  <b>THIS OPERATION IS NOT REVERSIBLE</b></li>
                                               
                                                </ol></ul>");
			
            define ("TOOLTIP_DEVICE_REBOOTDEVICE", "<span style='font-size: 16px; font-style:bold'>Reboot Button</span>
                                                <ul style='padding-left: 1em'><li>Click to perform a soft reboot of the ".$NXTText.".</li>
                                                <li>Rebooting will cause WebConfig to disconnect from the ".$NXTText.".</li>
                                                <li>Allow time for WebConfig to reconnect to the ".$NXTText." as the rebooting sequence progresses.</li>
                                                <li>If WebConfig fails to reconnect automatically within 2 minutes, refresh this page from the browser after verifying that the ".$NXTText." is running.</li></ul>");


            define ("TOOLTIP_SECURITY_TAB", "<b><i>Security Tab</i></b><br>".$NXTText."  security configuration. (Key Management, Reset Password and Tamper Settings)");
            define ("TOOLTIP_SECURITY_TAMPERACTIVEHIGH", "<span style='font-size: 16px; font-style:bold'>Tamper Active on Signal High</span>
                                                		   <ul style='padding-left: 1em'><li>If a card reader is attached to the system for dual authentication, this setting should match the tamper output for the card reader.</li>
								   	   <li>'Activate Tamper State on Signal High' should be selected if the card reader tamper condition activates on a high signal.</li></ul>");
            define ("TOOLTIP_SECURITY_TAMPERACTIVELOW", "<span style='font-size: 16px; font-style:bold'>Tamper Active on Signal Low</span>
                                                		   <ul style='padding-left: 1em'><li>If a card reader is attached to the system for dual authentication, this setting should match the tamper output for the card reader.</li> 
									   <li>'Activate Tamper State on Signal Low' should be selected if the card reader tamper condition activates on a low signal.</li></ul>");
             define ("TOOLTIP_OUTPUT_SECURITY_TAMPERACTIVEHIGH", "<span style='font-size: 16px; font-style:bold'>Signal High</span>
                                                		   <ul style='padding-left: 1em'><li>Set the output HIGH if tamper is detected.</li></ul>");
            define ("TOOLTIP_OUTPUT_SECURITY_TAMPERACTIVELOW", "<span style='font-size: 16px; font-style:bold'>Tamper Active on Signal Low</span>
                                                		   <ul style='padding-left: 1em'><li>Set the output LOW if tamper is detected.</li></ul>");


            define ("TOOLTIP_SECURITY_TAMPERNOTIFYADDR", "<span style='font-size: 16px; font-style:bold'>Notification Address</span>
                                                <ul style='padding-left: 1em'><li>Specify the IP address of the system/application that will receive the <i>Tamper Message</i></li></ul>");
            define ("TOOLTIP_SECURITY_TAMPERNOTIFYPORT", "<span style='font-size: 16px; font-style:bold'>Notification Port</span>
                                                <ul style='padding-left: 1em'><li>Specify the Port of the system/application that will receive the <i>Tamper Message</i></li></ul>");
            define ("TOOLTIP_SECURITY_TAMPERMESSAGE", "<span style='font-size: 16px; font-style:bold'>Tamper Message</span>
                                                <ul style='padding-left: 1em'><li>Specify the message that will be sent to the location above when a tamper occurs on the ".$NXTText.".</li></ul>");
            define ("TOOLTIP_SECURITY_OLDPWD", "<span style='font-size: 16px; font-style:bold'>Old Password</span>
                                                <ul style='padding-left: 1em'><li>Enter the existing ".$NXTText." logon password in this field.</li></ul>");
            define ("TOOLTIP_SECURITY_NEWPWD", "<span style='font-size: 16px; font-style:bold'>New Password</span>
                                                <ul style='padding-left: 1em'><li>Enter the desired ".$NXTText." logon password in this field.</li></ul>");
            define ("TOOLTIP_SECURITY_CONFIRMPWD", "<span style='font-size: 16px; font-style:bold'>Confirm Password</span>
                                                <ul style='padding-left: 1em'><li>Re-enter the desired ".$NXTText." logon password in this field.</li></ul>");
            define ("TOOLTIP_SECURITY_UPDATEPWDBUTTON", "<span style='font-size: 16px; font-style:bold'>Reset Password Button</span>
                                                <ul style='padding-left: 1em'><li>Click to reset the ".$NXTText." password.</li>
                                                <li>After the password is reset, Web Config redirects to the Logon screen. Enter the new password to log in.</li>
                                               </ul>");
	     define ("TOOLTIP_SECURITY_NANONXT_TEXT", $NXTText."  Device");
	     define ("TOOLTIP_SECURITY_PC_TEXT", "Host System");
	     define ("TOOLTIP_SECURITY_VALIDKEY_TEXT", "Valid Key");
	     define ("TOOLTIP_SECURITY_INVALIDKEY_TEXT", "Invalid/Expired Key!");
	     define ("TOOLTIP_SECURITY_REGENERATEKEY_TEXT", "Regenerate Key");
	     define ("TOOLTIP_SECURITY_DELETEKEY_TEXT", "Delete Key");
	     define ("TOOLTIP_SECURITY_DOWNLOADKEY_TEXT", "Download Key");
	     define ("TOOLTIP_SECURITY_KEYMGMTBUTTON", "<span style='font-size: 16px; font-style:bold'>Download Key</span>
                                                <ul style='padding-left: 1em'><li>Download the custom Key to this computer.</li>
                                                <li>The key must be downloaded each time the ".$NXTText." is switched from Use Default Key to Use Custom Key.</ul>");
	     define ("TOOLTIP_SECURITY_ADDHOSTKEYBUTTON", "<span style='font-size: 16px; font-style:bold'>Add Custom Key</span>
                                                <ul style='padding-left: 1em'><li>Click to add new custom key to the ".$NXTText.".</li></ul>");
	     define ("TOOLTIP_SECURITY_DELETEALLHOSTKEYSBUTTON", "<span style='font-size: 16px; font-style:bold'>Delete Custom Key</span>
                                                <ul style='padding-left: 1em'><li>Click to delete the custom key present on the ".$NXTText.".</li></ul>");
    
        define ("TOOLTIP_SECURITY_KEYMGMTDEFAULTRADIO", "<span style='font-size: 16px; font-style:bold'>Use Default Key</span>
                                                <ul style='padding-left: 1em'><li>Use the default key provided with Eyelock products to talk to this ".$NXTText.".</li>
                                                <li><a id='DefaultKeyExpiry'>Default Key Expires on 03-19-2025</a></li></ul>");
         define ("TOOLTIP_SECURITY_KEYMGMTCUSTOMRADIO", "<span style='font-size: 16px; font-style:bold'>Use Custom Key</span>
                                                <ul style='padding-left: 1em'><li>The ".$NXTText." will generate a custom key to use for talking to it.  Download the key to assign it in Eyelock applications.</li></ul>");

        define ("TOOLTIP_SECURITY_KEYMGMTVALIDITY", "<span style='font-size: 16px; font-style:bold'>Key Expires...</span>
                                                <ul style='padding-left: 1em'><li>The custom key is valid until the shown date and time.</li></ul>");

            define ("TOOLTIP_SOFTWARE_TAB", "<b><i>Software Tab</i></b><br>Configure ".$NXTText."  Firmware. (Update Firmware, Restore Firmware)");
            define ("TOOLTIP_SOFTWARE_UPDATENOWBUTTON", "<span style='font-size: 16px; font-style:bold'>Update Now! Button</span>

                                                <ul style='padding-left: 1em'><li>This button appears if a newer version of the ".$NXTText." firmware is available from the Eyelock.com website.</li>
                                                <li>Click to start the automatic software upgrade process.</li></ul>");
            define ("TOOLTIP_SOFTWARE_LOCALFILEBUTTON", "<span style='font-size: 16px; font-style:bold'>Local File... Button</span>
                                                <ul style='padding-left: 1em'><li>Clicking this button allows you to upgrade the ".$NXTText." firmware from a locally accessible Eyelock Upgrade Package file.</li>
                                                <li>After clicking the button, select the package file to begin the upgrade process.</li>
                                                 <li><strong>WARNING: Do not disconnect power to the Nano, disconnect the network to your workstation, or close this browser window during the update.</strong></li></ul>");
            define ("TOOLTIP_SOFTWARE_PTLOCALFILEBUTTON", "<span style='font-size: 16px; font-style:bold'>Local File... Portable Template Reader Firmware Management</span>
                                                <ul style='padding-left: 1em'><li>Clicking this button allows you to upgrade the Portable Template Reader firmware from a locally accessible Frimware image (.bin file).</li>
                                                <li>After clicking the button, select the image file to begin the upgrade process.</li>
                                                 <li><strong>WARNING: Do not disconnect power to the Nano, disconnect the network to your workstation, or close this browser window during the update.</strong></li></ul>");
			
			define ("TOOLTIP_SOFTWARE_DETAILSBUTTON", "<span style='font-size: 16px; font-style:bold'>Details... Button</span>
                                                <ul style='padding-left: 1em'><li>Clicking this button displays the ".$NXTText." upgrade dialog containing the details of the currently available upgrade package file.</li>
                                                <li>Begin the upgrade from the Details screen.</li></ul>");

            define ("TOOLTIP_SOFTWARE_ALLOWSITEADMINUPDATE", "<span style='font-size: 16px; font-style:bold'>Allow admin to Update</span>
                                                <ul style='padding-left: 1em'><li>Enable this option to allow a user who is logged in as <i>admin</i> to upgrade the ".$NXTText.".</li>
                                                <li>This option is only visible to the <i>installer</i>.</li></ul>");

            define ("TOOLTIP_SOFTWARE_NANORESTORE", "<span style='font-size: 16px; font-style:bold'>".$NXTText." Restore Points</span>
                                                <ul style='padding-left: 1em'><li>Select this option to display a list of available ".$NXTText." firmware Restore Points.</li>
                                                <li>Restore Points can be restored or deleted by selecting the checkbox next to the Restore Point and clicking either the <i>Restore...</i> or <i>Delete...</i> button.</li></ul>");
            define ("TOOLTIP_SOFTWARE_BOBRESTORE", "<span style='font-size: 16px; font-style:bold'>".$NXTText." Break Out Board Restore Points</span>
                                                <ul style='padding-left: 1em'><li>Select this option to display a list of available ".$NXTText." Break out Board firmware Restore Points.</li>
                                                <li>Restore Points can be restored or deleted by selecting the checkbox next to the Restore Point and clicking either the <i>Restore...</i> or <i>Delete...</i> button.</li></ul>");
            define ("TOOLTIP_SOFTWARE_RESTOREBUTTON", "<span style='font-size: 16px; font-style:bold'>Restore Points Restore Button</span>
                                                <ul style='padding-left: 1em'><li>Click to revert the ".$NXTText." firmware back to a previous Restore Point.</li>
                                                <li>Only the 1st 'checked' Restore Point will be restored.</li>
                                                <li>The restoration is an automated process and cannot be stopped once it has begun.</li>
                                                <li>The ".$NXTText." will retain up to five (5) restore points.</li></ul>");
            define ("TOOLTIP_SOFTWARE_DELETEBUTTON", "<span style='font-size: 16px; font-style:bold'>Restore Points Delete Button</span>
                                                <ul style='padding-left: 1em'><li>Clicking this button will permanently delete all of the 'checked' Restore Points from the ".$NXTText.".</li>
                                                <li>The deleted Restore Points cannot be retrieved.</li></ul>");


            define ("TOOLTIP_AUTHENTICATION_TAB", "<b><i>Authentication Tab</i></b><br>Configuration of ".$NXTText."  authentication parameters. (Match Mode, Repeat Authorization Period, Negative Match Timeout etc.)");
            define ("TOOLTIP_AUTHENTICATION_SINGLEEYE", "<span style='font-size: 16px; font-style:bold'>Single Eye Authentication</span>
                                                <ul style='padding-left: 1em'><li>Select this option to <i>allow</i> a biometric match on one eye for successful authentication.</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_DUALEYE", "<span style='font-size: 16px; font-style:bold'>Dual Eye Authentication</span>
                                                <ul style='padding-left: 1em'><li>Select this option to <i>require</i> a biometric match on both eyes for successful authentication.</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_REPEATPERIOD", "<span style='font-size: 16px; font-style:bold'>Repeat Authorization Period</span>
                                                <ul style='padding-left: 1em'><li>The <i>minimum</i> amount of time which must elapse between successful authentications.  This may have a minimum of 2 seconds to maintain image quality.</li>
                                                <li>Range (seconds): 2-60.</li>
                                                <li>Default value: 4.</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_ENABLENEGMATCHTIMEOUT", "<span style='font-size: 16px; font-style:bold'>Enable Negative Match Timeouts</span>
                                                <ul style='padding-left: 1em'><li>Select this option to enable processing of <i>Negative Match</i> timeout rules.</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_LOITERPERIOD", "<span style='font-size: 16px; font-style:bold'>Loiter Period</span>
                                                <ul style='padding-left: 1em'><li>Defines the <i>maximum</i> duration in seconds that the ".$NXTText." will attempt authentication before timing out and reporting a negative match.</li>
                                                <li>Range (seconds): 2-60.</li>
                                                <li>Default value: 6.</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_NEGMATCHRESET", "<span style='font-size: 16px; font-style:bold'>Negative Match Reset Timer</span>
                                                <ul style='padding-left: 1em'><li>Defines the <i>minimum</i> duration in seconds that must elapse after a negative match before the ".$NXTText." attempts another authentication.</li>
                                                <li>Range (seconds): 2-60.</li>
                                                <li>Default value: 4.</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_NETWORKMSGADDR", "<span style='font-size: 16px; font-style:bold'>Network Message Destination IP</span>
                                                <ul style='padding-left: 1em'><li>Specify the IP Address of the system/application that will receive the <i>Network Match</i> message.</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_NETWORKMSGPORT", "<span style='font-size: 16px; font-style:bold'>Notification Port</span>
                                                <ul style='padding-left: 1em'><li>Specify the Port of the system/application that will receive the <i>Network Match</i> message.</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_NETWORKMSGMSG", "<span style='font-size: 16px; font-style:bold'>Network Message Format</span>
                                                <ul style='padding-left: 1em'><li>Specify the format string for the message that will be sent to the location above when a successful match/authentication occurs.</li>
                                                <li>Default value: Matched:%d;Score:%0.4f;Time:%llu;ID:</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_IRISPROCESSINGMODE", "<span style='font-size: 16px; font-style:bold'>Iris Processing Mode</span>
                                                <ul style='padding-left: 1em'><li>Specify the mode for processing irises.  There are two options:</li>
                                                <li>Access Control Authentication - Uses matching feature to authenticate user irises and, optionally, interface with ACS systems.</li>
                                                <li>Iris Capture Mode - Capture irises and send them to a remote HTTP Endpoint.  No matching or ACS integration occurs.</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_CAPTURETIMEOUT", "<span style='font-size: 16px; font-style:bold'>Iris Capture Timeout</span>
                                                <ul style='padding-left: 1em'><li>Specify the during in milliseconds for processing user irises until sending the message to the HTTP endpoint.</li>
                                                <li>Range (Milliseconds): 1000-60000.</li>
                                                <li>Default value: 5000.</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_CAPTURERESETDELAY", "<span style='font-size: 16px; font-style:bold'>Iris Capture Reset Delay</span>
                                                <ul style='padding-left: 1em'><li>Specify the delay in milliseconds that the device will wait after capturing irises before processing the next user.</li>
                                                <li>Range (Milliseconds): 1000-300000.</li>
                                                <li>Default value: 5000.</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_MESSAGEFORMAT", "<span style='font-size: 16px; font-style:bold'>HTTP Message Format</span>
                                                <ul style='padding-left: 1em'><li>Specify the format of the message which will be sent to the HTTP Endpoint.</li>
                                                <li>Default value: SOAP (see documentation for schema of SOAP message)</li>
                                                <li>Alertnate value: JSON (see documentation for schema of JSON message)</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_PAYLOADFORMAT", "<span style='font-size: 16px; font-style:bold'>Message Image Payload Format</span>
                                                <ul style='padding-left: 1em'><li>Specify the graphical format of the iris images which will be sent to the HTTP Endpoint.</li>
                                                <li>Option 1: RAW - 8-bit, Single Channel image</li>
                                                <li>Option 2: PNG</li>
                                                <li>Option 2: J2K (JPEG 2000)</li>
                                                <li>Default value: RAW - 8-bit, Single Channel image</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_IRISIMAGEQUALITY", "<span style='font-size: 16px; font-style:bold'>Iris Image Quality</span>
                                                <ul style='padding-left: 1em'><li>Specify the quality of the encoded JPEG 2000 iris image.  A value of 100 uses lossless compression.</li>
                                                <li>Default value: 100</li></ul>");
            define ("TOOLTIP_IRISCAPTURE_WECOMEMSG", "<span style='font-size: 16px; font-style:bold'>Welcome Message</span>
                                                <ul style='padding-left: 1em'><li>Define the text string sent as part of the Iris Capture message.</li></ul>");
            define ("TOOLTIP_IRISCAPTURE_DEVICELOCATION", "<span style='font-size: 16px; font-style:bold'>Device Location</span>
                                                <ul style='padding-left: 1em'><li>Define a unique string to associate with this device in the Iris Capture message.</li></ul>");
            define ("TOOLTIP_IRISCAPTURE_HEARTBEATURL", "<span style='font-size: 16px; font-style:bold'>Heartbeat URL</span>
                                                <ul style='padding-left: 1em'><li>Define the URL to which this device will send heartbeat messages.    Leave empty to not send heartbeatmessages.</li></ul>");
            define ("TOOLTIP_IRISCAPTURE_POSTSENDERURL", "<span style='font-size: 16px; font-style:bold'>Post Destination Address</span>
                                                <ul style='padding-left: 1em'><li>The base URL to which all Iris Capture messages will be sent..</li></ul>");
            define ("TOOLTIP_IRISCAPTURE_ERRORURL", "<span style='font-size: 16px; font-style:bold'>Signal Error Endpoint</span>
                                                <ul style='padding-left: 1em'><li>The endpiont under the Post Destination Address to which signal error messages will be sent.  Leave empty to not send messages.</li></ul>");
            define ("TOOLTIP_IRISCAPTURE_POSTIRISURL", "<span style='font-size: 16px; font-style:bold'>Post Iris Endpoint</span>
                                                <ul style='padding-left: 1em'><li>The endpiont under the Post Destination Address to which iris images will be sent..</li></ul>");
            define ("TOOLTIP_IRISCAPTURE_MAINTENANCEURL", "<span style='font-size: 16px; font-style:bold'>Maintenance Endpoint</span>
                                                <ul style='padding-left: 1em'><li>The endpiont under the Post Destination Address to which maintenance messages will be sent.  Leave empty to not send messages.</li></ul>");

            define ("TOOLTIP_DATABASE_TAB", "<b><i>Database Tab</i></b><br>".$NXTText."  database configuration. (Network Matcher and Database Statistics)");
            define ("TOOLTIP_DATABASE_ENABLENETWORKMATCHER", "<span style='font-size: 16px; font-style:bold'>Enable Network Matcher</span>
                                                <ul style='padding-left: 1em'><li>Select this option to enable additional, off-device processing of biometric data using the <i>Eyelock Network Matcher Service</i>.</li></ul>");
            define ("TOOLTIP_DATABASE_NETWORKMATCHERADDR", "<span style='font-size: 16px; font-style:bold'>Network Matcher Destination Address</span>
                                                <ul style='padding-left: 1em'><li>Specify the IP Address of the Eyelock Network Matcher Service that will process the authentication request.</li></ul>");
            define ("TOOLTIP_DATABASE_NETWORKMATCHERPORT", "<span style='font-size: 16px; font-style:bold'>Network Matcher Destination Port</span>
                                                <ul style='padding-left: 1em'><li>Specify the Port of the Eyelock Network Matcher Service that will process the authentication request.</li></ul>");
            define ("TOOLTIP_AUTHENTICATION_SECURENETWORK", "<span style='font-size: 16px; font-style:bold'>Enable Secure Network Match Message</span>
                                                <ul style='padding-left: 1em'><li>Enable this option to send match message securely on the network.</li></ul>");
            define ("TOOLTIP_DATABASE_STATISTICS", "<span style='font-size: 16px; font-style:bold'>Remaining Template Space</span>
                                                <ul style='padding-left: 1em'><li>This value specifies the current count of biometric templates stored in the ".$NXTText."'s database out of the available maximum count of templates.</li></ul>");
			define ("TOOLTIP_DATABASE_NETWORKMATCHER_SECURECOMM", "<span style='font-size: 16px; font-style:bold'>Enable Secure Network Matcher</span>
                                                <ul style='padding-left: 1em'><li>Enable this option to communicate securely with the <i>Eyelock Network Matcher Service</i>.</li></ul>");
            define ("TOOLTIP_UPLOAD_PTFIRMWARE", "Upload a Portable Template Reader Firmware (.bin) file.");                       
            define ("TOOLTIP_ACS_TAB", "<b><i>ACS Tab</i></b><br>".$NXTText."  configuration for ACS/Door Panel system.(Protocol, ACS LED Control, Dual Authentication, Relays and ACS Test Option)");


            define ("TOOLTIP_LOGS_TAB", "<b><i>Logs Tab</i></b><br>Displays ".$NXTText."  log and allows the user to download the log file.");
            define ("TOOLTIP_LOGS_REFRESHBUTTON", "<span style='font-size: 16px; font-style:bold'>Refresh Button</span>
                                                <ul style='padding-left: 1em'><li>Clicking this button will synchronize the <i>Event Log</i> with the current contents of the log.</li>
                                                <li>The <i>Event Log</i> is synchronized when WebConfig is initially run. It is not continuously updated.</li></ul>");
            define ("TOOLTIP_LOGS_DOWNLOADBUTTON", "<span style='font-size: 16px; font-style:bold'>Download Button</span>
                                                <ul style='padding-left: 1em'><li>Click to download the ".$NXTText." <i>Event Log</i> as a Microsoft Excel CSV file.</li></ul>");
            define ("TOOLTIP_ACCESSCONTROLTYPE", "<span style='font-size: 16px; font-style:bold'>Authentication Scheme</span>
                                                <ul style='padding-left: 1em'><li>Select the authentication scheme the ".$NXTText." will use</li>
                                                <li>Schemes</li>
                                                <ul><li>Iris Only:  ".$NXTText." only passes iris authentication to the Access System</li>
                                                <li>Iris Or Card:  ".$NXTText." will send either a card credential or an iris authentication to the Access System</li>
                                                <li>Iris And Card:  ".$NXTText." will require both a card credential and an iris authentication (which matches the presented card)</li>
                                                <li>Iris And Card (PIN Pass-Through):  ".$NXTText." will require both a card credential and an iris authentication (which matches the presented card) then passes PIN through to the Access System</li>
                                                <li>PIN And Iris:  ".$NXTText." will send both PIN and an iris authentication to the Access System</li>
                                                <li>PIN, Card AND Iris:  ".$NXTText." will send a PIN, a card credential and an iris authentication (which matches the presented card)</li>
                                                </ul></ul>");

            define ("TOOLTIP_ACS_PROTOCOL", "<span style='font-size: 16px; font-style:bold'>ACS Protocol</span>
                                                <ul style='padding-left: 1em'><li>Use this dropdown to select the ACS Protocol that this ".$NXTText." will use.</li>
                                                <li>The following protocols are supported:</li>
                                                <ul><li>Wiegand</li>
                                                <li>HID Serial</li>
                                                <li>F2F</li>
                                                <li>PAC</li>
                                                <li>OSDP</li></ul></ul>");
            define ("TOOLTIP_OSDP_BAUD", "<span style='font-size: 16px; font-style:bold'>OSDP Baud Rate</span>
                                                <ul style='padding-left: 1em'><li>Controls the half duplex asynchronous serial communication rate for both the RS-485 Input and the RS-485 Output. Signalling defaults to 8 data bits, 1 stop bit an no parity bits. The default value for the Baud Rate is 9600.</li>
                                                </ul>");
            define ("TOOLTIP_OSDP_ADDRESS", "<span style='font-size: 16px; font-style:bold'>OSDP Address</span>
                                                <ul style='padding-left: 1em'><li>Specifies the OSDP address for the Output on a multi-drop RS-485 network. The default value for the address is 0.</li>
                                                </ul>");
            define ("TOOLTIP_ACS_LEDCONTROLLEDACS", "<span style='font-size: 16px; font-style:bold'>LED Controlled by ACS</span>
                                                		<br /><br />LED control allows for the ACS to control the state of the ".$NXTText." LEDs and the sounder. When LED control is not checked, the ".$NXTText." manages the LEDs internally.");
            define ("TOOLTIP_ACS_DUALAUTHENTICATION", "<span style='font-size: 16px; font-style:bold'>Dual Authentication</span>
                                                		<br /><br />Dual authentication requires that a user present a card to the attached card reader prior to presenting their eyes. 
								<br />The ".$NXTText." will look up the card in internal memory and then prompt the user the present their eyes by turning white. 
								<br />If the Iris presented matches the Iris is the record that was looked up, the ".$NXTText." will send the data the ACS. 
								<br />If a match is not made, the ".$NXTText." will send the negative match code.");

                                
            define ("TOOLTIP_ACS_TEMPLATEONCARDPASS", "<span style='font-size: 16px; font-style:bold'>Single Factor Authentication</span>
                                                		<br /><br />".$NXTText." will send a presented credential from either a connected Card Reader or from an Iris match to the Access Control Panel.");
            define ("TOOLTIP_ACS_TEMPLATEONCARD", "<span style='font-size: 16px; font-style:bold'>Portable Templates</span>
                                                		<br /><br />".$NXTText." will expect the user to present a credential card or mobile device containing their biometric  template before matching.
								<br />The ".$NXTText." will wait for the credential to be presented and then prompt the user the present their eyes by turning white.
                                <br /><strong>Saving after enabling Portable Templates will clear the local database on the ".$NXTText.".</strong>");
            define ("TOOLTIP_ACS_DUALAUTHPARITY", "<span style='font-size: 16px; font-style:bold'>Dual Authentication Parity</span>
                                                		<br /><br />Choose whether to check parity bits coming from the card reader during Dual Authentication card reading. ");
            define ("TOOLTIP_ACS_IRISWAITTIME", "<span style='font-size: 16px; font-style:bold'>Iris Wait Time</span>
                                                <ul style='padding-left: 1em'><li>Specifies the duration, in seconds, within which the user must present his iris' to the ".$NXTText." after scanning the card</li>
                                                <li>Duration Range (seconds): 2 - 60.</li>
                                                <li>Default value: 10.</li></ul>");
            define ("TOOLTIP_ACS_PINWAITTIME", "<span style='font-size: 16px; font-style:bold'>PIN Wait Time</span>
                                                <ul style='padding-left: 1em'><li>Specifies the duration, in seconds, within which the user must enter his PIN after scanning the card</li>
                                                <li>Duration Range (seconds): 2 - 60.</li>
                                                <li>Default value: 10.</li></ul>");
            define ("TOOLTIP_ACS_PINBURSTBITS", "<span style='font-size: 16px; font-style:bold'>PIN Burst Bits</span>
                                    <ul style='padding-left: 1em'><li>Specifies the bit size for PIN processing</li>
                                    <li>Default value: 4.</li></ul>");
            define ("TOOLTIP_ACS_ENABLERELAYS", "<span style='font-size: 16px; font-style:bold'>Enable Relays</span>
                                                <ul style='padding-left: 1em'><li>When checked, this option enables the operation of the physical relays on the ACS Panel.</li>
                                                <li>Both <i>Grant</i> and <i>Deny</i> relays are supported.</li>
                                                <li>The duration that the relays remain active can be configured using the sliders below.</li></ul>");
            define ("TOOLTIP_ACS_GRANTRELAYTIME", "<span style='font-size: 16px; font-style:bold'>Grant Relay Time</span>
                                                <ul style='padding-left: 1em'><li>Specifies the duration in seconds to trigger the <i>Grant</i> relay if authentication is successful.</li>
                                                <li>To disable the <i>Grant</i> relay individually, set this value to 0.</li>
                                                <li>Duration Range (seconds): 0 - 10.</li>
                                                <li>Default value: 3.</li></ul>");
            define ("TOOLTIP_ACS_DENYRELAYTIME", "<span style='font-size: 16px; font-style:bold'>Deny Relay Time</span>
                                                <ul style='padding-left: 1em'><li>Specifies the duration in seconds to trigger the <i>Deny</i> relay if authentication fails.</li>
                                                <li>To disable the <i>Deny</i> relay individually, set this value to 0.</li>
                                                <li>Duration Range (seconds): 0 - 10.</li>
                                                <li>Default value: 5.</li></ul>");
            define ("TOOLTIP_ACS_DURESSRELAYTIME", "<span style='font-size: 16px; font-style:bold'>Duress Relay Time</span>
                                                <ul style='padding-left: 1em'><li>Specifies the duration in seconds to trigger the <i>Duress</i> relay if authentication succeeds.</li>
                                                <li>To disable the <i>Duress</i> relay individually, set this value to 0.</li>
                                                <li>Duration Range (seconds): 0 - 10.</li>
                                                <li>Default value: 5.</li></ul>");
            define ("TOOLTIP_ACS_TESTCODE", "<span style='font-size: 16px; font-style:bold'>Facility Code</span>
                                                <ul style='padding-left: 1em'><li>Displays the preconfigured Facility Code to be used with the selected ACS protocol when the <i>Test Now!</i> button is clicked.</li>
                                                <li>Use the EyeEnroll application to preconfigure this value before testing the ACS system.</li></ul>");
            define ("TOOLTIP_ACS_TESTCARDID", "<span style='font-size: 16px; font-style:bold'>Card ID</span>
                                                <ul style='padding-left: 1em'><li>Displays the preconfigured Card ID to be used with the selected ACS protocol when the <i>Test Now!</i> button is clicked.</li>
                                                <li>Use the EyeEnroll application to preconfigure this value before testing the ACS system.</li></ul>");
            define ("TOOLTIP_ACS_TESTACSBUTTON", "<span style='font-size: 16px; font-style:bold'>Test Now! Button</span>
                                                <ul style='padding-left: 1em'><li>Clicking this button will will send the preconfigured test string to the ACS panel.  The EyEnroll application must be used to define the test string on the ".$NXTText.".</li></ul>");


            //////////////////////////////////////////////////////////////////////////////
            // Other Tool Tips...
            //////////////////////////////////////////////////////////////////////////////
            define ("TOOLTIP_HEADER_LANGUAGE", "<span style='font-size: 16px; font-style:bold'>Language Dropdown</span>
                                                <ul style='padding-left: 1em'><li>Use this dropdown to select the current language for the WebConfig user interface.</li>
                                                <li>Only languages which are available will be listed.</li></ul>");
            define ("TOOLTIP_HEADER_EYELOCKLOGO", "<span style='font-size: 16px; font-style:bold'>Eyelock Corporation</span>
                                                <ul style='padding-left: 1em'><li>Click <a href='http://www.eyelock.com'  target='_blank'>here</a> to visit the Eyelock website.</li></ul>");
            define ("TOOLTIP_HEADER_CLIENTVER", "<span style='font-size: 16px; font-style:bold'>WebConfig Client Version</span>
                                                <ul style='padding-left: 1em'><li>Represents the current version of the WebConfig client in use.</li></ul>");
            define ("TOOLTIP_HEADER_LICENSE", "<span style='font-size: 16px; font-style:bold'>License Link</span>
                                                <ul style='padding-left: 1em'><li>Click to view the <i>End User License Agreement</i>.</li></ul>");
            define ("TOOLTIP_HEADER_HELP", "<span style='font-size: 16px; font-style:bold'>Help Link</span>
                                                <ul style='padding-left: 1em'><li>Clicking this link will bring up a dialog box where <i>Popup Help System</i> settings can be modified.</li></ul>");
            define ("TOOLTIP_HEADER_LOGOUT", "<span style='font-size: 16px; font-style:bold'>Logout Link</span>
                                                <ul style='padding-left: 1em'><li>Clicking this link will log the user out of the current session and return to the login page.</li></ul>");

            define ("TOOLTIP_FOOTER_APPVERSION", "<span style='font-size: 16px; font-style:bold'>".$NXTText." Firmware Version</span>
                                                <ul style='padding-left: 1em'><li>Represents the current version of the application firmware that is running on the ".$NXTText.".</li></ul>");
            define ("TOOLTIP_FOOTER_BOBVERSION", "<span style='font-size: 16px; font-style:bold'>".$NXTText." ICM Version</span>
                                                <ul style='padding-left: 1em'><li>Represents the current version of the ICM that is running on the ".$NXTText.".</li></ul>");

            define ("TOOLTIP_FOOTER_SAVEBUTTON", "<span style='font-size: 16px; font-style:bold'>Save Button</span>
                                                <ul style='padding-left: 1em'><li>Click to permanently save any changes made to the ".$NXTText." settings.</li>
                                                <li>After the settings have been committed, the ".$NXTText." will automatically restart with the new settings taking effect immediately.</li></ul>");
			
			 define ("TOOLTIP_FOOTER_CANCELBUTTON", "<span style='font-size: 16px; font-style:bold'>Cancel Button</span>
                                                <ul style='padding-left: 1em'><li>Click to discard changes made and refresh the Configuration Page.</li>
                                                </ul>");

            define ("TOOLTIP_TAB_DEVICEIP", "<span style='font-size: 14px; font-style:bold'>Current ".$NXTText."  Name/IP Address</span>");
            define ("TOOLTIP_TAB_DEVICEMACADDR", "<span style='font-size: 14px; font-style:bold'>Current ".$NXTText."  MAC Address</span>");

            //////////////////////////////////////////////////////////////////////////////
            // Validation Error Messages
            //////////////////////////////////////////////////////////////////////////////
            define ("VALIDATOR_MSG_HOSTNAME", "Hostname must not contain spaces or special characters,</br> and must be less than 64 characters in length!");
            define("TITLE_BTN_TESTNWMS", "Test the currently applied Network Matcher settings.");
            define("TEST_NWMS", "Test NWMS");
            define("CB_NWMATCHERCOMMSECURE_TEXT", "unsecure");
            define("COPYRIGHTTEXT", "Copyright \AD&#169; 2014.  All Rights Reserved.");
            define("DISCONNECTWARNING", "Do not close the browser, disconnect power to the ".$NXTText.", or remove network connection during this process.");
            break;
   case "ru":
		define("ID", "Russian");
		define("TITLE_NANOLOGIN", "Вход");
		define("LOGIN_USERNAME", "Имя пользователя");
		define("LOGIN_PASSWORD", "Пароль");
		define("LOGIN_LOGIN", "Войти");
		define("LOGIN_FORGOTPASSWORD", "Забыли пароль?");
		define("LOGIN_USER_GROUPTITLE", "Группы пользователей");
		define("TITLE_NANOCONFIG", "Настройки ".$NXTText." ");
		define("TITLE_HELP", "Настройка справки");
		define("TITLE_LOGOUT", "Выход");
		define("HOME_TABLABEL", "Главная");
		define("HOME_HEADING", "Информация о ".$NXTText." ");
		define("HOME_DEVICEINFO_GROUPTITLE", "Информация об устройстве");
		define("HOME_DEVICEINFO_DEVICENAME", "Имя ".$NXTText." :");
        define("HOME_DEVICEINFO_IPADDR", "IP-адрес:");
        define("HOME_DEVICEINFO_IPV6ADDR", "IPv6ddress:");
		define("HOME_DEVICEINFO_SERIAL", "Device ID:");
		define("HOME_DEVICEINFO_MACADDR", "MAC-адрес:");
		define("HOME_DEVICEINFO_LOCALTIME", "Точка восстановления ".$NXTText.":");
		define("HOME_STORAGESTATS_GROUPTITLE", "Статистика используемой памяти ");
		define("HOME_STORAGESTATS_NETWORKMATCHENABLED", "Network Matcher:");
		define("HOME_STORAGESTATS_NETWORKMATCHADDRESS", "Network Matcher адрес:");
		define("HOME_STORAGESTATS_TEMPLATESPACE", "Количество незанятых пользователей");
		define("HOME_SOFTWARE_GROUPTITLE", "Информация о ПО");
		define("HOME_SOFTWARE_APPVER", "Версия прошивки ".$NXTText." :");
        define ("HOME_CAMERA_FPGAVER", "Camera FPGA Ver.: ");
        define ("HOME_CAMERA_PSOCVER", "Camera PSOC Ver.: ");
        define ("HOME_FIXED_BOARDVER", "Fixed Board Ver.: ");
		define("HOME_SOFTWARE_BOBVER", "Версия прошивки ICM:");
		define("HOME_HARDWARE_BOBVER", "Версия аппаратного обеспечения ICM:");
		define("HOME_SOFTWARE_LINUXVER", "Версия ОС Linux:");
		define("HOME_HARDWARE_PSOCVER", "Версия программируемой системы PSOC:");
		define("HOME_SOFTWARE_NANOLABEL", "Последнее обновление nano:");
		define("HOME_SOFTWARE_BOBLABEL", "Последнее обновление ICM:");
		define("HOME_SOFTWARE_FPGALABEL", "Последнее обновление Camera FPGA: ");
		define("HOME_SOFTWARE_CAMERAPSOCLABEL", "Последнее обновление Camera PSOC: ");
		define("HOME_SOFTWARE_FIXEDPSOCLABEL", "Последнее обновление Fixed Board: ");
		define("NETWORK_TABLABEL", "Сеть");
		define("NETWORK_HEADING", "Настройка сети для устройства:");
		define("NETWORK_DEVICENAME_GROUPTITLE", "Имя устройства:");
		define("NETWORK_DEVICENAME_LABEL", "Имя:");
		define("NETWORK_DHCP_GROUPTITLE", "Получать IP-адрес автоматически (DHCP)");
		define("NETWORK_DHCP_SETTINGS", "Настройки DHCP...");
		define("NETWORK_DHCPSETTINGS_TITLE", "Расширенные настройки DHCP");
		define("NETWORK_DHCPSETTINGS_TIMEOUTS_GROUPLABEL", "Настройки таймаутов");
		define("NETWORK_DHCPSETTINGS_TIMEOUTS_DHCPTIMEOUTLABEL", "Таймаут DHCP:");
		define("NETWORK_DHCPSETTINGS_TIMEOUTS_DHCPRETRIESLABEL", "Количество попыток в DHCP:");
		define("NETWORK_DHCPSETTINGS_TIMEOUTS_DHCPRETRYDELAYLABEL", "Задержка при повторной попытке:");
		define("NETWORK_DHCPSETTINGS_OK", "Да");
		define("NETWORK_STATICIP_GROUPLABEL", "Используйте следующий статический IP-адрес");
		define("NETWORK_STATICIP_DEVICEIPLABEL", "IP-адрес устройства:");
		define("NETWORK_STATICIP_BROADCASTNETWORKLABEL", "Ципкулярная сеть");
		define("NETWORK_STATICIP_SUBNETMASKLABEL", "Маска подсети:");
        define("NETWORK_STATICIP_DEFAULTGATEWAYLABEL", "Шлюз по умолчанию:");
        define ("NETWORK_IPV6_ENABLE", "Enable IPv6");
        define ("NETWORK_IPV6_CONFIGURATION", "IPv6 Configuration...");
            // IPV6 Pop-up
            define ("NETWORK_IPV6CONFIGURATION_TITLE", "IPv6 Configuration");
            define ("NETWORK_IPV6CONFIGURATION_GENERAL", "General");
            define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE", "DHCP Mode:");
            define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_INFORMATION", "Information Only");
            define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_NORMAL", "Normal");
            define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_AUTO", "Auto");
            define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_NONE", "None");
            define ("NETWORK_IPV6CONFIGURATION_GENERAL_ADDRESS_PREFIX_LENGTH", "IPv6 Address/Subnet Prefix Length:");
            define ("NETWORK_IPV6CONFIGURATION_GENERAL_DEFAULTGATEWAY", "Default Gateway:");
            define ("NETWORK_IPV6CONFIGURATION_GENERAL_DNS_SERVER1", "DNS Server 1:");
            define ("NETWORK_IPV6CONFIGURATION_GENERAL_DNS_SERVER2", "DNS Server 2:");
            define ("NETWORK_IPV6CONFIGURATION_STATUS", "Status");
            define ("NETWORK_IPV6CONFIGURATION_STATUS_LINK_LOCAL", "Link-local:");
			define ("NETWORK_IPV6CONFIGURATION_STATUS_GLOBAL", "Global:");
			define ("NETWORK_IPV6CONFIGURATION_STATUS_GLOBAL_STATELESS", "Global stateless:");
			define ("NETWORK_IPV6CONFIGURATION_STATUS_GLOBAL_STATELESS_TEMPORARY", "Global stateless temporary:");
            define ("NETWORK_IPV6CONFIGURATION_STATUS_NO_ADDRESSES", "Device currently has no IPv6 addresses assigned to it");
        define("SSL_PROTOCOL_LEGEND", "SSL Protocol");
        define("SSLPROTO_DEFAULT", "Legacy");
        define("SSLPROTO_TLS12", "TLS 1.2 (Only)");
            define ("NETWORK_CERTIFICATES_GROUPTITLE", "Certificates");
            define ("NETWORK_CACERT", "CA Certificate:");
            define ("NETWORK_CLIENTCERT", "Client Certificate:");
            define ("NETWORK_CLIENTPRIVATEKEY", "Client Private Key (and Certificate):");
            define ("NETWORK_CERTIFICATES_SETTINGS_GROUPTITLE", "Settings");
            define ("NETWORK_EAPOLVERSION", "EAPOL Version:");
            define ("NETWORK_EAPIDENTITY", "EAP Identity:");
            define ("NETWORK_PRIVTEKEYPWD", "Private Key Password:");
		
        define ("NETWORK_SETTINGS_ENABLEIEEE8021X", "Enable IEEE 802.1X");
        define ("NETWORK_SETTINGS_NOCERTIFICATE", "No Certificate File Uploaded");
        define ("NETWORK_SETTINGS_NOKEY", "No Private Key Uploaded");
        define ("NETWORK_802LOG_DOWNLOAD", "Download IEEE 802.1X Log...");
        define ("NETWORK_CHECKING_IP_ADDRESS_DUPLICATE", "Проверка уникальности IP-адреса устройства");
        define ("NETWORK_DUPLICATE_IP_ADDRESS", "Конфликт IP-адреса устройства");
        define ("NETWORK_IP_ADDRESS_IN_USE", "Обнаружен конфликт IP-адреса устройства с другой системой в сети.");
		define("DEVICE_TABLABEL", "Устройство");
		define("DEVICE_HEADING", "Настройки устройства");
		define("DEVICE_USERFEEDBACK_GROUPTITLE", "Отзывы");
		define("DEVICE_USERFEEDBACK_VOLUMELABEL", "Громкость динамика:");
		define("DEVICE_USERFEEDBACK_FREQUENCYLABEL", "Частота сигнала (Hz):");
		define("DEVICE_USERFEEDBACK_DURATIONLABEL", "Длительность сигнала:");
		define("DEVICE_USERFEEDBACK_TAMPERVOLUMELABEL", "Громкость сигнала взлома:");
		define("DEVICE_USERFEEDBACK_LEDBRIGHTNESSLABEL", "Яркость LED:");
		define("DEVICE_USERFEEDBACK_LOCATEDEVICE", "Найти устройство...");
		define("DEVICE_TIMESETTINGS_GROUPTITLE", "Настройка времени");
		define("DEVICE_TIMESETTINGS_SERVERLABEL", "Серверное время:");
		define("DEVICE_TIMESETTINGS_NANOTIMELABEL", "Локальное время nano:");
		define("DEVICE_TIMESETTINGS_UPDATETIME", "Начать синхронизацию!");
		define("DEVICE_TIMESETTINGS_SYNCHRONIZETIMELABEL", "Ежедневная синхронизация");
		define("DEVICE_TIMESETTINGS_UPDATELOCALTIME", "Синхронизация с сервером");
		define("DEVICE_ACTIVITIES_GROUPTITLE", "Действия");
		define("DEVICE_ACTIVITIES_FACTORYRESET", "Сброс к заводским настройкам");
		define("DEVICE_ACTIVITIES_REBOOTDEVICE", "Перезагрузить устройство");
        define("DEVICE_EXTERNAL_GROUPTITLE", "HBOX Settings (EyeLock Support Only!)");
        define("DEVICE_EXTERNAL_WELCOMEMESSAGE", "Welcome Message:");
        define("DEVICE_EXTERNAL_LOCATION", "Location:");
        define("DEVICE_EXTERNAL_POSTTITLE", "Post Event URLs");
        define("DEVICE_EXTERNAL_DESTINATIONURL", "Main URL:");
        define("DEVICE_EXTERNAL_IRISURL", "Iris Post Endpoint:");
        define("DEVICE_EXTERNAL_ERRORURL", "Error Endpoint:");
        define("DEVICE_EXTERNAL_HEARTBEATURL", "HeartBeat Endpoint:");
        define("DEVICE_EXTERNAL_MAINTENANCEURL", "Maintenance Endpoint:");
        define("DEVICE_EXTERNAL_POSTSCHEMEURL", "POST Scheme:");
        define("AUTHENTICATION_SETTINGS_IRISCAPTURESETTINGS", "Iris Capture Settings");
        define("AUTHENTICATION_SETTINGS_IRISCAPTURETIMEOUT", "Iris Capture Timeout:");
        define("AUTHENTICATION_SETTINGS_IRISCAPTURERESETDELAY", "Iris Capture Reset Delay:");
        define("AUTHENTICATION_SETTINGS_HTTPPOSTMSGFORMAT", "Http Post Message Format:");
        define("AUTHENTICATION_SETTINGS_IRISIMAGEFORMAT", "Iris Image Format:");
        define("AUTHENTICATION_SETTINGS_IRISIMAGEQUALITY", "Image Quality:");
        define("AUTHENTICATION_SETTINGS_100LOSSLESS", "(100 = Lossless)");
		define("DEVICE_ADVSETTINGS_GROUPTITLE", "Расширенные настройки устройства");
		define("DEVICE_ADVSETTINGS_LISTENINGPORTLABEL", "Порт ожидания:");
		define("DEVICE_ADVSETTINGS_EYEDESTADDR", "IP-адрес назначения изображения:");
		define("DEVICE_ADVSETTINGS_EYEDESTPORT", "Порт назначения изображения:");
		define("SECURITY_TABLABEL", "Безопасность");
		define("SECURITY_HEADING", "Настройки безопасности");
		define("SECURITY_PASSWORD_GROUPTITLE", "Смена пароля");
		define("SECURITY_PASSWORD_OLDPWDLABEL", "Используемый пароль:");
		define("SECURITY_PASSWORD_NEWPWDLABEL", "Новый пароль:");
		define("SECURITY_PASSWORD_CONFIRMPWDLABEL", "Подтвердите пароль:");
		define("SECURITY_PASSWORD_REMOVEPWDLABEL", "Очистить поле пароля");
		define("SECURITY_PASSWORD_RESETPWD", "Обновить пароль");
		define("SECURITY_TAMPER_SETTINGS", "Настройки сигнализации ");
		define("SECURITY_TAMPER_SIGNALHIGH", "Включать сигнализацию при сильном сигнале");
		define("SECURITY_TAMPER_SIGNALLOW", "Включать сигнализацию при слабом сигнале");
		define("SECURITY_TAMPER_NOTIFYADDRESS", "Адрес используемый для отправки уведомлений:");
		define("SECURITY_TAMPER_NOTIFYPORT", "Порт:");
		define("SECURITY_TAMPER_NOTIFYMESSAGE", "Сообщение сигнализации:");
		define("SOFTWARE_TABLABEL", "Программное обеспечение");
		define("SOFTWARE_HEADING", "Детали ПО/прошивки");
		define("SOFTWARE_STATUS_GROUPTITLE", "Версия/Обновление");
		define("SOFTWARE_CHECKUPDATES_LABEL", "Последняя проверка на обновление:");
		define("SOFTWARE_UPDATEDETAILS_TITLE", "Детали обновления ПО");
		define("SOFTWARE_AVAILUPDATE_NANOLABEL", "Новая версия ".$NXTText.":");
		define("SOFTWARE_AVAILUPDATE_BOBLABEL", "Новая версия ICM:");
		define("SOFTWARE_INSTALLEDUPDATES_NANOLABEL", "Установленные обновления ".$NXTText.":");
		define("SOFTWARE_INSTALLEDUPDATES_BOBLABEL", "Установленные обновления ICM:");
		define("SOFTWARE_UPDATE_ALLOWSITEADMIN", "Разрешить администратору обновлять устройство");
		define("SOFTWARE_STATUS_UPDATESTATUS_FAILED", "Ошибка соединения с сервером обновлений");
		define("SOFTWARE_STATUS_UPDATESTATUS_NEWVERSION", "Новая версия ПО доступна для ".$NXTText." !");
		define("SOFTWARE_STATUS_UPDATESTATUS_CURRENT", "На ".$NXTText."  установлена последняя версия ПО");
		define("SOFTWARE_STATUS_UPDATESTATUS_VERCORRUPT", "Версионные файлы могли быть повреждены");
		define("SOFTWARE_STATUS_UPDATESTATUS_CHECKINTERNET", "Проверить соединение с интернетом");
		define("SOFTWARE_STATUS_UPDATENOW", "Обновить сейчас");
		define("SOFTWARE_STATUS_LATER", "Обновить позже");
		define("SOFTWARE_STATUS_MANUALNANO", "Выбрать файл...");
		define("SOFTWARE_STATUS_MANUALBOB", "Выбрать ICM-файл...");
		define("SOFTWARE_STATUS_UPDATEDETAIL", "Детали...");
		define("SOFTWARE_MODE_NANOLABEL", "Точки восстановления ".$NXTText." ");
		define("SOFTWARE_MODE_BOBLABEL", "Точки восстановления ICM");
		define("SOFTWARE_MODE_DELETERESTOREPOINTS", "Удаление точки(ек) восстановления...");
		define("SOFTWARE_MODE_RESTORERESTOREPOINT", "Восстановление предыдущей версии...");
		define("SOFTWARE_RESTOREPOINTS_NONANO", "Нет точек восстановления для ".$NXTText."!");
		define("SOFTWARE_RESTOREPOINTS_NOBOB", "Нет точек восстановления для ICM!");
		define("SOFTWARE_RESTOREHEADER_SELECT", "Выбрать");
		define("SOFTWARE_RESTOREHEADER_RESTOREPOINTS", "Точки восстановления");
		define("SOFTWARE_RESTORE_GROUPTITLE", "Восстановить Прошивку");
		define("SOFTWARE_RESTORE_RESTORENOW", "Восстановить сейчас...");
		define("SOFTWARE_RESTORE_DELETERESTOREPOINTS", "Удалить...");
		define("AUTHENTICATION_TABLABEL", "Аутентификация");
		define("AUTHENTICATION_HEADING", "Настройки аутентификации");
        define("AUTHENTICATION_SETTINGS_IRISPROCESSINGMODE", "Iris Processing Mode:");
        define("AUTHENTICATION_SETTINGS_ACCESSCONTROLMODE", "Access Control Authentication");
        define("AUTHENTICATION_SETTINGS_ACCESSCONTROLAUTHENTICATION", "Access Control Authentication");
        define("AUTHENTICATION_SETTINGS_IRISCAPTUREMODE", "Iris Capture Mode");
        define("AUTHENTICATION_SETTINGS_IRISCAPTURE", "Iris Capture");
        define("DEVICE_EXTERNAL_POSTSCHEME", "Iris Post Scheme:");
        define("AUTHENTICATION_SETTINGS_IRISCAPTURESETTINGS", "Iris Capture Settings");
        define("AUTHENTICATION_SETTINGS_IRISCAPTURETIMEOUT", "Iris Capture Timeout:");
        define("AUTHENTICATION_SETTINGS_IRISCAPTURERESETDELAY", "Iris Capture Reset Delay:");
        define("AUTHENTICATION_SETTINGS_HTTPPOSTMSGFORMAT", "Http Post Message Format:");
        define("AUTHENTICATION_SETTINGS_IRISIMAGEFORMAT", "Iris Image Format:");
        define("AUTHENTICATION_SETTINGS_IRISIMAGEQUALITY", "Image Quality:");
        define("AUTHENTICATION_SETTINGS_100LOSSLESS", "(100 = Lossless)");
		define("AUTHENTICATION_MODE_GROUPTITLE", "Совпадение");
		define("AUTHENTICATION_MODE_SINGLEEYELABEL", "Использовать один глаз");
		define("AUTHENTICATION_MODE_DUALEYELABEL", "Использовать оба глаза");
		define("AUTHENTICATION_SETTINGS_GROUPTITLE", "Настройки");
		define("AUTHENTICATION_SETTINGS_REPEATPERIODLABEL", "Время между попытками авторизации:");
		define("AUTHENTICATION_SETTINGS_NEGMATCHTIMEOUTENABLEDLABEL", "Включить таймауты при несовпадении");
		define("AUTHENTICATION_SETTINGS_LOITERPERIODLABEL", "Период бездействия:");
		define("AUTHENTICATION_SETTINGS_NEGMATCHRESETLABEL", "Таймер сброса при несовпадении:");
		define("AUTHENTICATION_SETTINGS_DESTINATIONADDRESSLABEL", "IP-адрес получателя сетевого сообщения:");
		define("AUTHENTICATION_SETTINGS_DESTINATIONPORTLABEL", "Порт:");
		define("AUTHENTICATION_SETTINGS_SECURENETWORDLABEL", "Использовать защищенное сетевое сообщение");
		define("AUTHENTICATION_SETTINGS_MSGFORMATLABEL", "Формат сетевого сообщения:");
        define ("AUTHENTICATION_SETTINGS_SENDALLIMAGES", "Send All Images");
		define("DATABASE_TABLABEL", "База данных");
		define("DATABASE_HEADING", "Настройки базы данных");
		define("DATABASE_TYPE", "Тип базы данных:");
		define("DATABASE_SQLLITE", "SQLite");
		define("DATABASE_BINARY", "Двоичный файл");
		define("DATABASE_TYPE_GROUPTITLE", "Детали базы данных");
		define("DATABASE_TYPE_LOCALLABEL", "Локальный");
		define("DATABASE_TYPE_NETMATCHLABEL", "Включить Network Matcher");
		define("DATABASE_TYPE_NETMATCHADDRESSLABEL", "Адрес Network Matcher:");
		define("DATABASE_TYPE_NETMATCHPORTLABEL", "Порт ожидания Network Matcher");
		define("DATABASE_STATISTICS_GROUPTITLE", "Статистика базы данных");
		define("DATABASE_STATISTICS_TEMPLATESPACE", "Оставшееся свободное место:");
		define("DATABASE_SECURECOMM_NETMATCHLABEL", "Защищенное соединение с Network Matcher");
		define("ACP_TABLABEL", "СКД");
		define("ACP_HEADING", "Система Контроля Доступа (СКД)");
		define("ACP_PROTOCOL_GROUPTITLE", "Протокол Контроля Доступа");
		define("ACP_PROTOCOL_PROTOCOL", "Протокол:");
		define("ACP_PROTOCOL_DUALAUTHENABLEDLABEL", "Двойная аутентификация");
		define("ACP_PROTOCOL_DUALAUTHPARITY", "Проверка Битов Четности");
		define("ACP_PROTOCOL_DUALAUTHLEDENABLEDLABEL", "СКД контролирует LED-подсветку ");
		define("ACP_PROTOCOL_MATCHWAITIRISTIMELABEL", "Время ожидания радужной оболочки глаза:");
        define ("ACP_PROTOCOL_MATCHWAITPINTIMELABEL", "PIN Wait Time:");
        define ("ACP_PROTOCOL_PINBURSTBITS", "PIN Burst Bits:");
		define("ACP_PROTOCOL_RELAYTIMELABEL", "Время реле разрешения:");
		define("ACP_PROTOCOL_DENYRELAYTIMELABEL", "Время реле отказа:");
        define ("ACP_PROTOCOL_DURESSRELAYTIMELABEL", "Duress Relay Time:");
		define("ACP_PROTOCOL_ENABLERELAYTRIGGERSLABEL", "Включить реле");
		define("ACP_PROTOCOL_NEGMATCHTIMEOUTLABEL", "Таймаут после несовпадения");
		define("ACP_TEST_GROUPTITLE", "Настройки проверки СКД");
		define("ACP_TEST_TESTBUTTON", "Проверить сейчас!");
		define("ACP_TEST_CARDIDLABEL", "Номер тестовой карты");
		define("ACP_TEST_FACILITYCODELABEL", "Facility тестовой карты");
		define("ACP_TEST_SENDINGMESSAGE", "Проверочное сообщение отправляется на панель СКД...");
		define("ACP_NETWORK_SECURECOMMLABEL", "Защищенное соединение");
		define("ACP_TEST_TCPCONNECTIONFAILED", "Ошибка соединения сокета");
		define("ACP_TEST_FAILED", "Не удалось послать проверочную строку на СКД");
		define("ACP_TEST_CONNECTIONFAILED", "Не удалось установить соединение с прошивкой");
		define("LOGS_TABLABEL", "Логи");
		define("LOGS_HEADING", "Логи");
		define("LOGHEADER_STATUS", "Статус");
		define("LOGHEADER_DATE", "Дата/Время");
		define("LOGHEADER_NAME", "Имя");
		define("LOGHEADER_CODE", "Код СКД");
		define("LOGHEADER_MESSAGE", "Сообщение");
		define("LOGS_EVENTLOG_GROUPTITLE", "Лог событий");
		define("LOGS_EVENTLOG_REFRESHBUTTON", "Обновить!");
		define("LOGS_EVENTLOG_DOWNLOAD", "Скачать лог...");
		define("LOG_AUTOREFRESH_LABEL", "Автоматическое обновление:");
		define("DUMP_TABLABEL", "Дамп");
		define("DIALOG_HELPSETTINGS_TITLE", "Настройки помощи");
		define("DIALOG_HELPSETTINGS_ENABLEHELP", "Включить подсказки");
		define("DIALOG_HELPSETTINGS_POPUPTRIGGERMODE", "Показывать подсказку:");
		define("DIALOG_HELPSETTINGS_POPUPHOVER", "Навести мышь");
		define("DIALOG_HELPSETTINGS_POPUPCLICK", "Нажать мышь");
		define("DIALOG_HELPSETTINGS_POPUPDELAY", "Задержка курсора:");
		define("TOOLTIP_HELPSETTINGS_ENABLEHELP", " <span style='font-size: 16px; font-style:bold'>Включить подсказки</span> <ul style='padding-left: 1em'><li>Выберите эту опцию чтобы включить <i>Систему Подсказок</i>.</li> <li>Отмените чтобы полностью отключить <i>Систему Подсказок</i>.</li></ul>");
		define("TOOLTIP_HELPSETTINGS_POPUPMODE", " <span style='font-size: 16px; font-style:bold'>Режим работы Системы помощи</span> <ul style='padding-left: 1em'><li>Выберите метод вызова <i>Системы Подсказок</i> popups.</li></ul>");
		define("TOOLTIP_HELPSETTINGS_POPUPDELAY", " <span style='font-size: 16px; font-style:bold'>Задержка курсора</span> <ul style='padding-left: 1em'><li>Введите время в секундах, которое требуется держать мышь чтобы окно <i>Системы Подсказок</i> появилось на экране.</li> <li>Диапазон (секунды): 0 - 5.</li> <li>По умолчанию: 1.0.</li></ul>");
		define("TAMPER_ICON_ALTTEXT", "Сигнализация взлома!");
		define("TAMPER_TOOLTIP_TEXT", "Обнаружена попытка взлома.");
		define("ATOMIC_TOOLTIP_TEXT", "Приложение Eyelock запущено!");
		define("POWERBUTTON_TOOLTIP_TEXT", "Приложение Eyelock не запущено!");
		define("EYELOCK_APPLICATION_STATUS", "Статус приложения Eyelock");
		define("EYELOCK_MASTER_STATUSTEXT", "Статус основного устройства:");
		define("EYELOCK_SLAVE_STATUSTEXT", "Статус вспомогательного устройства:");
		define("EYELOCK_STATUS_RUNNING", "Выполняется");
		define("EYELOCK_STATUS_NOTRUNNING", "Не Выполняется");
		define("SECONDS_LABEL", "Секунды");
        define ("MILLISECONDS_LABEL", " Milliseconds");
		define("DEFAULT_EMPTY_FIELD", "Необязательный");
		define("REQUIRED_EMPTY_FIELD", "Обязательный");
		define("CHANGE_PASSWORD_OLD", "Используемый пароль");
		define("CHANGE_PASSWORD_NEW", "Новый пароль");
		define("CHANGE_PASSWORD_CONFIRM", "Подтвердить пароль");
		define("MSG_UNAVAILABLE", "Недоступен!");
		define("MSG_USERHELLO", "Здравствуйте");
		define("MSG_UNKNOWNUSER", "Неизвестный пользователь");
		define("MSG_ENABLED", "Включен");
		define("MSG_DISABLED", "Выключен");
		define("MSG_NEVER", "Никогда!");
		define("MSGBOX_INFORMATION_TITLE", "Информация");
		define("MSGBOX_INFORMATION_TITLESHORT", "Информация");
		define("MSGBOX_SUCCESS_TITLE", "Успешно");
		define("MSGBOX_FAILED_TITLE", "Ошибка");
		define("MSGBOX_TAMPERED_TITLE", "Попытка взлома!");
		define("MSGBOX_OKBUTTON", "Да");
		define("MSGBOX_CANCELBUTTON", "Отмена");
		define("MSGBOX_YESBUTTON", "Да");
		define("MSGBOX_NOBUTTON", "Нет");
		define("SAVING_SETTINGS", "Сохранение настроек...");
		define("SAVING_SETTINGSANDRESTART", "Сохранение настроек и перезагрузка...");
		define("SAVING_FEWMOMENTS", "Это может занять несколько секунд...");
		define("RELOADING_PAGE", "Обновление страницы... Подождите...");
		define("REFRESHING_PAGE", "Перезагрузка страницы... Подождите...");
		define("VALIDATION_FAILEDTITLE", "Ошибка валидации!");
		define("VALIDATION_MESSAGE1", "Некоторые поля содержат некорректные данные!");
		define("VALIDATION_MESSAGE2", "Пожалуйста, проверьте корректность данных во всех вкладках!");
		define("CONNECTION_FAILEDTITLE", "Ошибка соединения!");
		define("CONNECTION_MESSAGE1", "WebConfig не может установить соединение с устройством!");
		define("CONNECTION_MESSAGE2", "Проверьте IP-адрес устройства и включено ли оно!");
		define("LOADINGLOG_DETAIL", "Загрузка детализированного лога... Подождите...");
		define("ALERT_IPINUSE", "Указанный статический IP-адрес уже используется! Некоторые настройки не будут сохранены!");
        define ("ALERT_802CONFIG", "Unable to verify IEEE 802.1X configuation information.  Not all settings could be saved! Check to ensure that you have uploaded all of the required Certificates/Keys!");
		define("RESETPASSWORD_MESSAGETITLE", "Сменить пароль");
		define("RESETTINGPASSWORD_MESSAGE", "Сохранение нового пароля. Подождите...");
		define("RESETPASSWORD_SUCCESS", "Новый пароль был успешно сохранен!");
		define("RESETPASSWORD_LOGOUT", "Выйдите из системы, чтобы использовать новый пароль.");
		define("RESETPASSWORD_FAIL", "Ошибка при сохранении нового пароля!");
		define("KEY_MANAGEMENT_GROUPTITLE", "Настройки ключей доступа");
		define("KEY_MANAGEMENT_BUTTON", "Управление ключами доступа");
		define("ADDKEY_DIALOG_TITLE", "Создать новый ключ доступа");
		define("ADDKEY_DIALOG_MESSAGE", "Введите описание ключа доступа:");
		define("ADDKEY_DIALOG_CONTROLS", "  <table style=\width:100%\><tr style=\height:2px\ /> <tr><td>Имя компьютера :</td> <td><input id=\keyHostName\ type=\text\ name=\keyHostName\ style=\width:120px\ onblur=\checkHostName");
		define("ADDING_NEW_KEY", "Создание нового ключа доступа...");
		define("DELETEALLKEY_DIALOG_TITLE", "Удалить все ключи доступа");
		define("DELETEALLKEY_DIALOG_MESSAGE", "Попытка удаления всех ключей доступа данного устройства.");
		define("DELETING_ALL_KEYS", "Удаление всех ключей доступа...");
		define("DELETEKEY_DIALOG_TITLE", "Удалить ключ доступа");
		define("DELETEKEY_DIALOG_MESSAGE", "Попытка удаления ключа доступа данного устройства.");
		define("DELETING_HOST_KEY", "Удаление ключа доступа...");
		define("DOWNLOADING_KEY", "Загрузка ключа доступа...");
		define("REGENERATEKEY_DIALOG_TITLE", "Пересоздать системный ключ доступа ".$NXTText." ");
		define("REGENERATEKEY_DIALOG_MESSAGE", "Попытка пересоздания системного ключа доступа ".$NXTText." . Все ключи доступа, ранее загруженные с данного устройства, станут недействительными.");
		define("REGENERATING_NANO_KEY", "Пересоздание системного ключа доступа ".$NXTText." ");
		define("STARTING_EYELOCK_APPLICATION", "Запуск приложения Eyelock…");
		define("IDENTIFY_DEVICE_TITLE", "Идентификация устройства");
		define("IDENTIFY_DEVICE_MESSAGE", "Периодичное включение LED-подсветки устрайства...");
		define("IDENTIFY_DEVICE_MESSAGE2", "Чтобы прервать выполнение, намите Отмена...");
		define("RESETTING_DEVICE_MESSAGE", "Устройство сбрасывается к заводским настройкам...Подождите...");
		define("REBOOTING_DEVICE_MESSAGE", "Устройство перезагружается. Подождите...");
		define("REBOOTING_DEVICE_MESSAGE2", "Это может занять несколько минут...");
		define("WAITING_FOR_EYELOCK_RESTART", "Перезагрузка приложения Eyelock. Подождите...");
		define("DEVICE_TIME_SYNCHRONIZING", "Синхронизация локального времени устройства. Подождите...");
		define("DEVICE_TIME_SYNCHRONIZED", "Локальное время устройства установлено.");
		define("DEVICE_TIME_SYNCFAILED", "Не удалось установить локальное время устройства.");
		define("FACTORYRESET_DEVICE", "Устройство сбрасывается к заводских настроек");
		define("RESTORE_DEVICE", "Устройство восстанавливается из резервной копии...Устройство восстанавливается из резервной копии...Устройство восстанавливается из резервной копии...Устройство восстанавливается из резервной копии...");
		define("RESTORE_DEVICE_TITLE", "Восстановить прошивку устройства");
		define("RESTORE_DEVICE_DELETETITLE", "Удалить точку восстановления");
		define("RESTORE_DEVICE_DELETEMSG", "Удаление точек восстановления...");
		define("AUTOMATIC_LOGOUT", "Сейчас произойдет выход из системы!");
		define("LOGOUT_MESSAGE", "Выход из системы...Пожалуйста, подождите");
		define("REBOOT__DEVICE_TITLE", "Перезагрузить устройство");
		define("REBOOT_DEVICE_WARNING", "Нажмите ДА для перезагрузки устройства");
		define("MSG_AREYOUSURE", "Продолжить?");
		define("FACTORY_RESET_TITLE", "Подтвердите сброс к заводскиим настройкам");
		define("FACTORY_RESET_WARNING", "Это действие не может быть отменено...");
		define("FIRMWARE_UPDATE_NANOTITLE", "Происходит сброс ".$NXTText." до заводских настроек. Подождите...");
		define("FIRMWARE_UPDATESTATUS_UPLOAD", "Пакет загружается на Устройство");
		define("FIRMWARE_UPDATESTATUS_DOWNLOAD", "Пакет загружается с Сервера:");
		define("FIRMWARE_UPDATESTATUS_UNPACK", "Распаковка файлов...");
		define("FIRMWARE_UPDATESTATUS_VALIDATING", "Проверка образа ".$NXTText."");
		define("FIRMWARE_UPDATESTATUS_COPYING", "Копирование необходимых файлов...");
		define("FIRMWARE_UPDATESTATUS_RESTOREPOINT", "Создание точки восстановления ".$NXTText."...");
		define("FIRMWARE_UPDATESTATUS_UPDATING", "Производится обновление ".$NXTText." ...");
		define("FIRMWARE_UPDATESTATUS_VALIDATINGBOB", "Проверяется образ ICM...");
		define("FIRMWARE_UPDATESTATUS_RESTOREPOINTBOB", "Создание точки восстановления ICM...");
		define("FIRMWARE_UPDATESTATUS_UPDATINGBOB", "Производится обновление ICM");
		define("FIRMWARE_UPDATESTATUS_COMPLETE", "Обновление прошивки успешно завершено!");
		define("FIRMWARE_UPDATESTATUS_RESTORESETTINGS", "Восстановление настроек устройства...");
		define("FIRMWARE_UPDATE_TITLE", "Результаты обновления прошивки");
		define("FIRMWARE_UPDATE_FAILEDTITLE", "Не удалось обновить прошивку");
		define("FIRMWARE_UPDATE_FAILEDMESSAGE", "Ошибка при обновлении прошивки ".$NXTText." ");
		define("FIRMWARE_UPDATE_SUCCESS", "Прошивка успешно обновлена!");
		define("FIRMWARE_UPDATE_RELOAD", "Нажмите ДА для перезагрузки устройства");
		define("FIRMWARE_UPDATEERROR_BADFILETYPE", "Закачанный файл не является прошивкой или повреждён.");
		define("FIRMWARE_UPDATEERROR_UNPACKFAILED", "Не удалось распаковать файл прошивки. Возможно, файл испорчен или на устройстве не достаточно свободного места.");
		define("FIRMWARE_UPDATEERROR_VALIDATEFAILED", "Ошибка проверки исправности пакета. Возможно, содержимое испорчено");
		define("FIRMWARE_UPDATEERROR_RESTOREPOINTFAILED", "Не удалось создать точку восстановления. Возможно, на устройстве не достаточно свободного места");
		define("FIRMWARE_UPDATEERROR_INSTALLFAILED", "Не удалось полностью извлечь прошивку на устройстве. Возможно, на устройстве не достаточно свободного места");
		define("FIRMWARE_UPDATEERROR_BOBINSTALLFAILED", "Не удалось обновить прошивку контроллера!");
		define("FIRMWARE_UPDATEERROR_DEVICERESTOREFAILED", "Не удалось восстановить настройки устройства!");
		define("FIRMWARE_UPDATEERROR_SLAVECOPYFAILED", "Не удалось скопировать файлы на подчиненное устройство! Возможно, на устройстве не достаточно свободного места.Не удалось скопировать файлы на подчиненное устройство! Возможно, на устройстве не достаточно свободного места.");
		define("FIRMWARE_UPDATEERROR_SLAVEINSTALLFAILED", "Не удалось обновить подчинённое устройство");
		define("FIRMWARE_UPDATEERROR_UNKNOWNFAILED", "Неизвестная ошибка при обновлении устройства!");
		define("DATABASE_DETAILSUNAVAILABLE", "Детали недоступны!");
		define("NANO_DEVICE_STATUSTITLE", "Состояние устройства ".$NXTText." ");
		define("NANO_DEVICE_CONNDOWN", "Ошибка соединения с устройством!");
		define("NANO_DEVICE_RECONNECT", "Пожалуйста, проверьте устройство. Затем нажмите ДА, чтобы проверить статус.");
		define("TOOLTIP_LOGIN_installer", " У типа пользователя <b><i>installer</i></b> есть все редакторские права.");
		define("TOOLTIP_LOGIN_SITEADMIN", "У типа пользователя  <b><i>admin</i></b> есть полные права на чтение и ограниченные права на запись.");
		define("TOOLTIP_LOGIN_CHANGEPASSWORD", "Выберите тип пользователя, для которого вы хотите поменять пароль.");
		define("TOOLTIP_HOME_TAB", " <b><i>Домашняя Страница</i></b> <br>Показывает основные детали ".$NXTText.": Информация об устройстве, информация о программном обеспечении, Статистика хранения в базе данных");
		define("TOOLTIP_NETWORK_TAB", " <b><i>Сеть</i></b><br>Конфигурация сетевых параметров (параметры DHCP, адрес IP, имя хоста и т.д.)");
		define("TOOLTIP_NETWORK_NAME", " <span style='font-size: 16px; font-style:bold'>Имя Устройства</span>                                             <ul style='padding-left: 1em'><li>The <b><i>Name</i></b> Поле показывает имя хоста ".$NXTText." при работе DHCP или статичном IP в сети.</li>                                             <li>Допустимое <i>Имя Устройства</i> должно содержать только буквы и цифры и должно быть меньше 64 символов.</li></ul>");
		define("TOOLTIP_NETWORK_DHCP", " <span style='font-size: 16px; font-style:bold'>Получить адрес IP автоматически (DHCP)</span>                                             <ul style='padding-left: 1em'><li>При этом выборе ".$NXTText." становится доступным в сети по <i>Имени Устройства</i> через DHCP.</li>                                             <li>При использовании DHCP, ".$NXTText." доступен по <i>Имени Устройства</i> вместо адреса IP в сети.</li>                                             <li>Некоторые сети сконфигурированы для использования DHCP. Если нет, Web Config может быть найден по статистическому адресу IP.</li></ul>");
		define("TOOLTIP_NETWORK_ADVDHCPBUTTON", " <span style='font-size: 16px; font-style:bold'>Кнопка настроек DHCP</span>                                             <ul style='padding-left: 1em'><li>Нажмите для доступа к расширенным настройкам DHCP.</li></ul>");
		define("TOOLTIP_NETWORK_STATICIP", " <span style='font-size: 16px; font-style:bold'>Использовать следующий статичный адрес IP</span> <ul style='padding-left: 1em'><li>При этом выборе ".$NXTText." становится доступным в сети по выбранному статичному адресу IP.</li> <li>Выбранный адрес не должен использоваться другими устройствами в сети.</li> <li>Если адрес уже используется, ".$NXTText." не будет доступно в сети, и  <i>Имя Устройства</i> не появится.</li></ul>");
		define("TOOLTIP_NETWORK_DEVICEIP", " <span style='font-size: 16px; font-style:bold'>Адрес IP Устройства</span> <ul style='padding-left: 1em'><li>Введите статичный адрес IP.</li> <li>Если необходимо, проконсультируйтесь с Администратором Сети для выбора адреса IP.</li> <li>Эта настройка будет использованa только если  настройка <i>Использовать следующий статичный адрес IP</i> выбранa выше.</li></ul>");
		define("TOOLTIP_NETWORK_BROADCASTNETWORK", " <span style='font-size: 16px; font-style:bold'>Широковещательная сеть</span> <ul style='padding-left: 1em'> <li>Если необходимо, проконсультируйтесь с Администратором Сети для определения правильного значения.</li> <li>Эта настройка работает, только если  настройка <i>Используйте следующий статичный адрес IP</i> выбранa выше.</li></ul>");
		define("TOOLTIP_NETWORK_SUBNETMASK", " <span style='font-size: 16px; font-style:bold'>Маска подсети</span> <ul style='padding-left: 1em'> <li>Если необходимо, проконсультируйтесь с Администратором Сети для определения правильного значения.</li> <li>Эта настройка работает, только если настройка <i>Используйте следующий статичный адрес IP.</i> выбрана выше.</li></ul>");
		define("TOOLTIP_NETWORK_DEFAULTGATEWAY", " <span style='font-size: 16px; font-style:bold'>Основной Шлюз</span> <ul style='padding-left: 1em'> <li>Если необходимо, проконсультируйтесь с Администратором Сети для определения правильного значения.</li> <li>Эта настройка работает, только если настройка <i>Используйте следующий статичный адрес IP</i> выбрана выше.</li></ul>");
		define("TOOLTIP_NETWORK_DNS", " <span style='font-size: 16px; font-style:bold'>Сервер DNS</span> <ul style='padding-left: 1em'> <li>Если необходимо, проконсультируйтесь с Администратором Сети для определения правильного значения.</li> <li>Эта настройка работает, только если настройка <i>Используйте следующий статичный адрес IP</i> выбрана выше.</li></ul>");
        define ("TOOLTIP_NETWORK_ENABLEIEEE8021X", "<span style='font-size: 16px; font-style:bold'>Enable IEEE 802.1X Security</span>
                                            <ul style='padding-left: 1em'>
                                            <li>Consult with the local Network Administrator if necessary to determine the proper value.</li>
                                            <li>This setting enables the IEEE 802.1X network protocol on the ".$NXTText."</li></ul>");
        define ("TOOLTIP_NETWORK_CACERT", "<span style='font-size: 16px; font-style:bold'>CA Certificate</span>
                                            <ul style='padding-left: 1em'>
                                            <li>Use the Upload button to the right to browse and upload the CA Certificate file to the device.</li></ul>");
        define ("TOOLTIP_NETWORK_UPLOADCACERTIFICATE", "<span style='font-size: 16px; font-style:bold'>Upload CA Certificate</span>
                                            <ul style='padding-left: 1em'>
                                            <li>Use this button to browse and upload the CA Certificate file to the device.</li></ul>");
        define ("TOOLTIP_NETWORK_CLIENTCERT",  "<span style='font-size: 16px; font-style:bold'>Client Certificate</span>
                                            <ul style='padding-left: 1em'>
                                            <li>Use the Upload button to the right to browse and upload the Client Certificate file to the device.</li></ul>");
        define ("TOOLTIP_NETWORK_UPLOADCLIENTCERTIFICATE", "<span style='font-size: 16px; font-style:bold'>Upload Client Certificate</span>
                                            <ul style='padding-left: 1em'>
                                            <li>Use this button to browse and upload the Client Certificate file to the device.</li></ul>"); 
        define ("TOOLTIP_NETWORK_CLIENTPRIVATEKEY",  "<span style='font-size: 16px; font-style:bold'>Client Private Key</span>
                                            <ul style='padding-left: 1em'>
                                            <li>Use the Upload button to the right to browse and upload the Client Private Key file to the device.</li>
                                            <li>It is also acceptable to upload a combined client key and password (PEM) file.</li></ul>");
        define ("TOOLTIP_NETWORK_UPLOADPRIVATEKEY", "<span style='font-size: 16px; font-style:bold'>Upload Client Certificate</span>
                                            <ul style='padding-left: 1em'>
                                            <li>Use this button to browse and upload the Client Private Key file to the device.</li>
                                            <li>It is also acceptable to upload a combined client key and password (PEM) file.</li></ul>"); 
        define ("TOOLTIP_NETWORK_EAPOLVERSION", "<span style='font-size: 16px; font-style:bold'>EAPOL Version</span>
                                            <ul style='padding-left: 1em'>
                                            <li>Use the dropdown to specify the version of the EAPOL protocol to use.</li></ul>"); 
        define ("TOOLTIP_NETWORK_EAPIDENTITY", "<span style='font-size: 16px; font-style:bold'>EAP Identity</span>
                                            <ul style='padding-left: 1em'>
                                            <li>Specify your EAP Indentity.</li></ul>"); 
        define ("TOOLTIP_NETWORK_PRIVTEKEYPWD",  "<span style='font-size: 16px; font-style:bold'>Private Key Pasword</span>
                                            <ul style='padding-left: 1em'>
                                            <li>Specify your Private Key Password.</li></ul>"); 
        define ("TOOLTIP_NETWORK_DOWNLOADLOGBUTTON", "<span style='font-size: 16px; font-style:bold'>Download 802.1X Log Button</span>
                                                <ul style='padding-left: 1em'><li>Click to download the ".$NXTText." <i>IEEE 802.1X Log</i> as a text file.</li></ul>");
		define("TOOLTIP_ADVDHCP_TIMEOUT", " <span style='font-size: 16px; font-style:bold'>Таймаут DHCP</span> <ul style='padding-left: 1em'><li>Устанавливает, сколько секунд клиент DCHP устройства ".$NXTText." будет пытаться разрешить  <i>имя хоста</i> прежде чем сдаться.</li> <li>Диапазон (в секундах): 10 - 120.</li> <li>По умолчанию: 10.</li></ul>");
		define("TOOLTIP_ADVDHCP_RETRIES", " <span style='font-size: 16px; font-style:bold'>Количество попыток для DCHP.</span> <ul style='padding-left: 1em'><li>Устанавливает общее количество попыток разрешения  <i>имени хоста</i> прежде чем сдаться.</li> <li>Диапазон (в секундах): 0 - 10.</li> <li>По умолчанию: 0.</li></ul>");
        define("TOOLTIP_ADVDHCP_RETRYDELAY", " <span style='font-size: 16px; font-style:bold'>Задержка перед повторами</span> <ul style='padding-left: 1em'><li>Устанавливает задержку в секундах между попытками клиента DHCP.</li> <li>Диапазон (в секундах): 0 - 60.</li> <li>Значение по умолчанию: 10.</li></ul>");
        define("TOOLTIP_IPV6_ENABLE", "<span class='tooltip-header'>Enable IPv6</span>");
        define("TOOLTIP_IPV6_CONFIGURATION", "<span class='tooltip-header'>Specify IPv6 Configuration Details</span>");
        define ("TOOLTIP_IPV6_DHCP_MODE", "<span class='tooltip-header'>IPv6 DHCP Mode:</span>
                                            <ul class='tooltip-list'>
                                            <li><b>Stateless</b> – DHCPv6 is used to set DNS servers and NTP servers etc, but not to set IPv6 addresses</li>
                                            <li><b>Stateful</b> - DHCPv6 is used to set IPv6 addresses as well as DNS servers etc.</li>
                                            <li><b>Auto</b> – The use of DHCPv6 is determined by the router advertisements</li>
                                            <li><b>Off</b> - DHCPv6 is disabled.</li>
                                            </ul>");
        define("TOOLTIP_IPV6_ADDRESS", "<span class='tooltip-header'>IPv6 Network Address</span>");
        define("TOOLTIP_IPV6_GATEWAY", "<span class='tooltip-header'>IPv6 Default Gateway Address</span>");
        define("TOOLTIP_IPV6_DNS1", "<span class='tooltip-header'>IPv6 Pirmary DNS Server Address</span>");
        define("TOOLTIP_IPV6_DNS2", "<span class='tooltip-header'>IPv6 Secondary DNS Server Address</span>");
		define ("TOOLTIP_IPV6_STATUS", "<span class='tooltip-header'>Current IPv6 addresses assigned to the EXT device</span>
						<ul class='tooltip-list'>
						<li><b>Global</b> – addresses that are globally valid.</li>
						<li><b>Link-local</b> - addresses that are valid only for communications within the network segment or the broadcast domain that the EXT device is connected to.</li>
<li><b>Stateless</b> – addresses installed due to stateless address configuration.</li>
						<li><b>Temporary</b> - temporary random addresses (documented in RFC 4941).</li>
						</ul>");
		define("TOOLTIP_DEVICE_TAB", " <b><i>Device Tab</i></b><br>Конфигурация устройства  ".$NXTText." . (Яркость светодиода, управление громкостью, настройки времени, перезагрузка и сброс до заводских настроек)");
		define("TOOLTIP_DEVICE_SPEAKERVOLUME", " <span style='font-size: 16px; font-style:bold'>Громкость динамика</span> <ul style='padding-left: 1em'><li>Установите громкость динамика ".$NXTText.".</li> <li>Для отключения звука установите значение 0.</li> <li>Диапазон значений: 0 - 100.</li> <li>По умолчанию: 40.</li></ul>");
		define("TOOLTIP_DEVICE_LEDBRIGHTNESS", " <span style='font-size: 16px; font-style:bold'>Яркость подсветки светодиода</span> <ul style='padding-left: 1em'><li>Установите уровень яркости подсветки светодиода устройства ".$NXTText."</li> <li>Для отключения подсветки установите значение '0'.</li> <li>Диапазон яркости: 0 - 100.</li> <li>По умолчанию: 20.</li></ul>");
		define("TOOLTIP_DEVICE_TAMPERVOLUME", " <span style='font-size: 16px; font-style:bold'>Громкость сигнала при попытке взлома</span> <ul style='padding-left: 1em'><li>Установите громкость сигнала при <i>Попытке Взлома</i>.</li> <li>Для отключения сигнала установите значение '0'.</li> <li>Диапазон громкости: 0 - 100.</li> <li>По умолчанию: 10.</li></ul>");
		define("TOOLTIP_DEVICE_TIMESERVER", " <span style='font-size: 16px; font-style:bold'>Сетевой Сервер Времени</span> <ul style='padding-left: 1em'><li>The Адрес <i>Сетевого Сервера Времени</i> с которого получается текущее время.</li> <li>Замечание: ".$NXTText." ");
		define("TOOLTIP_DEVICE_SYNCHRONIZEDAILY", " <span style='font-size: 16px; font-style:bold'>Синхронизировать время ежедневно</span> <ul style='padding-left: 1em'><li>Эта настройка разрешает синхронизацию времени с указанным <i>Сервером Времени</i>  один раз в день.</li> <li>Значение по умолчанию: Включено.</li></ul>");
		define("TOOLTIP_DEVICE_LOCATEDEVICE", " <span style='font-size: 16px; font-style:bold'>Кнопка \"Узнать местоположение устройства\"</span> <ul style='padding-left: 1em'><li>Выберите чтобы подсветить светодиод ".$NXTText.".</li> <li>Светодиод будет работать, пока пользователь не отменит операцию.</li></ul>");
		define("TOOLTIP_DEVICE_SYNCHRONIZENOW", " <span style='font-size: 16px; font-style:bold'>Кнопка \"Синхронизировать сейчас!\". </span> <ul style='padding-left: 1em'><li>Нажать для немедленной синхронизации времени ".$NXTText." c выбранным <i>Сервером Времени</i>.</li></ul>");
		define("TOOLTIP_DEVICE_SYNCHRONIZEWITHHOST", " <span style='font-size: 16px; font-style:bold'>Кнопка \"Синхронизация с Хостом\"</span> <ul style='padding-left: 1em'><li>Нажать для немедленной синхронизации времени ".$NXTText." с текущем временем хоста.</li></ul>");
		define("TOOLTIP_DEVICE_FACTORYRESET", " <span style='font-size: 16px; font-style:bold'>Кнопка \"Сброс к заводским настройкам\"</span> <ul style='padding-left: 1em'><li>Нажать для сброса ".$NXTText." к  <i>заводским</i> настройкам.</li> <li>Когда настройки ".$NXTText." сброшены к заводским:</li><ol> <li>Все параметры возвращены к значениям по умолчанию.</li> <li>Сетевые настройки возвращены к значениям по умолчанию (DHCP).</li> <li>База данных на устройстве ".$NXTText." очищена</li> <li>Логи, хранящиеся на ".$NXTText." очищены.</li> <li>".$NXTText." перезагружается..</li> </ol></ul>");
		define("TOOLTIP_DEVICE_REBOOTDEVICE", " <span style='font-size: 16px; font-style:bold'>Кнопка перезагрузки</span> <ul style='padding-left: 1em'><li>Нажать для мягкой перезагрузки ".$NXTText.".</li> <li>Rebooting will cause WebConfig to disconnect from the ".$NXTText.".</li> <li>Для переподключения WebConfig необходимо подождать 2 минуты.</li> <li>Если WebConfig не подключится автоматически по истечении 2 минут, убедитесь, что ".$NXTText." работает, и обновите страницу браузера.</li></ul>");
		define("TOOLTIP_SECURITY_TAB", " <b><i>Безопасность</i></b><br>Конфигурация безопасности ".$NXTText.". (Управление ключом доступа, сброс пароля и настроек защиты от взлома.)");
		define("TOOLTIP_SECURITY_TAMPERACTIVEHIGH", " <span style='font-size: 16px; font-style:bold'>Активизировать тревогу попытки взлома на высоком сигнале.</span>                                                     <ul style='padding-left: 1em'><li>Если к системе подключен считыватель карт для двойной аутентификации, эта настройка должна совпадать с настройкой выходного сигнала для считывателя. </li>               <li>Следует выбрать \"Активизировать тревогу попытки на высоком сигнале\", если условие попытки взлома для считывателя срабатывает на высоком сигнале.</li></ul>");
		define("TOOLTIP_SECURITY_TAMPERACTIVELOW", " <span style='font-size: 16px; font-style:bold'>Активизировать тревогу попытки взлома на низком сигнале.</span>                                                		   <ul style='padding-left: 1em'><li>Если к системе подключен считыватель карт для двойной аутентификации, эта настройка должна совпадать с настройкой выходного сигнала для считывателя. </li>								   	   <li>Следует выбрать Активизировать тревогу попытки на низком сигнале, если условие попытки взлома для считывателя срабатывает на низком  сигнале.</li></ul>");
		define("TOOLTIP_SECURITY_TAMPERNOTIFYADDR", " <span style='font-size: 16px; font-style:bold'>Адрес Уведомлений</span> <ul style='padding-left: 1em'><li>Укажите адрес IP системы или приложения, которое будет получать <i>Сообщение попытки взлома</i></li></ul>");
		define("TOOLTIP_SECURITY_TAMPERNOTIFYPORT", " <span style='font-size: 16px; font-style:bold'>Порт Уведомлений</span> <ul style='padding-left: 1em'><li>Укажите порт IP системы или приложения, которое будет получать <i>Сообщение попытки взлома</i></li></ul>");
		define("TOOLTIP_SECURITY_TAMPERMESSAGE", " <span style='font-size: 16px; font-style:bold'>Сообщение о попытке взлома</span> <ul style='padding-left: 1em'><li>Укажите текст сообщения, которое будет отправлено по указанному выше адресу, если произойдет попытка взлома ".$NXTText.".</li></ul>");
		define("TOOLTIP_SECURITY_OLDPWD", " <span style='font-size: 16px; font-style:bold'>Старый Пароль</span> <ul style='padding-left: 1em'><li>Введите существующий пароль логина в ".$NXTText.".</li></ul>");
		define("TOOLTIP_SECURITY_NEWPWD", " <span style='font-size: 16px; font-style:bold'>Новый Пароль</span> <ul style='padding-left: 1em'><li>Введите желаемый пароль логина в ".$NXTText.".</li></ul>");
		define("TOOLTIP_SECURITY_CONFIRMPWD", " <span style='font-size: 16px; font-style:bold'>Подтверждение Пароля</span> <ul style='padding-left: 1em'><li>Повторно введите желаемый пароль логина в ".$NXTText.".</li></ul>");
		define("TOOLTIP_SECURITY_UPDATEPWDBUTTON", " <span style='font-size: 16px; font-style:bold'>Кнопка \"Изменить пароль\".</span> <ul style='padding-left: 1em'><li>Нажмите чтобы активизировать новый пароль.</li> <li>После изменения пароля Web Config будет перенаправлен на окно логина. Введите новый пароль для логина.</li>                                               </ul>");
		define("TOOLTIP_SECURITY_NANONXT_TEXT", "Устройство ".$NXTText."");
		define("TOOLTIP_SECURITY_PC_TEXT", "Главная СистемаГлавная Система");
		define("TOOLTIP_SECURITY_VALIDKEY_TEXT", "Правильный Ключ");
		define("TOOLTIP_SECURITY_INVALIDKEY_TEXT", "Неправильный ключ/ключ с истекшим сроком действия");
		define("TOOLTIP_SECURITY_REGENERATEKEY_TEXT", "Пересоздать ключ");
		define("TOOLTIP_SECURITY_DELETEKEY_TEXT", "Удалить ключ");
		define("TOOLTIP_SECURITY_DOWNLOADKEY_TEXT", "Скачать ключ");
		define("TOOLTIP_SECURITY_KEYMGMTBUTTON", " <span style='font-size: 16px; font-style:bold'>Кнопка Управления Ключом Доступа.</span> <ul style='padding-left: 1em'><li>Нажмите для настройки управления ключом доступа.</li></ul>");
		define("TOOLTIP_SECURITY_ADDHOSTKEYBUTTON", " <span style='font-size: 16px; font-style:bold'>Кнопка добавления хост-ключа</span> <ul style='padding-left: 1em'><li>Нажмите для добавления нового хост-ключа к ".$NXTText.".</li></ul>");
		define("TOOLTIP_SECURITY_DELETEALLHOSTKEYSBUTTON", " <span style='font-size: 16px; font-style:bold'>Кнопка \"Удалить хост-ключ\"</span> <ul style='padding-left: 1em'><li>Нажмите для удаления хост-ключа в ".$NXTText.".</li></ul>");
		define("TOOLTIP_SOFTWARE_TAB", " <b><i>Приложения</i></b><br>Настроить прошивку ".$NXTText.". (Обновить прошивку, Восстановить прошивку)");
		define("TOOLTIP_SOFTWARE_UPDATENOWBUTTON", " <span style='font-size: 16px; font-style:bold'>Кнопка \"Обновить Сейчас!\"</span><ul style='padding-left: 1em'><li>Эта кнопка появляется, если на сайте Eyelock.com доступна новая прошивка ".$NXTText.". </li> <li>Нажмите для начала автоматического обновления прошивки.</li></ul> <span style='font-size: 16px; font-style:bold'>Кнопка \"Обновить Сейчас!\"</span> <ul style='padding-left: 1em'><li>Эта кнопка появляется, если на сайте Eyelock.com доступна новая прошивка ".$NXTText.". </li>  <li>Нажмите для начала автоматического обновления прошивки.</li></ul>");
		define("TOOLTIP_SOFTWARE_LOCALFILEBUTTON", " <span style='font-size: 16px; font-style:bold'>Кнопка \"Локальная прошивка...\"</span> <ul style='padding-left: 1em'><li>Нажмите эту кнопку для обновления прошивки ".$NXTText." из локального файла EyeLock Upgrade Package</li> <li>После нажатия выберите файл для начала обновления.</li>                                                 <li><strong>ВНИМАНИЕ: Не отключайте питание от ".$NXTText." или сеть от Рабочей Станции во время обновления.</strong></li></ul>");
		define("TOOLTIP_SOFTWARE_DETAILSBUTTON", " <span style='font-size: 16px; font-style:bold'>Кнопка Детали...</span> <ul style='padding-left: 1em'><li>Нажатие на кнопку выведет на экран диалог обновления ".$NXTText.", содержащий детали обновления.</li> <li>Начните обновление в диалоге деталей.</li></ul>");
		define("TOOLTIP_SOFTWARE_ALLOWSITEADMINUPDATE", " <span style='font-size: 16px; font-style:bold'>Разрешить admin выполнять обновление</span> <ul style='padding-left: 1em'><li>Включите эту настройку, чтобы разрешить пользователю, зашедшему как");
		define("TOOLTIP_SOFTWARE_NANORESTORE", " <span style='font-size: 16px; font-style:bold'>Точки Восстановления ".$NXTText."</span> <ul style='padding-left: 1em'><li>Выберите эту настройку, чтобы просмотреть список доступных Точек Восстановления ".$NXTText."</li> <li>Точки восстановления можно использовать или удалить, выбрав чекбокс рядом с Точкой Восстановления и нажав кнопку <i>Восстановить...</i> или <i>Удалить...</i> </li></ul>");
		define("TOOLTIP_SOFTWARE_BOBRESTORE", " <span style='font-size: 16px; font-style:bold'>Точки Восстановления Break Out Board</span> <ul style='padding-left: 1em'><li>Выберите эту настройку, чтобы просмотреть список доступных Точек Восстановления Break out Board</li> <li>Точки восстановления можно использовать или удалить, выбрав чекбокс рядом с Точкой Восстановления и нажав кнопку <i>Восстановить...</i> или <i>Удалить...</i> </li></ul>");
		define("TOOLTIP_SOFTWARE_RESTOREBUTTON", " <span style='font-size: 16px; font-style:bold'>Кнопка отката к Точкам Восстановления</span> <ul style='padding-left: 1em'><li>Нажать до отката прошивки ".$NXTText." до предыдущей Точки Восстановления.</li> <li>Только первая выбранная Точка будет восстановлена.</li> <li>Восстановление - автоматизированный процесс и его невозможно остановить если начат.</li></ul>");
		define("TOOLTIP_SOFTWARE_DELETEBUTTON", " <span style='font-size: 16px; font-style:bold'>Кнопка удаления Точек Восстановления</span> <ul style='padding-left: 1em'><li>Нажатие на эту кнопку навсегда удалит все 'выбранные' Точки Восстановления в ".$NXTText.".</li> <li>Удаленные точки нельзя восстановить.</li></ul>");
		define("TOOLTIP_AUTHENTICATION_TAB", " <b><i>Идентификация </i></b><br>Конфигурация параметров идентификации ".$NXTText.". (Способ Сравнения, Период Повтора Авторизации, Пауза  при Негативном Результате Сравнения и т.д.)");
		define("TOOLTIP_AUTHENTICATION_SINGLEEYE", " <span style='font-size: 16px; font-style:bold'>Идентификация по одному глазу</span> <ul style='padding-left: 1em'><li>Выберите эту настройку, чтобы <i>разрешить</i> успешную идентификацию по биометрическому совпадению только одного глаза..</li></ul>");
		define("TOOLTIP_AUTHENTICATION_DUALEYE", " <span style='font-size: 16px; font-style:bold'>Идентификация по двум глазам</span> <ul style='padding-left: 1em'><li>Выберите эту настройку, чтобы <i>требовать</i> успешную идентификацию только  в случае биометрического совпадения обоих глаз.</li></ul>");
		define("TOOLTIP_AUTHENTICATION_REPEATPERIOD", " <span style='font-size: 16px; font-style:bold'>Период Повтора Авторизации</span> <ul style='padding-left: 1em'><li>The <i>минимальное</i> время, которое должно пройти между успешными идентификациями. Для поддержания качества изображения может потребоваться как минимум 2 секунды.</li> <li>Диапазон значений(в секундах): 2-60.</li> <li>По умолчанию: 4.</li></ul>");
		define("TOOLTIP_AUTHENTICATION_ENABLENEGMATCHTIMEOUT", " <span style='font-size: 16px; font-style:bold'>Включить таймаут при отрицательном результате сравнения</span> <ul style='padding-left: 1em'><li>Выберите эту настройку чтобы задействовать таймауты при <i>отрицательном результате сравнения</i>.</li></ul>");
		define("TOOLTIP_AUTHENTICATION_LOITERPERIOD", " <span style='font-size: 16px; font-style:bold'>Период Бездействия</span> <ul style='padding-left: 1em'><li>Определяет <i>максимальное</i> время в секундах, в течение которого ".$NXTText." будет пытаться идентифицировать прежде чем сообщить об отрицательном результате сравнения..</li> <li>Диапазон (в секундах): 2-60.</li> <li>Значение по умолчанию: 6.</li></ul>");
		define("TOOLTIP_AUTHENTICATION_NEGMATCHRESET", " <span style='font-size: 16px; font-style:bold'>Таймер сброса Отрицательного результата сравнения.</span> <ul style='padding-left: 1em'><li>Определяет <i>минимальное</i> время в секундах, которое должно пройти после отрицательного результата перед тем, как ".$NXTText." сделает следующую попытку идентификации..</li> <li>Диапазон в секундах: 2-60.</li> <li>Значение по умолчанию: 4.</li></ul>");
		define("TOOLTIP_AUTHENTICATION_NETWORKMSGADDR", " <span style='font-size: 16px; font-style:bold'>IP Адрес уведомлений</span> <ul style='padding-left: 1em'><li>Укажите адрес IP системы или приложения, которое будет получать <i>Network Match</i> уведомления.</li></ul>");
		define("TOOLTIP_AUTHENTICATION_NETWORKMSGPORT", " <span style='font-size: 16px; font-style:bold'>Порт уведомлений</span> <ul style='padding-left: 1em'><li>Укажите порт системы или приложения, которое будет получать <i>Network Match</i> уведомления.</li></ul>");
		define("TOOLTIP_AUTHENTICATION_NETWORKMSGMSG", " <span style='font-size: 16px; font-style:bold'>Формат Сетевых Сообщений</span> <ul style='padding-left: 1em'><li>Укажите формат строки сообщения, которое будет отсылаться по указанному выше адресу при успешном сравнении/идентификации.</li> <li>Значение по умолчанию: Matched:%d;Score:%0.4f;Time:%llu;ID:</li></ul>");
        define ("TOOLTIP_AUTHENTICATION_IRISPROCESSINGMODE", "<span style='font-size: 16px; font-style:bold'>Iris Processing Mode</span>
                                            <ul style='padding-left: 1em'><li>Specify the mode for processing irises.  There are two options:</li>
                                            <li>Access Control Authentication - Uses matching feature to authenticate user irises and, optionally, interface with ACS systems.</li>
                                            <li>Iris Capture Mode - Capture irises and send them to a remote HTTP Endpoint.  No matching or ACS integration occurs.</li></ul>");
        define ("TOOLTIP_AUTHENTICATION_CAPTURETIMEOUT", "<span style='font-size: 16px; font-style:bold'>Iris Capture Timeout</span>
                                            <ul style='padding-left: 1em'><li>Specify the duration in milliseconds for processing user irises until sending the message to the HTTP endpoint.</li>
                                            <li>Range (Milliseconds): 1000-60000</li>
                                            <li>Default value: 5000</li></ul>");
        define ("TOOLTIP_AUTHENTICATION_CAPTURERESETDELAY", "<span style='font-size: 16px; font-style:bold'>Iris Capture Reset Delay</span>
                                            <ul style='padding-left: 1em'><li>Specify the delay in milliseconds that the device will wait after capturing irises before processing the next user.</li>
                                            <li>Range (Milliseconds): 1000-300000</li>
                                            <li>Default value: 5000</li></ul>");
        define ("TOOLTIP_AUTHENTICATION_MESSAGEFORMAT", "<span style='font-size: 16px; font-style:bold'>HTTP Message Format</span>
                                            <ul style='padding-left: 1em'><li>Specify the format of the message which will be sent to the HTTP Endpoint.</li>
                                            <li>Default value: SOAP (see documentation for schema of SOAP message)</li>
                                            <li>Alertnate value: JSON (see documentation for schema of JSON message)</li></ul>");
        define ("TOOLTIP_AUTHENTICATION_PAYLOADFORMAT", "<span style='font-size: 16px; font-style:bold'>Message Image Payload Format</span>
                                            <ul style='padding-left: 1em'><li>Specify the graphical format of the iris images which will be sent to the HTTP Endpoint.</li>
                                            <li>Option 1: RAW - 8-bit, Single Channel image</li>
                                            <li>Option 2: PNG</li>
                                            <li>Option 2: J2K (JPEG 2000)</li>
                                            <li>Default value: RAW - 8-bit, Single Channel image</li></ul>");
        define ("TOOLTIP_AUTHENTICATION_IRISIMAGEQUALITY", "<span style='font-size: 16px; font-style:bold'>Iris Image Quality</span>
                                            <ul style='padding-left: 1em'><li>Specify the quality of the encoded JPEG 2000 iris image.  A value of 100 uses lossless compression.</li>
                                            <li>Default value: 100</li></ul>");
        define ("TOOLTIP_IRISCAPTURE_WECOMEMSG", "<span style='font-size: 16px; font-style:bold'>Welcome Message</span>
                                            <ul style='padding-left: 1em'><li>Define the text string sent as part of the Iris Capture message.</li></ul>");
        define ("TOOLTIP_IRISCAPTURE_DEVICELOCATION", "<span style='font-size: 16px; font-style:bold'>Device Location</span>
                                            <ul style='padding-left: 1em'><li>Define a unique string to associate with this device in the Iris Capture message.</li></ul>");
        define ("TOOLTIP_IRISCAPTURE_HEARTBEATURL", "<span style='font-size: 16px; font-style:bold'>Heartbeat URL</span>
                                            <ul style='padding-left: 1em'><li>Define the URL to which this device will send heartbeat messages.    Leave empty to not send heartbeatmessages.</li></ul>");
        define ("TOOLTIP_IRISCAPTURE_POSTSENDERURL", "<span style='font-size: 16px; font-style:bold'>Post Destination Address</span>
                                            <ul style='padding-left: 1em'><li>The base URL to which all Iris Capture messages will be sent..</li></ul>");
        define ("TOOLTIP_IRISCAPTURE_ERRORURL", "<span style='font-size: 16px; font-style:bold'>Signal Error Endpoint</span>
                                            <ul style='padding-left: 1em'><li>The endpiont under the Post Destination Address to which signal error messages will be sent.  Leave empty to not send messages.</li></ul>");
        define ("TOOLTIP_IRISCAPTURE_POSTIRISURL", "<span style='font-size: 16px; font-style:bold'>Post Iris Endpoint</span>
                                            <ul style='padding-left: 1em'><li>The endpiont under the Post Destination Address to which iris images will be sent..</li></ul>");
        define ("TOOLTIP_IRISCAPTURE_MAINTENANCEURL", "<span style='font-size: 16px; font-style:bold'>Maintenance Endpoint</span>
                                            <ul style='padding-left: 1em'><li>The endpiont under the Post Destination Address to which maintenance messages will be sent.  Leave empty to not send messages.</li></ul>");
		define("TOOLTIP_DATABASE_TAB", " <b><i>База Данных</i></b><br>Конфигурация Базы Данных ".$NXTText.". (статистика Network Matcher и базы данных )");
		define("TOOLTIP_DATABASE_ENABLENETWORKMATCHER", " <span style='font-size: 16px; font-style:bold'>Включить Network Matcher</span> <ul style='padding-left: 1em'><li>Выберите эту настройку, чтобы включить дополнительную (вне устройства) обработку биометрических данных, используя <i>Eyelock Network Matcher Service</i>.</li></ul>");
		define("TOOLTIP_DATABASE_NETWORKMATCHERADDR", " <span style='font-size: 16px; font-style:bold'>Адрес Network Matcher</span> <ul style='padding-left: 1em'><li>Укажите IP адрес Eyelock Network Matcher Service, который будет обрабатывать запросы идентификации.</li></ul>");
		define("TOOLTIP_DATABASE_NETWORKMATCHERPORT", " <span style='font-size: 16px; font-style:bold'>Порт Network Matcher</span> <ul style='padding-left: 1em'><li>Укажите порт Eyelock Network Matcher Service, который будет обрабатывать запросы идентификации.</li></ul>");
		define("TOOLTIP_AUTHENTICATION_SECURENETWORK", " <span style='font-size: 16px; font-style:bold'>Включить режим безопасности для Network Matcher</span> <ul style='padding-left: 1em'><li>Включите эту настройку для безопасной пересылки сообщений через сеть..</li></ul>");
		define("TOOLTIP_DATABASE_STATISTICS", " <span style='font-size: 16px; font-style:bold'>Оставшееся Место для Шаблонов</span> <ul style='padding-left: 1em'><li>Это значение показывает текущее количество биометрических шаблонов, сохраненных в базе данных ".$NXTText." и максимально возможное количество шаблонов..</li></ul>");
		define("TOOLTIP_DATABASE_NETWORKMATCHER_SECURECOMM", " <span style='font-size: 16px; font-style:bold'>Включить режим безопасности для Network Matcher</span> <ul style='padding-left: 1em'><li>Включите эту настройку для безопасного обмена данными с <i>Eyelock Network Matcher Service</i>.</li></ul>");
		define("TOOLTIP_ACS_TAB", " <b><i>СКД</i></b><br>Конфигурация ".$NXTText." для Системы СКД/Панели двери (Протокол, СКД контроль светодиодов, Двойная идентификация, настройка Реле и СКД)");
		define("TOOLTIP_LOGS_TAB", " <b><i>Логи</i></b><br>Выводит на экран лог ".$NXTText." и позволяет пользователю скачать файл лога.");
		define("TOOLTIP_LOGS_REFRESHBUTTON", " <span style='font-size: 16px; font-style:bold'>Кнопка обновления</span> <ul style='padding-left: 1em'><li>Нажатие на эту кнопку синхронизирует <i>Лог Событий</i> с текущим содержимым лога.</li> <li><i>Лог Событий</i> синхронизируется, когда WebConfig запускается первый раз. Он не обновляется постоянно.</li></ul>");
		define("TOOLTIP_LOGS_DOWNLOADBUTTON", " <span style='font-size: 16px; font-style:bold'>Кнопка Скачивания</span> <ul style='padding-left: 1em'><li>Нажмите чтобы скачать <i>Лог Событий</i> как текстовый файл.</li></ul>");
		define("TOOLTIP_ACS_PROTOCOL", " <span style='font-size: 16px; font-style:bold'>Протокол СКД</span> <ul style='padding-left: 1em'><li>Используйте это выпадающее меню, чтобы выбрать протокол СКД, который будет использоваться .</li> <li>Поддерживаются следующие протоколы:</li> <ul><li>Weigand</li> <li>Wegand-HID</li> <li>F2F</li> <li>PAC</li> <li>OSDP</li></ul><ul>");
		define("TOOLTIP_OSDP_BAUD", " <span style='font-size: 16px; font-style:bold'>Протокол СКД</span> <ul style='padding-left: 1em'><li>Выберите скорость передачи данных, которую будет использовать Nano при общении с панелью OSDP.</li> <ul>");
		
         define ("TOOLTIP_OSDP_ADDRESS", "<span style='font-size: 16px; font-style:bold'>OSDP Address</span>
                                                <ul style='padding-left: 1em'><li>Specifies the OSDP address for the Output on a multi-drop RS-485 network. The default value for the address is 0.</li>
                                                <ul>");
        
        define("TOOLTIP_ACS_LEDCONTROLLEDACS", "<span style='font-size: 16px; font-style:bold'>СКД контролирует светодиоды </span>                                                  <br /><br />Контроль за светодиодами позволяет СКД контролировать состояние светодиодов и звука ".$NXTText.". Когда контроль за светодиодами не включен, ".$NXTText." управляет светодиодами самостоятельно.");
		define("TOOLTIP_ACS_DUALAUTHENTICATION", "<span style='font-size: 16px; font-style:bold'>Двойная Идентификация</span>                                                  <br /><br />При Двойной Идентификации пользователь обязан предоставить карту в присоединенный считыватель прежде, чем предоставить свои глаза.<br />".$NXTText." ищет карту во внутренней памяти и затем включает белый свет, подсказывая пользователю, что прибор готов принять глаза<br />Если предоставленная радужная оболочка совпадает с оболочкой, найденной в базе, ".$NXTText." посылает данные в СКД<br />Если совпадения не произошло, ".$NXTText." посылает сообщение о несовпадении.");
		define("TOOLTIP_ACS_DUALAUTHPARITY", "<span style='font-size: 16px; font-style:bold'>Биты четности при Двойной Идентификации</span>                                                  <br /><br />Выберите, проверять ли биты четности, приходящие от Считывателя Карт во время двойной идентификации.");
		define("TOOLTIP_ACS_IRISWAITTIME", " <span style='font-size: 16px; font-style:bold'>Время ожидания Радужной Оболочки.</span> <ul style='padding-left: 1em'><li>Определяет время в секундах, в течение которого пользователь должен предоставить свою оболочку глаза ".$NXTText." после сканирования карты.</li> <li>Диапазон значений (в секундах): 2 - 60.</li><li>Значение по умолчанию: 10.</li></ul>");
        define ("TOOLTIP_ACS_PINWAITTIME", "<span style='font-size: 16px; font-style:bold'>PIN Wait Time</span>
                                    <ul style='padding-left: 1em'><li>Specifies the duration, in seconds, within which the user must enter his PIN after scanning the card</li>
                                    <li>Duration Range (seconds): 2 - 60.</li>
                                    <li>Default value: 10.</li></ul>");
        define ("TOOLTIP_ACS_PINBURSTBITS", "<span style='font-size: 16px; font-style:bold'>PIN Burst Bits</span>
                                    <ul style='padding-left: 1em'><li>Specifies the bit size for PIN processing</li>
                                    <li>Default value: 4.</li></ul>");
		define("TOOLTIP_ACS_ENABLERELAYS", "<span style='font-size: 16px; font-style:bold'>Включение Реле</span> <ul style='padding-left: 1em'><li>Выбор этой настройки включает функционирование физических реле в панели СКД.</li> <li>Оба<i>Разрешить</i> и <i>Отказать</i> реле поддерживаются.</li> <li>Время, в течении которого реле остаются активными, определяется контролями ниже.</li><ul>");
		define("TOOLTIP_ACS_GRANTRELAYTIME", "<span style='font-size: 16px; font-style:bold'>Время Реле Разрешения</span> <ul style='padding-left: 1em'><li>Определяет время в секундах для запуска Реле Разрешения при успешной идентификации.</li> <li>Чтобы отключить  реле <i>Разрешения</i> поменяйте его значение на 0.</li> <li>Диапазон значений (в секундах): 0 - 10.</li> <li>Значение по умолчанию: 3.</li></ul>");
		define("TOOLTIP_ACS_DENYRELAYTIME", "<span style='font-size: 16px; font-style:bold'>Время Реле Отказа</span> <ul style='padding-left: 1em'><li>Определяет время в секундах для запуска реле <i>Отказа</i> при неудачной идентификации.</li><li>Чтобы отключить реле <i>Отказа</i>  поменяйте его значение на 0.</li><li>Диапазон значений (в секундах): 0 - 10.</li><li>Значение по умолчанию: 5.</li></ul>");
        define("TOOLTIP_ACS_DURESSRELAYTIME", "<span style='font-size: 16px; font-style:bold'>Время Реле Отказа</span> <ul style='padding-left: 1em'><li>Определяет время в секундах для запуска реле <i>Отказа</i> при неудачной идентификации.</li><li>Чтобы отключить реле <i>Отказа</i>  поменяйте его значение на 0.</li><li>Диапазон значений (в секундах): 0 - 10.</li><li>Значение по умолчанию: 5.</li></ul>");
		define("TOOLTIP_ACS_TESTCODE", "<span style='font-size: 16px; font-style:bold'>Проверка Кода объекта</span> <ul style='padding-left: 1em'><li>Показывает определенный в настройках Код Объекта, который используется с выбранным протоколом СКД при нажатии на кнопку <i>Проверить Сейчас!</i></li> <li>Используйте EyEnroll чтобы сконфигуроровать значение кода объекта.</li></ul> <span style='font-size: 16px; font-style:bold'>Проверка Кода объекта</span> <ul style='padding-left: 1em'><li>Показывает определенный в настройках Код Объекта, который используется с выбранным протоколом СКД при нажатии на кнопку <i>Проверить Сейчас!</i></li> <li>Используйте EyEnroll чтобы сконфигуроровать значение кода объекта.</li></ul>");
		define("TOOLTIP_ACS_TESTCARDID", "<span style='font-size: 16px; font-style:bold'>Номер тестовой карты</span> <ul style='padding-left: 1em'><li>Показывает определенный в настройках номер карты, который используется с выбранным протоколом СКД при нажатии на кнопку <i>Проверить Сейчас!</i></li> <li>Используйте EyEnroll чтобы сконфигуроровать значение кода карты.</li></ul>");
		define("TOOLTIP_ACS_TESTACSBUTTON", "<span style='font-size: 16px; font-style:bold'>Кнопка Проверить Сейчас!</span> <ul style='padding-left: 1em'><li>Нажатие на эту кнопку пошлет сконфигурированную тестовую строку на панель СКД. Необходимо использовать приложение EyEnroll, чтобы сконфигурировать тестовую строку на ".$NXTText.".</li></ul>");
		define("TOOLTIP_HEADER_LANGUAGE", "<span style='font-size: 16px; font-style:bold'>Меню языков</span> <ul style='padding-left: 1em'><li>Используйте это выпадающее меню, чтобы выбрать текущий язык для интерфейса WebConfig.</li><li>Только доступные языки присутствуют в списке.</li></ul><span style='font-size: 16px; font-style:bold'>Меню языков</span> <ul style='padding-left: 1em'><li>Используйте это выпадающее меню, чтобы выбрать текущий язык для интерфейса WebConfig.</li><li>Только доступные языки присутствуют в списке.</li></ul>");
		define("TOOLTIP_HEADER_EYELOCKLOGO", " <span style='font-size: 16px; font-style:bold'>Корпорация Eyelock</span> <ul style='padding-left: 1em'><li>Нажмите <a href='http://www.eyelock.com'  target='_blank'>here</a> чтобы посетить сайт Eyelock.</li></ul>");
		define("TOOLTIP_HEADER_CLIENTVER", " <span style='font-size: 16px; font-style:bold'>Версия Клиента WebConfig</span> <ul style='padding-left: 1em'><li>Показывает текующую версию клиента WebConfig.</li></ul> <span style='font-size: 16px; font-style:bold'>Версия Клиента WebConfig</span> <ul style='padding-left: 1em'><li>Показывает текующую версию клиента WebConfig.</li></ul>");
		define("TOOLTIP_HEADER_HELP", "<span style='font-size: 16px; font-style:bold'>Помощь</span> <ul style='padding-left: 1em'><li>Нажатие на эту ссылку выведет на экран диалоговое окно с настройками  <i>Системы Всплывающей Помощи</i> настройки могут быть изменены.</li></ul><span style='font-size: 16px; font-style:bold'>Помощь</span> <ul style='padding-left: 1em'><li>Нажатие на эту ссылку выведет на экран диалоговое окно с настройками  <i>Системы Всплывающей Помощи</i> настройки могут быть изменены.</li></ul>");
		define("TOOLTIP_HEADER_LOGOUT", "<span style='font-size: 16px; font-style:bold'>Выйти из системы</span> <ul style='padding-left: 1em'><li>Нажатие на эту ссылку выведет пользователя из текущей сессии и вернёт его на страницу логина.</li></ul>");
		define("TOOLTIP_FOOTER_APPVERSION", " <span style='font-size: 16px; font-style:bold'>Версия прошивки ".$NXTText."</span> <ul style='padding-left: 1em'><li>Показывает текущую версию прошивки ".$NXTText.".</li></ul> <span style='font-size: 16px; font-style:bold'>Версия прошивки ".$NXTText."</span> <ul style='padding-left: 1em'><li>Показывает текущую версию прошивки ".$NXTText.".</li></ul>");
		define("TOOLTIP_FOOTER_BOBVERSION", "<span style='font-size: 16px; font-style:bold'>Версия ".$NXTText." ICM</span> <ul style='padding-left: 1em'><li>Показывает текущую версию ICM на ".$NXTText.".</li></ul><span style='font-size: 16px; font-style:bold'>Версия ".$NXTText." ICM</span> <ul style='padding-left: 1em'><li>Показывает текущую версию ICM на ".$NXTText.".</li></ul>");
		define("TOOLTIP_FOOTER_SAVEBUTTON", "<span style='font-size: 16px; font-style:bold'>Сохранить</span> <ul style='padding-left: 1em'><li>Нажмите, чтобы сохранить изменения, сделанные в настройках ".$NXTText.".</li><li>После сохранения настроек ".$NXTText." автоматически перезагрузится и использует новые настройки.</li></ul><span style='font-size: 16px; font-style:bold'>Сохранить</span> <ul style='padding-left: 1em'><li>Нажмите, чтобы сохранить изменения, сделанные в настройках ".$NXTText.".</li><li>После сохранения настроек ".$NXTText." автоматически перезагрузится и использует новые настройки.</li></ul>");
		define("TOOLTIP_TAB_DEVICEIP", " <span style='font-size: 14px; font-style:bold'>Текущее имя/IP адрес ".$NXTText."</span>");
		define("TOOLTIP_TAB_DEVICEMACADDR", "<span style='font-size: 14px; font-style:bold'>Текущий мак-адрес ".$NXTText."</span>");
		define("VALIDATOR_MSG_HOSTNAME", "Имя хоста не должно содержать пробелы или специальные символы, и должно содержать менее 64 символов.");
		define("TITLE_BTN_TESTNWMS", "Проверьте текущие настройки Network Matcher.");
		define("TEST_NWMS", "Проверьте NWMS");
		define("CB_NWMATCHERCOMMSECURE_TEXT", "небезопасный");
		define("COPYRIGHTTEXT", " Copyright \AD&#169; 2014. Все права защищены.");
		define("DISCONNECTWARNING", "Не отключайте питание от ".$NXTText." или сеть от этого компьютера во время этого процесса.");
		
        break;
		case "pt":
		  //////////////////////////////////////////////////////
        // LOGIN PAGE
        //////////////////////////////////////////////////////
            define("TITLE_NANOLOGIN", "Acesso ".$NXTText."");
            define("LOGIN_USERNAME", "Usuário");
            define("LOGIN_PASSWORD", "Senha");
            define("LOGIN_LOGIN", "Entrar");
            define("LOGIN_FORGOTPASSWORD", "Esqueceu sua senha?");
            define("LOGIN_USER_GROUPTITLE", "Nível Login de Usuário");

			
			define("TITLE_LICENSE_KEY", "Chave de Licença Nano");
			define("LK_ENTER_LICENSE_KEY_HEADER", "Digite a chave de linceça");
			define("LK_ENTER_KEY_TEXT", "Por favor, digite a chave de licençaencontrada no <br> Cartão de Chave de Licença incluído com o Pacote ".$NXTText.".");
			define("LK_DEFAULT_TEXT", "Chave de Licença...");
			define("LK_BTN_ACTIVATE_TEXT", "Ativar");
			define("LK_INVALID_LICENSE", "Licença digitada não é válida");
        //////////////////////////////////////////////////////
        // CONFIGURATION PAGE
        //////////////////////////////////////////////////////
            define("TITLE_NANOCONFIG", "Configuração ".$NXTText."");
            define("TITLE_HELP", "Configurações de Ajuda");
            define("TITLE_LOGOUT", "Sair");
			define("HEADER_LICENSE", "Licença");
			  define("SUBMIT_BUTTON", "Salvar");
			 define("CANCEL_BUTTON", "Cancelar");
			define("DLGBTN_CONTINUE","Continuarão");
			
			 define ("NWMS_TEST_FAILED","Teste NWMS falhou");
			define ("NWMS_TEST_SUCCESS","Teste NWMS realizado com sucesso");
			define ("NWMS_SUCCESS_DETAIL","O Analisador de Rede foi encontrado no endereço alvo!");
				define ("NWMS_FAIL_DETAIL_NOTFOUND","O Analisador de Rede não foi encontrado no endereço alvo!");
				define ("NWMS_FAIL_DETAIL_INVALID","A resposta teste foi inválida. Verifique se o nano está operacional.");
			
			define("SESSION_TIMEOUT_WARNING", "Sua sessão irá expirar");
			define("SESSION_TIMEOUT_CONTINUE", "Clique Continuar para se manter conectado.");
            //////////////////////////////////////////////////////
            // HOME TAB STRINGS
            //////////////////////////////////////////////////////
            define ("HOME_TABLABEL", "Home");
            define ("HOME_HEADING", "Informação ".$NXTText."");
            define ("HOME_DEVICEINFO_GROUPTITLE", "Informação de Dispositivo");
            define ("HOME_DEVICEINFO_DEVICENAME", "Nome ".$NXTText.":");
            define ("HOME_DEVICEINFO_IPADDR", "Endereço IP:");
            define ("HOME_DEVICEINFO_IPV6ADDR", "Endereço IPv6:");
			define ("HOME_DEVICEINFO_SERIAL", "ID de Dispositivo:");
            define ("HOME_DEVICEINFO_MACADDR", "Endereço MAC:");
            define ("HOME_DEVICEINFO_LOCALTIME", $NXTText." Tempo Instantâneo");
            define ("HOME_DEVICEINFO_PREVIOUSUPDATEFAILED", "Uma tentativa anterior para atualizar o firmware nesta ".$NXTText." falhou. <br> O ".$NXTText." foi revertida para o último estado de funcionamento conhecido. <br> Atualize o firmware para limpar esta mensagem.");

            define ("HOME_STORAGESTATS_GROUPTITLE", "Estatísticas de armazenamento");
            define("HOME_STORAGESTATS_PT_GROUPTITLE", "Informações Template Decoder portátil");
            define ("HOME_STORAGESTATS_NETWORKMATCHENABLED", "Combinação de Rede:");
            define ("HOME_STORAGESTATS_NETWORKMATCHADDRESS", "Endereço de CdR:");
            define ("HOME_STORAGESTATS_TEMPLATESPACE", "Templates restantes:");

            define ("HOME_PTINFO_BLEVERSION", "Versão BLE:");
            define ("HOME_PTINFO_MAINFWVERSION", "Versão Firmware:");
            define ("HOME_PTINFO_BOOTLOADERVERSION", "Versão Boot Loader:");
            define ("HOME_PTINFO_HARDWAREVERSION", "Versão Hardware:");
            define ("HOME_PTINFO_KEYPADVERSION", "Versão Keypad:");
            define ("HOME_PTINFO_CONFIGURATION", "Configuração:");
            define ("HOME_CAMERA_FPGAVER", "Camera FPGA Ver.: ");
            define ("HOME_CAMERA_PSOCVER", "Camera PSOC Ver.: ");
            define ("HOME_FIXED_BOARDVER", "Fixed Board Ver.: ");

            define ("HOME_SOFTWARE_GROUPTITLE", "Informação Software");
            define ("HOME_SOFTWARE_APPVER", "Firmware ".$NXTText.":");
            define ("HOME_SOFTWARE_BOBVER", "Versão FW ICM:");
            define ("HOME_HARDWARE_BOBVER", "Versão HW ICM:");
             define ("HOME_SOFTWARE_LINUXVER", "Versão Linux OS:");
             define ("HOME_HARDWARE_PSOCVER", "Versão M.B. PSOC");
            define ("HOME_SOFTWARE_NANOLABEL", "Última atualização");
            define ("HOME_SOFTWARE_BOBLABEL", "Última atualização</br>de ICM:");
			define ("HOME_SOFTWARE_FPGALABEL", "Última atualização</br>de Camera FPGA: ");
			define ("HOME_SOFTWARE_CAMERAPSOCLABEL", "Última atualização</br>de Camera PSOC: ");
			define ("HOME_SOFTWARE_FIXEDPSOCLABEL", "Última atualização</br>de Fixed Board: ");
			

          
            //////////////////////////////////////////////////////
            // NETWORK TAB STRINGS
            //////////////////////////////////////////////////////
            define ("NETWORK_TABLABEL", "Rede");
            define ("NETWORK_HEADING", "Configuração de Dispositivo de Rede");
            define ("NETWORK_DEVICENAME_GROUPTITLE", "Nome do dispositivo");
            define ("NETWORK_DEVICENAME_LABEL", "Nome:");
            define ("NETWORK_DHCP_GROUPTITLE", "Obter um endereço IP automaticamente (DHCP)");
            define ("NETWORK_DHCP_SETTINGS", "Configurações DHCP...");
			define("NETWORK_DNS_SERVER1", "Servidor DNS 1:");
			define("NETWORK_DNS_SERVER2", "Servidor DNS 2:");
			
			
                ///// ADV. DHCP SETTINGS DIALOG
                define ("NETWORK_DHCPSETTINGS_TITLE", "Configurações DHCP avançadas...");
                define ("NETWORK_DHCPSETTINGS_TIMEOUTS_GROUPLABEL", "Tempo Limite");
                define ("NETWORK_DHCPSETTINGS_TIMEOUTS_DHCPTIMEOUTLABEL", "Tempo Limite DHCP");
                define ("NETWORK_DHCPSETTINGS_TIMEOUTS_DHCPRETRIESLABEL", " DHCP Retentativas:");
                define ("NETWORK_DHCPSETTINGS_TIMEOUTS_DHCPRETRYDELAYLABEL", "Delay de retentativa:");
                define ("NETWORK_DHCPSETTINGS_OK", "OK");
            define ("NETWORK_STATICIP_GROUPLABEL", "Usar Endereço IP estático seguinte");
            define ("NETWORK_STATICIP_DEVICEIPLABEL", "IP do dispositivo:");
            define ("NETWORK_STATICIP_BROADCASTNETWORKLABEL", "Rede Broadcast:");
            define ("NETWORK_STATICIP_SUBNETMASKLABEL", "Máscara de subrede:");
            define ("NETWORK_STATICIP_DEFAULTGATEWAYLABEL", "Gateway padrão:");
            define ("NETWORK_IPV6_ENABLE", "IPV6 Ativo");
            define ("NETWORK_IPV6_CONFIGURATION", "Configuração IPv6...");
                // IPV6 Pop-up
                define ("NETWORK_IPV6CONFIGURATION_TITLE", "Configuração IPv6");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL", "Geral");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE", "Modo DHCP:");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_INFORMATION", "Sem estado");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_NORMAL", "Stateful");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_AUTO", "Auto");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_NONE", "Desligado");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_ADDRESS_PREFIX_LENGTH", "Endereço IPv6/Comprimento do prefixo de sub-rede:");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DEFAULTGATEWAY", "Gateway padrão:");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DNS_SERVER1", "Servidor DNS 1:");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DNS_SERVER2", "Servidor DNS 2:");
                define ("NETWORK_IPV6CONFIGURATION_STATUS", "Estado");
                define ("NETWORK_IPV6CONFIGURATION_STATUS_LINK_LOCAL", "Link-local:");
                define ("NETWORK_IPV6CONFIGURATION_STATUS_GLOBAL", "Global:");
                define ("NETWORK_IPV6CONFIGURATION_STATUS_GLOBAL_STATELESS", "Global stateless:");
				define ("NETWORK_IPV6CONFIGURATION_STATUS_GLOBAL_STATELESS_TEMPORARY", "Global stateless temporary:");
                define ("NETWORK_IPV6CONFIGURATION_STATUS_NO_ADDRESSES", "O dispositivo atualmente não tem endereços IPv6 atribuídos a ele");

            define("SSL_PROTOCOL_LEGEND", "SSL Protocolo");
            define("SSLPROTO_DEFAULT", "Legacy");
            define("SSLPROTO_TLS12", "TLS 1.2 (Only)");

            define ("NETWORK_CERTIFICATES_GROUPTITLE", "Certificates");
            define ("NETWORK_CACERT", "CA Certificate:");
            define ("NETWORK_CLIENTCERT", "Client Certificate:");
            define ("NETWORK_CLIENTPRIVATEKEY", "Client Private Key (and Certificate):");
            define ("NETWORK_CERTIFICATES_SETTINGS_GROUPTITLE", "Settings");
            define ("NETWORK_EAPOLVERSION", "EAPOL Version:");
            define ("NETWORK_EAPIDENTITY", "EAP Identity:");
            define ("NETWORK_PRIVTEKEYPWD", "Private Key Password:");

            define ("NETWORK_SETTINGS_ENABLEIEEE8021X", "Ativar IEEE 802.1X");
            define ("NETWORK_SETTINGS_NOCERTIFICATE", "No Certificate File Uploaded");
            define ("NETWORK_SETTINGS_NOKEY", "No Private Key Uploaded");
            define ("NETWORK_802LOG_DOWNLOAD", "Download IEEE 802.1X Log...");
            define ("NETWORK_CHECKING_IP_ADDRESS_DUPLICATE", "Checking for IP address duplicates");
            define ("NETWORK_DUPLICATE_IP_ADDRESS", "IP address conflict");
            define ("NETWORK_IP_ADDRESS_IN_USE", "Specified device IP address conflict with another system on the network.");


            //////////////////////////////////////////////////////
            // DEVICE TAB STRINGS
            //////////////////////////////////////////////////////
            define ("DEVICE_TABLABEL", "Dispositivo");
            define ("DEVICE_HEADING", "Configurações de Dispositivo");
            define ("DEVICE_USERFEEDBACK_GROUPTITLE", "Cometários");
            define ("DEVICE_USERFEEDBACK_VOLUMELABEL", "Volume de Altofalante:");
            define ("DEVICE_USERFEEDBACK_FREQUENCYLABEL", "Frequência (Hz):");
            define ("DEVICE_USERFEEDBACK_DURATIONLABEL", "Duração do Tom:");
            define ("DEVICE_USERFEEDBACK_TAMPERVOLUMELABEL", "Volume do Tom de Adulteração");
            define ("DEVICE_USERFEEDBACK_LEDBRIGHTNESSLABEL", "Brilho LED:");
            define ("DEVICE_USERFEEDBACK_LOCATEDEVICE", "Localizar dispositivo...");
            define ("DEVICE_TIMESETTINGS_GROUPTITLE", "Configurações de horário");
            define ("DEVICE_TIMESETTINGS_SERVERLABEL", "Servidor de horário:");
            define ("DEVICE_TIMESETTINGS_NANOTIMELABEL", "Horário local nano:");
            define ("DEVICE_TIMESETTINGS_UPDATETIME", "Sincronizar agora!");
            define ("DEVICE_TIMESETTINGS_SYNCHRONIZETIMELABEL", "Sincronizar diariamente");
	        define ("DEVICE_TIMESETTINGS_UPDATELOCALTIME", "Sincronizar com servidor");
            define ("DEVICE_ACTIVITIES_GROUPTITLE", "Atividades");
            define ("DEVICE_ACTIVITIES_FACTORYRESET", "Reset de fábrica");
			define ("DEVICE_ACS_OSDPINSTALLMODE", "OSDP Installation Mode"); 
            define ("DEVICE_ACTIVITIES_REBOOTDEVICE", "Reiniciar dispositivo");
            define ("DEVICE_EXTERNAL_GROUPTITLE", "HBOX Settings (EyeLock Support Only!)");
            define("DEVICE_EXTERNAL_WELCOMEMESSAGE", "Welcome Message:");
            define("DEVICE_EXTERNAL_LOCATION", "Location:");
            define("DEVICE_EXTERNAL_POSTTITLE", "Post Event URLs");
            define("DEVICE_EXTERNAL_DESTINATIONURL", "Main URL:");
            define("DEVICE_EXTERNAL_IRISURL", "Iris Post Endpoint:");
            define("DEVICE_EXTERNAL_ERRORURL", "Error Endpoint:");
            define("DEVICE_EXTERNAL_HEARTBEATURL", "HeartBeat Endpoint:");
            define("DEVICE_EXTERNAL_MAINTENANCEURL", "Maintenance Endpoint:");
            define("DEVICE_EXTERNAL_POSTSCHEMEURL", "POST Scheme:");
            define ("DEVICE_ADVSETTINGS_GROUPTITLE", "Configurações Avançadas de Dispositivo");
            define ("DEVICE_ADVSETTINGS_LISTENINGPORTLABEL", "Porta de Escuta de Rede:");
            define ("DEVICE_ADVSETTINGS_EYEDESTADDR", "Endereço de Destino de Olho:");
            define ("DEVICE_ADVSETTINGS_EYEDESTPORT", "Porta de Destino de Olho:");
			
			  define ("DEVICE_ADVSETTINGS_EYECONNECTTIMEOUT", "Eye Connect Timeout:");
			  define ("DEVICE_ADVSETTINGS_EYESENDTIMEOUT", "Eye Send Timeout:");
			

	define ("DEVICE_TIMESERVERFAIL_PING", "Não foi possível realizar Ping com o servidor de hora.");
			define ("DEVICE_TIMESERVERFAIL_SYNC", "Não foi possível sincronizar com o servidor de hora.");
			
			
			define("FACTORY_RESET_PROGRESS", "Uma restauração de fábrica está em andamento.");
			define("FACTORY_RESET_LOGGEDOUT", "Você foi desconectado por este processo.");
			define("FACTORY_RESET_TWOMINS", "Você poderá entrar novamente depois de 2 minutos.");
            //////////////////////////////////////////////////////
            // SECURITY TAB STRINGS
            //////////////////////////////////////////////////////
            define ("SECURITY_TABLABEL", "Segurança");
            define ("SECURITY_HEADING", "Configurações de Segurança");

            define ("SECURITY_PASSWORD_GROUPTITLE", "Resetar senha");
            define ("SECURITY_PASSWORD_OLDPWDLABEL", "Senha Atual:");
            define ("SECURITY_PASSWORD_NEWPWDLABEL", "Nova Senha:");
            define ("SECURITY_PASSWORD_CONFIRMPWDLABEL", "Confirmação de Senha:");
            define ("SECURITY_PASSWORD_REMOVEPWDLABEL", "Limpar Senha");
            define ("SECURITY_PASSWORD_RESETPWD", "Atualizar Senha");

            define ("SECURITY_TAMPER_SETTINGS", "Configurações Tamper");
            define ("SECURITY_TAMPER_SIGNALHIGH", "Ativar Estado Tamper em Alto Sinal");
            define ("SECURITY_TAMPER_SIGNALLOW", "Ativar Estado Tamper em Baixo Sinal");
            define ("SECURITY_TAMPER_NOTIFYADDRESS", "Endereço de Notificação:");
            define ("SECURITY_TAMPER_NOTIFYPORT", "Porta:");
            define ("SECURITY_TAMPER_NOTIFYMESSAGE", "Mensagem Tamper:");
	     
				define ("SECURITY_TAMPER_MESSAGE_SETTINGS", "Configurações de Mensagem Tamper");
             define ("SECURITY_CARD_READER_INPUT_TAMPER_SETTINGS", "Configurações de leitor de cartão de entrada Tamper");
              define ("SECURITY_NXT_OUTPUT_TAMPER_SETTINGS", "Configurações de Saída Tamper");
               define ("SECURITY_OUTPUT_TAMPER_SIGNALHIGH", "Alto Sinal");
            define ("SECURITY_OUTPUT_TAMPER_SIGNALLOW", "Baixo Sinal");
			
            //////////////////////////////////////////////////////
            // SOFTWARE TAB STRINGS
            //////////////////////////////////////////////////////
			define("SOFTWARE_FIRMWARELINK","Acesse <a href=\"http://help.eyelock.com\">help.eyelock.com</a> para o fimware mais recente.");
            define ("SOFTWARE_TABLABEL", "Software");
            define ("SOFTWARE_HEADING", "Detalhes Software/Firmware");
            define ("SOFTWARE_STATUS_GROUPTITLE", "Versão/Atualizar Estado");
            define ("SOFTWARE_CHECKUPDATES_LABEL", "Verificação mais recente para atualizações:");
            define ("SOFTWARE_UPDATEDETAILS_TITLE", "Detalhes de Atualização de Software");
            define ("SOFTWARE_AVAILUPDATE_NANOLABEL", "Nova Versão ".$NXTText.":");
            define ("SOFTWARE_AVAILUPDATE_BOBLABEL", "Nova Versão ICM:");
            define ("SOFTWARE_INSTALLEDUPDATES_NANOLABEL", "Atualização ".$NXTText." foi instalada:");
            define ("SOFTWARE_INSTALLEDUPDATES_BOBLABEL", "Atualização ICM foi instalada:");
            define ("SOFTWARE_UPDATE_ALLOWSITEADMIN", "Permitir admin a atualizar dispositivo");
			define ("VERSION_HEADER", "Versão");
			define("RESTORE_POINTS_TIME_STAMP","Restaurar Ponto de Horário ");



            define ("SOFTWARE_STATUS_UPDATESTATUS_FAILED", "Falha na comunicação com o Servidor de Atualização");
            define ("SOFTWARE_STATUS_UPDATESTATUS_NEWVERSION", "Uma nova atualização de software ".$NXTText." está disponível!");
            define ("SOFTWARE_STATUS_UPDATESTATUS_CURRENT", "O Software ".$NXTText." está atualizado!");
            define ("SOFTWARE_STATUS_UPDATESTATUS_VERCORRUPT", "Arquivos de versão parecem estar corrompidos!");
            define ("SOFTWARE_STATUS_UPDATESTATUS_CHECKINTERNET", "Verifique o estado de conexão com a Internet!");
            define ("SOFTWARE_STATUS_UPDATENOW", "Atualizar agora!");
            define ("SOFTWARE_STATUS_LATER", "Atualizar mais tarde");
            define ("SOFTWARE_STATUS_MANUALNANO", "Arquivo local...");
            define ("SOFTWARE_STATUS_MANUALBOB", "Arquivo ICM local...");
            define ("SOFTWARE_STATUS_UPDATEDETAIL", "Detalhes...");
            define ("SOFTWARE_MODE_NANOLABEL", "Pontos de restauração ".$NXTText."");
			define ("SOFTWARE_MODE_BOBLABEL", "Pontos de restauração ICM Board");
            define ("SOFTWARE_MODE_DELETERESTOREPOINTS", "Apagando Pontos de restauração...");
            define ("SOFTWARE_MODE_RESTORERESTOREPOINT", "Restaurando para uma versão anterior...");
            define ("SOFTWARE_RESTOREPOINTS_NONANO", "Nenhum ponto de restauração ".$NXTText." disponível!");
            define ("SOFTWARE_RESTOREPOINTS_NOBOB", "Nenhum ponto de restauração ICM disponível!");
            define ("SOFTWARE_RESTOREHEADER_SELECT", "Selecionar");
            define ("SOFTWARE_RESTOREHEADER_RESTOREPOINTS", "Restaurar Pontos");
            define ("SOFTWARE_RESTORE_GROUPTITLE", "Restaurar Firmware");
            define ("SOFTWARE_RESTORE_RESTORENOW", "Restaurar agora...");
            define ("SOFTWARE_RESTORE_DELETERESTOREPOINTS", "Apagar...");
			

            //////////////////////////////////////////////////////
            // AUTHENTICATION TAB STRINGS
            //////////////////////////////////////////////////////
            define ("AUTHENTICATION_TABLABEL", "Autenticação");
            define ("AUTHENTICATION_HEADING", "Configurar Detalhes de Autenticação");
            define ("AUTHENTICATION_SETTINGS_IRISPROCESSINGMODE", "Iris Processing Mode:");
            define ("AUTHENTICATION_SETTINGS_ACCESSCONTROLMODE", "Access Control Authentication");
            define ("AUTHENTICATION_SETTINGS_ACCESSCONTROLAUTHENTICATION", "Access Control Authentication");
            define ("AUTHENTICATION_SETTINGS_IRISCAPTUREMODE", "Iris Capture Mode");
            define ("AUTHENTICATION_SETTINGS_IRISCAPTURE", "Iris Capture");
            define ("DEVICE_EXTERNAL_POSTSCHEME", "Iris Post Scheme:");
			define ("AUTHENTICATION_SETTINGS_IRISCAPTURESETTINGS", "Iris Capture Settings");
			define ("AUTHENTICATION_SETTINGS_IRISCAPTURETIMEOUT", "Iris Capture Timeout:");
			define ("AUTHENTICATION_SETTINGS_IRISCAPTURERESETDELAY", "Iris Capture Reset Delay:");
			define ("AUTHENTICATION_SETTINGS_HTTPPOSTMSGFORMAT", "Http Post Message Format:");
			define ("AUTHENTICATION_SETTINGS_IRISIMAGEFORMAT", "Iris Image Format:");
			define ("AUTHENTICATION_SETTINGS_IRISIMAGEQUALITY", "Image Quality:");
			define ("AUTHENTICATION_SETTINGS_100LOSSLESS", "(100 = Lossless)");
            define ("AUTHENTICATION_MODE_GROUPTITLE", "Combinando");
            define ("AUTHENTICATION_MODE_SINGLEEYELABEL", "Usar Apenas Um Olho");
            define ("AUTHENTICATION_MODE_DUALEYELABEL", "Usar Ambos os Olhos");
            define ("AUTHENTICATION_SETTINGS_GROUPTITLE", "Configurações");
            define ("AUTHENTICATION_SETTINGS_REPEATPERIODLABEL", "Período de repetição de Autorização:");
            define ("AUTHENTICATION_SETTINGS_NEGMATCHTIMEOUTENABLEDLABEL", "Ativar Timeouts de Negativa de Combinação");
            define ("AUTHENTICATION_SETTINGS_LOITERPERIODLABEL", "Período de lentidão:");
            define ("AUTHENTICATION_SETTINGS_NEGMATCHRESETLABEL", "Tempo de reset de negativa de combinação");
            define ("AUTHENTICATION_SETTINGS_DESTINATIONADDRESSLABEL", "IP destino de mensagem de rede:");
            define ("AUTHENTICATION_SETTINGS_DESTINATIONPORTLABEL", "Porta:");
            define ("AUTHENTICATION_SETTINGS_SECURENETWORDLABEL", "Mensagem de combinação de rede segura");
            define ("AUTHENTICATION_SETTINGS_MSGFORMATLABEL", "Formato de mensagem de rede:");
            define ("AUTHENTICATION_SETTINGS_SENDALLIMAGES", "Send All Images");


            //////////////////////////////////////////////////////f
            // DATABASE TAB STRINGS
            //////////////////////////////////////////////////////
            define ("DATABASE_TABLABEL", "Base de dados");
            define ("DATABASE_HEADING", "Detalhes de configuração de base de dados");
            define ("DATABASE_TYPE", "Tipo de base de dados:");
            define ("DATABASE_SQLLITE", "SQLite");
            define ("DATABASE_BINARY", "Binário (arquivo flat)");
            define ("DATABASE_TYPE_GROUPTITLE", "Detalhes de base de dados");
            define ("DATABASE_TYPE_LOCALLABEL", "Local");
            define ("DATABASE_TYPE_NETMATCHLABEL", "Ativar Combinador de Rede (NWMS)");
            define ("DATABASE_TYPE_NETMATCHADDRESSLABEL", "Endereço de Combinador de Rede:");
			define ("TESTING_NETWORK_MATCHER", "Testando combinador de rede...");
            define ("DATABASE_TYPE_NETMATCHPORTLABEL", "Porta de Destino do Combinador de Rede:");
            define ("DATABASE_STATISTICS_GROUPTITLE", "Estatísticas de base de dados");
               define ("DATABASE_TOC_GROUPTITLE", "Templates portáteis");
            define ("DATABASE_STATISTICS_TEMPLATESPACE", "Espaço de Template restante:");
			define ("DATABASE_SECURECOMM_NETMATCHLABEL", "Combinador de rede segura");
            define("DATABASE_MOBILEMODE", "Modo Mobile:" );
            define ("TOOLTIP_TOC_MODE", "Template portátil em Aplicação Smartphone irá utilizar estes modos: Role para cima, Toque para enviar, pressione para enviar </ br> <strong> Não afeta sistema de cartão EV1.</ strong>");
            define ("TOOLTIP_TOC_IRIS_WAIT_TIME", "Tempo em segundos que o ".$NXTText." mantém um template portátil depois que foi apresentado. Mínimo de 10 segundos, máximo de 600 segundos.");
              define("TOOLTIP_TOC_DEFAULT_KEY", "Usar a chave ".$NXTText." padrão para templates portáteis.");
             define("TOOLTIP_TOC_CUSTOM_KEY", "Usar chave carregada para templates portáteis.");
            define("TOOLTIP_UPLOAD_CERTIFICATE", "Carregar uma chave .pfx para usar com templates portáteis. Fornecer a senha para a chave na caixa à esquerda.");
            define("TOOLTIP_CURRENT_CERTIFICATE", "Se estiver mostrando \"Upload key...\" por favor, carregue uma chave.");
            define("DATABASE_EXPIRATION", "Tempo de expiração de íris");
			
			
		
			
            //////////////////////////////////////////////////////
            // ACS TAB STRINGS
            //////////////////////////////////////////////////////
            define ("ACP_TABLABEL", "ACS");
            define ("ACP_HEADING", "Sistema de Controle de Acesso (ACS)");
            define ("ACP_PROTOCOL_GROUPTITLE", "Protocolo de Controle de Acesso");
            define ("ACP_PROTOCOL_PROTOCOL", "Protocolo:");
            define ("ACP_PROTOCOL_DUALAUTHENABLEDLABEL", "Autenticação de fator duplo");
             define ("ACP_PROTOCOL_TEMPLATEONCARD", "Template Portátil");
             define ("ACP_PROTOCOL_TEMPLATEONCARDPASS", "Autenticação de fator singular");
			 
             
               define ("ACP_PROTOCOL_DUALAUTHPARITY", "Verifique os Bits de Paridade");
            define ("ACP_PROTOCOL_DUALAUTHLEDENABLEDLABEL", "LED controlado por ACS");
            define ("ACP_PROTOCOL_MATCHWAITIRISTIMELABEL", "Tempo de espera de íris:");
            define ("ACP_PROTOCOL_MATCHWAITPINTIMELABEL", "Tempo de espera de PIN:");
            define ("ACP_PROTOCOL_PINBURSTBITS", "PIN Burst Bits:");
            define ("ACP_PROTOCOL_RELAYTIMELABEL", "Tempo de retransmissão permitido:");
            define ("ACP_PROTOCOL_DENYRELAYTIMELABEL", "Tempo de retransmissão negado:");
            define ("ACP_PROTOCOL_DURESSRELAYTIMELABEL", "Tempo de retransmissão duress:");
            define ("ACP_PROTOCOL_ENABLERELAYTRIGGERSLABEL","Ativar retransmissão");
            define ("ACP_PROTOCOL_NEGMATCHTIMEOUTLABEL", "Timeout de Combinação Negativa:");

            define ("ACP_TEST_GROUPTITLE", "Acesso Negado e Dado de teste");
            define ("ACP_TEST_TESTBUTTON", "Testar agora!");
            define ("ACP_TEST_CARDIDLABEL", "Número de Cartão:");
            define ("ACP_TEST_FACILITYCODELABEL", "Código de Facilidade:");
            define ("ACP_TEST_SENDINGMESSAGE", "Enviando mensagem de texto para o Painel de ACS ...");
            define ("ACP_NETWORK_SECURECOMMLABEL", "Comunicação segura");
            define ("ACP_TEST_TCPCONNECTIONFAILED", "Falha na conexão socket");
            define ("ACP_TEST_FAILED", "Falha ao enviar uma string teste ACS para o painel.");
            define ("ACP_TEST_CONNECTIONFAILED", "Não é possível estabelecer conexão de socket para firmware.");

            define ("AUTHENTICATION_SCHEME", "Esquema de Autenticação:");
            define ("ACP_DD_SINGLEFACTORIO", "Fator Simples [Uma única íris]");
            define ("ACP_DD_SINGLEFACTORIC", "Fator Simples [ris ou cartão]");
            define ("ACP_DD_DUALFACTORIC", "Fator Duplo [Íris e Cartão]");
            define ("ACP_DD_DUALFACTORICPP", "Fator Duplo [Íris e Cartão (PIN pass-through)]");
            define ("ACP_DD_DUALFACTORPI", "Fator Duplo [Íris e PIN]");
            define ("ACP_DD_DUALFACTORIPID", "Fator Duplo [Íris e PIN (Duress)]");
            define ("ACP_DD_DUALFACTORICPI", "Fator Tres [Íris, Cartão e PIN]");
            define ("ACP_DD_DUALFACTORICPID", "Fator Tres [Íris, Cartão e PIN (Duress)]");


			//todo:  Translate these to BR-pr
				define("PARITY_MASK_GROUP_TITLE", "Máscara de Paridade");
			define("TOOLTIP_ACS_PARITY_MASK_DISABLED", "Quando marcado o ".$NXTText." irá verificar os bits de paridade da leitora durante a Autenticação de Fator Duplo.");
			define("TOOLTIP_ACS_PARITY_MASK_ENABLED", "Quando marcado o ".$NXTText." irá ignorar os bits de paridade da leitora durante a Autenticação de Fator Duplo.");
			
			define("ACS_PARITY_MASK_DISABLED", "Sem Máscara");
			define("ACS_PARITY_MASK_ENABLED", "Máscara");
			
            //////////////////////////////////////////////////////
            // LOGS TAB STRINGS
            //////////////////////////////////////////////////////
			define ("LOG_NO_INFO", "Relatório de informação não disponível!");
			define("MATCH_FAIL_NO_IRIS","Falha de Combinação: Nenhuma Iris Presente");
			define("MATCH_FAIL_MISMATCH","Falha de Combinação: Iris Não Combina");
			define("MATCH_FAIL_INVALID_CARD","Falha de Combinação: Cartão Inválido");
            define("MATCH_SUCCESS","Match Success");
            define("MATCH_SUCCESSDURESS","Match Success (Duress)");
            define("MATCH_FAIL_INVALIDPIN","Falha de Combinação: PIN Inválido");
            define("MATCH_FAIL_NOPIN","Falha de Combinação: No PIN");
            define ("LOGS_TABLABEL", "Logs");
            define ("LOGS_HEADING", "Logs");
            define ("LOGHEADER_STATUS", "Estado");
            define ("LOGHEADER_DATE", "Data/Hora");
            define ("LOGHEADER_NAME", "Nome");
            define ("LOGHEADER_CODE", "Código ACS");
            define ("LOGHEADER_MESSAGE", "Mensagem");
            define ("LOGS_EVENTLOG_GROUPTITLE", "Log de Evento");
            define ("LOGS_EVENTLOG_REFRESHBUTTON", "Atualizar!");
            define ("LOGS_EVENTLOG_DOWNLOAD", "Baixando Log...");
            define ("LOG_AUTOREFRESH_LABEL", "Auto Refresh:");
			define("LOGS_TIMES_SHOWN_IN","Todas as horas mostradas em");
			define ("LOG_TYPE", "Modo de Log");
            //////////////////////////////////////////////////////
            // DUMP TAB STRINGS
            //////////////////////////////////////////////////////
            define ("DUMP_TABLABEL", "Descarregar");

			
            //////////////////////////////////////////////////////
            // HELP SETTINGS DIALOG STRINGS
            //////////////////////////////////////////////////////
            define ("DIALOG_HELPSETTINGS_TITLE", "Configurações de Ajuda");
            define ("DIALOG_HELPSETTINGS_ENABLEHELP", "Ativar Popup de Ajuda");
            define ("DIALOG_HELPSETTINGS_POPUPTRIGGERMODE", "Modo de Gatilho de Popup");
            define ("DIALOG_HELPSETTINGS_POPUPHOVER", "Hover do Mouse");
            define ("DIALOG_HELPSETTINGS_POPUPCLICK", "Clique do Mouse");
            define ("DIALOG_HELPSETTINGS_POPUPDELAY", "Atraso no hover do cursor:");


            //////////////////////////////////////////////////////
            // HELP DIALOG HELP STRINGS
            //////////////////////////////////////////////////////
            define ("TOOLTIP_HELPSETTINGS_ENABLEHELP", "<Span style = 'font-size: 16px; font-style: bold '> Ativar Popup de Ajuda </ span> 
			<ul style =' padding-left:. 1em '> 
			<li> Selecione esta opção para habilitar o <i> Sistema de Popup de Ajuda </ i> </ li> 
			<li> Desmarque a opção para desabilitar completamente o <i> Sistema de Popup de Ajuda </ i>. </ li> </ ul>");
            define ("TOOLTIP_HELPSETTINGS_POPUPMODE", " <span style='font-size: 16px; font-style:bold'>Modo de Popup de Ajuda</span>
			 <ul style='padding-left: 1em'><li>Selecione o método que é usado para iniciar <i>Sistema de Popup de Ajuda</i> popups.</li></ul>");
            define ("TOOLTIP_HELPSETTINGS_POPUPDELAY", " <span style='font-size: 16px; font-style:bold'>Cursor Hover Delay</span>
			 <ul style='padding-left: 1em'>
			 <li>Especifique o tempo em segundos que o ponteiro do mouse deve passar sobre o campo antes da janela do <i>Sistema de Popup de Ajuda</i> aparecer.</li>
			 <li>Variação de Duração (segundos): 0 - 5.</li> 
			 <li>Valor padrão: 1.0.</li></ul>");


            //////////////////////////////////////////////////////
            // COMMON STRINGS
            //////////////////////////////////////////////////////
            define ("TAMPER_ICON_ALTTEXT", "Alerta Tamper!");

            /////////////////////////////////////////////////////
            // TOOLTIP STRINGS
            /////////////////////////////////////////////////////
            define ("TAMPER_TOOLTIP_TEXT", "Detectado Dispositivo Tamper.");
            define ("ATOMIC_TOOLTIP_TEXT", "Aplicação Eyelock está em execução!");
            define ("POWERBUTTON_TOOLTIP_TEXT", "Aplicação Eyelock não está em execução!");
            define ("EYELOCK_APPLICATION_STATUS", "Estado de Aplicação Eyelock");
            define ("EYELOCK_MASTER_STATUSTEXT", "Estado de Dispositivo Mestre:");
            define ("EYELOCK_SLAVE_STATUSTEXT", "Estado de Dispositivo Escravo:");
            define ("EYELOCK_STATUS_RUNNING", "OK");
            define ("EYELOCK_STATUS_NOTRUNNING", "Falhou");

            /////////////////////////////////////////////////////
            // ASSORTED STRINGS
            /////////////////////////////////////////////////////
			define("MSG_UPDATING", "Atualizando");
            define ("SECONDS_LABEL", " Segundos");
            define ("MILLISECONDS_LABEL", " Millisegundos");
            define ("DEFAULT_EMPTY_FIELD", "Opcional");
            define ("REQUIRED_EMPTY_FIELD", "Obrigatório");
            define ("CHANGE_PASSWORD_OLD", "Senha Atual");
            define ("CHANGE_PASSWORD_NEW", "Nova Senha");
            define ("CHANGE_PASSWORD_CONFIRM", "Confirmação de Senha");
            define ("MSG_UNAVAILABLE", "Indisponível!");
            define ("MSG_USERHELLO", "Olá");
            define ("MSG_UNKNOWNUSER", "Usuário desconhecido");
            define ("MSG_ENABLED", "Ativado");
            define ("MSG_DISABLED", "Desativado");
            define ("MSG_NEVER", "Nunca!");
            define ("MSGBOX_INFORMATION_TITLE", "Informação");
            define ("MSGBOX_INFORMATION_TITLESHORT", "Informação");
            define ("MSGBOX_SUCCESS_TITLE", "Sucesso");
            define ("MSGBOX_FAILED_TITLE", "Falhou");
            define ("MSGBOX_TAMPERED_TITLE", "Tamper!");
            define ("MSGBOX_OKBUTTON", "OK");
            define ("MSGBOX_CANCELBUTTON", "Cancelar");
            define ("MSGBOX_YESBUTTON", "Sim");
            define ("MSGBOX_NOBUTTON", "Não");
            define ("SAVING_SETTINGS", "Salvando configurações...");
            define ("SAVING_SETTINGSANDRESTART", "Salvando configurações e reiniciando...");
            define ("SAVING_FEWMOMENTS", "Isso pode levar algum tempo...");
            define ("RELOADING_PAGE", "Recarregando a página... Por favor, aguarde...");
            define ("REFRESHING_PAGE", "Atualizando a página... Por favor, aguarde...");
            define ("VALIDATION_FAILEDTITLE", "Falha na validação!");
            define ("VALIDATION_MESSAGE1", "Alguns campos não contêm informações válidas!");
            define ("VALIDATION_MESSAGE2", "Por favor, verifique todas as abas de campos inválidos!");
            define ("CONNECTION_FAILEDTITLE", "Problema de conexão!");
            define ("CONNECTION_MESSAGE1", "WebConfig foi incapaz de conectar ao dispositivo!");
            define ("CONNECTION_MESSAGE2", "Por favor, verifique o endereço IP e a energia do dispositivo!");
            define ("LOADINGLOG_DETAIL", "Carregando detalhes do log... Por favor, aguarde...");
            define ("ALERT_IPINUSE", "O IP estático especificado já está em uso! Nem todas as configuralçoes foram salvas!");
            define ("ALERT_802CONFIG", "Unable to verify IEEE 802.1X configuation information.  Not all settings could be saved! Check to ensure that you have uploaded all of the required Certificates/Keys!");
            define ("RESETPASSWORD_MESSAGETITLE", "Reset senha");
            define ("RESETTINGPASSWORD_MESSAGE", "Resetando senha. Por favor, aguarde...");
            define ("RESETPASSWORD_SUCCESS", "Senha resetada com sucesso!");
            define ("RESETPASSWORD_LOGOUT", "Saia para usar sua nova senha.");
            define ("RESETPASSWORD_FAIL", "O reset de senha falhou");
	     define ("KEY_MANAGEMENT_GROUPTITLE", "Configurações de Gerenciamento de Chaves");
			
				define("DATABASE_CURRENT_KEY", "Chave:");
				define("DATABASE_CURRENT_KEY_DEFAULT", "Chave Padrão");
				define("DATABASE_CURRENT_KEY_CUSTOM", "Chave Personalizada");
			define("DATABASE_CURRENT_KEY_NOKEY", "Não Chave");
			
				define("DATABASE_TOC_MODE_WALKUP","Aproximar");
				define("DATABASE_TOC_MODE_TAPTOSEND","Tocar para enviar");
				define("DATABASE_TOC_MODE_PINTOSEND","Pressionar para enviar");
			
			define("DATABASE_PTUPLOAD", "Upload");
         define ("KEY_MANAGEMENT_DEFAULT", "Usar Chave Padrão");
          define ("KEY_MANAGEMENT_CUSTOM", "Usar Chave Personalizada");
	     define ("KEY_MANAGEMENT_BUTTON", "Baixando Chave...");
	     define ("ADDKEY_DIALOG_TITLE", "Adicionar Nova Chave de Encriptação");
	     define ("ADDKEY_DIALOG_MESSAGE", "Digitar detalhes de chave de encriptação abaixo:");
	     define ("ADDKEY_DIALOG_CONTROLS", "<table style=\"width:100%\"><tr style=\"height:2px\" /> <tr><td>Nome do Servidor :</td> <td><input id=\"keyHostName\" type=\"text\" name=\"keyHostName\" style=\"width:120px\" onblur=\"checkHostName()\"></td> <td /></tr> <tr style=\"height:2px\" /> <tr><td>Validity Period : </td><td><input id=\"keyValidPeriod\" type=\"text\" name=\"keyValidity\" style=\"width:120px\" onblur=\"checkValidity()\"></td><td>in days(5 to 3650)</td></tr> <tr style=\"height:2px\" /></table>");						
	     define ("ADDING_NEW_KEY", "Adicionando Nova Chave...");
	     define ("DELETEALLKEY_DIALOG_TITLE", "Apagando todas as chaves de servidor");
	     define ("DELETEALLKEY_DIALOG_MESSAGE", "Tentando apagar todas as chaves de servidor no dispositivo.");
	     define ("DELETING_ALL_KEYS", "Apagando todas as chaves...");
 	     define ("DELETEKEY_DIALOG_TITLE", "Apagar Chave de Servidor");
	     define ("DELETEKEY_DIALOG_MESSAGE", "Tentando apagar chave de servidor no dispositivo.");
    	     define ("DELETING_HOST_KEY", "Apagando Chave de servidor...");
            define ("DOWNLOADING_KEY", "Baixando Chave...");
	     define ("REGENERATEKEY_DIALOG_TITLE", "Regerar Chave ".$NXTText." ");
	     define ("REGENERATEKEY_DIALOG_MESSAGE", "Tentando regerar Chave ".$NXTText." . Isso fará que todas as chaves anteriormente baixadas fiquem inválidas.");
	     define ("REGENERATING_NANO_KEY", "Regerando Chave ".$NXTText." ...");
	     define ("STARTING_EYELOCK_APPLICATION", "Iniciando Aplicação Eyelock...");
            define ("IDENTIFY_DEVICE_TITLE", "Identificando Dispositivo");
            define ("IDENTIFY_DEVICE_MESSAGE", "Piscando repetidamente os LEDs do dispositivo...");
            define ("IDENTIFY_DEVICE_MESSAGE2", "Clique em cancelar para abortar.");
            define ("RESETTING_DEVICE_MESSAGE", "Resetando dispositivo... Por favor, aguarde....");
            define ("REBOOTING_DEVICE_MESSAGE", "Reiniciando dispositivo... Por favor, aguarde...");
            define ("REBOOTING_DEVICE_MESSAGE2", "Isso pode levar um ou dois minutos...");
            define ("WAITING_FOR_EYELOCK_RESTART", "Reiniciando Aplicação Eyelock... Por favor, aguarde...");
            define ("DEVICE_TIME_SYNCHRONIZING", "Sincronizando Hora do Servidor... Por favor, aguarde...");
            define ("DEVICE_TIME_SYNCHRONIZED", "A hora do dispositivo foi ajustada com sucesso!");
            define ("DEVICE_TIME_SYNCFAILED", "Falha ao ajustar a hora do dispositivo!");
            define ("FACTORYRESET_DEVICE", "Resetando dispositivo para configurações de fábrica...");
            define ("RESTORE_DEVICE", "Restaurando backup no dispositivo");
            define ("RESTORE_DEVICE_TITLE", "Restaurar Firmware do dispositivo");
            define ("RESTORE_DEVICE_DELETETITLE", "Apagar Ponto de Recuperação");
            define ("RESTORE_DEVICE_DELETEMSG", "Apagando Ponto(s) de Recuperação do dispositivo...");
			define("RESTORE_FAILED_NOT_SUPPORTED","A restauração deste ponto falhou, versão não é mais suportada");
			define("RESTORE_FAILED_NO_FILE","A restauração deste ponto falhou, não foi possível encontrar ponto de recuperação Nano");
			define("AUTOMATIC_LOGOUT","Agora, você poderá sair!");
			define("LOGOUT_MESSAGE","Saindo... Por favor, aguarde...");
			define("REBOOT__DEVICE_TITLE","Reiniciando dispositivo");
			define("REBOOT_DEVICE_WARNING","Clicando em Sim, o dispositivo será reiniciado...");
			define("MSG_AREYOUSURE","Você deseja continuar?");
			define("OSDP_INSTALL_MODE_SETTINGS", "Setting device to installation mode.");
			define("OSDPINSTALLMODE_TITLE","Confirmar Reset para as configurações de fábrica");
			
			define("FACTORY_RESET_TITLE","Confirmar Reset para as configurações de fábrica");
			define("FACTORY_RESET_WARNING","Esta ação não pode ser desfeita...");
			define("FIRMWARE_UPDATE_NANOTITLE","Processando atualização de Firmware ".$NXTText."... Por favor, aguarde...");
			define("UPGRADE_NOT_ALLOWED","Atualização falhou, versão da atualização não é suportada");
			define("FIRMWARE_UPDATESTATUS_UPLOAD","Carregando pacote para dispositivo...");
			define("FIRMWARE_UPDATESTATUS_DOWNLOAD","Baixando pacote para o servidor:");
			define("FIRMWARE_UPDATESTATUS_UNPACK","Desempacotando arquivos...");
			define("FIRMWARE_UPDATESTATUS_VALIDATING","Validando imagem ".$NXTText."...");
			define("FIRMWARE_UPDATESTATUS_COPYING","Copiando arquivos obrigatórios...");
			define("FIRMWARE_UPDATESTATUS_RESTOREPOINT","Criando Ponto de Recuperação ".$NXTText."...");
			define("FIRMWARE_UPDATESTATUS_UPDATING","Executando atualização do ".$NXTText."...");
			
			define("FIRMWARE_UPDATESTATUS_VALIDATINGBOB","Validando Imagem ICM...");
			define("FIRMWARE_UPDATESTATUS_RESTOREPOINTBOB","Criando Ponto de Recuperação ICM...");
			define("FIRMWARE_UPDATESTATUS_UPDATINGBOB","Executando atualização ICM...");
			define("FIRMWARE_UPDATESTATUS_COMPLETE","Atualização de Firmware completa!");
			define("FIRMWARE_UPDATESTATUS_RESTORESETTINGS","Restaurando Configurações de Dispositivo");
			define("FIRMWARE_UPDATE_TITLE","Resultados da atualização de Firmware");
			define("FIRMWARE_UPDATE_FAILEDTITLE","Atualização de Firmware falhou!");
			define("FIRMWARE_UPDATE_FAILEDMESSAGE","Atualização de Firmware ".$NXTText."  falhou!");
			define("FIRMWARE_UPDATE_SUCCESS","Firmware foi atualizado com sucesso!");
			define("FIRMWARE_UPDATE_RELOAD","Clique OK para reiniciar o dispositivo...");
			define("FIRMWARE_UPDATEERROR_BADFILETYPE","Arquivo carregado não é um pacote de firmware válido!");
			define("FIRMWARE_UPDATEERROR_UNPACKFAILED","Falha ao descompactar pacote de firmware! Pacote pode estar corrompido ou o dispositivo pode estar sem espaço.");
			define("FIRMWARE_UPDATEERROR_VALIDATEFAILED","Falha ao validar o conteúdo do pacote! O conteúdo pode estar corrompido.");
			define("FIRMWARE_UPDATEERROR_RESTOREPOINTFAILED","Falha ao criar ponto de restauração! Dispositivo pode estar sem espaço.");
			define("FIRMWARE_UPDATEERROR_INSTALLFAILED","Falha ao extrair completamente firmware do dispositivo! Dispositivo pode estar sem espaço.");
			define("FIRMWARE_UPDATEERROR_BOBINSTALLFAILED","Falha ao instalar a atualização de firmware!");
			define("FIRMWARE_UPDATEERROR_DEVICERESTOREFAILED","Falha ao restaurar configurações do dispositivo!");
			define("FIRMWARE_UPDATEERROR_SLAVECOPYFAILED","Falha ao copiar arquivos para dispositivo escravo! Dispositivo pode estar sem espaço.");
			define("FIRMWARE_UPDATEERROR_SLAVEINSTALLFAILED","Falha ao atualizar dispositivo escravo!");
			define("FIRMWARE_UPDATEERROR_UNKNOWNFAILED","Falha desconhecida ao atualizar dispositivo!");
			define("DATABASE_DETAILSUNAVAILABLE","Detalhes indisponíveis!");
			define("NANO_DEVICE_STATUSTITLE","Estado de Dispositivo ".$NXTText."");
			define("NANO_DEVICE_CONNDOWN","Não foi possível estabelecer comunicação. Por favor, verifique se o equipamento está ligado.");
			define("NANO_DEVICE_RECONNECT","Por favor, verifique o dispositivo, e então clique em OK para verificar o estado.");


            //Tooltip text
           define("TOOLTIP_LOGIN_installer","O tipo de usuário <b><i>instalador</i></b> permite privilégios de edição total.");
define("TOOLTIP_LOGIN_SITEADMIN","O tipo de usuário <b><i>admin</i></b> permite privilégios de leitura total e escrita limitada.");
define("TOOLTIP_LOGIN_CHANGEPASSWORD","Selecione o tipo de usuário para o qual você deseja mudar a senha.");
define("TOOLTIP_HOME_TAB"," <b><i>Aba Home</i></b> <br>Exibe detalhes básicos do ".$NXTText." . (Informações do dispositivo, informação do Software e Estatísticas de armazenamento de banco de dados)");

           define("TOOLTIP_NETWORK_TAB"," <b><i>Aba Rede</i></b><br>Configuração de parâmetros de rede. (Configurações de DHCP, Endereço IP, Nome de Servidor, etc.)");
			define("TOOLTIP_NETWORK_NAME"," <span style='font-size: 16px; font-style:bold'>Nome do dispositivo</span>
			<ul style='padding-left: 1em'><li>O campo <b><i>Nome</i></b> representa o nome do servidor de ".$NXTText." quando em execução DHCP ou IP Estático na rede.</li>
			<li>Um <i>Nome de Dispositivo</i> válido deve conter somente letras e números e deve ter até 64 caracteres.</li></ul>");
	define("TOOLTIP_NETWORK_DHCP"," <span style='font-size: 16px; font-style:bold'>Obter um endereço IP automaticamente (DHCP)</span>
	<ul style='padding-left: 1em'><li>Esta seleção faz com que o ".$NXTText."  se torne disponível na rede usando o <i>Nome do Dispositivo</i> através do DHCP.</li> 
	                                            <li>Ao utilizar DHCP, o ".$NXTText." pode ser achado na rede digitando o <i>Nome do Dispositivo</i> em vez do endereço IP.</li>
												<li>Algumas redes são configuradas para usar DHCP. Se a rede não está configurada para usar DHCP, a configuração Web pode ser acessada usando IP estático.</li></ul>");
												
define("TOOLTIP_NETWORK_SSLPROTOCOL"," <span style='font-size: 16px; font-style:bold'>Protocolo SSL</span>
<ul style='padding-left: 1em'><li>Selecione o Protocolo SSL que o ".$NXTText." usa para comunicação.</li><li>Padrão:  Usa SSL 3.0</li>
                                             <li>TLS 1.2:  Usa TLS 1.2</li></ul>");
define("TOOLTIP_NETWORK_ADVDHCPBUTTON"," <span style='font-size: 16px; font-style:bold'>Botão de Configurações de DHCP</span>
<ul style='padding-left: 1em'><li>Clique para acessar as configurações avançadas do DHCP.</li></ul>");
define("TOOLTIP_NETWORK_STATICIP"," <span style='font-size: 16px; font-style:bold'>Usar Endereço IP estático seguinte</span>
<ul style='padding-left: 1em'><li>Esta opção faz o ".$NXTText." disponível na rede usando um endereço IP estático específico.</li>
<li>O endereço IP configurado não deve ser usado em qualquer outro lugar da rede.</li>
<li> Se o endereço de IP já está em uso, o ".$NXTText." não estará disponível na rede e o <i>Nome do Dispositivo</i> não aparecerá.</li></ul>");

define("TOOLTIP_NETWORK_DEVICEIP"," <span style='font-size: 16px; font-style:bold'>Dispositivo IP</span>
<ul style='padding-left: 1em'><li>Digite o endereço IP estático aqui.</li>
<li>Consulte o administrador de rede local, se necessário, para determinar a existência de endereços IP estático disponíveis.</li>
<li>Esta configuração só se aplica se <i>Usar Endereço IP estático seguinte</i> estiver marcado acima.</li></ul>");

define("TOOLTIP_NETWORK_BROADCASTNETWORK"," <span style='font-size: 16px; font-style:bold'>Rede Broadcast</span>
<ul style='padding-left: 1em'><li>Consulte o administrador de Rede, se necessário, para determinar o valor apropriado.</li>
                                                <li>Esta configuração só se aplica se <i>Usar Endereço IP estático seguinte</i> estiver marcada acima.</li></ul>");

           
           define("TOOLTIP_NETWORK_SUBNETMASK","<span style='font-size: 16px; font-style:bold'>Máscara de Subrede</span>                                                <ul style='padding-left: 1em'>                                                <li>Consulte o administrador de rede local , se necessário, para determinar o valor apropriado.</li>                                                <li>Essa configuração só se aplica se <i> Use o seguinte endereço IP estático </ i> é selecionado acima.</li></ul>");
			
define("TOOLTIP_NETWORK_DEFAULTGATEWAY"," <span style='font-size: 16px; font-style:bold'>Gateway Padrão</span>
                                                <ul style='padding-left: 1em'>
<li>Consulte o administrador de rede, se necessário, para determinar o valor apropriado.</li>
<li>Esta configuração só se aplica se <i>Usar Endereço IP estático seguinte</i> estiver selecionado acima.</li></ul>");
define("TOOLTIP_NETWORK_DNS"," <span style='font-size: 16px; font-style:bold'>Servidor DNS(Domain Name System)</span>
<ul style='padding-left: 1em'>
<li>Consulte o administrador de rede, se necessário, para determinar o valor apropriado.</li>
<li>Esta configuração só se aplica se <i>Usar Endereço IP estático seguinte</i> estiver selecionado acima.</li></ul>");
            define ("TOOLTIP_NETWORK_ENABLEIEEE8021X", "<span style='font-size: 16px; font-style:bold'>Enable IEEE 802.1X Security</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Consult with the local Network Administrator if necessary to determine the proper value.</li>
                                                <li>This setting enables the IEEE 802.1X network protocol on the ".$NXTText."</li></ul>");
            define ("TOOLTIP_NETWORK_CACERT", "<span style='font-size: 16px; font-style:bold'>CA Certificate</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use the Upload button to the right to browse and upload the CA Certificate file to the device.</li></ul>");
            define ("TOOLTIP_NETWORK_UPLOADCACERTIFICATE", "<span style='font-size: 16px; font-style:bold'>Upload CA Certificate</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use this button to browse and upload the CA Certificate file to the device.</li></ul>");
            define ("TOOLTIP_NETWORK_CLIENTCERT",  "<span style='font-size: 16px; font-style:bold'>Client Certificate</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use the Upload button to the right to browse and upload the Client Certificate file to the device.</li></ul>");
            define ("TOOLTIP_NETWORK_UPLOADCLIENTCERTIFICATE", "<span style='font-size: 16px; font-style:bold'>Upload Client Certificate</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use this button to browse and upload the Client Certificate file to the device.</li></ul>"); 
            define ("TOOLTIP_NETWORK_CLIENTPRIVATEKEY",  "<span style='font-size: 16px; font-style:bold'>Client Private Key</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use the Upload button to the right to browse and upload the Client Private Key file to the device.</li>
                                                <li>It is also acceptable to upload a combined client key and password (PEM) file.</li></ul>");
            define ("TOOLTIP_NETWORK_UPLOADPRIVATEKEY", "<span style='font-size: 16px; font-style:bold'>Upload Client Certificate</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use this button to browse and upload the Client Private Key file to the device.</li>
                                                <li>It is also acceptable to upload a combined client key and password (PEM) file.</li></ul>"); 
            define ("TOOLTIP_NETWORK_EAPOLVERSION", "<span style='font-size: 16px; font-style:bold'>EAPOL Version</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Use the dropdown to specify the version of the EAPOL protocol to use.</li></ul>"); 
            define ("TOOLTIP_NETWORK_EAPIDENTITY", "<span style='font-size: 16px; font-style:bold'>EAP Identity</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Specify your EAP Indentity.</li></ul>"); 
            define ("TOOLTIP_NETWORK_PRIVTEKEYPWD",  "<span style='font-size: 16px; font-style:bold'>Private Key Pasword</span>
                                                <ul style='padding-left: 1em'>
                                                <li>Specify your Private Key Password.</li></ul>"); 

define ("TOOLTIP_NETWORK_DOWNLOADLOGBUTTON", "<span style='font-size: 16px; font-style:bold'>Download 802.1X Log Button</span>
                                                <ul style='padding-left: 1em'><li>Click to download the ".$NXTText." <i>IEEE 802.1X Log</i> as a text file.</li></ul>");

     

            
           



            //////////////////////////////////////////////////////
            // ADV. DHCP DIALOG HELP STRINGS
            //////////////////////////////////////////////////////
			
			define("TOOLTIP_ADVDHCP_TIMEOUT"," <span style='font-size: 16px; font-style:bold'>DHCP Timeout</span>
                                                <ul style='padding-left: 1em'><li>Especifica a duração em segundos que o cliente ".$NXTText." DHCP irá tentar resolver o <i>nome de servidor</i> ".$NXTText." antes de falhar.</li>
                                                <li>Variação de duração (segundos): 10 – 120.</li>
                                                <li>Valor padrão: 10.</li></ul>");
			define("TOOLTIP_ADVDHCP_RETRIES"," <span style='font-size: 16px; font-style:bold'>Retentativas DHCP</span>
                                                <ul style='padding-left: 1em'><li>Especifica o número total que o cliente ".$NXTText." DHCP irá tentar resolver o <i>nome do servidor</i> antes da falha acontecer.</li>
                                                <li>Retentativas (número de vezes) : 0 – 5</li>
                                                <li>Valor padrão: 0.</li></ul>");
			define("TOOLTIP_ADVDHCP_RETRYDELAY"," <span style='font-size: 16px; font-style:bold'>Atraso de Retentativa</span>
                                                <ul style='padding-left: 1em'><li>Especifica o atraso em segundos entre cada nova tentativa do cliente DHCP.</li>
                                                <li>Variação de duração (segundos): 0 – 60.</li>
                                                <li>Valor padrão: 10.</li></ul>");
            define ("TOOLTIP_IPV6_ENABLE", "<span class='tooltip-header'>IPV6 Ativo</span>");
            define ("TOOLTIP_IPV6_CONFIGURATION", "<span class='tooltip-header'>Especificar detalhes da configuração do IPv6</span>");
            define ("TOOLTIP_IPV6_DHCP_MODE", "<span class='tooltip-header'>Modo DHCP IPv6:</span>
                                                <ul class='tooltip-list'>
                                                <li><b>Sem estado</b> – O DHCPv6 é usado para definir servidores DNS e servidores NTP, etc., mas não para definir endereços IPv6</li>
                                                <li><b>Stateful</b> - O DHCPv6 é usado para definir endereços IPv6, servidores DNS, etc.</li>
                                                <li><b>Auto</b> – O uso de DHCPv6 é determinado pelos anúncios do roteador</li>
                                                <li><b>Off</b> - O DHCPv6 está desativado.</li>
                                                </ul>");
            define ("TOOLTIP_IPV6_ADDRESS", "<span class='tooltip-header'>Endereço de rede IPv6</span>");
            define ("TOOLTIP_IPV6_GATEWAY", "<span class='tooltip-header'>Endereço de gateway padrão do IPv6</span>");
            define ("TOOLTIP_IPV6_DNS1", "<span class='tooltip-header'>Endereço do servidor DNS primário IPv6</span>");
            define ("TOOLTIP_IPV6_DNS2", "<span class='tooltip-header'>Endereço do servidor DNS IPv6 secundário</span>");
			define ("TOOLTIP_IPV6_STATUS", "<span class='tooltip-header'>Current IPv6 addresses assigned to the EXT device</span>
							<ul class='tooltip-list'>
							<li><b>Global</b> – addresses that are globally valid.</li>
							<li><b>Link-local</b> - addresses that are valid only for communications within the network segment or the broadcast domain that the EXT device is connected to.</li>
							<li><b>Stateless</b> – addresses installed due to stateless address configuration.</li>
							<li><b>Temporary</b> - temporary random addresses (documented in RFC 4941).</li>
							</ul>");
			define("TOOLTIP_DEVICE_TAB"," <b><i>Aba Dispositivo</i></b><br>Configuração de dispositivo ".$NXTText." . (Brilho de LED, Controle de Volume, Configuração de Hora, Reboot e Reset de Fábrica)");
			define("TOOLTIP_DEVICE_SPEAKERVOLUME"," <span style='font-size: 16px; font-style:bold'>Volume Alto-falante</span>
                                                <ul style='padding-left: 1em'><li>Ajuste de volume do alto-falante ".$NXTText.".</li>
                                                <li>Especifique um valor '0' para deixar o alto-falante mudo.</li>
                                                <li>Variação de Volume: 0 – 100.</li>
                                                <li>Valor padrão: 40.</li></ul>");
			define("TOOLTIP_DEVICE_LEDBRIGHTNESS"," <span style='font-size: 16px; font-style:bold'>Brilho do LED</span>
                                                <ul style='padding-left: 1em'><li>Ajusta o nível de brilho do LED do the ".$NXTText.".</li>
                                                <li>Especifique valor '0' para desligar os LEDs.</li>
                                                <li>Variação do Brilho: 0 – 100.</li>
                                                <li>Valor padrão: 20.</li></ul>");
			define("TOOLTIP_DEVICE_TAMPERVOLUME"," <span style='font-size: 16px; font-style:bold'>Volume do tom de adulteração</span>
                                                <ul style='padding-left: 1em'><li>Ajusta o nível do volume do <i>Alarme de Adulteração</i>.</li>
                                                <li>Especifique um valor de '0' para silenciar o alarme.</li>
                                                <li>Variação do Volume: 0 – 100.</li>
                                                <li>Valor padrão: 10.</li></ul>");
			define("TOOLTIP_DEVICE_TIMESERVER"," <span style='font-size: 16px; font-style:bold'>Servidor de Horário da Internet</span>
                                                <ul style='padding-left: 1em'><li>O Endereço Web do <i>Servidor de Horário da Internet</i> do qual recuperar o horário atual.</li>
                                                <li>Observe que o ".$NXTText." <b><i>DEVE</i></b> ter acesso ao <i>Servidor de Horário da Internet</i> para esta função funcionar.</li>
                                                <li>Valor padrão: time.nist.gov.</li></ul>");
			define("TOOLTIP_DEVICE_SYNCHRONIZEDAILY"," <span style='font-size: 16px; font-style:bold'>Synchronizar Horário Diariamente</span>
                                                <ul style='padding-left: 1em'><li>Esta opção permite que o ".$NXTText." sincronize com o <i>Servido de Horário da Internet</i> especificado uma vez por dia.</li>
                                                <li>Valor padrão: Ativado.</li></ul>");
			define("TOOLTIP_DEVICE_LOCATEDEVICE"," <span style='font-size: 16px; font-style:bold'>Botão Localizar Dispositivo</span>
                                                <ul style='padding-left: 1em'><li>Clique para piscar os LEDs do ".$NXTText.".</li>
                                                <li>Os LEDs continuarão a piscar até que o usuário cancele a operação.</li></ul>");
			define("TOOLTIP_DEVICE_SYNCHRONIZENOW"," <span style='font-size: 16px; font-style:bold'>Botão Sincronizar Agora!</span>
                                                <ul style='padding-left: 1em'><li>Clique para sincronizar imediatamente o horário do ".$NXTText." com o <i>Servidor de Horário da Internet</i> especificado.</li></ul>");
			define("TOOLTIP_DEVICE_SYNCHRONIZEWITHHOST"," <span style='font-size: 16px; font-style:bold'>Botão Sincronizar com Servidor</span>
                                                <ul style='padding-left: 1em'><li>Clique para sincronizar imediatamente o horário do ".$NXTText." com o horário do sistema/servidor atual.</li></ul>");
			define("TOOLTIP_DEVICE_FACTORYRESET"," <span style='font-size: 16px; font-style:bold'>Botão Reset de Fábrica</span>
                                                <ul style='padding-left: 1em'><li>Clique para resetar ".$NXTText." para suas configurações de <i>Fábrica</i>.</li>
                                                <li>Ao resetar o ".$NXTText." para suas configurações de fábrica:</li><ol>                                                <li>Todas as configurações são reiniciadas para os valores padrões.</li>
                                                <li>Configurações de Rede são revertidas para o padrão (DHCP).</li>
                                                <li>A base de dados do ".$NXTText." será removida</li>
                                                <li>Os arquivos de relatório armazenados no ".$NXTText." serão apagados</li>
                                                <li>O ".$NXTText." reinicia.</li>                                                </ol></ul>");
			 define ("TOOLTIP_ACS_INSTALLATION_MODE", "<span style='font-size: 16px; font-style:bold'>Installation Mode</span>
                                                <ul style='padding-left: 1em'><li>Click to set ".$NXTText." to OSDP installation mode.</li>
                                                <li>When in this mode the ".$NXTText." can receive Secure Mode OSDP SCBK from the Control Panel. </li><ol>
                                                <li>This mode is required to use Secure Mode OSDP with a new Control Panel or during a new install.</li>
												<li>After confirming this option currently stored SCBK is deleted.  <b>THIS OPERATION IS NOT REVERSIBLE</b></li>
                                               
                                                </ol></ul>");
			
			
			define("TOOLTIP_DEVICE_REBOOTDEVICE"," <span style='font-size: 16px; font-style:bold'>Botão Reiniciar</span>
                                                <ul style='padding-left: 1em'><li>Clique para reiniciar o ".$NXTText.".</li>
                                                <li>Reiniciar fará com que o WebConfig seja desconectado do ".$NXTText.".</li>
                                                <li>Aguarde um tempo para o WebConfig reconectar ao ".$NXTText." enquanto o processo de reinício avança.</li>
                                                <li>Se o WebConfig falhar ao reconectar automaticamente em até 2 minutos, atualize apágina do navegador e depois verifique se o nano está em execução.</li></ul>");
			define("TOOLTIP_SECURITY_TAB"," <b><i>Aba Segurança</i></b><br>Configuração de Segurança do ".$NXTText." . (Gerenciamento de Chave, Reiniciar Chave e Configurações de Adulteração)");
			define("TOOLTIP_SECURITY_TAMPERACTIVEHIGH"," <span style='font-size: 16px; font-style:bold'>Ativar Adulteração em Sinal Alto</span>
                                                <ul style='padding-left: 1em'><li>Se um leitor de cartão está ligado ao sistema por autenticação dupla, esta configuração deve corresponder a saída de adulteração para o leitor de cartão.</li>
                                                <li>'Ativar Estado de Adulteração em Sinal Alto' deve estar selecionado se a condição de adulteração do leitor de cartão é ativada em um sinal alto.</li></ul>");
			define("TOOLTIP_SECURITY_TAMPERACTIVELOW"," <span style='font-size: 16px; font-style:bold'>Ativar Adulteração em Sinal Baixo</span>
                                                <ul style='padding-left: 1em'><li>Se um leitor de cartão está ligado ao sistema por autenticação dupla, esta configuração deve corresponder a saída de adulteração para o leitor de cartão.</li>
                                                <li>'Ativar Estado de Adulteração em Sinal Baixo' deve estar selecionado se a condição de adulteração do leitor de cartão é ativada em um sinal baixo.</li></ul>");
			define("TOOLTIP_OUTPUT_SECURITY_TAMPERACTIVEHIGH"," <span style='font-size: 16px; font-style:bold'>Sinal Alto</span>
                                                <ul style='padding-left: 1em'><li>Define uma saída ALTA, se a adulteração é detectada.</li></ul>");
			define("TOOLTIP_OUTPUT_SECURITY_TAMPERACTIVELOW"," <span style='font-size: 16px; font-style:bold'>Sinal Baixo</span>
                                                <ul style='padding-left: 1em'><li>Define uma saída BAIXO, se a adulteração é detectada.</li></ul>");
			define("TOOLTIP_SECURITY_TAMPERNOTIFYADDR"," <span style='font-size: 16px; font-style:bold'>Endereço de Notificação</span>
                                                <ul style='padding-left: 1em'><li>Indica o endereço IP do sistema/aplicação que receberá a <i>Mensagem de Adulteração</i></li></ul>");
			define("TOOLTIP_SECURITY_TAMPERNOTIFYPORT"," <span style='font-size: 16px; font-style:bold'>Porta de Notificação</span>
                                                <ul style='padding-left: 1em'><li>Indica a Porta do sistema/aplicação que receberá a <i>Mensagem de Adulteração</i></li></ul>");
			define("TOOLTIP_SECURITY_TAMPERMESSAGE"," <span style='font-size: 16px; font-style:bold'>Mensagem de Adulteração</span>
                                                <ul style='padding-left: 1em'><li>Indica a mensagem que será enviada para o local descrito acima quando ocorrer uma adulteração no ".$NXTText.".</li></ul>");
			define("TOOLTIP_SECURITY_OLDPWD"," <span style='font-size: 16px; font-style:bold'>Senha atual</span>
                                                <ul style='padding-left: 1em'><li>Digite senha de entrada existente no ".$NXTText." neste campo.</li></ul>");
			define("TOOLTIP_SECURITY_NEWPWD"," <span style='font-size: 16px; font-style:bold'>Senha nova</span>
                                                <ul style='padding-left: 1em'><li>Digite a nova senha de entrada desejada do ".$NXTText." neste campo.</li></ul>");
			define("TOOLTIP_SECURITY_CONFIRMPWD"," <span style='font-size: 16px; font-style:bold'>Confirmação de Senha</span>
                                                <ul style='padding-left: 1em'><li>Digite novamente a senha de entrada desejada para o ".$NXTText." neste campo.</li></ul>");

define("TOOLTIP_SECURITY_UPDATEPWDBUTTON"," <span style='font-size: 16px; font-style:bold'>Botão Reiniciar Senha</span>
                                                <ul style='padding-left: 1em'><li>Clique para reiniciar senha do ".$NXTText.".</li>
                                                <li>Depois que a senha é reiniciada, o Web Config redireciona para a tela de Entrada. Digite a noca senha para entrar.</li>                                               </ul>");
define("TOOLTIP_SECURITY_NANONXT_TEXT","Dispositivo ".$NXTText." ");
define("TOOLTIP_SECURITY_PC_TEXT","Sistema Servidor");
define("TOOLTIP_SECURITY_VALIDKEY_TEXT","Chave Válida");
define("TOOLTIP_SECURITY_INVALIDKEY_TEXT","Chave inválida/expirada!");
define("TOOLTIP_SECURITY_REGENERATEKEY_TEXT","Regerar Chave");
define("TOOLTIP_SECURITY_DELETEKEY_TEXT","Apagar Chave");
define("TOOLTIP_SECURITY_DOWNLOADKEY_TEXT","Baixar Chave");
define("TOOLTIP_SECURITY_KEYMGMTBUTTON"," <span style='font-size: 16px; font-style:bold'>Baixar Chave</span>
                                                <ul style='padding-left: 1em'><li>Baixa a chave personalizada para este computador.</li>
                                                <li>A chave deve ser baixada a cada vez que o ".$NXTText." é alterado de Usar Chave Padrão para Usar Chave Personalizada.</ul>");
define("TOOLTIP_SECURITY_ADDHOSTKEYBUTTON"," <span style='font-size: 16px; font-style:bold'>Adicionar Chave Personalizada</span>
                                                <ul style='padding-left: 1em'><li>Clique para adicionar uma nova chave personalizada ao ".$NXTText.".</li></ul>");
define("TOOLTIP_SECURITY_DELETEALLHOSTKEYSBUTTON"," <span style='font-size: 16px; font-style:bold'>Apagar Chave Personalizada</span>
                                                <ul style='padding-left: 1em'><li>Clique para apagar a chave personalizada presente no ".$NXTText.".</li></ul>");
define("TOOLTIP_SECURITY_KEYMGMTDEFAULTRADIO"," <span style='font-size: 16px; font-style:bold'>Usar Chave Padrão</span>
                                                <ul style='padding-left: 1em'><li>Use a chave padrão fornecida com os produtos Eyelock para falar com o ".$NXTText.".</li>
                                                <li><a id='DefaultKeyExpiry'>Chave Padrão expira em 19-03-2025</a></li></ul>");
define("TOOLTIP_SECURITY_KEYMGMTCUSTOMRADIO"," <span style='font-size: 16px; font-style:bold'>Usar Chave Personalizada</span>
                                                <ul style='padding-left: 1em'><li>O ".$NXTText." irá gerar uma chave personalizada para usá-la na comunicação com ele. Baixe a chave para atribuí-la nas aplicações Eyelock.</li></ul>");
define("TOOLTIP_SECURITY_KEYMGMTVALIDITY"," <span style='font-size: 16px; font-style:bold'>Chave Expira Em...</span>
                                                <ul style='padding-left: 1em'><li>A chave personalizada é válida até a data e hora exibidas.</li></ul>");
define("TOOLTIP_SOFTWARE_TAB"," <b><i>Aba Software</i></b><br>Configura o Firmware ".$NXTText." . (Atualizar Firmware, Restaurar Firmware)");
define("TOOLTIP_SOFTWARE_UPDATENOWBUTTON"," <span style='font-size: 16px; font-style:bold'>Botão Atualizar Agora!</span>
                                                <ul style='padding-left: 1em'><li>Este botão aparece se uma versão mais recente do firmware do ".$NXTText." estiver disponível no website Eyelock.com.</li>
                                                <li>Clique para iniciar o processo automático de atualização do software.</li></ul>");
define("TOOLTIP_SOFTWARE_LOCALFILEBUTTON"," <span style='font-size: 16px; font-style:bold'>Botão Arquivo Local...</span>
                                                <ul style='padding-left: 1em'><li>Clicar neste botão permite que você atualize o firmware do ".$NXTText." através de um arquivo de pacotes de atualização do Eyelock que esteja acessível localmente.</li>
                                                <li>Após clicar no botão, selecione o arquivo de pacotes para iniciar o processo de atualização.</li>
                                                <li><strong>CUIDADO: Não desligue o ".$NXTText.", desconecte a rede da sua estação de trabalho ou feche esta janela do navegador durante a atualização.</strong></li></ul>");
define("TOOLTIP_SOFTWARE_DETAILSBUTTON"," <span style='font-size: 16px; font-style:bold'>Botão Detalhes...</span>
                                                <ul style='padding-left: 1em'><li>Clicar neste botão exibirá uma caixa de diálogo de atualização do ".$NXTText." contendo detalhes do arquivo de pacotes de atualização disponível atualmente.</li>
                                                <li>Inicie a atualização da Tela de Detalhes.</li></ul>");
define("TOOLTIP_SOFTWARE_ALLOWSITEADMINUPDATE"," <span style='font-size: 16px; font-style:bold'>Permitir que o admin realize atualizações</span>
                                                <ul style='padding-left: 1em'><li>Ative esta opção para permitir um usuário que esteja conectado como <i>admin</i> faça a atualização do ".$NXTText.".</li>
                                                <li>Esta opção somente será visível para o <i>instalador</i>.</li></ul>");
define("TOOLTIP_SOFTWARE_NANORESTORE"," <span style='font-size: 16px; font-style:bold'>Pontos de Restauração do ".$NXTText."</span>
                                                <ul style='padding-left: 1em'><li>Selecione esta opção para exibir uma lista de Pontos de Restauração de firmware disponíveis para o ".$NXTText.".</li>
                                                <li>Pontos de Restauração podem ser restaurados ou deletados ao selecionar o checkbox correspondente a eles e, em seguida, clicando nos botões <i>Restaurar...</i> ou <i>Apagar...</i>.</li></ul>");
define("TOOLTIP_SOFTWARE_BOBRESTORE"," <span style='font-size: 16px; font-style:bold'>Pontos de Restauração do ".$NXTText." Break Out Board</span>
                                                <ul style='padding-left: 1em'><li>Selecione esta opção para exibir uma lista de Pontos de Restauração de firmware disponíveis para o ".$NXTText.".</li>
                                                <li>Pontos de Restauração podem ser restaurados ou deletados ao selecionar o checkbox correspondente a eles e, em seguida, clicando nos botões <i>Restaurar...</i> ou <i>Apagar...</i>.</li></ul>");
define("TOOLTIP_SOFTWARE_RESTOREBUTTON"," <span style='font-size: 16px; font-style:bold'>Botão Restaurar Pontos de Restauração</span>
                                                <ul style='padding-left: 1em'><li>Clique para reverter o firmware do ".$NXTText." de volta a um Ponto de Restauração prévio.</li>
                                                <li>Somente o primeiro Ponto de Restauração marcado será restaurado..</li>
                                                <li>A restauração é um processo automático e, uma vez iniciado, não pode ser parado..</li>
                                                <li>O ".$NXTText." irá reter até cinco (5) pontos de restauração.</li></ul>");
define("TOOLTIP_SOFTWARE_DELETEBUTTON"," <span style='font-size: 16px; font-style:bold'>Botão Apagar Pontos de Restauração</span>
                                                <ul style='padding-left: 1em'><li>Clicar neste botão irá apagar permanentemente todos os Pontos de Restauração marcados do ".$NXTText.".</li>
                                                <li>O Ponto de Restauração apagado não poderá ser recuperado.</li></ul>");



define("TOOLTIP_SOFTWARE_PTLOCALFILEBUTTON","<span style='font-size: 16px; font-style:bold'>Arquivo local... Gerenciamento de Firmware de Leitor de Modelo Portátil</span>
			<ul style='padding-left: 1em'><li>Clicar neste botão permite você a atualizar o firmware do Leitor de Modelo Portátil a partir de um imagem (arquivo .bin) de firmware acessível localmente.</li>
			<li>Após clicar no botão, seleciona o arquivo imagem para começar o processo de atualização.</li>
			<li><strong>CUIDADO: Não desligue o Nano, desconecte a rede de sua estação de trabalho ou feche esta janela do navegador durente a atualização.</strong></li></ul>");


       
         
		
define("TOOLTIP_AUTHENTICATION_TAB"," <b><i>Authentication Tab</i></b><br>Configuração dos parâmetros de autenticação do ".$NXTText." . (Modo de Correspondência, Tempo de Repetição de Autorização, Tempo de Expiração de Correspondência Negativa etc.)");
define("TOOLTIP_AUTHENTICATION_SINGLEEYE"," <span style='font-size: 16px; font-style:bold'>Autenticação de Um Olho</span>
                                                <ul style='padding-left: 1em'><li>Selecione esta opção para <i>permitir</i> uma correspondência biométrica de um olho para uma autenticação bem sucedida.</li></ul>");
define("TOOLTIP_AUTHENTICATION_DUALEYE"," <span style='font-size: 16px; font-style:bold'>Autenticação de Dois Olhos</span>
                                                <ul style='padding-left: 1em'><li>Selecione esta opção para <i>exigir</i> uma correspondência de ambos os olhos para uma autenticação bem sucedida.</li></ul>");
define("TOOLTIP_AUTHENTICATION_REPEATPERIOD"," <span style='font-size: 16px; font-style:bold'>Tempo de Repetição de Autorização</span>
                                                <ul style='padding-left: 1em'><li>A quantidade de tempo <i>mínima</i> que deve passar entre autenticações bem-sucedidads. Isto pode ter um mínimo de 2 segundos para manter a qualidade da imagem.</li>
                                                <li>Faixa em segundos: 2-60.</li>
                                                <li>Valor padrão: 4.</li></ul>");
define("TOOLTIP_AUTHENTICATION_ENABLENEGMATCHTIMEOUT"," <span style='font-size: 16px; font-style:bold'>Ativar Tempo de Expiração para Correspondência Negativa</span>
                                                <ul style='padding-left: 1em'><li>Selecione esta opção para ativar um processamento de regras de tempo de expiração para <i>Correspondência Negativa</i>.</li></ul>");
define("TOOLTIP_AUTHENTICATION_LOITERPERIOD"," <span style='font-size: 16px; font-style:bold'>Tempo de Atraso</span>
                                                <ul style='padding-left: 1em'><li>Define a duração <i>máxima</i> em segundos que o ".$NXTText." tentará autenticar o usuário antes do tempo expirar e reportar uma correspondência negativa.</li>
                                                <li>Faixa em segundos: 2-60.</li>
                                                <li>Valor padrão: 6.</li></ul>");
define("TOOLTIP_AUTHENTICATION_NEGMATCHRESET"," <span style='font-size: 16px; font-style:bold'>Temporizador de Reinício de Correspondência Negativa</span>
                                                <ul style='padding-left: 1em'><li>Define a duração <i>mínima</i> em segundos que deve se pssar depois de uma correspondência negativa antes do ".$NXTText." tentar outra autenticação.</li>
                                                <li>Faixa em segundos: 2-60.</li>
                                                <li>Valor Padrão: 4.</li></ul>");
define("TOOLTIP_AUTHENTICATION_NETWORKMSGADDR"," <span style='font-size: 16px; font-style:bold'>Endereço IP de Destino das Mensagens de Rede</span>
                                                <ul style='padding-left: 1em'><li>Indica o endereço IP do sistema/aplicação que receberá a mensagem de <i>Correspondência de Rede</i>.</li></ul>");
define("TOOLTIP_AUTHENTICATION_NETWORKMSGPORT"," <span style='font-size: 16px; font-style:bold'>Porta de Destino das Mensagens de Rede</span>
                                                <ul style='padding-left: 1em'><li>Indica a porta do sistema/aplicação que receberá a mensagem de <i>Correspondência de Rede</i>.</li></ul>");
define("TOOLTIP_AUTHENTICATION_NETWORKMSGMSG"," <span style='font-size: 16px; font-style:bold'>Formato de Mensagem de Rede</span>
                                                <ul style='padding-left: 1em'><li>Indica a string de formato para a mensagem que será enviada ao local descrito acima quando uma autenticação/correspondência bem-sucedida ocorrer.</li>
                                                <li>Valor padrão: Correspondidos:%d;Contagem:%0.4f;Tempo:%llu;ID:</li></ul>");
define ("TOOLTIP_AUTHENTICATION_IRISPROCESSINGMODE", "<span style='font-size: 16px; font-style:bold'>Iris Processing Mode</span>
                                    <ul style='padding-left: 1em'><li>Specify the mode for processing irises.  There are two options:</li>
                                    <li>Access Control Authentication - Uses matching feature to authenticate user irises and, optionally, interface with ACS systems.</li>
                                    <li>Iris Capture Mode - Capture irises and send them to a remote HTTP Endpoint.  No matching or ACS integration occurs.</li></ul>");
define ("TOOLTIP_AUTHENTICATION_CAPTURETIMEOUT", "<span style='font-size: 16px; font-style:bold'>Iris Capture Timeout</span>
                                    <ul style='padding-left: 1em'><li>Specify the during in milliseconds for processing user irises until sending the message to the HTTP endpoint.</li>
                                    <li>Range (Milliseconds): 1000-60000.</li>
                                    <li>Default value: 5000.</li></ul>");
define ("TOOLTIP_AUTHENTICATION_CAPTURERESETDELAY", "<span style='font-size: 16px; font-style:bold'>Iris Capture Reset Delay</span>
                                    <ul style='padding-left: 1em'><li>Specify the delay in milliseconds that the device will wait after capturing irises before processing the next user.</li>
                                    <li>Range (Milliseconds): 1000-300000.</li>
                                    <li>Default value: 5000.</li></ul>");
define ("TOOLTIP_AUTHENTICATION_MESSAGEFORMAT", "<span style='font-size: 16px; font-style:bold'>HTTP Message Format</span>
                                    <ul style='padding-left: 1em'><li>Specify the format of the message which will be sent to the HTTP Endpoint.</li>
                                    <li>Default value: SOAP (see documentation for schema of SOAP message)</li>
                                    <li>Alertnate value: JSON (see documentation for schema of JSON message)</li></ul>");
define ("TOOLTIP_AUTHENTICATION_PAYLOADFORMAT", "<span style='font-size: 16px; font-style:bold'>Message Image Payload Format</span>
                                    <ul style='padding-left: 1em'><li>Specify the graphical format of the iris images which will be sent to the HTTP Endpoint.</li>
                                    <li>Option 1: RAW - 8-bit, Single Channel image</li>
                                    <li>Option 2: PNG</li>
                                    <li>Option 2: J2K (JPEG 2000)</li>
                                    <li>Default value: RAW - 8-bit, Single Channel image</li></ul>");
define ("TOOLTIP_AUTHENTICATION_IRISIMAGEQUALITY", "<span style='font-size: 16px; font-style:bold'>Iris Image Quality</span>
                                    <ul style='padding-left: 1em'><li>Specify the quality of the encoded JPEG 2000 iris image.  A value of 100 uses lossless compression.</li>
                                    <li>Default value: 100</li></ul>");
define ("TOOLTIP_IRISCAPTURE_WECOMEMSG", "<span style='font-size: 16px; font-style:bold'>Welcome Message</span>
                                    <ul style='padding-left: 1em'><li>Define the text string sent as part of the Iris Capture message.</li></ul>");
define ("TOOLTIP_IRISCAPTURE_DEVICELOCATION", "<span style='font-size: 16px; font-style:bold'>Device Location</span>
                                    <ul style='padding-left: 1em'><li>Define a unique string to associate with this device in the Iris Capture message.</li></ul>");
define ("TOOLTIP_IRISCAPTURE_HEARTBEATURL", "<span style='font-size: 16px; font-style:bold'>Heartbeat URL</span>
                                    <ul style='padding-left: 1em'><li>Define the URL to which this device will send heartbeat messages.    Leave empty to not send heartbeatmessages.</li></ul>");
define ("TOOLTIP_IRISCAPTURE_POSTSENDERURL", "<span style='font-size: 16px; font-style:bold'>Post Destination Address</span>
                                    <ul style='padding-left: 1em'><li>The base URL to which all Iris Capture messages will be sent..</li></ul>");
define ("TOOLTIP_IRISCAPTURE_ERRORURL", "<span style='font-size: 16px; font-style:bold'>Signal Error Endpoint</span>
                                    <ul style='padding-left: 1em'><li>The endpiont under the Post Destination Address to which signal error messages will be sent.  Leave empty to not send messages.</li></ul>");
define ("TOOLTIP_IRISCAPTURE_POSTIRISURL", "<span style='font-size: 16px; font-style:bold'>Post Iris Endpoint</span>
                                    <ul style='padding-left: 1em'><li>The endpiont under the Post Destination Address to which iris images will be sent..</li></ul>");
define ("TOOLTIP_IRISCAPTURE_MAINTENANCEURL", "<span style='font-size: 16px; font-style:bold'>Maintenance Endpoint</span>
                                    <ul style='padding-left: 1em'><li>The endpiont under the Post Destination Address to which maintenance messages will be sent.  Leave empty to not send messages.</li></ul>");

		



          
      
  define("TOOLTIP_DATABASE_TAB"," <b><i>Aba Base de Dados</i></b><br>Configuração de Base de Dados do ".$NXTText." . (Combinador da Rede e Estatísticas de Base de Dados)");
define("TOOLTIP_DATABASE_ENABLENETWORKMATCHER"," <span style='font-size: 16px; font-style:bold'>Ativar Combinador da Rede</span>
                                                <ul style='padding-left: 1em'><li>Selecione esta opção para ativar um processamento adicional, fora do dispositivo de dados biométricos usando o <i>Serviço Combinador da Rede Eyelock</i>.</li></ul>");
define("TOOLTIP_DATABASE_NETWORKMATCHERADDR"," <span style='font-size: 16px; font-style:bold'>Endereço Destinatário de Combinador da Rede</span>
                                                <ul style='padding-left: 1em'><li>Indica o Endereço IP do Serviço Combinador da Rede Eyelock que irá processar as requisiçoes de autenticação.</li></ul>");
define("TOOLTIP_DATABASE_NETWORKMATCHERPORT"," <span style='font-size: 16px; font-style:bold'>Porta de Destinatário de Combinador da Rede</span>
                                                <ul style='padding-left: 1em'><li>Indica a Porta do Serviço Combinador da Rede Eyelock que irá processar as requisiçoes de autenticação.</li></ul>");
define("TOOLTIP_AUTHENTICATION_SECURENETWORK"," <span style='font-size: 16px; font-style:bold'>Ativar Mensagem de Correspondência de Rede Segura</span>
                                                <ul style='padding-left: 1em'><li>Ative esta opção para enviar mensagens de correspondência pela rede de forma segura.</li></ul>");
define("TOOLTIP_DATABASE_STATISTICS"," <span style='font-size: 16px; font-style:bold'>Espaço de Template Restante</span>
                                                <ul style='padding-left: 1em'><li>Este valor indica a quantidade de templates que ainda podem ser armazenados na base de dados do ".$NXTText.".</li></ul>");
define("TOOLTIP_DATABASE_NETWORKMATCHER_SECURECOMM"," <span style='font-size: 16px; font-style:bold'>Ativar Combinador em Rede Segura</span>
                                                <ul style='padding-left: 1em'><li>Ative esta opção para se comunicar de forma segura com o <i>Serviço Combinador em Rede Segura do Eyelock</i>.</li></ul>");
 



         
           define("TOOLTIP_UPLOAD_PTFIRMWARE","Carregar um arquivo (.bin) Firmware de Leitor de Modelo Portátil.");
  
			
			define("TOOLTIP_ACS_TAB"," <b><i>Aba ACS</i></b><br>Configuração do ".$NXTText."  para o Sistema ACS/Door Panel.(Protocolo, Controle de LED ACS, Autenticação Dupla, Retransmissões e Opção de Testes ACS)");
define("TOOLTIP_LOGS_TAB"," <b><i>Aba Logs</i></b><br>Exibe o logo do ".$NXTText."  e permite que o usuário baixe o arquivo de log.");
define("TOOLTIP_LOGS_REFRESHBUTTON"," <span style='font-size: 16px; font-style:bold'>Botão Atualizar</span>
                                                <ul style='padding-left: 1em'><li>Clicar neste botão irá sincronizar o <i>Relatório de Eventos</i> com o conteúdo atual do relatório.</li>
                                                <li>O <i>Relatório de Eventos</i> é sincronizado quando o WebConfig é executado inicialmente. A atualização do relatório não é contínua.</li></ul>");
define("TOOLTIP_LOGS_DOWNLOADBUTTON"," <span style='font-size: 16px; font-style:bold'>Botão Baixar</span>
                                                <ul style='padding-left: 1em'><li>Clique para baixar o <i>Event Log</i> do ".$NXTText." como um arquivo Microsoft Excel CSV.</li></ul>");
define("TOOLTIP_ACCESSCONTROLTYPE"," <span style='font-size: 16px; font-style:bold'>Esquema de Autenticação</span>
                                                <ul style='padding-left: 1em'><li>Selecione o esquema de autenticação que o ".$NXTText." utilizará</li>
                                                <li>Esquemas</li>
                                                <ul><li>Somente Íris: o ".$NXTText." enviará somente a autenticação da íris para o Sistema de Acesso</li>
                                                <li>Íris Ou Cartão: o ".$NXTText." enviará ou uma credencial de cartão ou uma autenticação de íris para o Sistema de Acesso</li>
                                                <li>Íris E Cartão: o ".$NXTText." irá requerer tanto uma credencial de cartão quanto uma autenticação de íris (que corresponde ao cartão apresentado)</li>
                                                <li>Iris And Card:  ".$NXTText." will require both a card credential and an iris authentication (which matches the presented card)</li>
                                                <li>Iris And Card (PIN Pass-Through):  ".$NXTText." will require both a card credential and an iris authentication (which matches the presented card) then passes PIN through to the Access System</li>
                                                <li>PIN And Iris:  ".$NXTText." will send both PIN and an iris authentication to the Access System</li>
                                                <li>PIN, Card AND Iris:  ".$NXTText." will send a PIN, a card credential and an iris authentication (which matches the presented card)</li></ul></ul>");
define("TOOLTIP_ACS_PROTOCOL"," <span style='font-size: 16px; font-style:bold'>Protocolo ACS</span>
                                                <ul style='padding-left: 1em'><li>Use esta lista para selecionar o protocolo ACS que o ".$NXTText." utilizará.</li>
                                                <li>Os seguintes protocolos são suportados:</li>
                                                <ul><li>Wiegand</li>
                                                <li>HID Serial</li>
                                                <li>F2F</li>
                                                <li>PAC</li>
                                                <li>OSDP</li></ul></ul>");
define("TOOLTIP_OSDP_BAUD"," <span style='font-size: 16px; font-style:bold'>Taxa de transmissão OSDP</span>
                                                <ul style='padding-left: 1em'><li>Controla a taxa de comunicação serial assíncrona semi-duplex tanto para Entrada RS-485 quanto para Saída RS-485. A sinalização padrão é 8 bits de dados, 1 bit de parada e nenhum bit de paridade. O valor padrão para Taxa de Transmissão é 9600.</li>
                                                </ul>");
define("TOOLTIP_OSDP_ADDRESS"," <span style='font-size: 16px; font-style:bold'>Endereço OSDP</span>
                                                <ul style='padding-left: 1em'><li>Indica o endereço OSDP  para a Saída na rede multi-drop RS-485. O valor pad~roa para o endereço é 0.</li>
                                                </ul>");
define("TOOLTIP_ACS_LEDCONTROLLEDACS"," <span style='font-size: 16px; font-style:bold'>LED Controlado por ACS</span>
                                                		<br /><br />O controle de LED permite que o ACS controle o estado dos LEDs do ".$NXTText." LEDs e a sirene. Quando o controle de LED não está marcado, o ".$NXTText." gerencia os LEDs internamente.");
define("TOOLTIP_ACS_DUALAUTHENTICATION"," <span style='font-size: 16px; font-style:bold'>Autenticação Dupla</span>
                                                		<br /><br />A autenticação dupla exige que o usuário apresente um cartão ao leitor de cartão antes de apresentar seus olhos.         
														<br />O ".$NXTText." irá procurar o cartão na memória interna e, em seguinda, pedirá que o usuário aprensete seus olhos, assim que fica branco.         
														<br />Se a íris apresentada corresponder a íris gravada que foi encontrada, o ".$NXTText." enviará o dado ao ACS.         
														<br />Se não for correspondente, o ".$NXTText." enviará um código de correspondência negativa.");
define("TOOLTIP_ACS_TEMPLATEONCARDPASS"," <span style='font-size: 16px; font-style:bold'>Autenticação de Fator Único</span>
                                                		<br /><br />O ".$NXTText." enviará a credencial apresentada ou para um Leitor de Cartão conectado ou a íris correspondente para o Painel de Controle de Acesso.");
define("TOOLTIP_ACS_TEMPLATEONCARD"," <span style='font-size: 16px; font-style:bold'>Template no Cartão</span>
                                                		<br /><br />O ".$NXTText." irá esperar que o usuário apresente um cartão de credencial contendo seus modelos biométricos antes de fazer a correspondência.
														<br />O ".$NXTText." irá aguardar a credencial ser apresentada e, em seguida, ao ficar branco, pedirá que o usuário apresente seus olhos.                                
														<br /><strong>Salvando após, a ativação de template no cartão apagará a base de dados do ".$NXTText.".</strong>");
define("TOOLTIP_ACS_DUALAUTHPARITY"," <span style='font-size: 16px; font-style:bold'>Paridade de Autenticação Dupla</span>
                                                		<br /><br />Escolha se verifica os bits de paridade provenientes do leitor de cartão durante a leitura do cartão de autenticação dupla. ");
define("TOOLTIP_ACS_IRISWAITTIME"," <span style='font-size: 16px; font-style:bold'>Tempo de Espera por Iris</span>
                                                <ul style='padding-left: 1em'><li>Indica a duaração, em segudos, que o usuário deve apresentar suas íris para o ".$NXTText." depois de ler o cartão</li>
                                                <li>Faixa de Duração em segundos: 2 - 60.</li>
                                                <li>Valor padão: 10.</li></ul>");
define ("TOOLTIP_ACS_PINWAITTIME", "<span style='font-size: 16px; font-style:bold'>PIN Wait Time</span>
                                    <ul style='padding-left: 1em'><li>Specifies the duration, in seconds, within which the user must enter his PIN after scanning the card</li>
                                    <li>Duration Range (seconds): 2 - 60.</li>
                                    <li>Default value: 10.</li></ul>");
define ("TOOLTIP_ACS_PINBURSTBITS", "<span style='font-size: 16px; font-style:bold'>PIN Burst Bits</span>
                                    <ul style='padding-left: 1em'><li>Specifies the bit size for PIN processing</li>
                                    <li>Default value: 4.</li></ul>");
define("TOOLTIP_ACS_ENABLERELAYS"," <span style='font-size: 16px; font-style:bold'>Ativar Retransmissão</span>
                                                <ul style='padding-left: 1em'><li>Quando marcada, esta opção ativa a operação de retransmissões físicas no Painel ACS.</li>
                                                <li>Tanto retransmissões de <i>Permitir</i> quanto de <i>Negar</i> são suportadas.</li>
                                                <li>A duração das retransmissões ativas restantes pode ser configurada usando os dispositivos abaixo.</li>
                                                </ul>");
define("TOOLTIP_ACS_GRANTRELAYTIME"," <span style='font-size: 16px; font-style:bold'>Tempo de Retransmissão de Permissão</span>
                                                <ul style='padding-left: 1em'><li>Indicação a duração em segundos para disparar uma retransmissão de <i>Permissão</i> se a autenticação é bem-sucedida.</li>
                                                <li>Para desabilitar a retransmissão de <i>Permissão</i> individualmente, defina este valor como 0.</li>
                                                <li>Faixa de Duração em segundos: 0 - 10.</li>
                                                <li>Valor Padrão: 3.</li></ul>");
define("TOOLTIP_ACS_DENYRELAYTIME"," <span style='font-size: 16px; font-style:bold'>Tempo de Retransmissão de Negação</span>
                                                <ul style='padding-left: 1em'><li>Indica a duração em segundos para disparar uma retransmissão de <i>Negação</i> se a autenticação falhar.</li>
                                                <li>Para desabilitar a retransmissão de <i>Negação</i> individualmente, defina este valor como 0.</li>
                                                <li>Faixa de Duração em segundos: 0 - 10.</li>
                                                <li>Valor Padrão: 5.</li></ul>");
define("TOOLTIP_ACS_DURESSRELAYTIME"," <span style='font-size: 16px; font-style:bold'>Tempo de Retransmissão de Duress</span>
                                                <ul style='padding-left: 1em'><li>Indica a duração em segundos para disparar uma retransmissão de <i>Negação</i> se a autenticação é bem-sucedida.</li>
                                                <li>Para desabilitar a retransmissão de <i>Duress</i> individualmente, defina este valor como 0.</li>
                                                <li>Faixa de Duração em segundos: 0 - 10.</li>
                                                <li>Valor Padrão: 5.</li></ul>");
define("TOOLTIP_ACS_TESTCODE"," <span style='font-size: 16px; font-style:bold'>Código de Facilidade</span>
                                                <ul style='padding-left: 1em'><li>Exibe o código de Facilidade pré-configurado para ser usado com o protocolo ACS selecionado quando o botão <i>Testar Agora!</i> for clicado.</li>
                                                <li>Use a aplicação EyeEnroll para pré-configurar este valor antes de testar o sistema ACS.</li></ul>");
define("TOOLTIP_ACS_TESTCARDID"," <span style='font-size: 16px; font-style:bold'>ID de Cartão</span>
                                                <ul style='padding-left: 1em'><li>Exibe o ID de Cartão pré-configurado para ser usado com o protocolo ACS selecionado quando o botão <i>Testar Agora!</i> for clicado.</li>
                                                <li>Use a aplicação EyeEnroll para pré-configurar este valor antes de testar o sistema ACS.</li></ul>");
define("TOOLTIP_ACS_TESTACSBUTTON"," <span style='font-size: 16px; font-style:bold'>Botão Testar Agora!</span>
                                                <ul style='padding-left: 1em'><li>Clicar neste botão irá enviar uma string de teste pré-configurada para o Painel ACS. A aplicação EyEnroll deve ser usada para definir a string de teste no ".$NXTText.".</li></ul>");

			

                                
        
   

         
         

            //////////////////////////////////////////////////////////////////////////////
            // Other Tool Tips...
            //////////////////////////////////////////////////////////////////////////////
			
			define("TOOLTIP_HEADER_LANGUAGE"," <span style='font-size: 16px; font-style:bold'>Lista de Idiomas</span>
                                                <ul style='padding-left: 1em'><li>Use esta lista para selecionar o idioma atual para a interface de usuário WebConfig.</li>
                                                <li>Somente idiomas que estão disponíveis serão listados.</li></ul>");
define("TOOLTIP_HEADER_EYELOCKLOGO"," <span style='font-size: 16px; font-style:bold'>Eyelock Corporation</span>
                                                <ul style='padding-left: 1em'><li>Clique <a href='http://www.eyelock.com'  target='_blank'>aqui</a> para visitar o site do Eyelock.</li></ul>");
define("TOOLTIP_HEADER_CLIENTVER"," <span style='font-size: 16px; font-style:bold'>Versão WebConfig Client</span>
                                                <ul style='padding-left: 1em'><li>Representa a versão atual do WebConfig client em uso.</li></ul>");
define("TOOLTIP_HEADER_LICENSE"," <span style='font-size: 16px; font-style:bold'>Link da Licença</span>
                                                <ul style='padding-left: 1em'><li>Clique para ver o <i>Contrato de Licença de Usuário Final</i>.</li></ul>");
define("TOOLTIP_HEADER_HELP"," <span style='font-size: 16px; font-style:bold'>Link de Ajuda</span>
                                                <ul style='padding-left: 1em'><li>Ao clicar neste link, irá abrir uma caixa de diálogo onde as configurações do <i>Popup do Sistema de Ajuda</i> podem ser modificadas.</li></ul>");
define("TOOLTIP_HEADER_LOGOUT"," <span style='font-size: 16px; font-style:bold'>Link de Logout</span>
                                                <ul style='padding-left: 1em'><li>Ao clicar neste link irá registrar que o usuário saiu da sessão atual e retorna a página de login.</li></ul>");
define("TOOLTIP_FOOTER_APPVERSION"," <span style='font-size: 16px; font-style:bold'>Versão do Firmware do ".$NXTText."</span>
                                                <ul style='padding-left: 1em'><li>Representa a versão atual do firmware da aplicação que está em execução no ".$NXTText.".</li></ul>");
define("TOOLTIP_FOOTER_BOBVERSION"," <span style='font-size: 16px; font-style:bold'>Versão ICM ".$NXTText."</span>
                                                <ul style='padding-left: 1em'><li>Representa a versão atual do ICM que está em execução no ".$NXTText.".</li></ul>");
define("TOOLTIP_FOOTER_SAVEBUTTON"," <span style='font-size: 16px; font-style:bold'>Botão Salvar</span>
                                                <ul style='padding-left: 1em'><li>Clique para salvar permanentemente quaisquer mudanças realizadas nas configurações do ".$NXTText.".</li>
                                                <li>Após as configurações serem salvas, o ".$NXTText." reiniciará automaticamente com as novas configurações surtindo efeito imediatamente.</li></ul>");
 define ("TOOLTIP_FOOTER_CANCELBUTTON", "<span style='font-size: 16px; font-style:bold'>Botão de Cancelar</span>
                                                <ul style='padding-left: 1em'><li>Clique para descartas as alterações e atualizar a pãgina de configurações.</li>
                                                </ul>");

			
define("TOOLTIP_TAB_DEVICEIP"," <span style='font-size: 14px; font-style:bold'>Endereço IP/Nome do ".$NXTText."  Atual</span>");
define("TOOLTIP_TAB_DEVICEMACADDR"," <span style='font-size: 14px; font-style:bold'>Endereço MAC ".$NXTText."  Atual</span>");

			
			define("PT_FIRMWARE_UPDATE_NANO_TITLE","Processando a atualização do Firmware do Leitor de Modelo Portátil... Por favor, aguarde...");

define("PT_FIRMWARE_UPDATE_STATUS_UPLOAD","Carregando Firmware de Leitor de Modelo Portátil");

define("PT_FIRMWARE_UPDATE_STATUS_UPDATING_BOB","Atualizando Firmware de Leitor de Modelo Portátil");



			 define("PT_FIRMWARE_UPDATE_TITLE","Atualização de Firmware de Leitor de Modelo Portátil");

define("PT_FIRWMARE_UPDATE_SUCCESS","Atualização de Firmware de Leitor de Modelo Portátil realizada com sucesso.");

define("PT_FIRWMARE_UPDATE_RELOAD","Clique OK para atualizar a página.");

define("PT_FIRWMARE_UPDATE_ERROR_FAILED","Falha ao atualizar Firmware de Leitor de Modelo Portátil.");

define("PT_FIRWMARE_UPDATE_FAILED","Falha ao atualizar Firmware de Leitor de Modelo Portátil.");


			
			
	define("PT_FIRMWARE_MANAGEMENT_TITLE","Gerenciamento de Firmware de Leitor de Modelo Portátil");

define("PT_FIRWMARE_UPDATE_WAITING","Aguardando o Leitor de Modelo Portátil reiniciar...");





			
 




            //////////////////////////////////////////////////////////////////////////////
            // Validation Error Messages
            //////////////////////////////////////////////////////////////////////////////
          define("VALIDATOR_MSG_HOSTNAME"," Nome do Servidor não deve conter espaços ou caracteres especiais,</br> e deve ter menos de 64 caracteres!");
			define("TITLE_BTN_TESTNWMS"," Testar as configurações de Combinador de Rede atualmente aplicadas.");
			define("TEST_NWMS"," Testar NWMS");
			define("CB_NWMATCHERCOMMSECURE_TEXT","Não seguro");
			define("COPYRIGHTTEXT","Copyright \AD&#169; 2014. Todos os direitos reservados.");
			define("DISCONNECTWARNING","Não feche o navegador, desligue o ".$NXTText." ou remova a conexão de rede durante este processo.");
			break;

        case "jp":
        //////////////////////////////////////////////////////
        // LOGIN PAGE
        //////////////////////////////////////////////////////
            define("TITLE_NANOLOGIN", $NXTText." ログイン");
            define("LOGIN_USERNAME", "ユーザー名");
            define("LOGIN_PASSWORD", "パスワード");
            define("LOGIN_LOGIN", "ログイン");
            define("LOGIN_FORGOTPASSWORD", "パスワードをお忘れですか?");
            define("LOGIN_USER_GROUPTITLE", "ユーザー ログイン レベル");

			//
			//License page
			//
			define("TITLE_LICENSE_KEY", "Nano ライセンス キー");
			define("LK_ENTER_LICENSE_KEY_HEADER", "ライセンス キーの入力");
			define("LK_ENTER_KEY_TEXT", "".$NXTText." パッケージに含まれているライセンス <br>キー カードにあるライセンス キーを入力してください。");
			define("LK_DEFAULT_TEXT", "ライセンス キー...");
			define("LK_BTN_ACTIVATE_TEXT", "アクティブ化");
			define("LK_INVALID_LICENSE", "入力されたライセンス キーが無効です。");
			  define("SUBMIT_BUTTON", "保存");
			 define("CANCEL_BUTTON", "キャンセル");
			define("DLGBTN_CONTINUE","続行");
        //////////////////////////////////////////////////////
        // CONFIGURATION PAGE
        //////////////////////////////////////////////////////
            define("TITLE_NANOCONFIG", $NXTText." の構成");
            define("TITLE_HELP", "ヘルプ設定");
            define("TITLE_LOGOUT", "ログアウト");
			   define("HEADER_LICENSE", "ライセンス");
			
			
				 define ("NWMS_TEST_FAILED","NWMS テスト失敗");
			define ("NWMS_TEST_SUCCESS","NWMS テスト成功");
			define ("NWMS_SUCCESS_DETAIL","Network Matcher がターゲット アドレスで見つかりました。");
				define ("NWMS_FAIL_DETAIL_NOTFOUND","Network Matcher がターゲット アドレスで見つかりませんでした。");
				define ("NWMS_FAIL_DETAIL_INVALID","テストの応答が無効です。nano が動作中であることを確認してください。");
			define("SESSION_TIMEOUT_WARNING", "セッションの有効期限がまもなく切れます");
			define("SESSION_TIMEOUT_CONTINUE", "ログインしたままにするには [続行] をクリックしてください。");
			
            //////////////////////////////////////////////////////
            // HOME TAB STRINGS
            //////////////////////////////////////////////////////
            define ("HOME_TABLABEL", "ホーム");
            define ("HOME_HEADING", $NXTText." の情報");
            define ("HOME_DEVICEINFO_GROUPTITLE", "デバイス情報");
            define ("HOME_DEVICEINFO_DEVICENAME", $NXTText." の名前:");
            define ("HOME_DEVICEINFO_IPADDR", "IP アドレス:");
            define ("HOME_DEVICEINFO_IPV6ADDR", "IPv6 アドレス:");
			define ("HOME_DEVICEINFO_SERIAL", "デバイス ID:");
            define ("HOME_DEVICEINFO_MACADDR", "MAC アドレス:");
            define ("HOME_DEVICEINFO_LOCALTIME", $NXTText." の時間スナップショット:");
            define ("HOME_DEVICEINFO_PREVIOUSUPDATEFAILED", "この ".$NXTText." では以前にファームウェアの更新に失敗しました。<br>".$NXTText." は、最後に正しく動作した状態に戻されました。<br>ファームウェアを更新すると、このメッセージがクリアされます。");

            define ("HOME_STORAGESTATS_GROUPTITLE", "ストレージ統計情報");
            define("HOME_STORAGESTATS_PT_GROUPTITLE", "ポータブル テンプレート デコーダー情報");
            define ("HOME_STORAGESTATS_NETWORKMATCHENABLED", "Network Matcher:");
            define ("HOME_STORAGESTATS_NETWORKMATCHADDRESS", "Network Matcher アドレス:");
            define ("HOME_STORAGESTATS_TEMPLATESPACE", "テンプレート残量:");

            define ("HOME_PTINFO_BLEVERSION", "BLE バージョン:");
            define ("HOME_PTINFO_MAINFWVERSION", "ファームウェア バージョン:");
            define ("HOME_PTINFO_BOOTLOADERVERSION", "ブート ローダー バージョン:");
            define ("HOME_PTINFO_HARDWAREVERSION", "ハードウェア バージョン:");
            define ("HOME_PTINFO_KEYPADVERSION", "キーパッド バージョン:");
            define ("HOME_PTINFO_CONFIGURATION", "構成:");

            define ("HOME_SOFTWARE_GROUPTITLE", "ソフトウェア情報");
            define ("HOME_SOFTWARE_APPVER", $NXTText." のファームウェア:");
            define ("HOME_SOFTWARE_BOBVER", "ICM ファームウェア バージョン: ");
            define ("HOME_HARDWARE_BOBVER", "ICM ハードウェア バージョン: ");
             define ("HOME_SOFTWARE_LINUXVER", "Linux OS バージョン: ");
             define ("HOME_HARDWARE_PSOCVER", "M.B. PSOC バージョン: ");
             define ("HOME_CAMERA_FPGAVER", "カメラ FPGA バージョン: ");
             define ("HOME_CAMERA_PSOCVER", "カメラ PSOC バージョン: ");
             define ("HOME_FIXED_BOARDVER", "固定ボード バージョン: ");

            define ("HOME_SOFTWARE_NANOLABEL", $NXTText." 更新済: ");
            define ("HOME_SOFTWARE_BOBLABEL", "ICM 更新済: ");
			define ("HOME_SOFTWARE_FPGALABEL", "カメラ FPGA 更新済: ");
			define ("HOME_SOFTWARE_CAMERAPSOCLABEL", "カメラ PSOC 更新済: ");
			define ("HOME_SOFTWARE_FIXEDPSOCLABEL", "固定ボード更新済: ");

       
            //////////////////////////////////////////////////////
            // NETWORK TAB STRINGS
            //////////////////////////////////////////////////////
            define ("NETWORK_TABLABEL", "ネットワーク");
            define ("NETWORK_HEADING", "デバイス ネットワーク構成");
            define ("NETWORK_DEVICENAME_GROUPTITLE", "デバイス名");
            define ("NETWORK_DEVICENAME_LABEL", "名前:");
            define ("NETWORK_DHCP_GROUPTITLE", "IP アドレスの自動取得 (DHCP)");
            define ("NETWORK_DHCP_SETTINGS", "DHCP 設定...");
			define("NETWORK_DNS_SERVER1", "DNS サーバー 1:");
			define("NETWORK_DNS_SERVER2", "DNS サーバー 2:");
                ///// ADV. DHCP SETTINGS DIALOG
                define ("NETWORK_DHCPSETTINGS_TITLE", "詳細 DHCP 設定");
                define ("NETWORK_DHCPSETTINGS_TIMEOUTS_GROUPLABEL", "タイムアウト");
                define ("NETWORK_DHCPSETTINGS_TIMEOUTS_DHCPTIMEOUTLABEL", "DHCP タイムアウト:");
                define ("NETWORK_DHCPSETTINGS_TIMEOUTS_DHCPRETRIESLABEL", "DHCP 再試行:");
                define ("NETWORK_DHCPSETTINGS_TIMEOUTS_DHCPRETRYDELAYLABEL", "再試行遅延:");
                define ("NETWORK_DHCPSETTINGS_OK", "OK");
            define ("NETWORK_STATICIP_GROUPLABEL", "次の静的 IP アドレスを使用");
            define ("NETWORK_STATICIP_DEVICEIPLABEL", "デバイス IP:");
            define ("NETWORK_STATICIP_BROADCASTNETWORKLABEL", "ブロードキャスト ネットワーク:");
            define ("NETWORK_STATICIP_SUBNETMASKLABEL", "サブネット マスク:");
            define ("NETWORK_STATICIP_DEFAULTGATEWAYLABEL", "デフォルト ゲートウェイ:");

            define ("NETWORK_IPV6_ENABLE", "IPv6 を有効にする");
            define ("NETWORK_IPV6_CONFIGURATION", "IPv6 構成...");
                // IPV6 Pop-up
                define ("NETWORK_IPV6CONFIGURATION_TITLE", "IPv6 構成");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL", "一般");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE", "DHCP モード:");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_INFORMATION", "ステートレス");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_NORMAL", "ステートフル");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_AUTO", "自動");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DHCP_MODE_NONE", "オフ");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_ADDRESS_PREFIX_LENGTH", "IPv6 アドレス/サブネット プレフィックス長:");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DEFAULTGATEWAY", "デフォルト ゲートウェイ:");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DNS_SERVER1", "DNS サーバー 1:");
                define ("NETWORK_IPV6CONFIGURATION_GENERAL_DNS_SERVER2", "DNS サーバー 2:");
                define ("NETWORK_IPV6CONFIGURATION_STATUS", "ステータス");
                define ("NETWORK_IPV6CONFIGURATION_STATUS_LINK_LOCAL", "リンクローカル:");
                define ("NETWORK_IPV6CONFIGURATION_STATUS_GLOBAL", "グローバル:");
                define ("NETWORK_IPV6CONFIGURATION_STATUS_GLOBAL_STATELESS", "グローバル ステートレス:");
				define ("NETWORK_IPV6CONFIGURATION_STATUS_GLOBAL_STATELESS_TEMPORARY", "グローバル ステートレス一時:");
                define ("NETWORK_IPV6CONFIGURATION_STATUS_NO_ADDRESSES", "デバイスには IPv6 アドレスが現在割り当てられていません");

            define("SSL_PROTOCOL_LEGEND", "SSL プロトコル");
            define("SSLPROTO_DEFAULT", "レガシー");
            define("SSLPROTO_TLS12", "TLS 1.2 (専用)");

            define ("NETWORK_CERTIFICATES_GROUPTITLE", "証明書");
            define ("NETWORK_CACERT", "CA 証明書:");
            define ("NETWORK_CLIENTCERT", "クライアント証明書:");
            define ("NETWORK_CLIENTPRIVATEKEY", "クライアント秘密キー (と証明書):");
            define ("NETWORK_CERTIFICATES_SETTINGS_GROUPTITLE", "設定");
            define ("NETWORK_EAPOLVERSION", "EAPOL バージョン:");
            define ("NETWORK_EAPIDENTITY", "EAP ID:");
            define ("NETWORK_PRIVTEKEYPWD", "秘密キー パスワード:");

            define ("NETWORK_SETTINGS_ENABLEIEEE8021X", "IEEE 802.1X を有効にする");
            define ("NETWORK_SETTINGS_NOCERTIFICATE", "証明書ファイルがアップロードされていません");
            define ("NETWORK_SETTINGS_NOKEY", "秘密キーがアップロードされていません");
            define ("NETWORK_802LOG_DOWNLOAD", "IEEE 802.1X ログのダウンロード...");
            
            define ("NETWORK_CHECKING_IP_ADDRESS_DUPLICATE", "IP アドレスの重複を確認中");
            define ("NETWORK_DUPLICATE_IP_ADDRESS", "IP アドレス競合");
            define ("NETWORK_IP_ADDRESS_IN_USE", "指定されたデバイス IP アドレスはネットワークの別のシステムと競合しています。");

            //////////////////////////////////////////////////////
            // DEVICE TAB STRINGS
            //////////////////////////////////////////////////////
            define ("DEVICE_TABLABEL", "デバイス");
            define ("DEVICE_HEADING", "デバイス設定");
            define ("DEVICE_USERFEEDBACK_GROUPTITLE", "フィードバック");
            define ("DEVICE_USERFEEDBACK_VOLUMELABEL", "スピーカー音量:");
            define ("DEVICE_USERFEEDBACK_FREQUENCYLABEL", "トーン周波数 (Hz):");
            define ("DEVICE_USERFEEDBACK_DURATIONLABEL", "トーン期間:");
            define ("DEVICE_USERFEEDBACK_TAMPERVOLUMELABEL", "改ざんトーン音量");
            define ("DEVICE_USERFEEDBACK_LEDBRIGHTNESSLABEL", "LED の明るさ:");
            define ("DEVICE_USERFEEDBACK_LOCATEDEVICE", "デバイスの発見...");
            define ("DEVICE_TIMESETTINGS_GROUPTITLE", "時刻設定");
            define ("DEVICE_TIMESETTINGS_SERVERLABEL", "タイム サーバー:");
            define ("DEVICE_TIMESETTINGS_NANOTIMELABEL", "nano ローカル時刻:");
            define ("DEVICE_TIMESETTINGS_UPDATETIME", "今すぐ同期");
            define ("DEVICE_TIMESETTINGS_SYNCHRONIZETIMELABEL", "毎日同期");
    	    define ("DEVICE_TIMESETTINGS_UPDATELOCALTIME", "ホストと同期");
            define ("DEVICE_ACTIVITIES_GROUPTITLE", "アクティビティ");
            define ("DEVICE_ACTIVITIES_FACTORYRESET", "出荷時の設定にリセット");
		    define ("DEVICE_ACS_OSDPINSTALLMODE", "OSDP インストール モード");
            define ("DEVICE_ACTIVITIES_REBOOTDEVICE", "デバイスのリブート");
            define ("DEVICE_EXTERNAL_GROUPTITLE", "HBOX 設定 (EyeLock サポート専用)");
            define("DEVICE_EXTERNAL_WELCOMEMESSAGE", "ようこそメッセージ:");
            define("DEVICE_EXTERNAL_LOCATION", "場所:");
            define("DEVICE_EXTERNAL_POSTTITLE", "イベント後の URL");
            define("DEVICE_EXTERNAL_DESTINATIONURL", "メイン URL:");
            define("DEVICE_EXTERNAL_IRISURL", "虹彩 POST エンドポイント:");
            define("DEVICE_EXTERNAL_ERRORURL", "エラー エンドポイント:");
            define("DEVICE_EXTERNAL_HEARTBEATURL", "ハートビート エンドポイント:");
            define("DEVICE_EXTERNAL_MAINTENANCEURL", "保守エンドポイント:");
            define("DEVICE_EXTERNAL_POSTSCHEMEURL", "POST スキーム:");
            define ("DEVICE_ADVSETTINGS_GROUPTITLE", "詳細デバイス設定");
            define ("DEVICE_ADVSETTINGS_LISTENINGPORTLABEL", "ネットワーク リスニング ポート:");
            define ("DEVICE_ADVSETTINGS_EYEDESTADDR", "目の情報の保存先アドレス:");
            define ("DEVICE_ADVSETTINGS_EYEDESTPORT", "目の情報の保存先ポート:");
			  define ("DEVICE_ADVSETTINGS_EYECONNECTTIMEOUT", "目の接続タイムアウト:");
			  define ("DEVICE_ADVSETTINGS_EYESENDTIMEOUT", "目の送信タイムアウト:");

			define ("DEVICE_TIMESERVERFAIL_PING", "タイム サーバーを Ping できませんでした。");
			define ("DEVICE_TIMESERVERFAIL_SYNC", "タイム サーバーと同期できませんでした。");
				define("FACTORY_RESET_PROGRESS", "出荷時の設定にリセット中です。");
			define("FACTORY_RESET_LOGGEDOUT", "このプロセスのためにログアウトされました。");
			define("FACTORY_RESET_TWOMINS", "約 2 分後に再びログインできます。");
            //////////////////////////////////////////////////////
            // SECURITY TAB STRINGS
            //////////////////////////////////////////////////////
            define ("SECURITY_TABLABEL", "セキュリティ");
            define ("SECURITY_HEADING", "セキュリティ設定");

            define ("SECURITY_PASSWORD_GROUPTITLE", "パスワード リセット");
            define ("SECURITY_PASSWORD_OLDPWDLABEL", "古いパスワード:");
            define ("SECURITY_PASSWORD_NEWPWDLABEL", "新しいパスワード:");
            define ("SECURITY_PASSWORD_CONFIRMPWDLABEL", "パスワードの確認:");
            define ("SECURITY_PASSWORD_REMOVEPWDLABEL", "パスワードのクリア");
            define ("SECURITY_PASSWORD_RESETPWD", "パスワードの更新");

            define ("SECURITY_TAMPER_SETTINGS", "改ざん設定");
            define ("SECURITY_TAMPER_SIGNALHIGH", "高信号で改ざん状態をアクティブにする");
            define ("SECURITY_TAMPER_SIGNALLOW", "低信号で改ざん状態をアクティブにする");
            define ("SECURITY_TAMPER_NOTIFYADDRESS", "通知アドレス:");
            define ("SECURITY_TAMPER_NOTIFYPORT", "ポート:");
            define ("SECURITY_TAMPER_NOTIFYMESSAGE", "改ざんメッセージ:");
			define ("SECURITY_TAMPER_MESSAGE_SETTINGS", "改ざんメッセージ設定");
	     
             define ("SECURITY_CARD_READER_INPUT_TAMPER_SETTINGS", "カード リーダーの入力改ざん設定");
              define ("SECURITY_NXT_OUTPUT_TAMPER_SETTINGS", "改ざん出力設定");
               define ("SECURITY_OUTPUT_TAMPER_SIGNALHIGH", "高信号");
            define ("SECURITY_OUTPUT_TAMPER_SIGNALLOW", "低信号");
            //////////////////////////////////////////////////////
            // SOFTWARE TAB STRINGS
            //////////////////////////////////////////////////////
			define ("SOFTWARE_FIRMWARELINK", "最新のファームウェアは <a href=\"http://help.eyelock.com\">help.eyelock.com</a> で入手できます。");
            define ("SOFTWARE_TABLABEL", "ソフトウェア");
            define ("SOFTWARE_HEADING", "ソフトウェア/ファームウェアの詳細");
            define ("SOFTWARE_STATUS_GROUPTITLE", "バージョン/更新ステータス");
            define ("SOFTWARE_CHECKUPDATES_LABEL", "前回の更新確認:");
            define ("SOFTWARE_UPDATEDETAILS_TITLE", "ソフトウェア更新の詳細");
            define ("SOFTWARE_AVAILUPDATE_NANOLABEL", "新しい ".$NXTText." のバージョン:");
            define ("SOFTWARE_AVAILUPDATE_BOBLABEL", "新しい ICM のバージョン:");
            define ("SOFTWARE_INSTALLEDUPDATES_NANOLABEL", $NXTText." の更新がインストールされました:");
            define ("SOFTWARE_INSTALLEDUPDATES_BOBLABEL", "ICM の更新がインストールされました:");
            define ("SOFTWARE_UPDATE_ALLOWSITEADMIN", "管理者にデバイスの更新を許可");

			define ("VERSION_HEADER", "バージョン");
			define("RESTORE_POINTS_TIME_STAMP", "復元ポイントのタイムスタンプ");
            define ("SOFTWARE_STATUS_UPDATESTATUS_FAILED", "更新サーバーに接続できませんでした。");
            define ("SOFTWARE_STATUS_UPDATESTATUS_NEWVERSION", "新しい ".$NXTText." のソフトウェア更新を入手できます。");
            define ("SOFTWARE_STATUS_UPDATESTATUS_CURRENT", "".$NXTText." のソフトウェアは最新です。");
            define ("SOFTWARE_STATUS_UPDATESTATUS_VERCORRUPT", "バージョン ファイルが破損しているようです。");
            define ("SOFTWARE_STATUS_UPDATESTATUS_CHECKINTERNET", "ブラウザーのインターネット接続ステータスを確認してください。");
            define ("SOFTWARE_STATUS_UPDATENOW", "今すぐ更新");
            define ("SOFTWARE_STATUS_LATER", "後で更新");
            define ("SOFTWARE_STATUS_MANUALNANO", "ローカル ファイル...");
            define ("SOFTWARE_STATUS_MANUALBOB", "ローカル ICM ファイル...");
            define ("SOFTWARE_STATUS_UPDATEDETAIL", "詳細...");
            define ("SOFTWARE_MODE_NANOLABEL", $NXTText." の復元ポイント");
			define ("SOFTWARE_MODE_BOBLABEL", "ICM ボードの復元ポイント");
            define ("SOFTWARE_MODE_DELETERESTOREPOINTS", "デバイスの復元ポイントを削除中...");
            define ("SOFTWARE_MODE_RESTORERESTOREPOINT", "以前のバージョンに復元中...");
            define ("SOFTWARE_RESTOREPOINTS_NONANO", "使用できる ".$NXTText." の復元ポイントがありません。");
            define ("SOFTWARE_RESTOREPOINTS_NOBOB", "使用できる ICM の復元ポイントがありません。");
            define ("SOFTWARE_RESTOREHEADER_SELECT", "選択");
            define ("SOFTWARE_RESTOREHEADER_RESTOREPOINTS", "復元ポイント");
            define ("SOFTWARE_RESTORE_GROUPTITLE", "ファームウェアの復元");
            define ("SOFTWARE_RESTORE_RESTORENOW", "今すぐ復元...");
            define ("SOFTWARE_RESTORE_DELETERESTOREPOINTS", "削除...");
			

            //////////////////////////////////////////////////////
            // AUTHENTICATION TAB STRINGS
            //////////////////////////////////////////////////////
            define ("AUTHENTICATION_TABLABEL", "虹彩処理");
            define ("AUTHENTICATION_HEADING", "虹彩処理詳細の構成");
            define ("AUTHENTICATION_SETTINGS_IRISPROCESSINGMODE", "虹彩処理モード:");
            define ("AUTHENTICATION_SETTINGS_ACCESSCONTROLMODE", "アクセス制御認証モード");
            define ("AUTHENTICATION_SETTINGS_ACCESSCONTROLAUTHENTICATION", "アクセス制御認証");
            define ("AUTHENTICATION_SETTINGS_IRISCAPTUREMODE", "虹彩キャプチャ モード");
            define ("AUTHENTICATION_SETTINGS_IRISCAPTURE", "虹彩キャプチャ");
            define ("DEVICE_EXTERNAL_POSTSCHEME", "虹彩 POST スキーム:");
            define ("AUTHENTICATION_SETTINGS_IRISCAPTURESETTINGS", "虹彩キャプチャの設定");
            define ("AUTHENTICATION_SETTINGS_IRISCAPTURETIMEOUT", "虹彩キャプチャ タイムアウト:");
            define ("AUTHENTICATION_SETTINGS_IRISCAPTURERESETDELAY", "虹彩キャプチャ リセット遅延:");
            define ("AUTHENTICATION_SETTINGS_HTTPPOSTMSGFORMAT", "HTTP POST メッセージの形式:");
            define ("AUTHENTICATION_SETTINGS_IRISIMAGEFORMAT", "虹彩画像形式:");
            define ("AUTHENTICATION_SETTINGS_IRISIMAGEQUALITY", "画質:");
            define ("AUTHENTICATION_SETTINGS_100LOSSLESS", "(100 = 損失なし)");
            define ("AUTHENTICATION_MODE_GROUPTITLE", "照合");
            define ("AUTHENTICATION_MODE_SINGLEEYELABEL", "片目の使用");
            define ("AUTHENTICATION_MODE_DUALEYELABEL", "両目の使用");
            define ("AUTHENTICATION_SETTINGS_GROUPTITLE", "設定");
            define ("AUTHENTICATION_SETTINGS_REPEATPERIODLABEL", "反復認証期間:");
            define ("AUTHENTICATION_SETTINGS_NEGMATCHTIMEOUTENABLEDLABEL", "照合失敗タイムアウトを有効にする");
            define ("AUTHENTICATION_SETTINGS_LOITERPERIODLABEL", "待機時間:");
            define ("AUTHENTICATION_SETTINGS_NEGMATCHRESETLABEL", "照合失敗リセット タイマー:");
            define ("AUTHENTICATION_SETTINGS_DESTINATIONADDRESSLABEL", "ネットワーク メッセージ宛先 IP:");
            define ("AUTHENTICATION_SETTINGS_DESTINATIONPORTLABEL", "ポート:");
            define ("AUTHENTICATION_SETTINGS_SECURENETWORDLABEL", "安全なネットワーク照合メッセージ");
            define ("AUTHENTICATION_SETTINGS_MSGFORMATLABEL", "ネットワーク メッセージの形式:");
            define ("AUTHENTICATION_SETTINGS_SENDALLIMAGES", "すべての画像の送信");

            //////////////////////////////////////////////////////f
            // DATABASE TAB STRINGS
            //////////////////////////////////////////////////////
            define ("DATABASE_TABLABEL", "データベース");
            define ("DATABASE_HEADING", "データベース構成の詳細");
            define ("DATABASE_TYPE", "データベース タイプ: ");
            define ("DATABASE_SQLLITE", "SQLite");
            define ("DATABASE_BINARY", "バイナリ (フラット ファイル)");
            define ("DATABASE_TYPE_GROUPTITLE", "データベースの詳細");
            define ("DATABASE_TYPE_LOCALLABEL", "ローカル");
            define ("DATABASE_TYPE_NETMATCHLABEL", "Network Matcher (NWMS) を有効にする");
            define ("DATABASE_TYPE_NETMATCHADDRESSLABEL", "Network Matcher アドレス:");
			define ("TESTING_NETWORK_MATCHER", "Network Matcher のテスト中...");
            define ("DATABASE_TYPE_NETMATCHPORTLABEL", "Network Matcher 宛先ポート:");
            define ("DATABASE_STATISTICS_GROUPTITLE", "データベース統計情報");
            define ("DATABASE_TOC_GROUPTITLE", "ポータブル テンプレート");
            define ("DATABASE_STATISTICS_TEMPLATESPACE", "テンプレート領域残量:");
			define ("DATABASE_SECURECOMM_NETMATCHLABEL", "安全な Network Matcher");
            define("DATABASE_MOBILEMODE", "モバイル モード:");
            define ("TOOLTIP_TOC_MODE", "スマートフォン アプリのポータブル テンプレートでは、歩行、タップで送信、PIN で送信のモードが使われます。</br><strong>EV1 カード システムには影響しません。</strong>");
            define ("TOOLTIP_TOC_IRIS_WAIT_TIME", "ポータブル テンプレートが提示された後、".$NXTText." で保持される秒数。最小で 10 秒、最大で 600 秒です。");
              define("TOOLTIP_TOC_DEFAULT_KEY", "デフォルトの ".$NXTText." キーをポータブル テンプレートに対して使います。");
             define("TOOLTIP_TOC_CUSTOM_KEY", "アップロードしたキーをポータブル テンプレートに対して使います。");
            define("TOOLTIP_UPLOAD_CERTIFICATE", "ポータブル テンプレートで使う .pfx キーをアップロードします。キーのパスワードを左側のボックスに入力してください。");
            define("TOOLTIP_CURRENT_CERTIFICATE", "「キーのアップロード...」とここに表示されたら、キーをアップロードしてください。");
            define("DATABASE_EXPIRATION", "虹彩有効期限");
			
			define("DATABASE_CURRENT_KEY", "現在のキー:");
			
			define("DATABASE_TOC_MODE_WALKUP","歩行");
				define("DATABASE_TOC_MODE_TAPTOSEND","タップで送信");
				define("DATABASE_TOC_MODE_PINTOSEND","PIN で送信");
            //////////////////////////////////////////////////////
            // ACS TAB STRINGS
            //////////////////////////////////////////////////////
            define ("ACP_TABLABEL", "ACS");
            define ("ACP_HEADING", "アクセス制御システム (ACS)");
            define ("ACP_PROTOCOL_GROUPTITLE", "アクセス制御プロトコル");
            define ("ACP_PROTOCOL_PROTOCOL", "プロトコル:");
            define ("ACP_PROTOCOL_DUALAUTHENABLEDLABEL", "2 要素認証");
             define ("ACP_PROTOCOL_TEMPLATEONCARD", "ポータブル テンプレート");
             define ("ACP_PROTOCOL_TEMPLATEONCARDPASS", "1 要素認証");
			 
             
               define ("ACP_PROTOCOL_DUALAUTHPARITY", "パリティ ビットの確認");
            define ("ACP_PROTOCOL_DUALAUTHLEDENABLEDLABEL", "ACS で LED を制御");
            define ("ACP_PROTOCOL_MATCHWAITIRISTIMELABEL", "虹彩待機時間:");
            define ("ACP_PROTOCOL_MATCHWAITPINTIMELABEL", "PIN 待機時間:");
            define ("ACP_PROTOCOL_PINBURSTBITS", "PIN バースト ビット:");
            define ("ACP_PROTOCOL_RELAYTIMELABEL", "許可リレー時間:");
            define ("ACP_PROTOCOL_DENYRELAYTIMELABEL", "拒否リレー時間:");
            define ("ACP_PROTOCOL_DURESSRELAYTIMELABEL", "脅迫リレー時間:");
            define ("ACP_PROTOCOL_ENABLERELAYTRIGGERSLABEL","リレーを有効にする");
            define ("ACP_PROTOCOL_NEGMATCHTIMEOUTLABEL", "照合失敗タイムアウト:");

            define ("ACP_TEST_GROUPTITLE", "アクセス拒否とテスト データ");
            define ("ACP_TEST_TESTBUTTON", "今すぐテスト");
            define ("ACP_TEST_CARDIDLABEL", "カード番号:");
            define ("ACP_TEST_FACILITYCODELABEL", "施設コード:");
            define ("ACP_TEST_SENDINGMESSAGE", "ACS パネルにテスト メッセージを送信中...");
            define ("ACP_NETWORK_SECURECOMMLABEL", "安全な通信");
            define ("ACP_TEST_TCPCONNECTIONFAILED", "ソケット接続失敗");
            define ("ACP_TEST_FAILED", "ACS テスト文字列をパネルに送信できませんでした。");
            define ("ACP_TEST_CONNECTIONFAILED", "ファームウェアとソケット接続を確立できません。");
	        define ("OSDP_INSTALL_MODE_SETTINGS", "デバイスをインストール モードに設定中。");
            define ("AUTHENTICATION_SCHEME", "認証スキーム:");
            define ("ACP_DD_SINGLEFACTORIO", "1 要素 [虹彩のみ]");
            define ("ACP_DD_SINGLEFACTORIC", "1 要素 [虹彩またはカード]");
            define ("ACP_DD_DUALFACTORIC", "2 要素 [虹彩とカード]");
            define ("ACP_DD_DUALFACTORICPP", "2 要素 [虹彩とカード (PIN パススルー)]");
            define ("ACP_DD_DUALFACTORPI", "2 要素 [虹彩と PIN]");
            define ("ACP_DD_DUALFACTORIPID", "2 要素 [虹彩と PIN (脅迫)]");
            define ("ACP_DD_DUALFACTORICPI", "3 要素 [虹彩とカードと PIN]");
            define ("ACP_DD_DUALFACTORICPID", "3 要素 [虹彩とカードと PIN (脅迫)]");
			
			define("PARITY_MASK_GROUP_TITLE", "パリティ マスキング");
			define("TOOLTIP_ACS_PARITY_MASK_DISABLED", "選択すると、2 要素認証時にリーダーのパリティ ビットが ".$NXTText." で確認されます");
			define("TOOLTIP_ACS_PARITY_MASK_ENABLED", "選択すると、2 要素認証時にリーダーのパリティ ビットが ".$NXTText." で無視されます");
			
			define("ACS_PARITY_MASK_DISABLED", "マスキングなし");
			define("ACS_PARITY_MASK_ENABLED", "マスキングあり");
            //////////////////////////////////////////////////////
            // LOGS TAB STRINGS
            //////////////////////////////////////////////////////
			define ("LOG_NO_INFO", "使用できるログ情報がありません。");
			define("MATCH_FAIL_NO_IRIS","照合失敗: 虹彩がありません");
			define("MATCH_FAIL_MISMATCH","照合失敗: 虹彩が一致しません");
			define("MATCH_FAIL_INVALID_CARD","照合失敗: カードが無効です");
            define("MATCH_SUCCESS","照合成功");
            define("MATCH_SUCCESSDURESS","照合成功 (脅迫)");
            define("MATCH_FAIL_INVALIDPIN","照合失敗: PIN が無効です");
            define("MATCH_FAIL_NOPIN","照合失敗: PIN がありません");

            define ("LOGS_TABLABEL", "ログ");
            define ("LOGS_HEADING", "ログ");
            define ("LOGHEADER_STATUS", "ステータス");
            define ("LOGHEADER_DATE", "日時");
            define ("LOGHEADER_NAME", "名前");
            define ("LOGHEADER_CODE", "ACS コード");
            define ("LOGHEADER_MESSAGE", "メッセージ");
            define ("LOGS_EVENTLOG_GROUPTITLE", "イベント ログ");
            define ("LOGS_EVENTLOG_REFRESHBUTTON", "リフレッシュ");
            define ("LOGS_EVENTLOG_DOWNLOAD", "ログのダウンロード...");
            define ("LOG_AUTOREFRESH_LABEL", "自動リフレッシュ:");
			define ("LOGS_TIMES_SHOWN_IN", "すべての時刻を次で表示: ");
			define ("LOG_TYPE", "ログ レベル");
            //////////////////////////////////////////////////////
            // DUMP TAB STRINGS
            //////////////////////////////////////////////////////
            define ("DUMP_TABLABEL", "ダンプ");


            //////////////////////////////////////////////////////
            // HELP SETTINGS DIALOG STRINGS
            //////////////////////////////////////////////////////
            define ("DIALOG_HELPSETTINGS_TITLE", "ヘルプ設定");
            define ("DIALOG_HELPSETTINGS_ENABLEHELP", "ポップアップ ヘルプを有効にする");
            define ("DIALOG_HELPSETTINGS_POPUPTRIGGERMODE", "ポップアップ トリガー モード:");
            define ("DIALOG_HELPSETTINGS_POPUPHOVER", "マウスのポイント");
            define ("DIALOG_HELPSETTINGS_POPUPCLICK", "マウスのクリック");
            define ("DIALOG_HELPSETTINGS_POPUPDELAY", "カーソルのポイント遅延:");


            //////////////////////////////////////////////////////
            // HELP DIALOG HELP STRINGS
            //////////////////////////////////////////////////////
            define ("TOOLTIP_HELPSETTINGS_ENABLEHELP", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ポップアップ ヘルプを有効にする&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションを選ぶと、&lt;i&gt;ポップアップ ヘルプ システム&lt;/i&gt;が有効になります。&lt;/li&gt;&lt;li&gt;選ばないと、&lt;i&gt;ポップアップ ヘルプ システム&lt;/i&gt;が完全に無効になります。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_HELPSETTINGS_POPUPMODE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ポップアップ ヘルプ モード&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;&lt;i&gt;ポップアップ ヘルプ システム&lt;/i&gt;のポップアップを開始するために使う方法を選びます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_HELPSETTINGS_POPUPDELAY", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;カーソルのポイント遅延&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;&lt;i&gt;ポップアップ ヘルプ システム&lt;/i&gt;のウィンドウが表示されるまでに、マウスポインターをポイントしたままにする必要がある秒数を指定します。&lt;/li&gt;&lt;li&gt;期間範囲 (秒): 0 - 5。&lt;/li&gt;&lt;li&gt;デフォルト値: 1.0。&lt;/li&gt;&lt;/ul&gt;");


            //////////////////////////////////////////////////////
            // COMMON STRINGS
            //////////////////////////////////////////////////////
            define ("TAMPER_ICON_ALTTEXT", "改ざんアラート!");

            /////////////////////////////////////////////////////
            // TOOLTIP STRINGS
            /////////////////////////////////////////////////////
            define ("TAMPER_TOOLTIP_TEXT", "デバイスの改ざんが検知されました。");
            define ("ATOMIC_TOOLTIP_TEXT", "Eyelock アプリケーションが実行されています。");
            define ("POWERBUTTON_TOOLTIP_TEXT", "Eyelock アプリケーションが実行されていません。");
            define ("EYELOCK_APPLICATION_STATUS", "Eyelock アプリケーション ステータス");
            define ("EYELOCK_MASTER_STATUSTEXT", "デバイス カメラ 1 のステータス:");
            define ("EYELOCK_SLAVE_STATUSTEXT", "デバイス カメラ 2 のステータス:");
            define ("EYELOCK_STATUS_RUNNING", "OK");
            define ("EYELOCK_STATUS_NOTRUNNING", "失敗");

            /////////////////////////////////////////////////////
            // ASSORTED STRINGS
            /////////////////////////////////////////////////////
            define ("SECONDS_LABEL", " 秒");
            define ("MILLISECONDS_LABEL", " ミリ秒");
			define("MSG_UPDATING", "更新中");
            define ("DEFAULT_EMPTY_FIELD", "オプション");
            define ("REQUIRED_EMPTY_FIELD", "必須");
            define ("CHANGE_PASSWORD_OLD", "古いパスワード");
            define ("CHANGE_PASSWORD_NEW", "新しいパスワード");
            define ("CHANGE_PASSWORD_CONFIRM", "パスワードの確認");
            define ("MSG_UNAVAILABLE", "使用不可");
            define ("MSG_USERHELLO", "こんにちは、");
            define ("MSG_UNKNOWNUSER", "不明なユーザー");
            define ("MSG_ENABLED", "有効");
            define ("MSG_DISABLED", "無効");
            define ("MSG_NEVER", "不可");
            define ("MSGBOX_INFORMATION_TITLE", "情報");
            define ("MSGBOX_INFORMATION_TITLESHORT", "情報");
            define ("MSGBOX_SUCCESS_TITLE", "成功");
            define ("MSGBOX_FAILED_TITLE", "失敗");
            define ("MSGBOX_TAMPERED_TITLE", "改ざん");
            define ("MSGBOX_OKBUTTON", "OK");
            define ("MSGBOX_CANCELBUTTON", "キャンセル");
            define ("MSGBOX_YESBUTTON", "はい");
            define ("MSGBOX_NOBUTTON", "いいえ");
            define ("SAVING_SETTINGS", "設定を保存中...");
            define ("SAVING_SETTINGSANDRESTART", "設定を保存して再起動中...");
            define ("SAVING_FEWMOMENTS", "しばらくお待ちください...");
            define ("RELOADING_PAGE", "ページを再ロード中...お待ちください...");
            define ("REFRESHING_PAGE", "ページをリフレッシュ中...お待ちください...");
            define ("VALIDATION_FAILEDTITLE", "検証に失敗しました。");
            define ("VALIDATION_MESSAGE1", "一部のフィールドの内容が有効ではありません。");
            define ("VALIDATION_MESSAGE2", "無効なフィールドがないか、すべてのタブを確認してください。");
            define ("CONNECTION_FAILEDTITLE", "接続エラー!");
            define ("CONNECTION_MESSAGE1", "WebConfig がデバイスと接続できませんでした。");
            define ("CONNECTION_MESSAGE2", "デバイスの IP アドレスと電源を確認してください。");
            define ("LOADINGLOG_DETAIL", "ログ詳細のロード中...お待ちください...");
            define ("ALERT_IPINUSE", "指定された静的 IP はすでに使用中です。保存できない設定があります。");
            define ("ALERT_802CONFIG", "IEEE 802.1X 構成情報を確認できません。保存できない設定があります。必要な証明書やキーをすべてアップロードしたことを確認してください。");
            define ("RESETPASSWORD_MESSAGETITLE", "パスワード リセット");
            define ("RESETTINGPASSWORD_MESSAGE", "パスワードをリセット中。お待ちください...");
            define ("RESETPASSWORD_SUCCESS", "パスワードが正常にリセットされました。");
            define ("RESETPASSWORD_LOGOUT", "ログアウトすると新しいパスワードを使用できます。");
            define ("RESETPASSWORD_FAIL", "パスワードをリセットできませんでした。");
	     define ("KEY_MANAGEMENT_GROUPTITLE", "キー管理の設定");
         define ("KEY_MANAGEMENT_DEFAULT", "デフォルト キーの使用");
          define ("KEY_MANAGEMENT_CUSTOM", "カスタム キーの使用");
			define("DATABASE_CURRENT_KEY_DEFAULT", "デフォルト キー");
				define("DATABASE_CURRENT_KEY_CUSTOM", "カスタム キー");
			define("DATABASE_PTUPLOAD", "アップロード...");
			
	     define ("KEY_MANAGEMENT_BUTTON", "キーのダウンロード...");
	     define ("ADDKEY_DIALOG_TITLE", "新しい暗号化キーの追加");
	     define ("ADDKEY_DIALOG_MESSAGE", "暗号化キーの詳細を次に入力してください。");
	     define ("ADDKEY_DIALOG_CONTROLS", "<table style=\"width:100%\"><tr style=\"height:2px\" /> <tr><td>Host Name :</td> <td><input id=\"keyHostName\" type=\"text\" name=\"keyHostName\" style=\"width:120px\" onblur=\"checkHostName()\"></td> <td /></tr> <tr style=\"height:2px\" /> <tr><td>Validity Period : </td><td><input id=\"keyValidPeriod\" type=\"text\" name=\"keyValidity\" style=\"width:120px\" onblur=\"checkValidity()\"></td><td>in days(5 to 3650)</td></tr> <tr style=\"height:2px\" /></table>");						
	     define ("ADDING_NEW_KEY", "新しいキーを追加中...");
	     define ("DELETEALLKEY_DIALOG_TITLE", "すべてのホスト キーの削除");
	     define ("DELETEALLKEY_DIALOG_MESSAGE", "デバイスのすべてのホスト キーを削除しようとしています。");
	     define ("DELETING_ALL_KEYS", "すべてのキーを削除中...");
 	     define ("DELETEKEY_DIALOG_TITLE", "ホスト キーの削除");
	     define ("DELETEKEY_DIALOG_MESSAGE", "デバイスのホスト キーを削除しようとしています。");
    	     define ("DELETING_HOST_KEY", "ホスト キーを削除中...");
            define ("DOWNLOADING_KEY", "キーのダウンロード中...");
	     define ("REGENERATEKEY_DIALOG_TITLE", "".$NXTText." キーの再生成");
	     define ("REGENERATEKEY_DIALOG_MESSAGE", "".$NXTText." キーを再生成しようとしています。これにより、以前にダウンロードしたキーはすべて無効になります。");
	     define ("REGENERATING_NANO_KEY", "".$NXTText." キーを再生成しています...");
	     define ("STARTING_EYELOCK_APPLICATION", "Eyelock アプリケーションを起動しています...");
            define ("IDENTIFY_DEVICE_TITLE", "デバイスの識別中");
            define ("IDENTIFY_DEVICE_MESSAGE", "デバイスの LED を繰り返し点滅中...");
            define ("IDENTIFY_DEVICE_MESSAGE2", "[キャンセル] をクリックすると中止します。");
            define ("RESETTING_DEVICE_MESSAGE", "デバイスをリセット中...お待ちください...");
            define ("REBOOTING_DEVICE_MESSAGE", "デバイスをリブート中...お待ちください...");
            define ("REBOOTING_DEVICE_MESSAGE2", "1、2 分かかる場合があります...");
            define ("WAITING_FOR_EYELOCK_RESTART", "Eyelock アプリケーションを再起動中...お待ちください...");
            define ("DEVICE_TIME_SYNCHRONIZING", "デバイス時刻を同期中...お待ちください...");
            define ("DEVICE_TIME_SYNCHRONIZED", "デバイスの時刻が正常に設定されました。");
            define ("DEVICE_TIME_SYNCFAILED", "デバイスの時刻を設定できませんでした。");
            define ("FACTORYRESET_DEVICE", "デバイスを出荷時のデフォルトにリセット中...");
            define ("RESTORE_DEVICE", "バックアップからデバイスを復元中...");
            define ("RESTORE_DEVICE_TITLE", "デバイス ファームウェアの復元");
            define ("RESTORE_DEVICE_DELETETITLE", "復元ポイントの削除");
            define ("RESTORE_DEVICE_DELETEMSG", "デバイスの復元ポイントを削除中...");
			define("RESTORE_FAILED_NOT_SUPPORTED", "復元ポイントを復元できませんでした。バージョンがサポートされなくなりました");
			define("RESTORE_FAILED_NO_FILE", "復元ポイントの処理に失敗しました。Nano の復元ポイントが見つかりませんでした。");
            define ("AUTOMATIC_LOGOUT", "今すぐログアウトされます。");
            define ("LOGOUT_MESSAGE", "ログアウトしています...お待ちください...");
            define ("REBOOT__DEVICE_TITLE", "デバイスのリブート");
            define ("REBOOT_DEVICE_WARNING", "[はい] をクリックすると、デバイスがリブートします...");
            define ("MSG_AREYOUSURE", "続行しますか?");
            define ("FACTORY_RESET_TITLE", "出荷時の設定にリセットの確認");
			
			 define ("OSDPINSTALLMODE_TITLE", "OSDP インストール モードの設定確認");
			
            define ("FACTORY_RESET_WARNING", "この処理は元に戻せません...");
            define ("FIRMWARE_UPDATE_NANOTITLE", "".$NXTText." のファームウェアを更新中...お待ちください...");
			
			define ("PT_FIRMWARE_UPDATE_NANO_TITLE","ポータブル テンプレート リーダーのファームウェアを更新中...お待ちください...");
			  define ("PT_FIRMWARE_UPDATE_STATUS_UPLOAD","ポータブル テンプレート リーダーのファームウェアをアップロード中");
			  define ("PT_FIRMWARE_UPDATE_STATUS_UPDATING_BOB","ポータブル テンプレート リーダーのファームウェアを更新中");

			  define ("PT_FIRMWARE_UPDATE_TITLE","ポータブル テンプレート リーダーのファームウェアの更新");
			  define ("PT_FIRWMARE_UPDATE_SUCCESS","ポータブル テンプレート リーダーのファームウェアが正常に更新されました。");
			  define ("PT_FIRWMARE_UPDATE_RELOAD","[OK] をクリックすると、ページがリフレッシュされます。");
			  define ("PT_FIRWMARE_UPDATE_ERROR_FAILED","ポータブル テンプレート リーダーのファームウェアを更新できませんでした。");
			  define ("PT_FIRWMARE_UPDATE_FAILED","ポータブル テンプレート リーダーのファームウェアを更新できませんでした。");
			
			
			define("PT_FIRMWARE_MANAGEMENT_TITLE", "ポータブル テンプレート リーダーのファームウェアの管理");
			define("PT_FIRWMARE_UPDATE_WAITING", "ポータブル テンプレート リーダーの再起動を待機中...");
			
			
			
			define ("UPGRADE_NOT_ALLOWED", "アップグレードに失敗しました。アップグレード バージョンがサポートされていません");
            define ("FIRMWARE_UPDATESTATUS_UPLOAD", "パッケージをデバイスにアップロード中...");
            define ("FIRMWARE_UPDATESTATUS_DOWNLOAD", "パッケージをサーバーからダウンロード中: ");
            define ("FIRMWARE_UPDATESTATUS_UNPACK", "ファイルをアンパック中...");
            define ("FIRMWARE_UPDATESTATUS_VALIDATING", "".$NXTText." の画像を検証中...");
            define ("FIRMWARE_UPDATESTATUS_COPYING", "必要なファイルをコピー中...");
            define ("FIRMWARE_UPDATESTATUS_RESTOREPOINT", "".$NXTText." の復元ポイントを作成中...");
            define ("FIRMWARE_UPDATESTATUS_UPDATING", "".$NXTText." の更新を実行中...");
            define ("FIRMWARE_UPDATESTATUS_VALIDATINGBOB", "ICM の画像を検証中...");
            define ("FIRMWARE_UPDATESTATUS_RESTOREPOINTBOB", "ICM の復元ポイントを作成中...");
            define ("FIRMWARE_UPDATESTATUS_UPDATINGBOB", "ICM の更新を実行中...");
            define ("FIRMWARE_UPDATESTATUS_COMPLETE", "ファームウェアの更新完了");
            define ("FIRMWARE_UPDATESTATUS_RESTORESETTINGS", "デバイスの設定を復元中...");
            define ("FIRMWARE_UPDATE_TITLE", "ファームウェアの更新結果");
            define ("FIRMWARE_UPDATE_FAILEDTITLE", "ファームウェアの更新失敗");
	     define ("FIRMWARE_UPDATE_FAILEDMESSAGE", $NXTText." のファームウェアを更新できませんでした。");
            define ("FIRMWARE_UPDATE_SUCCESS", "ファームウェアが正常に更新されました。");
            define ("FIRMWARE_UPDATE_RELOAD", "[OK] をクリックすると、デバイスがリブートします...");
            define ("FIRMWARE_UPDATEERROR_BADFILETYPE", "アップロードされたファイルが有効なファームウェア パッケージではありません。");
            define ("FIRMWARE_UPDATEERROR_UNPACKFAILED", "ファームウェア パッケージをアンパックできませんでした。パッケージが破損しているか、デバイスの容量がなくなった可能性があります。");
            define ("FIRMWARE_UPDATEERROR_VALIDATEFAILED", "パッケージの内容を検証できませんでした。内容が破損している可能性があります。");
            define ("FIRMWARE_UPDATEERROR_RESTOREPOINTFAILED", "復元ポイントを作成できませんでした。デバイスの容量がなくなった可能性があります。");
            define ("FIRMWARE_UPDATEERROR_INSTALLFAILED", "デバイスにファームウェアを完全に抽出できませんでした。デバイスの容量がなくなった可能性があります。");
            define ("FIRMWARE_UPDATEERROR_BOBINSTALLFAILED", "ブレークアウト ボードのファームウェアの更新を正常にインストールできませんでした。");
            define ("FIRMWARE_UPDATEERROR_DEVICERESTOREFAILED", "デバイスの設定を復元できませんでした。");
            define ("FIRMWARE_UPDATEERROR_SLAVECOPYFAILED", "スレーブ デバイスにファイルをコピーできませんでした。デバイスの容量がなくなった可能性があります。");
            define ("FIRMWARE_UPDATEERROR_SLAVEINSTALLFAILED", "スレーブ デバイスをアップグレードできませんでした。");
            define ("FIRMWARE_UPDATEERROR_UNKNOWNFAILED", "デバイスの更新中に不明なエラーが発生しました。");
            define ("DATABASE_DETAILSUNAVAILABLE", "詳細は不明です。");
            define ("NANO_DEVICE_STATUSTITLE", $NXTText." デバイス ステータス");
            define ("NANO_DEVICE_CONNDOWN", "通信できません。デバイスの電源が入っていることを確認してください。");
            define ("NANO_DEVICE_RECONNECT", "デバイスを確認してから、[OK] をクリックしてステータスを確認してください。");

            //Tooltip text
            define ("TOOLTIP_LOGIN_installer", "<b>Installer</b><br><b><i>Installer</i></b> ユーザー タイプには、すべての編集権限があります。");
            define ("TOOLTIP_LOGIN_SITEADMIN", "<b>Admin</b><br><b><i>Admin</i></b> ユーザー タイプには、すべての読み取り権限がありますが、書き込み権限に制限があります。");
            define ("TOOLTIP_LOGIN_CHANGEPASSWORD", "パスワードを変更するユーザー タイプを選んでください。");

            define ("TOOLTIP_HOME_TAB", "<b><i>[ホーム] タブ</i></b> <br> ".$NXTText." の基本情報 (デバイス情報、ソフトウェア情報およびデータベース ストレージの統計情報) が表示されます。");

            define ("TOOLTIP_NETWORK_TAB", "<b><i>[ネットワーク] タブ</i></b><br>ネットワーク パラメーター (DHCP 設定、IP アドレス、ホスト名など) の構成。 )");
            define ("TOOLTIP_NETWORK_NAME", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;デバイス名&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;&lt;b&gt;&lt;i&gt;[名前]&lt;/i&gt;&lt;/b&gt; フィールドは、ネットワークで DHCP または静的 IP を実行しているときの ".$NXTText." のホスト名を表します。&lt;/li&gt;&lt;li&gt;有効な&lt;i&gt;デバイス名&lt;/i&gt;は、文字と数字のみを含み、64 文字未満である必要があります。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_DHCP", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;IP アドレスの自動取得 (DHCP)&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このセクションでは、DHCP を介して&lt;i&gt;デバイス名&lt;/i&gt;を使って ".$NXTText." を使えるようにします。&lt;/li&gt;&lt;li&gt;DHCP を使うと、IP アドレスのかわりに&lt;i&gt;デバイス名&lt;/i&gt;を入力して ".$NXTText." にアクセスできるようになります。&lt;/li&gt;&lt;li&gt;一部のネットワークは DHCP を使うように構成されています。DHCP を使うようにネットワークが構成されていない場合、静的 IP を使って Web Config にアクセスできます。&lt;/li&gt;&lt;/ul&gt;");
             define ("TOOLTIP_NETWORK_SSLPROTOCOL", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;SSL プロトコル&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;".$NXTText." が通信に使う SSL プロトコルを選びます。&lt;/li&gt;
                                             &lt;li&gt;レガシー: SSL 3.0 または TLS 1.2 を使えます&lt;/li&gt;
                                             &lt;li&gt;TLS 1.2: TLS 1.2 のみを使えます&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_ADVDHCPBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;DHCP 設定ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、DHCP の詳細設定にアクセスできます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_STATICIP", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;次の静的 IP アドレスを使用&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションでは、指定した静的 IP アドレスを使って ".$NXTText." をネットワークで使えるようにします。&lt;/li&gt;&lt;li&gt;構成する IP アドレスは、ネットワークの他の場所で使われていないことが必要です。&lt;/li&gt;&lt;li&gt;IP アドレスがすでに使用中の場合は、".$NXTText." をネットワークで使用できず、&lt;i&gt;デバイス名&lt;/i&gt;も表示されません。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_DEVICEIP", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;デバイス IP&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;静的 IP アドレスをここに入力します。&lt;/li&gt;&lt;li&gt;使用可能な IP アドレスがあるかどうかは、必要に応じてローカルのネットワーク管理者に問い合わせてください。&lt;/li&gt;&lt;li&gt;この設定が適用されるのは、上記の &lt;i&gt;[次の静的 IP アドレスを使用]&lt;/i&gt; を選んだ場合のみです。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_BROADCASTNETWORK", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ブロードキャスト ネットワーク&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;正しい値は、必要に応じてローカルのネットワーク管理者に問い合わせてください。&lt;/li&gt;&lt;li&gt;この設定が適用されるのは、上記の &lt;i&gt;[次の静的 IP アドレスを使用]&lt;/i&gt; を選んだ場合のみです。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_SUBNETMASK", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;サブネット マスク&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;正しい値は、必要に応じてローカルのネットワーク管理者に問い合わせてください。&lt;/li&gt;&lt;li&gt;この設定が適用されるのは、上記の &lt;i&gt;[次の静的 IP アドレスを使用]&lt;/i&gt; を選んだ場合のみです。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_DEFAULTGATEWAY", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;デフォルト ゲートウェイ&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;正しい値は、必要に応じてローカルのネットワーク管理者に問い合わせてください。&lt;/li&gt;&lt;li&gt;この設定が適用されるのは、上記の &lt;i&gt;[次の静的 IP アドレスを使用]&lt;/i&gt; を選んだ場合のみです。&lt;/li&gt;&lt;/ul&gt;");
 	        define ("TOOLTIP_NETWORK_DNS", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;DNS (Domain Name System) サーバー&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;正しい値は、必要に応じてローカルのネットワーク管理者に問い合わせてください。&lt;/li&gt;&lt;li&gt;この設定が適用されるのは、上記の &lt;i&gt;[次の静的 IP アドレスを使用]&lt;/i&gt; を選んだ場合のみです。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_ENABLEIEEE8021X", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;IEEE 802.1X のセキュリティ を有効にする&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;正しい値は、必要に応じてローカルのネットワーク管理者に問い合わせてください。&lt;/li&gt;&lt;li&gt;この設定では、IEEE 802.1X ネットワーク プロトコルが ".$NXTText." で有効になります。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_CACERT", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;CA 証明書&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;右側の [アップロード] ボタンを使って、CA 証明書ファイルを参照してデバイスにアップロードします。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_UPLOADCACERTIFICATE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;CA 証明書のアップロード&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このボタンを使って、CA 証明書ファイルを参照してデバイスにアップロードします。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_CLIENTCERT",  "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;クライアント証明書&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;右側の [アップロード] ボタンを使って、クライアント証明書ファイルを参照してデバイスにアップロードします。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_UPLOADCLIENTCERTIFICATE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;クライアント証明書のアップロード&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このボタンを使って、クライアント証明書ファイルを参照してデバイスにアップロードします。&lt;/li&gt;&lt;/ul&gt;"); 
            define ("TOOLTIP_NETWORK_CLIENTPRIVATEKEY",  "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;クライアント秘密キー&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;右側の [アップロード] ボタンを使って、クライアント秘密キー ファイルを参照してデバイスにアップロードします。&lt;/li&gt;&lt;li&gt;クライアント キーとパスワードを組み合わせた (PEM) ファイルをアップロードすることもできます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_NETWORK_UPLOADPRIVATEKEY", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;クライアント証明書のアップロード&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このボタンを使って、クライアント秘密キーファイルを参照してデバイスにアップロードします。&lt;/li&gt;&lt;li&gt;クライアント キーとパスワードを組み合わせた (PEM) ファイルをアップロードすることもできます。&lt;/li&gt;&lt;/ul&gt;"); 
            define ("TOOLTIP_NETWORK_EAPOLVERSION", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;EAPOL バージョン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;使用する EAPOL プロトコルのバージョンをドロップダウンを使って指定します。&lt;/li&gt;&lt;/ul&gt;"); 
            define ("TOOLTIP_NETWORK_EAPIDENTITY", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;EAP ID&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;EAP ID を指定します。&lt;/li&gt;&lt;/ul&gt;"); 
            define ("TOOLTIP_NETWORK_PRIVTEKEYPWD",  "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;秘密キー パスワード&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;秘密キー パスワードを指定します。&lt;/li&gt;&lt;/ul&gt;"); 
            define ("TOOLTIP_NETWORK_DOWNLOADLOGBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[802.1X ログのダウンロード] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、".$NXTText." の &lt;i&gt;IEEE 802.1X ログ&lt;/i&gt;がテキスト ファイルとしてダウンロードされます。&lt;/li&gt;&lt;/ul&gt;");


            //////////////////////////////////////////////////////
            // ADV. DHCP DIALOG HELP STRINGS
            //////////////////////////////////////////////////////
            define ("TOOLTIP_ADVDHCP_TIMEOUT", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;DHCP タイムアウト&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;".$NXTText." DHCP クライアントが ".$NXTText." の&lt;i&gt;ホスト名&lt;/i&gt; を解決できないと判断するまでの秒数を指定します。&lt;/li&gt;&lt;li&gt;期間範囲 (秒): 10 - 120。&lt;/li&gt;&lt;li&gt;デフォルト値: 10。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ADVDHCP_RETRIES", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;DHCP 再試行&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;".$NXTText." DHCP クライアントが ".$NXTText." の&lt;i&gt;ホスト名&lt;/i&gt;の解決に失敗した場合の合計試行回数を指定します。&lt;/li&gt;&lt;li&gt;試行回数: 0 - 5。&lt;/li&gt;&lt;li&gt;デフォルト値: 0。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ADVDHCP_RETRYDELAY", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;再試行遅延&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;DHCP クライアントの再試行の間隔を秒数で指定します。&lt;/li&gt;&lt;li&gt;期間範囲 (秒): 0 - 60。&lt;/li&gt;&lt;li&gt;デフォルト値: 10。&lt;/li&gt;&lt;/ul&gt;");

            define ("TOOLTIP_IPV6_ENABLE", "<span class='tooltip-header'>IPv6 を有効にする</span>");
            define ("TOOLTIP_IPV6_CONFIGURATION", "<span class='tooltip-header'>IPv6 構成詳細の指定</span>");
            define ("TOOLTIP_IPV6_DHCP_MODE", "<span class='tooltip-header'>IPv6 DHCP モード:</span><ul class='tooltip-list'><li><b>ステートレス</b> &ndash; DNS サーバーや NTP サーバーなどの設定に DHCPv6 を使いますが、IPv6 アドレスの設定には使いません。</li><li><b>ステートフル</b> - IPv6 アドレスおよび DNS サーバーなどの設定に DHCPv6 を使います。</li><li><b>自動</b> &ndash; ルーター アドバタイズメントによって DHCPv6 の使用が決まります。</li><li><b>オフ</b> - DHCPv6 が無効になります。</li></ul>");
            define ("TOOLTIP_IPV6_ADDRESS", "<span class='tooltip-header'>IPv6 ネットワーク アドレス</span>");
            define ("TOOLTIP_IPV6_GATEWAY", "<span class='tooltip-header'>IPv6 デフォルト ゲートウェイ アドレス</span>");
            define ("TOOLTIP_IPV6_DNS1", "<span class='tooltip-header'>IPv6 プライマリ DNS サーバー アドレス</span>");
            define ("TOOLTIP_IPV6_DNS2", "<span class='tooltip-header'>IPv6 セカンダリ DNS サーバー アドレス</span>");
			define ("TOOLTIP_IPV6_STATUS", "&lt;span class=&apos;tooltip-header&apos;&gt;EXT デバイスに現在割り当てられている IPv6 アドレス&lt;/span&gt;&lt;ul class=&apos;tooltip-list&apos;&gt;&lt;li&gt;&lt;b&gt;グローバル&lt;/b&gt; &amp;ndash; グローバルに有効なアドレス。&lt;/li&gt;&lt;li&gt;&lt;b&gt;リンクローカル&lt;/b&gt; - EXT デバイスが接続されているネットワーク セグメントまたはブロードキャスト ドメイン内の通信のみに有効なアドレス。&lt;/li&gt;&lt;li&gt;&lt;b&gt;ステートレス&lt;/b&gt; &amp;ndash; ステートレス アドレス構成のためにインストールされたアドレス。&lt;/li&gt;&lt;li&gt;&lt;b&gt;一時&lt;/b&gt; - 一時ランダム アドレス (RFC 4941 に記載)。&lt;/li&gt;&lt;/ul&gt;");

            define ("TOOLTIP_DEVICE_TAB", "<b><i>[デバイス] タブ</i></b><br>".$NXTText." デバイスの構成。(LED の明るさ、音量調節、時刻設定、リブート、出荷時設定にリセット)");
            define ("TOOLTIP_DEVICE_SPEAKERVOLUME", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;スピーカー音量&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;".$NXTText." のスピーカー音量を調節します。&lt;/li&gt;&lt;li&gt;値を &amp;apos;0&amp;apos; に設定するとスピーカーが無音になります。&lt;/li&gt;&lt;li&gt;音量範囲: 0 - 100。&lt;/li&gt;&lt;li&gt;デフォルト値: 40。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_DEVICE_LEDBRIGHTNESS", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;LED の明るさ&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;".$NXTText." の LED の明るさレベルを調節します。&lt;/li&gt;&lt;li&gt;値を &amp;apos;0&amp;apos; に設定すると LED がオフになります。&lt;/li&gt;&lt;li&gt;明るさ範囲: 0 - 100。&lt;/li&gt;&lt;li&gt;デフォルト値: 20.。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_DEVICE_TAMPERVOLUME", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;改ざんトーン音量&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;&lt;i&gt;改ざんアラーム&lt;/i&gt;の音量レベルを調節します。&lt;/li&gt;&lt;li&gt;値を &amp;apos;0&amp;apos; に設定するとアラームが無音になります。&lt;/li&gt;&lt;li&gt;音量範囲: 0 - 100。&lt;/li&gt;&lt;li&gt;デフォルト値: 10。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_DEVICE_TIMESERVER", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;インターネット タイム サーバー&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;現在の時刻を取得する&lt;i&gt;インターネット タイム サーバー&lt;/i&gt;の Web アドレス。&lt;/li&gt;&lt;li&gt;この機能が動作するために、".$NXTText." が&lt;i&gt;インターネット タイム サーバー&lt;/i&gt;にアクセスできる&lt;b&gt;&lt;i&gt;必要があります&lt;/i&gt;&lt;/b&gt;。&lt;/li&gt;&lt;li&gt;デフォルト値: time.nist.gov。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_DEVICE_SYNCHRONIZEDAILY", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;毎日時刻を同期&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションでは、指定された&lt;i&gt;インターネット タイム サーバー&lt;/i&gt;と ".$NXTText." が 1 日に 1 回同期できるようにします。&lt;/li&gt;&lt;li&gt;デフォルト値: 有効。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_DEVICE_LOCATEDEVICE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[デバイスの発見] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると ".$NXTText." の LED が点滅します。&lt;/li&gt;&lt;li&gt;ユーザーが操作をキャンセルするまで、LED は点滅し続けます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_DEVICE_SYNCHRONIZENOW", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[今すぐ同期] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、指定した&lt;i&gt;インターネット タイム サーバー&lt;/i&gt;と ".$NXTText." の時刻が即座に同期されます。&lt;/li&gt;&lt;/ul&gt;");
	    define ("TOOLTIP_DEVICE_SYNCHRONIZEWITHHOST", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[ホストと同期] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、現在のホストまたはシステムの時刻と ".$NXTText." の時刻が即座に同期されます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_DEVICE_FACTORYRESET", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[出荷時の設定にリセット] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると ".$NXTText." が&lt;i&gt;出荷時&lt;/i&gt;の設定にリセットされます。&lt;/li&gt;&lt;li&gt;".$NXTText." が出荷時の設定にリセットされると、&lt;/li&gt;&lt;ol&gt;&lt;li&gt;すべての設定がデフォルト値にリセットされます。&lt;/li&gt;&lt;li&gt;ネットワーク設定がデフォルト (DHCP) に戻ります。&lt;/li&gt;&lt;li&gt;".$NXTText." のデータベースがクリアされます。&lt;/li&gt;&lt;li&gt;".$NXTText." に保存されているログ ファイルがクリアされます。&lt;/li&gt;&lt;li&gt;".$NXTText." がリブートします。&lt;/li&gt;&lt;/ol&gt;&lt;/ul&gt;");
			
			 define ("TOOLTIP_ACS_INSTALLATION_MODE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;セキュア モード OSDP インストール モード&lt;/span&gt;&amp;quot;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、".$NXTText." がセキュア モード OSDP インストール モードに設定されます。&lt;/li&gt;
                                                &lt;li&gt;このモードでは、".$NXTText." がセキュア モード OSDP SCBK を制御パネルから受信できます。&lt;/li&gt;&lt;ol&gt;
                                                &lt;li&gt;このモードでは、新しい制御パネルの使用時または新規インストール時にセキュア モード OSDP を使用する必要があります。&lt;/li&gt;
												&lt;li&gt;このオプションを確定すると、現在保存されている SCBK が削除されます。&lt;b&gt;この操作は元に戻せません&lt;/b&gt;&lt;/li&gt;
                                                &lt;/ol&gt;&lt;/ul&gt;");
			
            define ("TOOLTIP_DEVICE_REBOOTDEVICE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[リブート] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、".$NXTText." のソフト リブートが実行されます。&lt;/li&gt;&lt;li&gt;リブートすると、WebConfig が ".$NXTText." から切断されます。&lt;/li&gt;&lt;li&gt;リブート シーケンスが進んで、WebConfig が ".$NXTText." と再接続されるまでお待ちください。&lt;/li&gt;&lt;li&gt;WebConfig が 2 分以内に自動的に再接続されない場合は、".$NXTText." が動作していることを確認した後、ブラウザーでこのページをリフレッシュしてください。&lt;/li&gt;&lt;/ul&gt;");


            define ("TOOLTIP_SECURITY_TAB", "&lt;b&gt;&lt;i&gt;[セキュリティ] タブ&lt;/i&gt;&lt;/b&gt;&lt;br&gt;".$NXTText." のセキュリティ構成。(キー管理、パスワードのリセット、改ざん設定)");
            define ("TOOLTIP_SECURITY_TAMPERACTIVEHIGH", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;高信号で改ざんをアクティブにする&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;2 要素認証のシステムにカード リーダーが取り付けられている場合は、この設定がカード リーダーの改ざん出力と一致している必要があります。&lt;/li&gt;&lt;li&gt;&amp;apos;カード リーダーの改ざん状態が高信号でアクティブになる場合は、[高信号で改ざん状態をアクティブにする] を選んでください。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_SECURITY_TAMPERACTIVELOW", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;低信号で改ざんをアクティブにする&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;2 要素認証のシステムにカード リーダーが取り付けられている場合は、この設定がカード リーダーの改ざん出力と一致している必要があります。&lt;/li&gt;&lt;li&gt;&amp;apos;カード リーダーの改ざん状態が低信号でアクティブになる場合は、[低信号で改ざん状態をアクティブにする] を選んでください。&lt;/li&gt;&lt;/ul&gt;");
             define ("TOOLTIP_OUTPUT_SECURITY_TAMPERACTIVEHIGH", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;高信号&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;改ざんが検知された場合の出力を「高」に設定します。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_OUTPUT_SECURITY_TAMPERACTIVELOW", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;低信号で改ざんをアクティブにする&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;改ざんが検知された場合の出力を「低」に設定します。&lt;/li&gt;&lt;/ul&gt;");


            define ("TOOLTIP_SECURITY_TAMPERNOTIFYADDR", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;通知アドレス&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;&lt;i&gt;改ざんメッセージ&lt;/i&gt;を受信するシステムやアプリケーションの IP アドレスを指定します&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_SECURITY_TAMPERNOTIFYPORT", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;通知ポート&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;&lt;i&gt;改ざんメッセージ&lt;/i&gt;を受信するシステムやアプリケーションのポートを指定します&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_SECURITY_TAMPERMESSAGE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;改ざんメッセージ&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;".$NXTText." で改ざんが発生した場合に上記の場所に送信するメッセージを指定します。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_SECURITY_OLDPWD", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;古いパスワード&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;既存の ".$NXTText." ログオン パスワードをこのフィールドに入力します。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_SECURITY_NEWPWD", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;新しいパスワード&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;希望する ".$NXTText." ログオン パスワードをこのフィールドに入力します。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_SECURITY_CONFIRMPWD", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;パスワードの確認&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;希望する ".$NXTText." ログオン パスワードをこのフィールドにもう一度入力します。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_SECURITY_UPDATEPWDBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[パスワードのリセット] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると ".$NXTText." のパスワードがリセットされます。&lt;/li&gt;&lt;li&gt;パスワードがリセットされると、Web Config のログオン画面が表示されます。新しいパスワードを入力してログインしてください。&lt;/li&gt;&lt;/ul&gt;");
	     define ("TOOLTIP_SECURITY_NANONXT_TEXT", $NXTText."デバイス");
	     define ("TOOLTIP_SECURITY_PC_TEXT", "ホスト システム");
	     define ("TOOLTIP_SECURITY_VALIDKEY_TEXT", "有効なキー");
	     define ("TOOLTIP_SECURITY_INVALIDKEY_TEXT", "キーが無効または期限切れです");
	     define ("TOOLTIP_SECURITY_REGENERATEKEY_TEXT", "キーの再生成");
	     define ("TOOLTIP_SECURITY_DELETEKEY_TEXT", "キーの削除");
	     define ("TOOLTIP_SECURITY_DOWNLOADKEY_TEXT", "キーのダウンロード");
	     define ("TOOLTIP_SECURITY_KEYMGMTBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;キーのダウンロード&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;カスタム キーをこのコンピューターにダウンロードします。&lt;/li&gt;&lt;li&gt;".$NXTText." を [デフォルト キーの使用] から [カスタム キーの使用] に切り替えるたびにキーをダウンロードする必要があります。&lt;/ul&gt;");
	     define ("TOOLTIP_SECURITY_ADDHOSTKEYBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;カスタム キーの追加&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、新しいカスタム キーが ".$NXTText." に追加されます。&lt;/li&gt;&lt;/ul&gt;");
	     define ("TOOLTIP_SECURITY_DELETEALLHOSTKEYSBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;カスタム キーの削除&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、".$NXTText." にあるカスタム キーが削除されます。&lt;/li&gt;&lt;/ul&gt;");
    
        define ("TOOLTIP_SECURITY_KEYMGMTDEFAULTRADIO", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;デフォルト キーの使用&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;Eyelock 製品とともに提供されるデフォルト キーを使って ".$NXTText." と対話します。&lt;/li&gt;&lt;li&gt;&lt;a id=&apos;DefaultKeyExpiry&apos;&gt;デフォルト キーは 2025 年 3 月 19 日に期限が切れます&lt;/a&gt;&lt;/li&gt;&lt;/ul&gt;");
         define ("TOOLTIP_SECURITY_KEYMGMTCUSTOMRADIO", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;カスタム キーの使用&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;対話に使用するカスタム キーが ".$NXTText." で生成されます。キーをダウンロードして、Eyelock アプリケーションに割り当ててください。&lt;/li&gt;&lt;/ul&gt;");

        define ("TOOLTIP_SECURITY_KEYMGMTVALIDITY", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;キーの有効期限...&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;表示される日時までカスタム キーが有効です。&lt;/li&gt;&lt;/ul&gt;");

            define ("TOOLTIP_SOFTWARE_TAB", "&lt;b&gt;&lt;i&gt;[ソフトウェア] タブ&lt;/i&gt;&lt;/b&gt;&lt;br&gt;".$NXTText." のファームウェアを構成します。(ファームウェアの更新、ファームウェアの復元)");
            define ("TOOLTIP_SOFTWARE_UPDATENOWBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[今すぐ更新] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;新しいバージョンの ".$NXTText." のファームウェアが Eyelock.com Web サイトで入手可能になると、このボタンが表示されます。&lt;/li&gt;&lt;li&gt;クリックすると、 ソフトウェアの自動アップグレード プロセスが開始します。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_SOFTWARE_LOCALFILEBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[ローカル ファイル...] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このボタンをクリックすると、ローカルでアクセス可能な Eyelock アップグレード パッケージで ".$NXTText." のファームウェアをアップグレードできます。&lt;/li&gt;&lt;li&gt;このボタンをクリックして、パッケージ ファイルを選択すると、アップグレード プロセスが始まります。&lt;/li&gt;&lt;li&gt;&lt;strong&gt;警告: 更新中は、Nano の電源を切断したり、ワークステーションのネットワークを切断したり、ブラウザー ウィンドウを閉じたりしないでください。&lt;/strong&gt;&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_SOFTWARE_PTLOCALFILEBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ローカル ファイル... ポータブル テンプレート リーダーのファームウェアの管理&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このボタンをクリックすると、ローカルでアクセス可能なファームウェア イメージ (.bin ファイル) でポータブル テンプレート リーダーのファームウェアをアップグレードできます。&lt;/li&gt;&lt;li&gt;このボタンをクリックして、イメージ ファイルを選択すると、アップグレード プロセスが始まります。&lt;/li&gt;&lt;li&gt;&lt;strong&gt;警告: 更新中は、Nano の電源を切断したり、ワークステーションのネットワークを切断したり、ブラウザー ウィンドウを閉じたりしないでください。&lt;/strong&gt;&lt;/li&gt;&lt;/ul&gt;");
			
			define ("TOOLTIP_SOFTWARE_DETAILSBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[詳細...] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このボタンをクリックすると、".$NXTText." のアップグレード ダイアログに、現在使用可能なアップグレード パッケージ ファイルの詳細が表示されます。&lt;/li&gt;&lt;li&gt;[詳細] 画面からアップグレードを開始してください。&lt;/li&gt;&lt;/ul&gt;");

            define ("TOOLTIP_SOFTWARE_ALLOWSITEADMINUPDATE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;管理者にデバイスの更新を許可&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションを有効にすると、&lt;i&gt;admin&lt;/i&gt; でログインしたユーザーが ".$NXTText." をアップグレードできるようになります。&lt;/li&gt;&lt;li&gt;このオプションは &lt;i&gt;installer&lt;/i&gt; に対してのみ表示されます。&lt;/li&gt;&lt;/ul&gt;");

            define ("TOOLTIP_SOFTWARE_NANORESTORE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;".$NXTText." の復元ポイント&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションを選ぶと、使用可能な ".$NXTText." ファームウェアの復元ポイントのリストが表示されます。&lt;/li&gt;&lt;li&gt;復元ポイントの横のチェック ボックスを選んで、&lt;i&gt;[復元...]&lt;/i&gt; または &lt;i&gt;[削除...]&lt;/i&gt; ボタンをクリックすれば、復元ポイントを復元または削除できます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_SOFTWARE_BOBRESTORE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;".$NXTText." ブレークアウト ボードの復元ポイント&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションを選ぶと、使用可能な ".$NXTText." のブレークアウト ボードのファームウェア復元ポイントのリストが表示されます。&lt;/li&gt;&lt;li&gt;復元ポイントの横のチェック ボックスを選んで、&lt;i&gt;[復元...]&lt;/i&gt; または &lt;i&gt;[削除...]&lt;/i&gt; ボタンをクリックすれば、復元ポイントを復元または削除できます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_SOFTWARE_RESTOREBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;復元ポイント復元ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、".$NXTText." のファームウェアが以前の復元ポイントに戻ります。&lt;/li&gt;&lt;li&gt;選択した最初の復元ポイントが復元されます。&lt;/li&gt;&lt;li&gt;復元は自動プロセスなので一度開始すると停止できません。&lt;/li&gt;&lt;li&gt;".$NXTText." に復元ポイントを 5 個まで保持できます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_SOFTWARE_DELETEBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;復元ポイント削除ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このボタンをクリックすると、選択したすべての復元ポイントが ".$NXTText." から永続的に削除されます。&lt;/li&gt;&lt;li&gt;削除された復元ポイントは回復できません。&lt;/li&gt;&lt;/ul&gt;");


            define ("TOOLTIP_AUTHENTICATION_TAB", "&lt;b&gt;&lt;i&gt;[認証] タブ&lt;/i&gt;&lt;/b&gt;&lt;br&gt;".$NXTText." 認証パラメーターの構成。(照合モード、反復認証期間、照合失敗タイムアウトなど)");
            define ("TOOLTIP_AUTHENTICATION_SINGLEEYE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;片目認証&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションを選ぶと、片目の生体照合で正常に認証&lt;i&gt;できます&lt;/i&gt;。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_DUALEYE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;両目認証&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションを選ぶと、両目の生体照合を&lt;i&gt;しないと&lt;/i&gt;正常に認証できません。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_REPEATPERIOD", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;反復認証期間&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;正常に認証するために必要な&lt;i&gt;最小&lt;/i&gt;間隔。画質を確保するために 2 秒以上必要です。&lt;/li&gt;&lt;li&gt;範囲 (秒): 2  - 60。&lt;/li&gt;&lt;li&gt;デフォルト値: 4。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_ENABLENEGMATCHTIMEOUT", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;照合失敗タイムアウトを有効にする&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションを選ぶと、&lt;i&gt;照合失敗&lt;/i&gt;のタイムアウト規則の処理が有効になります。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_LOITERPERIOD", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;待機時間&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;".$NXTText." の認証がタイムアウトして照合失敗が報告されるまでの&lt;i&gt;最小&lt;/i&gt;期間を指定します。&lt;/li&gt;&lt;li&gt;範囲 (秒): 2  - 60。&lt;/li&gt;&lt;li&gt;デフォルト値: 6。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_NEGMATCHRESET", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;照合失敗リセット タイマー&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;照合に失敗した後に、".$NXTText." が別の認証を試みるまでに経過する必要がある&lt;i&gt;最小&lt;/i&gt;期間を秒数で指定します。&lt;/li&gt;&lt;li&gt;範囲 (秒): 2  - 60。&lt;/li&gt;&lt;li&gt;デフォルト値: 4。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_NETWORKMSGADDR", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ネットワーク メッセージ宛先 IP&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;&lt;i&gt;ネットワーク照合&lt;/i&gt;メッセージを受信するシステムやアプリケーションの IP アドレスを指定します。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_NETWORKMSGPORT", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;通知ポート&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;&lt;i&gt;ネットワーク照合&lt;/i&gt;メッセージを受信するシステムやアプリケーションのポートを指定します。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_NETWORKMSGMSG", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ネットワーク メッセージの形式&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;正常に照合や認証が行われた場合に、上記の場所に送信されるメッセージの形式文字列を指定します。&lt;/li&gt;&lt;li&gt;デフォルト値: 照合済:%d;スコア:%0.4f;時間:%llu;ID:&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_IRISPROCESSINGMODE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;虹彩処理モード&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;虹彩処理のモードを指定します。2 つのオプションがあります。&lt;/li&gt;&lt;li&gt;アクセス制御認証 - 照合機能を使ってユーザーの虹彩を認証し、オプションで ACS システムとインタフェースできます。&lt;/li&gt;&lt;li&gt;虹彩キャプチャ モード - 虹彩をキャプチャして、リモート HTTP エンドポイントに送信します。照合や ACS 統合は行われません。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_CAPTURETIMEOUT", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;虹彩キャプチャ タイムアウト&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;メッセージを HTTP エンドポイントに送信する前にユーザーの虹彩を処理する時間をミリ秒単位で指定します。&lt;/li&gt;&lt;li&gt;範囲 (ミリ秒): 1000  - 60000。&lt;/li&gt;&lt;li&gt;デフォルト値: 5000。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_CAPTURERESETDELAY", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;虹彩キャプチャ リセット遅延&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;虹彩をキャプチャした後、次のユーザーを処理するまでにデバイスが待機する遅延時間をミリ秒単位で指定します。&lt;/li&gt;&lt;li&gt;範囲 (ミリ秒): 1000  - 300000。&lt;/li&gt;&lt;li&gt;デフォルト値: 5000。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_MESSAGEFORMAT", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;HTTP メッセージの形式&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;HTTP エンドポイントに送信されるメッセージの形式を指定します。&lt;/li&gt;&lt;li&gt;デフォルト値: SOAP (SOAP メッセージのスキーマについては文書を参照してください)&lt;/li&gt;&lt;li&gt;代替値: JSON (JSON メッセージのスキーマについては文書を参照してください)&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_PAYLOADFORMAT", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;メッセージ画像ペイロード形式&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;HTTP エンドポイントに送信される虹彩画像のグラフィック形式を指定します。&lt;/li&gt;&lt;li&gt;オプション 1: RAW - 8 ビット、シングル チャネル画像&lt;/li&gt;&lt;li&gt;オプション 2: PNG&lt;/li&gt;&lt;li&gt;オプション 3: J2K (JPEG 2000)&lt;/li&gt;&lt;li&gt;デフォルト値: RAW - 8 ビット、シングル チャネル画像&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_IRISIMAGEQUALITY", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;虹彩画質&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;エンコードされた JPEG 2000 虹彩画像の画質を指定します。100 を指定すると無損失圧縮になります。&lt;/li&gt;&lt;li&gt;デフォルト値: 100&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_IRISCAPTURE_WECOMEMSG", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ようこそメッセージ&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;虹彩キャプチャ メッセージの一部として送信されるテキスト文字列を定義します。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_IRISCAPTURE_DEVICELOCATION", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;デバイスの場所&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;虹彩キャプチャ メッセージで、このデバイスに関連付ける独自の文字列を定義します。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_IRISCAPTURE_HEARTBEATURL", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ハートビート URL&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このデバイスのハートビート メッセージの送信先の URL を定義します。空にしておくと、ハートビート メッセージは送信されません。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_IRISCAPTURE_POSTSENDERURL", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;送信先アドレス&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;すべての虹彩キャプチャ メッセージの宛先のベース URL。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_IRISCAPTURE_ERRORURL", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;シグナル エラー エンドポイント&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;シグナル エラー メッセージの送信先アドレスのエンドポイント。空にしておくと、メッセージは送信されません。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_IRISCAPTURE_POSTIRISURL", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;虹彩送信エンドポイント&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;虹彩画像の送信先アドレスのエンドポイント。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_IRISCAPTURE_MAINTENANCEURL", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;保守エンドポイント&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;保守メッセージの送信先アドレスのエンドポイント。空にしておくと、メッセージは送信されません。&lt;/li&gt;&lt;/ul&gt;");

            define ("TOOLTIP_DATABASE_TAB", "&lt;b&gt;&lt;i&gt;[データベース] タブ&lt;/i&gt;&lt;/b&gt;&lt;br&gt;".$NXTText." のデータベース構成。(Network Matcher およびデータベース統計情報)");
            define ("TOOLTIP_DATABASE_ENABLENETWORKMATCHER", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;Network Matcher を有効にする&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションを選ぶと、&lt;i&gt;Eyelock Network Matcher サービス&lt;/i&gt;を使って、生体認証データをデバイス以外で追加処理できます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_DATABASE_NETWORKMATCHERADDR", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;Network Matcher 宛先アドレス&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;認証要求を処理する Eyelock Network Matcher サービスの IP アドレスを指定します。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_DATABASE_NETWORKMATCHERPORT", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;Network Matcher 宛先ポート&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;認証要求を処理する Eyelock Network Matcher サービスのポートを指定します。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_AUTHENTICATION_SECURENETWORK", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;安全なネットワーク照合メッセージを有効にする&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションを有効にすると、照合メッセージがネットワークで安全に送信されます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_DATABASE_STATISTICS", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;テンプレート領域残量&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;この値には、使用可能なテンプレートの最大数から ".$NXTText." のデータベースに現在保存されている生体認証テンプレート数を差し引いた値が表示されます。&lt;/li&gt;&lt;/ul&gt;");
			define ("TOOLTIP_DATABASE_NETWORKMATCHER_SECURECOMM", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;安全な Network Matcher を有効にする&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションを有効にすると、&lt;i&gt;Eyelock Network Matcher サービス&lt;/i&gt;と安全に通信できます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_UPLOAD_PTFIRMWARE", "ポータブル テンプレート リーダーのファームウェア (.bin) ファイルをアップロードします。");                       
            define ("TOOLTIP_ACS_TAB", "&lt;b&gt;&lt;i&gt;[ACS] タブ&lt;/i&gt;&lt;/b&gt;&lt;br&gt;ACS やドア パネル システムの ".$NXTText." の構成。 (プロトコル、ACS LED 制御、2 要素認証、リレー、ACS テスト オプション)");


            define ("TOOLTIP_LOGS_TAB", "&lt;b&gt;&lt;i&gt;[ログ] タブ&lt;/i&gt;&lt;/b&gt;&lt;br&gt;".$NXTText." のログの表示およびダウンロードが可能です。");
            define ("TOOLTIP_LOGS_REFRESHBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[リフレッシュ] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このボタンをクリックすると、ログの現在のコンテンツと&lt;i&gt;イベント ログ&lt;/i&gt;が同期されます。&lt;/li&gt;&lt;li&gt;&lt;i&gt;イベント ログ&lt;/i&gt;は、WebConfig を初めて実行したときに同期されます。継続的には更新されません。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_LOGS_DOWNLOADBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[ダウンロード] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、".$NXTText." の&lt;i&gt;イベント ログ&lt;/i&gt;が Microsoft Excel CSV ファイルとしてダウンロードされます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ACCESSCONTROLTYPE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;認証スキーム&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;".$NXTText." で使う認証スキームを選びます。&lt;/li&gt;&lt;li&gt;スキーム&lt;/li&gt;&lt;ul&gt;&lt;li&gt;虹彩のみ: ".$NXTText." がアクセス システムに虹彩認証のみを渡します&lt;/li&gt;&lt;li&gt;虹彩またはカード: カードの資格証明または虹彩認証のどちらかを ".$NXTText." がアクセス システムに送信します&lt;/li&gt;&lt;li&gt;虹彩とカード: カードの資格証明と (提示されたカードと一致する) 虹彩認証の両方が ".$NXTText." で必要になります&lt;/li&gt;&lt;li&gt;虹彩とカード (PIN パススルー): カードの資格証明と (提示されたカードと一致する) 虹彩認証の両方が ".$NXTText." で必要になり、PIN はアクセス システムにそのまま渡されます&lt;/li&gt;&lt;li&gt;PIN と虹彩: PIN と虹彩認証の両方を ".$NXTText." がアクセス システムに送信します&lt;/li&gt;&lt;li&gt;PIN とカードと虹彩: PIN とカードの資格証明と (提示されたカードと一致する) 虹彩認証を ".$NXTText." が送信します&lt;/li&gt;&lt;/ul&gt;&lt;/ul&gt;");

            define ("TOOLTIP_ACS_PROTOCOL", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ACS プロトコル&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このドロップダウンで、この ".$NXTText." で使う ACS プロトコルを選びす。&lt;/li&gt;&lt;li&gt;次のプロトコルがサポートされています。&lt;/li&gt;&lt;ul&gt;&lt;li&gt;Wiegand&lt;/li&gt;&lt;li&gt;HID シリアル&lt;/li&gt;&lt;li&gt;F2F&lt;/li&gt;&lt;li&gt;PAC&lt;/li&gt;&lt;li&gt;OSDP&lt;/li&gt;&lt;/ul&gt;&lt;/ul&gt;");
            define ("TOOLTIP_OSDP_BAUD", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;OSDP ボー レート&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;RS-485 入力と RS-485 出力両方の半二重非同期シリアル通信速度を制御します。信号のデフォルトはデータ 8 ビット、ストップ ビット 1、パリティ ビットなしです。ボー レートのデフォルト値は 9600 です。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_OSDP_ADDRESS", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;OSDP アドレス&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;マルチドロップ RS-485 ネットワークの出力用 OSDP アドレスを指定します。アドレスのデフォルト値は 0 です。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ACS_LEDCONTROLLEDACS", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ACS で LED を制御&lt;/span&gt;&lt;br /&gt;&lt;br /&gt;LED 制御では、".$NXTText." の LED と音響器の状態を ACS で制御できます。  LED 制御を有効にしないと、".$NXTText." が内部的に LED を管理します。");
            define ("TOOLTIP_ACS_DUALAUTHENTICATION", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;2 要素認証&lt;/span&gt;&lt;br /&gt;&lt;br /&gt;2 要素認証では、虹彩を認証する前に、付属のカード リーダーにユーザーがカードを提示する必要があります。&lt;br /&gt;".$NXTText." では、内部メモリーのカードが参照された後、ランプが白に変わって、ユーザーに虹彩認証を促します。&lt;br /&gt;参照された記録とユーザーの虹彩が一致すれば、".$NXTText." から ACS にデータが送信されます。&lt;br /&gt;一致しない場合は、照合失敗コードが ".$NXTText." から送信されます。");

                                
            define ("TOOLTIP_ACS_TEMPLATEONCARDPASS", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;1 要素認証&lt;/span&gt;&lt;br /&gt;&lt;br /&gt;付属のカード リーダーまたは虹彩認証のどちらかに提示された資格情報が ".$NXTText." からアクセス制御パネルに送信されます。");
            define ("TOOLTIP_ACS_TEMPLATEONCARD", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;1 要素認証&lt;/span&gt;&lt;br /&gt;&lt;br /&gt;付属のカード リーダーまたは虹彩認証のどちらかに提示された資格情報が ".$NXTText." からアクセス制御パネルに送信されます。");
            define ("TOOLTIP_ACS_DUALAUTHPARITY", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;2 要素認証パリティ&lt;/span&gt;&lt;br /&gt;&lt;br /&gt;2 要素認証のカード読み取り時に、カードリーダーから送信されるパリティ ビットを確認するかどうかを選択します。");
            define ("TOOLTIP_ACS_IRISWAITTIME", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;虹彩待機時間&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;カードをスキャンした後に、ユーザーが ".$NXTText." で虹彩認証をするまでの許容時間を秒数で指定します&lt;/li&gt;&lt;li&gt;期間範囲 (秒): 2 - 60。&lt;/li&gt;&lt;li&gt;デフォルト値: 10。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ACS_PINWAITTIME", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;PIN 待機時間&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;カードをスキャンした後に、ユーザーが PIN を入力するまでの許容時間を秒数で指定します&lt;/li&gt;&lt;li&gt;期間範囲 (秒): 2 - 60。&lt;/li&gt;&lt;li&gt;デフォルト値: 10。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ACS_PINBURSTBITS", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;PIN バースト ビット&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;PIN 処理のビット サイズを指定します&lt;/li&gt;&lt;li&gt;デフォルト値: 4。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ACS_ENABLERELAYS", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;リレーを有効にする&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このオプションを選択すると、ACS パネルの物理的なリレーの動作が有効になります。&lt;/li&gt;&lt;li&gt;&lt;i&gt;許可&lt;/i&gt;と&lt;i&gt;拒否&lt;/i&gt;のリレーがサポートされています。&lt;/li&gt;&lt;li&gt;下部のスライダーを使って、リレーをアクティブにしておく時間を指定できます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ACS_GRANTRELAYTIME", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;許可リレー時間&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;認証に成功した場合に&lt;i&gt;許可&lt;/i&gt;リレーをトリガーする秒数を指定します。&lt;/li&gt;&lt;li&gt;&lt;i&gt;許可&lt;/i&gt;リレーを個別に無効にするには、この値をゼロに設定してください。&lt;/li&gt;&lt;li&gt;期間範囲 (秒): 0 - 10。&lt;/li&gt;&lt;li&gt;デフォルト値: 3。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ACS_DENYRELAYTIME", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;拒否リレー時間&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;認証に失敗した場合に&lt;i&gt;拒否&lt;/i&gt;リレーをトリガーする秒数を指定します。&lt;/li&gt;&lt;li&gt;&lt;i&gt;拒否&lt;/i&gt;リレーを個別に無効にするには、この値をゼロに設定してください。&lt;/li&gt;&lt;li&gt;期間範囲 (秒): 0 - 10。&lt;/li&gt;&lt;li&gt;デフォルト値: 5。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ACS_DURESSRELAYTIME", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;脅迫リレー時間&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;認証に成功した場合に&lt;i&gt;脅迫&lt;/i&gt;リレーをトリガーする秒数を指定します。&lt;/li&gt;&lt;li&gt;&lt;i&gt;脅迫&lt;/i&gt;リレーを個別に無効にするには、この値をゼロに設定してください。&lt;/li&gt;&lt;li&gt;期間範囲 (秒): 0 - 10。&lt;/li&gt;&lt;li&gt;デフォルト値: 5。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ACS_TESTCODE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;施設コード&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;選択した ACS プロトコルで、&lt;i&gt;[今すぐテスト]&lt;/i&gt; ボタンをクリックしたときに使われる事前定義済の施設コードが表示されます。&lt;/li&gt;&lt;li&gt;ACS システムをテストする前に、EyeEnroll アプリケーションを使って、この値を事前に定義しておいてください。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ACS_TESTCARDID", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;カード ID&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;選択した ACS プロトコルで、&lt;i&gt;[今すぐテスト]&lt;/i&gt; ボタンをクリックしたときに使われる事前定義済のカード ID が表示されます。&lt;/li&gt;&lt;li&gt;ACS システムをテストする前に、EyeEnroll アプリケーションを使って、この値を事前に定義しておいてください。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_ACS_TESTACSBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[今すぐテスト] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このボタンをクリックすると、事前定義済のテスト文字列が ACS パネルに送信されます。EyEnroll アプリケーションを使って、".$NXTText." のテスト文字列を定義しておく必要があります。&lt;/li&gt;&lt;/ul&gt;");


            //////////////////////////////////////////////////////////////////////////////
            // Other Tool Tips...
            //////////////////////////////////////////////////////////////////////////////
            define ("TOOLTIP_HEADER_LANGUAGE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[言語] ドロップダウン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このドロップダウンを使って、WebConfig ユーザー インタフェースの言語を選びます。&lt;/li&gt;&lt;li&gt;使用可能な言語のみが表示されます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_HEADER_EYELOCKLOGO", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;Eyelock Corporation&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;&lt;a href=&apos;http://www.eyelock.com&apos;  target=&apos;_blank&apos;&gt;ここ&lt;/a&gt; をクリックすると、Eyelock の Web サイトが表示されます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_HEADER_CLIENTVER", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;WebConfig クライアント バージョン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;現在使用中の WebConfig クライアントのバージョンが表示されます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_HEADER_LICENSE", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ライセンス リンク&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、&lt;i&gt;エンド ユーザー ライセンス契約&lt;/i&gt;が表示されます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_HEADER_HELP", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ヘルプ リンク&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このリンクをクリックすると、ダイアログ ボックスが表示されて、&lt;i&gt;ポップアップ ヘルプ システム&lt;/i&gt;の設定を変更できます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_HEADER_LOGOUT", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;ログアウト リンク&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;このリンクをクリックすると、現在のセッションからログアウトして、ログイン ページに戻ります。&lt;/li&gt;&lt;/ul&gt;");

            define ("TOOLTIP_FOOTER_APPVERSION", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;".$NXTText." ファームウェア バージョン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;".$NXTText." で実行中のアプリケーション ファームウェアの現在のバージョンが表示されます。&lt;/li&gt;&lt;/ul&gt;");
            define ("TOOLTIP_FOOTER_BOBVERSION", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;".$NXTText." ICM バージョン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;".$NXTText." で実行中の ICM の現在のバージョンが表示されます。&lt;/li&gt;&lt;/ul&gt;");

            define ("TOOLTIP_FOOTER_SAVEBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[保存] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、".$NXTText." の設定に加えた変更が永続的に保存されます。&lt;/li&gt;&lt;li&gt;設定をコミットすると、".$NXTText." が自動的に再起動して、新しい設定が即座に有効になります。&lt;/li&gt;&lt;/ul&gt;");
			
			 define ("TOOLTIP_FOOTER_CANCELBUTTON", "&lt;span style=&apos;font-size: 16px; font-style:bold&apos;&gt;[キャンセル] ボタン&lt;/span&gt;&lt;ul style=&apos;padding-left: 1em&apos;&gt;&lt;li&gt;クリックすると、加えた変更が破棄されて、構成ページがリフレッシュされます。&lt;/li&gt;
                                                &lt;/ul&gt;");

            define ("TOOLTIP_TAB_DEVICEIP", "&lt;span style=&apos;font-size: 14px; font-style:bold&apos;&gt;現在の ".$NXTText." の名前/IP アドレス");
            define ("TOOLTIP_TAB_DEVICEMACADDR", "&lt;span style=&apos;font-size: 14px; font-style:bold&apos;&gt;現在の ".$NXTText." の MAC アドレス");

            //////////////////////////////////////////////////////////////////////////////
            // Validation Error Messages
            //////////////////////////////////////////////////////////////////////////////
            define ("VALIDATOR_MSG_HOSTNAME", "ホスト名に空白や特殊文字を含めることはできず、</br>64 文字未満にする必要があります。");
            define("TITLE_BTN_TESTNWMS", "現在適用されている Network Matcher の設定をテストします。");
            define("TEST_NWMS", "NWMS のテスト");
            define("CB_NWMATCHERCOMMSECURE_TEXT", "安全ではありません");
            define("COPYRIGHTTEXT", "Copyright \AD&#169; 2014-2020.  All Rights Reserved.");
            define("DISCONNECTWARNING", "この処理の実行中は、ブラウザーを閉じたり、".$NXTText." の電源を切断したり、ネットワークを切断したりしないでください。");
            break;



/*
        case "de":
            define("WELCOME_TXT","Willkommen!");
            define("CHOOSE_TXT","Sprache auswÃ¤hlen");
            break;


        default:
            define("WELCOME_TXT","Welcome!");
            define("CHOOSE_TXT","Choose Language");
            break;
*/
    }
}
?>
