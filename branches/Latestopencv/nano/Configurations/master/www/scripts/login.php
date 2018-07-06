<?php
require("passwd.php");

$logIn = false;
 
//Global variables
$username = 'root';
$password = 'root';



function CheckValidUsernameandPassword($strUser, $strPwd)
{
    //check if we have a login cookie
   

  if (ValidateUserPwd($strUser, $strPwd))
    {
        return true;
    }
    else
    {
        return false;
    }
}

//things start here
//TODO: add function to check for license files on the NXT.  There will be two:
//FactoryLicense and UserLicense.  Each will contain the 6 digit code.  The submit form from the licenseKey page will trigger a function that installs the
//UserLicense file.  This does not have to be done as Root so long as we can write a new file somewhere.

//how to do the redirect:
//If FactoryLicense does not exist, go to the login page.
//If FactoryLicense exists but the UserLicense does not, go to the License Page
//If both exist go to the login page
//we only need to check one thing


//First check the user input
if (get_magic_quotes_gpc())
{
    $username = stripslashes($_POST['username']);
    $password = stripslashes($_POST['password']);
}
else
{
    $username = $_POST['username'];
    $password = $_POST['password'];
}


// Add code here to check user/name password for Linux (try to login via shell_exec?  Check results?)
$logIn = CheckValidUsernameandPassword($username, $password);
$logindebug = sprintf("check password is %d", $logIn);
//error_log($logindebug);
 
//If the user can log in
if ($logIn)
{
    //Start the session and set the OK var to true
   
    session_start();
    $_SESSION["LoggedIn"] = true;

    if ($username === "installer")
        $_SESSION["UserName"] = "installer";
    else
        $_SESSION["UserName"] = "admin";
 
        $_SESSION["LastInteractTime"] = time();
    //And send the user to the private part of your website
    header("Location: /scripts/WebConfig.php");
    
}
else
{
    //If the user isn't logged in, send him back to the homepage
    header("Location: /index.php");
}
die;
?>
