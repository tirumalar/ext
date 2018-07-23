<?php
    
require("checklogin.php"); // Make sure user is logged on to get to this page...
require("passwd.php");
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");

if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
{
 //   error_log("checklogin: not logged in uploadcert");
    die;
}

$user = $_POST['user'];
$oldPassword = $_POST['oldpwd'];
$newPassword = $_POST['newpwd'];

//file_put_contents("/home/phpdebug.log", "newPassword, oldPassword - $newPassword, $oldPassword\n", FILE_APPEND);

$bResult = ResetPassword($user, $oldPassword, $newPassword);

if ($bResult)
	echo "resetpassword|success";
else
	echo "resetpassword|fail";

function ResetPassword($user, $oldpass, $newpass)
{
	//file_put_contents("/home/phpdebug.log", "ResetPassword begin: newpass, user, oldpass  - $newpass, $user, $oldpass\n", FILE_APPEND);
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
			ulink($tempPassFile);
			return false;
		}
	}
	else
	{
		// legacy implementation
		//file_put_contents("/home/phpdebug.log", "newpass, user - $newpass, $user\n", FILE_APPEND);
		$strCmd = sprintf("47".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s", $newpass, $newpass, $user);//"echo -e '%s\\n%s\\n' | passwd -a MD5 %s", $newpass, $newpass, $user);
		$cmdResult = NXTW_shell_exec($strCmd);
	}
	
	return ValidateUserPwd($user, $newpass);
}

?>
