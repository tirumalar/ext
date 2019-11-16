<?php

require("checklogin.php"); // Make sure user is logged on to get to this page...
if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
{
    //error_log("checklogin: not logged in upgrade");
    die;
}
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

if (isset($_FILES["0"]) && $_FILES["0"]["error"] == UPLOAD_ERR_OK)
{
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
                die("|fail"); //output error
    }
	echo("|Total 0 of xx"); //when we start loading, set the "updating" state with 0 so the progress bar resets
     $tarFileName = $_FILES['0']['name']; //uploaded tar filename //name of the bootloaderFW file 
	     $tartempFileName    = $_FILES['0']['tmp_name'];
		 $filePath = escapeshellarg('/home/firmware/'.$tarFileName);
	 $strCmd = sprintf("mv %s %s", escapeshellarg($tartempFileName), $filePath);
    $strResult = shell_exec($strCmd); //has no result
	 
	 
	//command is /home/root/icm_communicator -r (firmware.bin).  NOTE That we should make sure this firmware is actually firmware.  potential security hole.  I need the NXTW number for this.
//	error_log("tar file is " . $tarFileName . " " . $filePath);
	chmod($filePath, 0777);
	//how this works:
	//kick off the ICM communicator pointing to the file
	//After about 4 seconds, start monitoring the output file, tail -n 1 output...
	//watch that until the icm_communicator process is no longer found
	  NXTW_shell_exec("39");//"/home/www/scripts/KillApplication.sh");
	
	NXTW_shell_exec("997".chr(0x1F).$filePath, true);//todo: get the NXTW number from fang.  It should take 1 parameter: the file to load.  Don't wait for response, we don't care, we're gonna watch the upgrade
	if(	watchICMCommunicator())
	{
			sleep(30); //need to wait for the firmware to actually write so we can update the versions
		PostProcessRestartApps();
	
		die("|success");
	}
	else
	{
		PostProcessRestartApps();
		
		die("|fail");
	}
	
}
function watchICMCommunicator()
{
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
		echo "|".$lastline;
	}
$lastline = shell_exec("tail -n 1 /home/www/updateprogress");
    //error_log("finished icm upgrade last line is ".$lastline);
    if((strpos($lastline , "Programming Reader Start ...") != FALSE || strpos($lastline, "time:")!= FALSE))
        return TRUE;
        else
    return FALSE;
}

function PostProcessRestartApps()
	{
		chmod("/home/www/scripts/StartApplication.sh", 777);
		NXTW_shell_exec("511");//"/home/www/scripts/KillApplication.sh");
			 
	}



?>