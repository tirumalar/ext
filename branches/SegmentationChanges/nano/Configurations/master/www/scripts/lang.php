<?php
if (!isset($_SESSION["SESSIONSTARTED"]))
    session_start(); // For cookies

function setLanguageFile($lang)
{
	$langfile = fopen("/home/language", "w");
		fwrite($langfile, $lang);
		fclose($langfile);
}
function getLanguageFile()
{
	if(file_exists("/home/language"))
	{
	$langfile = fopen("/home/language", "r");
		$lang = fgets($langfile);
		fclose($langfile);
		return $lang;
	}
	else
	{
		return "us";
	}
}

if (!isset($_SESSION['lang']))
{
	$defaultLang = getLanguageFile();
//	error_log("default language is :".$defaultLang);
    $_SESSION['lang'] = $defaultLang;
    $currLang = $defaultLang;
}
else
{
    if ($_SESSION['lang'] != "")
    {
        // If user initiated reload with new language...
        if (isset($_GET['lang']))
        {
			//$GET['lang'] = filter_var($GET['lang'], FILTER_SANITIZE_STRING);
            $currLang = $_GET['lang'];
            $_SESSION['lang'] = $currLang;
        }
        else
            $currLang = $_SESSION['lang'];
    }
    else
    {
        $_SESSION['lang'] = "us";
        $currLang = "us";
    }
	//error_log("session language is :".$currLang);
}

// Ensure that our model data has correct language setup.
if (isset($model))
    $model->SetCurrentLanguageId($currLang);

switch($currLang)
{
    case "us":
        define("CHARSET","UTF-8");
        define("LANGCODE", "en");
		setLanguageFile("us");
        break;
	case "pt":
        define("CHARSET","UTF-8");
        define("LANGCODE", "pt");
		setLanguageFile("pt");
        break;

/*
    case "de":
        define("CHARSET","ISO-8859-1");
        define("LANGCODE", "de");
        break;
    case "ja":
        define("CHARSET","UTF-8");
        define("LANGCODE", "ja");
        break;
*/
    default:
        define("CHARSET","ISO-8859-1");
        define("LANGCODE", "en");
        break;
  }

header("Content-Type: text/html; X-Content-Type-Options: nosniff;charset=".CHARSET);
header("Content-Language: ".LANGCODE);
?>