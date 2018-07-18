<?php

function initPassStorage(&$passStorage, &$isRoFs)
{
	$passStorageDefault = "/etc/shadow";
	$passStorageCustom = "/home/www-internal/shadow";
	$unameWithRoFs = "3.0.0-BSP-dm37x-2.4-2Eyelock_NXT_6.0";
	$unameHbox = "4.13.0-37-generic";
	
	$passStorage = $passStorageDefault;
	
	$uname = php_uname("r");
	//echo "uname -r: $uname\n";
	
	// Uname is not fixed on Ubuntu
	// TODO: properly identify device type
	//if ($uname === $unameWithRoFs || $uname == $unameHbox )
	{
		$isRoFs = true;
		if (!file_exists($passStorageCustom))
		{
			$fileContent = "";
			$handle = fopen($passStorage, "r");
			if ($handle) 
			{
				while (($line = fgets($handle)) !== false) 
				{
					$lineParts = explode(":", $line);
					//echo "user: $lineParts[0]\n";
					if ($lineParts[0] === "installer" || $lineParts[0] === "admin")
					{
						$fileContent .= $line;
					}
				}
				fclose($handle);
			}
			file_put_contents($passStorageCustom, $fileContent);
		}
		$passStorage = $passStorageCustom;
	}		
}

function ValidateUserPwd($user, $pass)
{
	if (empty($pass))
	{
		return false;
	}
	
	$passStorage = "/home/shadow";
	$isRoFs = false;
	
	initPassStorage($passStorage, $isRoFs);
	
	//printf("opening %s\n", $passStorage);
	
	$lineParts = NULL;
	$userFound = false;
	$handle = fopen($passStorage, "r");
	if ($handle) 
	{
		while (($line = fgets($handle)) !== false) 
		{
			// installer:$1$Z7CZwAtQ$VBe9ILsIIULBCqxLbLAry0:17519:0:99999:7:::
			$lineParts = explode(":", $line);
			if ($lineParts[0] === $user)
			{
				$userFound = true;
				break;
			}
		}
		fclose($handle);
	} 
	
	if (!$userFound)
	{
		//echo "cannot find user $user entry in password storage\n";
		return false;
	}
	
	// workaround for locked accounts (usermod -L)
	// installer:!$1$f5aIzw4s$Ge5/SjNkfJsPjoZo414tB/:17517:0:99999:7:::
	$hashedPass = $lineParts[1];
	if ($hashedPass[0] === '!')
	{
		$hashedPass = substr($hashedPass, 1);
	}	
	
	//echo "hashed pass: $hashedPass\n";
	
	if (empty($hashedPass))
	{
		return false;
	}
	
	return hash_equals($hashedPass, crypt($pass, $hashedPass));
}

