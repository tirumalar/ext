<?php // logtail.php
 
require("checklogin.php"); // Make sure user is logged on to get to this page...
if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
{
    //error_log("checklogin: not logged in logtail");
    die;
}

    $strLogFile = "/home/root/nxtEvent.log";
    $strLogFile1 = "/home/root/nxtEvent.log.1";
    $tmpLog = "/home/root/tempNxtEvent.log";

	

	//$p = "\$p";

    //$cmd = "tail -n 5000 {$strLogFile}>$tmpLog";
    
    $cmd = "chmod 777 /home/www/scripts/preparelog.sh";

    exec("$cmd");

    // $cmd = "tail -n 5000 {$strLogFile1}>>$tmpLog";

      $cmd = "/home/www/scripts/preparelog.sh";
	//echo $cmd;
     exec("$cmd");

      $cmd = "tail -n 5000 {$tmpLog}";

       exec("$cmd 2>&1", $output);

    $linecount = 0;
    foreach ($output as $outputline)
    {
        echo ("$outputline\n");
        $linecount++;
    }
    //if we didn't load enough lines load from the next file.  IF the next file doesn't exist this should do nothing.
    //if($linecount < 5000)
   // {
        

      //  exec("$cmd 2>&1", $output1);
        
      //  foreach ($output1 as $outputline)
      //  {
       //     echo ("$outputline\n");
       //     $linecount++;
        //    if($linecount >= 5000)
        //        break;
       // }
   //}
?>
