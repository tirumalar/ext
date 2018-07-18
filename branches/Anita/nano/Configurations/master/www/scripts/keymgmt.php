<?php

require("checklogin.php"); // Make sure user is logged on to get to this page...
include_once($_SERVER['DOCUMENT_ROOT']."/scripts/debug.php");
if (!isset($_SESSION["LoggedIn"]) || $_SESSION["LoggedIn"] == false)
{
    //error_log("checklogin: not logged in keymgmt");
    die;
}
class MyDB extends SQLite3
{
      function __construct()
      {
		  //chown for keys.db
		  NXTW_shell_exec("88"); //when we get this fast we serious *** 
		  
         $this->open('/home/root/keys.db');
      }
      function __destruct()
      {
	$this->close();
      }
}

if (isset($_REQUEST['action']))
{
    switch($_REQUEST['action'])
    {
        case 'loadallkeys':
	 {
	     echo 'loadallkeys:'.loadallkeys();
	     break;
	 }
	 case 'hostexists':
	 {
	     echo 'hostexists|'.checkHostExists($_REQUEST['hostname']);
	     break;
	 }
	 case 'addnewkey':
	 {
	     $bResult = AddNewKey($_REQUEST['hostname'], $_REQUEST['validity']);

            if ($bResult)
                echo "addnewkey|success";
            else
                echo "addnewkey|fail";
	     break;
	 }

	 case 'deleteallkeys':
	 {
	     $bResult = DeleteAllKeys();

            if ($bResult)
                echo "deleteallkeys|success";
            else
                echo "deleteallkeys|fail";
	     break;
	 }

	 case 'deletekey':
	 {
	     $bResult = DeleteKey($_REQUEST['id']);

            if ($bResult)
                echo "deletekey|success";
            else
                echo "deletekey|fail";
	     break;
	 }

	 case 'downloadkey':
	 {
	     echo "downloadkey|".DownloadKeyPFX($_REQUEST['id'], $_REQUEST['hostname']);
	     break;
	 }

	 case 'regeneratenanokey':
	 {
	     $bResult = RegenerateNanoKey();

            if ($bResult)
                echo "regeneratenanokey|success";
            else
                echo "regeneratenanokey|fail";
	     break;
	 }
	 
	 default:
        {
            echo "default|result done";
            break;
        }
    }
}

function loadallkeys()
{
   $result = "";
   $db = new MyDB();
   if($db){
	$sql ="SELECT * FROM Keys WHERE isDevice='1'";
   	$ret = $db->query($sql);
   	while($row = $ret->fetchArray(SQLITE3_ASSOC) ){
           $result = $row['isDevice'].'|'.$row['id'].'|'.$row['host'].'|'.$row['validity'];
	}
	$sql ="SELECT * FROM Keys WHERE isDevice='0'";
   	$ret = $db->query($sql);
   	while($row = $ret->fetchArray(SQLITE3_ASSOC) ){
           $result .= '\n'.$row['isDevice'].'|'.$row['id'].'|'.$row['host'].'|'.$row['validity'];
	}
   } else {
       $result = "fail";
   }
   return $result;
}

function checkHostExists($hostname)
{
   $db = new MyDB();
   if($db){
	$sql ="SELECT COUNT(*) FROM Keys WHERE host = '$hostname'";
   	$ret = $db->query($sql);
	$result = $ret->fetchArray(SQLITE3_ASSOC)['COUNT(*)'];
   } else {
       $result = "fail";
   }
   return $result;
}

function AddNewKey($hostname, $validity)
{
   //$strCmd = sprintf("cd /home/root;./KeyMgr -a -i -1 -n %s -d 0 -h %s;sync;killall -KILL Eyelock;", $validity, $hostname);


    $strCmd = sprintf("21".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s", escapeshellarg($validity), escapeshellarg($hostname), escapeshellarg($hostname.'.bin'));//"cd /home/root;./KeyMgr -c -n %s -d 0 -h %s -o %s.bin", $validity, $hostname, $hostname);
     NXTW_shell_exec($strCmd);
     $strCmd = sprintf("22".chr(0x1F)."%s".chr(0x1F)."%s".chr(0x1F)."%s", escapeshellarg($hostname), escapeshellarg($hostname.'.crt'), escapeshellarg($hostname.'.key'));//"cd /home/root;./KeyMgr -p -i -1 -d 0 -h %s -c %s.crt -k %s.key;sync;killall -KILL Eyelock;", $hostname, $hostname, $hostname);
      NXTW_shell_exec($strCmd);
     $strCmd = sprintf("23".chr(0x1F)."%s", escapeshellarg('/home/root/'.$hostname.'.crt'));//"cp /home/root/%s.crt /home/root/rootCert/certs",$hostname);
      NXTW_shell_exec($strCmd);
    $strCmd = sprintf("24".chr(0x1F)."%s" , escapeshellarg($hostname));
	//"cp /home/root/%s.key /home/root/rootCert/certs",$hostname);
     NXTW_shell_exec($strCmd);


  
   sleep(5);
   return true;
}

function DeleteAllKeys()
{
   $result = "";
   $db = new MyDB();
   if($db){
	$sql ="DELETE FROM Keys WHERE isDevice='0' and host != 'eyelock-pc'";
   	$status = $db->query($sql);
   	if($status)
	   $result = "success";  
   	else
 	   $result = "fail";
   } else {
       $result = "fail";
   }
   return $result;
}

function DeleteKey($id)
{
   $db = new MyDB();
   if($db){
	$sql ="DELETE FROM Keys WHERE id='$id'";
   	$result = $db->query($sql);
   	if($result){
   		$db->close();
        $strCmd = sprintf("25");//"cd /home/root;sync;killall -KILL Eyelock;");
        NXTW_shell_exec($strCmd);
		return "success";
	}else
 		return "fail";
   } else {
      return "fail";
   }
}

function DownloadKey($id, $hostname)
{
   $strCmd = sprintf("cd /home/root;./KeyMgr -g -d 1 -p %s -o %s", escapeshellarg($id), escapeshellarg($hostname.'.bin'));
   $result = shell_exec($strCmd);
   //return $result;
   //sleep(2);
   return '/home/root/'.$hostname.'.bin|'.$hostname.'.bin';
}

function DownloadKeyPFX($id, $hostname)
{
    
   $strCmd = sprintf("26");//"chmod 777 /home/www/scripts/makePFX.sh");
   $result = NXTW_shell_exec($strCmd);
   $strCmd = sprintf("27".chr(0x1F)."%s", escapeshellarg('/home/root/rootCert/certs/'.$hostname));//"/home/www/scripts/makePFX.sh /home/root/rootCert/certs/%s",  $hostname);
   $result = NXTW_shell_exec($strCmd);
   $strCmd = sprintf("28".chr(0x1F)."%s", escapeshellarg('/home/root/rootCert/certs/'.$hostname.'.pfx'));//"cp /home/root/rootCert/certs/%s.pfx /home/root/rootCert/certs/download.pfx",  $hostname);
   $result = NXTW_shell_exec($strCmd);



   //return $result;
   //sleep(2);
   return $hostname.'.pfx|'.$hostname.'.pfx';
}

function RegenerateNanoKey()
{
   $result=false;
   $db = new MyDB();
   if($db){
	$sql ="SELECT * FROM Keys WHERE isDevice='1'";
   	$ret = $db->query($sql);
   	$row = $ret->fetchArray(SQLITE3_ASSOC);
	$db->close();
    $strCmd = sprintf("29");//"cd /home/root;./KeyMgr -a -i 1 -n 8100 -d 1 -h nanoNxtDefault;sync;killall -KILL Eyelock;");
    NXTW_shell_exec($strCmd);
	$result = true;
   } else {
      $result=false;
   }
   return $result;
}

?>