<?php
ini_set('display_errors',1); 
error_reporting(E_ALL);  


require("checklogin.php"); // Make sure user is logged on to get to this page...

$unameWithRoFs = "3.0.0-BSP-dm37x-2.4-2Eyelock_NXT_6.0";
$uname = php_uname("r");

function getLogLevel()
{
	// /home/root/nxtLog.cfg
	// looking for  line:
	//log4j.rootLogger=INFO, Rlog,stdout
	$level = "INFO";
	$logconfig = fopen("/home/root/nxtlog.cfg","r");
	if($logconfig == FALSE)
		return $level;
	while(!feof($logconfig))
	{
		$logline = fgets($logconfig);
		//echo $logline;
		if(strstr($logline, "log4j.rootLogger") != FALSE)
		{
			$parts = explode("=", $logline);
			//["log4j.rootLogger", "INFO, Rlog,stdout"]
			$target = explode(",", $parts[1])[0];
			//"NFO", "Rlog", "stdout"
			$level = $target;
			
			
		}
		
		
	}
	fclose($logconfig);
	return $level;
}

//////////////////////////////////////////////
// Handle Locale specific preprocessing...
/////////////////////////////////////////////    
//if (!defined("SESSIONSTARTED"))
if(!isset($_SESSION["SESSIONSTARTED"]))
    session_start(); // For cookie

/////////////////////////////////////////////
// Include our worker classes
/////////////////////////////////////////////
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/WebConfigModel.php");
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/WebConfigView.php");
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/WebConfigController.php");
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/rpc.php");

//require_once('./FirePHPCore/fb.php');
//FB::setEnabled(false);  // uncomment when going live...

// We need to save some values across posts (like logged on status)
//ob_start();

$model = new Model();
$controller = new Controller($model);
$view = new View($controller, $model);

// Must be included *after* our Model object is created!
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/lang.php");
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/webconfigtext.php");

// Moved to here so Login dialog displays correct text
$controller->Load();
defineStrings($model->eyeLockINI->HardwareType); // Load the selected language strings


// If the user is executing some action (we currently don't have any POST actions)
if (isset($_POST['action']) && !empty($_POST['action']))
{
    // FB::info($_POST['action'], '$_POST[action]');
    switch ($_POST['action'])
    {
        default:
        {
            $controller->{$_POST['action']}($_POST);
            break;
        }
    }
}

$factoryLicenseFile = ($uname === $unameWithRoFs) ? "/tmp/etc/FactoryLicense" : "/etc/FactoryLicense";

//////error_log((!isset($_SESSION["LoggedIn"])));
//////error_log($_SESSION["LoggedIn"] == false);
//$foundFactoryLicense = (file_exists("/etc/FactoryLicense"))?("found factory"):("not found factory");
//$foundUserLicense = (file_exists("/home/UserLicense"))?("found user"):("not found user");
//error_log($foundFactoryLicense . " ". $foundUserLicense);

if(file_exists($factoryLicenseFile) && ! file_exists("/home/UserLicense"))
{
	
	    require_once($_SERVER['DOCUMENT_ROOT']."/licenseKey.html");
	 
}
else
{
		
		
if ((isset($_SESSION["LoggedIn"]) && $_SESSION["LoggedIn"] == true) )
   echo $view->Configure(); 
else
    // Display the page...
    echo $view->Login();
}
?>

