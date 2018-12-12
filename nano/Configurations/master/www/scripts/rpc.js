///////////////////////////////////////////////////
// RPC Request and Response Handler Code
///////////////////////////////////////////////////

// This function sends our rpc request to the server...
// The action param defines what we want executed on the server
// side...

function sndReq(script, action, displaytext, bShowDialog, nDialogTimeout)
{
    var arParameters = [];

    nDefinedTimeout = typeof nDialogTimeout !== 'undefined' ? nDialogTimeout : 30000;

    sndReqParams(script, action, arParameters, displaytext, bShowDialog, nDefinedTimeout);
}

var showDialog;
var deleteFirmwareDialog = false;

function sndReqParams(script, action, arParameters, displaytext, bShowWaitDialog, nDialogTimeout)
{
    nDefinedTimeout = typeof nDialogTimeout !== 'undefined' ? nDialogTimeout : 30000;

    // Handle default value...
    bShowWaitDialog = typeof bShowWaitDialog !== 'undefined' ? bShowWaitDialog : true;
    // In here, depending up the 'action' we may want to display
    // wait dialogs or other such things to update the user
    // as to the status...
    showDialog=bShowWaitDialog;
    if (bShowWaitDialog)
        ShowWaitingDlg(displaytext, "", true, nDefinedTimeout);
    else if (typeof displaytext !== 'undefined')
    {
        if (displaytext != "")
            $("#loading-text").text(displaytext);
    }

    // In here, depending up the 'action' we may want to display
    // wait dialogs or other such things to update the user
    // as to the status...
    // Loop through the paramters to build up our full string
    // for each object in the array, add it to our list of params...
    var strParameters = "action=" + action;

    var arrayLength = arParameters.length;

    // securecomm is always sent...
    var securecomm = 'false';

    if ((strSecureComm === "true") || (strSecureComm === "secure")) // from eyelock.ini...
        securecomm = 'true';

    strParameters += '&' + 'securecomm' + '=' + securecomm;

    for (var i = 0; i < arrayLength; i++)
    {
        strParameters += '&' + arParameters[i][0] + '=' + arParameters[i][1];
    }

    $.ajax({
         cache: false,
        url: script + '?' + strParameters,
        type: 'GET',
        success: function (data, textStatus, jqXHR) {
            //data - response from server
            handleResponse(data);
        },
        error: function (jqXHR, textStatus, errorThrown) {
        //    console.log(jqXHR.statusText);
        }
    });
}

function sndReqWithTimeout(script, action, nTimeout)
{
    var arParameters = [];

    nDefinedTimeout = typeof nTimeout !== 'undefined' ? nTimeout : 5000;

    sndReqWithTimeout_Params(script, action, arParameters, nDefinedTimeout);
}

function sndReqWithTimeout_Params(script, action, arParameters, nTimeout)
{
    var strParameters = "action=" + action;
    var arrayLength = arParameters.length;

    // securecomm is always sent...
    var securecomm = 'false';

    if ((strSecureComm === "true") || (strSecureComm === "secure")) // from eyelock.ini...
        securecomm = 'true';

    strParameters += '&' + 'securecomm' + '=' + securecomm;

    for (var i = 0; i < arrayLength; i++)
    {
        strParameters += '&' + arParameters[i][0] + '=' + arParameters[i][1];
    }

    $.ajax({
         cache: false,
        url: script + '?' + strParameters,
        type: 'GET',
        timeout: nTimeout,
        success: function (data, textStatus, jqXHR) {
            //data - response from server
            handleResponse(data);
        },
        complete: function (xhr, status) {
        },
        error: function (jqXHR, textStatus, errorThrown) {
            handleError(action, textStatus);
        }
    });
}

function handleError(action, textStatus)
{
	if (textStatus === "timeout") {
		if(action==='getdb'){
			$('#templatesavailable').text(GlobalStrings.MsgUpdating);
			$('#templatesavailable2').text(GlobalStrings.MsgUpdating);
		}
	}
}

var templateTimeout, timeOutPeriod;

function CheckforDBTemplates(nTimeout) {
	if (typeof templateTimeout !== 'undefined')
		clearTimeout(templateTimeout);
    if (!upgradeInProgress) {

        timeOutPeriod = typeof nTimeout !== 'undefined' ? nTimeout : timeOutPeriod;

        sndReqWithTimeout('/scripts/rpc.php', "getdb", 10000);
    }
    templateTimeout = setTimeout(CheckforDBTemplates, timeOutPeriod);
}

function sndDeviceStatusReq(script, action, nTimeout)
{
    var arParameters = [];

    var strParameters = "action=" + action;

    var arrayLength = arParameters.length;

    // securecomm is always sent...
    var securecomm = 'false';

    if ((strSecureComm === "true") || (strSecureComm === "secure")) // from eyelock.ini...
        securecomm = 'true';

    strParameters += '&' + 'securecomm' + '=' + securecomm;

    for (var i = 0; i < arrayLength; i++)
    {
        strParameters += '&' + arParameters[i][0] + '=' + arParameters[i][1];
    }

    $.ajax({
         cache: false,
        url: script + '?' + strParameters,
        type: 'GET',
        timeout: nTimeout,
        success: function (data, textStatus, jqXHR) {
            //data - response from server
            handleResponse(data);
        },
        complete: function (xhr, status) {
        },
        error: function (jqXHR, textStatus, errorThrown) {
            if (textStatus === "timeout") {
                // Ok this is a timeout or some other error, tell the user that he's hosed... unless we're in the middle of a reboot...
                if (!upgradeInProgress) {//shut off the response during the update. this seems to block restarts.
                    if (!bRebootDevice && !ignoreTimeout)
                        ShowMessageBox(GlobalStrings.NanoDeviceStatus, GlobalStrings.NanoDeviceConnectionDown, GlobalStrings.NanoDeviceOkToReconnect, "MB_ICONERROR", "OK", GetDeviceStatusErrorOk, null);
                }
            }
        }
    });
}

function GetDeviceStatusErrorOk() {
    bStillWaitingForResponse = false;
    CheckDeviceStatus(5000); // Start checking the status again...
}



function GetDeviceStatusNotRunningOk() {
    ShowWaitingDlg(GlobalStrings.ReloadingPage, "", true, 30000);
    location.reload(true);
}


// When the server completes executing, it sends a response
// and this function gets called...  we do something with it...

var bMasterRunning = false;
var bSlaveRunning = false;

function handleResponse(theResponse)
{
    var response = theResponse;
    var update = new Array();

    if (response.indexOf('|') != -1)
    {
        update = response.split('|');

        if (update[0] === 'startapplication') {
            if (update[1] === 'success') {
                $('#atomic').show();
                $('#imgpower').hide();
            }
        }
        else if (update[0] === 'checkapplication') {
            nCheckEyelockRunningCount--;

            var aretheyrunning = new Array();

            aretheyrunning = update[1].split(',');

            bMasterRunning = (aretheyrunning[0] === 'masterrunning');
            bSlaveRunning = (aretheyrunning[1] === 'slaverunning');
			bSlaveRunning = bSlaveRunning || (HardwareType != '0');  //just to make sure.  If it's a CMX, the non existant Salve is always running
			//TODO: if this is a chamonix, don't check for slave running
            if (bMasterRunning && bSlaveRunning) {
                nCheckEyelockRunningCount = 0; //reset...
                bRebootDevice = false;


                $('#atomic').show();
                $('#imgpower').hide();
                $('#imageloader').attr({
                    'src': '/img/flowerloader.gif'
                });

                clearTimeout(SubmitTimeout);

                if (bRefreshAfterUpgrade) {
                    bRefreshAfterUpgrade = false;
                    bRefreshAttempt = true;
                    ShowWaitingDlg(GlobalStrings.RefreshingPage, "", true, 30000);
                    location.reload(true);
                }
                else {
                    if ('none' != ($('#loading-div-background').css('display'))) {
                        HideWaitingDlg();
                    }
                }
            }
            else {
                $('#atomic').hide();
                $('#imgpower').show();

                // Whoa.  We never saw both instances of Eyelock running after multiple tests...  this is a failure to restart case... warn the user...
                if (nCheckEyelockRunningCount <= 0) {
                    clearTimeout(SubmitTimeout);

                    if ('none' != ($('#loading-div-background').css('display')))
                        HideWaitingDlg();

                    var string1 = GlobalStrings.EyelockMasterStatus + " " + (bMasterRunning ? GlobalStrings.EyelockStatusRunning : GlobalStrings.EyelockStatusNotRunning);
					var string2 = "";
					if(HardwareType === '0')
                    	var string2 = GlobalStrings.EyelockSlaveStatus + " " + (bSlaveRunning ? GlobalStrings.EyelockStatusRunning : GlobalStrings.EyelockStatusNotRunning);

                    $('#imgpower').tooltipster('content', string1 + "</br>" + string2);

                    ShowMessageBox(GlobalStrings.EyelockApplicationStatus, string1, string2, "MB_ICONERROR", "OK", GetDeviceStatusNotRunningOk);
                }
            }
        }
        else if (update[0] === 'getdevicestatus') {
            bStillWaitingForResponse = false;

            var testrunning = new Array();

            testrunning = update[1].split(',');

            bMasterRunning = (testrunning[0] === 'masterrunning');
            bSlaveRunning = (testrunning[1] === 'slaverunning');
			bSlaveRunning = bSlaveRunning || (HardwareType != '0');
            var bTamper = (testrunning[2] === 'tampered');

            if (bMasterRunning && bSlaveRunning) {
                $('#atomic').show();
                $('#imgpower').hide();
                $('#imageloader').attr({
                    'src': '/img/flowerloader.gif'
                });
            }
            else {
                $('#atomic').hide();
                $('#imgpower').show();
            }

            if (bTamper) {
                $('.tampered').show();
            }
            else {
                $('.tampered').hide();

            }
            //if we're showing the message box for unable to talk to the device clear it
            if(  $("#msgboxtitletext").text === GlobalStrings.NanoDeviceStatus )
            {
                HideMessageBox();

                }
        }
        else if (update[0] === 'getdb') {
            var AvailSpace = 0;
            if (parseInt(MaxTemplateCount) - parseInt(update[1]) > 0)
                AvailSpace = (parseInt(MaxTemplateCount) - parseInt(update[1]));

            var theSpace = AvailSpace + " of " + MaxTemplateCount;

            $('#templatesavailable').text(theSpace);
            $('#templatesavailable2').text(theSpace);
        }
        else if (update[0] === 'getacstestdata') {
            var dbvalues = new Array();

            dbvalues = update[1].split("-");

            if (dbvalues.length == 2) {
				if(dbvalues[0] == "null")
					{
					$('#acstestfacility').hide();
					document.getElementById("testnowbutton").setAttribute("style","float:left; margin-top: -1em; margin-left: 2em;");
					//$('#testnowbutton').css("margin-top","-1em;");
					$('#acsTestFacilityLine').hide();
					}
				
				else
					{
					$('#acstestfacility').show();
					document.getElementById("testnowbutton").setAttribute("style","float:left; margin-top: -2em; margin-left: 2em;");
					$('#acsTestFacilityLine').show();
					}
                $('#acstestfacility').text(dbvalues[0]);
                $('#acstestcardid').text(dbvalues[1])
            }
        }
        else if (update[0].indexOf('acstest') != -1) {
            HideOnlyWaitingDlg();

            if (update[1] === 'failed') {
                ShowMessageBox(GlobalStrings.TCPConnectionFailed, GlobalStrings.ACSTestFailed, GlobalStrings.UnableToConnect, "MB_ICONERROR", "OK");
            }
        }
        else if (update[0].indexOf('nwmstest') != -1) {
            HideOnlyWaitingDlg();

            if (update[1] === '0') {
                ShowMessageBox(GlobalStrings.MessageBoxFailed, GlobalStrings.NwmsTestFailed,GlobalStrings.NwmsTestFailDetailNotFound, "MB_ICONERROR", "OK");
            }
            else if (update[1] === '1')
            {
                ShowMessageBox(GlobalStrings.MessageBoxSuccess, GlobalStrings.NwmsTestSuccess,GlobalStrings.NwmsTestSuccessDetail, "MB_ICONINFORMATION", "OK");
            }
            else
             ShowMessageBox(GlobalStrings.MessageBoxFailed, GlobalStrings.NwmsTestFailed, GlobalStrings.NwmsTestFailDetailInvalid, "MB_ICONERROR", "OK");
        }
        else if (update[0] === 'resetpassword') {
            HideOnlyWaitingDlg();

            if (update[1] === 'success')
                ShowMessageBox(GlobalStrings.ResetPasswordTitle, GlobalStrings.ResetPasswordSuccess, GlobalStrings.ResetPasswordLogout, "MB_ICONINFORMATION", "OK");
            else
                ShowMessageBox(GlobalStrings.ResetPasswordTitle, GlobalStrings.ResetPasswordFail, "", "MB_ICONINFORMATION", "OK");
        }
        else if (update[0] === 'updatetime') {
            
			if(!submitTimeSync)//don't hide this if we're in submit mode, we're still using the dialog
				HideOnlyWaitingDlg();
		
			//updatetime|{$strDate}
			//0| 1
            // Current time match is also 'success'
            if (update[1] == '0')
				{
					if(!submitTimeSync)//suppress the success message if issued from settings save
						ShowMessageBox(GlobalStrings.MessageBoxSuccess, GlobalStrings.DeviceTimeSynchronized, update[2], "MB_ICONINFORMATION", "OK");
					timeSubmitOK = true;
				}
            else
				{
					if(submitTimeSync)
						HideOnlyWaitingDlg(); //we can hide it now that we failed.
					timeSubmitOK = false;
					var updateFailReason = "unknown";
					switch(update[1])
						{
							case '2':
								updateFailReason = GlobalStrings.DEVICE_TIMESERVERFAIL_PING;
								break;
							case '3':
								updateFailReason = GlobalStrings.DEVICE_TIMESERVERFAIL_SYNC;
								break;
						}
                	ShowMessageBox(GlobalStrings.MessageBoxFailed, GlobalStrings.DeviceTimeSyncFailed, updateFailReason, "MB_ICONERROR", "OK");
				}
			timeSubmitComplete = true;
			
        }
	 else if (update[0] === 'updatelocaltime') {
	 	HideOnlyWaitingDlg();

            // Current time match is also 'success'
            if ((update[1] === '') || (update[1].indexOf("current time matches remote time") != -1))
                ShowMessageBox(GlobalStrings.MessageBoxSuccess, GlobalStrings.DeviceTimeSynchronized, update[2], "MB_ICONINFORMATION", "OK");
            else
                ShowMessageBox(GlobalStrings.MessageBoxFailed, GlobalStrings.DeviceTimeSyncFailed, update[1], "MB_ICONERROR", "OK");
	 }
        else if (update[0] === 'gettime') {
            // Current time match is also 'success'
            if (update[1] != "")
                $('#localtimespan').text(update[1]);
            else
                $('#localtimespan').text(GlobalString.LogUnavailable);
        }
        else if (update[0] === 'factoryreset') {
            bRefreshAttempt = true;
          // ShowWaitingDlg(GlobalStrings.LoggingOut, "", false); // Refresh will happen before this times out...  Failure will result in 'failed to connect dialog'...
            location.href = "/scripts/factoryResetLogout.php"; // log user out
        }
		else if (update[0] === 'osdpreset')
			{
				// RebootDeviceOK(); // now reboot...
            	//setTimeout(function () { location.reload(true); }, 120000);
				location.reload(true);
			}
        else if (update[0] === 'getrestorepoints') {
            // first element is nano restore points, second is bob restore points...
            var restorepoints = new Array();

            if (update.length > 1)
                restorepoints = update[1].split("^");

            var strNano = "";
            var strBob = "";

            if (restorepoints.length >= 1)
                strNano = restorepoints[0];

            if (restorepoints.legnth > 1)
                strBob = restorepoints[1];

            GenerateRestoreTable(strNano, strBob);

            // Hide the waiting dlg if it is shown...
            //   HideWaitingDlg();
        } 
        else if (update[0] === 'deleterestorepoints') {
            sndReq('/scripts/rpc.php', 'getrestorepoints', "", false);
            if(showDialog ||deleteFirmwareDialog )
			{
				deleteFirmwareDialog = false;
            	HideOnlyWaitingDlg();
			}
        }
        else if (update[0] === 'restorerestorepoint') {
			//update1 is the result
			result = update[1];
			//restore can fail if it targets an out of date file or if the file was not found (unlikely case though)
			if(result.indexOf("result Failed no longer supported") >=0)
				{
						ShowMessageBox(GlobalStrings.MessageBoxFailed, GlobalStrings.RestoreFailedNotSupported, "", "MB_ICONERROR", "OK");	
						HideOnlyWaitingDlg();
					return;
				}
				if(result.indexOf("result Failed could not find file")>=0)
				{
						ShowMessageBox(GlobalStrings.MessageBoxFailed, GlobalStrings.RestoreFailedNoFile,"", "MB_ICONERROR", "OK");	
						HideOnlyWaitingDlg();
					return;
				}
            $("#loading-text-second").text("");
            RebootDeviceOK(); // now reboot...
            setTimeout(function () { location.reload(true); }, 120000);
        }
        else if (update[0] === 'rebootdevice') {
            // Just start looking for the app to start running...
            nCheckEyelockRunningCount = 10; // Check for 1.5 minutes after reboot before failing...
            WaitforSystemReboot(35000);

            // Set UI to 'not running' state
            $('#atomic').hide();
            $('#imgpower').show();

            $('#imageloader').attr({
                'src': '/img/apploading.gif'
            });
        }
    }
}
var tlsModeChange = false;
var IEEEModeChange = false;

function sndSubmit(modeChange)
{
    tlsModeChange = modeChange;
    var params = $('#webconfig').serialize();

   $.ajax({
        cache: false,
        url: '/scripts/saveconfig.php',
        type: 'POST',
        data: params,
        success: function (data, textStatus, jqXHR) {
            //data - response from server
            handleSubmitResponse(data);
        },
        error: function (jqXHR, textStatus, errorThrown) {

        }
    });
}

function sndResetPassword(arParameters)
{
	$.ajax({
        cache: false,
        url: '/scripts/resetPassword.php',
        type: 'POST',
        data: arParameters,
        success: function (data, textStatus, jqXHR) {
            //data - response from server
            handleResetPasswordResponse(data);
        },
        error: function (jqXHR, textStatus, errorThrown) {

        }
    });
}

function handleResetPasswordResponse(response)
{
	if (response.indexOf('|') != -1)
	{
		var update = new Array();
		update = response.split('|');	
		if (update[0] === 'resetpassword') 
		{
			HideOnlyWaitingDlg();

			if (update[1] === 'success')
				ShowMessageBox(GlobalStrings.ResetPasswordTitle, GlobalStrings.ResetPasswordSuccess, GlobalStrings.ResetPasswordLogout, "MB_ICONINFORMATION", "OK");
			else
				ShowMessageBox(GlobalStrings.ResetPasswordTitle, GlobalStrings.ResetPasswordFail, "", "MB_ICONINFORMATION", "OK");
		}
	}
}


// When the server completes executing, it sends a response
// and this function gets called...  we do something with it...
var SubmitTimeout;

function handleSubmitResponse(theResponse)
{
    var response = theResponse;

    // For now, assume that Eyelock app has been restarted...
    // Change the text in the Waiting dialog (and the icon as well)
    // Start timer checking for apprestarted... once it's running... hide it...

    var update = new Array();
    if (response.indexOf('|' != -1))
    {
        update = response.split('|');

        if (update[0] === 'saveconfig')
        {
            if (update[1] === 'error') 
            {
                switch (update[2]) {
                    default:
                    case "802CertFail":
                        {
                            //HideOnlyWaitingDlg();

                            if (('none' != ($('#loading-div-background').css('display'))) && ('none' != ($('#loading-div-container').css('display')))) {
                                $("#loading-div-background").hide();
                                $("#loading-div-container").hide();
                            }

                           //ShowMessageBox(GlobalStrings.MessageBoxFailed, GlobalStrings.CertFailure, "", "MB_ICONERROR", "OK");
                            alert(GlobalStrings.CertFailure);

                            // All certs are deleted... fix it...
                            document.getElementById('CACertificateFile').value = GlobalStrings.NoCertFile;
                            document.getElementById('ClientCertificateFile').value = GlobalStrings.NoCertFile;
                            document.getElementById('ClientPrivateKeyFile').value = GlobalStrings.NoKeyFile;
                            document.getElementById('Eyelock_EnableIEEE8021X').checked = false;
//                            HideWaitingDlg();
                            bRefreshAttempt = true;
                            ShowWaitingDlg(GlobalStrings.RefreshingPage, "", true, 30000);
                            location.reload(true); // reload with same url...

                            break;
                        }
                    case "IPInUse":
                        {
                            //HideWaitingDlg();
                            if (('none' != ($('#loading-div-background').css('display'))) && ('none' != ($('#loading-div-container').css('display')))) {
                                $("#loading-div-background").hide();
                                $("#loading-div-container").hide();
                            }
                            alert(GlobalStrings.IpInUse);
                            //ShowMessageBox(GlobalStrings.MessageBoxFailed, GlobalStrings.IpInUse, "", "MB_ICONERROR", "OK");

                            //ShowWaitingDlg(GlobalString.ReloadingPage, "", true, 30000);
                            // location.reload(true);
                            break;
                        }
                }
            }
            else if (update[1] === 'refreshpageip') {
                //  THe php doesn't seem to respond with this message.
                // bStaticIPReboot = false; // cancels this mode (cuz we got a response...)
                // HideWaitingDlg();
                // bRefreshAttempt = true;
                //ShowWaitingDlg(GlobalStrings.RefreshingPage, "", true, 30000);
                // location.href = "http://" + update[2]; // reload with new url...
            }
            else if (update[1] === 'refreshpagedhcp') {
                //the php responds with this mesasge.  It triggers an early redirect!  since we're already waiting to redirect
                //let's just do nothing.  -Michael HEster
                // bStaticIPReboot = false; // reset for good measure
                // HideWaitingDlg();
                // bRefreshAttempt = true;
                //ShowWaitingDlg(GlobalStrings.RefreshingPage, "", true, 30000);
                //location.href = "http://" + update[2] + ".local"; // reload with new url...
            }
            else if (update[1] === 'refreshpagenetwork') {
            }
            else // assume success...
            {
                if (!tlsModeChange && !deviceNameChange && !IEEEModeChange) {
                    HideWaitingDlg();
                    bRefreshAttempt = true;
                    ShowWaitingDlg(GlobalStrings.RefreshingPage, "", true, 30000);
                    sleep(5000); // give the app a chance to restart
                    location.reload(true); // reload with same url...
                }
                //tls change: is going to reboot the device shortly... don't redirect.
                // Just start looking for the app to start running...
                //                nCheckEyelockRunningCount = 10;
                //                CheckforEyelockRunning(3000);
                // Set UI to 'not running' state
                //                $('#atomic').hide();
                //              $('#imgpower').show();

                //            $('#imageloader').attr({
                //              'src': '/img/apploading.gif'
                //        });
            }
        }
    }
}




function sleep(milliseconds) {
  var start = new Date().getTime();
  for (var i = 0; i < 1e7; i++) {
    if ((new Date().getTime() - start) > milliseconds){
      break;
    }
  }
}



function CheckDeviceStatus(nTimeout) {
   // If nTimeout is undefined, use the global
    if (!upgradeInProgress) { //shut off the ping watchdog during an update
        if (typeof nTimeout !== 'undefined')
            nCurrentStatusTimeout = nTimeout;

        if (!bStillWaitingForResponse) {
            bStillWaitingForResponse = true;
            sndDeviceStatusReq('/scripts/rpc.php', "getdevicestatus", 15000); // timeout for this request...
        }
    }
    setTimeout(CheckDeviceStatus, nCurrentStatusTimeout);
}


function CheckforEyelockRunning(nTimeout) {
    // If nTimeout is undefined, use the global
    if (!upgradeInProgress) {//shut off the db watchdog during an update
        if (typeof nTimeout !== 'undefined')
            nCurrentTimeout = nTimeout;

        if (ignoreTimeout)
            return;
        clearTimeout(SubmitTimeout);

        sndReq('/scripts/rpc.php', "checkapplication", GlobalStrings.WaitingForEyelockRestart, false);
    }
    SubmitTimeout = setTimeout(CheckforEyelockRunning, nCurrentTimeout);
}



function WaitforSystemReboot(nTimeout) {
    // If nTimeout is undefined, use the global
    if (typeof nTimeout !== 'undefined')
        nCurrentTimeout = nTimeout;

    clearTimeout(SubmitTimeout);

    bRebootDevice = true;
	ignoreTimeout = true;
   // sndReq('/scripts/rpc.php', "checkapplication", GlobalStrings.RebootingPleaseWait, false);
    ShowWaitingDlg(GlobalStrings.RebootingPleaseWait, "", true, 120000);
      setTimeout(function () { location.reload(true); }, 120000);//after 2 minutes do the redirect
    // The Waiting dlg is already up... change the text on line one and line 2...
    $("#loading-text").text(GlobalStrings.RebootingDeviceMessage);
    $("#loading-text-second").text(GlobalStrings.RebootingDeviceMessage2);


}


