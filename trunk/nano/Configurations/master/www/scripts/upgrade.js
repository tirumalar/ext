///////////////////////////////////////////////////////////////////
// Automatic upgrade functions to handle getting xml from website and parsing to determine if new upgrade
// avaialable or not...
//////////////////////////////////////////////////////////////////

// These globals hold our currently updated
// update objects if available...
var autoUpdateNanoInfo = new Object(),
    mandatory = false,
    packagefileurl = null,
    packagefilename = null,
    nanofilename = null,
    bobfilename = null,
    packageblob = null,
    details = null;

var bRefreshAfterUpgrade = false;

function lerp(a1, a2, b1, b2, a)
{
    return b1 + (b2 - b1) / (a2 - a1) * (a - a1);
}

function setProgress(step)
{
    var imgelement = document.getElementById("uploading");
    var imgdiv = document.getElementById("progressloader");
   imgelement.src = "/img/progressbar.png";
    var width = lerp(0, 10, 0, 250, step);
   imgelement.width = width;
   imgelement.height = 15;
   document.getElementById("uploading").setAttribute("style", "margin-left: 25px");
   document.getElementById("uploading").setAttribute("align", "left");
  
}
var upgradeInProgress = false;
var maxRestorePointCount = 4;
// Look download the xml from the cloud to check version info
function CheckUpgrade()
{
    return;
    // Reset before check
    if (autoUpdateNanoInfo) {
        mandatory = false;
        packagefileurl = null;
        packagefilename = null;
        nanofilename = null;
        bobfilename = null;
        packageblob = null;
        details = null;
    }

    // We don't check for the admin
    if (LoggedonUser !== "installer") {
         $("#upgradedlgversionlabel").hide();
         $("#upgradeversionlabel").hide();
          $("#upgradedlgversionlabel2").hide();
          $("#upgradeversionlabel2").hide();
        return;
    }


    // send request to well known URL to get XML describing current available upgrades.. Bob and nano Firmware
    // sndReqXML or something similar
    var oReq = new XMLHttpRequest();
    oReq.onload = reqUpgradeListener;

    var strURL = "";

    if (strUpdateURL == "")
        strURL = "https://eyelock.com/updates/nanonxtversioninfo.xml"; // default
    else
        strURL = strUpdateURL;

    oReq.open("get", strURL, true);
    oReq.send();
}



function reqUpgradeListener() {
    var xmlDoc;
    var NanoMode = "NoUpgradeAvailable";
    var BobMode ="BobNoUpgradeAvailable";

    if ((this.status == 200) || (this.status == 304)){

        // Create an XML parser for the DOM and parse the XML, then populate all of our tab controls...
        if (window.DOMParser) {
            parser = new DOMParser();
            xmlDoc = parser.parseFromString(this.responseText, "application/xml");
        }
        else // Internet Explorer
        {
            xmlDoc = new ActiveXObject("Microsoft.XMLDOM");
            xmlDoc.async = false;
            xmlDoc.loadXML(this.responseText);
        }


       // console.log(xmlDoc.getElementsByTagName("nanoversion")[0].childNodes[0].nodeValue);
       // console.log(xmlDoc.getElementsByTagName("bobversion")[0].childNodes[0].nodeValue);
       // console.log(xmlDoc.getElementsByTagName("description")[0].childNodes[0].nodeValue);

        // Read the XML and populate our autoUpdateNanoInfo object...

        // If it's a forced upgrade, we don't even need to compare version numbers...
        autoUpdateNanoInfo.mandatory = xmlDoc.getElementsByTagName("mandatory")[0].childNodes[0].nodeValue;

        if (autoUpdateNanoInfo.mandatory != "true") {
             if(  xmlDoc.getElementsByTagName("nanofilename")[0].childNodes[0] != null)
            NanoMode = CompareVersions(strAppVer, xmlDoc.getElementsByTagName("nanoversion")[0].childNodes[0].nodeValue, true);
            if(  xmlDoc.getElementsByTagName("bobfilename")[0].childNodes[0] != null)
                BobMode = CompareVersions(strBobVer, xmlDoc.getElementsByTagName("bobversion")[0].childNodes[0].nodeValue, false);
        }
        else {
            NanoMode = "UpgradeAvailable";
            BobMode = "BobUpgradeAvailable";
        }
    }
    else if (this.status == 404) // No upgrade file... means no upgrade available...
    {
        NanoMode = "NoUpgradeAvailable"; // essentially same as if versions match...
        BobMode = "BobNoUpgradeAvailable";
    }
    else
        NanoMode = "BadServer";

    // To update UI...
    // If both NanoMode and BobMode are both "NoUpgradeAvailable" then it's easy
    // If NanoMode = "BadServer" then it's easy...
    // If NanoMode = "upgradeavailable and/or "bobupgradeavaialble", then setup the UI appropriately


    if (NanoMode == "BadServer") {
        if (strUpdateURL == "")
            strURL = "https://eyelock.com/updates/nanoversioninfo.xml"; // default
        else
            strURL = strUpdateURL;

        // Update UI with info that version numbers could not be read...
        $("#upgradestatus").text(GlobalStrings.SoftwareUpdateStatusFailed);
        $("#upgradeversionlabel").hide();
        $("#upgradeversion").text(GlobalStrings.SoftwareUpdateCheckInternet);
        $("#upgradeversionlabel2").hide();
        var strServer = "URL: " + strURL;
        $("#upgradeversion2").text(strServer);

        // Also our popup
        $("#upgradedlgstatus").text(GlobalStrings.SoftwareUpdateStatusFailed);
        $("#upgradedlgversionlabel").hide();
        $("#upgradedlgversion").text(GlobalStrings.SoftwareUpdateCheckInternet);
        $("#upgradedlgversionlabel2").hide();
        $("#upgradedlgversion2").text(strServer);
    }
    else if ((NanoMode == "NoUpgradeAvailable") && (BobMode == "BobNoUpgradeAvailable")) {
        // Populate the html
        $("#upgradestatus").text(GlobalStrings.SoftwareUpdateStatusCurrent);
        $("#upgradeversionlabel").hide();
        $("#upgradeversion").text("");
        $("#upgradeversionlabel2").hide();
        $("#upgradeversion2").text("");

        // Also our popup
        $("#upgradedlgstatus").text(GlobalStrings.SoftwareUpdateStatusCurrent);
        $("#upgradedlgversionlabel").hide();
        $("#upgradedlgversion").text("");
        $("#upgradedlgversionlabel2").hide();
        $("#upgradedlgversion2").text("");

        $("#updatedescription").text("");
    }
    else if ((NanoMode == "UpgradeAvailable") || (BobMode == "BobUpgradeAvailable")) {
        var bNanoAvailable = false;
        var bBobAvailable = false;

        autoUpdateNanoInfo.packagefileurl = xmlDoc.getElementsByTagName("packagefileurl")[0].childNodes[0].nodeValue;
        autoUpdateNanoInfo.packagefilename = xmlDoc.getElementsByTagName("packagefilename")[0].childNodes[0].nodeValue;

        if (NanoMode == "UpgradeAvailable") {
            autoUpdateNanoInfo.nanofilename = xmlDoc.getElementsByTagName("nanofilename")[0].childNodes[0].nodeValue;

            if (autoUpdateNanoInfo.nanofilename != "")
                bNanoAvailable = true;
        }

        if (BobMode == "BobUpgradeAvailable") {
            autoUpdateNanoInfo.bobfilename = xmlDoc.getElementsByTagName("bobfilename")[0].childNodes[0].nodeValue;

            if (autoUpdateNanoInfo.bobfilename != "")
                bBobAvailable = true;
        }

        // Ok, we now know whether we are installing 1 or other or both... Update the UI
        if (bNanoAvailable || bBobAvailable) {
            $("#upgradestatus").text(GlobalStrings.SoftwareUpdateStatusNew);
            $("#upgradedlgstatus").text(GlobalStrings.SoftwareUpdateStatusNew);

            if (bNanoAvailable) {
                $("#upgradeversion").text(xmlDoc.getElementsByTagName("nanoversion")[0].childNodes[0].nodeValue);
                $("#upgradedlgversion").text(xmlDoc.getElementsByTagName("nanoversion")[0].childNodes[0].nodeValue);

            }
            else if (bBobAvailable) {
                $("#upgradeversionlabel").text($("#upgradeversionlabel2").text());
                $("#upgradeversion").text(xmlDoc.getElementsByTagName("bobversion")[0].childNodes[0].nodeValue);
                $("#upgradedlgversionlabel").text($("#upgradeversionlabel2").text());
                $("#upgradedlgversion").text(xmlDoc.getElementsByTagName("bobversion")[0].childNodes[0].nodeValue);
            }


            // Indicate that the bob upgrade is also available (if the nano is already...)
            if (bBobAvailable && bNanoAvailable) {
                $("#upgradeversion2").text(xmlDoc.getElementsByTagName("bobversion")[0].childNodes[0].nodeValue);
                $("#upgradedlgversion2").text(xmlDoc.getElementsByTagName("bobversion")[0].childNodes[0].nodeValue);
            }
            else {
                $("#upgradeversionlabel2").hide();
                $("#upgradeversion2").text("");
                $("#upgradedlgversionlabel2").hide();
                $("#upgradedlgversion2").text("");
            }


            $("#updatedescription").html(xmlDoc.getElementsByTagName("description")[0].childNodes[0].nodeValue).text();
//            $("#updatedescription").text(xmlDoc.getElementsByTagName("description")[0].childNodes[0].nodeValue);

            $("#updatenowbutton").show();

            autoUpdateNanoInfo.details = "these are details";//xmlDoc.getElementsByTagName("details")[0].childNodes[0].nodeValue;
            $("#detailsbutton").show();
        }
        else // shouldn't happen
        {
            $("#upgradestatus").text(GlobalStrings.SoftwareUpdateStatusCurrent);
            $("#upgradeversion").text("");
            $("#upgradeversion2").text("");

            $("#upgradedlgstatus").text(GlobalStrings.SoftwareUpdateStatusCurrent);
            $("#upgradedlgversion").text("");
            $("#upgradedlgversion2").text("");
        }

        // Automatically popup our upgrade dialog to give user a chance to upgrade Now!
        ShowUpgradeDialog();
    }
}



function CompareVersions(strCurrent, strUpgrade, bNano)
{
    var arCurrentNumber = new Array();
    var arCurrent = new Array();
    var arUpgrade = new Array();

    strCurrent = strCurrent.trim();
    strUpgrade = strUpgrade.trim();


    // Lop off the (AES) part if it exists...
    arCurrentNumber = strCurrent.split(" ");
    if (arCurrentNumber.length > 1)
        arCurrent = arCurrentNumber[1].split(".");
    else
        arCurrent = strCurrent.split(".");

    arUpgrade = strUpgrade.split(".");

    if ((arCurrent.length != arUpgrade.length) || (arCurrent.length < 3)) {
        
        return bNano ? "NoUpgradeAvailable" : "BobNoUpgradeAvailable";
    }

    // Grab the version numbers...
    var nMajorCurrent = parseInt(arCurrent[0]);
    var nMajorUpgrade = parseInt(arUpgrade[0]);
    var nMinorCurrent = parseInt(arCurrent[1]);
    var nMinorUpgrade = parseInt(arUpgrade[1]);
    var nBuildCurrent = parseInt(arCurrent[2]);
    var nBuildUpgrade = parseInt(arUpgrade[2]);

/*
    console.log("1 current");

    console.log(nMajorCurrent);
    console.log(nMinorCurrent);
    console.log(nBuildCurrent);

    console.log("1 upgrade");

    console.log(arUpgrade[0]);

    console.log(nMajorUpgrade);
    console.log(nMinorUpgrade);
    console.log(nBuildUpgrade);
*/

    // Compare the version numbers.
    if (nMajorUpgrade > nMajorCurrent) {
            //console.log("major");

        return bNano ? "UpgradeAvailable" : "BobUpgradeAvailable";
    }
    else if (nMajorUpgrade == nMajorCurrent) {
    if (nMinorUpgrade > nMinorCurrent) {
            //console.log("minor");

        return bNano ? "UpgradeAvailable" : "BobUpgradeAvailable";
    }
    else if (nMinorUpgrade == nMinorCurrent) {
    if (nBuildUpgrade > nBuildCurrent) {
            //console.log("build");

        return bNano ? "UpgradeAvailable" : "BobUpgradeAvailable";
    }
    }
    }

        //console.log("2");

    return bNano ? "NoUpgradeAvailable" : "BobNoUpgradeAvailable";
}




//////////////////////////////////////////////////////////////////////////////
// Manual upgrade file selection stuff...
//////////////////////////////////////////////////////////////////////////////
// Register our 'upgrade browse' button so that we can take some action...
$(document).ready(function()
{
    document.getElementById('manualnano').addEventListener('change', handleFileSelect, false);
});


// User clicked to Update button when a new update is avaialable...
// Could be either nano update or a bob update depending upon what's available...
$(document).ready(function () {
    $('#updatenowbutton').click(function () {
        //console.log('Clicked');
        $('#updatenowbutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
        //add a new check to prevent update if there are too many restore points
         ignoreTimeout = true;
//        var pointcount = CountRestorePoints();
//        if (pointcount > maxRestorePointCount) {  //delete points until the max is reached.
//            //var strmsg = "Too many restore points.  Cannot upgrade until points are deleted";
//            // ShowMessageBox("Too Many Restore Points", strmsg, "", "MB_ICONERROR", "OK", null);
//            //return;
//			//todo:  We're deleting the restorepoints in the script now. 
//	    // deleteOldestAvailableRestorePoint(pointcount - maxRestorePointCount);
//          //pointcount = CountRestorePoints();
//        }
        // Update nano if appropriate
        if (autoUpdateNanoInfo && ((autoUpdateNanoInfo.nanofilename != null) || (autoUpdateNanoInfo.bobfilename != null))) {
            //Time stamp added to URL to for 'no caching' on our download.
            DownloadFirmware(autoUpdateNanoInfo.packagefileurl + "/" + autoUpdateNanoInfo.packagefilename + "?&timestamp=" + new Date().getTime());
        }
    })
});


$(document).ready(function () {
    $('#detailsbutton').click(function () {
        //console.log('Clicked');
        $('#detailsbutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)


        // Update nano if appropriate
        // Display upgrade details in details dialog
        ShowUpgradeDialog();

//        $('#details-text1').text(autoUpdateNanoInfo.details);
//        ShowDetailsDialog();
    })
});




$(document).ready(function () {
    $('#upgrade_laterbtn').click(function () {
        HideUpgradeDialog();
    })
});


$(document).ready(function () {
    $('#upgrade_nowbtn').click(function () {
        HideUpgradeDialog();

        // Update nano if appropriate
        if (autoUpdateNanoInfo && ((autoUpdateNanoInfo.nanofilename != null) || (autoUpdateNanoInfo.bobfilename != null))) {
            DownloadFirmware(autoUpdateNanoInfo.packagefileurl + "/" + autoUpdateNanoInfo.packagefilename);
        }
    })
});


// If the user has successfully selected a file
// Call sndFile to start an ajax upload of the file to the nano
function handleFileSelect(evt) {
        var files = evt.target.files; // FileList object

         var pointcount = CountRestorePoints();
        if (pointcount > maxRestorePointCount) {

           // var strmsg = "Too many restore points.  Cannot upgrade until points are deleted";
           // ShowMessageBox("Too Many Restore Points", strmsg, "", "MB_ICONERROR", "OK", null);
           // return;
	   //S  deleteOldestAvailableRestorePoint(pointcount - maxRestorePointCount);
        }

        // DMOTODO!!!! VALIDATE FILE HERE BEFORE SENDING IT!!!!
        var data = new FormData();
        $.each(files, function (key, value) {
            data.append(key, value);
        });

        UploadNanoFirmware(data);
}



// Download the firmware into a blob from the eyelock site....
function DownloadFirmware(strPkgFilename)
{
    ShowWaitingDlg(GlobalStrings.FirmwareUpdateNanoTitle, GlobalStrings.FirmwareUpdateStatusDownload, false);


    var oReq = new XMLHttpRequest();

    oReq.addEventListener("progress", function (evt) {
        // For download update the percentage if available...
        if (evt.lengthComputable) {
            $('#loading-text-second').text(GlobalStrings.FirmwareUpdateStatusDownload + Math.ceil((evt.loaded * 100) / evt.total) + "%");
        }
    }, false);

    oReq.open("GET", strPkgFilename, true);
    oReq.responseType = "arraybuffer";

    oReq.onload = function(oEvent) {
        autoUpdateNanoInfo.packageblob = new Blob([oReq.response], {type: "application/x-tar"});
        var data = new FormData();

        data.append('0', autoUpdateNanoInfo.packageblob);
        data.append('packagefilename', autoUpdateNanoInfo.packagefilename);

        if (autoUpdateNanoInfo.nanofilename != null)
            data.append('nanofilename', autoUpdateNanoInfo.nanofilename);

        if (autoUpdateNanoInfo.bobfilename != null)
            data.append('bobfilename', autoUpdateNanoInfo.bobfilename);

        // Send the pkg file to the nano...
        UploadNanoFirmware(data);
       };

    oReq.send();

    upgradeInProgress = true;
////////////////////////////////////////////
// Ajax binary download not working correctly in this version...
///////////////////////////////////////////////
/**
    $.ajax({
        xhr: function () {
            var xhr = new window.XMLHttpRequest();
            xhr.overrideMimeType("application/octet-stream");
            xhr.addEventListener("progress", function (evt) {
                // For download update the percentage if available...
                if (evt.lengthComputable) {
                    $('#loading-text-second').text(Math.ceil((evt.loaded * 100) / evt.total) + "%");
                }
            }, false);

            return xhr;
        },
        type: 'GET',
        url: strPkgFilename,
        responseType: 'arraybuffer',
        cache: false,
        processData: false, // Don't process the files
     //   contentType: false, // Set content type to false as jQuery will tell the server its a query 
        complete: function (xhr, status) {
        },
        error: function (xhr, ajaxOptions, thrownError) {
            HideWaitingDlg();
            ShowMessageBox(GlobalStrings.FirmwareUpdateFailedTitle, thrownError, "", "MB_ICONERROR", "OK");
        },
        success: function (thedata, theStatus, xhr) {
            var data = new FormData();

            console.log(thedata.size);

            var arbuffer = new ArrayBuffer(thedata);
            //var thebuf2 = new Uint8Array(2);

            var thebuf = new Uint8Array(arbuffer);

            //var theblob = new Blob([thebuf], { type: 'application/octet-stream' });

            autoUpdateNanoInfo.packageblob = new Blob([thebuf], { type: 'application/octet-stream' });

            console.log("blobsize");
            //            var contentsize = xhr.getResponseHeader("Content-Length");
            //            console.log(contentsize);

            console.log(autoUpdateNanoInfo.packageblob.size);

            data.append('0', autoUpdateNanoInfo.packageblob);

            data.append('packagefilename', autoUpdateNanoInfo.packagefilename);

            if (autoUpdateNanoInfo.nanofilename != null)
                data.append('nanofilename', autoUpdateNanoInfo.nanofilename);

            if (autoUpdateNanoInfo.bobfilename != null)
                data.append('bobfilename', autoUpdateNanoInfo.bobfilename);


            // Send the pkg file to the nano...
            UploadNanoFirmware(data);
        }
    }); 

    */
}



function UploadNanoFirmware(data)
{
        var lastStatus = "";

        //We always append the current nano and ICM versions.
        data.append("currentnanoversion", strAppVer);
        data.append("currentbobversion", strBobVer);
         ignoreTimeout = true;
         upgradeInProgress = true;
        ShowWaitingDlg(GlobalStrings.FirmwareUpdateNanoTitle, GlobalStrings.FirmwareUpdateStatusUpload, false);

        $.ajax({
             cache: false,
            xhr: function () {
                var xhr = new window.XMLHttpRequest();

                xhr.addEventListener("loadstart", function (evt) {
                    $('#progressloader').show();
                }, false);

                xhr.addEventListener("loadend", function (evt) {
                    $('#progressloader').hide();
                }, false);

                xhr.addEventListener("progress", function (evt) {
                    if (evt.lengthComputable) {
                        $('#loading-text-second').text((evt.loaded * 100) / evt.total + "%");
                        $('#loading-text-second').show();
                    }


                    var response = evt.currentTarget.response;
                    var lines = response.split("|");
                    if (lines.length)
                        lastStatus = lines[lines.length - 1];
                    else
                        lastStatus = "";

                    var statusText = "";
                   // console.log(lastStatus);
                    switch (lastStatus) {
                        case "1":
                            {
                                $('#progressloader').hide();
                                statusText = GlobalStrings.FirmwareUpdateStatusUnpack;
                                setProgress(1);
                                break;
                            }

                        case "2":
                            {
                                statusText = GlobalStrings.FirmwareUpdateStatusValidating;
                                setProgress(2);
                                break;
                            }

                        case "3":
                            {
                                statusText = GlobalStrings.FirmwareUpdateCopyRequiredFiles; // Copy file to slave device
                                setProgress(3);
                                break;
                            }

                        case "4": // 
                            {
                                statusText = GlobalStrings.FirmwareUpdateStatusRestorePoint;
                                setProgress(4);
                                break;
                            }

                        case "5":
                            {
                                // Ok, the restore point should exist... let's reload...
                                sndReq('/scripts/rpc.php', 'getrestorepoints', "", false);

                                statusText = GlobalStrings.FirmwareUpdateStatusUpdating;
                                setProgress(5);
                                break;
                            }

                        case "6":
                            {
                                statusText = GlobalStrings.FirmwareUpdateStatusRestoreSettings;
                                setProgress(6);
                                break;
                            }


                        case "7":
                            {
                                statusText = GlobalStrings.FirmwareUpdateStatusValidatingBob;
                                setProgress(7);
                                break;
                            }


                        case "8":
                            {
                                statusText = GlobalStrings.FirmwareUpdateStatusRestorePointBob;
                                setProgress(8);
                                break;
                            }


                        case "9":
                            {
                                statusText = GlobalStrings.FirmwareUpdateStatusUpdatingBob;
                                setProgress(9);
                                break;
                            }

                            // Restarting eyelock...
                        case "10":
                            {
                                statusText = GlobalStrings.FirmwareUpdateComplete;
                                setProgress(10);
                                
                                break;
                            }
                        case "11":
                        	{
                        		statusText = "Validating nano NXT Patch ...";
                                setProgress(2);
                                break;
                        	}
                        case "12":
                        	{
                        		statusText = "Unpacking nano NXT Patch ...";
                                setProgress(5);
                                break;
                        	}
                        case "13":
                        	{
                        		statusText = "Applying Patch to nano NXT...";
                                setProgress(8);
                                break;
                        	}
                        case "14":
                        	{
                        		statusText = "Applied Patch Successfully ...";
                                setProgress(10);
                                break;
                        	}

                            // Restarting eyelock...
                            // case "6":
                            {
                                /**
                                alert("in 6");
                                // We now do all of this after the user clicks the OK button
                                $('#loading-text').text(GlobalStrings.WaitingForEyelockRestart);

                                statusText = "";

                                // Set UI to 'not running' state
                                $('#atomic').hide();
                                $('#imgpower').show();

                                $('#imageloader').attr({
                                'src': '/img/apploading.gif'
                                });

                                // Just start looking for the app to start running...
                                CheckforEyelockRunning(1000);
                                **/
                                break;
                            }
                            //case "bobProgress":


                        default:
                            {
                                statusText = "";

                                /**
                                // Update percent bar for upload portion of this monster...
                                if (oEvent.lengthComputable) {
                                var percentComplete = oEvent.loaded / oEvent.total;
                                // ...
                                } else {
                                // Unable to compute progress information since the total size is unknown
                                // Just continue to display the ajax loading bar...
                                }
                                **/
                                break;
                            }
                    }

                    if (lastStatus.indexOf("upgradeProgress") != -1)  //bobProgress|<some text from the ICM communicator>
                    {
                       // console.log(response);
                        //response was already split into the object Lines. lines[0] is "bobProgress", lines[1] is ICM communicator output
                        //there's a start indicator then the ICM will mark the register it wrote to from 33 to 133, then 255.  We'll use a simple
                        //scale for 33-133 equaling 0 to 100, ignoring other output.  
                        //lines we want look llike this:
                        //Programming row: 37 

                        //todo: lines are Programming Row: x of y
                        statusText = GlobalStrings.FirmwareUpdateStatusUpdating; //should set to "Processing frimware update"
                        var progressText = lines[1];
                        //if (progressText.indexOf("Programming row") != -1) 
						{
                            //lerp:  f(x) = f0 + (f1 - f0) /(x1-x0) * (x - x0)
                            var parts = progressText.split(":");
                            var xofy = parts[1].split("of");

                            var x = parseInt(xofy[0]);
                            // var y = parseInt(xofy[1]);
                            var complete = lerp(0, 100, 0, 100, x);
                            //clamp:  f(a0, a1, b)...  clamp = (a1 - b) / (a1 - a0)... if clamp > 1 clamp = a1, if clamp < 0 clamp = a0
                            var clamp = complete / 100;
                            if (clamp < 0)
                                complete = 0;
                            if (clamp > 1)
                                complete = 100;

                            setProgress(lerp(0, 100, 9, 10, complete));

                            //statusText = GlobalStrings.FirmwareUpdateStatusUpdatingBob + " " + complete.toString() + "% complete.";
                            // $('#loading-text-second').text(complete + "%");
                        }

                    }

                    if ((evt.lengthComputable && (evt.loaded < evt.total)) || !evt.lengthComputable)
                        $('#loading-text-second').html(statusText);
                }, false);

                xhr.upload.addEventListener("progress", function (evt) {
                    if (evt.lengthComputable) {
                        setProgress((evt.loaded * 10) / evt.total);
                       // $('#loading-text-second').text((evt.loaded * 100) / evt.total + "%");
                        $('#loading-text-second').show();
                    }
                }, false);

                return xhr;
            },

            type: 'POST',
            url: '/scripts/upgrade.php',
            data: data,
            processData: false, // Don't process the files
            contentType: false, // Set content type to false as jQuery will tell the server its a query 
            complete: function (xhr, status) {
                HideWaitingDlg();

                // Now we want to show our success/failure dialog...
                switch (lastStatus) {
                    case "success":
                        {
                            // Reset all text strings in Software Tab on success...
                            $("#upgradestatus").text(GlobalStrings.SoftwareUpdateStatusCurrent);
                            $("#upgradeversionlabel").hide();
                            $("#upgradeversion").text("");
                            $("#upgradeversionlabel2").hide();
                            $("#upgradeversion2").text("");

                            // Update most recent upgrade date... to today's date and time
                            $('#updatenowbutton').hide();
                            $('#detailsbutton').hide();
                            UpgradeFirmwareOK();
                            // ShowMessageBox(GlobalStrings.FirmwareUpdateTitle, GlobalStrings.FirmwareUpdateSuccess, GlobalStrings.FirmwareUpdateReload, "MB_ICONSUCCESS", "OK", UpgradeFirmwareOK);
                            break;
                        }


                        // On Failure we need to figure out what string to display        
                    default:
                        {
                            var strResult = "";

                            // Map the error code to an error string...
                            switch (lastStatus) {
                                //Unsupported File Type!                               
                                case "101":
                                    {
                                        strResult = GlobalStrings.FirmwareUpdateErrorBadFiletype;
                                        break;
                                    }

                                    //Failed to Move/Unpack uploaded firmware!
                                case "102":
                                    {
                                        strResult = GlobalStrings.FirmwareUpdateErrorUnpackFailed;
                                        break;
                                    }

                                    //ValidateUploadFiles Master Failed!
                                case "103":
                                    {
                                        strResult = GlobalStrings.FirmwareUpdateErrorValidateFailed;
                                        break;
                                    }

                                    //ValidateUploadFiles Slave Failed!
                                case "104": // validate nano file
                                case "109": // validate bob file
                                    {
                                        strResult = GlobalStrings.FirmwareUpdateErrorValidateFailed;
                                        break;
                                    }


                                    //CreateRestorePoint Failed!
                                case "105":
                                    {
                                        strResult = GlobalStrings.FirmwareUpdateErrorRestorePointFailed;
                                        break;
                                    }

                                    // Failed to extract and install firmware
                                case "106":
                                    {
                                        strResult = GlobalStrings.FirmwareUpdateErrorInstallFailed;
                                        break;
                                    }
									 case "166":
                                    {
                                        strResult = GlobalStrings.FirmwareUpdateErrorVersionInvalid;
                                        break;
                                    }

                                    // Failed to restore device settings after upgrade...
                                case "107":
                                    {
                                        strResult = GlobalStrings.FirmwareUpdateErrorDeviceRestoreFailed;
                                        break;
                                    }

                                    // BoB firmware update failed...
                                case "108":
                                    {
                                        strResult = GlobalStrings.FirmwareUpdateErrorBobInstallFailed;
                                        break;
                                    }

                                    // Failed to copy update to slave device...
                                case "110":
                                    {
                                        strResult = GlobalStrings.FirmwareUpdateErrorCopytoSlaveFailed;
                                        break;
                                    }

                                    // Failed to validate patch file...
                                case "111":
                                    {
                                        strResult = "Failed To Apply nano NXT patch.";
                                        break;
                                    }

                                default:
                                    {
                                        strResult = GlobalStrings.FirmwareUpdateErrorUnknownFailed;
                                        break;
                                    }

                            }
                            HideOnlyWaitingDlg();
                            strResult += " [Error: " + lastStatus + "]";
                            ShowMessageBox(GlobalStrings.FirmwareUpdateFailedTitle, GlobalStrings.FirmwareUpdateFailed, strResult, "MB_ICONERROR", "OK", UpgradeFirmwareOK);
                        }
                }
            },
            error: function (xhr, ajaxOptions, thrownError) {
                HideOnlyWaitingDlg();
                ShowMessageBox(GlobalStrings.FirmwareUpdateFailedTitle, thrownError, "", "MB_ICONERROR", "OK", UpgradeFirmwareOK);
            }
        }); 

    }


function handleFileSelectBob(evt) {
        var files = evt.target.files; // FileList object

        // DMOTODO!!!! VALIDATE FILE HERE BEFORE SENDING IT!!!!
        var data = new FormData();
        $.each(files, function (key, value) {
            data.append(key, value);
        });

        data.append("filetype", "bobfirmware");

        $.ajax({
             cache: false,
            url: '/scripts/upgrade.php',
            type: 'POST',
            data: data,
            cache: false,
            processData: false, // Don't process the files
            contentType: false // Set content type to false as jQuery will tell the server its a query 
        });
    }



function sndFile(filename)
{
    var strParameters = "";
    // dmotodo - filetype varies whether nano or bob firmware...
    var arParameters = [["uploadedfile", filename], ["filetype", "nanofirmware"]];

    for (var i = 0; i < arParameters.length; i++)
        strParameters += '&' + arParameters[i][0] + '=' + arParameters[i][1];

    filehttp.open("POST", "/scripts/upgrade.php?" + strParameters, true);
    filehttp.setRequestHeader("Content-type","multipart/form-data");
    filehttp.onreadystatechange = handleUpgradeResponse;

    filehttp.send(null);
}


function handleUpgradeResponse()
{
    if (filehttp.readyState == 4) // 4 means completely done.
    {
        var response = filehttp.responseText;

    }
}


function ShowUpgradeDialog()
{
    $("#loading-div-background").show();
    $("#upgrade-div-container").show();
}


function HideUpgradeDialog()
{
    $("#loading-div-background").hide();
    $("#upgrade-div-container").hide();
}


function ShowDetailsDialog()
{
    $("#loading-div-background").show();
    $("#details-div-container").show();
}


function HideDetailsDialog()
{
    $("#loading-div-background").hide();
    $("#details-div-container").hide();
}


// Ok, complete... let the user know we're restarting the app...
// This is actually a bit of a fake out... the app has likely already restarted...
function UpgradeFirmwareOK() {
    // Now I guess we Reboot...

    bRefreshAfterUpgrade = true; // force a refresh once the device comes back up...
    RebootDeviceOK();
/*
    // Just refresh the page...
   

    
*/
 //ShowWaitingDlg(GlobalStrings.RefreshingPage, "", true, 30000);
//location.reload(true);
/*
    // Set UI to 'not running' state
    $('#atomic').hide();
    $('#imgpower').show();

    $('#imageloader').attr({
        'src': '/img/apploading.gif'
    });

    // Just start looking for the app to start running...
    ShowWaitingDlg(GlobalStrings.WaitingForEyelockRestart, "", true, 30000);

    CheckforEyelockRunning(1000);
*/
}
