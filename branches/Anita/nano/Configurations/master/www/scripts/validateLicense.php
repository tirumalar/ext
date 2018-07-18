<?php

include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");

$unameWithRoFs = "3.0.0-BSP-dm37x-2.4-2Eyelock_NXT_6.0";
$uname = php_uname("r");

//validate the license

$factoryLicenseFile = ($uname === $unameWithRoFs) ? "/tmp/etc/FactoryLicense" : "/etc/FactoryLicense";

$factoryLicense = fopen($factoryLicenseFile, "r");
$factoryLicenseKey = fread($factoryLicense, 7);

$userLicenseKey = $_POST['tbLicenseKey'];
$result = strcmp($userLicenseKey, $factoryLicenseKey);
//error_log($factoryLicenseKey.", ".$userLicenseKey.", ".$result);
if(strcmp($userLicenseKey, $factoryLicenseKey) == 0)
{
	//write and redirect
	$userLicenseFile=	fopen("/home/UserLicense", "w");	
	if($userLicenseFile == FALSE)
		error_log("failed to open UserLicense file");
	fwrite($userLicenseHandle, $userLicenseKey);
	fclose($userLicenseFile);
	echo "ok";
	NXTW_shell_exec(1337);
	//$result = shell_exec("ssh root@192.168.40.2 touch /home/UserLicense");
	
	
	
	
}
else
{
	echo "invalid";
		//header("Location: /index.php");
}





?>