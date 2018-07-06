<?php
    
require("checklogin.php"); // Make sure user is logged on to get to this page...
if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
{
    //error_log("checklogin: not logged in upgradeprox");
    die;
}
    // sanitize the input for a modicum of security...
    $path_parts = pathinfo($_REQUEST['url']);
    $file_name  = $path_parts['basename'];
    $file_path  = 'https://eyelock.com/updates/' . $file_name;

//    $xml = simplexml_load_file($file_path);

    $handle = fopen($file_path, "rb");
    $contents = '';
    while (!feof($handle)) {
      $contents .= fread($handle, 8192);
    }
    fclose($handle);

//    readfile($file_path);

    //echo "{$xml->{'version'}}";
    echo "{$contents}";
?>