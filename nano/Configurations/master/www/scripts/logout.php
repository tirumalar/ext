<?php
//First check if the user is logged in, this again prevents some error messages
include($_SERVER['DOCUMENT_ROOT']."/scripts/checklogin.php");
 
//Delete the session variables
unset($_SESSION["LoggedIn"]);
unset($_SESSION["UserName"]);

//session_unset();
 
//And send the user back to the homepage
$indexpath = $_SERVER['DOCUMENT_ROOT']."/index.php";
echo "<meta http-equiv='refresh' content='0;URL=/index.php'/>";
?>