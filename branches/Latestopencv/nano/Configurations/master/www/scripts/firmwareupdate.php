<?php
// Include classes that read/write settings

require("checklogin.php"); // Make sure user is logged on to get to this page...
if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
{
    //error_log("checklogin: not logged in firmwareup ");
    die;
}
include_once("inieditor.php");
include_once("interfaceeditor.php");


// The Save functionality is called via Ajax by our page to avoid a page refresh on submit...
// We simply create our objects, and pass the post data in to our save functions...
$eyeLockINI = new INIEditor("/home/root/Eyelock.ini");
$interfaceSettings = new InterfaceEditor();

// Load all of our existing settings so that we can replace them later...
$eyeLockINI->LoadIniSettings();
$interfaceSettings->LoadInterfaceSettings($eyeLockINI->HardwareType);


if (isset($_REQUEST['action']))
{
    switch ($_REQUEST['action'])
    {
        case 'upgradefirmware':
        {
/**            
           // DMOTODO this is the 'filename' of the new firmaware... create this file on disk somewhere (where?) with the data stream passed in...
            tar file for update
                 b.TarFileForUpdate, 
            
                 // DMOTODO maybe hard code this?
            remote folder
             "/home/"
            
             // DMO, this should be generated locally to get the local time (not the browser time...
            backup filename
                string.Format("root_{0}.tgz", m_DateTimeStamp.ToString("yyyyMMdd_HHmmss"))))
**/
            //Save the contents to disk with given filename
            if (CreateUpgradeFile($_REQUEST['tarfilename'], $_REQUEST['tarfiledata']))
            {
/*
                // Check the newly created tar file for corruption
                if (!CheckTarFile($_REQUEST['tarfilename'])) // If fails, file is corrupted...
                    return false;

                if (!BackupSystemTree($strBackupFilename))
                    return false; // Maybe cleanup upgrade tar file?

                // Kill the Eyelock app
                KillAppOnBoard("Eyelock");

                if (!InstallUpgradeFile($_REQUEST['tarfilename']))
                    return false; // Maybe cleanup upgrade tar file?

                if (!UpgradePostProcess())
                    return false; // Maybe cleanup upgrade tar file?

                if (!UpgradeFinalize())
                    return false; // Maybe cleanup upgrade tar file?
*/
                return true;
            }

            break;
        }
    }
}


/////////////////////////////////////////////////////////
// Creates a file in the filesystem containing all of the
// contents of the tar upgrade file
/////////////////////////////////////////////////////////
function CreateUpgradeFile($strFilename, $fileContents)
{
    // Ok, we have a filename and a big chunk of data...
    // create the file in the correct location and copy the data into it
    $strCmd = sprintf("echo %s > %s", escapeshellarg($fileContents), escapeshellarg($strFilename));
    return shell_exec($strCmd);

    //return true;
}


/////////////////////////////////////////////////////
// CheckTarFile 
/////////////////////////////////////////////////////
function CheckTarFile($strFilename)
{
    $ResultMd5Check = -1;

    $strResult = shell_exec("tr '\r' '\n' < /home/checkmd5.sh > /home/tmpfilesed");

    $strResult = shell_exec("mv /home/tmpfilesed /home/checkmd5.sh;chmod a+x /home/checkmd5.sh");

    $strResult = shell_exec("rm /home/tmpfilesed");

    $strCmd = sprintf("cd /home;sync;sleep 2;./checkmd5.sh %s", escapeshellarg($strFilename));
    $strResult = shell_exec($strCmd);

    $ResultMd5Check = intval($strResult);

    return ($ResultMd5Check === 0);
}



/////////////////////////////////////////////////////
// Backup Current System
/////////////////////////////////////////////////////
function BackupSystemTree($strBackupFilename)
{
    // If no backup filename supplied, generate one...
    if (strlen($strBackupFilename) <= 0)
    {
        $strDateTimeStamp = date("Y-m-d_H:i:s"); 
        $strBackupFilename = sprintf("root_%s.tgz", $strDateTimeStamp);
    }

    $strResult = shell_exec("[ -e /home/root/eyestartup ] && echo 0 || echo 1");

    if($strResult === "0\n")
    {
        $strCmd = sprintf("cd ..;cp -R /home/eyelock/data /home/root/;sync;tar czf %s root", escapeshellarg($strBackupFilename));
        $strResult = shell_exec($strCmd);
    }
    else
    {
        $strCmd = sprintf("cd ..;tar czf %s root", escapeshellarg($strBackupFilename));
        $strResult = shell_exec($strCmd);
    }                   
    
    // DMOTODO how to check for successful backup????
    return TRUE;
}


/////////////////////////////////////////////////////
// Install the new firmware (untar it)
/////////////////////////////////////////////////////
function InstallUpgradeFile($strFilename)
{
    $strCmd = sprintf("cd ..;rm -rf root;tar xzf %s", escapeshellarg($strFilename));
    $strResult = shell_exec($strCmd);

    // assume success and look for eyestartup
    $strResult = shell_exec("[ -e /home/root/eyestartup ] && echo 0 || echo 1");

    if ($strResult !== "0\n") //root related files existing
    {
        $strResult = shell_exec("rm -rf /etc/init.d/eyestartup;rm -rf /etc/rc5.d/S95eyestartup;ln -sf /home/root/startup.sh /etc/rc5.d/S95Startup;sync");
    }
    else
    {
        $strResult = shell_exec("sh /home/root/install.sh");
    }

    return true;
}


/////////////////////////////////////////////////////
// Bring previous settings forward...
/////////////////////////////////////////////////////
function UpgradePostProcess()
{
    if (!SetExecPrivileges("/home/root/", "Eyelock"))
    {
       // log.Debug(string.Format("error when setting exe privileges for file {0} to board {1}", "Eyelock", p_IpOfBoard.ToString()));
    }

    if (!SetExecPrivileges("/home/root/", "*.sh"))
    {
       // log.Debug(string.Format("error when setting exe privileges for file {0} to board {1}", "Eyelock", p_IpOfBoard.ToString()));
    }

/**
                    // set the entries in Eyelock.ini
                    List<string> p_ListKeysToChange;

                    log.Debug(String.Format("IsMatcher {0}", IsMatcher));
                    if (IsMatcher)
                    {
                        p_ListKeysToChange = new List<string>()
                                                    {      "GRI.HDMatcher.0.Address", "GRI.SlaveAddressList"
                                                        , "GRI.HBDestAddress", "GRI.MatchResultDestAddr", "GRITrigger.F2FEnable", "GRITrigger.WeigandEnable"
                                                        , "GRI.NwListenerSecure", "GRITrigger.DualAuthenticationMode", "GRITrigger.EnableNegativeMatchTimeout"
                                                    };  // "GRI.EyeDestAddr",

                        if (((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRITrigger_RelayEnable != null)
                            p_ListKeysToChange.Add("GRITrigger.RelayEnable");

                        if (((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRI_AuthorizationToneDurationSeconds != 0)
                            p_ListKeysToChange.Add("GRI.AuthorizationToneDurationSeconds");

                        if (((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRI_AuthorizationToneVolume != 0.0m)
                            p_ListKeysToChange.Add("GRI.AuthorizationToneVolume");

                        if (((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRI_AuthorizationToneFrequency != 0)
                            p_ListKeysToChange.Add("GRI.AuthorizationToneFrequency");

                        if (((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRI_RepeatAuthorizationPeriod != 0)
                            p_ListKeysToChange.Add("GRI.RepeatAuthorizationPeriod");

                        if (((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRITrigger_RelayTimeInMS != 0)
                            p_ListKeysToChange.Add("GRITrigger.RelayTimeInMS");

                        if (((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRI_LEDBrightness != 0)
                            p_ListKeysToChange.Add("GRI.LEDBrightness");

                        if (((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRITrigger_NegativeMatchTimeout != 0)
                            p_ListKeysToChange.Add("GRITrigger.NegativeMatchTimeout");

                        if (((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRITrigger_NegativeMatchResetTimer != 0)
                            p_ListKeysToChange.Add("GRITrigger.NegativeMatchResetTimer");

                        if (((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRI_CameraRecipe != "1800,255:1100,255" &&
                            ((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRI_CameraRecipe != "2300,255:1100,255")
                        {
                                p_ListKeysToChange.Add("GRI.CameraRecipe");
                        }

                        if (((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRITrigger_WeigandHidEnable != null)
                            p_ListKeysToChange.Add("GRITrigger.WeigandHidEnable");

                        if (((ConfigIniDetailsNano)HolderForSoftwareUpdate.DetailsConfigIni).GRITrigger_PACEnable != null)
                            p_ListKeysToChange.Add("GRITrigger.PACEnable");
                    }
                    else
                    {
                        p_ListKeysToChange = new List<string>()
                                                    {     "GRI.HDMatcher.0.Address","Eyelock.TSMasterDestAddr"
                                                        , "GRI.HBDestAddress", "GRI.MatchResultDestAddr"
                                                        , "GRITrigger.F2FEnable", "GRI.NwListenerSecure"
                                                    }; // "GRI.EyeDestAddr",
                    }

                        
                    if (!l_IpHandler.ChangeSomeReaderConfigEntries_SingleThreaded(p_IpOfBoard, RemoteFolder, RemoteFile, HolderForSoftwareUpdate.DetailsConfigIni, p_ListKeysToChange))
                    {
                        FireStepOcurred(new StatusMessage(string.Format("error when setting Eyelock.ini to custom values to board {0} with IP {1}", p_MacAddress, p_IpOfBoard.ToString()), StatusMessageTypes.Error));
                        log.Debug(string.Format("error when setting Eyelock.ini to custom values to board {0} with IP {1}", p_MacAddress, p_IpOfBoard.ToString()));
                    }

                    // set interfaces if IP is static
                    if (HolderForSoftwareUpdate.IsStatic)
                    {
                        l_IpHandler.IpAssigned += new IpHandler.AssignIPHandler(UpdateSoftware_l_IpHandler_IpAssigned);
                        //l_IpHandler.ChangeInterfaces(p_IpOfBoard
                        //                              , HolderForSoftwareUpdate.StaticInterfaces.IpOfBoard
                        //                              , "/home/root/"
                        //                              , "interfaces"
                        //                              , HolderForSoftwareUpdate.StaticInterfaces.Network.ToString()
                        //                              , HolderForSoftwareUpdate.StaticInterfaces.SubNetMask.ToString()
                        //                              , HolderForSoftwareUpdate.StaticInterfaces.BroadcastIp.ToString()
                        //                              , HolderForSoftwareUpdate.StaticInterfaces.Gateway.ToString()
                        //                            );
                        HolderForSoftwareUpdate.StaticInterfaces.SerialNumber = SerialNumber; 
                        l_IpHandler.ChangeInterfaces(p_IpOfBoard
                              , HolderForSoftwareUpdate.StaticInterfaces.IpOfBoard
                              , GetHomePath.homePath
                              , "interfaces"
                              , HolderForSoftwareUpdate.StaticInterfaces.Network.ToString()
                              , HolderForSoftwareUpdate.StaticInterfaces.SubNetMask.ToString()
                              , HolderForSoftwareUpdate.StaticInterfaces.BroadcastIp.ToString()
                              , HolderForSoftwareUpdate.StaticInterfaces.Gateway.ToString()
                              , HolderForSoftwareUpdate.StaticInterfaces.SerialNumber.ToString(),
                              HolderForSoftwareUpdate.StaticInterfaces.SerialNumber.ToString()
                            );
                        log.Debug("HolderForSoftwareUpdate Is Static");

                    }
                    else
                    {
                        CompletionOfSotwareUpload(p_IpOfBoard, true);
                        log.Debug("CompletionOfSotwareUpload ");
                    }  // of if/else

**/
}

function UpgradeFinalize()
{
/**      
        public void CompletionOfSotwareUpload(IPAddress p_NewIpOfBoard, bool p_SholdCreateEyelockRun=false )
        {
            IpHandler l_IpHandler = Device.CreateIpHandler(DeviceTypes.Nano, p_NewIpOfBoard, p_NewIpOfBoard, MacAddress, Username, Password, WaitTimeAfterBoardReboots, SshTimeout);
            log.Debug(String.Format("CompletionOfSotwareUpload entered NewIpOfBoard {0} SholdCreateEyelockRun {1}", p_NewIpOfBoard, p_SholdCreateEyelockRun));
            l_IpHandler.StepOcurred += new StepOcurrence(l_IpHandler_StepOcurred);

            if (p_SholdCreateEyelockRun)
            {
                // create Eyelock.run for app to run
                // reboot board
                log.Debug("p_SholdCreateEyelockRun");
                l_IpHandler.Execute_install(p_NewIpOfBoard, 65000);
                //l_IpHandler.CreateFileAndReboot(p_NewIpOfBoard, MacAddress, "/home/root/", "Eyelock.run", true);
                l_IpHandler.CreateFileAndReboot(p_NewIpOfBoard, MacAddress, GetHomePath.homePath, "Eyelock.run", true);

                // wait for board to reboot
                //System.Threading.Thread.Sleep(WaitTimeAfterBoardReboots);

                FireStepOcurred(new StatusMessage(string.Format("pinging hostname {0} for board {1}", Hostname, MacAddress), StatusMessageTypes.Info));

                BulkPingerDN l_BulkPinger = new BulkPingerDN(p_NewIpOfBoard.ToString(), 20, 5000);

                l_BulkPinger.DevicePinged += new PingDeviceHandlerDN(CompletionOfSotwareUpload_l_PingDoer_DevicePinged);

                l_BulkPinger.StartPinging();
                log.Debug("l_BulkPinger.StartPinging");

            }
            else
            {
                log.Debug("SoftwareUpdate started");
                FireSoftwareUpdated(true);
                log.Debug("SoftwareUpdated");
            }// of if

        }  // of CompletionOfSotwareUpload()

        protected void CompletionOfSotwareUpload_l_PingDoer_DevicePinged(string p_HostnameOrAddress, IPAddress p_IpTarget, string p_Result, bool p_bSuccess)
        {
            FireStepOcurred(new StatusMessage(string.Format("board {3}, hostname: {0} pinged (ip: {2}). It was {1}"
                                                            , p_HostnameOrAddress, (p_bSuccess) ? "successful" : "failed"
                                                            , (p_IpTarget!=null)?p_IpTarget.ToString():""
                                                            , MacAddress
                                                           )
                                              , StatusMessageTypes.Info
                                             )
                           );

            FireSoftwareUpdated(p_bSuccess);

        }  // of CompletionOfSotwareUpload_l_PingDoer_DevicePinged()

**/
}

function SetExecPrivileges($strFolder, $strFilename)
{
    $strCmd = sprintf("cd %s; chmod a+x %s;sleep 5", escapeshellarg($strFolder), escapeshellarg($strFilename));
    $strResult = shell_exec($strCmd);
    
    return TRUE;
}


function KillAppOnBoard($strAppName)
{
    $strFileName = "identityNkillEyelock.sh";

    $strResult = shell_exec("[ ! -e /home/root/identityNkillEyelock.sh ] && echo 0 || echo 1;");

    if ($strResult === "1\n")
    {
        $strCmd = sprintf("sh %s", escapeshellarg('/home/root/'.$strAppName));
        $strResult = shell_exec($strCmd);
    }
    else
    {
        //found root-related files, so need to rerun for killing app
        $strResult = shell_exec("killall -s SIGKILL bash");

        $strCmd = sprintf("killall -s SIGKILL %s", escapeshellarg($strAppName));
        $strResult = shell_exec($strCmd);
    }
}
?>