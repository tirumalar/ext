<?php

require("checklogin.php"); // Make sure user is logged on to get to this page...
if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
{
    //error_log("checklogin: not logged in upgrade");
    die;
}
//$this < look for this when finishing
header('Content-Type: application/octet-stream; X-Content-Type-Options: nosniff;');
header('Cache-Control: no-store, no-cache, must-revalidate, private'); // recommended to prevent caching of event data.
header('Pragma: no-cache');

// Include classes that read/write settings
include_once("inieditor.php");
include_once("restarteyelock.php");
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");


// Turn off output buffering
ini_set('output_buffering', 'off');
// Turn off PHP output compression
ini_set('zlib.output_compression', false);
// Implicitly flush the buffer(s)
ini_set('implicit_flush', true);
ob_implicit_flush(true);
// Clear, and turn off output buffering
while (ob_get_level() > 0) {
    // Get the curent level
    $level = ob_get_level();
    // End the buffering
    ob_end_clean();
    // If the current level has not changed, abort
    if (ob_get_level() == $level) break;
}   


$bManualUpgrade = FALSE;
$nanoFilename = "";
$bobFilename = "";
$bUpgradeNano = FALSE;
$bUpgradeBob = FALSE;


function debug_shell_exec($strExecMe)
{
    debugPrint("---------------------------------------------");
    debugPrint($strExecMe);
    $strResult = shell_exec($strExecMe);
    debugPrint($strResult);
    debugPrint("X--------------------------------------------");
    return $strResult;

}

function icmProgress($strExecMe)
{
   
    $count = 0;
    $cmd = $strExecMe;
    //this will never get done...
    $descriptorspec = array(
        0 => array("pipe", "r"),   // stdin is a pipe that the child will read from
        1 => array("pipe", "w"),////,   // stdout is a pipe that the child will write to
        2 => array("pipe", "a")    // stderr is a pipe that the child will write to
    );
   // flush();
    //$process = proc_open($cmd, $descriptorspec, $pipes);
	NXTW_shell_exec($cmd,true); //exits immediately after sending, no wait for response
   $lastline = "";
   sleep(4);
   // if (is_resource($process)) {
//        while ($s = fgets($pipes[1])) {
//            $resp = sprintf("bobProgress.%s", $s);
//            $lastline = $s;
//            debugPrint($resp);
//            //error_log($resp);
//            echo $resp;
//            flush();
//            $count++;
//        }
//    }
	$running = shell_exec( "ps -A| grep icm_communicato");
//	error_log($running);
	while(strpos($running , "icm_communicato") != FALSE)
	{
		sleep(1);
		$running = shell_exec( "ps -A| grep icm_communicato");
		if(strpos($running, "icm_communicato") == FALSE)
			break; //proc is done
		$lastline = shell_exec("tail -n 1 /home/www/updateprogress");
		echo "running..." . $lastline;
	}
$lastline = shell_exec("tail -n 1 /home/www/updateprogress");
    //error_log("finished icm upgrade last line is ".$lastline);
    if((strpos($lastline , "Successfully") != FALSE || strpos($lastline, "time:")!= FALSE))
        return "ICM program done (can't tell if its actually successful!!!)";
        else
    return "";


}
function stringContains($haystack, $needle)
{
	if(strpos($haystack, $needle) !== false)
		return true;
	if(strpos($haystack, $needle) === false)
		return false;
	
	
}

function getProgressOfUpgrade()
{
	$lastline = shell_exec("cat /home/updateFw1.log");
		
	//echo $lastline;
	$lines = explode("\n", $lastline);
	$progress = 0;
	foreach($lines as $line)
	{
		
		if(stringContains($line, "FW validation")) 
			$progress = 5;
		
		if(stringContains($line, 'Restore point creation done.') )
			$progress = 15;
			 
		if(stringContains($line, "Upgrading master") )
			$progress = 30;
			
		if(stringContains($line, "Upgrading slave") )
			$progress = 60;
			
		if(stringContains($line, "Slave: Done") ) 
			$progress = 70;
			
		if(stringContains($line, "Current ICM version") )
			$progress = 90;
			
		if(stringContains($line, "STATUS:SUCCESS") )
			$progress = 100;
		
		
		
	}
	return $progress;
	
	
}

function updateProgress($strExecMe)
{
//   error_log("the error is a sleeper");
	unlink("/home/updateFw1.log");
    $count = 0;
    $cmd = $strExecMe;
    //this will never get done...
    $descriptorspec = array(
        0 => array("pipe", "r"),   // stdin is a pipe that the child will read from
        1 => array("pipe", "w"),////,   // stdout is a pipe that the child will write to
        2 => array("pipe", "a")    // stderr is a pipe that the child will write to
    );
   // flush();
    //$process = proc_open($cmd, $descriptorspec, $pipes);
//	error_log("execute updateProgress()");
	 sleep(5);
	NXTW_shell_exec($cmd,true); //exits immediately after sending, no wait for response
   $lastline = "";
   sleep(5);
   // if (is_resource($process)) {
//        while ($s = fgets($pipes[1])) {
//            $resp = sprintf("bobProgress.%s", $s);
//            $lastline = $s;
//            debugPrint($resp);
//            //error_log($resp);
//            echo $resp;
//            flush();
//            $count++;
//        }
//    }
	$running = shell_exec( "ps aux| grep [f]wHandler");
	//error_log("is running?" . $running);
	while(strpos($running , "fwHandler") != FALSE)
	{
		sleep(1);
		$running = shell_exec( "ps aux| grep [f]wHandler");
	//		error_log("is running?" . $running);
		if(strpos($running, "fwHandler") == FALSE)
			break; //proc is done
		$progress = getProgressOfUpgrade();
		
	//	error_log("progress is " .$progress);
		if($progress > 90)
			break;
		
		echo "upgradeProgress|" . $progress;
	}
$lastline = shell_exec("tail -n 5 /home/updateFw1.log");
	
    //error_log("finished icm upgrade last line is ".$lastline);
    if (strpos($lastline , "STATUS:UNSUCCESSFUL") != FALSE)
        return FALSE;
    else
// DMO ATH-1929 this test and text return does nothing
// swithed over to returning a bool
//   if((strpos($lastline , "STATUS:SUCCESS") != FALSE || strpos($lastline, "time:")!= FALSE))
        return TRUE;//DMO orig. code assumed 'success'?  "ICM program done (can't tell if its actually successful!!!)";
}

function setFlag($strFlagName)
{
    $testfile = fopen($strFlagName, "w");
    fwrite($testfile, "updating!");
    fclose($testfile);

}
function clearFlag($strFlagName)
{
    unlink($strFlagName);
}
///Clear the log when printing with this one.  
   function debugPrintClearLog($strPrintMe)
{
     $strCmd = sprintf("echo -e \"%s\n\" > /home/debug.txt", $strPrintMe);
    $strResult2 = shell_exec($strCmd);
}
//appends to the current log
   function debugPrint($strPrintMe)
{
     $strCmd = sprintf("echo -e \"%s\n\" >> /home/debug.txt", $strPrintMe);
    $strResult2 = shell_exec($strCmd);
}
 //error_log("begin upgrade");
if (isset($_FILES["0"]) && $_FILES["0"]["error"] == UPLOAD_ERR_OK)
{
    ############ Edit settings ##############
    $UploadDirectory    = '/home/'; //specify upload directory ends with / (slash)
    ##########################################
   
    /*
    Note : You will run into errors or blank page if "memory_limit" or "upload_max_filesize" is set to low in "php.ini".
    Open "php.ini" file, and search for "memory_limit" or "upload_max_filesize" limit
    and set them adequately, also check "post_max_size".
    */
   
    //check if this is an ajax request
    if (!isset($_SERVER['HTTP_X_REQUESTED_WITH'])){
        die("|Not an Ajax Request!");
    }
   
   
    //allowed file type Server side check
    switch(strtolower($_FILES['0']['type']))
        {
            //allowed file types
            case 'application/x-zip-compressed':
            case 'binary/octet-stream':
            case 'application/octet-stream':
            case 'application/x-tar':

                break;
            default:
                die("|101"); //output error
    }
    
    setFlag("/home/updateInProgress.txt");
    setFlag("/home/slaveNotUpdatedYet.txt");
    clearFlag("/home/restoreSoftware.txt");
    // This file contains both our image tar and its md5 file...
    $tarFileName = $_FILES['0']['name']; //uploaded tar filename

    // Handle the auto case...
    if ($tarFileName == "blob")
        $tarFileName = $_POST['packagefilename'];
    else
        $bManualUpgrade = TRUE;

    $tartempFileName    = $_FILES['0']['tmp_name'];

    $strFirmwareDir = $UploadDirectory."firmware";
    debugPrintClearLog("upgrade started");

     debugPrint(sprintf("Manual upgrade1? %d", $bManualUpgrade));
    if ($bManualUpgrade)
    {
        $bUpgradeNano = FALSE;
        $bUpgradeBob = FALSE;
    }
    else
    {
        if (isset($_POST['nanofilename']))
        {
            $nanoFilename = $_POST['nanofilename'];

            // Double check for automated upgrade and only upgrade if newer
            if (CompareVersions($upgradexml->currentnanoversion, $upgradexml->nanoversion) > 0)
               $bUpgradeNano = TRUE;
        }
        
        if (isset($_POST['bobfilename']))
        {
            $bobFilename = $_POST['bobfilename'];
             $currentBobVersion = getCurrentBobVersion();

             debugPrint(sprintf("bob version current vs upgrade auto: %s ... %s \n",$currentBobVersion,$upgradexml->bobversion ));

            if (CompareVersions($currentBobVersion, $upgradexml->bobversion) > 0)
                $bUpgradeBob = TRUE;

             debugPrint(sprintf("bob compare version result is : %d\n",$bUpgradeBob ));
        }
    }

   $tlsEnabled = (  $eyeLockINI->Eyelock_TLSEnable == "false")?(0):(1);//$eyeLockINI->Eyelock_TLSEnable;

    // PHP stores the file in a tmp folder under a different name... We need to move the file to our 'Upgrade' directory with the
    // correct filename...   
  
    if (MoveAndUnpackUpdateFile($tartempFileName, $strFirmwareDir, $tarFileName))
    {
        
        //extend our cookie to 15 minutes for the update to complete...  don't want to refresh back to the login screen after the update... well
        //maybe that would solve half of our UI issues anyway but let's not do that.
      

        // Load our existing stuff so that after the upgrade we can restore
        // our settings...
        $eyeLockINI = new INIEditor("/home/root/Eyelock.ini");
        $eyeLockINI->LoadIniSettings(true);

         //upload complete... clear the timestamps.. script clears these, ignore them
    // unlink("/home/nanoupdate.txt");
    // unlink("/home/bobupdate.txt");

           $eyeLockINI->Eyelock_SoftwareUpdateDateNano = "";
           $eyeLockINI->Eyelock_SoftwareUpdateDateBob = "";

          

        debugPrint(sprintf("Manual upgrade2? %d", $bManualUpgrade));
        // We need to manually extract the nano and/or bob filenames from the xml file...
        if ($bManualUpgrade)
        {
            // Open the XML file and read the bob and nano filename fields...
			if($eyeLockINI->HardwareType === '0')
				$XMLFilename = $strFirmwareDir."/NanoNXTVersionInfo.xml";
			else if ($eyeLockINI->HardwareType === '1')
				$XMLFilename = $strFirmwareDir."/NanoEXTVersionInfo.xml";
			else
            	$XMLFilename = $strFirmwareDir."/HBoxVersionInfo.xml";
            if (file_exists($XMLFilename))
            {
                $upgradexml = simplexml_load_file($XMLFilename);

                $nanoFilename = $upgradexml->nanofilename;
                $bobFilename = $upgradexml->bobfilename;

                // For manual upgrades, always do the upgrade unless the installed version is a match...
                if (($nanoFilename != "") && file_exists($strFirmwareDir."/".$nanoFilename))
                {
                    // Check to see if we can upgrade this one..
                    //if (CompareVersions($upgradexml->currentnanoversion, $upgradexml->nanoversion) != 0) //apply if it isn't the current version
                         $bUpgradeNano = TRUE;
                }

                if (($bobFilename != "") && file_exists($strFirmwareDir."/".$bobFilename))
                {
                    //if (!isset($_POST['cbForceICMUpgrade']))
                    $currentBobVersion = getCurrentBobVersion();
                   ////  debugPrint(sprintf("bob version current vs upgrade manual: %s ... %s \n",$currentBobVersion,$upgradexml->bobversion ));
                    if (CompareVersions($currentBobVersion, $upgradexml->bobversion) != 0) //apply if it isn't the current version
                        $bUpgradeBob = TRUE;

                      //debugPrint(sprintf("bob compare version result is : %d\n",$bUpgradeBob ));
                }
            }
            else if(file_exists($strFirmwareDir."/Patch.xml"))
            {
            	$patchxml = simplexml_load_file($strFirmwareDir."/Patch.xml");
            	$patchfilename = $patchxml->patchfilename;
            	if (($patchfilename != "") && file_exists($strFirmwareDir."/".$patchfilename))
                {
                	echo '|11';
                	if(ValidateUploadedFiles($strFirmwareDir, $patchfilename))
                	{
                		echo '|12'; 
	                	if(copyAndUnpackLinuxPatch($strFirmwareDir, $patchfilename))
	                	{
	                		echo '|13';
	                		if(runLinuxPatch($strFirmwareDir, "master"))
	                		{
								if($eyeLockINI->HardwareType === '0')
								{
									if(runLinuxPatch($strFirmwareDir, "slave"))
										echo '|14';
									else
									{
										cleanUpLinuxPatchFiles($strFirmwareDir);
												die("|111");
									}
								}
	                		}
	                		else
							{
								cleanUpLinuxPatchFiles($strFirmwareDir);
										die("|111");
							}
	                	}
	                	else
	                	{
							cleanUpLinuxPatchFiles($strFirmwareDir);
	                		die("|102");
	                	}
	                }
	                else
	                {
				cleanUpLinuxPatchFiles($strFirmwareDir);
	                	die("|103");
	                }
                }
                else
                {
                	cleanUpLinuxPatchFiles($strFirmwareDir);
                	die("|103");
                }
		  		 cleanUpLinuxPatchFiles($strFirmwareDir);
            	  sleep(2);
            	  die('|success');    
            }
            else
            {
                CleanupFiles($strFirmwareDir);
                die("|109");
            }
        }

        echo '|2';
         setFlag("/home/firmwareupdate.txt");
        // Upgrading Nano firmware
		//new 11/3/2016, the SDK upgrade script takes it from here.
		RunSDKUpgrade();
        if ($bUpgradeNano && false) //disabled code section
        {
            // Before doing a thing, we must validate the uploaded tar and associated md5 files...
            // First the master...
            if (ValidateUploadedFiles($strFirmwareDir, $nanoFilename))
            {
                // Now the slave
				//CMX:  The file for the slave should always be there, if we use the same firmware file for NXT and CMX.  I hope we do eventually split it though
				//this will be a lot of juggling and complexity that we just don't need.  -HESTER
				//this is done this way so that we don't assume the slave file exists.  I antiicapte that we'll split the firmware files.  There is too
				//much difference in the compilation to allow the same build to run on the NXt and the CMX at the same time.
				$validateSlave = true;
				$nanoSlaveFilename = str_replace("Master", "Slave", $nanoFilename);
				if($eyeLockINI->HardwareType === '0')
				{
                	
					$validateSlave = ValidateUploadedFiles($strFirmwareDir, $nanoSlaveFilename);
				}
				
                if ($validateSlave)
                {
                    echo '|3'; // files validated... copying files to slave device
                    
                    $cmd = "50";//'/home/root/nano_led 100 80 0 1';
                    NXTW_shell_exec($cmd);
						$extractResult = ExtractMasterFirmware("/home", $strFirmwareDir, $nanoFilename); //performs the unzip and unencrypt parts, and extracts to upgradeTemp
						//alos checks the version
						if($extractResult === "106")
						{
							  CleanupFiles($strFirmwareDir);
							  
							die("|106");	
						}
						if($extractResult === "166") //failed because of version
						{
							  CleanupFiles($strFirmwareDir);
							  
							die("|166");
						}
					//die("|666"); //protective measure for downgrade tests
				
					if($eyeLockINI->HardwareType === '0') //Only push the slave file if its an NXT
						
					   {                    // Ok, now before we do another thing make sure that we can copy the file to the slave...
							if (!CopySlaveFileToSlaveDevice($strFirmwareDir, $nanoSlaveFilename))
							{
								CleanupFiles($strFirmwareDir);
								die("|110"); // Failed to copy file to slave device... aborts upgrade...
							}
							else
								echo '|4';
					   }
					
                    // Do the Master stuff...
                    // Ok, now we need to create our restore point, then upgrade the firmware...
                    // Creates restore points for both master and slave...
                     setFlag("/home/createrestorepoint.txt");
                    if (CreateRestorePoint("/home", "root", $strFirmwareDir."/nano/restorepoints", ConstructRestorePointFilename(), "www"))
                    {
                       
                        clearFlag("/home/createrestorepoint.txt");
                        echo '|5';

                        KillApplication(); // Stop the Eyelock app, before the upgrade...

                        // DMOTODO -- change this to just "/home" when we're ready to go.
                        if (UpgradeMasterFirmware("/home", $strFirmwareDir, $nanoFilename))
                        {
                            if(!commitMasterFirmwareUpgrade())
                            {
                                 CleanupFiles($strFirmwareDir);
                                die ("|104"); //give a different error message this time

                            }
                            // Now do the Slave stuff...
							if($eyeLockINI->HardwareType === '0')
							{
								 setFlag("/home/slaveUpdating.txt");
								 clearFlag("/home/slaveNotUpdatedYet.txt");
								if (!UpgradeSlaveFirmware("/home", $strFirmwareDir, $nanoSlaveFilename))
								{
									CleanupFiles($strFirmwareDir);
									die("|111");
								}
								setFlag("/home/slaveUpdated.txt");
								clearFlag("/home/slaveUpdating.txt");
								}
                            echo '|6';
							

                            // Make sure that existing settings are retained, and start things up...
                            if (!UpgradePostProcess($eyeLockINI, $bUpgradeNano, $bUpgradeBob))
                            {
                                CleanupFiles($strFirmwareDir);
                                die("|107");
                            }
                        }
                        else
                        {
                            CleanupFiles($strFirmwareDir);
                            die("|106");
                        }
                    }
                    else
                    {
                        CleanupFiles($strFirmwareDir);
                        die("|105");
                    }
                }
                else
                {
                    CleanupFiles($strFirmwareDir);
                    die ("|104");
                }
            }
            else
            {
                CleanupFiles($strFirmwareDir);
                die("|103");
            }
        }
         clearFlag("/home/firmwareupdate.txt");

        // Upgrading Bob firmware
        if ($bUpgradeBob && false)
        {
   // no 7 for now         echo '|7';

            // No bob file validation for now.
          // if (ValidateUploadedFiles($strFirmwareDir, $bobFilename))
          //  {
// no 8 for now                 echo '|8';

                $strFullLengthFileName = sprintf("%s/%s",$strFirmwareDir, $bobFilename );
                debugPrint($strFullLengthFileName);
           if(scriptExist($strFullLengthFileName))
           {
                echo '|9';
                debugPrint("file existed");
                if (!UpgradeBobFirmware($strFirmwareDir, $bobFilename))
                {
                    //CleanupFiles($strFirmwareDir);
                    die("|108");
                }
                else
                {
                    // Make sure that we write our 'upgrade timestamp' if it wasn't alredy written as part of the nano upgrade...
                    if (!$bUpgradeNano) // This will have already been done if the nano firmware was upgraded...
                    {
                        // this can fail and we really don't care... all it does is write our timestamp...
                        UpgradePostProcess($eyeLockINI, $bUpgradeNano, $bUpgradeBob);
                    }
                }
            }
            else
            {
                debugPrint("file didn't exist");
                die("|108");
                  
            }
                
        }
		if(false)
		{
			copyFirmwaretoRoot($strFirmwareDir, $bobFilename);
	
			
			CleanupFiles($strFirmwareDir);
	
			if ($bUpgradeNano)
			{
				echo '|10';
				// Now we need to start the app, etc...
				UpgradePostProcessSettingsChange();
			}
			  clearFlag("/home/slaveUpdated.txt");
			clearFlag("/home/updateInProgress.txt");
	//		   error_log("setTLSOnlyWebConfig>".$tlsEnabled);
			$eyeLockINI->setTLSOnlyWebConfig($tlsEnabled); //preserve the TLS mode on the web config 
			
			echo '|success';  //echo the success and die after reboot, to ensure the success is received.
	//        die('|success');
			PostProcessRebootDevice();
			die;
		}
    }
    else
    {
        die("|102");
    }
}
else
{
    die('|Something wrong with upload! Is "upload_max_filesize" set correctly?');
}

function RunSDKUpgrade()
{
	/* 	• Decrypt the fwhandler.tar.gz file
	• Unzip the fwhandler.tar.gz file (tar xvf)
	• Configure the script to write to a file   (./MultiChannelLogger -f"/home/updateFw1.log" -S
	• Run the script
	• Track the progress through the NXTW app.
*/
 echo '|5';
	NXTW_shell_exec("401"); // /home/root/KeyMgr -d -i fwHandler.tar.gz -o /home/firmware/fwHandlerExc.tar.gz
    if (!getnxtWResult()) // If the Decrypt fails, we fail the upgrade... necessary for security see ATH-1929
    {
   		die("|102");
    }
    else
    { 
	    NXTW_shell_exec("402"); // tar xvf fwHandlerExc.tar.gz
	    NXTW_shell_exec("405");  // chmod 777 /home/firmware/fwHandler/*
	    NXTW_shell_exec("403");  // /home/firmware/fwHandler/MultiChannelLogger -f"/home/updateFw1.log" -S
	    //NXTW_shell_exec("404");  // /home/firmware/fwHandler/fwHandler.sh upgrade
	    if (!updateProgress("404"))
            die("|102");
        else
        {
	        echo "|success";
    	    die;
        }
    }
}


// If '0' does not exist in the result file, it's a failure
function getnxtWResult()
{
    $resultFile = fopen("/home/nxtwResult","r");
	$resultText = fgets($resultFile);
	fclose($resultFile);

    $pos = strpos($resultText, "STATUS:DECRYPTFAILURE");
    if ($pos === FALSE)
        return TRUE;
    else
        return FALSE;
}


function UpgradePostProcessSettingsChange()
    {
        // Creates the Eyelock.run file if it does not already exist...
		NXTW_shell_exec("201"); // chown -R www:www /tmp/*
	//	error_log("cleanup root folder");
		shell_exec("rm /home/root/*.gz");
        CreateEyelockRun();
        RestartApplication();     
    }
function PostProcessRebootDevice()
	{
	//	error_log("postprocreboot");
		$apprunning = shell_exec("ps -A | grep nxtW");
	//	error_log("app running? " . $apprunning);
		shell_exec("touch /home/nxtW.run");
		sleep(15); 
		
		NXTW_shell_exec("38");//
            NXTW_shell_exec("39");//"/home/www/scripts/KillApplication.sh");
	  			 NXTW_shell_exec("206");
            NXTW_shell_exec("40");//
            NXTW_shell_exec("41");//'i2cset -y 3 0x2e 4 8'); // reset command to motherboard if the above command fails
            NXTW_shell_exec("29");//'reboot'); //third and one more step to ensure reboot
			
			//shell_exec('ssh root@192.168.40.2 reboot');
		  	shell_exec("chmod 777 /home/www/scripts/KillApplication.sh");
			shell_exec('i2cset -y 3 0x2e 4 7'); //enable watchdog to force the reboot
			shell_exec('i2cset -y 3 0x2e 4 8'); // reset command to motherboard if the above command fails
			shell_exec('reboot'); 
          
			 
	}
function scriptExist($filename)
{
    NXTW_shell_exec(sprintf("510"));
	 $strCmd = sprintf("51".chr(0x1F)."%s", $filename);//"xx /home/www/scripts/testExist.sh \"%s\"", $filename);
     debugPrint($strCmd);
    $strResult = NXTW_shell_exec($strCmd);
      debugPrint($strResult);
       $pos = strpos($strResult, "1");
    if ($pos !== 0)
        return TRUE;
        else
    return FALSE;

}


/////////////////////////////////////////////////////
// Backup Current System
/////////////////////////////////////////////////////

function saveDB()
{
  //error_log("saveDB()");
      $strCmd = sprintf("mv %s/%s %s/%s", "/home/root", "test.db", "/home/root", "test.db.upgrade");
    $strResult = debug_shell_exec($strCmd);
    $strCmd = sprintf("mv %s/%s %s/%s", "/home/root/rootCert/certs", "nanoNXTDefault.crt", "/home/root/rootCert/certs", "nanoNXTDefault.crt.upgrade");
    $strResult = debug_shell_exec($strCmd);
    $strCmd = sprintf("mv %s/%s %s/%s", "/home/root/rootCert/certs", "nanoNXTDefault.key", "/home/root/rootCert/certs", "nanoNXTDefault.key.upgrade");
    $strResult = debug_shell_exec($strCmd);
    $strCmd = sprintf("mv %s/%s %s/%s", "/home/root", "keys.db", "/home/root", "keys.db.upgrade");
    $strResult = debug_shell_exec($strCmd);
}

function commitMasterFirmwareUpgrade()
{
     //error_log("commitMasterFirmwareUpgrade()");
     debug_shell_exec("cp /home/root/*.log /home/upgradeTemp/root");
   debug_shell_exec("chmod 777 /home/upgradeTemp/root/Eyelock");
   $eyelockTestResult = shell_exec("/home/upgradeTemp/root/Eyelock -v");
    debugPrint($eyelockTestResult);
     if(strpos($eyelockTestResult, "Eyelock") == FALSE)
   return FALSE; //test version failed, result does not contain the eyelock version


//perform the folder swap.  Last step is to move the swap script into the home folder so it can operate
       $strResult = debug_shell_exec("cp /home/upgradeTemp/www/swapFolders /home");
        $strResult = debug_shell_exec("chmod 777 /home/swapFolders");
       $strResult = debug_shell_exec("/home/swapFolders /home/upgradeTemp/root /home/root");
       $strResult = debug_shell_exec("/home/swapFolders /home/upgradeTemp/www /home/www");
        $strResult = debug_shell_exec("/home/swapFolders /home/upgradeTemp/default /home/default");
          $strResult = debug_shell_exec("/home/swapFolders /home/upgradeTemp/ext_libs /home/ext_libs");
         // $strResult = debug_shell_exec("/home/swapFolders /home/upgradeTemp/backup /home/backup");
          $strResult = debug_shell_exec("/home/swapFolders /home/upgradeTemp/factory /home/factory");
             $strResult = debug_shell_exec("cp /home/upgradeTemp/php /home/php");
       return TRUE;
}

/////////////////////////////////////////////////////
// Install the new firmware (untar it)
/////////////////////////////////////////////////////
function ExtractMasterFirmware($strUpdateLocationParent, $strUpgradeFileDirectory, $strUpgradeFilename)
{
	 $strTarFilename = substr($strUpgradeFilename, 0, $strUpgradeFilename.length-3); // lop off the .gz extension
   // DMOTODO put this in a script? to handle errors... or break up to handle errors...
   $strCmd = sprintf("52".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s", 
   escapeshellarg($strUpdateLocationParent),
   escapeshellarg($strUpgradeFileDirectory.'/'.$strUpgradeFilename),
   escapeshellarg($strUpgradeFileDirectory.'/out.tar.gz'),
   escapeshellarg($strUpgradeFileDirectory.'/out.tar.gz'), 
   escapeshellarg($strUpgradeFileDirectory.'/'.$strUpgradeFilename));//"cd %s;/home/root/KeyMgr -d -i %s/%s -o %s/out.tar.gz;mv %s/out.tar.gz %s/%s", 
   $strResult = NXTW_shell_exec($strCmd);
      setFlag("/home/untarpackage.txt");
      //Folder swapping!  In order to make the upgrade process as "atomic" as possible we'll extract everything to a temp folder
      //then we'll swap the folders on the drive
      // $strResult = NXTW_shell_exec("53");//"rm -rf /home/upgradeTemp");
      // $strResult = NXTW_shell_exec("54");//"mkdir /home/upgradeTemp");
	  sleep(5);
	  mkdir("/home/upgradeTemp");
		//shell_exec("mkdir /home/upgradeTemp"); //creates with www user as owner instead of root as owner
       //todo:  It's possible that htis command fails.  If it does there will be an error message somehow.  I want to capture that message.  It should appear in strResult, so i can use that
       //to send a False back, indicating that there was an unpack error.
	   shell_exec("ls");
	sleep(15);
    $strCmd = sprintf("cd %s;gzip -d %s", escapeshellarg($strUpdateLocationParent), escapeshellarg($strUpgradeFileDirectory.'/'.$strUpgradeFilename));
    $strResult = debug_shell_exec($strCmd);
     sleep(3);
     $strCmd = sprintf("tar xf %s -C /home/upgradeTemp",  escapeshellarg($strUpgradeFileDirectory.'/'.$strTarFilename));
    //$strResult = debug_shell_exec($strCmd);

    exec($strCmd, $outoutArray, $returnCode);

    //error_log("untar fw result is ".$returnCode);
    if($returnCode != 0)
        return "106";
		
	if(! MinimumVersionUpgradeTemp()) //is upgrade lower than minimum version
		 return "166";
		
		return "0";	
}


function UpgradeMasterFirmware($strUpdateLocationParent, $strUpgradeFileDirectory, $strUpgradeFilename)
{

    //todo:  remove the time file

    //error_log("UpgradeMasterFirmware()");


    $strTarFilename = substr($strUpgradeFilename, 0, $strUpgradeFilename.length-3); // lop off the .gz extension

    // Store the settings, they'll be replaced later...
//    $strCmd = sprintf("mv %s/%s %s/%s", "/home/root", "Eyelock.ini", "/home/root", "Eyelock.ini.upgrade");
//    $strResult = debug_shell_exec($strCmd);

    // Before untarring the restore point, we need to store off the db... we don't blow it away (only worry about it on the master)...
  


 
		
		// saveDB();
      clearFlag("/home/untarpackage.txt");
	  //check the version
	
     setFlag("/home/runPatch.txt");
     if(file_exists("/home/upgradeTemp/root/patch.sh"))
     {
         //error_log("found patch, running it");
         debugPrint("Found patch, running it");
         NXTW_shell_exec("55");//"chmod 777 /home/upgradeTemp/root/patch.sh");
         NXTW_shell_exec("56");//"/home/upgradeTemp/root/patch.sh");


     }
     
     clearFlag("/home/runPatch.txt");
       $strResult = debug_shell_exec("mkdir /home/upgradeTemp/root/rootCert");
       $strResult = debug_shell_exec("mkdir /home/upgradeTemp/root/rootCert/certs");

      NXTW_shell_exec("57");//"rm /home/upgradeTemp/root/test.db");
      NXTW_shell_exec("58");//"rm /home/upgradeTemp/root/keys.db");

      //error_log("restoring backups");
       //restore the backups before we swap folders!
       $strCmd = sprintf("cp /home/root/test.db /home/upgradeTemp/root/test.db");
    $strResult = debug_shell_exec($strCmd);
    $strCmd = sprintf("cp /home/root/rootCert/certs/nanoNXTDefault.crt /home/upgradeTemp/root/rootCert/certs/nanoNXTDefault.crt");
    $strResult = debug_shell_exec($strCmd);
    $strCmd = sprintf("cp /home/root/rootCert/certs/nanoNXTDefault.key /home/upgradeTemp/root/rootCert/certs/nanoNXTDefault.key");
    $strResult = debug_shell_exec($strCmd);
    $strCmd = sprintf("cp /home/root/keys.db /home/upgradeTemp/root/keys.db");
    $strResult = debug_shell_exec($strCmd);
   
     $strResult = debug_shell_exec("cp /home/root/rootCert/certs/*.* /home/upgradeTemp/root/rootCert/certs");
     
     
      //$strResult = debug_shell_exec("cp /home/upgradeTemp/www/ssllibs/libssl.so.0.9.8 /lib");
       //$strResult = debug_shell_exec("cp /home/upgradeTemp/www/ssllibs/libcrypto.so.0.9.8 /lib");
        //$strResult = debug_shell_exec("cp /home/upgradeTemp/www/ssllibs/openssl /usr/sbin");
  
      // $strCmd = sprintf("cp /home/upgradeTemp/root/*.db /home/root/");
   // $strResult = debug_shell_exec($strCmd);

      $strCmd = sprintf("cp /home/root/id.txt /home/upgradeTemp/root/id.txt");
    $strResult = debug_shell_exec($strCmd);
    
     $strCmd = sprintf("cp /home/root/MAC.txt /home/upgradeTemp/root/MAC.txt");
    $strResult = debug_shell_exec($strCmd);

     // $strCmd = sprintf("mv /home/root/mac.txt /home/upgradeTemp/root/mac.txt");
   // $strResult = debug_shell_exec($strCmd);

  //  $strCmd = sprintf("cp /home/upgradeTemp/root/rootCert/certs/nanoNXTDefault.* /home/root/rootCert/certs");
  //  $strResult = debug_shell_exec($strCmd);
    
   //error_log("ready to swap");

    // Restore the database
   // $strCmd = sprintf("mv %s/%s %s/%s", "/home/root", "test.db.upgrade", "/home/root", "test.db");
  //  $strResult = debug_shell_exec($strCmd);
   // $strCmd = sprintf("mv %s/%s %s/%s", "/home/root/rootCert/certs", "nanoNXTDefault.crt.upgrade", "/home/root/rootCert/certs", "nanoNXTDefault.crt");
   // $strResult = debug_shell_exec($strCmd);
  //  $strCmd = sprintf("mv %s/%s %s/%s", "/home/root/rootCert/certs", "nanoNXTDefault.key.upgrade", "/home/root/rootCert/certs", "nanoNXTDefault.key");
  ////  $strResult = debug_shell_exec($strCmd);
  //  $strCmd = sprintf("mv %s/%s %s/%s", "/home/root", "keys.db.upgrade", "/home/root", "keys.db");
   // $strResult = debug_shell_exec($strCmd);

    // Ok, at this point we can assume that we have installed the new nano firmware...
    // Now we need to do it over on the slave device...
   // $strCmd = sprintf("cd %s;rm -rf %s;tar xf %s/%s", $strUpdateLocationParent, $strUpdateLocation, $strUpgradeFileDirectory, $strUpgradeFilename);
   // $strResult = debug_shell_exec($strCmd);
    // Copy file to remote system and execute some commands...
  //  $ cat sayhi.sh | ssh myserver 'cat > ./remotehi.sh ; chmod +x ./remotehi.sh ; ./remotehi.sh'
  //  hello, world!



/**
    // assume success and look for eyestartup
    $strResult = debug_shell_exec("[ -e /home/root/eyestartup ] && echo 0 || echo 1");

    if ($strResult !== "0\n") //root related files existing
    {
        $strResult = debug_shell_exec("rm -rf /etc/init.d/eyestartup;rm -rf /etc/rc5.d/S95eyestartup;ln -sf /home/root/startup.sh /etc/rc5.d/S95Startup;sync");
    }
    else
    {
        $strResult = debug_shell_exec("sh /home/root/scripts/install.sh");
    }
**/ 
    return true;
}

function UpgradeSlaveFirmware($strUpdateLocationParent, $strUpgradeFileDirectory, $strUpgradeFilename)
{
    //error_log("UpgradeSlaveFirmware()");
    // Make sure the update directory exists...
	if($eyeLockINI->HardwareType !== '0') return TRUE; //we skip this for CMX.  If we managed to call this, retrn true without doing anything.
	
	
    $strCmd = sprintf("1330".chr(0x1F)."%s",  escapeshellarg(sprintf("mkdir %s", $strUpdateLocationParent))); //sprintf("1330 root@192.168.40.2 %s",
    $strResult = NXTW_shell_exec($strCmd); 


   
    $strTarFilename = substr($strUpgradeFilename, 0, $strUpgradeFilename.length-3); // lop off the .gz extension

	//copy the Eyelock.ini to a backup spot
	$strCmd = sprintf("1331"); //sprintf("ssh root@192.168.40.2 'cp /home/root/Eyelock.ini /home/EyelockBak.ini'");
	$strResult = NXTW_shell_exec($strCmd); 
	
	//trigger the upgrade
    // File should already be unzipped for us on the slave...
    //sprintf("ssh root@192.168.40.2 'cd %s;/home/root/KeyMgr -d -i %s -o %s/out.tar.gz;mv %s/out.tar.gz %s'", 
    $strCmd = sprintf("1332".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s",
    escapeshellarg($strUpdateLocationParent),
	escapeshellarg($strUpgradeFileDirectory.'/'.$strUpgradeFilename),
   
    escapeshellarg($strUpgradeFileDirectory),
    escapeshellarg($strUpgradeFileDirectory), 
    escapeshellarg($strUpgradeFileDirectory.'/'.$strUpgradeFilename));
    $strResult = NXTW_shell_exec($strCmd);
    
    //$strCmd = sprintf("ssh root@192.168.40.2 'cd %s;gzip -d %s;tar xf %s'", 
    $strCmd = sprintf("1333".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s", 
	escapeshellarg($strUpdateLocationParent), 
	 escapeshellarg($strUpgradeFileDirectory.'/'.$strUpgradeFilename),
	  escapeshellarg($strUpgradeFileDirectory.'/'.$strTarFilename));
    $strResult = NXTW_shell_exec($strCmd); 
	//
//    $strCmd = sprintf("chmod 777 /home/www/scripts/merge.sh");
//	$strResult = debug_shell_exec($strCmd);
//
//	//upload the merge script
//	$strCmd = sprintf("scp /home/www/scripts/merge.sh root@192.168.40.2:/home/root/merge.sh");
//	$strResult = debug_shell_exec($strCmd);
//	
//	//Copy the ini back for merging
//	$strCmd = sprintf("ssh root@192.168.40.2 'cp /home/EyelockBak.ini /home/root/EyelockBak.ini'"); 
//	$strResult = debug_shell_exec($strCmd); 
//	
//    //trigger the merge
//	$strCmd = sprintf("ssh root@192.168.40.2 'chmod 777 /home/root/merge.sh'"); 
//	$strResult = debug_shell_exec($strCmd); 
//
//	//trigger the merge
//	$strCmd = sprintf("ssh root@192.168.40.2 '/home/root/merge.sh'"); 
//	$strResult = debug_shell_exec($strCmd); 
	NXTW_shell_exec("1334");



    return TRUE;
}

function copyFirmwaretoRoot($strUpgradeFileDirectory, $strUpgradeFilename)
{
     $strCmd = sprintf("cp %s %s", escapeshellarg($strUpgradeFileDirectory.'/'.$strUpgradeFilename), escapeshellarg('/home/root/'.$strUpgradeFilename));
     $strResult4 = debug_shell_exec($strCmd);

}

function runBobFirmwareUpgrade($strUpgradeFileDirectory, $strUpgradeFilename, $icmCommunicatorPath, $icmCommunicatorName, $timeout)
{
	NXTW_shell_exec("38");//
    NXTW_shell_exec("39");//"/home/www/scripts/KillApplication.sh"); //this will be the newly updated version of this btw.  We can use this to ensure there is no
	//reboot during the ICM update
			
	sleep(5);
    SetExecPrivileges($icmCommunicatorPath,  $icmCommunicatorName);
	chmod("/home/root/icm_communicator" ,0777);
	chmod("/home/upgradeTemp/root/icm_communicator" ,0777);
    $strCmd = sprintf( "999".chr(0x1F)."%s".chr(0x1F)."%s ",$timeout,  escapeshellarg($strUpgradeFileDirectory."/".$strUpgradeFilename)) ;
    //$strResult = debug_shell_exec($strCmd);
    //error_log("running ".$strCmd);
    $strResult = icmProgress($strCmd);
    $strResult3 = shell_exec("rm /home/bobUp.txt");


    $strCmd = sprintf("echo -e \"%s\" >> /home/bobUp.txt", $strResult);
    $strResult2 = debug_shell_exec($strCmd);

  

    
    $pos = strpos($strResult, "ICM program done (can't tell if its actually successful!!!)");
    $pos2 = strpos($strResult, "time:");
    //error_log(sprintf("one or two: %d  %d", $pos, $pos2));
    if ($pos !== false || $pos2 !== false)
    {
         clearFlag("/home/icmupdate.txt");
        return TRUE;}
    else
        return FALSE;

}

function UpgradeBobFirmware($strUpgradeFileDirectory, $strUpgradeFilename)
{
    //error_log("upgradeBobFirmware()");
     setFlag("/home/icmupdate.txt");
    // Execute icm_connector with the file option... to start the upgrade
   // SetExecPrivileges("/home/upgradeTemp/root/", "icm_communicator");

    //$strCmd = sprintf("/home/upgradeTemp/root/icm_communicator -p %s/%s", $strUpgradeFileDirectory, $strUpgradeFilename);
    //$strResult = debug_shell_exec($strCmd);
    
    //$strResult = icmProgress($strCmd);
   // $strResult3 = shell_exec("rm /home/bobUp.txt");


    //$strCmd = sprintf("echo -e \"%s\" >> /home/bobUp.txt", $strResult);
    //$strResult2 = debug_shell_exec($strCmd);

  $result = runBobFirmwareUpgrade($strUpgradeFileDirectory, $strUpgradeFilename, "/home/upgradeTemp/root/", "icm_communicator", "30"); //default
  if($result)
    return TRUE;
    //error_log("Failed with 30, try 20");
    $result = runBobFirmwareUpgrade($strUpgradeFileDirectory, $strUpgradeFilename, "/home/upgradeTemp/root/", "icm_communicator", "20"); //default
  if($result)
    return TRUE;
     //error_log("Failed with 20, try 40");
    $result = runBobFirmwareUpgrade($strUpgradeFileDirectory, $strUpgradeFilename, "/home/upgradeTemp/root/", "icm_communicator", "40"); //default
  if($result)
    return TRUE;
     //error_log("Failed with 40, full fail");
    return FALSE;

}


function copyAndUnpackLinuxPatch($strPatchFileDir, $strPatchFilename)
{
    $tmpPatchDir = $strPatchFileDir.'/patch';
    $strTarFilename = substr($strPatchFilename, 0, $strPatchFilename.length-3);   //remove .gz extn of file
	
    $strCmd = sprintf("mkdir -p %s;mv %s %s",  escapeshellarg($tmpPatchDir), escapeshellarg($strPatchFileDir.'/'.$strPatchFilename), escapeshellarg($tmpPatchDir.'/'.$strPatchFilename));
    $strResult = debug_shell_exec($strCmd);
	
    $strCmd = sprintf("cd %s;/home/root/KeyMgr -d -i %s -o out.tar.gz;mv out.tar.gz %s", escapeshellarg($tmpPatchDir), escapeshellarg($strPatchFilename), escapeshellarg($strPatchFilename));
    $strResult = debug_shell_exec($strCmd);
    
    $strCmd = sprintf("cd %s;gzip -d %s;tar xf %s", escapeshellarg($tmpPatchDir), escapeshellarg($strPatchFilename), escapeshellarg($strTarFilename));
    $strResult = debug_shell_exec($strCmd);
    
    // Copy decrypted tar to slave...
    //$strCmd = sprintf("ssh root@192.168.40.2 'mkdir -p %s'", $tmpPatchDir);
	
	$strCmd = sprintf("208".chr(0x1F)."%s",escapeshellarg(sprintf("mkdir -p %s", $tmpPatchDir)));
	//"cd %s;chmod a+x install.sh;sh install.sh master update", $tmpPatchDir);
	if($eyeLockINI->HardwareType === '0')
		$strResult = NXTW_shell_exec($strCmd); 
	
   // $strCmd = sprintf("ssh root@192.168.40.2 %s", escapeshellarg(sprintf("mkdir -p %s", $tmpPatchDir)));
    //$strResult = debug_shell_exec($strCmd); 
    
	$strCmd = "209".chr(0x1F).escapeshellarg($tmpPatchDir.'/'.$strTarFilename).chr(0x1F).$tmpPatchDir.chr(0x1F).$strTarFilename;
	//"cd %s;chmod a+x install.sh;sh install.sh master update", $tmpPatchDir);
	if($eyeLockINI->HardwareType === '0')
		$strResult = NXTW_shell_exec($strCmd); 
	
  //  $strCmd = sprintf("cat %s | ssh root@192.168.40.2 'cat > %s/%s'", escapeshellarg($tmpPatchDir.'/'.$strTarFilename), $tmpPatchDir, $strTarFilename);
   // $strResult = debug_shell_exec($strCmd); 
        
		$strCmd = "210".chr(0x1F).escapeshellarg($tmpPatchDir).chr(0x1F).escapeshellarg($strTarFilename);
	//"cd %s;chmod a+x install.sh;sh install.sh master update", $tmpPatchDir);
	if($eyeLockINI->HardwareType === '0')
		$strResult = NXTW_shell_exec($strCmd); 
  //  $strCmd = sprintf("ssh root@192.168.40.2 'cd %s;tar xf %s'", $tmpPatchDir, $strTarFilename);
    //$strResult = debug_shell_exec($strCmd); 
    
    return TRUE;
}
function runLinuxPatch($strPatchFileDir, $Client)
{
	$tmpPatchDir = $strPatchFileDir.'/patch';
	$masterUpdated = $slaveUpdated = TRUE;

	if ($Client == "slave")
	{
		$strCmd = "210".chr(0x1F).escapeshellarg($tmpPatchDir);
	//"cd %s;chmod a+x install.sh;sh install.sh master update", $tmpPatchDir);
		$strResult = NXTW_shell_exec($strCmd); 
		//$strCmd = sprintf("ssh root@192.168.40.2 'cd %s;chmod a+x install.sh;sh install.sh slave update'", $tmpPatchDir);
		//$strResult = debug_shell_exec($strCmd); 
		preg_match("(SUCCESSINSTALLPATCH)", $strResult, $matches);
		if($matches[0] != "SUCCESSINSTALLPATCH")
			$slaveUpdated = FALSE;
	}
	else if ($Client == "master")
	{
		$strCmd = sprintf("59".chr(0x1F)."%s", $tmpPatchDir);//"cd %s;chmod a+x install.sh;sh install.sh master update", $tmpPatchDir);
		$strResult = NXTW_shell_exec($strCmd); 
		preg_match("(SUCCESSINSTALLPATCH)", $strResult, $matches);
		if($matches[0] != "SUCCESSINSTALLPATCH")
			$masterUpdated = $slaveUpdated = FALSE;
	}
	
	$strCmd = sprintf("60".chr(0x1F)."%s", $tmpPatchDir);//"ssh root@192.168.40.2 'cd %s;chmod a+x install.sh;sh install.sh slave restore'", $tmpPatchDir);
	if(!$slaveUpdated)
		NXTW_shell_exec($strCmd);
		
	$strCmd = sprintf("61".chr(0x1F)."%s", $tmpPatchDir);//"cd %s;chmod a+x install.sh;sh install.sh master restore", $tmpPatchDir);
	if(!$masterUpdated)
		NXTW_shell_exec($strCmd);
	
	return $masterUpdated && $slaveUpdated;
}

function cleanUpLinuxPatchFiles($strPatchFileDir)
{
	$tmpPatchDir = $strPatchFileDir.'/patch';

$strCmd = "212".chr(0x1F).escapeshellarg($tmpPatchDir);
	//"cd %s;chmod a+x install.sh;sh install.sh master update", $tmpPatchDir);
	if($eyeLockINI->HardwareType === '0')
		$strResult = NXTW_shell_exec($strCmd); 
	//$strCmd = sprintf("ssh root@192.168.40.2 'rm -R %s'", $tmpPatchDir);
	//$strResult = debug_shell_exec($strCmd); 
	
	$strCmd = sprintf("rm -R %s", $tmpPatchDir);
	$strResult = debug_shell_exec($strCmd); 
	
	$strCmd = sprintf("cd %s;rm *.*", $strPatchFileDir);
	$strResult = debug_shell_exec($strCmd); 
}


/////////////////////////////////////////////////////
// Bring previous settings forward...
/////////////////////////////////////////////////////
function UpgradePostProcess($eyeLockINI, $bNano, $bBob)
{
    if (!SetExecPrivileges("/home/root/", "Eyelock"))
    {
       // log.Debug(string.Format("error when setting exe privileges for file {0} to board {1}", "Eyelock", p_IpOfBoard.ToString()));
    }

    if (!SetExecPrivileges("/home/root/", "*.sh"))
    {
       // log.Debug(string.Format("error when setting exe privileges for file {0} to board {1}", "Eyelock", p_IpOfBoard.ToString()));
    }

    // This function restores all of the user's existing configuration information...
    $strDateTimeStamp = date('D jS M Y h:i:s A')."  (UTC)";
    $fw = fopen("/home/nanoupdate.txt", "w");
    fwrite($fw, $strDateTimeStamp);
    fclose($fw);
    $fw2 =fopen("/home/bobupdate.txt", "w");
    fwrite($fw2, $strDateTimeStamp);
    fclose($fw2);

     
    if ($bNano)
    {
        // Need this to force it to be written
        $eyeLockINI->set("Eyelock.SoftwareUpdateDateNano", $strDateTimeStamp);

        //write the update time stamp to a separate file on the home folder so it doesn't get destroyed
       
    }

    if ($bBob)
    {
        // Need this to force it to be written
        $eyeLockINI->set("Eyelock.SoftwareUpdateDateBob", $strDateTimeStamp);
        
    }

    RestoreDeviceSettings($eyeLockINI);

    //remove the update flag
    unlink("/home/updateInProgress.txt");

    return TRUE;
}




///////////////////////////////////////////////////////////////////////////
// Restore Eyelock.ini, interfaces, and reloadinterfaces file settings...
///////////////////////////////////////////////////////////////////////////
function RestoreDeviceSettings($eyeLockINI)
{
    // Copy our saved settings file back...
//    $strCmd = sprintf("mv %s/%s %s/%s", "/home/root", "Eyelock.ini.upgrade", "/home/root", "Eyelock.ini");
//    $strResult = debug_shell_exec($strCmd);

    $eyeLockINI->UpgradeIniSettings();

    sleep(2); // Allow some time for the user to read the status...
}



///////////////////////////////////////////////
// Helpers
///////////////////////////////////////////////

// Move file from directory, to directory (same filename); creates dir if not exists...


function getCurrentBobVersion()
{
    $strLogFile = "/home/root/BobVersion";
   
    $cmd = "head -n 1 {$strLogFile}";

    exec("$cmd 2>&1", $output);

    $bobFWLine = "";
    $linecount = 0;
    foreach ($output as $outputline)
    {
       $bobFWLine = $outputline;
       break;
        $linecount++;
    }
    //ICM software version: 3.1.5
    $tokens = explode(":", $bobFWLine);

    return trim($tokens[1]);

}


function MoveAndUnpackUpdateFile($strTempFilepath, $strFirmwareDirectory, $strFilename)
{
    echo '|1';

    //First, make sure our target directory exists...
    $strCmd = sprintf("mkdir -p %s", escapeshellarg($strFirmwareDirectory));
    $strResult = debug_shell_exec($strCmd);
    debugPrint($strResult);
    // Now copy/rename the temp file from its upload location into the correct destination directory...
    $strCmd = sprintf("mv %s %s", escapeshellarg($strTempFilepath), escapeshellarg($strFirmwareDirectory.'/'.$strFilename));
    $strResult = debug_shell_exec($strCmd);

    debugPrint($strResult);

    return UnpackUpdateFile($strFirmwareDirectory, $strFilename);
}


// Unpack our single 'update' file into its tgz and md5 component files...
function CleanupFiles($strtheFirmwareDir)
{
	
    // Master
    $strCmd = sprintf("cd %s; rm *.*", escapeshellarg($strtheFirmwareDir));

    $strResult = debug_shell_exec($strCmd);

    //Now for the slave...
	$strCmd = "213".chr(0x1F).escapeshellarg($strtheFirmwareDir);
	
		$strResult = NXTW_shell_exec($strCmd); 
   // $strCmd = sprintf("ssh root@192.168.40.2 'cd %s; rm *.*'", $strtheFirmwareDir);
   // $strResult = debug_shell_exec($strCmd); 

    // Must recreate the Eyelock.run file here
   // CreateEyelockRun();
   //so... the startup script will create these files?  OK then let's go with that.

    return TRUE;
}


// Unpack our single 'update' file into its tgz and md5 component files...
function UnpackUpdateFile($strDirectory, $strName)
{
    $strCmd = sprintf("cd %s;tar xf %s", escapeshellarg($strDirectory), escapeshellarg($strName));
   // $strCmd = sprintf("tar xf %s/%s", $strDirectory, $strName);
    debugPrint(sprintf("unpack... command is %s", $strCmd));
    $strResult = shell_exec($strCmd);
    //error_log("unpack tar result is ".$strResult);
    return TRUE; //dmotodo check for success...
}


function ValidateUploadedFiles($strFirmwareDir, $strFilename)
{
    //echo $strFirmwareDir;
    //echo $strFilename;

    $ResultMd5Check = -1;

    // Make sure that checkmd5 script is ok...
    $strResult = debug_shell_exec("tr '\r' '\n' < /home/root/scripts/checkmd5.sh > /home/root/scripts/tmpfilesed");
    $strResult = debug_shell_exec("mv /home/root/scripts/tmpfilesed /home/root/scripts/checkmd5.sh;chmod a+x /home/root/scripts/checkmd5.sh");
  //  $strResult = debug_shell_exec("rm /home/root/scripts/tmpfilesed");

    // Format of packagefile <filename>_pkg.tgz
    // Contents of package file are just <filename> without the _pkg...
    $strCmd = sprintf("cd %s;sync;sleep 2;/home/root/scripts/checkmd5.sh %s", escapeshellarg($strFirmwareDir."/".$strFilename));

    $strResult = debug_shell_exec($strCmd);

    $ResultMd5Check = intval($strResult);

    return ($ResultMd5Check === 0);
}



function CopySlaveFileToSlaveDevice($strFirmwareDir, $strFilename, $strSlaveCredentials = "root@192.168.40.2")
{
 //   $sshConfigFilename = "/home/sshconfigcomplete";
//    if (!file_exists($sshConfigFilename))
    {
//        root@nanonxt7001:~# ssh-keygen -t rsa
//        $strResult = debug_shell_exec("ssh-keygen -t rsa");

//        root@nanonxt7001:~# ssh root@192.168.40.2 mkdir -p .ssh
//        $strResult = debug_shell_exec("ssh root@192.168.40.2 mkdir -p /root/.ssh");

//        root@nanonxt7001:~# cat .ssh/id_rsa.pub | ssh root@192.68.40.2 'cat >> .ssh/authorized_keys'
//        $strResult = debug_shell_exec("cat /root/.ssh/id_rsa.pub | ssh root@192.68.40.2 'cat >> /root/.ssh/authorized_keys'");

//        $strResult = debug_shell_exec(sprintf("touch %s", $sshConfigFilename));
    }
    //TODO:
    //if sshpass exists... use sshpass -p command to login
    //sshpass -p (password) ssh root@192.168.40.2
    //file will be in /usr/bin
    {
       
       
       
        // The file has already been validated, we don't do it again...
        // cheat and copy the file over using cat and a pipe...
      //  $strCmd = sprintf("ssh root@192.168.40.2 'mkdir %s'", $strFirmwareDir);
	  $strCmd = sprintf("1335".chr(0x1F)."%s", escapeshellarg($strFirmwareDir));
	  
        $strResult = NXTW_shell_exec($strCmd); 

      //  $strCmd = sprintf("cat %s/%s | ssh root@192.168.40.2 'cat > %s/%s'", $strFirmwareDir, $strFilename, $strFirmwareDir, $strFilename);
	  $strCmd = sprintf("1336".chr(0x1F)."%s".chr(0x1F)."%s", escapeshellarg($strFirmwareDir.'/'.$strFilename),escapeshellarg($strFirmwareDir.'/'.$strFilename));
        $strResult = NXTW_shell_exec($strCmd); 

        
        $strCmd = sprintf("cp %s /home/root", escapeshellarg($strFirmwareDir.'/'.$strFilename));
        $strResult = debug_shell_exec($strCmd); 

        //if this is gonan fail what does it say when it fails?

        // Ok, now what?  Do we need to deal with settings?
    }

    return TRUE;
}


function SetExecPrivileges($strFolder, $strFilename)
{
    $strCmd = sprintf("62".chr(0x1F)."%s".chr(0x1F)."%s", escapeshellarg($strFolder), escapeshellarg($strFilename));//"cd %s; chmod a+x %s", $strFolder, $strFilename);
    $strResult = NXTW_shell_exec($strCmd);
    
    return TRUE;
}


function KillAppOnBoard($strAppName)
{
    $strFileName = "identityNkillEyelock.sh";

    $strResult = NXTW_shell_exec("63");//"[ ! -e /home/root/identityNkillEyelock.sh ] && echo 0 || echo 1;");

    if ($strResult === "1\n")
    {
        $strCmd = sprintf("64".chr(0x1F)."%s", $strAppName);//"sh /home/root/%s", $strAppName);
        $strResult = NXTW_shell_exec($strCmd);
    }
    else
    {
        //found root-related files, so need to rerun for killing app
        $strResult = NXTW_shell_exec("65");//"killall -s SIGKILL bash");

        $strCmd = sprintf("66".chr(0x1F)."%s", $strAppName);//"killall -s SIGKILL %s", $strAppName);
        $strResult = NXTW_shell_exec($strCmd);
    }
}
function MinimumVersionUpgradeTemp()
{
	//output if it works:
	//./Eyelock
 	//Version(AES) 3.06.1262
	//2016-08-16 14:15:06,365 INFO  [EyelockMain] - ./Eyelock Version(AES) 3.06.1262
	//chmod("/home/upgradeTemp/root/Eyelock", 777);
	shell_exec("chmod 777 /home/upgradeTemp/root/Eyelock");
	sleep(1);
	$versionOutput = shell_exec("/home/upgradeTemp/root/Eyelock -v"); //hardcoded string doesn't need escape
	$lines = explode('\n', $versionOutput);
	$versionString = "n/a";
	foreach($lines as $line)
	{
		if(strpos($line, "Version"))
		{
			$versionTokens = explode(' ', $line);
			$versionString = $versionTokens[2]; //there appears to be another line with version info in it.  We'll enver get to it
			break;
				
		}
		
	}
	if(strpos($versionString, '.') === FALSE) //what we got wasn't a version string, return false
	return false;
	error_log($versionString);
	$arCurrent = explode('.', $versionString);
	 $nMajorCurrent = intval($arCurrent[0]);
   
    $nMinorCurrent = intval($arCurrent[1]);
   
    $nBuildCurrent = intval($arCurrent[2]);
  
	
	$result = ($nMajorCurrent > 4 ||
	($nMajorCurrent == 4 && $nMinorCurrent > 0) ||
	 ($nMajorCurrent == 4 && $nMinorCurrent == 0 && $nBuildCurrent >= 1260));
	 
	if(!$result)
	return false;
	return true;
	
	
}
function CompareVersions($strCurrent, $strUpgrade)
{
    $strCurrent = trim($strCurrent);
    $strUpgrade = trim($strUpgrade);

    // Lop off the (AES) part if it exists...
    $arCurrentNumber = explode(" ", $strCurrent);
    if ($arCurrentNumber.length > 1)
        $arCurrent = explode(".", $arCurrentNumber[1]);
    else
        $arCurrent = explode(".", $strCurrent);

    $arUpgrade = explode(".", $strUpgrade);

    if (($arCurrent.length != $arUpgrade.length) || ($arCurrent.length < 3)) {
        return -1;
    }

    // Grab the version numbers...
    $nMajorCurrent = intval($arCurrent[0]);
    $nMajorUpgrade = intval($arUpgrade[0]);
    $nMinorCurrent = intval($arCurrent[1]);
    $nMinorUpgrade = intval($arUpgrade[1]);
    $nBuildCurrent = intval($arCurrent[2]);
    $nBuildUpgrade = intval($arUpgrade[2]);

    // Compare the version numbers.
    if (nMajorUpgrade > nMajorCurrent) {
        return 1;
    }
    else if (nMajorUpgrade == nMajorCurrent) {
        if (nMinorUpgrade > nMinorCurrent) {
            return 1;
        }
        else if (nMinorUpgrade == nMinorCurrent) {
            if (nBuildUpgrade > nBuildCurrent) {
                return 1;
            }
            else if (nBuildUpgrade == nBuildCurrent) {
                return 0;
            }
        }
    }

    return -1;
}
?>
