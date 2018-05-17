<?php
    
require("checklogin.php"); // Make sure user is logged on to get to this page...
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");
if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
{
 //   error_log("checklogin: not logged in uploadcert");
    die;
}
$key = hex2bin('');
$iv = hex2bin('');

//unit test for encrypt/decrypt
//$teststring = "helloworld";
//$crypto = openssl_encrypt($teststring, "AES-128-CBC", $key, TRUE, $iv);
//$decrypto = openssl_decrypt($crypto,"AES-128-CBC", $key, TRUE, $iv);
//var_dump($crypto, $decrypto);


$target_dir = "/home/portableTemplateCerts/";
$password = $_POST["password"];
$target_file = $target_dir . basename($_FILES["0"]["name"]);
$uploadOk = 1;
$imageFileType = pathinfo($target_file,PATHINFO_EXTENSION);
// Check if image file is a actual image or fake image
//error_log("invoked uploadCertificate.php");
if (isset($_FILES["0"]) && $_FILES["0"]["error"] == UPLOAD_ERR_OK)
{
   
	//NXTW_shell_exec(sprintf("67".chr(0x1F)."%s", $target_dir));//"mkdir ".$target_dir);
	shell_exec(sprintf("mkdir %s", escapeshellarg($target_dir)));
    // Allow certain file formats
   // //error_log($_POST["password"]);
    //error_log("File uploaded ".$target_file);
    if($imageFileType != "pfx" ) {
        echo "4";
        $uploadOk = 0;
    }
    //error_log("File passed pfx test");
    // Check if $uploadOk is set to 0 by an error
    if ($uploadOk == 0) {
       // echo "3";
    // if everything is ok, try to upload file
    } else {
        //error_log("try load file from " . $_FILES["0"]["tmp_name"]);
        if (move_uploaded_file($_FILES["0"]["tmp_name"], $target_file)) {
             //error_log("upload done");
             $myfile = fopen("/home/portableTemplateCerts/selectedPFX.txt", "w");
             fwrite($myfile, $target_file);

             if($password == "")
                $password = "00";

             fwrite($myfile, "\r\n".$password);
           
             fclose($myfile);



           $encryptcmd = sprintf("68");
           $decryptcmd  = sprintf("69");

           NXTW_shell_exec($encryptcmd);
         //unit test only: decrypt the pfx file
           NXTW_shell_exec($decryptcmd);
           NXTW_shell_exec(sprintf("70".chr(0x1F)."%s", "/home/portableTemplateCerts/selectedPFX.txt"));//unlink("/home/portableTemplateCerts/selectedPFX.txt");
		    //remove the unencrypted version

            echo "1";

        } else {
             //error_log("upload failed");
            echo "0";
        }
    }
}
else
{
    //  error_log("post not set, check uploaded ".$target_file);
}
?>