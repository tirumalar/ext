<?php
/** This class does all the work... **/
//When inserting a new paramter, do a document or site search, and look for "INSERT NEW PARAMETER".  This will highlight all spots where your parameter should be added.

require("checklogin.php"); // Make sure user is logged on to get to this page...
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");
 function loadLastUpdateTime()
    {
       
       try{
	if(!file_exists("/home/nanoupdate.txt"))
		return "";
        $fh = fopen("/home/nanoupdate.txt", "r");
        if($fh == false)
            return "";
        $line = fgets($fh);
        fclose($fh);
        return $line;
       }
       catch (Exception $e)
       {
           return "";
       }
    }
     function loadLastBobUpdateTime()
    {
         try{
	if(!file_exists("/home/bobupdate.txt"))
		return "";

        $fh = fopen("/home/bobupdate.txt", "r");
        if($fh == false)
            return "";
        $line = fgets($fh);
        fclose($fh);
        return $line;
         }
       catch (Exception $e)
       {
           return "";
       }
    }

function setSlaveConfigValue($name, $value)
{
	//slave debug settings are changed here
	//error_log("Try set slave config value " . $name . "," . $value);
	//DMOHBOXOUT $strcmdResult = NXTW_shell_exec(sprintf("251".chr(0x1F)."%s".chr(0x1F)."%s",$name, $value)); 
}

function writeOutLogLevel($level)
	
	{
	// /home/root/nxtLog.cfg
	// looking for  line:
	//log4j.rootLogger=INFO, Rlog,stdout
		
		$logconfig = fopen("/home/root/nxtlog.cfg","r");
		if($logconfig == FALSE)
			return;
		$logout = fopen("/home/root/nxtlog.cfg.out", "w");
		if($logout == FALSE)
		{
			fclose($logconfig);
			return;
		}
			$logline = fgets($logconfig);
		while(!feof($logconfig))
		{

			//echo $logline;
			if(strstr($logline, "log4j.rootLogger") != FALSE)
			{
				$logline = "log4j.rootLogger=".$level.", Rlog,stdout\n";


			}

				fwrite($logout, $logline);
			$logline = fgets($logconfig);

		}
		fclose($logconfig);
			fclose($logout);
	
		//commit changes to the log config file
		shell_exec("mv /home/root/nxtlog.cfg.out /home/root/nxtlog.cfg");
	//return $level;
}
class INIEditor
{
    var $m_sIni_file;               // name of the ini-file, 

    // Arrays to hold section|key|values and comments...
    var $m_aParameters = array();   // multiple array.  var[sections][parameter][value or text]
    var $m_aComments = array();     // array with comments, index is section or _start for the header-comments
    var $m_aSaveParameters = array();

    ////////////////////////////////////////////////////
    // eyeLock.ini values of interest
    ////////////////////////////////////////////////////
    //public $GRI_HBDestAddress = "";

    var $MatcherIndex = -1;
    var $MatcherCount = 0;
    var $MatcherAddress = ""; // Field name associated with PCMATCHER
    var $MatcherIDLocal = "1";

    public $GRI_EnableNWHDMatcher = "false"; // Internal variable...
    public $GRI_HDMatcherID = -1;   // The currently stored matcher index.  If it matches PCMATCHER entry, then the internal variable GRI_EnableNWHDMatcher gets set to TRUE...
    public $GRI_HDMatcher_Address = ""; // Just the address...
	public $HardwareType = "0";  //Set through hardware system 0 = NXT, 1 = EXT, 2 = HBOX
    public $GRI_SlaveAddressListAddr = "";
    public $GRI_SlaveAddressListPort = "";
    public $GRI_MatchResultDestAddr = "";
    public $GRI_MatchResultNwMsgFormat = "";
    public $GRI_NwDispatcherSecure = "secure";
    public $Eyelock_TamperOutSignalHighToLow = "";
    public $server_port;
    public $GRI_HDMatcher_port;
    public $GRITrigger_F2FEnable;
    public $GRITrigger_WeigandEnable;
    public $GRITrigger_WeigandHidEnable;
    public $GRITrigger_PACEnable;
    public $GRITrigger_RelayEnable;

    public $OSDPBAUDRate;
    public $OSDPAddress;

    public $GRITrigger_OSDPEnable;
    public $GRITrigger_OSDPInputEnable;
    public $GRITrigger_DualAuthenticationMode;
    public $GRITrigger_TemplateOnCard;

    public $GRITrigger_PortableTemplatesUseCustomKey;
    public $GRITrigger_PortableTemplateCustomKeyPath;

    public $GRITrigger_TOCPassThrough;
    public $GRITrigger_TOCMobileMode;
    public $GRITrigger_DualAuthenticationParity;
    public $GRITrigger_DualAuthNLedControlledByACS;
    public $GRITrigger_DualAuthNCardMatchWaitIrisTime;
    public $GRITRIGGER_TOCCardExpiredTime;

    public $Eyelock_ACPTestCardID;
    public $Eyelock_ACPTestFacilityCode;
    public $Eyelock_DualMatcherPolicy = "";
    public $Eyelock_BoBVersion;
    public $GRI_BobSupportDualAuthN = false;
    public $GRI_BobSupportWiegandHID = false;
    public $GRI_BobSupportPAC = false;
//    public $Eyelock_EnableIEEE8021X;
    public $Eyelock_EnableNegativeMatchTimeout;
    public $Eyelock_NegativeMatchTimeout = 12;
    public $Eyelock_NegativeMatchResetTimer = 4;

    public $Eyelock_TamperSignalHighToLow = "";
    public $Eyelock_TamperNotifyAddr;
    public $Eyelock_TamperNotifyMessage;

    public $Eyelock_AuthenticationMode = 1;
    public $Eyelock_PinBurstBits = 4;
    public $Eyelock_WaitPinTime = 10;

    public $GRI_SecureComm = "secure";
    public $GRITrigger_EnableRelayWithSignal = "false";
    public $GRITrigger_RelayTimeInMS;
    public $GRITrigger_DenyRelayTimeInMS;
    public $GRI_AuthorizationToneFrequency = 0;  // Not currently used, must leave in here though
    public $GRI_AuthorizationToneVolume = 0;
    public $GRI_AuthorizationToneDurationSeconds = 0;
    public $GRI_RepeatAuthorizationPeriod = 4;
    public $GRI_TamperToneVolume = 0;
    public $GRI_LEDBrightness = 0;
    public $GRI_InternetTimeAddr = "";
    public $GRI_InternetTimeSync = "";
    public $Eyelock_DeviceLocation = ""; // takes some sort of geographic location – a string with 1 to 150 characters
    public $Eyelock_WelcomeMessage = ""; // a string with a pattern (for ex, like the network match message)  , ex  “Welcome %s”,name) 
    public $Eyelock_HttpPostSenderDestAddress = ""; // a URL 
    public $Eyelock_HttpPostSenderDestPostIris = "";//   - End point for posting IRIS images
    public $Eyelock_HttpPostSenderDestSignalError = "";//– End point for posting errors
    public $Eyelock_HttpPostSenderDestSignalHeartBeat = "";//– End point for posting heartbeats
    public $Eyelock_HttpPostSenderDestSignalMaintenance = "";//– End point for posting maintenance messages.
    public $Eyelock_HttpPostSenderDestScheme = ""; // "http" or "https"
    public $Eyelock_SPAWAREnable = "false";

    public $GRI_EyeDestAddr;
    public $GRI_EyeDestPort;

    public $Eyelock_MaxTemplateCount = "1000";
    public $Eyelock_SoftwareUpdateURL = "https://eyelock.com/updates/nanoversioninfo.xml";
    public $Eyelock_SoftwareUpdateDateCheck = "Never";
    public $Eyelock_SoftwareUpdateDateNano = "Never";
    public $Eyelock_SoftwareUpdateDateBob = "Never";

    public $Eyelock_EnablePopupHelp = "true";
    public $Eyelock_PopupHelpTriggerHover = 1;
    public $Eyelock_PopupHelpDelay = 1;
    public $Eyelock_TLSEnable = "false";
    public $Eyelock_AllowSiteAdminUpgrade = "false";
    public $Eyelock_NwMatcherCommSecure = "secure";
	public $GRI_EyeConnecTimeoutmsec = "4000";
	public $GRI_EyeSendTimeoutmsec = "5000";
	
	
	public $Eyelock_SystemReadyDebug = false;
	public $Eyelock_Debug = false;
	public $Eyelock_MatchDispDebug= false;
	public $Eyelock_OpenSSL_Debug= false;
	public $Eyelock_xSSLDebug = false;
	
	public $GRI_DBDebug= false;
	public $GRI_EyeDispatcherDebug = false;
	public $GRI_HBDebug = false;
	public $GRI_HDDebug= false;
	public $GRI_LEDDebug = false;
	public $GRI_MPDebug= false;

	public $GRI_NwDebug = false;
	public $GRI_NwMmDebug = false;
	public $GRI_SpoofDebug= false;
	public $GRITrigger_CmxDebug = false;
	public $GRITrigger_F2FDebug = false;
	public $GRITrigger_WeigandDebug= false;
	public $MT9P001_Debug = false;
	public $NwListener_Debug= false;
	public $GRI_MatchMgrDebug = false;
	//INSERT NEW PARAMETER LOCAL VARIABLE HERE
	
    
	// Constructor...
	function __construct ($inifile)
	{
        $this->m_sIni_file = $inifile;
		//CMX todo:  Put in a method to detect whether this is a CMX device.
		//$HardwareType = "0";
		
        // This must be here... this loads the original file, before the 'posted' values are saved.
        // The original values are then checked against the posted values to check for updating/saving
        // Then they are saved.  Once saved, right before the html is echoed, the eyelock.ini file
        // is loaded again with the new saved info....
  //      $this->LoadIniSettings();  
	}

     function LocalGetRemoteFolderFileList($strRemoteFolder, $bNanoRestore)
    {
        $cmdls = "";

        if ($bNanoRestore)
            $cmdls = sprintf("cd %s; ls | grep -e %s", escapeshellarg($strRemoteFolder), "\"root_[0-9]\{4\}[0-9]\{2\}[0-9]\{2\}_[0-9]\{2\}[0-9]\{2\}[0-9]\{2\}.tgz\"");
        else
            $cmdls = sprintf("cd %s; ls | grep -e %s", escapeshellarg($strRemoteFolder), "\"bob_[0-9]\{4\}[0-9]\{2\}[0-9]\{2\}_[0-9]\{2\}[0-9]\{2\}[0-9]\{2\}.tgz\"");

        $ls = shell_exec($cmdls);

	    if ($bNanoRestore)
            $cmdls = sprintf("cd %s; ls | grep -e %s", escapeshellarg($strRemoteFolder), "\"root_[0-9]\{4\}[0-9]\{2\}[0-9]\{2\}_[0-9]\{2\}[0-9]\{2\}[0-9]\{2\}_.*.tgz\"");
        else
            $cmdls = sprintf("cd %s; ls | grep -e %s", escapeshellarg($strRemoteFolder), "\"bob_[0-9]\{4\}[0-9]\{2\}[0-9]\{2\}_[0-9]\{2\}[0-9]\{2\}[0-9]\{2\}_.*.tgz\"");
        
        $ls = $ls.shell_exec($cmdls);
    // ok
        if (NULL === $ls)
            $ls = "\n";

        $cmdls = str_replace("\n", "*", $ls);
    //    $arFiles = explode("|", $cmdls);

        // Return the string of restore points...
        return $cmdls;
    }

   
    function LoadIniSettings()
    {
        $section="Dummy"; // dummy section for header
		$key="_none";
		$text="";

		$fh = fopen($this->m_sIni_file, "r");

        if ($fh != false)
        {
		    while (!feof ($fh))
		    {
			    $line = fgets($fh);
      
			    // trick which space at the beginning to fool strpos
			    if ((strpos(" ". $line,";") == 1) || (strpos(" ". $line,"#") == 1))
			    {	
				    $this->m_aComments[$section][] = $line;	// commentline
			    }
			    elseif (strpos(" ". $line,"[") == 1)
			    {
				    $section=trim(substr($line,1,strpos($line,"]")-1));	// section
			    }
			    elseif (strpos($line,"=") > 0 )
			    {
				    list($key, $value) = explode ("=", $line);
				    $text = trim($text);
				    $key = trim($key);
				    $value=trim($value);
        
				    // Located and entry so add the key, value, comment to the parameters array...
				    $this->add($key, $value, $section, $text); 
			    }
		    }

            // We have loaded all of the valid key|value pairs, now process
            // those values and populate our members appropriately
            // The processed members are what we actually display in our UI
            $this->ProcessIniValues();
        }
    }


    function SaveIniSettings($post)
    {
        $this->UpdateIniValues($post);
   		$this->SaveIniValues();
        $this->ProcessIniValues(); // We must call this after a 'Save' to refresh our class members with the newly saved values...
    }


    function UpgradeIniSettings()
    {
        $this->set_all();
   		$this->SaveIniValues();
    }


    /////////////////////////////////////////////////////////////////
    // This function looks through the key|value pairs and sets up
    // our member variables as appropriate... including defaults if necessary
    function ProcessIniValues()
    {
        // Hardware flag... we never write this, it is set differently for each device in the firmware
        if (!$this->get("Eyelock.HardwareType", $this->HardwareType))
        {
            $this->HardwareType = "0";
            $this->add("Eyelock.HardwareType", $this->HardwareType);
        }

        if ($this->get("GRI.HDMatcherCount", $this->MatcherCount))
        {
            // look for the network matcher value... if it doesn't exist, add it...
            for ($i = 0; $i < $this->MatcherCount; $i++) 
            {
                $MatcherType = sprintf("GRI.HDMatcher.%d.Type", $i);

                if ($this->get($MatcherType, $MatcherTypeVal))
                {
                    // Set it back to Local if turned off...
                    if ($MatcherTypeVal === "LOCAL")
                    {
                        $this->MatcherIDLocal = $i;
                        continue;
                    }

                    if ($MatcherTypeVal === "PCMATCHER")
                    {
                        // store the index so we can write the correct one back out...
                        $this->MatcherIndex = $i;
                        $this->MatcherAddress = sprintf("GRI.HDMatcher.%d.Address", $this->MatcherIndex);

                        // We found a PCMATCHER entry... make sure address and buffsize are there...
                        if (!$this->get($this->MatcherAddress, $this->GRI_HDMatcher_Address))
                        {
                            $this->GRI_HDMatcher_Address = "";
                            $this->add($this->MatcherAddress, $this->GRI_HDMatcher_Address);
                        }

                        // Make sure BuffSize exists...
                        $BufSize = sprintf("GRI.HDMatcher.%d.BuffSize", $this->MatcherIndex);

                        if (!$this->get($BufSize, $this->GRI_HDMatcher_BuffSize))
                        {
                            $this->GRI_HDMatcher_BuffSize = "0";
                            $this->add($BufSize, $this->GRI_HDMatcher_BuffSize);
                        }

                        // Ok, we have an entry... is it the select pcmatcher mode?
                        if (($this->get("GRI.HDMatcherID", $MatcherID)) && (intval($MatcherID) === $i))
                            $this->GRI_EnableNWHDMatcher = "true";
                    }
                }
            }

            // No matcher entries were found... in this case we add them in with the correct numbers... as the next one in sequence...
            if ($this->MatcherIndex === -1)
            {
                // Add it on the end...
                $this->MatcherIndex = $this->MatcherCount;

                $MatcherType = sprintf("GRI.HDMatcher.%d.Type", $this->MatcherIndex);
                // We found a PCMATCHER entry... make sure address and buffsize are there...
                $this->GRI_HDMatcher_Type = "PCMATCHER";
                $this->add($MatcherType, $this->GRI_HDMatcher_Type);

                $this->MatcherAddress = sprintf("GRI.HDMatcher.%d.Address", $this->MatcherIndex);
                // We found a PCMATCHER entry... make sure address and buffsize are there...
                $this->GRI_HDMatcher_Address = "";
                $this->add($this->MatcherAddress, $this->GRI_HDMatcher_Address);
                $this->GRI_EnableNWHDMatcher = "false";

                // Make sure BuffSize exists...
                $BufSize = sprintf("GRI.HDMatcher.%d.BuffSize", $this->MatcherIndex);
                $this->GRI_HDMatcher_BuffSize = "134217728";
                $this->add($BufSize, $this->GRI_HDMatcher_BuffSize);

                // Also set the count...
                $this->MatcherCount++;
                $this->GRI_HDMatcherCount = sprintf("%d", $this->MatcherCount);
                $this->add("GRI.HDMatcherCount", $this->GRI_HDMatcherCount);

                $this->GRI_EnableNWHDMatcher = "false";
            }
        }


        if (!$this->get("server.port", $this->server_port))
        {
            $this->server_port = "0";
            $this->add("server.port", $this->server_port);
        }

        // This could be commented out... if so, we leave it empty...
        if (!$this->get("GRI.MatchResultDestAddr", $this->GRI_MatchResultDestAddr))
        {
            $this->GRI_MatchResultDestAddr = "";
            // and we add it to our array, it will get added back into the file ONLY if
            // the user specifies a value...
            $this->add("GRI.MatchResultDestAddr", $this->GRI_MatchResultDestAddr);
        }

        if (!$this->get("GRI.MatchResultNwMsgFormat", $this->GRI_MatchResultNwMsgFormat))
        {
            $this->GRI_MatchResultNwMsgFormat = "Matched:%d;Score:%0.4f;Time:%llu;ID:";
            $this->add("GRI.MatchResultNwMsgFormat", $this->GRI_MatchResultNwMsgFormat);
        }

       
        if (!$this->get("GRI.NwDispatcherSecure", $this->GRI_NwDispatcherSecure))
        {
            $this->GRI_NwDispatcherSecure = "secure";
            $this->add("GRI.NwDispatcherSecure", $this->GRI_NwDispatcherSecure);
        }
        if ( !$this->get( "GRITrigger.PortableTemplatesUseCustomKey", $this->GRITrigger_PortableTemplatesUseCustomKey ) ) {
        	$this->GRITrigger_PortableTemplatesUseCustomKey = "false";
        	$this->add( "GRITrigger.PortableTemplatesUseCustomKey", $this->GRITrigger_PortableTemplatesUseCustomKey );
        }
		
		if ( !$this->get( "Eyelock.SystemReadyDebug", $this->Eyelock_SystemReadyDebug ) ) {
			$this->Eyelock_SystemReadyDebug = "false";
			$this->add( "Eyelock.SystemReadyDebug", $this->Eyelock_SystemReadyDebug );
		}
		if ( !$this->get( "Eyelock.Debug", $this->Eyelock_Debug ) ) {
			$this->Eyelock_Debug = "false";
			$this->add( "Eyelock.Debug", $this->Eyelock_Debug );
		}
		if ( !$this->get( "Eyelock.MatchDispDebug", $this->Eyelock_MatchDispDebug ) ) {
			$this->Eyelock_MatchDispDebug = "false";
			$this->add( "Eyelock.MatchDispDebug", $this->Eyelock_MatchDispDebug );
		}
		if ( !$this->get( "Eyelock.OpenSSL.Debug", $this->Eyelock_OpenSSL_Debug ) ) {
			$this->Eyelock_OpenSSL_Debug = "false";
			$this->add( "Eyelock.OpenSSL.Debug", $this->Eyelock_OpenSSL_Debug );
		}
		if ( !$this->get( "Eyelock.SSLDebug", $this->Eyelock_xSSLDebug   ) ) {
			$this->Eyelock_xSSLDebug = "false";
			$this->add( "Eyelock.SSLDebug", $this->Eyelock_xSSLDebug   );
		}
		if ( !$this->get( "GRI.DBDebug", $this->GRI_DBDebug ) ) {
			$this->GRI_DBDebug = "false";
			$this->add( "GRI.DBDebug", $this->GRI_DBDebug );
		}
		if ( !$this->get( "GRI.EyeDispatcherDebug", $this->GRI_EyeDispatcherDebug ) ) {
		//	error_log("EyeDispatcherDebug not found");
			$this->GRI_EyeDispatcherDebug = "false";
			$this->add( "GRI.EyeDispatcherDebug", $this->GRI_EyeDispatcherDebug );
		}
		if ( !$this->get( "GRI.HBDebug", $this->GRI_HBDebug ) ) {
			$this->GRI_HBDebug = "false";
			$this->add( "GRI.HBDebug", $this->GRI_HBDebug );
		}
		if ( !$this->get( "GRI.HDDebug", $this->GRI_HDDebug ) ) {
			$this->GRI_HDDebug = "false";
			$this->add( "GRI.HDDebug", $this->GRI_HDDebug );
		}
		if ( !$this->get( "GRI.LEDDebug", $this->GRI_LEDDebug ) ) {
	//		error_log("leddebug not found ");
			$this->GRI_LEDDebug = "false";
			$this->add( "GRI.LEDDebug", $this->GRI_LEDDebug );
		}
		if ( !$this->get( "GRI.MPDebug", $this->GRI_MPDebug ) ) {
			$this->GRI_MPDebug = "false";
			$this->add( "GRI.MPDebug", $this->GRI_MPDebug );
		}
		
		if ( !$this->get( "GRI.NwDebug", $this->GRI_NwDebug ) ) {
			$this->GRI_NwDebug = "false";
			$this->add( "GRI.NwDebug", $this->GRI_NwDebug );
		}
		if ( !$this->get( "GRI.NwMmDebug", $this->GRI_NwMmDebug ) ) {
			$this->GRI_NwMmDebug = "false";
			$this->add( "GRI.NwMmDebug", $this->GRI_NwMmDebug );
		}
		if ( !$this->get( "GRI.SpoofDebug", $this->GRI_SpoofDebug ) ) {
			$this->GRI_SpoofDebug = "false";
			$this->add( "GRI.SpoofDebug", $this->GRI_SpoofDebug );
		}
		if ( !$this->get( "GRITrigger.CmxDebug", $this->GRITrigger_CmxDebug ) ) {
			$this->GRITrigger_CmxDebug = "false";
			$this->add( "GRITrigger.CmxDebug", $this->GRITrigger_CmxDebug );
		}
		if ( !$this->get( "GRITrigger.F2FDebug", $this->GRITrigger_F2FDebug ) ) {
			$this->GRITrigger_F2FDebug = "false";
			$this->add( "GRITrigger.F2FDebug", $this->GRITrigger_F2FDebug );
		}
		if ( !$this->get( "GRITrigger.WeigandDebug", $this->GRITrigger_WeigandDebug ) ) {
			$this->GRITrigger_WeigandDebug = "false";
			$this->add( "GRITrigger.WeigandDebug", $this->GRITrigger_WeigandDebug );
		}
		if ( !$this->get( "MT9P001.Debug", $this->MT9P001_Debug ) ) {
			$this->MT9P001_Debug = "false";
			$this->add( "MT9P001.Debug", $this->MT9P001_Debug );
		}
		if ( !$this->get( "NwListener.Debug", $this->NwListener_Debug ) ) {
			$this->NwListener_Debug = "false";
			$this->add( "NwListener.Debug", $this->NwListener_Debug );
		}
		
		if ( !$this->get( "GRI.MatchMgrDebug", $this->GRI_MatchMgrDebug ) ) {
			$this->GRI_MatchMgrDebug = "false";
			$this->add( "GRI.MatchMgrDebug", $this->GRI_MatchMgrDebug );
		}
        //INSERT NEW PARAMETER GET HERE
		 if (!$this->get("GRI.EyeConnecTimeoutmsec", $this->GRI_EyeConnecTimeoutmsec))
        {
            $this->GRI_EyeConnecTimeoutmsec = "4000";
            $this->add("GRI.EyeConnecTimeoutmsec", $this->GRI_EyeConnecTimeoutmsec);
        }
		
		 if (!$this->get("GRI.EyeSendTimeoutmsec", $this->GRI_EyeSendTimeoutmsec))
        {
            $this->GRI_EyeSendTimeoutmsec = "5000";
            $this->add("GRI.EyeSendTimeoutmsec", $this->GRI_EyeSendTimeoutmsec);
        }
		
    
        if (!$this->get("GRITrigger.WeigandEnable", $this->GRITrigger_WeigandEnable))
        {
            $this->GRITrigger_WeigandEnable = "true";
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("GRITrigger.WeigandEnable", $this->GRITrigger_WeigandEnable);
        }

        if (!$this->get("GRITrigger.WeigandHidEnable", $this->GRITrigger_WeigandHidEnable))
        {
            $this->GRITrigger_WeigandHidEnable = "false";
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("GRITrigger.WeigandHidEnable", $this->GRITrigger_WeigandHidEnable);
        }

        if (!$this->get("GRITrigger.F2FEnable", $this->GRITrigger_F2FEnable))
        {
            $this->GRITrigger_F2FEnable = "false";
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("GRITrigger.F2FEnable", $this->GRITrigger_F2FEnable);
        }

        if (!$this->get("GRITrigger.PACEnable", $this->GRITrigger_PACEnable))
        {
            $this->GRITrigger_PACEnable = "false";
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("GRITrigger.PACEnable", $this->GRITrigger_PACEnable);
        }

        if (!$this->get("GRITrigger.OSDPEnable", $this->GRITrigger_OSDPEnable))
        {
            $this->GRITrigger_OSDPEnable = "false";
            // Missing (but required) elements must be 'added'
            $this->add("GRITrigger.OSDPEnable", $this->GRITrigger_OSDPEnable);
        }
        if (!$this->get("GRITrigger.OSDPInputEnable", $this->GRITrigger_OSDPInputEnable))
        {
            $this->GRITrigger_OSDPInputEnable = "false";
            // Missing (but required) elements must be 'added'
            $this->add("GRITrigger.OSDPInputEnable", $this->GRITrigger_OSDPInputEnable);
        }
       
        if (!$this->get("GRITrigger.DualAuthenticationMode", $this->GRITrigger_DualAuthenticationMode))
        {
            $this->GRITrigger_DualAuthenticationMode = "false";
            // Missing (but required) elements must be 'added'
            $this->add("GRITrigger.DualAuthenticationMode", $this->GRITrigger_DualAuthenticationMode);
        }
        
        if (!$this->get("GRITrigger.MobileMode", $this->GRITrigger_TOCMobileMode))
        {
            $this->GRITrigger_TemplateOnCard = "false";
            // Missing (but required) elements must be 'added'
            $this->add("GRITrigger.MobileMode", $this->GRITrigger_TOCMobileMode);
        }
        if (!$this->get("GRITrigger.TransTOCMode", $this->GRITrigger_TemplateOnCard))
        {
            $this->GRITrigger_TemplateOnCard = "false";
            // Missing (but required) elements must be 'added'
            $this->add("GRITrigger.TransTOCMode", $this->GRITrigger_TemplateOnCard);
        }
         if (!$this->get("GRITrigger.PassThroughMode", $this->GRITrigger_TOCPassThrough))
        {
            $this->GRITrigger_TOCPassThrough = "false";
            // Missing (but required) elements must be 'added'
            $this->add("GRITrigger.PassThroughMode", $this->GRITrigger_TOCPassThrough);
        }
        
        if (!$this->get("GRITrigger.DualAuthenticationParity", $this->GRITrigger_DualAuthenticationParity))
        {
            $this->GRITrigger_DualAuthenticationParity = "true";
            // Missing (but required) elements must be 'added'
            $this->add("GRITrigger.DualAuthenticationParity", $this->GRITrigger_DualAuthenticationParity);
        }
        if (!$this->get("GRITrigger.DualAuthNLedControlledByACS", $this->GRITrigger_DualAuthNLedControlledByACS))
        {
            $this->GRITrigger_DualAuthNLedControlledByACS = "false";
            // Missing (but required) elements must be 'added'
            $this->add("GRITrigger.DualAuthNLedControlledByACS", $this->GRITrigger_DualAuthNLedControlledByACS);
        }
       
         if (!$this->get("GRITrigger.TOCCardExpiredTime", $this->GRITRIGGER_TOCCardExpiredTime))
        {
            $this->GRITRIGGER_TOCCardExpiredTime = 60;
            // Missing (but required) elements must be 'added'
            $this->add("GRITrigger.TOCCardExpiredTime", $this->GRITRIGGER_TOCCardExpiredTime);
        }
        else
            $this->GRITRIGGER_TOCCardExpiredTime;

        if (!$this->get("GRITrigger.DualAuthNCardMatchWaitIrisTime", $this->GRITrigger_DualAuthNCardMatchWaitIrisTime))
        {
            $this->GRITrigger_DualAuthNCardMatchWaitIrisTime = 10;
            // Missing (but required) elements must be 'added'
            $this->add("GRITrigger.DualAuthNCardMatchWaitIrisTime", $this->GRITrigger_DualAuthNCardMatchWaitIrisTime);
        }
        else
            $this->GRITrigger_DualAuthNCardMatchWaitIrisTime /= 1000;

        if (!$this->get("Eyelock.ACPCardID", $this->Eyelock_ACPCardID))
        {
            $this->Eyelock_ACPCardID = "";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.ACPCardID", $this->Eyelock_ACPCardID);
        }

        if (!$this->get("Eyelock.OSDPBaudRate", $this->OSDPBAUDRate))
        {
            $this->OSDPBAUDRate = "";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.OSDPBaudRate", $this->OSDPBAUDRate);
        }
        if (!$this->get("Eyelock.OSDPAddress", $this->OSDPAddress))
        {
            $this->OSDPAddress = "0";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.OSDPAddress", $this->OSDPAddress);
        }
        if (!$this->get("Eyelock.ACPTestFacilityCode", $this->Eyelock_ACPTestFacilityCode))
        {
            $this->Eyelock_ACPTestFacilityCode = "";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.ACPFacilityCode", $this->Eyelock_ACPTestFacilityCode);
        }

        // If this value doesn't exist, we need to manually add it into our internal array...
        // When saved, we make sure it gets writting out...
        if (!$this->get("Eyelock.DualMatcherPolicy", $this->Eyelock_DualMatcherPolicy))
        {
            $this->Eyelock_DualMatcherPolicy = "false"; // setup a default
            $this->add("Eyelock.DualMatcherPolicy", $this->Eyelock_DualMatcherPolicy);
        }
        if($this->Eyelock_DualMatcherPolicy === "0")
            $this->Eyelock_DualMatcherPolicy = "false";
        if($this->Eyelock_DualMatcherPolicy === "1")
            $this->Eyelock_DualMatcherPolicy = "true";
/**
        if (l_ParamDic.ContainsKey("BobSupportDualAuthN"))
        {
            l_GRI_BobSupportDualAuthN = (l_ParamDic["BobSupportDualAuthN"] == "true") ? true : false;
        }

        if (l_ParamDic.ContainsKey("BobSupportWiegandHID"))
        {
            l_GRI_BobSupportWiegandHID = (l_ParamDic["BobSupportWiegandHID"] == "true") ? true : false;
        }

        if (l_ParamDic.ContainsKey("BobSupportPAC"))
        {
            l_GRI_BobSupportPAC = (l_ParamDic["BobSupportPAC"] == "true") ? true : false;
        }
**/
/*
        if (!$this->get("Eyelock.EnableIEEE8021X", $this->Eyelock_EnableIEEE8021X))
        {
            $this->Eyelock_EnableIEEE8021X = "false";
            $this->add("Eyelock.EnableIEEE8021X", $this->Eyelock_EnableIEEE8021X);
        }
*/
        if (!$this->get("Eyelock.EnableNegativeMatchTimeout", $this->Eyelock_EnableNegativeMatchTimeout))
        {
            $this->Eyelock_EnableNegativeMatchTimeout = "0";
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("Eyelock.EnableNegativeMatchTimeout", $this->Eyelock_EnableNegativeMatchTimeout);
        }

        if (!$this->get("Eyelock.NegativeMatchTimeout", $this->Eyelock_NegativeMatchTimeout))
        {
            $this->Eyelock_NegativeMatchTimeout = 12;
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("Eyelock.NegativeMatchTimeout", $this->Eyelock_NegativeMatchTimeout);
        }
        else
            $this->Eyelock_NegativeMatchTimeout /= 1000;

        if (!$this->get("Eyelock.NegativeMatchResetTimer", $this->Eyelock_NegativeMatchResetTimer))
        {
            $this->Eyelock_NegativeMatchResetTimer = 4;
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("Eyelock.NegativeMatchResetTimer", $this->Eyelock_NegativeMatchResetTimer);
        }

        // If this value doesn't exist, we need to manually add it into our internal array...
        // When saved, we make sure it gets writting out...
        if (!$this->get("Eyelock.TamperSignalHighToLow", $this->Eyelock_TamperSignalHighToLow))
        {
            $this->Eyelock_TamperSignalHighToLow = "true"; // setup a default
            $this->add("Eyelock.TamperSignalHighToLow", $this->Eyelock_TamperSignalHighToLow);
        }
       
         if (!$this->get("Eyelock.TamperOutSignalHighToLow", $this->Eyelock_TamperOutSignalHighToLow ))
        {
            //error_log("load default for Eyelock.TamperOutSignalHighToLow");
            
            $this->Eyelock_TamperOutSignalHighToLow = "false"; // setup a default
            $this->add("Eyelock.TamperOutSignalHighToLow", $this->Eyelock_TamperOutSignalHighToLow);
        }
       $logme = sprintf("Eyelock.TamperOutSignalHighToLow is %s", $this->Eyelock_TamperOutSignalHighToLow);
       //error_log($logme);

        if (!$this->get("Eyelock.TamperNotifyAddr", $this->Eyelock_TamperNotifyAddr))
        {
            $this->Eyelock_TamperNotifyAddr = "";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.TamperNotifyAddr", $this->Eyelock_TamperNotifyAddr);
        }

        if (!$this->get("Eyelock.TamperNotifyMessage", $this->Eyelock_TamperNotifyMessage))
        {
            $this->Eyelock_TamperNotifyMessage = "Tamper Occurred!";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.TamperNotifyMessage", $this->Eyelock_TamperNotifyMessage);
        }

        if (!$this->get("Eyelock.MaxTemplateCount", $this->Eyelock_MaxTemplateCount))
        {
            $this->Eyelock_MaxTemplateCount = "20000";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.MaxTemplateCount", $this->Eyelock_MaxTemplateCount);
        }

        if (!$this->get("Eyelock.AuthenticationMode", $this->Eyelock_AuthenticationMode))
        {
            $this->Eyelock_AuthenticationMode = 1; // setup a default
            $this->add("Eyelock.AuthenticationMode", $this->Eyelock_AuthenticationMode);
        }

        if (!$this->get("Eyelock.PinBurstBits", $this->Eyelock_PinBurstBits))
        {
            $this->Eyelock_PinBurstBits = 4; // setup a default
            $this->add("Eyelock.PinBurstBits", $this->Eyelock_PinBurstBits);
        }

        if (!$this->get("Eyelock.WaitPinTime", $this->Eyelock_WaitPinTime))
        {
            $this->Eyelock_WaitPinTime = 10; // setup a default
            $this->add("Eyelock.WaitPinTime", $this->Eyelock_WaitPinTime);
        }
        else
            $this->Eyelock_WaitPinTime /= 1000;

        if (!$this->get("Eyelock.SoftwareUpdateURL", $this->Eyelock_SoftwareUpdateURL))
        {
            $this->Eyelock_SoftwareUpdateURL = "https://eyelock.com/updates/nanonxtversioninfo.xml";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.SoftwareUpdateURL", $this->Eyelock_SoftwareUpdateURL);
        }

        if (!$this->get("Eyelock.SoftwareUpdateDateCheck", $this->Eyelock_SoftwareUpdateDateCheck))
        {
            $this->Eyelock_SoftwareUpdateDateCheck = "Never";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.SoftwareUpdateDateCheck", $this->Eyelock_SoftwareUpdateDateCheck);
        }

        if (!$this->get("Eyelock.SoftwareUpdateDateNano", $this->Eyelock_SoftwareUpdateDateNano))
        {
            $this->Eyelock_SoftwareUpdateDateNano = "Never";

            $lastUpdateTime = loadLastUpdateTime();

            if($lastUpdateTime == "")
            {
                if(file_exists("/home/updateInProgress.txt"))
                {
                    $this->Eyelock_SoftwareUpdateDateNano = "Previous Update Failed...";
                }
                else
                {


                $res = shell_exec("ls -l /home/firmware/nano/restorepoints |wc -l");
                $dirtest = strpos($res, "No such file or directory");
                if($dirtest === "false")
                {  if($res !== "0")
                    $this->Eyelock_SoftwareUpdateDateNano = "Previous Update Failed...";
                }
                else
                $this->Eyelock_SoftwareUpdateDateNano = "Timestamp not available.";
                }
            }
            else
                $this->Eyelock_SoftwareUpdateDateNano = $lastUpdateTime;

            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.SoftwareUpdateDateNano", $this->Eyelock_SoftwareUpdateDateNano);
        }
        else
            if(file_exists("/home/updateInProgress.txt"))
                    $this->Eyelock_SoftwareUpdateDateNano = "Previous Update Failed...";
        

        if (!$this->get("Eyelock.SoftwareUpdateDateBob", $this->Eyelock_SoftwareUpdateDateBob))
        {
            $this->Eyelock_SoftwareUpdateDateBob = "Never";
            // Missing (but required) elements must be 'added'
            
            
             $lastUpdateTime = loadLastBobUpdateTime();
              if(file_exists("/home/updateInProgress.txt"))
              {
                    $this->Eyelock_SoftwareUpdateDateNano = "Previous Update Failed...";
              }
            else
            {
                if($lastUpdateTime == "")
                {
                    $res = shell_exec("ls -l /home/firmware/nano/restorepoints |wc -l");
                        $dirtest = strpos($res, "No such file or directory");
                    if($dirtest === "false")
                    {  if($res !== "0")
                        $this->Eyelock_SoftwareUpdateDateBob = "Previous Update Failed...";
                    }
                    else
                        $this->Eyelock_SoftwareUpdateDateBob = "Timestamp not available.";
                }
                else
                    $this->Eyelock_SoftwareUpdateDateBob = $lastUpdateTime;

            }
            $this->add("Eyelock.SoftwareUpdateDateBob", $this->Eyelock_SoftwareUpdateDateBob);
        }
         else
            if(file_exists("/home/updateInProgress.txt"))
                    $this->Eyelock_SoftwareUpdateDateBob = "Previous Update Failed...";

        if (!$this->get("GRITrigger.EnableRelayWithSignal", $this->GRITrigger_EnableRelayWithSignal))
        {
            $this->GRITrigger_EnableRelayWithSignal = "false";
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("GRITrigger.EnableRelayWithSignal", $this->GRITrigger_EnableRelayWithSignal);
        }

        if (!$this->get("GRITrigger.RelayTimeInMS", $this->GRITrigger_RelayTimeInMS))
        {
            $this->GRITrigger_RelayTimeInMS = 3;
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("GRITrigger.RelayTimeInMS", $this->GRITrigger_RelayTimeInMS);
        }
        else
            $this->GRITrigger_RelayTimeInMS /= 1000;

        if (!$this->get("GRITrigger.DenyRelayTimeInMS", $this->GRITrigger_DenyRelayTimeInMS))
        {
            $this->GRITrigger_DenyRelayTimeInMS = 3;
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("GRITrigger.DenyRelayTimeInMS", $this->GRITrigger_DenyRelayTimeInMS);
        }
        else
            $this->GRITrigger_DenyRelayTimeInMS /= 1000;


/** Not configurable from UI
        if (!$this->get("GRI.AuthorizationToneFrequency", $this->GRI_AuthorizationToneFrequency))
        {
            $this->GRI_AuthorizationToneFrequency = "440"; // A
            //FireStepOcurred(new StatusMessage(string.Format("error in RetrieveConfigIni_SingleThreaded when getting details GRI.AuthorizationToneFrequency (Key is missing from file) from Eyelock.ini, Board's Ip: {0}", p_BoardIp), StatusMessageTypes.Error));
        }
**/

        if (!$this->get("GRI.AuthorizationToneVolume", $this->GRI_AuthorizationToneVolume))
        {
            $this->GRI_AuthorizationToneVolume = 10; //default I see no reason to not have this available...
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("GRI.AuthorizationToneVolume", $this->GRI_AuthorizationToneVolume);
        }

        if (!$this->get("GRI.AuthorizationToneDurationSeconds", $this->GRI_AuthorizationToneDurationSeconds))
        {
            $this->GRI_AuthorizationToneDurationSeconds = "4";
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("GRI.AuthorizationToneDurationSeconds", $this->GRI_AuthorizationToneDurationSeconds);
        }

        if (!$this->get("GRI.LEDBrightness", $this->GRI_LEDBrightness))
        {
            $this->GRI_LEDBrightness = "1";
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("GRI.LEDBrightness", $this->GRI_LEDBrightness);
        }
		else
            // Convert 0-255 from ini file into 0-100;
            $this->GRI_LEDBrightness = ($this->GRI_LEDBrightness/255)*100;

        
        if (!$this->get("GRI.TamperToneVolume", $this->GRI_TamperToneVolume))
        {
            $this->GRI_TamperToneVolume = 10; //default I see no reason to not have this available...
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("GRI.TamperToneVolume", $this->GRI_TamperToneVolume);
        }


        if (!$this->get("GRI.InternetTimeAddr", $this->GRI_InternetTimeAddr))
        {
            $this->GRI_InternetTimeAddr = "time.nist.gov";
            // Missing (but required) elements must be 'added'
            $this->add("GRI.InternetTimeAddr", $this->GRI_InternetTimeAddr);
        }

        if (!$this->get("GRI.InternetTimeSync", $this->GRI_InternetTimeSync))
        {
            $this->GRI_InternetTimeSync = "true";
            // Missing (but required) elements must be 'added'
            $this->add("GRI.InternetTimeSync", $this->GRI_InternetTimeSync);
        }

        if (!$this->get("Eyelock.HttpPostSenderDestAddress", $this->Eyelock_HttpPostSenderDestAddress))
        {
            $this->Eyelock_HttpPostSenderDestAddress = "";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.HttpPostSenderDestAddress", $this->Eyelock_HttpPostSenderDestAddress);
        }

        if (!$this->get("Eyelock.DeviceLocation", $this->Eyelock_DeviceLocation))
        {
            $this->Eyelock_DeviceLocation = "";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.DeviceLocation", $this->Eyelock_DeviceLocation);
        }

        if (!$this->get("Eyelock.WelcomeMessage", $this->Eyelock_WelcomeMessage))
        {
            $this->Eyelock_WelcomeMessage = "";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.WelcomeMessage", $this->Eyelock_WelcomeMessage);
        }

        if (!$this->get("Eyelock.HttpPostSenderDestPostIris", $this->Eyelock_HttpPostSenderDestPostIris))
        {
            $this->Eyelock_HttpPostSenderDestPostIris = "";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.HttpPostSenderDestPostIris", $this->Eyelock_HttpPostSenderDestPostIris);
        }

        if (!$this->get("Eyelock.HttpPostSenderDestSignalError", $this->Eyelock_HttpPostSenderDestSignalError))
        {
            $this->Eyelock_HttpPostSenderDestSignalError = "";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.HttpPostSenderDestSignalError", $this->Eyelock_HttpPostSenderDestSignalError);
        }

        if (!$this->get("Eyelock.HttpPostSenderDestSignalHeartBeat", $this->Eyelock_HttpPostSenderDestSignalHeartBeat))
        {
            $this->Eyelock_HttpPostSenderDestSignalHeartBeat = "";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.HttpPostSenderDestSignalHeartBeat", $this->Eyelock_HttpPostSenderDestSignalHeartBeat);
        }

        if (!$this->get("Eyelock.HttpPostSenderDestSignalMaintenance", $this->Eyelock_HttpPostSenderDestSignalMaintenance))
        {
            $this->Eyelock_HttpPostSenderDestSignalMaintenance = "";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.HttpPostSenderDestSignalMaintenance", $this->Eyelock_HttpPostSenderDestSignalMaintenance);
        }

        if (!$this->get("Eyelock.HttpPostSenderDestScheme", $this->Eyelock_HttpPostSenderDestScheme))
        {
            $this->Eyelock_HttpPostSenderDestScheme = "https";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.HttpPostSenderDestScheme", $this->Eyelock_HttpPostSenderDestScheme);
        }

        if (!$this->get("Eyelock.SPAWAREnable", $this->Eyelock_SPAWAREnable))
        {
            $this->Eyelock_SPAWAREnable = "false";
            // Missing (but required) elements must be 'added'
            $this->add("Eyelock.SPAWAREnable", $this->Eyelock_SPAWAREnable);
        }

        if (!$this->get("GRI.EyeDestAddr", $this->GRI_EyeDestAddr))
        {
            $this->GRI_EyeDestAddr = "";
            // Missing (but required) elements must be 'added'
            $this->add("GRI.EyeDestAddr", $this->GRI_EyeDestAddr);
        }

        if ($this->get("GRI.RepeatAuthorizationPeriod", $this->GRI_RepeatAuthorizationPeriod))
        {
            $this->GRI_RepeatAuthorizationPeriod /= 1000;
        }
        else
        {
            $this->GRI_RepeatAuthorizationPeriod = 4;
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("GRI.RepeatAuthorizationPeriod", $this->GRI_RepeatAuthorizationPeriod);
        }

        if (!$this->get("GRI.NwListenerSecure", $this->GRI_SecureComm))
            $this->GRI_SecureComm = "secure"; // default to true if not there... this value is never saved...

/** Securecomm is out               
        if ($this->get("GRI.NwListenerSecure", $this->GRI_SecureComm) && $this->get("GRI.HBDispatcherSecure", $temp)
                && $this->get("GRI.NwDispatcherSecure", $temp) && $this->get("Eyelock.PullDBSecure", $temp)
                && $this->get("GRI.EyeDispatcherSecure", $temp) && $this->get("Eyelock.MasterSlaveCommSecure", $temp)
                && $this->get("Eyelock.SlaveMasterCommSecure", $temp) && $this->get("Eyelock.WeigandDispatcherSecure", $temp)
                && $this->get("Eyelock.F2FDispatcherSecure", $temp) && $this->get("Eyelock.NwLEDDispatcherSecure", $temp))
        {
            //reached here, when string "<...>secure" is there (though be it secure/nonsecure)
            // dmo, not sure about this logic.  seems unnecessary
            if ($this->GRI_SecureComm != "secure")
                $this->GRI_SecureComm = "nonsecure";
        }
        else
        {
            //reached here, only when string "<...>secure" is not there
            $this->GRI_SecureComm = "nonsecure";
        }
**/

        // Help dialog
        if (!$this->get("Eyelock.EnablePopupHelp", $this->Eyelock_EnablePopupHelp))
        {
            $this->Eyelock_EnablePopupHelp = "true";
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("Eyelock.EnablePopupHelp", $this->Eyelock_EnablePopupHelp);
        }

        if (!$this->get("Eyelock.PopupHelpTriggerHover", $this->Eyelock_PopupHelpTriggerHover))
        {
            $this->Eyelock_PopupHelpTriggerHover = 1;
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("Eyelock.PopupHelpTriggerHover", $this->Eyelock_PopupHelpTriggerHover);
        }

        if (!$this->get("Eyelock.PopupHelpDelay", $this->Eyelock_PopupHelpDelay))
        {
            $this->Eyelock_PopupHelpDelay = 1000;
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("Eyelock.PopupHelpDelay", $this->Eyelock_PopupHelpDelay);

            $this->Eyelock_PopupHelpDelay = 1;
        }
        else
            $this->Eyelock_PopupHelpDelay /= 1000;


        if (!$this->get("Eyelock.AllowSiteAdminUpgrade", $this->Eyelock_AllowSiteAdminUpgrade))
        {
            $this->Eyelock_AllowSiteAdminUpgrade = "false";
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("Eyelock.AllowSiteAdminUpgrade", $this->Eyelock_AllowSiteAdminUpgrade);
        }
         if (!$this->get("Eyelock.TLSEnable", $this->Eyelock_TLSEnable))
        {
            $this->Eyelock_TLSEnable = "true";
            // ALWAYS add the value if it's missing from the Eyelock.ini file
            $this->add("Eyelock.TLSEnable", $this->Eyelock_TLSEnable);
        }
       //// if (!$this->get("Eyelock.NwMatcherCommSecure", $this->Eyelock_NwMatcherCommSecure))
      //  {
      //      $this->Eyelock_NwMatcherCommSecure = "unsecure";
      //      $this->add("Eyelock.NwMatcherCommSecure", $this->Eyelock_NwMatcherCommSecure);
      //  }
    }
  

    /** get parameters from any place in your project with this function */
    function get_ini_parameter($key)
    {
        $return = "";

	    if (!$this->get($key &$return))
    	    echo "variable $key does not exist !";
    
	    return $return;
    }


    // set new parameters from any place in your project with this function
    function set_ini_parameter ($key, $value)
    {
	    $this->set($key, $value);
	    return;
    }
	//write the log level to the nxtLog.cfg file
	
	
	//////////////////////////////////////////////////////////////////////////////////////////////
	// Called when user hits submit to process the form's new values
	// Only changed values are updated into the internal parameters array and are then written to disk
    // Further processing may occur after the element is written out to the file in the save() function
    //////////////////////////////////////////////////////////////////////////////////////////////
	function UpdateIniValues($row)
	{
        // Flags for checkbox handling (we don't get POST'd values for unchecked boxes)
        $FoundSecure = FALSE;
        $bFoundInternetTimeSync = FALSE;
        //OSDP checkboxes disabled... set these to "true"
       // $bFoundOSDPEnabled = TRUE;
        // $bFoundOSDPInputEnabled = TRUE;
        $bFoundTemplateOnCardPass = FALSE;
        $bFoundDualAuthEnabled = FALSE;
        $bFoundTemplateOnCard = FALSE;
        $bFoundDualAuthParityEnabled = FALSE;
        $bFoundDualAuthLEDEnabled = FALSE;
        $bFoundHDMatcherEnabled = FALSE;
        $bFoundRelayEnable = FALSE;
     //   $bFoundEnableIEEE8021X = FALSE;
        $bFoundEnableNegativeMatchTimeout = FALSE;
        $bFoundEnablePopupHelp = FALSE;
        $bFoundAllowSiteAdminUpgrade = FALSE;
        $bFoundDispatchSecure = FALSE;
		$bODSPEnabled = FALSE;
       // $bFoundNwMatcherCommSecure = FALSE;
		$bFound_Eyelock_SystemReadyDebug = false;
		$bFound_Eyelock_Debug = false;
		$bFound_Eyelock_MatchDispDebug = false;
		$bFound_Eyelock_OpenSSL_Debug = false;
		$bFound_Eyelock_SSLDebug  = false;
		$bFound_GRI_DBDebug = false;
		$bFound_GRI_EyeDispatcherDebug = false;
		$bFound_GRI_HBDebug = false;
		$bFound_GRI_HDDebug = false;
		$bFound_GRI_LEDDebug = false;
		$bFound_GRI_MPDebug = false;
		
		$bFound_GRI_NwDebug = false;
		$bFound_GRI_NwMmDebug = false;
		$bFound_GRI_SpoofDebug = false;
		$bFound_GRITrigger_CmxDebug = false;
		$bFound_GRITrigger_F2FDebug = false;
		$bFound_GRITrigger_WeigandDebug = false;
		$bFound_MT9P001_Debug = false;
		$bFound_NwListener_Debug = false;
		$bFound_GRI_MatchMgrDebug = false;
        $bFound_HttpPostSenderDest = false; 
        $bFound_HttpPostSenderDestPostIris = false;
        $bFound_HttpPostSenderDestSignalError = false;
        $bFound_HttpPostSenderDestSignalHeartBeat = false;
        $bFound_HttpPostSenderDestSignalMaintenance = false;

       //INSERT NEW PARAMETER CHECKBOX PARAMETER FALSE HERE
        
        // Get the posted dropdown selection and map to flags as appropriate...
		foreach ($row as $key => $value) 
		{
            $logme = sprintf("Writing to ini %s  =>  %s", $key, $value);
    //        error_log($logme);
          //if($key === "OSDPEnable")
          //{
           //   $this->set("GRITrigger.OSDPEnable", $value);
            ////   $bFoundOSDPEnabled = TRUE;
         // }
         // else 
          //if($key === "OSDPInputEnable")
         //// {
            //  $this->set("GRITrigger.OSDPInputEnable", $value);
             //  $bFoundOSDPInputEnabled = TRUE;
         // }
			//else 
			
            if  ($key === "protocol")
			{
				if (($value === "osdp") )
				{
					$this->set("GRITrigger.OSDPEnable", "true");
                   
					$this->set("GRITrigger.WeigandEnable", "true");
                    $this->set("GRITrigger.WeigandHidEnable", "false");
					$this->set("GRITrigger.PACEnable", "false");
                   $this->set("GRITrigger.F2FEnable", "false");
					$bODSPEnabled = true;
				}
				if (($value === "weigand") )
				{
					$this->set("GRITrigger.OSDPEnable", "false");
                    // $this->set("GRITrigger.OSDPInputEnable", "false");
					$this->set("GRITrigger.WeigandEnable", "true");
                    $this->set("GRITrigger.WeigandHidEnable", "false");
					$this->set("GRITrigger.PACEnable", "false");
                    $this->set("GRITrigger.F2FEnable", "false");
				}
				else if (($value === "hid") )
				{
					$this->set("GRITrigger.OSDPEnable", "false");
                    //  $this->set("GRITrigger.OSDPInputEnable", "false");
					$this->set("GRITrigger.WeigandEnable", "false");
                    $this->set("GRITrigger.WeigandHidEnable", "true");
					$this->set("GRITrigger.PACEnable", "false");
                    $this->set("GRITrigger.F2FEnable", "false");
				}
				else if (($value === "pac") )
				{
					$this->set("GRITrigger.OSDPEnable", "false");
                    //  $this->set("GRITrigger.OSDPInputEnable", "false");
					$this->set("GRITrigger.WeigandEnable", "false");
                    $this->set("GRITrigger.WeigandHidEnable", "false");
					$this->set("GRITrigger.PACEnable", "true");
                    $this->set("GRITrigger.F2FEnable", "false");
				}
				else if (($value === "f2f") )
				{
					$this->set("GRITrigger.OSDPEnable", "false");
                  //    $this->set("GRITrigger.OSDPInputEnable", "false");
					$this->set("GRITrigger.WeigandEnable", "false");
                    $this->set("GRITrigger.WeigandHidEnable", "false");
					$this->set("GRITrigger.PACEnable", "false");
                    $this->set("GRITrigger.F2FEnable", "true");
				}
			}
            else if ($key === "accessControlType")
			{
                // If portable template is selected, we have already forced this setting to "3".  Don't overwrite that....
                if (!$bFoundTemplateOnCard)
                {
				    if (($value === "IrisOnly") )
				    {
					    $this->set("Eyelock.AuthenticationMode", "1");
                    }
				    else if ($value === "IrisorCard")
				    {
					    $this->set("Eyelock.AuthenticationMode", "2");
                    }
				    else if ($value === "IrisandCard")
				    {
					    $this->set("Eyelock.AuthenticationMode", "3");
                    }
				    else if ($value === "IrisandCardPP")
				    {
					    $this->set("Eyelock.AuthenticationMode", "4");
                    }
				    else if ($value === "PinandIris")
				    {
					    $this->set("Eyelock.AuthenticationMode", "5");
                    }
				    else if ($value === "PinCardandIris")
				    {
					    $this->set("Eyelock.AuthenticationMode", "6");
                    }
				    else if ($value === "PinandIrisDuress")
				    {
					    $this->set("Eyelock.AuthenticationMode", "7");
                    }
				    else if ($value === "PinCardandIrisDuress")
				    {
					    $this->set("Eyelock.AuthenticationMode", "8");
                    }
                }
            }
            else if ($key === "pinBurstBits")
            {
   				if ($value === "FourBits")
                   $this->set("Eyelock.PinBurstBits", "4");
                else
                   $this->set("Eyelock.PinBurstBits", "8");
            }
            else if ($key === "Eyelock_WaitPinTime")
            {
               $this->set("Eyelock.WaitPinTime", $value*1000);
            }
			else if  ($key === "loglevel")
			{
				writeOutLogLevel($value); //notee: log levels stored in the HTML.  MAybe a security problem, probably not.  Its writing to a file.
			}
			else if  ($key === "GRI_EyeConnecTimeoutmsec")
			{
				$this->set("GRI.EyeConnecTimeoutmsec",$value); //notee: log levels stored in the HTML.  MAybe a security problem, probably not.  Its writing to a file.
			}
			else if  ($key === "GRI_EyeSendTimeoutmsec")
			{
				$this->set("GRI.EyeSendTimeoutmsec",$value); //notee: log levels stored in the HTML.  MAybe a security problem, probably not.  Its writing to a file.
				
			}
			//INSERT NEW PARAMETER SET HERE.  IF its a checkbox go to the next INSERT comment as well
			//GRI_MatchMgrDebug
			else if ( $key === "GRI_MatchMgrDebug" ) {
				$this->set( "GRI.MatchMgrDebug", $value );
				$bFound_GRI_MatchMgrDebug = true;
			} 
			else if ( $key === "Eyelock_SystemReadyDebug" ) {
				$this->set( "Eyelock.SystemReadyDebug", $value );
				$bFound_Eyelock_SystemReadyDebug = true;
			} 
			else if ( $key === "Eyelock_Debug" ) {
				$this->set( "Eyelock.Debug", $value );
				$bFound_Eyelock_Debug = true;
			} 
			else if ( $key === "Eyelock_MatchDispDebug" ) {
				$this->set( "Eyelock.MatchDispDebug", $value );
				$bFound_Eyelock_MatchDispDebug = true;
			} 
			else if ( $key === "Eyelock_OpenSSL_Debug" ) {
				$this->set( "Eyelock.OpenSSL.Debug", $value );
				$bFound_Eyelock_OpenSSL_Debug = true;
			} 
			else if ( $key === "Eyelock_SSLDebug" ) {
		//		error_log("found Eyelock.SSLDebug");
				$this->set( "Eyelock.SSLDebug", "true" );
				$bFound_Eyelock_SSLDebug = true;
		//		error_log(" found it is " .$bFound_Eyelock_SSLDebug);
			} 
			else if ( $key === "GRI_DBDebug" ) {
				$this->set( "GRI.DBDebug", $value );
				$bFound_GRI_DBDebug = true;
			} 
			else if ( $key == "GRI_EyeDispatcherDebug" ) {
				$this->set( "GRI.EyeDispatcherDebug", $value );
				$bFound_GRI_EyeDispatcherDebug = true;
			} 
			else if ( $key === "GRI_HBDebug" ) {
				$this->set( "GRI.HBDebug", $value );
				setSlaveConfigValue("7", "true");
				$bFound_GRI_HBDebug = true;
			} 
			else if ( $key === "GRI_HDDebug" ) {
				$this->set( "GRI.HDDebug", $value );
				setSlaveConfigValue("8", "true");
				$bFound_GRI_HDDebug = true;
			} 
			else if ( $key === "GRI_LEDDebug" ) {
				$this->set( "GRI.LEDDebug", "true" );
				$bFound_GRI_LEDDebug = true;
				//error_log("found it is ".$bFound_GRI_LEDDebug);
			} 
			else if ( $key === "GRI_MPDebug" ) {
				$this->set( "GRI.MPDebug", $value );
				$bFound_GRI_MPDebug = true;
			}
			else if ( $key === "GRI_NwDebug" ) {
				$this->set( "GRI.NwDebug", $value );
				$bFound_GRI_NwDebug = true;
			} 
			else if ( $key === "GRI_NwMmDebug" ) {
				$this->set( "GRI.NwMmDebug", $value );
				$bFound_GRI_NwMmDebug = true;
			} 
			else if ( $key === "GRI_SpoofDebug" ) {
				$this->set( "GRI.SpoofDebug", $value );
				$bFound_GRI_SpoofDebug = true;
			} 
			else if ( $key === "GRITrigger_CmxDebug" ) {
				$this->set( "GRITrigger.CmxDebug", $value );
				$bFound_GRITrigger_CmxDebug = true;
			} 
			else if ( $key === "GRITrigger_F2FDebug" ) {
				$this->set( "GRITrigger.F2FDebug", $value );
				$bFound_GRITrigger_F2FDebug = true;
			} 
			else if ( $key === "GRITrigger_WeigandDebug" ) {
				$this->set( "GRITrigger.WeigandDebug", $value );
				$bFound_GRITrigger_WeigandDebug = true;
			} 
			else if ( $key === "MT9P001_Debug" ) {
				$this->set( "MT9P001.Debug", $value );
				setSlaveConfigValue("17", "true");
				$bFound_MT9P001_Debug = true;
			} 
			else if ( $key === "NwListener_Debug" ) {
				$this->set( "NwListener.Debug", $value );
				$bFound_NwListener_Debug = true;
			}
			
/*
            else if ($key === GRITrigger_OSDPEnabled)
            {
                $this->set("GRITrigger.OSDPEnabled", "true");
                $bFoundOSDPEnabled = TRUE;
            }
*/
            else if($key === "mobileMode")
            {
                 $this->set("GRITrigger.MobileMode", $value);

            }
            else if ($key === "GRITrigger_DualAuthenticationMode")
            {
                $this->set("GRITrigger.DualAuthenticationMode", "true");
                $this->set("GRITrigger.OSDPInputEnable", "false");
                $bFoundDualAuthEnabled = TRUE;
            }
            else if ($key === "GRITrigger_TemplateOnCard")
            {
                $this->set("GRITrigger.TransTOCMode", "true");
                $bFoundTemplateOnCard = TRUE;
                //help prevent db deletion
                shell_exec("cp /home/default/test.db /home/root/");  //just always copy
				$this->set("Eyelock.AuthenticationMode", "3");
            }
            else if ($key === "GRITrigger_TOCPassThrough")
            {
                $this->set("GRITrigger.PassThroughMode", "true");
                $bFoundTemplateOnCardPass = TRUE;
            }
            else if ($key === "useParityMaskEnabled")
            {
                //error_log("save ptkmc ".$value);
                $this->set("GRITrigger.DualAuthenticationParity", "false");
				 $bFoundDualAuthParityEnabled = TRUE;
            }
            else if ($key === "useParityMaskDisabled")
            {
                //error_log("save ptkmc ".$value);
                $this->set("GRITrigger.DualAuthenticationParity", "true");
				 $bFoundDualAuthParityEnabled = TRUE;
            }
            else if ($key === "GRITrigger_DualAuthNLedControlledByACS")
            {
                $this->set("GRITrigger.DualAuthNLedControlledByACS", "true");
                $bFoundDualAuthLEDEnabled = TRUE;
            }
            else if ($key === "Eyelock_ACPTestCardID")
            {
                if ($value != $this->Eyelock_ACPTestCardID)
      				$this->set("Eyelock.ACPTestCardID", $value);
            }
            else if ($key === "Eyelock_ACPTestFacilityCode")
            {
                if ($value != $this->Eyelock_ACPTestFacilityCode)
      				$this->set("Eyelock.ACPTestFacilityCode", $value);
            }
			else if ($key === "server_port")
			{
                if ($value != $this->server_port)
    				$this->set("server.port", $value);
		    }
/** Out

// Secure is no longer editable, it's always enabled and on be default from the factory.
            else if ($key === "GRI_NwListenerSecure")
            {
                // Handle case of going from nonsecure to secure...
                if ($value != $this->GRI_SecureComm)
                {
                    $this->set("GRI.NwListenerSecure", "secure", "");
                    // Need to add in all the other values...
                    $this->set("GRI.HBDispatcherSecure", "secure", "");
                    $this->set("GRI.NwDispatcherSecure", "secure", "");
                    $this->set("Eyelock.PullDBSecure",  "secure", "");
                    $this->set("GRI.EyeDispatcherSecure", "secure", "");
                    $this->set("Eyelock.MasterSlaveCommSecure", "secure", "");
                    $this->set("Eyelock.SlaveMasterCommSecure", "secure", "");
                    $this->set("Eyelock.WeigandDispatcherSecure", "secure", "");
                    $this->set("Eyelock.F2FDispatcherSecure", "secure", "");
                    $this->set("Eyelock.NwLEDDispatcherSecure", "secure", "");

                    $FoundSecure = TRUE; 
                }
            }
**/
            else if ($key === "GRI_EnableNWHDMatcher")
            {
                $bFoundHDMatcherEnabled = TRUE;

                $this->GRI_HDMatcherID = sprintf("%d", $this->MatcherIndex);
                $this->set("GRI.HDMatcherID", $this->GRI_HDMatcherID);
            }
            else if ($key === "GRI_HDMatcher_Address")
            {
                // Ok, we found the IP now we need to grab the port...
                $networkip = trim($value);
                if (strlen($networkip)>0)
                {
                    $networkip .= ":";
                    $networkip .= trim($row['GRI_NWHDPort']);
                }

                if ($networkip != $this->GRI_HDMatcher_Address)
                {
                    $this->MatcherAddress = sprintf("GRI.HDMatcher.%d.Address", $this->MatcherIndex);
				    $this->set($this->MatcherAddress, $networkip, "");
                }

                // Always write these other values...

                $MatcherType = sprintf("GRI.HDMatcher.%d.Type", $this->MatcherIndex);
                // We found a PCMATCHER entry... make sure address and buffsize are there...
                $this->set($MatcherType, "PCMATCHER");

                // Make sure BuffSize exists...
                $BufSize = sprintf("GRI.HDMatcher.%d.BuffSize", $this->MatcherIndex);
                $this->set($BufSize, "0");

                // Also set the count...
                $this->GRI_HDMatcherCount = sprintf("%d", $this->MatcherCount);
                $this->set("GRI.HDMatcherCount", $this->GRI_HDMatcherCount);

            }
			// so that we can pass in the correct key name...
			else if ($key === "GRI_MatchResultDestAddr")
			{
                // Ok, we found the IP now we need to grab the port...
                $networkip = trim($value);
                if (strlen($networkip)>0)
                {
                    $networkip .= ":";
                    $networkip .= trim($row['GRI_MatchResultDestPort']);
                }

                if ($networkip != $this->GRI_MatchResultDestAddr)
				    $this->set("GRI.MatchResultDestAddr", $networkip, "");
			}
            else if ($key === "GRI_MatchResultNwMsgFormat")
            {
                if ($value != $this->GRI_MatchResultNwMsgFormat)
      				$this->set("GRI.MatchResultNwMsgFormat", $value);
            }
            else if ($key === "GRI_NwDispatcherSecure")
            {
                $this->set("GRI.NwDispatcherSecure", "secure");
                $bFoundDispatchSecure = TRUE;
            }

      
            else if ($key === "Eyelock_DualMatcherPolicy")
            {
               
                // This may not have previously existed... so must always write it...
                // even if it didn't change, shouldn't be an issue...
  				$this->set("Eyelock.DualMatcherPolicy", $value);
            }
            else if ($key === "tlsmoderadio")
            {
                $this->set("Eyelock.TLSEnable", $value);
                if($value === "true")
                {
                    //error_log("SetTLSONlyWebCOnfig engage");
                     $this->setTLSOnlyWebConfig(1);
					$temp = "";
					
					if(!$this->get("Eyelock.Cipher", $temp ))
						$this->add("Eyelock.Cipher","TLSv1.2");
					$this->set("Eyelock.Cipher","TLSv1.2");
                }
                else
				{
                  $this->setTLSOnlyWebConfig(0);
					//is cipher suite available?
					if(!$this->get("Eyelock.Cipher", $temp )) //cipher suites selected should allow for SSL3 support,  as if we need it at this point...
						$this->add("Eyelock.Cipher","TLSv1.2:TLSv1:SSLv3");
					$this->set("Eyelock.Cipher","TLSv1.2:TLSv1:SSLv3");
					//$this->remove("Eyelock.Cipher"); // It's impossible currently to delete a line from the ini through web config.
				}

            }
            //OSDPBAUDRate
             else if ($key === "osdpBaudRate")
            {
                // This may not have previously existed... so must always write it...
                // even if it didn't change, shouldn't be an issue...
  				$this->set("Eyelock.OSDPBaudRate", $value);
            }
            //Eyelock.OSDPAddress
             else if ($key === "OSDP_Address")
            {
                // This may not have previously existed... so must always write it...
                // even if it didn't change, shouldn't be an issue...
  				$this->set("Eyelock.OSDPAddress", $value);
            }
            else if ($key === "GRI_AuthorizationToneVolume")
            {
                if ($value != $this->GRI_AuthorizationToneVolume)
      				$this->set("GRI.AuthorizationToneVolume", $value);
            }
//            else if ($key === "GRI_AuthorizationToneFrequency")
//            {
//                if ($value != $this->GRI_AuthorizationToneFrequency)
//      				$this->set("GRI.AuthorizationToneFrequency", $value);
//            }
            else if ($key === "GRI_AuthorizationToneDurationSeconds")
            {
                if ($value != $this->GRI_AuthorizationToneDurationSeconds)
      				$this->set("GRI.AuthorizationToneDurationSeconds", $value);
            }
            else if ($key === "GRI_RepeatAuthorizationPeriod")
            {
                if ($value != $this->GRI_RepeatAuthorizationPeriod*1000)
      				$this->set("GRI.RepeatAuthorizationPeriod", $value*1000);
            }
            else if ($key === "GRI_TamperToneVolume")
            {
                if ($value != $this->GRI_TamperToneVolume)
      				$this->set("GRI.TamperToneVolume", $value);
            }
            else if ($key === "GRI_LEDBrightness")
            {
                if ($value != $this->GRI_LEDBrightness)
      				$this->set("GRI.LEDBrightness", ($value/100)*255);
            }
            else if ($key === "GRI_InternetTimeAddr")
            {
                // Since this may not already exist in the file, we always write it out.
                //if ($value != $this->GRI_InternetTimeAddr)
  				$this->set("GRI.InternetTimeAddr", $value);
            }
            else if ($key === "GRI_InternetTimeSync")
            {
                // Since this may not already exist in the file, we always write it out.
                //if ($value != $this->GRI_InternetTimeSync)
      			$this->set("GRI.InternetTimeSync", "true");
                $bFoundInternetTimeSync = TRUE;
            }
            else if ($key == "Eyelock_DeviceLocation")
            {
                // Since this may not already exist in the file, we always write it out.
  				$this->set("Eyelock.DeviceLocation", $value);
            }
            else if ($key == "Eyelock_WelcomeMessage")
            {
                // Since this may not already exist in the file, we always write it out.
  				$this->set("Eyelock.WelcomeMessage", $value);
            }
            else if ($key == "Eyelock_HttpPostSenderDestAddress")
            {
                // Since this may not already exist in the file, we always write it out.
  				$this->set("Eyelock.HttpPostSenderDestAddress", $value);

                if ($value != "")
                   $bFound_HttpPostSenderDest = true; 
            }
            else if ($key == "Eyelock_HttpPostSenderDestPostIris")
            {
                // Since this may not already exist in the file, we always write it out.
  				$this->set("Eyelock.HttpPostSenderDestPostIris", $value);
                
                if ($value != "")
                   $bFound_HttpPostSenderDestPostIris = true; 
            }
            else if ($key == "Eyelock_HttpPostSenderDestSignalError")
            {
                // Since this may not already exist in the file, we always write it out.
  				$this->set("Eyelock.HttpPostSenderDestSignalError", $value);

                if ($value != "")
                   $bFound_HttpPostSenderDestSignalError = true; 
            }
            else if ($key == "Eyelock_HttpPostSenderDestSignalHeartBeat")
            {
                // Since this may not already exist in the file, we always write it out.
  				$this->set("Eyelock.HttpPostSenderDestSignalHeartBeat", $value);

                if ($value != "")
                   $bFound_HttpPostSenderDestSignalHeartBeat = true; 
            }
            else if ($key == "Eyelock_HttpPostSenderDestSignalMaintenance")
            {
                // Since this may not already exist in the file, we always write it out.
  				$this->set("Eyelock.HttpPostSenderDestSignalMaintenance", $value);

                if ($value != "")
                   $bFound_HttpPostSenderDestSignalMaintenance = true; 
            }
            else if ($key == "Eyelock_HttpPostSenderDestScheme")
            {
                // Since this may not already exist in the file, we always write it out.
  				$this->set("Eyelock.HttpPostSenderDestScheme", $value);
            }
            else if ($key === "GRI_EyeDestAddr")
			{
                // Ok, we found the IP now we need to grab the port...
                $networkip = $value;

                if (strlen($networkip)>0)
                {
                    $networkip .= ":";
                    $networkip .= $row['GRI_EyeDestPort'];
                }

    		    $this->set("GRI.EyeDestAddr", $networkip, "");
			}
/*
            else if ($key === "Eyelock_EnableIEEE8021X")
			{
                $this->set("Eyelock.EnableIEEE8021X", "true");
                $bFoundEnableIEEE8021X = TRUE;
            }
*/
            else if ($key === "Eyelock_EnableNegativeMatchTimeout")
            {
   				$this->set("Eyelock.EnableNegativeMatchTimeout", "1"); // Will be 0 or 1
                $bFoundEnableNegativeMatchTimeout = TRUE;
            }
            else if ($key === "Eyelock_NegativeMatchTimeout")
            {
   				$this->set("Eyelock.NegativeMatchTimeout", $value*1000);
            }
            else if ($key === "Eyelock_NegativeMatchResetTimer")
            {
   				$this->set("Eyelock.NegativeMatchResetTimer", $value);
            }
            else if ($key === "Eyelock_TamperSignalHigh")
            {
                // This may not have previously existed... so must always write it...
                // even if it didn't change, shouldn't be an issue...
  				$this->set("Eyelock.TamperSignalHighToLow", $value);
            }
           
             else if ($key === "portableTemplatekeyMgmtCustom")
            {
                //error_log("save ptkmc ".$value);
                $this->set("GRITrigger.PortableTemplatesUseCustomKey",$value);
            }
              else if ($key === "portableTemplatekeyMgmtDefault")
            {

                $this->set("GRITrigger.PortableTemplatesUseCustomKey","false");
            }

            else if ($key === "Eyelock_TamperOutSignalHigh")
            {
                // This may not have previously existed... so must always write it...
                // even if it didn't change, shouldn't be an issue...
  				$this->set("Eyelock.TamperOutSignalHighToLow", $value);
            }
             else if ($key === "Eyelock_TamperSignalLow")
            {
                // This may not have previously existed... so must always write it...
                // even if it didn't change, shouldn't be an issue...
  				$this->set("Eyelock.TamperSignalHighToLow", "false");
            }
            else if ($key === "Eyelock_TamperOutSignalLow")
            {
                // This may not have previously existed... so must always write it...
                // even if it didn't change, shouldn't be an issue...
  				$this->set("Eyelock.TamperOutSignalHighToLow", "false");
            }
            else if ($key === "Eyelock_TamperNotifyAddr")
			{
                // Ok, we found the IP now we need to grab the port...
                $networkip = $value;

                if (strlen($networkip)>0)
                {
                    $networkip .= ":";
                    $networkip .= $row['Eyelock_TamperNotifyPort'];
                }

                // Always add, it may not have existed...
            //    if ($networkip != $this->Eyelock_TamperNotifyAddr)
			    $this->set("Eyelock.TamperNotifyAddr", $networkip, "");
			}
            else if ($key === "Eyelock_TamperNotifyMessage")
            {
                // Since this may not already exist in the file, we always write it out.
  				$this->set("Eyelock.TamperNotifyMessage", $value);
            }
/*
            else if ($key === "Eyelock_MaxTemplateCount")
            {
                // Since this may not already exist in the file, we always write it out.
  				$this->set("Eyelock.MaxTemplateCount", $value);
            }
*/
            else if ($key === "GRITrigger_EnableRelayWithSignal")
            {
   				$this->set("GRITrigger.EnableRelayWithSignal", "true");
                $bFoundRelayEnable = TRUE;
            }
            else if($key == "GRITrigger_DualAuthNCardMatchWaitIrisTime2")
            {
   				$this->set("GRITrigger.TOCCardExpiredTime", $value);
            }
            else if ($key === "GRITrigger_DualAuthNCardMatchWaitIrisTime")
            {
                if ($value != $this->GRITrigger_DualAuthNCardMatchWaitIrisTime*1000)
      				$this->set("GRITrigger.DualAuthNCardMatchWaitIrisTime", $value*1000);
            }
            else if ($key === "GRITrigger_RelayTimeInMS")
            {
                // Since this may not already exist in the file, we always write it out.
  				$this->set("GRITrigger.RelayTimeInMS", $value*1000);
            }

            else if ($key === "GRITrigger_DenyRelayTimeInMS")
            {
                // Since this may not already exist in the file, we always write it out.
  				$this->set("GRITrigger.DenyRelayTimeInMS", $value*1000);
            }
            else if ($key === "Eyelock_EnablePopupHelp")
            {
   				$this->set("Eyelock.EnablePopupHelp", "true");
                $bFoundEnablePopupHelp = TRUE;
            }
   			else if  ($key === "helptrigger")
			{
				$this->set("Eyelock.PopupHelpTriggerHover", ($value === "hover") ? 1 : 0);
            }
            else if ($key === "Eyelock_PopupHelpDelay")
            {
   				$this->set("Eyelock.PopupHelpDelay", $value*1000);
            }
            else if ($key === "Eyelock_AllowSiteAdminUpgrade")
            {
   				$this->set("Eyelock.AllowSiteAdminUpgrade", "true");
                $bFoundAllowSiteAdminUpgrade = TRUE;
            }
           // else if ($key === "NW_Matcher_Comm_Secure")
            //{
   			//	$this->set("Eyelock.NwMatcherCommSecure", "secure");
             //   $bFoundNwMatcherCommSecure = TRUE;
           // }
		}

/** SecureComm is out
        // We don't get POSTs for unchecked checkboxes..
        if (!$FoundSecure)
        {
            if ("nonsecure" != $this->GRI_SecureComm)
            {
                $this->set("GRI.NwListenerSecure", "nonsecure", "");
                // Set all the others...
                $this->set("GRI.HBDispatcherSecure", "nonsecure", "");
                $this->set("GRI.NwDispatcherSecure", "nonsecure", "");
                $this->set("Eyelock.PullDBSecure",  "nonsecure", "");
                $this->set("GRI.EyeDispatcherSecure", "nonsecure", "");
                $this->set("Eyelock.MasterSlaveCommSecure", "nonsecure", "");
                $this->set("Eyelock.SlaveMasterCommSecure", "nonsecure", "");
                $this->set("Eyelock.WeigandDispatcherSecure", "nonsecure", "");
                $this->set("Eyelock.F2FDispatcherSecure", "nonsecure", "");
                $this->set("Eyelock.NwLEDDispatcherSecure", "nonsecure", "");
            }
        }
**/

		//INSERT NEW PARAMETER CHECKBOX FALSE SET HERE.  If checkbox was not checked the browser doesn't send it.  Use the boolean for the new checkbox and set False if the boolean is false
        // Need this because 'unchecked' boxes aren't added to the POST array (crap design, thx HTML).
        if (!$bFoundInternetTimeSync)
            $this->set("GRI.InternetTimeSync", "false");
 
        if (!$bFoundTemplateOnCard)
            $this->set("GRITrigger.TransTOCMode", "false");

        if (!$bFoundDualAuthEnabled)
		{
            $this->set("GRITrigger.DualAuthenticationMode", "false");
			$this->set("GRITrigger.OSDPInputEnable", "false");
		}
            
        if (!$bFoundTemplateOnCardPass)
            $this->set("GRITrigger.PassThroughMode", "false");
        if(!$bFoundDualAuthParityEnabled)
             $this->set("GRITrigger.DualAuthenticationParity", "true");
        if (!$bFoundDualAuthLEDEnabled)
            $this->set("GRITrigger.DualAuthNLedControlledByACS", "false");

		if(!$bFound_GRI_MatchMgrDebug)
		{
			$this->remove( "GRI.MatchMgrDebug");
		}

		if ( !$bFound_Eyelock_SystemReadyDebug ) {
			$this->remove( "Eyelock.SystemReadyDebug");
		}

		if ( !$bFound_Eyelock_Debug ) {
			$this->remove( "Eyelock.Debug");
		}

		if ( !$bFound_Eyelock_MatchDispDebug ) {
			$this->remove( "Eyelock.MatchDispDebug");
		}

		if ( !$bFound_Eyelock_OpenSSL_Debug ) {
			$this->remove( "Eyelock.OpenSSL.Debug");
		}

		if ( !$bFound_Eyelock_SSLDebug ) {
			//error_log("Got here with " . $bFound_Eyelock_SSLDebug . " when it shouldn't?");
			$this->remove( "Eyelock.SSLDebug");
		}

		if ( !$bFound_GRI_DBDebug ) {
			$this->remove( "GRI.DBDebug");
		}

		if ( !$bFound_GRI_EyeDispatcherDebug ) {
			$this->remove( "GRI.EyeDispatcherDebug");
		}

		if ( !$bFound_GRI_HBDebug ) {
			$this->remove( "GRI.HBDebug");
			setSlaveConfigValue("7", "false");
		}

		if ( !$bFound_GRI_HDDebug ) {
			$this->remove( "GRI.HDDebug");
			setSlaveConfigValue("8", "false");
		}

		if ( !$bFound_GRI_LEDDebug ) {
			$this->remove( "GRI.LEDDebug");
		}

		if ( !$bFound_GRI_MPDebug ) {
			$this->remove( "GRI.MPDebug");
		}
		
		if ( !$bFound_GRI_NwDebug ) {
			$this->remove( "GRI.NwDebug");
		}

		if ( !$bFound_GRI_NwMmDebug ) {
			$this->remove( "GRI.NwMmDebug");
		}

		if ( !$bFound_GRI_SpoofDebug ) {
			$this->remove( "GRI.SpoofDebug");
		}

		if ( !$bFound_GRITrigger_CmxDebug ) {
			$this->remove( "GRITrigger.CmxDebug");
		}

		if ( !$bFound_GRITrigger_F2FDebug ) {
			$this->remove( "GRITrigger.F2FDebug");
		}

		if ( !$bFound_GRITrigger_WeigandDebug ) {
			$this->remove( "GRITrigger.WeigandDebug");
		}

		if ( !$bFound_MT9P001_Debug ) {
			$this->remove( "MT9P001.Debug");
			setSlaveConfigValue("17", "false");
		}

		if ( !$bFound_NwListener_Debug ) {
			$this->remove( "NwListener.Debug");
		}
		
        if (!$bFoundHDMatcherEnabled)
        {
            // Need to set the matcherID to the REMOTE entry...
            $this->GRI_HDMatcherID = sprintf("%d", $this->MatcherIDLocal);
            $this->set("GRI.HDMatcherID", $this->GRI_HDMatcherID);
            
            //Remove PCMATCHER fields from ini and decrease HDMatcherCount by 1
            if($this->MatcherIndex > -1){
            	//remove all fields related to PCMATCHER
            	$this->removeKeyFromINI(sprintf('GRI.HDMatcher.%d',$this->MatcherIndex));
            	if($this->MatcherCount > 0){
            		$this->MatcherCount = $this->MatcherCount - 1;
            		$this->set("GRI.HDMatcherCount", $this->MatcherCount);
            	}
            } 
        }


        if ($bFound_HttpPostSenderDest || $bFound_HttpPostSenderDestPostIris ||
            $bFound_HttpPostSenderDestSignalError || $bFound_HttpPostSenderDestSignalHeartBeat ||
            $bFound_HttpPostSenderDestSignalMaintenance)
        {
   			$this->set("Eyelock.SPAWAREnable", "true");
        }
        else
        {
   			$this->set("Eyelock.SPAWAREnable", "false");
        }

/*
        if (!$bFoundEnableIEEE8021X)
   			$this->set("Eyelock.EnableIEEE8021X", "false");
*/
        if (!$bFoundEnableNegativeMatchTimeout)
			$this->set("Eyelock.EnableNegativeMatchTimeout", "0");

        if (!$bFoundRelayEnable)
			$this->set("GRITrigger.EnableRelayWithSignal", "false");

        if (!$bFoundEnablePopupHelp)
			$this->set("Eyelock.EnablePopupHelp", "false");

        if (!$bFoundAllowSiteAdminUpgrade)
			$this->set("Eyelock.AllowSiteAdminUpgrade", "false");
        //if(!$bFoundOSDPEnabled)
        //     $this->set("GRITrigger.OSDPEnabled", "false");


        //if(!$bFoundOSDPInputEnabled)
        //     $this->set("GRITrigger.OSDPInputEnable", "false");
             
        if (!$bFoundDispatchSecure)
            $this->set("GRI.NwDispatcherSecure", "unsecure");
		
		//if (!$bFoundNwMatcherCommSecure)
			$this->set("Eyelock.NwMatcherCommSecure", "secure");
		
        // Always set our hidden values...
        $this->set("Eyelock.MaxTemplateCount", $this->Eyelock_MaxTemplateCount);
        $this->set("Eyelock.SoftwareUpdateURL", $this->Eyelock_SoftwareUpdateURL);
        $this->set("Eyelock.SoftwareUpdateDateCheck", $this->Eyelock_SoftwareUpdateDateCheck);
        $this->set("Eyelock.SoftwareUpdateDateNano", $this->Eyelock_SoftwareUpdateDateNano);
        $this->set("Eyelock.SoftwareUpdateDateBob", $this->Eyelock_SoftwareUpdateDateBob);
	}


	// Gets the value stored in the specified key
	function get($parameter, &$value)
	{			
		foreach ($this->m_aParameters as $section => $parameters)
		{
			if (isset($parameters[$parameter]))
            {
				$value = $parameters[$parameter]['value'];
                return TRUE;
            }
		}

		return false;
	}
	///DO NOT CALL THIS FROM CODE WHICH USES DATA FROM THE BROWSER, only call internally, otherwise this exposes a security vulnerability
	function remove($parameter)
	{			//error_log("try remove key ".$parameter);
		foreach ($this->m_aParameters as $section => $parameters)
		{
			if (isset($parameters[$parameter]))
            {
				//unset($parameters[$parameter]);
				$this->set($parameter, "XXDELETEMEXX");
                return TRUE;
            }
		}

		return false;
	}
	///TLS only when isset is true, Everything when it is not.
   function setTLSOnlyWebConfig($isset)
    {
      
        $lighttpdconf = fopen("/home/www/lighttpd.conf", "r");
        $lighttpdconfout = fopen("/home/www/lighttpd.conf.tmp", "w");
        //error_log("set is " .$isset);
        $haslines = 0;
        while(!feof($lighttpdconf))
        {
            $line = fgets($lighttpdconf);
			
      //    error_log($line);
            {
			
                if(strpos($line, "ssl.use-sslv2") !== false)
                {
					
                 $haslines = 1;
                  if($isset == 1)
                    //continue; //don't write the line 
					  $line = "ssl.use-sslv2 = \"disable\"\n";
					else
                    //continue; //don't write the line
					  $line = "#ssl.use-sslv2 = \"disable\"\n";
                }
                if(strpos($line, "ssl.use-sslv3") !== false) 
                 {
					 
                 $haslines = 1;
                  if($isset == 1)
                    //continue; //don't write the line
					  $line = "ssl.use-sslv3 = \"disable\"\n";
					else
                    //continue; //don't write the line
					  $line = "#ssl.use-sslv3 = \"disable\"\n";
                }
				if(strpos($line, "ssl.cipher-list = \"TLSv1.2:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS\"") !== false )
				{
					if($isset)	
						$line = "ssl.cipher-list = \"TLSv1.2:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS\"\n";
					else
						$line = "#ssl.cipher-list = \"TLSv1.2:!aNULL:!eNULL:!EXPORT:!DES:!RC4:!MD5:!PSK:!aECDH:!EDH-DSS\"\n";
				}
            }
            fputs($lighttpdconfout, $line);
        }
         //error_log("set ".$isset." haslines".$haslines );
        if($isset == 1 && $haslines == 0)
            {
              //  fwrite($lighttpdconfout, "ssl.use-sslv2 = \"disable\"\n");
              //  fwrite($lighttpdconfout, "ssl.use-sslv3 = \"disable\"\n");
               
               
            }
            fclose($lighttpdconf);
            fclose($lighttpdconfout);

               //error_log("saveconfig");
                shell_exec("rm /home/www/lighttpd.conf");
            shell_exec("mv /home/www/lighttpd.conf.tmp /home/www/lighttpd.conf");
            //error_log("done");
    }
	// sets the value for the specified key into the internal array	
	function set($key, $value, $text="")
	{	
        foreach ($this->m_aParameters as $section => $parameters)
		{
			if (isset($parameters[$key]))
			{
				$this->m_aParameters[$section][$key]['value'] = $value;
                $this->AddToSaveList($key);

				if (strlen($text) > 0)
    				$this->m_aParameters[$section][$key]['text'] = $text;
			}
		}
	}


    // Called during upgrade to bring all loaded settings forward...
    function set_all()
    {
        foreach ($this->m_aParameters as $section => $parameters)
		{
            foreach ($parameters as $key => $value)
                $this->set($key, $value['value']);
        }
    }
	


	function add($parameter, $value, $section1="Dummy", $text="")
	{
		foreach ($this->m_aParameters as $section => $parameters)
		{
            // If already set, it's not an add, it's an update...
			if (isset($parameters[$parameter]))
			{
		//		FB::warn("add() detected value already exists in array!  Not added!");
				return false;
			}
		}

		$this->m_aParameters[$section1][$parameter]['value'] = $value;
		$this->m_aParameters[$section1][$parameter]['text'] = $text;
		
		return true;
	}
	
	function RemoveKeyFromINI($key)
	{
        // DMOTODO - this may be unsecure... need to escapecommandarg $key
		shell_exec("cd /home/root; sed -i '/^".$key."/ d' Eyelock.ini");
	}

	// Save the new values out to the disk file.
	// We only want to update the values that were editable... not the entire file
	// We want to keep the rest of the file intact.	
	function SaveIniValues()
	{
        // First make a backup copy of the file...
		$BackupFile = substr_replace($this->m_sIni_file, 'bak', strrpos($this->m_sIni_file, '.') +1);

		copy($this->m_sIni_file, $BackupFile);

		$bfh = fopen($BackupFile, "r");
		$fh = fopen ($this->m_sIni_file, "w");
		$key="";
		$text="";

		// Logic here (to maintain all formatting) is to simply read each line, then write it out unless
		// it is one of the edited values.  If it's an edited value then we do some processing and write out
		// the edited value instead...
		while (!feof ($bfh))
		{
			$line = fgets($bfh);
			
			if ((strpos(" ". $line,";") == 1) || (strpos(" ". $line,"#") == 1))
			{
				fwrite($fh, $line);	// commentline ; only valid in first pos of line.
			}elseif (strpos(" ". $line,"[") == 1)
			{
				fwrite($fh, $line);	// section
			}
			elseif (strpos($line,"=") > 0 )
			{
				list($key, $value) = explode ("=", $line);
				$text = trim($text);
				$key = trim($key);
				$value=trim($value);
				$pos = strpos($line,"XXDELETEMEXX");
				//error_log($line . " :deletemexx? ".$pos);
				if($pos !== false)
				{
			//		error_log("not writing line to ini file");
					continue; //skip this line
				}
				// Now we should have a key value pair...
				// Here we do our custom processing...
				fwrite($fh, $this->ProcessEditedValue($line, $key));
			}
			else
				fwrite($fh, $line); // Any other lines.
		}

        // Finally, add any 'new' values to the bottom on the ini file...
        // These are values that are new in the UI but never existed in the ini file before...
        $this->SaveAllNewValues($fh);

		fclose($bfh);
		fclose($fh);
	}


    // For each value that has changed, we add an element to our array.
    // The array is then parsed in the save function to do 2 things:
    //      1:  Actually replace the line of text in the ini file
    //      2:  Execute any additional functionality that is necessary on a value change
    function AddToSaveList($key)
    {
        // Already set?  Bail...
		if (isset($this->m_aSaveParameters[$key]))
        {
			return false;
        }

		$this->m_aSaveParameters[$key]['processor'] = NULL;
    }


	// Replace line with new values
	function ProcessEditedValue($line, $key)
	{
        // Is this a value we need to save?
        if (isset($this->m_aSaveParameters[$key]))
        {
		    // Loop through our internally saved values and find our key... 
            // then write out the updated values...
		    foreach($this->m_aParameters as $section => $aValues)
		    {
                // We only allow editing of a handful of values...
			    // So only replace the lines that we process...
			    foreach($aValues as $updatedkey => $updatedvalue)
			    {
				    // If we found it in our internal array
                    // So we can write out the value...
				    if ($key === $updatedkey)
				    {
                        // If we need to do some special processing on this entry, do it now...
                        if (isset($this->m_aSaveParameters[$key]['processor']))
                            $this->m_aSaveParameters[$key]['processor']();
						if($updatedvalue['value'] === "XXDELETEMEXX")
							$line = "";
						else
							$line = "{$key}={$updatedvalue['value']}\n"; // Build the updated entry and return it...

                        // Now remove this key from our saved array... any items left over in the saved array are brand New items that will be saved
                        // out separately...
                        unset($this->m_aSaveParameters[$key]);

    					return $line;
				    }
                }
            }
		}

		return $line;
	}


    function SaveAllNewValues($fh)
    {
        // then write out the updated values...
		foreach($this->m_aSaveParameters as $key => $value)
		{
           if (isset($key))
           {
               // Find the setting and its value in our internal parameters array
		        foreach($this->m_aParameters as $section => $aValues)
		        {
                    // We only allow editing of a handful of values...
			        // So only replace the lines that we process...
			        foreach($aValues as $updatedkey => $updatedvalue)
			        {
				        // If we found it in our internal array
                        // So we can write out the value...
				        if ($key === $updatedkey)
				        {
							//error_log("add new ". $key . " value ". $updatedvalue['value']);
							if($updatedvalue['value'] === "XXDELETEMEXX")
								continue;
                            $line = "{$key}={$updatedvalue['value']}\n"; // Build the updated entry and return it...
                            fwrite($fh, $line);
                        }
                    }
                }
            }
        }
    }
} // Class
?>
