<?php

require("checklogin.php"); // Make sure user is logged on to get to this page...
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");
include_once("inieditor.php");
//include_once("/home/www/debug.php");
 //ob_start();

    function PostProcessSettingsChange()
    {
        // Creates the Eyelock.run file if it does not already exist...
		NXTW_shell_exec("201"); // chown -R www:www /tmp/*
        CreateEyelockRun();
        RestartApplication();     
    }
    function RebootDevice()
    {
		$eyeLockINI = new INIEditor("/home/root/Eyelock.ini");
	    $eyeLockINI->LoadIniSettings(true);

		if ($eyeLockINI->HardwareType == '1') // ext
		{
			NXTW_shell_exec("1339"); // cd /home/root; ./fwHandler.sh reboot
		}
		else
		{
       // $strResult = NXTW_shell_exec("29");//"reboot");
	   shell_exec("touch /home/nxtW.run");
		sleep(15); 
		
            NXTW_shell_exec("38");//
	   NXTW_shell_exec("39");//"/home/www/scripts/KillApplication.sh");
	   sleep(5);
            NXTW_shell_exec("40");//
		sleep(2); 
            NXTW_shell_exec("40");//
		sleep(2); 
            NXTW_shell_exec("40");//
		sleep(2); 
	   NXTW_shell_exec("206");
		   sleep(2);
		NXTW_shell_exec("1338");
		sleep(2);

		shell_exec("printf 'fixed_set_rgb(100,80,0)\n' | nc -q 1 192.168.4.172 50");

            NXTW_shell_exec("29");//'reboot'); //third and one more step to ensure reboot
		   sleep(2);
            NXTW_shell_exec("41");//'i2cset -y 3 0x2e 4 8'); // reset command to motherboard if the above command fails
		}	
    }

    function DeleteEyelockRun()
    {
        $strResult = shell_exec("cd /home/root;rm Eyelock.run");
         //$strResult = shell_exec("ssh root@192.168.40.2 'cd /home/root;rm Eyelock.run'"); 
		  NXTW_shell_exec("202");
    }


    function CreateEyelockRun()
    {
        $strResult = shell_exec("cd /home/root;ls -la | grep Eyelock.run");

        // If file exists, we're done...
        if (strlen(trim($strResult)) > 0)
            return TRUE;
        else
        {
            // create the file
            $strResult = shell_exec("touch Eyelock.run");

            // write it
            $strResult = shell_exec("echo > /home/root/Eyelock.run;sleep 2");

            $strResult = shell_exec("sync");

            return TRUE;
        }
         $strResult = NXTW_shell_exec("202");//shell_exec("ssh root@192.168.40.2 'cd /home/root;rm Eyelock.run'"); 
        if (strlen(trim($strResult)) > 0)
            return TRUE;
        else
        {
			NXTW_shell_exec("203");
            // create the file
            //$strResult = shell_exec("touch Eyelock.run");
            // $strResult = shell_exec("ssh root@192.168.40.2 'cd /home/root;touch Eyelock.run'"); 
            // write it
           /// $strResult = shell_exec("echo > /home/root/Eyelock.run;sleep 2");

            // $strResult = shell_exec("ssh root@192.168.40.2 'sleep 2'"); 

            //$strResult = shell_exec("ssh root@192.168.40.2 'sync'");

            return TRUE;
        }

        


        return FALSE;
    }


    function RestartApplication()
    {
        // This kills it...
        $strResult = NXTW_shell_exec("30");//"killall -s SIGKILL Eyelock");
        //error_log(sprintf("restart applicaiton: killall %s", $strResult));

      //  $strResult = shell_exec("i2cset -y 3 0x2e 4 7");
     //   //error_log(sprintf("restart applicaiton: i2cset %s", $strResult));
        // bash script startup.sh is polling and will restart eyelock if it goes down...
    }


    // THis function kills the watchdog and the Eyelock app.  Reboot is required to bring it back up.
    function KillApplication()
    {
        // Kill the application as a shell script
        NXTW_shell_exec("31");//"chmod 777 /home/www/scripts/KillApplication.sh");
        NXTW_shell_exec("32");//"/home/www/scripts/KillApplication.sh");
        return;
        /*
        $strResult = shell_exec("i2cset -y 3 0x2e 4 6");
       
        shell_exec(sprintf("echo test %s>restart.txt", $strResult));
        DeleteEyelockRun();

        //kill a watchdog
        $strResult = shell_exec("killall –KILL PushButton");
       // $firephp->log($strResult);
        // Kill the app.
        RestartApplication();

        // Also, individually kill the app on the slave...
       // $strResult = shell_exec("ssh root@192.168.40.2 'killall -s SIGKILL Eyelock'"); 
       // $firephp->log($strResult);*/

    }


    /////////////////////////////////////////////////////
// Backup Current System
/////////////////////////////////////////////////////
function CreateRestorePoint($strBackupParentDir, $strDirTreeToBackup, $strBackupToDirectory, $strBackupFilename, $wwwDirTree)
{
      //error_log("restartEyelock.php::CreateRestorePoint()");
    // If no backup filename supplied, generate one...
    if (strlen($strBackupFilename) <= 0)
        $strBackupFilename = ConstructRestorePointFilename();


    //remove the database before upgrading... we saved it before doing this

   // $strResult = shell_exec("rm /home/root/*.db");

    // MASTER
    // Make sure our restorepoints directory exists...
    $strCmd = sprintf("mkdir -p %s", escapeshellarg($strBackupToDirectory));
    $strResult = shell_exec($strCmd);

    $strResult = shell_exec("[ -e /home/root/eyestartup ] && echo 0 || echo 1");

    if($strResult === "0\n")
        $strCmd = sprintf("cd %s;cp -R /home/eyelock/data /home/root/;sync;tar cf %s %s %s", escapeshellarg($strBackupParentDir), escapeshellarg($strBackupToDirectory.'/'.$strBackupFilename), escapeshellarg($strDirTreeToBackup), escapeshellarg($wwwDirTree));
    else
        $strCmd = sprintf("cd %s;tar cf %s %s", escapeshellarg($strBackupParentDir), escapeshellarg($strBackupToDirectory.'/'.$strBackupFilename), escapeshellarg($strDirTreeToBackup));

    // Do the backup...
    $strResult = shell_exec($strCmd);


    // SLAVE
    // Make sure our restorepoints directory exists...
	
	NXTW_shell_exec("204".chr(0x1F).escapeshellarg($strBackupToDirectory));
	
   // $strCmd = sprintf("ssh root@192.168.40.2 'mkdir -p %s'", $strBackupToDirectory);
    //$strResult = shell_exec($strCmd);

	NXTW_shell_exec("205".chr(0x1F).escapeshellarg($strBackupParentDir).chr(0x1F).escapeshellarg($strBackupToDirectory."/".$strBackupFilename).chr(0x1F).escapeshellarg($strDirTreeToBackup));
   // $strCmd = sprintf("ssh root@192.168.40.2 'cd %s;tar cf %s/%s %s'", $strBackupParentDir, $strBackupToDirectory, $strBackupFilename, $strDirTreeToBackup);

    // Do the backup...
   // $strResult = shell_exec($strCmd);

    // DMOTODO - detect failure?
    /**
    if (cmdBackup.Error.Trim().Length > 0)
    {
        FireStepOcurred(new StatusMessage(string.Format("error when backing up root folder, Board's Ip: {0}", OldIpOfBoard), StatusMessageTypes.Error));
        l_bSuccess = false;
    }
    **/
    // DMOTODO how to check for successful backup????
    return TRUE;
}

function getCurrentAppVersion()
{
	$cmdVersion = shell_exec("cd /home/root/ && ./Eyelock -v");
	preg_match("/Version\(AES\)\s(.*)/",$cmdVersion,$matches);
	return trim($matches[1]);
}

function ConstructRestorePointFilenameTime($time)
{
    $strDateTimeStamp = date("Ymd_His", $time);
    $version = getCurrentAppVersion();
    return sprintf("root_%s_%s.tgz", $strDateTimeStamp,$version);
}


function ConstructRestorePointFilename()
{
    return ConstructRestorePointFilenameTime(time());
}


?>
