<?php
// Include classes that read/write settings
//error_log("saveconfig_preinclude");
require("checklogin.php"); // Make sure user is logged on to get to this page...
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");
if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
{
    //error_log("checklogin: not logged in save");
    die;
}



include_once("inieditor.php");
include_once("interfaceeditor.php");
include_once("restarteyelock.php");
include_once("IEEE802Details.php");



///Clear the log when printing with this one.  

// The Save functionality is called via Ajax by our page to avoid a page refresh on submit...
// We simply create our objects, and pass the post data in to our save functions...
$eyeLockINI = new INIEditor("/home/root/Eyelock.ini");
$interfaceSettings = new InterfaceEditor();
$IEEE802Settings = new IEEE802Details();

//error_log("saveconfig_postinclude");
// Save everything out...
// Need to figure out how exactly we can provide reasonable feedback during this process...


$eyeLockINI->LoadIniSettings(); // Need to load what it was, so that we can compare things for saving out... (maybe rethink this code... why check at all here?)
$eyeLockINI->SaveIniSettings($_POST);
$interfaceSettings->LoadInterfaceSettings($eyeLockINI->HardwareType); // Must load the current state so we know how to process things when we write the new stuff out.
//$IEEE802Settings->LoadIEEE802Config();


if (!$IEEE802Settings->SaveIEEE802Config($_POST))
{
    $IEEE802Settings->EnableIEEE8021X = "false"; // Failure?  Turn this back off...
    echo "saveconfig|error|802CertFail";
//            echo "saveconfig|error|IPInUse";
}
else
{
    $prevStaticIPFlag = $interfaceSettings->IsStatic;
    $prevStaticIP = $interfaceSettings->IpOfBoard;
    $prevDeviceName = $interfaceSettings->DeviceName;

    if (!$interfaceSettings->SaveInterfaceSettings($_POST, false, $eyeLockINI->HardwareType)) // This is simpler, just rewrites the script with the updated values... (no matter what)
    {
        // DMOTODO
        // For now, this is just a static IP in use failure!  Add enhanced error reporting later!
        echo "saveconfig|error|IPInUse";
    }
    else
    {
        // ok, save interfaces settings ultimately calls reloadinterfaces.sh for us... no need for further processing here.
        // Now that everything is saved... we need to restart the app or reboot depending on what has changed...
   // WriteToTempFile("postProcChanges\n");
        PostProcessSettingsChange();
   // WriteToTempFile("postProcChangesDone\n");
        // Ok, if we switch from DHCP to Static or vice versa... OR if we changed the static IP... we need
        // To notify the client so that it can reload...
    
        $fullReboot = FALSE;
  
        if ($_POST['deviceipmode'] === "staticip") // changing to or staying on static...
        {
            if ((!$prevStaticIPFlag || ($_POST['IpOfBoard'] !== $prevStaticIP)) && isset($_POST['IpOfBoard']))
            {
                echo "saveconfig|refreshpageip|{$_POST['IpOfBoard']}";   // The url to use in the refresh..
                $fullReboot = TRUE;
            }
            else
            {
                if ($IEEE802Settings->IsFullRebootNeeded())
                    echo "saveconfig|refreshpagenetwork|success";
                else
                    echo "saveconfig|success|success";
            }
        }
        else // must be dhcp            
        {
            if (($prevStaticIPFlag || ($_POST['DeviceName'] !== $prevDeviceName) && isset($_POST['DeviceName'])))
            {
                echo "saveconfig|refreshpagedhcp|{$_POST['DeviceName']}"; // The url to use in the refresh...
                 $fullReboot = TRUE;
            }
            else
            {
                if ($IEEE802Settings->IsFullRebootNeeded())
                    echo "saveconfig|refreshpagenetwork|success";
                else
                    echo "saveconfig|success|success";
            }
        }

        if (($fullReboot == TRUE) || $IEEE802Settings->IsFullRebootNeeded())  //only restart for IP address change
        {
//		    WriteToTempFile("reboot\r\n");
              RebootDevice();
        }
    }   
}


/**
function PostProcessSettingsChange()
{
    // Creates the Eyelock.run file if it does not already exist...
    CreateEyelockRun();
    RestartApplication();     
}


// DMOTODO add error handling...
function CreateEyelockRun()
{
    $strResult = shell_exec("cd /home/root;ls -la | grep Eyelock.run");

    // If file exists, we're done...
    if (strlen(trim($strResult)) > 0)
        return FALSE;
    else
    {
        // create the file
        $strResult = shell_exec("touch Eyelock.run");

        // write it
        $strResult = shell_exec("echo > /home/root/Eyelock.run;sleep 6");

        return TRUE;
    }

    return FALSE;
}


function RestartApplication()
{
    // This kills it...
    $strResult = shell_exec("killall -s SIGKILL Eyelock");

    // bash script startup.sh is polling and will restart eyelock if it goes down...
}**/
?>
