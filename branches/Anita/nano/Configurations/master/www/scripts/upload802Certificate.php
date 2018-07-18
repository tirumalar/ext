<?php
    
require("checklogin.php"); // Make sure user is logged on to get to this page...
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");
if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
{
   // error_log("checklogin: not logged in uploadcert");
    die;
}


$target_dir = "/home/802.1XCerts/";
$target_file = $target_dir . basename($_FILES["0"]["name"]);
$certType = $_POST["certType"];
$uploadOk = 1;
$imageFileType = pathinfo($target_file,PATHINFO_EXTENSION);

// Check if image file is a actual image or fake image
// error_log("invoked uploadCertificate.php");
if (isset($_FILES["0"]) && $_FILES["0"]["error"] == UPLOAD_ERR_OK)
{
	shell_exec(sprintf("mkdir %s", escapeshellarg($target_dir)));

    // Any filetype can be used for any key...
   // if ($certType !== "private_key")
   // {
   //     if ($imageFileType != "pfx" )
   //     {
    //        echo "4";
    //        $uploadOk = 0;
     //   }
    //}
   // else
    {
        if (($imageFileType != "pfx" ) && ($imageFileType != "pem" ) && ($imageFileType != "der" ))
        {
            echo "4";
            $uploadOk = 0;
        }
    }

    //error_log("File passed pfx test");
    // Check if $uploadOk is set to 0 by an error
    if ($uploadOk == 0)
    {
       // echo "3";
    // if everything is ok, try to upload file
    }
    else
    {
        //error_log("try load file from " . $_FILES["0"]["tmp_name"]);
        if (move_uploaded_file($_FILES["0"]["tmp_name"], $target_file))
        {
            ProcessCertsInfoFile($target_dir, "802Certs.txt", $target_file, $certType);
            echo "1";
        }
        else
        {
             //error_log("upload failed");
            echo "0";
        }
    }
}
else
{
   //   error_log("post not set, check uploaded ".$target_file);
}

function ProcessCertsInfoFile($thePath, $theInfoFile, $theFilename, $type)
{
    $CertNewContents = "";

    if (file_exists(sprintf("%s%s", $thePath, $theInfoFile)))
    {
        $bFoundEntry = FALSE;
        $CertFileContents = file_get_contents(sprintf("%s%s", $thePath, $theInfoFile));
        // Parse the file
        $arFileItems = explode("\n", $CertFileContents);

        for($i=0 ; $i<count($arFileItems) ; $i++)
        {
            $line = $arFileItems[$i];
            if (isset($line))
            {
                $arFileItem = explode("=", trim($line));

                // Parse out filepath and identify file
                if (isset($arFileItem[0]))
                {
                    // Update this one...
                    if ($type === trim($arFileItem[0]))
                    {
                        $bFoundEntry = TRUE;
                        $CertNewContents .= sprintf("%s=%s\n", $type, $theFilename);
                    }
                    else
                        $CertNewContents .= sprintf("%s\n", $arFileItems[$i]);
                }
            }
        }

        // Always add in if not already found...
        if (!$bFoundEntry)
            $CertNewContents .= sprintf("%s=%s\n", $type, $theFilename);

        // Overwrite the updated file...
        file_put_contents(sprintf("%s%s", $thePath, $theInfoFile), $CertNewContents);
    }
    else // just write this entry in the file... it's gonna be a brand new file...
    {
        $CertNewContents = sprintf("%s=%s\n", $type, $theFilename);
        file_put_contents(sprintf("%s%s", $thePath, $theInfoFile), $CertNewContents);
    }
}
?>