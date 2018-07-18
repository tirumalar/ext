<?php
//Start the session if it hasn't been started yet
//include_once("/home/www/debug.php");
//$firephp = FirePHP::getInstance(true);


if (!isset($_SESSION["SESSIONSTARTED"]))
{
    //error_log("session not started...");
    session_start();
    if(!isset( $_SESSION["LastInteractTime"] ))
       $_SESSION["LastInteractTime"] = time();
}
else if($_SESSION["SESSIONSTARTED"] == 0)
{
    //error_log("session not started is 0...");
    session_start();
      if(!isset( $_SESSION["LastInteractTime"] ))
       $_SESSION["LastInteractTime"] = time();
}

$currentTime = time();
//error_log("current time is ".$currentTime .  " last interact is " . $_SESSION["LastInteractTime"] . " session set is " . isset($_SESSION["SESSIONSTARTED"]));

//error_log("loggedin set is " . isset($_SESSION["LoggedIn"]));
//$firephp->log("check login");
//Check if the user has been logged in
if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
{
    //error_log("checklogin: not logged in ");
       $indexpath = $_SERVER['DOCUMENT_ROOT']."/index.php";
    //If he hasn't, send him back to the login page
 //   echo "<meta http-equiv='refresh' content='0;URL=/login.html'/>";
  //  die;
}
$cookieLogin = FALSE;

if ($currentTime - $_SESSION["LastInteractTime"] > 300 && isset($_SESSION["LoggedIn"]) &&  $_SESSION["LoggedIn"] == true)
{
      //error_log("checklogin: session timeout, logged in ");
       
    //Delete the session variables
    unset($_SESSION["LoggedIn"]);
    unset($_SESSION["UserName"]);
    //session_unset();
 
    //And send the user back to the homepage
    $indexpath = $_SERVER['DOCUMENT_ROOT']."/index.php";
    //headers already sent...
    //echo "<meta http-equiv='refresh' content='0;URL=/index.php'/>";
}

$_SESSION["LastInteractTime"] = time();

//Tell your program the session has been started. This will prevent some useless error messages
//define("SESSIONSTARTED", 1);
$_SESSION["SESSIONSTARTED"] = 1;
?>
