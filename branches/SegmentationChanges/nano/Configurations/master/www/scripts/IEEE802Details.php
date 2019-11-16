<?php

require("checklogin.php"); // Make sure user is logged on to get to this page...
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");
// Class handles reading/updating interface specifics of the nano...
class IEEE802Details
{
   ////////////////////////////////////////////////////
    // 802 values of interest
    ////////////////////////////////////////////////////
    public $EnableIEEE8021X = "false";
    public $CACertPath = "";
    public $ClientCertPath = "";
    public $ClientPrivateKeyPath = "";
    public $CACertFullPath = "";
    public $ClientCertFullPath = "";
    public $ClientPrivateKeyFullPath = "";
    public $EAPOLVersion = "2";
    public $EAPIdentity = "";
    public $PrivateKeyPassword = "";

    public $ExistCACertFullPath = "";
    public $ExistClientCertFullPath = "";
    public $ExistClientPrivateKeyFullPath = "";
    public $ExistEAPOLVersion = "";
    public $ExistEAPIdentity = "";
    public $ExistPrivateKeyPassword = "";

    public $CertFolder;
    public $ConfigFilename;
    public $ConfigFileContents;
    public $CertFileContents;

    public $bfullReboot = FALSE;
   
	
    // Constructor...
	function __construct ()
	{
        // Then parse results to populate variables...
        $this->CertFolder = "/home/www-internal/802.1XCerts";
        $this->ConfigFilename = "wpa_supplicant-wired.conf";
	}


    function LoadIEEE802Config()
    {
        if (!$this->GetCertFilenames($this->CertFolder) || !$this->PreParseConfig())
            $this->EnableIEEE8021X = "false";
    }


    function PreParseConfig()
    {
    	if (file_exists("/home/www-internal/802.1XCerts/wpa_supplicant-wired.conf"))
        {
            $ConfigFileContents = file_get_contents("/home/www-internal/802.1XCerts/wpa_supplicant-wired.conf");
            $this->Parse802Config($ConfigFileContents);
            return TRUE;
	    }
        else
            return FALSE;
    }

    function IsFullRebootNeeded()
    {
        return $this->bfullReboot;
    }
    

    function SaveIEEE802Config($row)
    {
        $this->bfullReboot = FALSE;

        $this->ExistCACertFullPath = "";
        $this->ExistClientCertFullPath = "";
        $this->ExistClientPrivateKeyFullPath = "";
        $this->ExistEAPOLVersion = "";
        $this->ExistEAPIdentity = "";
        $this->ExistPrivateKeyPassword = "";

        // Now update/create our changed config file, but only if it is enabled...
        if (!$this->Update802ConfigFile($row, $this->isIEEE802Enabled($row))) 
        {
            // We only get here if something is wrong with the certs... warn user to setup the certs again...
            // Could be a corrupted certs text file... so Delete it.
            unlink(sprintf("%s/%s", $thepath, "802Certs.txt"));

            $this->CACertPath = "";
            $this->ClientCertPath = "";
            $this->ClientPrivateKeyPath = "";
            $this->CACertFullPath = "";
            $this->ClientCertFullPath = "";
            $this->ClientPrivateKeyFullPath = "";
            $this->EnableIEEE8021X = "false";


            return FALSE;
        }

        return TRUE;
    }




    // We populate our string, then just rewrite the conf file contents with the string
    function Update802ConfigFile($row, $bEnabled)
    {
   //     $cert_dir = "/home/www-internal/802.1XCerts/";
   //  	shell_exec(sprintf("mkdir %s", escapeshellarg($cert_dir)));

	//	error_log("update802ConfigFile...");
        $bNoCertFile = FALSE;
        $bRetVal = TRUE;

        // Load config values...
        $this->PreParseConfig();

        // Load Cert information we call this again for simplicity
        if (!$this->GetCertFilenames($this->CertFolder))
        {
            // If we are trying to turn on 802, we must make sure that our certs exist (at least some)
            if ($bEnabled)
            {
                //return FALSE;//some fail value so we can show_source something in theUI
                $bRetVal = FALSE;
            }
            else
            {
                $bNoCertFile = TRUE;
            }
        }

        //Gather up our values...
   		foreach ($row as $key => $value) 
		{
            if ($key === "EAPVersion")
            {
                if ($this->EAPOLVersion !== $value)
                    $this->bfullReboot = $bEnabled;
                $this->EAPOLVersion = $value;
            }
            elseif  ($key === "EAPIdentityName")
            {
                if ($this->EAPIdentity !== $value)
                    $this->bfullReboot = $bEnabled;

                $this->EAPIdentity = $value;
            }
            else if ($key === "PrivateKeyPassword")
            {
                if ($this->PrivateKeyPassword !== $value)
                    $this->bfullReboot = $bEnabled;

                $this->PrivateKeyPassword = $value;
            }
        }


        // Now see if any of the certs have changed...
        if (!$bNoCertFile) // If the cert file was never created, we don't even need to check.
        {
            if (($this->CACertFullPath !== $this->ExistCACertFullPath) || ($this->ClientCertFullPath !== $this->ExistClientCertFullPath) ||
                ($this->ClientPrivateKeyFullPath !== $this->ExistClientPrivateKeyFullPath))
            {
                $this->bfullReboot = $bRetVal;

            }
        }

        // Handle case here if we're on, and we're turning off... or if we didn't change anything and just turned on
        if (($bEnabled && ($this->EnableIEEE8021X !== "true")) || (!$bEnabled && ($this->EnableIEEE8021X !== "false")))
        {
            $this->bfullReboot = $bRetVal;

        }
            

        // Using our current settings, create our backup string, in case we need to make the backup files...
        if ($bEnabled)
            $ConfigFileContents = sprintf("#EnableIEEE8021X=%s\n\n", "true");
        else
            $ConfigFileContents = sprintf("#EnableIEEE8021X=%s\n\n", "false");
        $ConfigFileContents .= "ctrl_interface=/var/run/wpa_supplicant\n\n";
        $ConfigFileContents .= "ctrl_interface_group=0\n\n";
        $ConfigFileContents .= sprintf("eapol_version=%s\n\n", $this->EAPOLVersion);
        $ConfigFileContents .= "ap_scan=0\n\n";
        $ConfigFileContents .= "network={\nkey_mgmt=IEEE8021X\neap=TLS\n";
	    $ConfigFileContents .= sprintf("identity=\"%s\"\n", $this->EAPIdentity);
        if ($this->CACertFullPath === "")
            $ConfigFileContents .= "#ca_cert=\"\"\n";
        else
	        $ConfigFileContents .= sprintf("ca_cert=\"%s\"\n", $this->CACertFullPath);
        if ($this->ClientCertFullPath === "")
	        $ConfigFileContents .= "#client_cert=\"\"\n";
        else
            $ConfigFileContents .= sprintf("client_cert=\"%s\"\n", $this->ClientCertFullPath);
	    $ConfigFileContents .= sprintf("private_key=\"%s\"\n", $this->ClientPrivateKeyFullPath);
	    $ConfigFileContents .= sprintf("private_key_passwd=\"%s\"\n}", $this->PrivateKeyPassword);

        file_put_contents("/home/www-internal/802.1XCerts/wpa_supplicant-wired.conf", $ConfigFileContents);


       return $bRetVal;
    }

  
    /////////////////////////////////////////////////
    // Helper functions
    /////////////////////////////////////////////////
    function Parse802Config($ConfigFileContents)
    {
        $arConfig = explode("\n", $ConfigFileContents);

        // Loop through all lines...
        for($i=0 ; $i<count($arConfig) ; $i++)
        {
            $line = $arConfig[$i];
            if (isset($line))
            {
                $arLine = explode("=", $line);

                if (isset($arLine[0]) && isset($arLine[1]))
                {
                    if (trim($arLine[0]) === "#EnableIEEE8021X")
                        $this->EnableIEEE8021X = trim($arLine[1]);
                    else if (trim($arLine[0]) === "eapol_version")
                        $this->EAPOLVersion = trim($arLine[1]);
                    else if (trim($arLine[0]) === "identity")
                        $this->EAPIdentity = trim(trim($arLine[1]),'"');
                    else if (trim($arLine[0]) === "private_key_passwd")
                        $this->PrivateKeyPassword = trim(trim($arLine[1]),'"');
                    else if (trim($arLine[0]) === "ca_cert")
                        $this->ExistCACertFullPath = trim(trim($arLine[1]),'"');
                    else if (trim($arLine[0]) === "client_cert")
                        $this->ExistClientCertFullPath = trim(trim($arLine[1]),'"');
                    else if (trim($arLine[0]) === "private_key")
                        $this->ExistClientPrivateKeyFullPath = trim(trim($arLine[1]),'"');
                }
            }        
        }
    }



    function ProcessCertFileItems($arFileItems)
    {
        for($i=0 ; $i<count($arFileItems) ; $i++)
        {
            $line = $arFileItems[$i];
            if (isset($line))
            {
                $arFileItem = explode("=", trim($line));

                // Parse out filepath and identify file
                if (isset($arFileItem[0]))
                {
                    $strCert = trim($arFileItem[0]);
                    if (isset($arFileItem[1]))
                    {
                        $strCertPath = trim($arFileItem[1]);
            	        if (file_exists($strCertPath))
                        {
                            if ($strCert === "ca_cert")
                            {
                                $this->CACertPath = basename($strCertPath);
                                $this->CACertFullPath = $strCertPath;
                            }
                            else if ($strCert === "client_cert")
                            {
                                $this->ClientCertPath = basename($strCertPath);
                                $this->ClientCertFullPath = $strCertPath;
                            }
                            else if ($strCert === "private_key")
                            {
                                $this->ClientPrivateKeyPath = basename($strCertPath);
                                $this->ClientPrivateKeyFullPath = $strCertPath;
                            }

                        }
                   //     else
                   //         return FALSE;  // If any file doesn't exist, we fail...
                    }
                }
            }
        }


        // Ok, we now know what we have for certs... Enforce config correctly...
        if ($this->ClientPrivateKeyPath === "") // Private key is required
        {
            return FALSE;
        }
        else
        {
             if (((strtolower(trim(pathinfo($this->ClientPrivateKeyPath, PATHINFO_EXTENSION))) !== "pem") && (strtolower(trim(pathinfo($this->ClientPrivateKeyPath, PATHINFO_EXTENSION))) !== "der")) && ($this->ClientCertPath === ""))
             {
                return FALSE;
             }
        }

        return TRUE;
    }


    function GetCertFilenames($thepath)
    {
       // Read the certs Text file...
	    if (file_exists(sprintf("%s/%s", $thepath, "802Certs.txt")))
        {
            $CertFileContents = file_get_contents(sprintf("%s/%s", $thepath, "802Certs.txt"));

            // Parse the file
            $arFileResult = explode("\n", $CertFileContents);

            return $this->ProcessCertFileItems($arFileResult);
        }

        // No certs added...
        return FALSE;
    }


    function isIEEE802Enabled($row)
    {
  		foreach ($row as $key => $value) 
		{
            if  ($key === "Eyelock_EnableIEEE8021X")
			{
				return TRUE;
            }
        }

        return FALSE;
    }

} // Class
?>
