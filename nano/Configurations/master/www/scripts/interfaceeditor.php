<?php

require("checklogin.php"); // Make sure user is logged on to get to this page...
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");
// Class handles reading/updating interface specifics of the nano...
class InterfaceEditor
{
	var $m_sIni_file;               // name of the ini-file, 

    // Arrays to hold section|key|values and comments...
	var $m_aParameters = array();   // multiple array.  var[sections][parameter][value or text]
	var $m_aComments = array();     // array with comments, index is section or _start for the header-comments

    ////////////////////////////////////////////////////
    // Interface values of interest
    ////////////////////////////////////////////////////
    public $DeviceName;
    public $DHCPTimeout = "10";           //restoreinterfaces.sh
    public $DHCPRetries = "0";            //restoreinterfaces.sh
    public $DHCPRetryDelay = "10"; //restoreinterfaces.sh
    public $IpOfBoard;
    public $Network;
    public $SubNetMask;
    public $BroadcastIp;
    public $Gateway;
    public $MacAddress;
    public $dns1;
    public $dns2;
    public $IsStatic;
    public $IsMaster;
    public $FactorySerialNumber;
    public $SerialNumber;
    public $Id;
    public $RootFolder;
    public $InterfacesFile;
    public $ReloadInterfacesFile;
    public $Tampered;
	
	private $unameWithRoFs = "3.0.0-BSP-dm37x-2.4-2Eyelock_NXT_6.0";
	private $uname = "";
	private $resolvFile = "/etc/resolv.conf";
	private $etcNetworkPath = "/etc/network/";
	
  
	
    // Constructor...
	function __construct ()
	{
        // SSH commands to get information

        // Then parse results to populate variables...

		$key="_none";
		$text="";

        $this->SerialNumber = "(unknown)";
        $this->Id = "(unknown)";

        $this->RootFolder = "/home/root";
        $this->InterfacesFile = "interfaces";
        $this->ReloadInterfacesFile = "reloadinterfaces.sh";
		
		$this->uname = php_uname("r");
		$this->resolvFile = ($this->uname === $this->unameWithRoFs) ? "/tmp/etc/resolv.conf" : "/etc/resolv.conf";
		$this->etcNetworkPath = ($this->uname === $this->unameWithRoFs) ? "/tmp/etc/network/" : "/etc/network/";

	 $this->LoadDNSSettings();
	}

    function LoadDNSSettings()
    {
	$this->dns1 = '8.8.8.8';
	$this->dns2 = '8.8.4.4';
	if(file_exists($this->resolvFile)){
		$dnsFile = file_get_contents($this->resolvFile);
		$regex_pattern = "/nameserver\s+((?:[0-9]{1,3}\.){3}[0-9]{1,3})/";
		preg_match_all($regex_pattern,$dnsFile,$matches);
		if(isset($matches[1][0]))
			$this->dns1=$matches[1][0];
		if(isset($matches[1][1]))
			$this->dns2=$matches[1][1];
	}
    }
    
    function SaveDNSSettings($newDns1, $newDns2)
    {
	
	$current = "nameserver ".$newDns1."\n";
	if(isset($newDns2))
		$current .= "nameserver ".$newDns2."\n";
	file_put_contents($this->resolvFile, $current);
    }

    function LoadInterfaceSettings($bIsHBOX)
    {
        //Ok, retrieve the Mac Address of the device...
        $cmdString = "echo $(ifconfig enp10s0 |";
        $cmdString .= "grep \"HWaddr\" |";
        $cmdString .= "tr -s ' ' |";
        $cmdString .= "cut -d ' ' -f5 |";
        $cmdString .= "tr '[A-Z]' '[a-z]' |";
        $cmdString .= "tail -c 18)";

        /////////////////////////////////////////////////
        // Get the easy ones...
        $this->MacAddress = strtoupper(trim(shell_exec($cmdString)));

        /////////////////////////////////////////////////
        // Get the Factory Serial Number
		$hostname="";
		if ($this->uname !== $this->unameWithRoFs)
		{
			// Backup the original hostname (we use it during 'factory reset')
			$strResult = shell_exec("cd /etc;ls -la | grep FactoryHostname");

			// If it's not there, copy the hostname file...
			if (strlen(trim($strResult)) <= 0)
			   NXTW_shell_exec("3"); //"cp /etc/hostname /etc/FactoryHostname"

			// Get current hostname
			$hostname = shell_exec("cat /etc/hostname");
		}
		else
		{
			// /etc/FactoryHostname in read-only FS is assumed to be created on factory
			$hostname = shell_exec("cat /tmp/etc/hostname");
		}
       		
        $cmdFactory = str_replace("\n", "|", $hostname);
        $arFactory = explode("|", $cmdFactory);

        $this->FactorySerialNumber = $arFactory[0];

        ////////////////////////////////////////////////
        // Compose the device name...
        $arSerial = explode("-", $this->FactorySerialNumber);
        //return string.Format("{0}-{1}.local", factorSerialExp[0], (IsMatcher) ? 1 : 0); ; 
        $this->DeviceName = sprintf("%s", $arSerial[0]);// ????, $this->Id);

        ////////////////////////////////////////////////
        // Extract Serial Number from avahi interface...
        $avahiFile = ($this->uname === $this->unameWithRoFs) ? "/tmp/etc/avahi/services/udip.service" : "/etc/avahi/services/udip.service";
		$strAvahi = shell_exec("cat ".$avahiFile." | grep EYELOCK");

        $strAvahi = trim(str_replace("<name replace-wildcards=\"yes\">", "", $strAvahi));
        $strAvahi = trim(str_replace("</name>", "", $strAvahi));

        $arAvahi = explode("_", $strAvahi);

/*
        // sanity checks
        if (l_arrstr[0] != "EYELOCK")
        {
            throw new Exception("wrong company name");
        }

        if (l_arrstr[1] != "NANO")
        {
            throw new Exception("wrong device type");
        }

        if (l_arrstr[4] != l_HwAddress)
        {
            throw new Exception("something fishy with the macaddress from avahi string");
        }
*/
        if (isset($arAvahi[2]))
            $this->IsMaster = ($arAvahi[2] == "M");

        if (isset($arAvahi[3]))
            $this->SerialNumber = $arAvahi[3];


/*       ///////////////////////////////////////////////
        // Now extract IP information
        $cmdIfConfig = shell_exec("ifconfig");

        $cmdIfConfig = str_replace("\n", "|", $cmdIfConfig);
        $arIfConfig = explode("|", $cmdIfConfig);

        $IPString = null;
//        $matchstr = sprintf("enp10s0      Link encap:Ethernet  HWaddr %s", strtoupper($this->MacAddress));
        $matchstr = sprintf("enp10s0");


        //////////////////////////////////////////////
        // Get the IPs of the board
        foreach ($arIfConfig as $key=>$theString)
        {
            if (isset($theString))
            {
                $theString = trim($theString);

                $pos = strpos($theString, $matchstr);

                if ($pos !== false)
                {
                    $IPString = trim($arIfConfig[$key+1]); // IP string should be the next string
                    break;
                }
            }
        }

        if (!isset($IPString)) // This could be the case for the ip on the avahi broadcast
        {
            $matchstr = sprintf("enp10s0:avahi Link encap:Ethernet  HWaddr %s", strtoupper($this->MacAddress));

            foreach ($arIfConfig as $key=>$theString)
            {
                if (isset($theString))
                {
                    $theString = trim($theString);

                    $pos = strpos($theString, $matchstr);

                    if ($pos !== false)
                    {
                        $IPString = trim($arIfConfig[$key+1]); // IP string should be the next string
                        break;
                    }
                }
            }

        }
*/

	// Now extract IP information
	$IPString = shell_exec("ifconfig enp10s0| grep 'inet addr:'");
  	if (!isset($IPString)) 
	     $IPString = shell_exec("ifconfig enp10s0:avahi| grep 'inet addr:'"); 

        if (isset($IPString))
        {
         //   echo "{$IPString}</br>";

            // Line after identified the enp10s0 line...
            $this->IpOfBoard = trim(substr($IPString, strpos($IPString, "addr:")+5, (strpos($IPString, "Bcast:") - strpos($IPString, "addr:")-5)));
            $this->BroadcastIp = trim(substr($IPString, strpos($IPString, "Bcast:")+6, (strpos($IPString, "Mask:") - strpos($IPString, "Bcast:")-6)));
            $this->SubNetMask = trim(substr($IPString, strpos($IPString, "Mask:")+5, (strlen($IPString) - strpos($IPString, "Mask:")-5)));
        }
        else
        {
            $this->IpOfBoard = "Unknown";
            $this->BroadcastIp = "Unknown";
            $this->SubNetMask = "Unknown";
            //echo "Failed to parse IP addresses</br>";
            //DMOTODO - could not parse IP addresses!
        }


        ///////////////////////////////////////////////
        // Get the Gateway from netstat...
        $cmdNetStat = trim(shell_exec("netstat -nr"));
        $cmdNetStat = str_replace("\n", "|", $cmdNetStat);
        $arNetStat = explode("|", $cmdNetStat);

        // Ok, now remove useless strings...
        foreach ($arNetStat as $key => $theString)
        {
            if (strlen($theString) === 0)
                unset($arNetStat[$key]); // clear it...
            else
            {
                $pos = strpos($theString, "Kernel");
                if ($pos !== false)
                {
                    if ($pos == 0)
                        unset($arNetStat[$key]); // clear it...
                }
                else
                {
                    $pos = strpos($theString, "Destination");
                    if ($pos !== false)
                    {
                        if ($pos == 0)
                            unset($arNetStat[$key]); // clear it...
                    }
                }
            }
        }

        foreach ($arNetStat as $key=>$theString)
        {
            if (isset($theString))
            {
                $sDestination = trim(substr($theString, 0, 16));
                $sGateway = trim(substr($theString, 16, 15));
                $sFlags = trim(substr($theString, 48, 7));
                $sIface = trim(substr($theString, 73, strlen($theString) - 73));

                if (($sFlags === "UG") && ($sIface === "enp10s0"))
                {
                    $this->Gateway = $sGateway;
                    // DMOTODO - enuser this is a valid IP!!!
//                    if(!filter_var($this->Gateway, FILTER_VALIDATE_IP))
//                    {
//                        echo "Not a valid IP address!";
//                    }
                }

                if (($sFlags === "U") && ($sIface === "enp10s0"))
                {
                    $this->Network = $sDestination;
                    // DMOTODO - enuser this is a valid IP!!!
 //                   if(!filter_var($this->Network, FILTER_VALIDATE_IP))
 //                   {
 //                       echo "Not a valid IP address!";
 //                   }
                }
            }
        }

        // This flag determines dhcp status
        $this->IsStatic = $this->IsIPStatic($this->IpOfBoard);

        // Load up the DHCP Settings
        $this->LoadReloadInterfacesSettings($bIsHBOX);

        // Determine Tamper flag
        $tampered = shell_exec("ls -la /home/root | grep tamper");

        // If file exists, we're done...
        if (strlen(trim($tampered)) > 0)
            $this->Tampered = TRUE;
        else
            $this->Tampered = FALSE;
    }


    function SaveInterfaceSettings($row, $bForceUpdate, $bIsHBOX)
    {
        // These 2 calls are smart enough to only update what is necessary.
        $bWasStaticIP = $this->IsStatic;
   
        // Now update/create our changed interfaces file, but only if something has changed
        if (!$this->UpdateInterfacesFile($row, (!$bWasStaticIP && ($row['deviceipmode'] === "staticip")), $bForceUpdate, $bIsHBOX)) // Internally checks for changes and only updates as necessary... so we always call it...
            return FALSE;
	//		WriteToTempFile("interfaces write done\n");
	//	error_log("update interfaces done");
        // This function only updates what is necessary...
        // If we are switching from Static, to DHCP then we need to delete some files...
        $this->ConfigureAsDHCP($row, ($bWasStaticIP && ($row['deviceipmode'] !== "staticip")), $bForceUpdate, $bIsHBOX);
	//	WriteToTempFile("configure as php done\n");

        return TRUE;
    }

    //DMOHBOX New if HBOX, this is a no op .... just return
    function LoadReloadInterfacesSettings($bIsHBOX)
    {
        if (!$bIsHBOX)
        {
            // Parse reloadinterfaces.sh for dhcp settings
            $strUdhcpc = shell_exec(sprintf("grep udhcpc %s", escapeshellarg($this->RootFolder.'/'.$this->ReloadInterfacesFile)));
            $arUdhcpc = explode("-", $strUdhcpc);

            // Assume 0 for this... if it is not found in the string, we don't write it out when saving...
            $this->DHCPRetries = "0";

            foreach ($arUdhcpc as $key => $theString)
            {
                if (strpos(trim($theString), 'T') === 0) // timeout
                {
                    list($key, $timeout) = explode(" ", $theString);
                    if (isset($timeout))
                        $this->DHCPTimeout = trim($timeout);
                    else
                        $this->DHCPTimeout = "10"; // default to 10 seconds...
                }
                else if (strpos(trim($theString), 't') === 0) // retries
                {
                    list($key, $retries) = explode(" ", $theString);
                    if (isset($retries))
                        $this->DHCPRetries = trim($retries);
                    else
                        $this->DHCPRetries = "0"; // default to 3 retries
                }
                else if (strpos(trim($theString), 'A') === 0) // retry delay
                {
                    list($key, $retrydelay) = explode(" ", $theString);
                    if (isset($retrydelay))
                        $this->DHCPRetryDelay = trim($retrydelay);
                    else
                        $this->DHCPRetryDelay = "10"; // default to 10 seconds...
                }
            }
        }
    }

    //DMOHBOX New if HBOX, this is a no op .... just return
    function SaveReloadInterfacesSettings($row, $bForceUpdate, $bIsHBOX)
    {
        // Update reloadinterfaces.sh with dhcp settings
        // Build up our new string using the supplied values...
        $bChanged = FALSE;  // Only update if things have changed...
        $strUdhcpc = "";
        $strDHCPTimeout = "";
        $strDHCPRetries = "0";
        $strDHCPRetryDelay = "";

        if (!$bIsHBOX)
        {
            if (!$bForceUpdate)
            {
		        foreach ($row as $key => $value) 
		        {
                    if ($key === "DHCP_Timeout")
                    {
                        $strDHCPTimeout = $value;

                        if ($this->DHCPTimeout !== $value)
                            $bChanged = TRUE;
                    }

                    if ($key === "DHCP_Retries")
                    {
                        $strDHCPRetries = $value;

                        if ($this->DHCPRetries !== $value)
                            $bChanged = TRUE;
                    }

                    if ($key === "DHCP_RetryDelay")
                    {
                        $strDHCPRetryDelay = $value;

                        if ($this->DHCPRetryDelay !== $value)
                            $bChanged = TRUE;
                    }
                }
            }
            else
                $bChanged = TRUE;

            if ($bChanged)
            {
                if ($strDHCPRetries === "0")
                    $strDHCPRetries = "";
                else
                    $strDHCPRetries = sprintf(" -t %s", $strDHCPRetries);

                $strUdhcpc = sprintf("\t\tudhcpc -i enp10s0 -b -T %s%s -A %s -S\n", $strDHCPTimeout, $strDHCPRetries, $strDHCPRetryDelay);


                // Ok, that's our string...  Now write it back out to the correct spot in the file... sed could be used...
                // but we're just going to write the file out manually...
                // We do this by reading from the backup, and writing one line at a time, replacing the single
                // line that we are updating...
                // First make a backup copy of the file...
                $ReloadFullPath = sprintf("%s/%s", $this->RootFolder, $this->ReloadInterfacesFile);
		        $BackupFile = substr_replace($ReloadFullPath, 'bak', strrpos($ReloadFullPath, '.') +1);
		        copy($ReloadFullPath, $BackupFile);

                // Now we write the new updated file out...
		        $bfh = fopen($BackupFile, "r");
		        $fh = fopen ($ReloadFullPath, "w");

		        // Logic here (to maintain all formatting) is to simply read each line, then write it out unless
		        // it is one of the edited values.  If it's an edited value then we do some processing and write out
		        // the edited value instead...
		        while (!feof ($bfh))
		        {
			        $line = fgets($bfh);

                    // Replace only this line...
                    if (strpos($line, "udhcpc") !== FALSE)
                        fwrite($fh, $strUdhcpc);
                    else
				        fwrite($fh, $line);
		        }

		        fclose($bfh);
		        fclose($fh);
            }
        }
    }


    /////////////////////////////////////////////////
    // Helper functions
    /////////////////////////////////////////////////
    private function IsIPStatic($boardIP)
    {
        $bIsStatic = false;
	
	// Uname is not fixed on Ubuntu
	// TODO: properly identify device type
	//if ($this->uname == "4.13.0-36-generic")
	{
		$strInterfaces = shell_exec('grep enp10s0 /etc/network/interfaces | grep static'); 
          	if(strlen($strInterfaces) == 0)
		{
			$bIsStatic = false;
		}
		else
		{
			$bIsStatic = true;
		}
	}
	/*else if ($this->uname === $this->unameWithRoFs) 
	{
		$bIsStatic = file_exists("/tmp/etc/rc.conf");
	}
	else
	{
		$strInterfaces = shell_exec('grep IPADDR0 /etc/rc.d/rc.conf | grep dhcp'); 
          	if(strlen($strInterfaces) == 0)
		$bIsStatic = true;
	}*/
        return $bIsStatic;
    }


    private function GetRemoteFolderFileList($strIP, $strRemoteFolder)
    {
        $cmdls = sprintf("cd %s; ls", escapeshellarg($strRemoteFolder));

        // Now extract IP information
        $ls = shell_exec($cmdls);

        $cmdls = str_replace("\n", "|", $ls);
        $arFiles = explode("|", $cmdls);


        return $arFiles;
    }



    function UpdateInterfacesFile($row, $bSwitchedToStatic, $bForceUpdate)
    {
	//	error_log("updateInterfacesFile...");
        $bChanged = $bSwitchedToStatic;
        $bDeviceNameChanged = FALSE;
	 $NewDeviceName = "";
        $NewIpOfBoard = "";
        $NewNetwork = "";
        $NewSubNetMask = "";
        $NewBroadcastIp = "";
        $NewGateway = "";
        $NewMacAddress = "";
        $NewFactorySerialNumber = "";
        $NewSerialNumber = "";


        // Using our current settings, create our backup string, in case we need to make the backup files...
        $strCurrentInterfaces = "auto lo enp10s0\n";
        $strCurrentInterfaces .= "iface lo inet loopback\n\n";
        $strCurrentInterfaces .= "iface enp10s0 inet static\n";
        $strCurrentInterfaces .= sprintf("address %s\n", $this->IpOfBoard);
        $strCurrentInterfaces .= sprintf("network %s\n", $this->Network);
        $strCurrentInterfaces .= sprintf("netmask %s\n", $this->SubNetMask);
        $strCurrentInterfaces .= sprintf("broadcast %s\n", $this->BroadcastIp);
        $strCurrentInterfaces .= sprintf("gateway %s\n", $this->Gateway);
        $strCurrentInterfaces .= sprintf("hwaddress ether %s\n", $this->MacAddress);

        // Ok, that takes care of all the backup issues... Now, let's write the new values...
        // First, we update our internal values, then write out the new interfaces file...
		foreach ($row as $key => $value) 
		{
           
            if ($key === "DeviceName")
            {
                $NewDeviceName = $value;

                if ($value !== $this->DeviceName)
                {
                    $bDeviceNameChanged = TRUE;
                    $this->DeviceName = $value;
                }
            }
            if ($key === "IpOfBoard")
            {
                $NewIpOfBoard = $value;

                if ($value !== $this->IpOfBoard)
                {
                    $bChanged = TRUE;
                    $this->IpOfBoard = $value;

                    // If the IP has changed, we need to make sure the IP is available...
                    // Stop on the first successful packet, or wait up to 2 seconds... before timing out...
                    $strCmd = sprintf("8".chr(0x1F)."%s", escapeshellarg($this->IpOfBoard));//"ping -w 2 -c 1 %s", $this->IpOfBoard);
                    $strCmdResult = NXTW_shell_exec($strCmd);

                    $arCmdResult = explode(",", $strCmdResult);

                    if (isset($arCmdResult[1]))
                    {
                        $arReceived = explode(" ", trim($arCmdResult[1]));

                        if (isset($arReceived[0]))
                        {
                            $strCount = trim($arReceived[0]);

                            if ((int)$strCount >= 1)
                            {
                                return FALSE;
                            }
                        }
                    }
                }
            }
            if ($key === "Network")
            {
                $NewNetwork = $value;

                if ($value !== $this->Network)
                {
                    $bChanged = TRUE;
                    $this->Network = $value;
                }
            }
            if ($key === "SubNetMask")
            {
                $NewSubNetMask = $value;

                if ($value !== $this->SubNetMask)
                {
                    $bChanged = TRUE;
                    $this->SubNetMask = $value;
                }
            }
            if ($key === "BroadcastNetwork")
            {
                $NewBroadcastIp = $value;

                if ($value !== $this->BroadcastIp)
                {
                    $bChanged = TRUE;
                    $this->BroadcastIp = $value;
                }
            }
            if ($key === "Gateway")
            {
                $NewGateway = $value;

                if ($value !== $this->Gateway)
                {
                    $bChanged = TRUE;
                    $this->Gateway = $value;
                }
            }
            if ($key === "MacAddress")
            {
                $NewMacAddress = $value;

                if ($value !== $this->MacAddress)
                {
                    $bChanged = TRUE;
                    $this->MacAddress = $value;
                }
            }
	     if ($key === "DNS_1" && $value !== $this->dns1)
	     {
		  $bChanged = TRUE;
                $this->dns1 = $value;
	     }
	     if ($key === "DNS_2" && $value !== $this->dns2)
	     {
                $bChanged = TRUE;
                $this->dns2 = $value;
	     }
        }


        // We don't do any further processing, unless the user has actually changed a value...
        // DMOTODO add code to change hostname...
        if ($bDeviceNameChanged)
        {
		//	error_log("try changehost name");
            // If the device name (ie. hostname) has changed... write out the hostname file...
//            $strcmdResult = sprintf("echo '%s' > '%s'", $this->DeviceName, "/etc/hostname");
			$strcmdResult = NXTW_shell_exec(sprintf("4".chr(0x1F)."%s", escapeshellarg($this->DeviceName))); // nxtW will identify read-only FS and correct hostname file
        }
	
	 if ($bChanged)
        {
            //DMOTODO
            // PING new IP and make sure it's not in use... if it is in use, fail with result...

            // Now make a backup, etc...
            // If backup exists, copy backup into 'interfaces' file (restore it...)
        //      $strcmdResult = shell_exec(sprintf("[ -e /etc/network/%s-bkup ] && cp /etc/network/%s-bkup /etc/network/%s", $this->InterfacesFile, $this->InterfacesFile, $this->InterfacesFile));
            // if intefaces exists, copy interfaces into 'bkup' or ??
            $strcmdResult = NXTW_shell_exec(sprintf("5".chr(0x1F)."%s".chr(0x1F)."%s", escapeshellarg($this->etcNetworkPath.$this->InterfacesFile), escapeshellarg($this->etcNetworkPath.$this->InterfacesFile.'-bkup')));//[ -e /etc/network/%s ] && cp /etc/network/%s /etc/network/%s-bkup || echo > /etc/network/%s", $this->InterfacesFile, $this->InterfacesFile, $this->InterfacesFile, $this->InterfacesFile));
            // write interfaces.txt file on the board
            $strcmdResult = NXTW_shell_exec(sprintf("6".chr(0x1F)."%s".chr(0x1F)."%s", escapeshellarg($this->etcNetworkPath.$this->InterfacesFile.'-restore'), escapeshellarg($this->etcNetworkPath.$this->InterfacesFile.'-new')));//echo > /etc/network/%s-restore;echo > /etc/network/%s-new

			$f = fopen("/home/www/iftemp","w");
			fwrite($f, $strCurrentInterfaces);
			fclose($f);
			
            $strCmd = sprintf(sprintf("7".chr(0x1F)."%s".chr(0x1F)."%s", "/home/www/iftemp", escapeshellarg($this->InterfacesFile.'-restore')));//"echo -e %s >> /etc/network/%s-restore", $strCurrentInterfaces, $this->InterfacesFile));

            $strcmdResult = NXTW_shell_exec($strCmd);


            // Ok, all backups are complete... Update our interfaces file with the new values...
            $strNewInterfaces = "auto lo enp10s0\n";
            $strNewInterfaces .= "iface lo inet loopback\n\n";
            $strNewInterfaces .= "iface enp10s0 inet static\n";
            $strNewInterfaces .= sprintf("address %s\n", $this->IpOfBoard);
            $strNewInterfaces .= sprintf("network %s\n", $this->Network);
            $strNewInterfaces .= sprintf("netmask %s\n", $this->SubNetMask);
            $strNewInterfaces .= sprintf("broadcast %s\n", $this->BroadcastIp);
            $strNewInterfaces .= sprintf("gateway %s\n", $this->Gateway);
            $strNewInterfaces .= sprintf("hwaddress ether %s\n", $this->MacAddress);
			$f = fopen("/home/www/iftemp","w");
			fwrite($f, $strNewInterfaces);
			fclose($f);
	//		error_log("new interfaces file is > ".$strNewInterfaces);
            // Write the new interfaces file...
            // Create the 'new' interfaces file...
            $strcmdResult = NXTW_shell_exec(sprintf("9".chr(0x1F)."%s".chr(0x1F)."%s","/home/www/iftemp", escapeshellarg($this->InterfacesFile.'-new')));//"echo -e %s >> /etc/network/%s-new", $strNewInterfaces, $this->InterfacesFile));
            // copy the 'new' interfaces file into 'interfaces' in both etc/network, and home/root overwriting any file that may be there...
            $strcmdResult = NXTW_shell_exec(sprintf("10".chr(0x1F)."%s".chr(0x1F)."%s", escapeshellarg($this->etcNetworkPath.$this->InterfacesFile.'-new'), escapeshellarg($this->etcNetworkPath.$this->InterfacesFile)));//"cp /etc/network/%s-new  /etc/network/%s", $this->InterfacesFile, $this->InterfacesFile));
            $strcmdResult = NXTW_shell_exec(sprintf("11".chr(0x1F)."%s".chr(0x1F)."%s", escapeshellarg($this->etcNetworkPath.$this->InterfacesFile.'-new'), escapeshellarg($this->RootFolder.'/'.$this->InterfacesFile)));//"cp /etc/network/%s-new  %s/%s", $this->InterfacesFile, $this->RootFolder, $this->InterfacesFile));

	     // save dns settings to resolv.conf file
	     $this->SaveDNSSettings($this->dns1, $this->dns2);

            //todo: update address 0 in the rc.conf file
           //this whole area is trying to configure for static so we'll stick the rc.conf mod here
           //todo:  DNS is not always the gateway
           //ensure we can execute the script
           // $strcmdResult = NXTW_shell_exec(sprintf("12"));//"chmod 777 /home/www/scripts/SetIp.sh"));
		   chmod("/home/www/scripts/SetIp.sh", 777);

            $strcmdResult = NXTW_shell_exec(sprintf("13".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s", escapeshellarg($this->IpOfBoard), escapeshellarg($this->SubNetMask), escapeshellarg($this->Gateway), escapeshellarg($this->dns1), escapeshellarg($this->dns2)));//"/home/www/scripts/SetIp.sh %s %s %s %s %s", $this->IpOfBoard,$this->SubNetMask,$this->Gateway, $this->dns1, $this->dns2));
           
            //$strcmdResult = NXTW_shell_exec(sprintf("14".chr(0x1F)."%s", $strcmdResult));//"echo -e \\\"%s\\\" >>test.txt", $strcmdResult));
           
            // create checksum file
            $strcmdResult = NXTW_shell_exec(sprintf("15".chr(0x1F)."%s".chr(0x1F)."%s", escapeshellarg($this->RootFolder.'/'.$this->InterfacesFile), escapeshellarg($this->RootFolder.'/'.$this->InterfacesFile.'.md5')));//"md5sum %s/%s > %s/%s.md5", $this->RootFolder, $this->InterfacesFile, $this->RootFolder, $this->InterfacesFile));

            // Always change serial number???
            //if (p_oldSerialNumber != p_SerialNumber)
            {
                // DMOTODO -- for now do not update... I can't find where in IPConfig SerialNumber will ever ever change.
                // Question for Rajesh:  Does the value in udip.service need to match the hostname?
//                $strcmdResult = shell_exec(sprintf("sed -i -e \"s/\(.*_\)\(.*_\)\(.*_\)\(.*_\)/\1\2\3%s_/g\" /etc/avahi/services/udip.service  ", $this->SerialNumber));
            }
	//		WriteToTempFile("16");
            // create Eyelock.run (flag file for the board's app to run)
            $strCmdResult = NXTW_shell_exec(sprintf("16".chr(0x1F)."%s", escapeshellarg($this->RootFolder.'/Eyelock.run')));//"touch %s/Eyelock.run;echo > /home/eyelock/Eyelock.run", $this->RootFolder));
	//		WriteToTempFile("16done");
        }

        if ($bDeviceNameChanged || $bChanged)
        {
            // reboot
            // DMOTODO countdown timer followed by attempt to reconnect...
		//	WriteToTempFile("changed\n");
			NXTW_shell_exec("166");
           // shell_exec(sprintf("%s/%s", $this->RootFolder, $this->ReloadInterfacesFile));
	//		WriteToTempFile("changeddone\n");
        }

        return TRUE;
    }
    


    function ConfigureAsDHCP($row, $bSwitchingtoDHCP, $bForceUpdate, $bIsHBOX)
    {
        $bSuccess = true;

        // Update DHCP timeout and retries (just do it always)
        $this->SaveReloadInterfacesSettings($row, $bForceUpdate, $bIsHBOX);

        // If moving into DHCP mode from static... we need to do more...
        if ($bSwitchingtoDHCP)
        {
            // delete interfaces file
             //ensure we can execute the script
            $strcmdResult = NXTW_shell_exec(sprintf("17"));//"chmod 777 /home/www/scripts/SetDHCP.sh"
            NXTW_shell_exec(sprintf("171"));
	    
            $this->SaveDNSSettings($this->Gateway,$this->Gateway);

            if ($this->DeleteFileAndReboot($this->RootFolder, $this->InterfacesFile, false, $bIsHBOX))
                $bSuccess = $this->DeleteFileAndReboot($this->RootFolder, $this->InterfacesFile . ".md5", true, $bIsHBOX);
            else
                $bSuccess = false;
        }

        return $bSuccess;
    }


    function DeleteFileAndReboot($RemoteFolder, $RemoteFile, $bShouldReboot, $bIsHBOX)
    {
        $bSuccess = true;

        $strcmdResult = NXTW_shell_exec(sprintf("18".chr(0x1F)."%s", escapeshellarg($RemoteFolder.'/'.$RemoteFile)));//"rm %s/%s", $RemoteFolder, $RemoteFile));

        if ($bShouldReboot === true)
        {
            // reboot
            // Copy the dhcp version of the interfaces file from our backup before rebooting...
            $strcmdResult = NXTW_shell_exec(sprintf("19".chr(0x1F)."%s".chr(0x1F)."%s", $this->InterfacesFile, $this->InterfacesFile));//"cp /etc/network/%s-bkup  /etc/network/%s", $this->InterfacesFile, $this->InterfacesFile));


            // DMOHBOX -- this should NOT execute in an HBOX environment... the file won't exist...
            if (!$bIsHBOX)
                NXTW_shell_exec(sprintf("20".chr(0x1F)."%s".chr(0x1F)."%s", $this->RootFolder, $this->ReloadInterfacesFile));//"%s/%s", $this->RootFolder, $this->ReloadInterfacesFile));
                //set the DHCP in the rc.conf
            
            // No reboot necessary according to Rajesh.  Running reloadinterfaces.sh is enough...
        }

        return $bSuccess;

    }
} // Class
?>
