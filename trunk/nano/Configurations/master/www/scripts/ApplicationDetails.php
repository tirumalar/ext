<?php
// Class handles reading the device to determine the application details...

require("checklogin.php"); // Make sure user is logged on to get to this page...
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");

class ApplicationDetails
{
    ////////////////////////////////////////////////////
    // Values we store for display...
    ////////////////////////////////////////////////////
    public $AppFilename;
    public $AppVersion = "N/A"; 
    public $BobVersion = "N/A";
    public $BobHWVersion = "N/A";
    public $CameraFPGAVersion = "N/A";
    public $CameraPSOCVersion = "N/A";
    public $FixedBoardVersion = "N/A";
    //public $HardwareType = "0"; //set based on some kind of hardware flag, used to determine which functions are retained for each device.
    public $TOCBLEVersion = "N/A";
    public $TOCMainFWVersion = "N/A";
    public $TOCBootLoaderFwVersion = "N/A";
    public $TOCHWVersion = "N/A";
    public $TOCKeypadFWVersion = "N/A";
    public $TOCConfiguration = "N/A";

    public $ClientVersion = "1.5.27"; // UPDATE THIS VALUE FOR EACH NEW RELEASE!
    public $LinuxVersion = "4.0.0";
    public $PSOCVersion  = "0xFF";
    public $IsAppRunning;
    public $IsEyelockDotRunFilePresent;
    public $IsSQLLite = false;
    public $PreviousUpdateFailed = false;
    public $serialNumber = "0.0";

    // Constructor...
	function __construct ()
	{
        
       $this->AppFilename = "Eyelock";
       $this->LoadApplicationDetails();
	}
    function getPSOCVer()
    {
         $outPsocVer = NXTW_shell_exec("1"); //"chmod 777 /home/www/scripts/psocver.sh"
         $outPsocVer = NXTW_shell_exec("101");
            $outPsocVer = shell_exec("head /home/psocver.txt");
       //  $firephp->log('psoc version:');
       // $firephp->log($outPsocVer);
        $PSOCVersion = $outPsocVer;
        return $outPsocVer;
    }
    function getLinuxVer()
    {
         $tux = shell_exec("uname -a");
        
       // $firephp->log('linux version:');
       // $firephp->log($tux);

         $tuxTokens = explode(" ", $tux);
        //Linux nanoname version
        //0     1        2
        $LinuxVersion = $tuxTokens[2];
        return $LinuxVersion;
    }

    function getCameraFPGAVer()
    {
    }

    function getCameraPSOCVer()
    {
    }

    function getFixedBoardVer()
    {
    }

    function LoadApplicationDetails()
    {
        $cmdVersion = "";
        $cmdRunFile = "";
        $cmdForBobFile = "";
        $TOCReaderVersionFile = "";
        $hasTOC = FALSE;
		$serialFile = fopen("/home/root/id.txt", "r");
		if($serialFile)
		{	$this->serialNumber = fgets($serialFile);
		 	$this->serialNumber = str_replace("\n", "", $this->serialNumber);
		 	$this->serialNumber = str_replace("\r", "", $this->serialNumber);
			fclose($serialFile);
		}
         //linux version...................................................
       
        //$cmdEyelockFiles = shell_exec("[ ! -e /home/root/led_rgb.sh ] && echo 0 || echo 1;");
        $cmdGrep = shell_exec(sprintf("ps -e | grep %s", $this->AppFilename));
      
        // DMO we now assume everything is in home root
//        if ($cmdGrep === "1\n") //eyelock-user related files
        {
            $cmdVersion = shell_exec(sprintf("cd %s && %s -v", "/home/root/", escapeshellarg('./'.$this->AppFilename)));
          //  $cmdRunFile = shell_exec(sprintf("sh /home/root/led_rgb.sh %s", "runInfo"));
            //$cmdForBobFile = shell_exec("exec /home/root/GetVersion.sh");
            $cmdForBobFile = file_get_contents("/home/root/BobVersion");
            $cmdForOimFile = file_get_contents("/home/root/OimVersion");
	    $cmdForBobFile = $cmdForBobFile."\n".$cmdForOimFile;
            if(file_exists("/home/root/TOCReaderVersion"))
            {
                $hasTOC = TRUE;
              $TOCReaderVersionFile = file_get_contents("/home/root/TOCReaderVersion");
            }
        }


        $PreviousUpdateFailed =file_exists("/home/softwarerestore.txt");
       

//        else //root user related files
//        {
//           $cmdVersion = shell_exec(sprintf("%s%s -v", "/home/root/", $this->AppFilename));
//           $cmdForBobFile = shell_exec("ls -la /home/root/ | grep Eyelock.run");
//        }

        //$strResultMod = str_replace("|", "^", $cmdVersion);

        $strVersion = str_replace("\n", "|", $cmdVersion);
        $arVersion = explode("Version", $strVersion);
      
   //     $strResultMod = str_replace("[$", "|", $cmdVersion);
   //     $strVersion = str_replace("\n", "|", $strResultMod);
   //     $arVersion = explode("|", $strVersion);

        foreach ($arVersion as $key=>$theString)
        {
            if ($key === 1)
            {
           //     $arParseVersion = explode("[", $theString);

           //     foreach ($arParseVersion as $key2=>$theString2)
                {
                 //   if (/*($key2 === 0) &&*/ (isset($theString2)))
                    if ((isset($theString)))
                    {
                        $theTrimmedStringParts = explode("|", $theString);
			            $theTrimmedString = $theTrimmedStringParts[0];
                        $this->AppVersion = trim(str_replace("Version", "", $theTrimmedString));
                        break;
                    }
                }

                break;
            }
        }

        $pos = strpos($cmdGrep, $this->AppFilename);
        if ($pos !== false)
            $this->IsAppRunning = TRUE;
        else
            $this->IsAppRunning = FALSE;

        $this->IsEyelockDotRunFilePresent = $this->IsAppRunning;

        // Look at version # to determin SQLLite vs. Binary file database...
        $strTemp = trim(str_replace("(AES)", "", $this->AppVersion));

        $arVersionStr = explode(".", $strTemp);

        // We only look at the first two values of the version number...
        if (isset($arVersionStr[0]))
            $MajorVer = ltrim(trim($arVersionStr[0]), '0');
        else
            $MajorVer = 0;

        if (isset($arVersionStr[1]))
            $MinorVer = ltrim(trim($arVersionStr[1]), '0');
        else
            $MinorVer = 0;

        if (isset($MajorVer))
        {
            if (($MajorVer >= 3) || (($MajorVer == 2) && ($MinorVer > 8)))
                $this->IsSQLLite = TRUE;
        }
    
        // get bob version number
        $bFoundSoftware = FALSE;
        $bFoundHardware = FALSE;
        $bFoundCameraPSOC = FALSE;
        $bFoundCameraFPGA = FALSE;
        $bFoundFixedBoard = FALSE;

        $BobVersion = "N/A"; // default
        $BobHWVersion = "N/A";
        $CameraFPGAVersion = "N/A";
        $CameraPSOCVersion = "N/A";
        $FixedBoardVersion = "N/A";

        $strBobFileResults = str_replace("\n", ":", $cmdForBobFile);
        
        $arBobFileResults = explode(":", $strBobFileResults);
        //////error_log("bobcount".count($arBobFileResults));
        foreach ($arBobFileResults as $key=>$theString)
        {
            if ($bFoundSoftware)
            {
                $bFoundSoftware = FALSE;
                $this->BobVersion = trim($theString);
            }
            else if ($bFoundHardware)
            {
                $bFoundHardware = FALSE;
                $this->BobHWVersion = trim($theString);
            }
            else if ($bFoundCameraPSOC)
            {
                $bFoundCameraPSOC = FALSE;
                $this->CameraPSOCVersion = trim($theString);
            }
            else if ($bFoundCameraFPGA)
            {
                $bFoundCameraFPGA = FALSE;
                $this->CameraFPGAVersion = trim($theString);
            }
            else if ($bFoundFixedBoard)
            {
                $bFoundFixedBoard = FALSE;
                $this->FixedBoardVersion = trim($theString);
            }


            $pos = strpos($theString, "software version");
            if ($pos !== false)
            {
                $bFoundSoftware = true;
                continue;
            }

            $pos = strpos($theString, "hardware version");
            if ($pos !== false)
            {
                $bFoundHardware = true;
                continue;
            }

            $pos = strpos($theString, "FPGA VERSION");
            if ($pos !== false)
            {
                $bFoundCameraFPGA = true;
                continue;
            }

            $pos = strpos($theString, "Fixed board Verson");
            if ($pos !== false)
            {
                $bFoundFixedBoard = true;
                continue;
            }

            $pos = strpos($theString, "Cam Psoc Version");
            if ($pos !== false)
            {
                $bFoundCameraPSOC = true;
                continue;
            }
        }

        if($hasTOC){
                //parse TOCReaderVersionFile
               //  public $TOCBLEVersion = "N/A";
            //public $TOCMainFWVersion = "N/A";
            //public $TOCBootLoaderFwVersion = "N/A";
           // public $TOCHWVersion = "N/A";
           // public $TOCKeypadFWVersion = "N/A";
           // public $TOCConfiguration = "N/A";
           $foundBLE = FALSE;
           $foundMain = FALSE;
           $foundBLVersion = FALSE;
           $foundHWVersion = FALSE;
           $foundKeypadVersion = FALSE;
           $foundconfig = FALSE;
           ////error_log($TOCReaderVersionFile);
                 $strTOCReaderVersionFileResults = str_replace("\n", ":", $TOCReaderVersionFile);
                  ////error_log("n ".$strTOCReaderVersionFileResults);
                   $strTOCReaderVersionFileResults = str_replace("\r", ":", $strTOCReaderVersionFileResults);
                    ////error_log("r ".$strTOCReaderVersionFileResults);
                $arTOCReaderVersionFileResults = explode(":", $strTOCReaderVersionFileResults);
                ////error_log(count($arTOCReaderVersionFileResults));
                    foreach ($arTOCReaderVersionFileResults as $key=>$theString)
                    {
                        //////error_log($key);
                      //  ////error_log($theString);
                        if ($foundMain)
                        {
                             
                            $foundMain = FALSE;
                            $this->TOCMainFWVersion = trim($theString);
                              ////error_log("foundmain >". $this->TOCMainFWVersion );
                        }
                        else if ($foundBLE)
                        {
                            
                            $foundBLE = FALSE;
                            $this->TOCBLEVersion = trim($theString);
                             ////error_log("foundble >".$this->TOCBLEVersion);
                        }
                          else if ($foundHWVersion)
                        {
                           

                            $foundHWVersion = FALSE;
                            $this->TOCHWVersion = trim($theString);
                            ////error_log("foundhw >". $this->TOCHWVersion );
                        }
                        else if($foundBLVersion)
                        {
                                                       

                            $foundBLVersion = FALSE;
                            $this->TOCBootLoaderFwVersion = trim($theString);
                              ////error_log("foundBL >". $this->TOCBootLoaderFwVersion );
                        }
                          else if ($foundKeypadVersion)
                        {
                                                       

                            $foundKeypadVersion = FALSE;
                            $this->TOCKeypadFWVersion = trim($theString);
                              ////error_log("foundkey >".$this->TOCKeypadFWVersion);
                        }
                          else if ($foundconfig)
                        {
                            /*
                            Config parameter meaning
Maximum Allowed RF Baud Rate:  (BIT4 | BIT3)
            01: 106 KBits / Second
            10: 212 KBits  / Second
            11: 424 KBits / Second

Internal Use For WaveLynx Only:  BIT2

BLE Support:   BIT1
            0: Not supported 
            1: Supported

13.56 MHz Credential Support:   BIT0
            0: Not supported 
            1: Supported

                            */                             

                            $foundconfig = FALSE;
                            $this->TOCConfiguration = trim($theString);
                            ////error_log("foundconfig >".  $this->TOCConfiguration);
                        }

                        $pos = strpos($theString, "Main FW version");
                        if ($pos !== false)
                        {
                            $foundMain = true;
                            continue;
                        }
                        $pos = strpos($theString, "Bootloader FW version");
                        if ($pos !== false)
                        {
                            $foundBLVersion = true;
                            continue;
                        }
                        $pos = strpos($theString, "HW version");
                        if ($pos !== false)
                        {
                            $foundHWVersion = true;
                            continue;
                        }
                        $pos = strpos($theString, "BLE version");
                        if ($pos !== false)
                        {
                            $foundBLE = true;
                            continue;
                        }
                        $pos = strpos($theString, "Keypad FW version");
                        if ($pos !== false)
                        {
                            $foundKeypadVersion = true;
                            continue;
                        }

                        $pos = strpos($theString, "Configuration");
                        if ($pos !== false)
                            $foundconfig = true;
                    }
       
        }
            $outPsocVer = NXTW_shell_exec("2"); //"/psocver.sh"
            $outPsocVer = shell_exec("head psocver.txt");
     
        $PSOCVersion = $outPsocVer;
      
    
        //PSOC version....................................................
         $tux = shell_exec("uname -a");
        
      

         $tuxTokens = explode(" ", $tux);
        //Linux nanoname version
        //0     1        2
        $LinuxVersion = $tuxTokens[2];

        

    }
/*
        /// <summary>
        ///  -v
        /// </summary>
        /// returns : version of app
        public virtual string FindVersionOfApplication_ST(string p_RemoteFolder, string p_RemoteAppName)
        {
            string l_Version = string.Empty;

            string l_strResultForVersion = string.Empty;

            try
            {
                var connectionInfo = new PasswordConnectionInfo(OldIpOfBoard.ToString(), Username, Password);
                connectionInfo.Timeout = TimeSpan.FromSeconds(SshTimeout);
                using (var client = new SshClient(connectionInfo))
                {
                    client.Connect();

                    FireStepOcurred(new StatusMessage(string.Format("Connected in FindVersionOfApplication_ST, Board's Ip: {0}", OldIpOfBoard), StatusMessageTypes.Connected));

                    do
                    {
                        var cmdIfConfig = client.RunCommand(string.Format("cd {0};chmod a+x {1};./{1} -v", p_RemoteFolder, p_RemoteAppName));

                        l_strResultForVersion = cmdIfConfig.Result;
                    } while (l_strResultForVersion == "");
                    
                    client.Disconnect();

                    FireStepOcurred(new StatusMessage(string.Format("Disconnected in FindVersionOfApplication_ST, Board's Ip: {0}", OldIpOfBoard), StatusMessageTypes.Disconnected));

                }  // of using SshClient

                string l_strResultMod = l_strResultForVersion.Replace("|", "^");

                List<string> l_Lines = new List<string>(l_strResultMod.Replace("\n", "|").Split('|'));

                l_Version = l_Lines[1].Split('[')[0].Replace("Version", "").Trim();
            
            }
            catch(Exception ex)
            {
                l_Version = "N/A";

                FireStepOcurred(new StatusMessage(string.Format("Error in FindVersionOfApplication_ST, Board's Ip: {0}, ex: {1}", OldIpOfBoard, ex.Message), StatusMessageTypes.Error));

            }  // of try/catch

            return l_Version;

        }  // of FindVersionOfApplication()

*/
} // class
