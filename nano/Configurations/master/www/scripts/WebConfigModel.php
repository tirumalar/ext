<?php


// Include classes that read/write settings
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/inieditor.php");
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/IEEE802Details.php");
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/IPv6Details.php");
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/interfaceeditor.php");
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/ApplicationDetails.php");


class Model
{
    // Model Members...
    private $inifile;
    public $eyeLockINI;
    public $interfaceSettings;
    public $ipv6Settings;
    public $IEEE802Settings;
    public $applicationDetails;
    private $currLanguage = "us"; // default
    private $lasttab;


    // create out reader/writer objects...
    public function __construct()
    {
         $this->inifile = "/home/root/Eyelock.ini";
         $this->eyeLockINI = new INIEditor($this->inifile);
         $this->IEEE802Settings = new IEEE802Details();
         $this->interfaceSettings = new InterfaceEditor();
         $this->ipv6Settings = new IPv6Details();
         $this->applicationDetails = new ApplicationDetails();
    }

    ///////////////////////////////////////////////////////
    // Functions for access to INI processing
    ///////////////////////////////////////////////////////
    function GetIniParam($key)
    {
        return $this->eyeLockINI->get_ini_parameter($key);
    }


    // set new parameters from any place in your project with this function
    function SetIniParam($key, $value)
    {
        return $this->eyeLockINI->set_ini_parameter($key, $value);
    }


    function LoadAllSettings()
    {
        // Need to load in constructor so it's available for parsing in the html
        // Do not remove these loads!  These are 'reloads' that occur AFTER a successful SAVE operation!
        $this->eyeLockINI->LoadIniSettings();
        $this->IEEE802Settings->LoadIEEE802Config();
        $this->interfaceSettings->LoadInterfaceSettings($this->eyeLockINI->HardwareType);
        $this->ipv6Settings->LoadConfig();

        // Get the local nano time, and kickoff our clock...
//         echo '<script> startTime(); </script>';
    }


    // This function updates all elements...
    function UpdateAllSettings($thePost)
    {
        // Display 'wait page'?
        //echo "{$thePost}</br>";

        // Allow each reader/writer class to update itself...
        $this->eyeLockINI->SaveIniSettings($thePost);
        $this->IEEE802Settings->SaveIEEE802Config($thePost);

        // This function might cause a device reboot to be invoked...
        $this->interfaceSettings->SaveInterfaceSettings($thePost, false, $this->eyeLockINI->HardwareType);
    }

    ///////////////////////////////////////////////////////
    // Helper Functions 
    ///////////////////////////////////////////////////////
    public function IsDebug()
    {
        return true;
    }

    public function SetLoggedOn($success)
    {
       // $_SESSION['loggedon'] = TRUE;
    }


    function SetLastTab($tabID)
    {
        $this->lasttab = $tabID;
    }

    function IsLastSelectedTab($tabID)
    {
        if (isset($this->lasttab))
        {
            if ($tabID === $this->lasttab)
                return TRUE;
            else
                return FALSE;
        }
        else
            return ($tabID === "tab1");
    }

    function SplitLeft($value, $delimiter)
    {
        if (!isset($value) || ($value === ""))
            return "";

        list($left, $right) = explode($delimiter, $value);
        return $left;
    }

    function SplitRight($value, $delimiter)
    {
        if (!isset($value) || ($value === ""))
            return "";

        list($left, $right) = explode($delimiter, $value);
        return $right;
    }

    function SetCurrentLanguageId($currLang)
    {
        // Make the language available to javascript...
        $this->currLanguage = $currLang;
    }


    function GetCurrentLanguageId()
    {
        return $this->currLanguage;
    }
}    
?>
