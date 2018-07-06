<?php 
ini_set('display_errors',1); 
error_reporting(E_ALL);
if(!isset($_SESSION["SESSIONSTARTED"]))
    session_start(); // For cookie
  



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
defineStrings($model->eyeLockINI->HardwareType); // Load the selected language strings
		
 require_once($_SERVER['DOCUMENT_ROOT']."/factoryResetLogout.html");



?>