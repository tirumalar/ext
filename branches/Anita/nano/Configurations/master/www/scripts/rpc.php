<?php
    

require("checklogin.php"); // Make sure user is logged on to get to this page...
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");
include_once("restarteyelock.php");
include_once("inieditor.php");
require("passwd.php");

function logTimeSync($message)
{
	$log = fopen("/home/root/nxtEvent.log", "a");
	fwrite($log, date("Y-m-d H:i:s, e"). " > ".$message."\n");
	fclose($log);
	
	
}
// Handle all requests except for 'save'... save is handled
// via the typical form 'submit' process so that we can
// do all of our validation, etc...
// No need to get fancy here.  We execute all of the actions below...
if (isset($_REQUEST['action']))
{
    if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
    {
        //error_log("checklogin: not logged in rpc ");
        die;
    }
    switch($_REQUEST['action'])
    {
        case 'resetpassword':
        {
            $bResult = ResetPassword($_REQUEST['user'], $_REQUEST['oldpwd'], $_REQUEST['newpwd'], $_REQUEST['removepwd']);

            if ($bResult)
                echo "resetpassword|success";
            else
                echo "resetpassword|fail";

            break;
        }

        case 'identifydevice':
        case 'identifydevicestop':
        {
            $nLoopTimes = 3;
            $nSleepDelay = 1;
            $strAppName = "Eyelock";
            $strScriptFolder = "/home/root";
            $strScriptFilename = "nano_led";
            $strLEDScript = "led_rgb.sh";

            $strCmd = sprintf("[ ! -e %s ] && echo 0 || echo 1;", escapeshellarg($strScriptFolder.'/'.$strLEDScript));
            $cmdResult = shell_exec($strCmd);
            $strReturn = "";

            // If root led application exists, use it...
            if ($cmdResult === "1\n") //eyelock-user related files
            {
            	if ($_REQUEST['action'] === 'identifydevice')
            	{
                	//sh /home/root/led_rgb.sh rgb looptime nano_led
                	$strCmd = sprintf("33".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%d".chr(0x1F)."%s", escapeshellarg($strScriptFolder), escapeshellarg('./'.$strLEDScript), 3, escapeshellarg($strScriptFilename));//"cd %s;sh ./%s rgb %d %s", $strScriptFolder, $strLEDScript, 3, $strScriptFilename);
                	$cmdResult = NXTW_shell_exec($strCmd);
                }
                else
                {
                    // check DualAuthenticationMode settings...
				    echo "check DualAuthenticationMode settings...";
				    $eyeLockINI = new INIEditor("/home/root/Eyelock.ini");
				    $eyeLockINI->LoadIniSettings(true);
        			$eyeLockINI->get("GRITrigger.DualAuthenticationMode", $dualMode);
                	//sh /home/root/led_rgb.sh last nano_led.sh true/false
                	$strCmd = sprintf("34".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s", escapeshellarg($strScriptFolder), escapeshellarg('./'.$strLEDScript), escapeshellarg($strScriptFilename), escapeshellarg($dualMode));//"cd %s;sh ./%s last %s %s", $strScriptFolder, $strLEDScript, $strScriptFilename, $dualMode);
                	$cmdResult = NXTW_shell_exec($strCmd);
                }
            }
            else // otherwise run the flash individually...
            {
                for ($i = 0; $i < $nLoopTimes; $i++)
                {
                    $strCmd = sprintf("35".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%d".chr(0x1F)."%d".chr(0x1F)."%d".chr(0x1F)."%d", $strScriptFolder, $strScriptFilename, 10, 0, 0, 0);//"cd %s;./%s %d %d %d %d", $strScriptFolder, $strScriptFilename, 10, 0, 0, 0);
                    $cmdResult = NXTW_shell_exec($strCmd);
                    sleep($nSleepDelay);

                    $strCmd = sprintf("35".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%d".chr(0x1F)."%d".chr(0x1F)."%d".chr(0x1F)."%d", $strScriptFolder, $strScriptFilename, 0, 10, 0, 0);//"cd %s;./%s %d %d %d %d", $strScriptFolder, $strScriptFilename, 0, 10, 0, 0);
                    $cmdResult = NXTW_shell_exec($strCmd);
                    sleep($nSleepDelay);

                    $strCmd = sprintf("35".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%d".chr(0x1F)."%d".chr(0x1F)."%d".chr(0x1F)."%d", $strScriptFolder, $strScriptFilename, 0, 0, 10, 0);//"cd %s;./%s %d %d %d %d", $strScriptFolder, $strScriptFilename, 0, 0, 10, 0);
                    $cmdResult = NXTW_shell_exec($strCmd);
                    sleep($nSleepDelay);

                }  // of loop

        		if ($dualMode === "true")
        			$strCmd = sprintf("35".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%d".chr(0x1F)."%d".chr(0x1F)."%d".chr(0x1F)."%d", $strScriptFolder, $strScriptFilename, 0, 0, 0, 255);//"cd %s;./%s %d %d %d %d", $strScriptFolder, $strScriptFilename, 0, 0, 0, 255);
        		else
                	$strCmd = sprintf("35".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%d".chr(0x1F)."%d".chr(0x1F)."%d".chr(0x1F)."%d", $strScriptFolder, $strScriptFilename, 1, 1, 1, 255);//"cd %s;./%s %d %d %d %d", $strScriptFolder, $strScriptFilename, 1, 1, 1, 255);
                $cmdResult = NXTW_shell_exec($strCmd);

                $strCmd = sprintf("36".chr(0x1F)."%s".chr(0x1F)."%s", $strScriptFolder, $strAppName);//"cd %s;touch %s.run", $strScriptFolder, $strAppName);
                $cmdResult = NXTW_shell_exec($strCmd);
            }

            echo "identifydevice|{$strReturn}";
            break;
        }
		case 'osdpreset':
			{
				  $strResult = NXTW_shell_exec("406");//"/home/root/scripts/factoryReset.sh");
					   echo "osdpreset|osdpreset done";
		
					RestartApplication();
		
			}
			break;
        case 'factoryreset':
        {
            //todo: trigger logout before reset
            //Delete the session variables

            // Just call Fang's script...
           // shell_exec('ssh root@192.168.40.2 /home/root/scripts/factoryReset.sh');
            unset($_SESSION["LoggedIn"]);
            unset($_SESSION["UserName"]);

           	chmod("/home/root/scripts/factoryReset.sh", 0777);
            $strResult = NXTW_shell_exec("37");//"/home/root/scripts/factoryReset.sh");
            echo "factoryreset|factoryreset done";
/**
            $strScriptFolder = "/home/root";
            $strDefaultScriptFolder = "/home/default";

            // Copy default Eyelock.ini, etc... file from default dir...
            $strCmd = sprintf("cp %s/Eyelock.ini %s", $strDefaultScriptFolder, $strScriptFolder);
            $strResult = shell_exec($strCmd);
            // DMOTODO validate copy OK in $strResult...

            // Make sure interfaces and interfaces.md5 are deleted (resets us to DHCP on the reboot)
            $strCmd = sprintf("rm %s/interfaces", $strScriptFolder);
            $strResult = shell_exec($strCmd);

            $strCmd = sprintf("rm %s/interfaces.md5", $strScriptFolder);
            $strResult = shell_exec($strCmd);

            // remove the log file...
            $strCmd = sprintf("rm %s/log.txt", $strScriptFolder);
            $strResult = shell_exec($strCmd);


            // Copy default restore, etc... file from default dir... resets DHCP defaults
            $strCmd = sprintf("cp %s/reloadinterfaces.sh %s", $strDefaultScriptFolder, $strScriptFolder);
            $strResult = shell_exec($strCmd);

            // Copy the factory /etc/hostname file back...
            $strResult = shell_exec("cp /etc/FactoryHostname /etc/hostname");

            // DMOTODO validate copy OK in $strResult...

            // clear the database out...
            ResetDatabase();

            // Reset password (we already have this in a function...)
//            $_REQUEST['user'], $_REQUEST['oldpwd'], $_REQUEST['newpwd'], $_REQUEST['removepwd']
            //ResetPassword($_REQUEST['user'], "", "eyelock", );

            // Now do a 'soft' reboot
            shell_exec(sprintf("%s/%s", $strScriptFolder, "reloadinterfaces.sh"));

            // Force a proper Eyelock app restart...
            PostProcessSettingsChange();
**/
            // Respond to this request with error string or success...
           // echo "factoryreset|factoryreset done";

            //  unset($_SESSION["LoggedIn"]);
         //   unset($_SESSION["UserName"]);

       

            break;
        }

        case 'rebootdevice':
        {
            // do something, the put our response in the response field...
            // First kill the eyelock app...  this allows for the UI to then properly wait for the app to come back up.
	//	PostProcessSettingsChange(); // Kills the eyelock app
		  
		 
		  
 	  //  NXTW_shell_exec("38");//
         //   NXTW_shell_exec("39");//"/home/www/scripts/KillApplication.sh");
	 //  NXTW_shell_exec("206");
          //  NXTW_shell_exec("40");//
          //  NXTW_shell_exec("41");//'i2cset -y 3 0x2e 4 8'); // reset command to motherboard if the above command fails
          //  NXTW_shell_exec("29");//'reboot'); //third and one more step to ensure reboot
			
			//shell_exec('ssh root@192.168.40.2 reboot');
		  //	shell_exec("chmod 777 /home/www/scripts/KillApplication.sh");
			//shell_exec('i2cset -y 3 0x2e 4 7'); //enable watchdog to force the reboot
			//shell_exec('i2cset -y 3 0x2e 4 8'); // reset command to motherboard if the above command fails
			//shell_exec('reboot'); 
            echo "rebootdevice|rebootdevice done";
			//  sleep(25);
			 RebootDevice();
		 
            break;
        }


        case 'gettime':
        {
            // do something, the put our response in the response field...
            $strDate = shell_exec("date");

            echo "gettime|{$strDate}";
            break;
        }

        case 'updatetime':
        {
            // do something, the put our response in the response field...
		
			//we'll call this function from two locations, first from the button, then from the save settings part before we submit.  We'll need to validate the timeserver, IF IT CHANGES, to make sure that the new server is valid.  Call to 42 should respond with a 0.  
			//if this RPC call fails, we need to block the settings save and call up an error message.
			$strDate = NXTW_shell_exec(sprintf("42".chr(0x1F)."%s", escapeshellarg($_REQUEST['timeserver'])));//"rdate -s {$_REQUEST['timeserver']} 2>&1");
		    $resfile = fopen("/home/root/scripts/mynxtw", "r");
			$strDate = fread($resfile,1);
			$strintDate = intval($strDate);
  		//	error_log("result of time sync attempt >".$strDate);
			if($strintDate == 0)
			{
				// set the hardware clock.
				$strResult = NXTW_shell_exec("43");//"/sbin/hwclock -w"); // Does no harm to call this even on failure...

				$strtheDate = NXTW_shell_exec("44");//"date 2>&1");

				echo "updatetime|{$strDate}|{$strtheDate}";
				logTimeSync("Sync to Time Server ".$strServer ." successful");
				
			}
			else
			{
				logTimeSync("Sync to Time Server ".$strServer ." failed");
				echo "updatetime|{$strDate}";
			}
		
            break;
        }

	    case 'updatelocaltime':
        {
	        $strDate = NXTW_shell_exec(sprintf("45".chr(0x1F)."%s", escapeshellarg($_REQUEST['localtime'])));//"date -s '{$_REQUEST['localtime']}' 2>&1");

            $strSyncHwTime = "TIMESYNC_MSG";
            $strSyncHwTimeResult = SendTCPMessage($strSyncHwTime);

            $strtheDate = NXTW_shell_exec("44");//"date 2>&1");

            echo "updatelocaltime|{$strDate}|{$strtheDate}";
			logTimeSync("Sync to Local Workstation time.");
            break;
        }

        case 'acstest':
        {
            // do something, the put our response in the response field...
            // Can loop through $REQUEST to get the rest of the values... then pass to the
            // script for execution...
            // Connet to 127.0.0.1:8082 -> send string "acstest|protocol|cardid|facilitycode\n" (wait for response)
            // Build up string... then make request to correct socket...
            //$strTestACS = "TESTACS|{$_REQUEST['protocol']}|{$_REQUEST['Eyelock_ACPTestCardID']}|{$_REQUEST['Eyelock_ACPTestFacilityCode']}";
            $strTestACS = "TESTACS";

            $strAcsResult = SendTCPMessage($strTestACS);

            echo "acstest|{$strAcsResult}|{$strTestACS}";
            break;
        }
        
        case 'testmatch':
        {
            // do something, the put our response in the response field...
            // Can loop through $REQUEST to get the rest of the values... then pass to the
            // script for execution...
            // Connet to 127.0.0.1:8082 -> send string "acstest|protocol|cardid|facilitycode\n" (wait for response)
            // Build up string... then make request to correct socket...
            //$strTestACS = "TESTACS|{$_REQUEST['protocol']}|{$_REQUEST['Eyelock_ACPTestCardID']}|{$_REQUEST['Eyelock_ACPTestFacilityCode']}";
            $strTestACS = "TESTMATCH;{$_REQUEST['ipaddress']}";

            $strAcsResult = SendTCPMessage($strTestACS);

            echo "nwmstest|{$strAcsResult}|{$strTestACS}";
            break;
        }

        case 'getacstestdata':
        {
            $strResult = SendTCPMessage("GETACS");
              $strReturn = str_replace("|", "-",  $strResult);
               $strReturn = str_replace(";", "",  $strReturn);
            echo "getacstestdata|{$strReturn}";

            break;
          //  $strCmdResult = SendSQLDBMessage("select display from acsTestData;");

            // Before sending this out... parse the string so our rpc.js code is consistent in its parsing...
            //$arResponse = split(";", $strCmdResult);

//            if (isset($arResponse[1]) && ($arResponse[1] === "OK") && isset($arResponse[3]))
 //           {
  //              $strReturn = str_replace(",", "-", $arResponse[3]);
   //             echo "getacstestdata|{$strReturn}";
    //        }
     //       else
      //          echo "getacstestdata|fail";
       //     break;
        }


        case 'firmwareupdate':
        {
            // some value of 'REQUEST" should contain entire .tar file structure...
            // untar it to master
            // patch up Eyelock.ini
            // untar it to slave
            // reboot

            echo "firmwareupdate|result";
            break;
        }


        case 'getdb':
        {
            // Kick off script to clean up db...
            $strResult = SendTCPMessage("GET_USER_COUNT");

            echo "getdb|{$strResult}";
            break;
        }

        case 'checkapplication':
        {
            $strResponse = "checkapplication|";

            echo CheckApplicationStatus($strResponse);

            break;
        }


        case 'startapplication':
        {
            // DMOTODO - how to restart...
            // $strResult = shell_exec($strCmd);

            // DMOTODO -- return results of start attempt here...
            echo "startapplication|success";
            break;
        }  // of CreateEyelockRunFile_ST()




        case 'deleterestorepoints':
        {
            // We should a param that lists all of the restore points to delete delimited by '|'s
            $bNano = ($_REQUEST['target'] === 'nano');

            $arFiles = explode("|", $_REQUEST['restorepoints']);

            // Delete each file...
            foreach ($arFiles as $key=>$theFile)
            {
                if (isset($theFile))
                {
                    $theFile = trim($theFile);
					//18:
                    $cmd = sprintf("18".chr(0x1F)."%s", $bNano ? escapeshellarg("/home/firmware/nano/restorepoints/".$theFile) : escapeshellarg("/home/firmware/bob/restorepoints/".$theFile));
                    $ls = NXTW_shell_exec($cmd);

                    // Now delete them from the slave...
                    $cmd = sprintf("180".chr(0x1F)."%s",  $bNano ? escapeshellarg("/home/firmware/nano/restorepoints/".$theFile) : escapeshellarg("/home/firmware/bob/restorepoints/".$theFile));
                    $ls = NXTW_shell_exec($cmd);
                }
            }

            echo "deleterestorepoints|done";
            break;
        }

        case 'getrestorepoints':
        {
            // Get the list of '*' delimited filenames and return it...
            $strNanoFileList = LocalGetRemoteFolderFileList("/home/firmware/nano/restorepoints", true);
            // Lob off the trail '*' it causes us grief.
            $strNanoFileList = substr($strNanoFileList, 0, $strNanoFileList.length-1);

            $strBobFileList = LocalGetRemoteFolderFileList("/home/firmware/bob/restorepoints", false);
            // Lob off the trail '*' it causes us grief.

            $strBobFileList = substr($strBobFileList, 0, $strBobFileList.length-1);

            // Also need to get list of bob restore points...
            $strReturnList = sprintf("%s^%s", $strNanoFileList, $strBobFileList);

            echo "getrestorepoints|{$strReturnList}";
            break;
        }

        case 'extendcookie':
        {
           
              echo "extendcookie|ok";
             break;
        }


        case 'restorerestorepoint':
        {
 		//error_log("RPC.php initiate restore point restore");
       
            // We should have a param that lists all of the restore points to delete delimited by '|'s
            $bNano = ($_REQUEST['target'] === 'nano');

            $arFiles = explode("|", $_REQUEST['restorepoints']);

            // which file to restore... the first one...
            $theFile = "";
            $thetarFile = "";
            foreach ($arFiles as $key=>$theFile)
            {
                if (isset($theFile))
                {
                    $theFile = trim($theFile);
                    $thetarFile = $bNano ? "/home/firmware/nano/restorepoints"."/".$theFile : "/home/firmware/bob/restorepoints"."/".$theFile;
                    if (file_exists($thetarFile))
                        break;
                    else
                        die("restorerestorepoint|result Failed could not find file.");
                }
            }

			if(!MinimumVersionRestorePoint($thetarFile))
				 die("restorerestorepoint|result Failed no longer supported.");

            // Create a new restore point...
            // Do the Master stuff...
            // Ok, now we need to create our restore point, then upgrade the firmware...  NO we don't.  This is not a firmware upgrade it is a
            //restore point restoration.  There is no point in creating a restore point for a broken state! -MH
            // This already creates on for us on the Slave device too...
            // if (            CreateRestorePoint("/home", "root", $bNano ? "/home/firmware/nano/restorepoints" : "/home/firmware/bob/restorepoints", ConstructRestorePointFilename()))
            NXTW_shell_exec("310");
			unlink("/home/root/Eyelock.run");
            NXTW_shell_exec("31");//"chmod 777 /home/www/scripts/KillApplication.sh");
            NXTW_shell_exec("32");//"/home/www/scripts/KillApplication.sh");
           //  KillApplication();

            {
                // Before untarring the restore point, we need to store off the db... we don't blow it away (only worry about it on the master)...
                $strCmd = sprintf("mv %s/%s %s/%s", "/home/root", "test.db", "/home/root", "test.db.restore");
                $strResult = shell_exec($strCmd);
		        $strCmd = sprintf("mv %s/%s %s/%s", "/home/root", "keys.db", "/home/root", "keys.db.restore");
                $strResult = shell_exec($strCmd);

		  // Restore the restore point here.
                $strCmd = sprintf("cd %s;tar xf %s", "/home", escapeshellarg($thetarFile));
                $strResult = shell_exec($strCmd);

                // Now, put the original test.db file back... cuz it was just overwritten with an older version
                $strCmd = sprintf("mv %s/%s %s/%s", "/home/root", "test.db.restore", "/home/root", "test.db");
                $strResult = shell_exec($strCmd);
		        $strCmd = sprintf("mv %s/%s %s/%s", "/home/root", "keys.db.restore", "/home/root", "keys.db");
                $strResult = shell_exec($strCmd);
				 NXTW_shell_exec("207".chr(0x1F).$thetarFile);
                // Handle the slave here too...
               // $strCmd = sprintf("ssh root@192.168.40.2 'cd %s;tar xf %s'", "/home", $thetarFile);
               // $strResult = shell_exec($strCmd);
            }
            sleep(5);
              NXTW_shell_exec("310");
              NXTW_shell_exec("32");//"/home/www/scripts/KillApplication.sh");
	        //error_log("RPC.php Main FW restored");

            $bobfw = FindBobFirmwareFilename();
            //error_log(sprintf("RPC.php Finding bob FW: %s" , $bobfw));
            if($bobfw != "empty")
            {            
                $res = RestoreICMFirmware("",$bobfw);
               // if(!$res)
               ////{
                   // //error_log("RPC.php restore failed");
                    // die("restorerestorepoint|result failed ICM restore failed.");
               // }
            }
                //error_log("RPC.php restore done");
                
            echo "restorerestorepoint|result done";
           
            break;
        }


        // Returns version strings for nano, bob, and Webconfig plus a flag for each describing
        // whether an upgrade is available...
        case 'getupgradestatus':
        {
/**
            $url = "http://127.0.0.1/upgrade/nanoversion.xml";

            // Pull the version file, and return the contents...
            // separate files for nano, bob, and webconfig...
            // check for error, if file doesn't exist, need to return 'no upgrade avail' flag...
            $xml = simplexml_load_file($url);

/**
    type="Nano"
    desription="A new Nano NXT Firmware update is avaialable!"
    date="1970"
    version="x.x.x"
    mandatory="true"
    url="http://127.0.0.1/upgrade"
    file="nanoupgrade.tgz"

            // Make sure file is valid
            if ("Nano === $xml->Upgrade->type")
            {
                $description = $xml->Upgrade->description;
                $version = $xml->Upgrade->version;
                $mandatory = $xml->Upgrade->mandatory;
                $url = $xml->Upgrade->url;
                $filename = $xml->Upgrade->filename;

                // Ok, this is all of our data...
                if (NewerVersion($_REQUEST['currentnanoversion'], $version)
                {
                }
            }


            $xml->NanoVersionNumber
            $xml->NanoFirmware


**/
            echo "getupgradestatus|none";
            break;
        }


        // Used to check eyelock running and tamper...
        case 'getdevicestatus':
        {
            $strgds = "getdevicestatus|";

            $strgds = CheckApplicationStatus($strgds);

            // Now check for tamper...
            $tampered = shell_exec("ls -la /home/root | grep tamper");
            // If file exists, we're done...
            if (strlen(trim($tampered)) > 0)
                $strgds = $strgds.",tampered";
            else
                $strgds = $strgds.",nottampered";

            echo $strgds;
            break;
        }

	    default:
        {
            echo "default|result done";
            break;
        }
     }
}

function FindBobFirmwareFilename()
{
    $files = glob("/home/root/*.cyacd");
    if(sizeof($files) == 0)
    {
         //error_log("no found file");
        return "empty";
    }
    else{
        $ret = $files[0];
        //error_log(sprintf("found file %s" , $ret));
        return $ret;
    }
}


function MinimumVersionRestorePoint($restorePoint)
{
	//it is sufficient secure enough to simply check the restore point filename	
	//root_20160815_194614_3.06.1262.tgz... restore point f ilename. 
	//first explode by _ to get the 3.06.1262.tgz part on its own.  Then explode that by . to split the version numbers
	error_log($restorePoint);
	$explode1 = explode('_', $restorePoint);
	$versionString = $explode1[count($explode1) -1]; //last element is the version
	$arCurrent = explode('.', $versionString);
	 $nMajorCurrent = intval($arCurrent[0]);
   
    $nMinorCurrent = intval($arCurrent[1]);
   
    $nBuildCurrent = intval($arCurrent[2]);
	if($nMajorCurrent > 3 ||
	($nMajorCurrent == 4 && $nMinorCurrent > 0) ||
	 ($nMajorCurrent == 0 && $nMinorCurrent == 0 && $nBuildCurrent >= 1260))
	return true;
	return false;
	
}

function rpcicmProgress($strExecMe)
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
	//error_log($running);

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

function RestoreICMFirmware($strUpgradeFileDirectory, $strUpgradeFilename)
{
    //TODO: comment this return
    //return TRUE;
    //error_log(sprintf("Starting bob update %s ", $strUpgradeFilename));
     //setFlag("/home/icmupdate.txt");
    // Execute icm_connector with the file option... to start the upgrade
    shell_exec("chmod 777 /home/root/icm_communicator");

   // $strCmd = sprintf("/home/root/icm_communicator -p %s", $strUpgradeFilename);
    
	$strCmd = sprintf( "998".chr(0x1F)."%s".chr(0x1F)."%s ",$timeout,  escapeshellarg($strUpgradeFileDirectory."/".$strUpgradeFilename)) ;
	
	
     //error_log(sprintf("update cmd is %s ", $strCmd));
    $strResult = rpcicmProgress($strCmd);
    //$strResult = shell_exec($strCmd);
    //$strResult3 = shell_exec("rm /home/bobUp.txt");
     //error_log(sprintf("update result is %s ", $strResult));
     sleep(10);
   // $strCmd = sprintf("echo -e \"%s\" >> /home/bobUp.txt", $strResult);
    //$strResult2 = debug_shell_exec($strCmd);

  
    return TRUE;
    
    $pos = strpos($strResult, "ICM program done (can't tell if its actually successful!!!)");
    if ($pos !== false)
    {
        // clearFlag("/home/icmupdate.txt");
        return TRUE;
    }
    else
        return FALSE;
}


function CheckApplicationStatus($stringResult)
{
	$eyeLockINI = new INIEditor("/home/root/Eyelock.ini");
    $eyeLockINI->LoadIniSettings(true);
    // Process master
    $cmdGrep = shell_exec(sprintf("ps -e | grep %s", "Eyelock"));
//	error_log("found? ".$cmdGrep);
    $pos = strpos($cmdGrep, "Eyelock");
    if ($pos !== false)
        $stringResult = $stringResult."masterrunning";
    else
        $stringResult = $stringResult."masternotrunning";

    //We only check for the slave if we are an NXT.
	if ($eyeLockINI->HardwareType == '0')
    {
        $pos = NXTW_shell_exec("200");
	    //turns oout handling of this was incorrect-
	    //the NXTwResult will leave the text empty.  This is the not-running case.  If the app is running then the result text length is greater than 0. 
	    $resultFile = fopen("/home/nxtwResult","r");
		    $resultText = fgets($resultFile);
		    fclose($resultFile);
		    $pos = strlen($resultText);

        if ($pos > 0)
            $stringResult = $stringResult.",slaverunning";
        else
            $stringResult = $stringResult.",slavenotrunning";
    }
   //if ($pos !== false)
   //     $stringResult = $stringResult.",slaverunning";
    //else
     //   $stringResult = $stringResult.",slavenotrunning";

    return $stringResult;
}


function SendSQLDBMessage($strMessage, $strAddr = "127.0.0.1", $strPort = "8085")
{
    // Note $strMessage must already contain trailing semicolon...
    $len = strlen($strMessage) + strlen("RECEIVESQLITE;;");

    $SQLRequest = sprintf("RECEIVESQLITE;%d;%s", $len + strlen(sprintf("%d", $len)), $strMessage);

    return SendTCPMessage($SQLRequest, $strAddr, $strPort);
}



// DMOTODO add retries and error handling in here...
function SendTCPMessage($strMessage, $strAddr = "127.0.0.1", $strPort = "8085")
{
    $client = NULL;

// DMOTODO for now ssl isn't working... we always just communicate with port 8091.
/**
    if (isset($_REQUEST['securecomm']) && ($_REQUEST['securecomm'] === "true"))
    {
        $strCert = "/home/root/eyelock.pem";

        $streamContext = stream_context_create();
        stream_context_set_option($streamContext, 'ssl', 'local_cert', $strCert);

        $client = stream_socket_client("ssl://127.0.0.1:8081", $error, $errorString, 2, STREAM_CLIENT_CONNECT, $streamContext);
    }
    else
**/
	try{
        $client = stream_socket_client("tcp://$strAddr:$strPort", $errno, $errorMessage);

		if ($client === false)
		{
			return "failed|".$errorMessage;
			//throw new UnexpectedValueException("Failed to connect: $errorMessage");
		}

		fwrite($client, "{$strMessage}\n");
		$strReturn = stream_get_contents($client);
		fclose($client);

        return $strReturn;
	}
	catch (Exception $e)
	{
		return   "failed|invalid IP";
	}
}


function ResetDatabase()
{
    $strDB1 = "RECEIVESQLITE;19;delete from person;";
    $strDB2 = "RECEIVESQLITE;24;delete from updateState;";

    // DMOTODO - check for errors?
    $strCmdResult = SendSQLDBMessage($strDB1);
    $strCmdResult = SendSQLDBMessage($strDB2);
}

function ResetPassword($user, $oldpass, $newpass, $strremovepwd)
{
	if (!ValidateUserPwd($user, $oldpass))
	{
		//echo "invalid current password for $user\n";
		return false;
	}
	if (strpos($newpass, "\"") || strpos($newpass, "\'"))
	{
		return false;
	}
	
	$passStorage = "/etc/shadow";
	$isRoFs = false;
	
	initPassStorage($passStorage, $isRoFs);
	if ($isRoFs)
	{
		$salt = base64_encode(random_bytes(6)); // PHP v7 is required 
		// base64 MIME can contain +, which is not supposed to be used for crypt salt
		$salt = str_replace("+", ".", $salt); 
		$newPassHashed = crypt($newpass, "$1$".$salt."$");
		
		//echo "newPassHashed: $newPassHashed\n";
		
		$lineParts = NULL;
		$passChanged = false;
		$fileContent = "";
		$handle = fopen($passStorage, "r");
		if ($handle) 
		{
			while (($line = fgets($handle)) !== false) 
			{
				$lineParts = explode(":", $line);
				//echo "user: $lineParts[0]\n";
				if ($lineParts[0] === $user)
				{
					$lineParts[1] = $newPassHashed;
					$passChanged = true;
				}
				
				$fileContent .= implode(":", $lineParts);
			}
			fclose($handle);
		} 
		
		if (!$passChanged)
		{
			//echo "cannot change password because user $user entry not found in password storage\n";
			return false;
		}
		
		$tempPassFile = $passStorage."temp";
		if (file_put_contents($tempPassFile, $fileContent))
		{
			rename($tempPassFile, $passStorage);
		}
		else
		{
			//echo "cannot write temp pass file $tempPassFile\n";
			unlink($tempPassFile);
			return false;
		}
	}
	else
	{
		// legacy implementation
		$strCmd = sprintf("47".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s", $newpass, $newpass, $user);//"echo -e \"%s\\n%s\\n\" | passwd -a MD5 %s", $newpass, $newpass, $user);
		$cmdResult = NXTW_shell_exec($strCmd);
	}
	
	return ValidateUserPwd($user, $newpass);
}

/*
{
    date_default_timezone_set('UTC');
    $new_date = new DateTime($time);
    $new_date->setTimeZone(new DateTimeZone('America/New_York'));
    return $new_date->format("Y-m-d h:i:s");

    //JAVASCRIPT to convert string into Date object...
    var s = '2012-10-20 00:00';
    var bits = s.split(/\D/);
    var date = new Date(bits[0], --bits[1], bits[2], bits[3], bits[4]);

}
*/

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

    if (NULL === $ls)
        $ls = "\n";

    $cmdls = str_replace("\n", "*", $ls);
//    $arFiles = explode("|", $cmdls);

    // Return the string of restore points...
    return $cmdls;
}
?>
