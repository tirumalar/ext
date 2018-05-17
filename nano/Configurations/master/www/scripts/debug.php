<?php
//if(!function_exists("ShellMSG"))
//{
	function WriteToTempFile($strContent)
	{
		$file = fopen("/home/www/temp", "a");
		fwrite($file, $strContent);
		fclose($file);	
	}
	
//example with 2 parameters: $strcmdResult = NXTW_shell_exec(sprintf("5".chr(0x1F)."%s".chr(0x1F)."%s", escapeshellarg('/etc/network/'.$this->InterfacesFile)	


	function NXTW_shell_exec($strMessage, $returnImmediately=false, $strAddr = "127.0.0.1", $strPort = "1234")
	{
		$client = NULL;
	
		$flog = fopen("/home/www/msglog", "a");
		
		
	
		$len = strlen($strMessage);
		//todo: remove this debug text
		$msgpreview = "" . $len .chr(0x1F) . $strMessage;
		
		fwrite($flog,"=======================msg========================\n");
		fwrite($flog, $strMessage ."\n");
		fwrite($flog, "==================================================\n");
		//test extraction part
		{
			$stringparts = explode(chr(0x1F), $msgpreview);
			
			$responseLength = intval($stringparts[0]);
			$response = "";
			
			fwrite($flog,"====================TEST=========================\n");
			if($responseLength > 0)
			{
				
				unset($stringparts[0]);
				
				foreach($stringparts as $part)
					fwrite($flog,$part);
					$response = implode(chr(0x1F), $stringparts);
					fwrite($flog,"msgtest response length was " . $responseLength . " , response was >" . $response."\n"); 
				
			}	
			fwrite($flog,"===================TESTEND=======================\n");
		}
		
		//todo:  enable this line to suppress the warning message that appears in the browser if the connection fails.
		//error_reporting(E_ALL ^ E_WARNING);
		$client = stream_socket_client("tcp://$strAddr:$strPort", $errno, $errorMessage, 1);
	
		if ($client === false)
		{
			fwrite($flog, $strMessage ." failedNoConnect\n");
			fclose($flog);
			return "failed|".$errorMessage;
			//throw new UnexpectedValueException("Failed to connect: $errorMessage");
		}
			$msgpreview = "" . $len .chr(0x1F) . $strMessage;
		 stream_set_timeout ( $client , 30  );
		fwrite($client, $msgpreview);
		fwrite($flog, $msgpreview ."\n");
		fflush($flog); //see what commands are locked up
		if($returnImmediately)
		{
			fclose($client);
			 return "0";
				 
		}
		$strReturn = stream_get_contents($client);
		if($strReturn === false)
		{
			fwrite($flog, $strMessage ." failed timeout\n");
				fclose($flog);
		fclose($client);
			return "failed_timeout|";
		}
		{
			$stringparts = explode(chr(0x1F), $strReturn);
			$responseLength = intval($stringparts[0]);
			$response = "";
			
			
			if($responseLength > 0)
			{
				
				unset($stringparts[0]);
				foreach($stringparts as $part)
					fwrite($flog,$part);
					$response = implode(chr(0x1F), $stringparts);
			}	
			fwrite($flog,"real response length was " . $responseLength . " , response was >" . $response ."\n"); 
			$strReturn = $response;
		}
		
			fclose($flog);
		fclose($client);
	
		return $strReturn;
	}
//}
?>