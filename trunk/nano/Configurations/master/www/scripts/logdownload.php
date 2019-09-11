<?php 
  
require("checklogin.php"); // Make sure user is logged on to get to this page...
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");
if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
{
    //error_log("checklogin: not logged in logdl");
    die;
}
	
   header("Content-Type: application/octet-stream;  X-Content-Type-Options: nosniff;");
   header("Content-Disposition: attachment; filename={$_GET['dlfilename']}");
$slavelog = false;
if(isset($_GET['slavelog']))
{
	if(!file_exists("/home/slavelogs")) //this checks for a directory too apparently.
		mkdir("/home/slavelogs");
	
	//250 will download all of the possible slave logs to the directory /home/slavelogs/
	if($_GET['slavelog'] == "1")
		$slavelog = true;
}

if (isset($_GET['logfiletype']))
{
	 //this checks for a directory too apparently. 
	if(!file_exists("/home/slavelogs"))
		mkdir("/home/slavelogs");
//DMOHBOXOUT	$pos = NXTW_shell_exec("250"); //pick a number	
//	error_log("is slave " . $_GET['slavelog'] . " type " .$_GET['logfiletype']);
    $urllog = "";
	$slaveLogPath = "";
    switch ($_GET['logfiletype'])
    {
        case '1':
        {
          $urllog = "/home/root/nxtLog.log";
			$slaveLogPath = "/home/slavelogs/nxtLog.log";
			
          break;
        }

        case '2':
        {
          $urllog = "/var/log/messages";
		$slaveLogPath = "/home/slavelogs/messages";
          break;
        }
        case '7':
        {
          $urllog = "/home/root/eyelockShellScripts.log";
		$slaveLogPath = "/home/slavelogs/eyelockShellScripts.log";
          break;
        }

        case '3':
        {
           $urllog = "/home/root/tempNxtEvent.log";
			$slaveLogPath = "/home/slavelogs/nxtLog.log"; //tempNxtEvent doesn't exist on slave, download the detailed log instead
           break;
        }

        case '4':
        {
            $urllog = "/home/root/rootCert/certs/download.pfx";
            break;
        }

        case '5':
        {
            $urllog = "/home/root/wpa_supplicant.log";
            break;
        }

		default:
    		die;
   }

	if($slavelog && $_GET['logfiletype'] != '4')
			{
				//if(file_exists($slaveLogPath))
				//{
					//for current testing, don't delete it.  250 doesnt exist
					
				//	unlink($slaveLogPath);
				//}
				
				
				$urllog = $slaveLogPath; 
	//			error_log("slave log path is ".$urllog);
				
			}
	
	
   readfile($urllog);
}
?>
