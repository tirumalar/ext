/////////////////////////////////////////////////////////////////////
// Button click event handlers
/////////////////////////////////////////////////////////////////////

defaultKeyManagementState = false;
var tlsModeChange = false;
var submittingChanges = false;
var timeSubmitComplete = false;
var timeSubmitOK = false;
var currentTimeServer;
var bDuressChanged = false;
var bOldEnableRelay = false;

function finishSubmit()
{
     for (var i = 0; i < 9; i++)
                $("#tab-content" + i + "-label").css('background-image', 'none');
            //re-enable these for the transmit
            document.getElementById("Eyelock_TamperNotifyAddr").removeAttribute("disabled");
            document.getElementById("tamperinputsignalhigh").removeAttribute("disabled");
            document.getElementById("tamperinputsignallow").removeAttribute("disabled");
            // document.getElementById("tamperoutputsignalhigh").removeAttribute("disabled");
          //  document.getElementById("tamperoutputsignallow").removeAttribute("disabled");


            document.getElementById("Eyelock_TamperNotifyPort").removeAttribute("disabled");
            document.getElementById("Eyelock_TamperNotifyMessage").removeAttribute("disabled");
			ignoreTimeout = true; //all submits will refresh the browser.  we can turn off the timeout.
            sndSubmit(tlsModeChange); // POST all of our values to our saveconfig.php file for saving on the server...
}


$(document).ready(function () {
	$('#cancelbutton').click(function(){
		ShowWaitingDlg(GlobalStrings.ReloadingPage, "", true, 30000);
		location.reload(true);
	});
});

//handle certificate upload
$(document).ready(function()
{
    if(document.getElementById('certToUpload') != null)
        document.getElementById('certToUpload').addEventListener('change', handleCertFileSelect, false);

    if(document.getElementById('CAcertToUpload') != null)
        document.getElementById('CAcertToUpload').addEventListener('change', handleCACertFileSelect, false);

    if(document.getElementById('ClientcertToUpload') != null)
        document.getElementById('ClientcertToUpload').addEventListener('change', handleClientCertFileSelect, false);

    if(document.getElementById('PrivateKeyToUpload') != null)
        document.getElementById('PrivateKeyToUpload').addEventListener('change', handlePrivateKeyFileSelect, false);

});



$(document).ready(function() {
	if($.validator == null)return;
    $.validator.addMethod("pwcheck", function(value) {
   return (password.length > 7)  &&
	// If password contains both lower and uppercase characters, increase strength value.
	 (password.match(/([a-z].*[A-Z])|([A-Z].*[a-z])/)) &&
	// If it has numbers and characters, increase strength value.
	 (password.match(/([a-zA-Z])/) && password.match(/([0-9])/))&&
	// If it has one special character, increase strength value.
	 (password.match(/([!,%,&,@,#,$,^,*,?,_,~])/)) ; // has a digit
});
});
$(document).ready(function()
{
    if(document.getElementById('bootLoaderUpload') != null)
        document.getElementById('bootLoaderUpload').addEventListener('change', handleBootLoadFileSelect, false);
});
$(document).ready(function()
{
     $('#portableTemplatekeyMgmtDefault').click(function () {
        if(document.getElementById('portableTemplatekeyMgmtDefault').checked)
        {
              $('#certPassword').hide();
               $('#uploadCertButton').hide();
        }
        else
        {
             $('#certPassword').show();
               $('#uploadCertButton').show();
           
        }
    });
     $('#portableTemplatekeyMgmtCustom').click(function () {
        if(document.getElementById('portableTemplatekeyMgmtDefault').checked)
        {
              $('#certPassword').hide();
               $('#uploadCertButton').hide();
        }
        else
        {
             $('#certPassword').show();
               $('#uploadCertButton').show();
           
        }
    });
});



function handleCertFileSelect(evt) {
        var files = evt.target.files; // FileList object

        // DMOTODO!!!! VALIDATE FILE HERE BEFORE SENDING IT!!!!
        var data = new FormData();
        $.each(files, function (key, value) {
            data.append(key, value);
        });
       var certPassword =  document.getElementById('certPassword').value;
       var lastStatus = "";
       data.append("password", certPassword);

       $.ajax({
           xhr: function () {
               var xhr = new window.XMLHttpRequest();

               return xhr;
           },
            cache: false,
           type: 'POST',
           url: '/scripts/uploadCertificate.php',
           data: data,
           processData: false, // Don't process the files
           contentType: false, // Set content type to false as jQuery will tell the server its a query 
           complete: function (xhr, status) {
            
               response = xhr.responseText;
               switch (response) {
                   case "1": //success

                       ShowMessageBox("Upload success", "Uploaded pfx file successfully.  ", "The file is now selected as the portable template Key.", "MB_ICONSUCCESS", "OK");
                       document.getElementById('currentTemplateTag').innerHTML = "Custom Key";
                       break;
                   case "4":

                       ShowMessageBox("Upload Failed", "Uploaded file was not pfx file.   ", "Please upload a PFX file for the portable template key.", "MB_ICONVALIDATEFAILED", "OK");
                       break;
                   default:
                       break;
               }
               // Now we want to show our success/failure dialog...
               document.getElementById('certToUpload').value = "";
           },
           error: function (xhr, ajaxOptions, thrownError) {
            
               
               document.getElementById('certToUpload').value = "";
           }
       }); 
}



function handleCACertFileSelect(evt) {
        var files = evt.target.files; // FileList object

        // DMOTODO!!!! VALIDATE FILE HERE BEFORE SENDING IT!!!!
        var data = new FormData();
        $.each(files, function (key, value) {
            data.append(key, value);
        });

       data.append("certType", "ca_cert");

       $.ajax({
           xhr: function () {
               var xhr = new window.XMLHttpRequest();

               return xhr;
           },
            cache: false,
           type: 'POST',
           url: '/scripts/upload802Certificate.php',
           data: data,
           processData: false, // Don't process the files
           contentType: false, // Set content type to false as jQuery will tell the server its a query 
           complete: function (xhr, status) {
            
               response = xhr.responseText;
               switch (response) {
                   case "1": //success

                       ShowMessageBox("Upload success", "Uploaded certificate file successfully.  ", "The file is now selected as the IEEE 802.1X CA Certificate for this device.", "MB_ICONSUCCESS", "OK");
                       document.getElementById('CACertificateFile').value = evt.target.files[0].name;
                       //DMOTODO -- update the edit field here?
                       break;

                   case "4":
                       ShowMessageBox("Upload Failed", "Uploaded file was not valid certificate file.   ", "Please upload a CA Certificate for this device.", "MB_ICONVALIDATEFAILED", "OK");
                       break;

                   default:
                       break;
               }

               // Now we want to show our success/failure dialog...
               document.getElementById('CAcertToUpload').value = "";
           },
           error: function (xhr, ajaxOptions, thrownError) {
               document.getElementById('CAcertToUpload').value = "";
           }
       }); 
}



function handleClientCertFileSelect(evt) {
        var files = evt.target.files; // FileList object

        // DMOTODO!!!! VALIDATE FILE HERE BEFORE SENDING IT!!!!
        var data = new FormData();
        $.each(files, function (key, value) {
            data.append(key, value);
        });

       data.append("certType", "client_cert");

       $.ajax({
           xhr: function () {
               var xhr = new window.XMLHttpRequest();

               return xhr;
           },
            cache: false,
           type: 'POST',
           url: '/scripts/upload802Certificate.php',
           data: data,
           processData: false, // Don't process the files
           contentType: false, // Set content type to false as jQuery will tell the server its a query 
           complete: function (xhr, status) {
            
               response = xhr.responseText;
               switch (response) {
                   case "1": //success

                       ShowMessageBox("Upload success", "Uploaded client certificate file successfully.  ", "The file is now selected as the IEEE 802.1X Client Certificate for this device.", "MB_ICONSUCCESS", "OK");
                       document.getElementById('ClientCertificateFile').value = evt.target.files[0].name;
                       break;

                   case "4":
                       ShowMessageBox("Upload Failed", "Uploaded file was not valid certificate file.   ", "Please upload a Client Certificate for this device.", "MB_ICONVALIDATEFAILED", "OK");
                       break;

                   default:
                       break;
               }

               // Now we want to show our success/failure dialog...
               document.getElementById('ClientcertToUpload').value = "";
           },
           error: function (xhr, ajaxOptions, thrownError) {
               document.getElementById('ClientcertToUpload').value = "";
           }
       }); 
}


function handlePrivateKeyFileSelect(evt) {
        var files = evt.target.files; // FileList object

        // DMOTODO!!!! VALIDATE FILE HERE BEFORE SENDING IT!!!!
        var data = new FormData();
        $.each(files, function (key, value) {
            data.append(key, value);
        });

       data.append("certType", "private_key");

       $.ajax({
           xhr: function () {
               var xhr = new window.XMLHttpRequest();

               return xhr;
           },
            cache: false,
           type: 'POST',
           url: '/scripts/upload802Certificate.php',
           data: data,
           processData: false, // Don't process the files
           contentType: false, // Set content type to false as jQuery will tell the server its a query 
           complete: function (xhr, status) {
            
               response = xhr.responseText;
               switch (response) {
                   case "1": //success

                       ShowMessageBox("Upload success", "Uploaded private key file successfully.  ", "The file is now selected as the IEEE 802.1X private certificate key for this device.", "MB_ICONSUCCESS", "OK");
                       document.getElementById('ClientPrivateKeyFile').value = evt.target.files[0].name;
                       break;
                       

                   case "4":
                       ShowMessageBox("Upload Failed", "Uploaded file was not valid private key file.   ", "Please upload a Private Certificate Key for this device.", "MB_ICONVALIDATEFAILED", "OK");
                       break;

                   default:
                       break;
               }

               // Now we want to show our success/failure dialog...
               document.getElementById('PrivateKeyToUpload').value = "";
           },
           error: function (xhr, ajaxOptions, thrownError) {
               document.getElementById('PrivateKeyToUpload').value = "";
           }
       }); 
}


function submitComplete()
{
	 passwordokchange = true;

        if ($('#oldpassword').val() != '' || $('#newpassword').val() != '' || $('#confirmpassword').val() != '') {
            if ($('#passwordfields').data('bValidator').validate())
                $('#resetpwdbutton').click();
            else {
                passwordokchange = false;

            }

        }

        $('#webconfig').blur(); // Pop focus off... (doesn't do it on it's own for some reason)

      
        if ($('#webconfig').data('bValidator').validate() && passwordokchange) {
            // DMOTODO - clear all icons from all tabs that may or may not have them visible... on successful validation...
            // If user is actually changing the static IP, we need to handle the refresh differently...
            submittingChanges = true;

            tlsModeChange = (initialTLSConfigMode == "true" && document.getElementById('TLSRadio').checked == false) ||
            (initialTLSConfigMode == "false" && document.getElementById('TLSRadio').checked == true);

			deviceNameChange = (document.getElementById('DeviceName').value != strDeviceName);
			newDeviceName = document.getElementById('DeviceName').value;

            // Check to see if any 802 settings have changed.
            // Special case... we know that saving will fail for 802 if enabled and user has no certs setup...
            IEEEModeChange = ((InitialEnabled == "true" && document.getElementById('Eyelock_EnableIEEE8021X').checked == false) ||
                              (InitialEnabled == "false" && document.getElementById('Eyelock_EnableIEEE8021X').checked == true) ||
                              ((document.getElementById('Eyelock_EnableIEEE8021X').checked == true) &&
                                (((document.getElementById('CACertificateFile').value != GlobalStrings.NoCertFile) && (InitialCACertPath != document.getElementById('CACertificateFile').value)) ||
                                ((document.getElementById('ClientCertificateFile').value != GlobalStrings.NoCertFile) && (InitialClientCertPath != document.getElementById('ClientCertificateFile').value)) ||
                                ((document.getElementById('ClientPrivateKeyFile').value != GlobalStrings.NoKeyFile) && (InitialClientPrivateKeyPath != document.getElementById('ClientPrivateKeyFile').value)) ||
                                (InitialEAPOLVersion != document.getElementById('EAPVersion').value) ||
                                (InitialEAPIdentity != document.getElementById('EAPIdentityName').value) ||
                                (InitialEAPPrivateKeyPassword != document.getElementById('PrivateKeyPassword').value))));

                // Additional test here for simplicity...
            if (IEEEModeChange)
            {
                if ((document.getElementById('Eyelock_EnableIEEE8021X').checked == true) && 

                        (((document.getElementById('CACertificateFile').value == GlobalStrings.NoCertFile) &&
                        (document.getElementById('ClientCertificateFile').value == GlobalStrings.NoCertFile) &&
                        (document.getElementById('ClientPrivateKeyFile').value == GlobalStrings.NoKeyFile)) ||

                        // If there is no ClientCert but there is a Private key and the private key is a PFX file, then we flag as bad...
                        (((document.getElementById('ClientPrivateKeyFile').value != GlobalStrings.NoKeyFile) && (document.getElementById('ClientPrivateKeyFile').value.indexOf(".pfx") != -1) &&
                         (document.getElementById('ClientCertificateFile').value == GlobalStrings.NoCertFile))))
                   )
                {
                    IEEEModeChange = false; // Override if enabled but no certs uploaded.
                }
            }


            if (initialStaticIP != document.getElementById('ipofboard').value || initialStaticSubNetMask != document.getElementById('subnetmask').value || initialStaticGateway != document.getElementById('gateway').value || ipTypeChanged() || tlsModeChange || deviceNameChange || IEEEModeChange) {
                bStaticIPReboot = true;
                bRebootDevice = true;
                ignoreTimeout = true;
                //test reboot time on my device (how long it was down) is about 1 minute 5 seconds.
                if (tlsModeChange || deviceNameChange || IEEEModeChange) {
                    ShowWaitingDlg(GlobalStrings.SavingSettings, "", true, 1500000);
                    setTimeout(function () { RebootDeviceOK(); }, 5000);
                }
                else
                    ShowWaitingDlg(GlobalStrings.SavingSettingsRestart, GlobalStrings.FewMoments, true, 90000);
            }
            else
                ShowWaitingDlg(GlobalStrings.SavingSettings, "", true, 30000);


            applyKeyMgmtChanges();
            // Clear the failed validation icon on any and all tabs...
        }
        else { // Validation failed, tut tut.
            // Then show and icon in each tab for each failed element
            ShowMessageBox(GlobalStrings.ValidationFailedTitle, GlobalStrings.ValidationFailedMsg1, GlobalStrings.ValidationFailedMsg2, "MB_ICONVALIDATEFAILED", "OK");
        }
}


var intervalID;
function verifyTimeSubmit()
{
	if(timeSubmitComplete)
				{
					clearInterval(intervalID);
					if(timeSubmitOK)
						{
							 submitComplete(); //time submit was ok, lets finish this
						}
				}
	else
		{
			
		}
}

function submitPostIpDuplicateCheck() {
    if(currentTimeServer != document.getElementById('GRI_InternetTimeAddr').value && !timeSubmitOK)
    {
        submitTimeSync = true;
        timeSubmitOK = false;
        timeSubmitComplete = false;
        updateTime();
        
        intervalID = setInterval(verifyTimeSubmit, 2000); //wait for timesync to finish
    }
    else
    {
        submitComplete();
    }

    // reset some globals...
    bDuressChanged = false;
    bOldEnableRelay = false;
}

// SUBMIT BUTTON - give a little feedback...
$(document).ready(function () {
    $("#webconfig").submit(function (event) {
        event.preventDefault();
        if (isCurrentlyStaticIP === 1 && (initialStaticIP != document.getElementById('ipofboard').value)) {
            sndReqParams('/scripts/rpc.php', 'checkipaddressduplicate', [['ipaddress', document.getElementById('ipofboard').value]], GlobalStrings.CheckingIpAddressDuplicate, true);
        } else {
            submitPostIpDuplicateCheck();
        }
    });

    for (var i = 0; i < 9; i++) {
        $("#tab" + i).attr("onfocus", "$('#tab" + i + "').parent().addClass('tabOutline');");
        $("#tab" + i).attr("onblur", "$('#tab" + i + "').parent().removeClass('tabOutline');");
    }
    $("#manualnano").attr("onfocus", "$('#manualnano').parent().addClass('outline');");
    $("#manualnano").attr("onblur", "$('#manualnano').parent().removeClass('outline');");
});





// RESET PASSWORD BUTTON
$(document).ready(function () {
    // DMOTODO -- Add in all the other parameters that must be passed back...
    // Should also force a parsely validation here before sending the request!!!!
    // Actually, use parsely to ensure that the button is disabled unless everything is correct...
    // so it can't even be clicked.
	
    $('#resetpwdbutton').click(function () {
        $('#resetpwdbutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)

        var userValue = "";

        if ($('#passwordfields').data('bValidator').validate()) {
            // Force siteadmin if not logged on as installer
            if ('none' == ($('#passworduser').css('display')))
                userValue = "admin";
            else {
                var cSelect = document.getElementById("logintype");

                userValue = cSelect.options[cSelect.selectedIndex].value;
            }

            var cSelect = document.getElementById("logintype");

		    var resetPasswdData = { "user": userValue, "oldpwd": document.getElementById('oldpassword').value, "newpwd":document.getElementById('newpassword').value };
            sndResetPassword(resetPasswdData);
			
            return true;
        }
        else {
             ShowMessageBox(GlobalStrings.ResetPasswordTitle, GlobalStrings.ResetPasswordFail, "", "MB_ICONINFORMATION", "OK");
            return false;
        }
    });
	
	var tsele = document.getElementById('GRI_InternetTimeAddr');
	if(tsele == null) return;
	currentTimeServer = tsele.value;
});
/*
function databaseTabShowTemplatesRemaining()
{
    if(document.getElementById('GRI_EnableNWHDMatcher') == null ||  document.getElementById("GRITrigger_TemplateOnCard") == null)
        return;
     if(document.getElementById('GRI_EnableNWHDMatcher').checked || document.getElementById("GRITrigger_TemplateOnCard").checked == true)
        {
              $('#databaseTemplatesAvailable').hide();
              $('#storageStatsNormal').hide();
               $('#storageStatsPT').show();
              
        }
        else
        {
             $('#storageStatsPT').hide();
             $('#storageStatsNormal').show();
             $('#databaseTemplatesAvailable').show();
        }
}
*/
function databaseTabShowTemplatesRemaining()
{
    if(document.getElementById('GRI_EnableNWHDMatcher') == null ||  document.getElementById("GRITrigger_TemplateOnCard") == null)
        return;
     if(document.getElementById('GRI_EnableNWHDMatcher').checked)
        {
              $('#databaseTemplatesAvailable').hide();
           
              
        }
        else
        {
           
            
        }

        if( document.getElementById("GRITrigger_TemplateOnCard").checked == true)
        {
			 $('#databaseTemplatesAvailable').hide();
               $('#storageStatsNormal').hide();
               $('#storageStatsPT').show();
        }
        else
        {
			 $('#databaseTemplatesAvailable').show();
             $('#storageStatsPT').hide();
             $('#storageStatsNormal').show();
        }

}

$(document).ready(function () {
      databaseTabShowTemplatesRemaining(); //on initialize of form

});

//GRI_EnableNWHDMatcher
$(document).ready(function () {
    $('#GRI_EnableNWHDMatcher').click(function () {
        databaseTabShowTemplatesRemaining();  //when user clicks the check box on Enable Network Matcher
        setEnabledTestNWMSButton();
    });
});

$(document).ready(function () {
	timeSubmitOK = false;
    $('#keyMgmtDefault').click(function () {
        //$('#keymgmtbutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
        //$("#loading-keymgmtdiv-background").show();
        //$("#keymgmt-div-container").show();
        //loadAllKeys();
        //addNewKeyOK();
      //  deleteAllKeysOK();
        //  $('#keymgmtbutton').hide();
        document.getElementById('keyMgmtDefault').checked = true;
         document.getElementById('keyMgmtCustom').checked = false;
    });
});

function applyKeyMgmtChanges()
{
    acted = false;
    if( document.getElementById('keyMgmtDefault').checked == true && defaultKeyManagementState == false)
       {
           deleteAllKeysOK();
           acted = true;
       }
       if(document.getElementById('keyMgmtCustom').checked == true && defaultKeyManagementState == true)
         {
             addNewKeyOK();
             acted = true;
         }
         if(!acted)
            finishSubmit();
}

$(document).ready(function () {
    $('#keyMgmtCustom').click(function () {
	//$('#keymgmtbutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
	//$("#loading-keymgmtdiv-background").show();
    	//$("#keymgmt-div-container").show();
	//loadAllKeys();
    // addNewKeyOK();
     //deleteAllKeysOK();
      // $('#keymgmtbutton').show();
       document.getElementById('keyMgmtDefault').checked = false;
         document.getElementById('keyMgmtCustom').checked = true;
    });
});
function updateDefaultKeyManagementState()
{
     defaultKeyManagementState = (document.getElementById('keyMgmtDefault').checked == true);

}
// KEY MANAGEMENT BUTTON
$(document).ready(function () {

    //check how many keys we have. if 3.. show the radios for custom, else show for default
    loadAllKeys();
    setTimeout(updateDefaultKeyManagementState, 3000);


});

// KEY MANAGEMENT BUTTON
$(document).ready(function () {
    $('#keymgmtbutton').click(function () {
        //	$('#keymgmtbutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
        //$("#loading-keymgmtdiv-background").show();
        //$("#keymgmt-div-container").show();
        //loadAllKeys();
         downloadKey(3, getHostName());
    });
   
});

// KEY MANAGEMENT CLOSE BUTTON
$(document).ready(function () {
    $('#keymgmt_close_button').click(function () {
	$("#loading-keymgmtdiv-background").hide();
    	$("#keymgmt-div-container").hide();
    });
});

// ADD HOST KEY BUTTON
$(document).ready(function () {
    $('#addkeybutton').click(function () {
        $('#addkeybutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
        //  Show_AddKeyMsgBox(GlobalStrings.AddKeyTitle, GlobalStrings.AddKeyMessage, "", "MB_ICONINFORMATION", "OKCANCEL", addNewKeyOK, null);
        //$("#msgbox-text2").html(GlobalStrings.AddKeyControls);
        addNewKeyOK();
    });
});

var hostName,validity;
function checkHostName() {
    hostName = $('#keyHostName').val();
    if(hostName=='') {
	$('#keyHostName').css('border','1px solid red');
	return true;
    }else{
	$('#keyHostName').css('border','1px solid gray');
	return false;
    }
}

function checkValidity() {
    validity = $('#keyValidPeriod').val();
    if(validity<5||validity>7305){
	$('#keyValidPeriod').css('border','1px solid red');
	return true;
    }else{
	$('#keyValidPeriod').css('border','1px solid gray');
	return false;
    }
}

function loadAllKeys()
{
	keymgmtReq('loadallkeys', 'undefined');
}
function getHostName()
{
    return $('#home_devicename').text().split(':')[1].split('.')[0];
}
function addNewKeyOK()
{
	//keymgmtReq('hostexists',hostName);
    hostName = getHostName();
    validity = 7305; //won't always be 5 leap years in here
    var arParameters = [["hostname", hostName]];
    	keymgmtReqParams('hostexists', arParameters,'undefined');
}

function addNewKey()
{
 	var arParameters = [["hostname", hostName],
                           ["validity", validity]];
    	keymgmtReqParams('addnewkey', arParameters, GlobalStrings.AddingNewKey);
}

// DELETE ALL HOST KEYS BUTTON
$(document).ready(function () {
    $('#deleteallkeysbutton').click(function () {
        //$('#deleteallkeysbutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
        // ShowMessageBox(GlobalStrings.DeleteAllKeyTitle, GlobalStrings.DeleteAllKeyMessage, GlobalStrings.AreYouSure, "MB_ICONWARNING", "OKCANCEL", deleteAllKeysOK, null);
        deleteAllKeysOK();
    });
});

function deleteAllKeysOK()
{
	keymgmtReq('deleteallkeys', 'undefined');
}

var delKeyID;
function deleteKey(keyID)
{
	delKeyID = keyID;
	ShowMessageBox(GlobalStrings.DeleteKeyTitle, GlobalStrings.DeleteKeyMessage, GlobalStrings.AreYouSure, "MB_ICONWARNING", "OKCANCEL", deleteKeyOK, null);
}

function deleteKeyOK()
{
	var arParameters = [["id", delKeyID]];
    	keymgmtReqParams('deletekey', arParameters, GlobalStrings.DeletingKey);
}

function downloadKey(keyID, host)
{
	var arParameters = [["id", keyID],["hostname",host]];
    	keymgmtReqParams('downloadkey', arParameters, GlobalStrings.DownloadingKey);
}

function regenerateNanoKey()
{
	ShowMessageBox(GlobalStrings.RegenerateNanoKeyTitle, GlobalStrings.RegenerateNanoKeyMessage, GlobalStrings.AreYouSure, "MB_ICONWARNING", "OKCANCEL", regenerateNanoKeyOK, null);
}

function regenerateNanoKeyOK()
{
	keymgmtReq('regeneratenanokey', GlobalStrings.RegeneratingNanoKey);
}

// UPDATE FIRMWARE BUTTON
/**
$(document).ready(function () {
    $('#updatenowbutton').click(function () {
        $('#updatenowbutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
        var arParameters = [["tarfilename", "/home/root/testtar.txt"],
                            ["tarfiledata", "write this into the file"]
                            ];
        sndReqParams('/scripts/firmwareupdate.php', 'upgradefirmware', arParameters);
    });
});
**/


 // POWER BUTTON
 $(document).ready(function () {
     $('.powerbutton').on("click", "img", function () {
         sndReq('/scripts/rpc.php', 'startapplication', GlobalStrings.StartingEyelockApp);
     });
 });


 // IDENTIFY DEVICE BUTTON
 $(document).ready(function()
 {
    $('#identifydevicebutton').click(function()
    {
        $('#identifydevicebutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
        sndReq('/scripts/rpc.php', 'identifydevice', "", false); // Don't show this, throw up a message box instead.
        ShowMessageBox(GlobalStrings.IdentifyDeviceTitle, GlobalStrings.IdentifyDeviceMessage, GlobalStrings.IdentifyDeviceMessage2, "MB_ICONLOADING", "CANCEL", CancelIdentifyDevice, null);
    });
 });


 // FACTORY RESET BUTTON
 $(document).ready(function () {
     $('#factoryresetbutton').click(function () {
         $('#factoryresetbutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)

         ShowMessageBox(GlobalStrings.FactoryResetTitle, GlobalStrings.FactoryResetWarning, GlobalStrings.AreYouSure, "MB_ICONEXCLAMATION", "YESNO", FactoryResetOK, null);
     });
 });

// FACTORY RESET BUTTON
 $(document).ready(function () {
     $('#osdpinstallmode').click(function () {
         $('#osdpinstallmode').blur(); // Pop focus off... (doesn't do it on it's own for some reason)

         ShowMessageBox(GlobalStrings.osdpInstallModeTitle, GlobalStrings.osdpInstallModeWarning, GlobalStrings.AreYouSure, "MB_ICONEXCLAMATION", "YESNO", OsdpSecureModeOK, null);
     });
 });

 window.onbeforeunload = function () {
     //if (submittingChanges) //we know we're not intending to leave, don't logout
         return;
     $.ajax({
         xhr: function () {
             var xhr = new window.XMLHttpRequest();

             return xhr;
         },
          cache: false,
         type: 'POST',
         url: '/scripts/logout.php',
          async: false,
         // data: data,
         processData: false, // Don't process the files
         contentType: false, // Set content type to false as jQuery will tell the server its a query 
         complete: function (xhr, status) {

             response = xhr.responseText;


             // Now we want to show our success/failure dialog...

         },
         error: function (xhr, ajaxOptions, thrownError) {



         }
     });
     //location.replace('') ;
 };


// If user says go ahead... well, then go ahead...
 function FactoryResetOK() {
        sndReq('/scripts/rpc.php', 'factoryreset', GlobalStrings.FactoryResetDefaults, true);
 }

function OsdpSecureModeOK() {
        sndReq('/scripts/rpc.php', 'osdpreset', GlobalStrings.OsdpInstallModeDefaults, true);
 }

// When user 'cancels' the Idendtify device requests...
 function CancelIdentifyDevice() {
	sndReq('/scripts/rpc.php', 'identifydevicestop', "", false);
     HideWaitingDlg();
 }


// REBOOT DEVICE BUTTON
 $(document).ready(function()
 {
    $('#rebootdevicebutton').click(function()
    {
        $('#rebootdevicebutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
        ShowMessageBox(GlobalStrings.RebootDeviceTitle, GlobalStrings.RebootWarning, GlobalStrings.AreYouSure, "MB_ICONEXCLAMATION", "YESNO", RebootDeviceOK, null);
    });
 });


// If user says go ahead... well, then go ahead...
 function RebootDeviceOK() {
	 ignoreTimeout = true;
        sndReq('/scripts/rpc.php', 'rebootdevice', GlobalStrings.RebootingDeviceMessage, true, 150000);
        setTimeout(function () { 
		
		if(deviceNameChange)
			window.location = "https://" + newDeviceName+".local/";
		else
			location.reload(true); }, 120000);//after 2 minutes do the redirect
                //test reboot time on my device (how long it was down) is about 1 minute 5 seconds.
             

 }

 function setEnabledTestNWMSButton()
 {
     var checkme = document.getElementById('GRI_EnableNWHDMatcher').checked;
     if(checkme)
     {
         document.getElementById("nmstestbutton").removeAttribute("disabled");
         //$('#').setAttribute("disabled", "disabled");

     }
     else
     {
        document.getElementById("nmstestbutton").setAttribute("disabled", "disabled");

     }

 }

 $(document).ready(function () {
     setEnabledTestNWMSButton();





 });


// ACS TEST BUTTON
 $(document).ready(function () {
     // Initial state of relay button
      bOldEnableRelay = document.getElementById('GRITrigger_EnableRelayWithSignal').checked;
     // DMOTODO -- Add in all the other parameters that must be passed back...
     // Should also force a parsely validation here before sending the request!!!!
     // Actually, use parsely to ensure that the button is disabled unless everything is correct...
     // so it can't even be clicked.
     $('#acstestbutton').click(function () {
        $('#acstestbutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
         // Create parameter array
         // must extract the current contents of each element that will be a parameter...
         // protocol, destaddr, destport, format, test data?, secure?
         var arParameters = [["protocol", document.getElementById('protocol').value]
                        //    ["Eyelock_ACPTestCardID", document.getElementById('Eyelock_ACPTestCardID').value],
                        //    ["Eyelock_ACPTestFacilityCode", document.getElementById('Eyelock_ACPTestFacilityCode').value],
                            ];
         sndReqParams('/scripts/rpc.php', 'acstest', arParameters, GlobalStrings.SendingTestMessageToACS);
     });
 });


 // ACS TEST BUTTON
 $(document).ready(function () {
     // DMOTODO -- Add in all the other parameters that must be passed back...
     // Should also force a parsely validation here before sending the request!!!!
     // Actually, use parsely to ensure that the button is disabled unless everything is correct...
     // so it can't even be clicked.
     $('#nmstestbutton').click(function () {
        $('#nmstestbutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
         // Create parameter array
         // must extract the current contents of each element that will be a parameter...
         // protocol, destaddr, destport, format, test data?, secure?
         var arParameters = [["ipaddress", document.getElementById('GRI_HDMatcher_Address').value + ":" + document.getElementById('GRI_NWHDPort').value ]
                        //    ["Eyelock_ACPTestCardID", document.getElementById('Eyelock_ACPTestCardID').value],
                        //    ["Eyelock_ACPTestFacilityCode", document.getElementById('Eyelock_ACPTestFacilityCode').value],
                            ];
         sndReqParams('/scripts/rpc.php', 'testmatch', arParameters, GlobalStrings.MsgTestingNetworkMatcher);
     });
 });



 // DELETE RESTORE POINTS BUTTON
 $(document).ready(function () {
     $('#deletefirmwarebutton').click(function () {
         $('#deletefirmwarebutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)

         var bNano = true;//document.getElementById('nanorestoreradio').checked;
         if(AreRestorePointsSelected())
            ShowMessageBox(GlobalStrings.DeleteRestorePointTitle, GlobalStrings.DeleteRestorePointMsg, GlobalStrings.AreYouSure, "MB_ICONEXCLAMATION", "YESNO", DeleteFirmwareOK, null);
     });
 });

 function AreRestorePointsSelected()
 {
     var bNano = true;//document.getElementById('nanorestoreradio').checked;

         var arList = "";
         var strList = "";

         if (bNano)
            arList = arRestoreList;
        else
            arList = arBobRestoreList;

         for (var i = arList.length-1; i >= 0; i--)
         {
             var theCheckbox = (bNano ? "restore_" : "bobrestore_") + i;

             if (document.getElementById(theCheckbox).checked)
             {
                strList +=   arList[i];
                strList += "|";
             }
         }

         return strList != "";
 }
 function CountRestorePoints()
 {
     var bNano = true;//document.getElementById('nanorestoreradio').checked;

         var arList = "";
         var strList = "";

         if (bNano)
            arList = arRestoreList;
        else
            arList = arBobRestoreList;

        return arList.length;
        
 }

// If user says go ahead... well, then go ahead...
 function DeleteFirmwareOK() {
        // Ok, need to:
        // 1.  Determine if Nano or Bob
        // 2.  Find all checked items in our list and build up our delete string
        // 3.  Pass the delete string to rpc.php
         var bNano = true;//document.getElementById('nanorestoreradio').checked;

         var arList = "";
         var strList = "";
		deleteFirmwareDialog = true;
         if (bNano)
            arList = arRestoreList;
        else
            arList = arBobRestoreList;

         for (var i = arList.length-1; i >= 0; i--)
         {
             var theCheckbox = (bNano ? "restore_" : "bobrestore_") + i;

             if (document.getElementById(theCheckbox).checked)
             {
                strList +=   arList[i];
                strList += "|";
             }
         }

         var arParameters = [["target", bNano ? "nano" : "bob"],
                             ["restorepoints", strList]];

         sndReqParams('/scripts/rpc.php', 'deleterestorepoints', arParameters, GlobalStrings.DeletingRestorePoints, true);

         // DMOTODO the successfull response from this would cause us to blow away and then repopulate our list...
         // So we need to basically just send another request to getrestorepoints...
 }

function deleteOldestAvailableRestorePoint(count)
{
        var bNano = true;//document.getElementById('nanorestoreradio').checked;
	 if (bNano)
            arList = arRestoreList;
        else
            arList = arBobRestoreList;

        if (count > arList.length)
            count = arList.length;

            var points = "";
            for (i = 0; i < count; i++)
                points += arList[i] + "|";

            

	var arParameters = [["target", bNano ? "nano" : "bob"],
                             ["restorepoints",points]];

      	sndReqParams('/scripts/rpc.php', 'deleterestorepoints', arParameters, GlobalStrings.DeletingRestorePoints, false, 600000);
}

 // RESTORE RESTORE POINTS BUTTON
 $(document).ready(function () {
     $('#restorefirmwarebutton').click(function () {
         $('#restorefirmwarebutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
         if(AreRestorePointsSelected())  //make sure at least one point is selected before proceeding
         ShowMessageBox(GlobalStrings.RestoreDeviceTitle, GlobalStrings.RestoreDevice, GlobalStrings.AreYouSure, "MB_ICONEXCLAMATION", "YESNO", RestoreFirmwareOK, null);
     });
 });


// If user says go ahead... well, then go ahead...
 function RestoreFirmwareOK() {
        // DMOTODO
        // Ok, need to:
        // 1.  Determine if Nano or Bob
        // 2.  Find all checked items in our list and build up our delete string
        // 3.  Pass the delete string to rpc.php
         var bNano = true;//document.getElementById('nanorestoreradio').checked;

         var arList = "";
         var strList = "";

         if (bNano)
            arList = arRestoreList;
         else
            arList = arBobRestoreList;

         for (var i = arList.length-1; i >= 0; i--)
         {
             var theCheckbox = (bNano ? "restore_" : "bobrestore_") + i;

             if (document.getElementById(theCheckbox).checked)
             {
                strList +=   arList[i];
                strList += "|";
                break; // Only grab the first one!
             }
         }

         var arParameters = [["target", bNano ? "nano" : "bob"],
                             ["restorepoints", strList]];
         upgradeInProgress = true; //technically it's an update
         sndReqParams('/scripts/rpc.php', 'restorerestorepoint', arParameters, GlobalStrings.RestoreRestorePoint, true, 180000);
		ignoreTimeout = true;
         $("#loading-text-second").text(GlobalStrings.RebootingDeviceMessage2);
 }



 // DHCP Settings Button
 $(document).ready(function()
 {
    $('#dhcpsettingsbutton').click(function()
    {
        $('#dhcpsettingsbutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
        ShowDHCPDlg();
    });
 });


 $(document).ready(function()
 {
    $('#dhcpsettingsOKbutton').click(function()
    {
        // DMOTODO -- do some form of validation before closing!
        HideDHCPDlg();
    });
 });

function twoDigit(num)
{
    if(num<10)
	return '0'+num;
    return num;
}
function updateTime()
{
	 var arParameters = [["timeserver", document.getElementById('GRI_InternetTimeAddr').value]];

        sndReqParams('/scripts/rpc.php', 'updatetime', arParameters, GlobalStrings.DeviceTimeSynchronizing);
}
 $(document).ready(function()
 {
    $('#updatetimebutton').click(function()
    {
		submitTimeSync = false;
        $('#updatetimebutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
       updateTime();
    });

    $('#updatelocaltimebutton').click(function()
    {
        $('#updatelocaltimebutton').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
	
	curDate = new Date();
	year = curDate.getUTCFullYear();
	month = parseInt(curDate.getUTCMonth())+1;
	date = curDate.getUTCDate();
	hour = curDate.getUTCHours();
	mins = curDate.getUTCMinutes();
	secs = curDate.getUTCSeconds();

        var arParameters = [["localtime", year+"-"+twoDigit(month)+"-"+twoDigit(date)+" "+twoDigit(hour)+":"+twoDigit(mins)+":"+twoDigit(secs)]];

        sndReqParams('/scripts/rpc.php', 'updatelocaltime', arParameters, GlobalStrings.DeviceTimeSynchronizing);
    });
 });


 $(document).ready(function () {
     $('#refreshlog').click(function () {
         $('#refreshlog').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
         tailLog(true);
     });
 });

  var urllog = "/home/root/nxtEvent.log";


  $(document).ready(function () {
      $('#downloadlog').mouseup(function (e) {
          if (e.ctrlKey) {
              logtype = "1";
              dllogname = strDeviceName + "_log_detailed.txt"
          }
          else if (e.altKey) {
              logtype = "2";
              dllogname = strDeviceName + "_log_messages.rtf"
          }
          else {
              logtype = "3";
              dllogname = strDeviceName + "_log.csv"
          }
      });
  });


var download = function() {
       for(var i=0; i<arguments.length; i++) {
         var iframe = $('<iframe style="visibility: collapse;"></iframe>');
         $('body').append(iframe);
         var content = iframe[0].contentDocument;
         var form = '<form action="' + arguments[i] + '" method="GET"></form>';
         content.write(form);
         $('form', content).submit();
         setTimeout((function(iframe) {
           return function() { 
             iframe.remove(); 
           }
         })(iframe), 2000);
       }
     }      

 $(document).ready(function () {
     $('#downloadlog').click(function () {
         $('#downloadlog').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
		
         var ifrm = document.getElementById('downloadframe');
		  var ifrm2 = document.getElementById('downloadframe2');
//         ifrm.src = "/scripts/logdownload.php?path=" + urllog + "&" + "dlfilename=" + dllogname;
		
		 if(logtype == "1" || logtype == "2")
		 {
           ifrm.src = "/scripts/logdownload.php?logfiletype=" + logtype + "&" + "dlfilename=camera1_" + dllogname + "&slavelog=0";// + slaveLog;
		   ifrm2.src ="/scripts/logdownload.php?logfiletype=" + logtype + "&" + "dlfilename=camera2_" + dllogname + "&slavelog=1";
		 }
		 else
			  ifrm.src = "/scripts/logdownload.php?logfiletype=" + logtype + "&" + "dlfilename=" + dllogname + "&slavelog=0";
		// if(logtype == "1" || logtype == "2")
		 // ifrm.src = "/scripts/logdownload.php?logfiletype=" + logtype + "&" + "dlfilename=camera2_" + dllogname + "&slavelog=1";// + slaveLog;
     });
 });



 $(document).ready(function () {
     $('#autorefreshlog').click(function () {
         if (document.getElementById('autorefreshlog').checked)
             autotailLog();
     });
 });



 // Download 802 Logs...
 $(document).ready(function ()
 {
     $('#download802log').click(function ()
     {
         $('#download802log').blur(); // Pop focus off... (doesn't do it on it's own for some reason)
		
         var ifrm = document.getElementById('downloadframe');
   	     ifrm.src = "/scripts/logdownload.php?logfiletype=5" + "&" + "dlfilename=IEEE_802.1X_Log.txt";
     });
 });



/////////////////////////////////////////////////////////////////////
// Process mouse click to show hidden settings...
/////////////////////////////////////////////////////////////////////
 $(document).ready(function () {
     $('#tab-content2-label').mouseup(function (e) {


         if (e.ctrlKey && e.altKey) {
             $('#devicesettings-normal').hide();
             $('#devicesettings-debug').show();
             $('#devicesettings-advanced').hide();
         }
         else if (e.ctrlKey) {
             // If LED not already shown, show them...
             if ('none' == ($('#devicesettings-normal').css('display'))) {
                 $('#devicesettings-normal').show();

                 $('#devicesettings-debug').hide();

                 $('#devicesettings-advanced').hide();
             }
             else {
                 $('#devicesettings-normal').hide();
                 $('#devicesettings-debug').hide();
                 $('#devicesettings-advanced').show();
             }

         }
         else if (e.shiftKey) {
             $('#devicesettings-hbox').show();
            $('#devicesettings-normal').hide();
            $('#devicesettings-debug').hide();
         }
         else {
             $('#devicesettings-normal').show();

             $('#devicesettings-debug').hide();

             $('#devicesettings-advanced').hide();
             $('#devicesettings-hbox').hide();
         }
     });
 });


 
 $(document).ready(function () {
     $('#tab-content8-label').mouseup(function (e) {
         if (e.ctrlKey) {
             // If LED not already shown, show them...
             if ('none' == ($('#refreshlog-advanced').css('display'))) {
                 $('#refreshlog-advanced').show();
             }
             else {
                 $('#refreshlog-advanced').hide();
             }
         }
		 
		
		 
     });
 });

 

/////////////////////////////////////////////////////////////////////
// Enable/Disable for Network settings radio buttons
/////////////////////////////////////////////////////////////////////
$(document).ready(function () {
     $('#dhcpradio').click(function () {
  //      $('#dhcpsettingsbutton').removeAttr('disabled');
         $('#ipofboard').attr({
             'disabled': 'disabled'
         });
         $('#broadcastnetwork').attr({
             'disabled': 'disabled'
         });
         $('#subnetmask').attr({
             'disabled': 'disabled'
         });
         $('#gateway').attr({
             'disabled': 'disabled'
         });
	  $('#dns1').attr({
             'disabled': 'disabled'
         });
  	  $('#dns2').attr({
             'disabled': 'disabled'
         });

        $('#dhcpsettingsbutton').show();
        isCurrentlyStaticIP = 0;
     });
 });


 // Set enabled/disabled state of config dependant controls...
 $(document).ready(function () {
     $('#staticradio').click(function () {
         //      $('#dhcpsettingsbutton').attr({
         //          'disabled': 'disabled'
         //      });
         $('#ipofboard').removeAttr('disabled');
         $('#broadcastnetwork').removeAttr('disabled');
         $('#gateway').removeAttr('disabled');
         $('#subnetmask').removeAttr('disabled');
	  $('#dns1').removeAttr('disabled');
	  $('#dns2').removeAttr('disabled');
         $('#dhcpsettingsbutton').hide();
         isCurrentlyStaticIP = 1;
     });
 });

 $(document).ready(function () {
     // :OnLoad set our enabled, disabled state for our DOM elements based on our loaded configuration
     // This function MUST remain after the definition of the click handlers immediately above.
     // The script is run sequentially... of course.
     if (!isStaticIP)
        $('#dhcpradio').click();
     else
         $('#staticradio').click();
 });

 function ipTypeChanged()
 {
     return isStaticIP != isCurrentlyStaticIP;
 }

 function enableNWMSFields()
 {
     
     if (document.getElementById('GRI_EnableNWHDMatcher').checked) {
             $('#GRI_HDMatcher_Address').removeAttr('disabled');
             $('#GRI_NWHDPort').removeAttr('disabled');
             $('#NW_Matcher_Comm_Secure').removeAttr('disabled');
         }
         else {
             $('#GRI_HDMatcher_Address').attr({
                 'disabled': 'disabled'
             });
             $('#GRI_NWHDPort').attr({
                 'disabled': 'disabled'
             });
             $('#NW_Matcher_Comm_Secure').attr({
                 'disabled': 'disabled'
             });
         }
 }
/////////////////////////////////////////////////////////////////////
// Enable/Disable for Database (Network Matcher controls)
/////////////////////////////////////////////////////////////////////
 $(document).ready(function () {
     $('#GRI_EnableNWHDMatcher').click(function () {
         enableNWMSFields();
     });
 });

 $(document).ready(function () {
     // :OnLoad set our enabled, disabled state for our DOM elements based on our loaded configuration
     // This function MUST remain after the definition of the click handlers immediately above.
     // The script is run sequentially... of course.
//     if (!isHDMatcherEnabled) // If we are initially NOT 'checked' on loading, make sure we disable things...
  //      $('#GRI_EnableNWHDMatcher').click();
 });
 
// Respond to language change by reloading in new language...
 $(document).ready(function () {
     $('#countrycode').change(function () {
         ShowWaitingDlg(GlobalStrings.ReloadingPage, "", true, 30000);
         window.location = "/index.php?lang=" + $(this).val();
     });
 });

 $(document).ready(function() {
     
     if (document.getElementById('GRI_EnableNWHDMatcher').checked ||document.getElementById('GRITrigger_TemplateOnCard').checked) {
             document.getElementById("TemplatesAvailableLabel").setAttribute("style", "visibility: hidden");
		 	document.getElementById("databaseTemplatesAvailable").setAttribute("style", "display:none");
             
         }
         else
         {
             document.getElementById("TemplatesAvailableLabel").setAttribute("style", "visibility: visible");
             document.getElementById("databaseTemplatesAvailable").setAttribute("style", "");
         }

 })

 $(document).ready(function () {
     $('#GRI_EnableNWHDMatcher').click(function () {
         if (document.getElementById('GRI_EnableNWHDMatcher').checked || document.getElementById('GRITrigger_TemplateOnCard').checked) {
             document.getElementById("TemplatesAvailableLabel").setAttribute("style", "visibility: hidden");
             document.getElementById("databaseTemplatesAvailable").setAttribute("style", "display: none");
         }
         else
         {
             document.getElementById("TemplatesAvailableLabel").setAttribute("style", "visibility: visible");
             document.getElementById("databaseTemplatesAvailable").setAttribute("style", "");
         }
     });
 });
 /*
 $(document).ready(function () {
      $('#tamperinputsignalhigh').click(function () {
            document.getElementById("tamperinputsignallow").checked = false;
    });
     $('#tamperinputsignallow').click(function () {
            document.getElementById("tamperinputsignalhigh").checked = false;
    });
      $('#tamperoutputsignalhigh').click(function () {
            document.getElementById("tamperoutputsignallow").checked = false;
    });
     $('#tamperoutputsignallow').click(function () {
            document.getElementById("tamperoutputsignalhigh").checked = false;
    });
 });
 */
 $(document).ready(function () {
     $('#GRITrigger_TemplateOnCard').click(function () {
         if (document.getElementById('GRITrigger_TemplateOnCard').checked) {

             $('#SchemeSelector').hide();
             //  document.getElementById("GRITrigger_DualAuthenticationMode").checked = false;
             $('#iriswaittimeitem2').show();
             $('#LIMobileMode').show();
             $('#portableTemplateCerts').show();
			 $('#portableTemplateBootloader').show();
             $('#databaseTemplatesAvailable').hide();
             $('#databaseDetailsDiv').hide();
             $('#TemplatesAvailableLabel').hide();
             document.getElementById('GRI_EnableNWHDMatcher').checked = false;
             enableNWMSFields();
             databaseTabShowTemplatesRemaining();
             document.getElementById("TemplatesAvailableLabel").setAttribute("style", "visibility: hidden");
              document.getElementById("GRITrigger_DualAuthNCardMatchWaitIrisTime2").value = 60;
               $("#iriswaittimeslider2").slider("value", parseFloat(60));
         }
         else {
             $('#portableTemplateCerts').hide();
			  $('#portableTemplateBootloader').hide();
             $('#SchemeSelector').show();
             $('#iriswaittimeitem2').hide();
             $('#LIMobileMode').hide();
             $('#databaseDetailsDiv').show();
             $('#databaseTemplatesAvailable').show();
             // $('#TemplatesAvailableLabel').show();
             databaseTabShowTemplatesRemaining();
             document.getElementById("TemplatesAvailableLabel").setAttribute("style", "visibility: visible");
             //  document.getElementById("GRITrigger_DualAuthenticationMode").checked = true;

         }
         // GRITrigger_DualAuthenticationMode_click();
     });

 });

 function GRITrigger_DualAuthenticationMode_click() {
     //if (document.getElementById('GRITrigger_DualAuthenticationMode').checked || document.getElementById("GRITrigger_TOCPassThrough").checked) {
    var caccessControlType = document.getElementById("accessControlType");

    var bDualEye = ((caccessControlType.options[caccessControlType.selectedIndex].value == "IrisandCard" ) ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "IrisandCardPP" ) ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIris" ) ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinCardandIris") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIrisDuress") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinCardandIrisDuress"));

    var bPin =      ((caccessControlType.options[caccessControlType.selectedIndex].value == "IrisandCardPP") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIris") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinCardandIris") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIrisDuress") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinCardandIrisDuress"));

    var bPinNoWait = ((caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIris") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIrisDuress"));

    var bDuress = ((caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIrisDuress") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinCardandIrisDuress"));


    if (bDualEye)
    {
        $('#dualauthitem3').show();
		$('#iriswaittimeitem').show();
        //  document.getElementById("tampersignalhigh").checked = false;
        // document.getElementById("tampersignallow").checked = true;
        document.getElementById("Eyelock_TamperNotifyAddr").removeAttribute("disabled");
        document.getElementById("tamperinputsignalhigh").removeAttribute("disabled");
        document.getElementById("tamperinputsignallow").removeAttribute("disabled");
        //  document.getElementById("tamperoutputsignalhigh").removeAttribute("disabled");
        //  document.getElementById("tamperoutputsignallow").removeAttribute("disabled");

        document.getElementById("Eyelock_TamperNotifyPort").removeAttribute("disabled");
        document.getElementById("Eyelock_TamperNotifyMessage").removeAttribute("disabled");
    // document.getElementById("GRITrigger_TOCPassThrough").checked = false;
    }
    else 
    {
         document.getElementById("tamperinputsignalhigh").checked = false;
         document.getElementById("tamperinputsignallow").checked = true;
         document.getElementById("tamperinputsignalhigh").value = false;
         document.getElementById("tamperinputsignallow").value = true;
         // document.getElementById("Eyelock_TamperNotifyAddr").setAttribute("disabled", "disabled");
         document.getElementById("tamperinputsignalhigh").setAttribute("disabled", "disabled");
         document.getElementById("tamperinputsignallow").setAttribute("disabled", "disabled");

       //  document.getElementById("GRITrigger_TOCPassThrough").checked = true;
         //  document.getElementById("tamperoutputsignalhigh").checked = false;
         // document.getElementById("tamperoutputsignallow").checked = true;
         // document.getElementById("tamperoutputsignalhigh").value = false;
         // document.getElementById("tamperoutputsignallow").value = true;
         // document.getElementById("Eyelock_TamperNotifyAddr").setAttribute("disabled", "disabled");
         // document.getElementById("tamperoutputsignalhigh").setAttribute("disabled", "disabled");
         //document.getElementById("tamperoutputsignallow").setAttribute("disabled", "disabled");

         //  document.getElementById("Eyelock_TamperNotifyPort").setAttribute("disabled", "disabled");
         // document.getElementById("Eyelock_TamperNotifyMessage").setAttribute("disabled", "disabled");


        $('#iriswaittimeitem').hide();
         $('#dualauthitem3').hide();
		  $("#parityMasking").hide();

//         document.getElementById("GRITrigger_DualAuthenticationParity").checked = true;
     }

     if (bPin) {
         if (!bPinNoWait)
            $('#waitpintimeitem').show();
         else
            $('#waitpintimeitem').hide();

         $('#pinburstbitsitem').show();
     }
     else
     {
         $('#waitpintimeitem').hide();
         $('#pinburstbitsitem').hide();
     }


    if (bDuress)
    {
        document.getElementById('GRITrigger_EnableRelayWithSignal').checked = true;
        bDuressChanged = true;
        $('#GRITrigger_EnableRelayWithSignal').attr({'disabled': 'disabled'});

        $('#playerduressrelaytimelabel').show();
        $('#playerdenyrelaytimelabel').hide();
    }
    else if (bDuressChanged)
    {
        document.getElementById('GRITrigger_EnableRelayWithSignal').checked = bOldEnableRelay;
        bDuressChanged = false;
    }

    if (!bDuress) {
        $('#GRITrigger_EnableRelayWithSignal').removeAttr('disabled');
        $('#playerduressrelaytimelabel').hide();
        $('#playerdenyrelaytimelabel').show();
    }



	 if(document.getElementById('GRITrigger_DualAuthenticationMode').checked)
		 {
			  
			 // $("#parityMasking").show();
		 }
	 else
		 {
			//  $("#parityMasking").hide();
			  
		 }
	 
	 
	 
 }

 $(document).ready(function () {
     $('#GRITrigger_DualAuthenticationMode').click(function (){GRITrigger_DualAuthenticationMode_click(); } );

 });


 $(document).ready(function () {
     $('#GRITrigger_EnableRelayWithSignal').click(function (){bOldEnableRelay = document.getElementById('GRITrigger_EnableRelayWithSignal').checked; } );
     GRITrigger_DualAuthenticationMode_click()
 });


$(document).ready(function () {
     $('#useParityMaskDisabled').click(function (){
	 
		 document.getElementById("useParityMaskEnabled").checked = false;
	 
	 } );

 });
 $(document).ready(function () {
     $('#useParityMaskEnabled').click(function (){
	 
		  document.getElementById("useParityMaskDisabled").checked = false;
	 
	 } );

 });


function showIrisOrCardOption()
{
	$("select option").each(function(index, val) {
		if(navigator.appName == 'Microsoft Internet Explorer'|| navigator.appName == 'Netscape') {
			if (this.nodeName.toUpperCase() === 'OPTION' ) {
				var span = $(this).parent();
				var opt = this;
				if($(this).parent().is('span') && $(this).attr('id') =='irisorcardoption') {
					$(opt).show();
					$(span).replaceWith(opt);
				}
			}
		} else {
			$(this).show(); //all other browsers use standard .show()
		}
	});
}


function hideIrisOrCardOption()
{
	$("select option").each(function(index, val){
	if ($(this).is('option') && (!$(this).parent().is('span') && $(this).attr('id') =='irisorcardoption'))
		$(this).wrap((navigator.appName == 'Netscape' || navigator.appName == 'Microsoft Internet Explorer') ? '<span>' : null).hide();
	});
}


function showIrisAndCardOption()
{
	$("select option").each(function(index, val) {
		if(navigator.appName == 'Microsoft Internet Explorer'|| navigator.appName == 'Netscape') {
			if (this.nodeName.toUpperCase() === 'OPTION' ) {
				var span = $(this).parent();
				var opt = this;
				if($(this).parent().is('span') && $(this).attr('id') =='irisandcardoption') {
					$(opt).show();
					$(span).replaceWith(opt);
				}
			}
		} else {
			$(this).show(); //all other browsers use standard .show()
		}
	});
}

function hideIrisAndCardOption()
{
	$("select option").each(function(index, val){
	if ($(this).is('option') && (!$(this).parent().is('span') && $(this).attr('id') =='irisandcardoption'))
		$(this).wrap((navigator.appName == 'Netscape' || navigator.appName == 'Microsoft Internet Explorer') ? '<span>' : null).hide();
	});
}


function showWeigandModes()
{
	$("select option").each(function(index, val) {
		if(navigator.appName == 'Microsoft Internet Explorer'|| navigator.appName == 'Netscape') {
			if (this.nodeName.toUpperCase() === 'OPTION' ) {
				var span = $(this).parent();
				var opt = this;
				if ($(this).parent().is('span') && ($(this).attr('id') == 'irisandcardppoption' || $(this).attr('id') == 'pinandirisoption' || $(this).attr('id') == 'pincardandirisoroption' ||
                                $(this).attr('id') == 'pinandirisoduressroption' || $(this).attr('id') == 'pincardandirisorduressoption'))
                {
					$(opt).show();
					$(span).replaceWith(opt);
				}
			}
		} else {
			$(this).show(); //all other browsers use standard .show()
		}
	});
}


function hideWeigandModes()
{
	$("select option").each(function(index, val){
	if ($(this).is('option') && (!$(this).parent().is('span')  && ($(this).attr('id') == 'irisandcardppoption' || $(this).attr('id') == 'pinandirisoption' || $(this).attr('id') == 'pincardandirisoroption' ||
                                $(this).attr('id') == 'pinandirisoduressroption' || $(this).attr('id') == 'pincardandirisorduressoption')))
		$(this).wrap((navigator.appName == 'Netscape' || navigator.appName == 'Microsoft Internet Explorer') ? '<span>' : null).hide();
	});
}


function handleProtocolSelector()
{
 	 var cSelect = document.getElementById("protocol");
     var actSelect = document.getElementById("accessControlType"); //select single factor when OSDP is selected to clear the selection of passthrough mode


	 if (cSelect.options[cSelect.selectedIndex].value == "osdp")
     {
         if (cSelect.options[cSelect.selectedIndex].value == "osdp") 
         {
             if ((actSelect.selectedIndex > 0) && (actSelect.selectedIndex != 2))
             {
                 actSelect.selectedIndex = 0;
                 GRITrigger_DualAuthenticationMode_click();
             }

             $('#OSDPBaudRate').show();
             $('#osdpinstallmode').show();

             $('#irisorcardoption').hide();
             hideIrisOrCardOption();

             $('#irisandcardoption').show();
             showIrisAndCardOption();
         }
         else // must be HID
         {
             if (actSelect.selectedIndex >= 2)
             {
                 actSelect.selectedIndex = 0;
                 GRITrigger_DualAuthenticationMode_click();
             }

             $('#irisorcardoption').show();
             showIrisOrCardOption();

             $('#irisandcardoption').hide();
             hideIrisAndCardOption();
         }

        // Hide weigand only options...
     // OUTUNTILPASSTHROUGHISFIXED   $('#irisandcardppoption').hide();
        $('#pinandirisoption').hide();
        $('#pincardandirisoroption').hide();
        $('#pinandirisoduressroption').hide();
        $('#pincardandirisorduressoption').hide();

        hideWeigandModes();

        //$('#acstestsettings').hide();
     }
     else
     {
         if (cSelect.options[cSelect.selectedIndex].value == "hid") 
         {
             $('#irisorcardoption').show();
             showIrisOrCardOption();

             $('#irisandcardoption').hide();
             hideIrisAndCardOption();
         }
         else 
         {
            $('#irisorcardoption').show();
            $('#irisandcardoption').show();
            showIrisOrCardOption();
            showIrisAndCardOption();
         }

        $('#OSDPBaudRate').hide();  //don't show baud rate unless OSDP is selected
        $('#osdpinstallmode').hide();

        //$('#acstestsettings').show();

        // Deal with Weigand
        if (cSelect.options[cSelect.selectedIndex].value != "weigand")
        {
            if (actSelect.selectedIndex > 2)
            {
                actSelect.selectedIndex = 0;
                GRITrigger_DualAuthenticationMode_click();
            }

            // Hide weigand only options...
       // OUTUNTILPASSTHROUGHISFIXED     $('#irisandcardppoption').hide();
            $('#pinandirisoption').hide();
            $('#pincardandirisoroption').hide();
            $('#pinandirisoduressroption').hide();
            $('#pincardandirisorduressoption').hide();

            hideWeigandModes();
        }
        else 
        {
            // Show weigand only options...
    // OUTUNTILPASSTHROUGHISFIXED        $('#irisandcardppoption').show();
            $('#pinandirisoption').show();
            $('#pincardandirisoroption').show();
            $('#pinandirisoduressroption').show();
            $('#pincardandirisorduressoption').show();

            showWeigandModes();
        }
     }
}


$(document).ready(function () {

    handleProtocolSelector();

});
// Handle Protocol select (show hide some controls based on selected protocol)
$(document).ready(function () {
    $('#protocol').change(function () {
        var cSelect = document.getElementById("protocol");
        //         var bDualEye = document.getElementById("GRITrigger_DualAuthenticationMode").checked;
        var caccessControlType = document.getElementById("accessControlType");
        var bDualEye = ((caccessControlType.options[caccessControlType.selectedIndex].value == "IrisandCard") ||
                         (caccessControlType.options[caccessControlType.selectedIndex].value == "IrisandCardPP") ||
                         (caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIris") ||
                         (caccessControlType.options[caccessControlType.selectedIndex].value == "PinCardandIris") ||
                        (caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIrisDuress") ||
                        (caccessControlType.options[caccessControlType.selectedIndex].value == "PinCardandIrisDuress"));

        var bPinNoWait = ((caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIris") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIrisDuress"));

        var bPin = ((caccessControlType.options[caccessControlType.selectedIndex].value == "IrisandCardPP") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIris") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinCardandIris") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinandIrisDuress") ||
                    (caccessControlType.options[caccessControlType.selectedIndex].value == "PinCardandIrisDuress"));


        if (cSelect.options[cSelect.selectedIndex].value == "weigand" || cSelect.options[cSelect.selectedIndex].value == "osdp" || cSelect.options[cSelect.selectedIndex].value == "pac" || cSelect.options[cSelect.selectedIndex].value == "f2f") {
            $('#dualauthitem').show();
            // $('#dualauthitem2').show();

            if (bDualEye) {
                $('#iriswaittimeitem').show();
                $('#dualauthitem3').show();
            }
            else {
                $('#iriswaittimeitem').hide();
                $('#dualauthitem3').hide();
            }

            if (bPin && (cSelect.options[cSelect.selectedIndex].value != "osdp")) {
                if (!bPinNoWait)
                    $('#waitpintimeitem').show();
                else
                    $('#waitpintimeitem').hide();

                $('#pinburstbitsitem').show();
            }
            else {
                $('#waitpintimeitem').hide();
                $('#pinburstbitsitem').hide();
            }
        }
        else {
            //  document.getElementById("GRITrigger_DualAuthenticationMode").checked = false;
            $('#dualauthitem').hide();
            //$('#dualauthitem2').hide();
            $('#iriswaittimeitem').hide();

            if (bPin)
            {
                if (!bPinNoWait)
                    $('#waitpintimeitem').show();
                 else
                    $('#waitpintimeitem').hide();

                $('#pinburstbitsitem').show();
            }
            else 
            {
                $('#waitpintimeitem').hide();
                $('#pinburstbitsitem').hide();
            }

        }

        handleProtocolSelector();
        //         if (cSelect.options[cSelect.selectedIndex].value == "osdp") {
        //             $('#OSDPBaudRate').show();
        //             $('#irisorcardoption').hide();
        //			 hideIrisOrCardOption();
        //             var actSelect = document.getElementById("accessControlType"); //select single factor when OSDP is selected to clear the selection of passthrough mode
        //             actSelect.selectedIndex = 0;
        //
        //             //$('#acstestsettings').hide();
        //         }
        //         else {
        //             $('#OSDPBaudRate').hide();  //don't show baud rate unless OSDP is selected
        //             $('#irisorcardoption').show();
        //			showIrisOrCardOption();
        //             //$('#acstestsettings').show();
        //         }


    });
});


function handleAuthenticationSelector()
{
 	 var cSelect = document.getElementById("irismode");

	 if (cSelect.options[cSelect.selectedIndex].value == "irisauthentication")
     {
        $('#IrisAuthentication').show();
        $('#IrisCapture').hide();
     }
     else
     {
        $('#IrisAuthentication').hide();
        $('#IrisCapture').show();
     }
}


$(document).ready(function () {
    handleAuthenticationSelector();
    handleIrisCaptureImageFormatSelector();
});



// Handle Authentication select (show hide some controls based on selected protocol)
$(document).ready(function () {
    $('#irismode').change(function () {
         handleAuthenticationSelector();
    });
});


function handleIrisCaptureImageFormatSelector()
{
 	 var cSelect = document.getElementById("httppostpayloadformat");

	 if (cSelect.options[cSelect.selectedIndex].value == "FORMAT_J2K")
        $('#imagequalityfields').show();
     else
        $('#imagequalityfields').hide();
}

// Handle Authentication select (show hide some controls based on selected protocol)
$(document).ready(function () {
    $('#httppostpayloadformat').change(function () {
         handleIrisCaptureImageFormatSelector();
    });
});


$(document).ready(function () {
    $('#accessControlType').change(function () {
        var cSelect = document.getElementById("accessControlType");
        var cSelectProtocol = document.getElementById("protocol");
        var bDualEye = false;
        var bPin = false;
        var bPinNoWait = false;

        if (cSelectProtocol.options[cSelectProtocol.selectedIndex].value == "osdp")
            bDualEye = cSelect.options[cSelect.selectedIndex].value != "IrisOnly";
        else if (cSelectProtocol.options[cSelectProtocol.selectedIndex].value == "hid")
            bDualEye = false;
        else {
            bDualEye = ((cSelect.options[cSelect.selectedIndex].value == "IrisandCard") ||
                         (cSelect.options[cSelect.selectedIndex].value == "IrisandCardPP") ||
                         (cSelect.options[cSelect.selectedIndex].value == "PinandIris") ||
                         (cSelect.options[cSelect.selectedIndex].value == "PinCardandIris") ||
                         (cSelect.options[cSelect.selectedIndex].value == "PinandIrisDuress") ||
                         (cSelect.options[cSelect.selectedIndex].value == "PinCardandIrisDuress"));

            bPinNoWait = ((cSelect.options[cSelect.selectedIndex].value == "PinandIris") ||
                         (cSelect.options[cSelect.selectedIndex].value == "PinandIrisDuress"));

            bPin = ((cSelect.options[cSelect.selectedIndex].value == "IrisandCardPP") ||
                    (cSelect.options[cSelect.selectedIndex].value == "PinandIris") ||
                    (cSelect.options[cSelect.selectedIndex].value == "PinCardandIris") ||
                    (cSelect.options[cSelect.selectedIndex].value == "PinandIrisDuress") ||
                    (cSelect.options[cSelect.selectedIndex].value == "PinCardandIrisDuress"));
        }


        if (bDualEye) {
            $('#iriswaittimeitem').show();
            $('#dualauthitem3').show();
        }
        else {
            $('#iriswaittimeitem').hide();
            $('#dualauthitem3').hide();
        }

        if (bPin) {
            if (!bPinNoWait)
                $('#waitpintimeitem').show();
            else
               $('#waitpintimeitem').hide();

            $('#pinburstbitsitem').show();
        }
        else {
            $('#waitpintimeitem').hide();
            $('#pinburstbitsitem').hide();
        }


        if (cSelect.options[cSelect.selectedIndex].value == "IrisOnly") {
            //              document.getElementById("GRITrigger_TOCPassThrough").checked = false;
            //              document.getElementById("GRITrigger_DualAuthenticationMode").checked = false;
        }

        if (cSelect.options[cSelect.selectedIndex].value == "IrisorCard") {
            //              document.getElementById("GRITrigger_TOCPassThrough").checked = true;
            //              document.getElementById("GRITrigger_DualAuthenticationMode").checked = false;
        }

        if (cSelect.options[cSelect.selectedIndex].value == "IrisandCard") {
            //              document.getElementById("GRITrigger_TOCPassThrough").checked = false;
            //              document.getElementById("GRITrigger_DualAuthenticationMode").checked = true;
        }

        if (cSelect.options[cSelect.selectedIndex].value == "IrisandCardPP") {
            //              document.getElementById("GRITrigger_TOCPassThrough").checked = false;
            //              document.getElementById("GRITrigger_DualAuthenticationMode").checked = true;
        }

        if (cSelect.options[cSelect.selectedIndex].value == "PinandIris") {
            //              document.getElementById("GRITrigger_TOCPassThrough").checked = false;
            //              document.getElementById("GRITrigger_DualAuthenticationMode").checked = true;
        }

        if (cSelect.options[cSelect.selectedIndex].value == "PinCardandIris") {
            //              document.getElementById("GRITrigger_TOCPassThrough").checked = false;
            //              document.getElementById("GRITrigger_DualAuthenticationMode").checked = true;
        }

        GRITrigger_DualAuthenticationMode_click(); //process the state change
    });
});



// On Form init...
$(document).ready(function () {
    enable802Fields();
});

$(document).ready(function () 
{
    $('#Eyelock_EnableIEEE8021X').click(function ()
    {
        enable802Fields();
    });
});


function enable802Fields()
 {
     if (document.getElementById('Eyelock_EnableIEEE8021X').checked) {
            $('#browseCACertButton').removeAttr('disabled');
            $('#CACertbutton').removeAttr('disabled');
            $('#CAcertToUpload').removeAttr('disabled');
            $('#browseClientCertButton').removeAttr('disabled');
            $('#ClientCertbutton').removeAttr('disabled');
            $('#ClientcertToUpload').removeAttr('disabled');
            $('#browsePrivateKeyButton').removeAttr('disabled');
            $('#PrivateKeybutton').removeAttr('disabled');
            $('#PrivateKeyToUpload').removeAttr('disabled');

             $('#EAPVersion').removeAttr('disabled');
             $('#EAPIdentityName').removeAttr('disabled');
             $('#PrivateKeyPassword').removeAttr('disabled');

             $('#download802log').removeAttr('disabled');
         }
         else {
                $('#browseCACertButton').attr({'disabled': 'disabled'});
                $('#CACertbutton').attr({'disabled': 'disabled'});
                $('#CAcertToUpload').attr({'disabled': 'disabled'});
                $('#browseClientCertButton').attr({'disabled': 'disabled'});
                $('#ClientCertbutton').attr({'disabled': 'disabled'});
                $('#ClientcertToUpload').attr({'disabled': 'disabled'});
                $('#browsePrivateKeyButton').attr({'disabled': 'disabled'});
                $('#PrivateKeybutton').attr({'disabled': 'disabled'});
                $('#PrivateKeyToUpload').attr({'disabled': 'disabled'});

                $('#EAPVersion').attr({'disabled': 'disabled'});
                $('#EAPIdentityName').attr({'disabled': 'disabled'});
                $('#PrivateKeyPassword').attr({'disabled': 'disabled'});

                $('#download802log').attr({'disabled': 'disabled'});
         }
 }


/* //OSDP checkbox
 $(document).ready(function () {
     $('#OSDPEnable').change(function () {
         var cbox = document.getElementById("OSDPEnable");
         var enable = cbox.checked;

        

         if (enable) {
             $('#OSDPBaudRate').show();
             //$('#acstestsettings').hide();
         }
         else {
              $('#OSDPBaudRate').hide();  //don't show baud rate unless OSDP is selected
             //$('#acstestsettings').show();
         }
     });
 });*/




 /////////////////////////////////////////////////////////////////////
// Enable/Disable for Database (Network Matcher controls)
/////////////////////////////////////////////////////////////////////
 $(document).ready(function () {
     $('#GRITrigger_RelayEnable').click(function () {
         if (document.getElementById('GRITrigger_RelayEnable').checked) {
             $('#relaytime').removeAttr('disabled');
             $('#playerrelaytimelabel').removeAttr('disabled');
             $('#playerrelaytime').removeAttr('disabled');
             $('#GRITrigger_RelayTimeInMS').removeAttr('disabled');

             $('#denyrelaytime').removeAttr('disabled');
             $('#playerdenyrelaytimelabel').removeAttr('disabled');
             $('#playerdenyrelaytime').removeAttr('disabled');
             $('#GRITrigger_DenyRelayTimeInMS').removeAttr('disabled');
         }
         else {
             $('#relaytime').attr({
                 'disabled': 'disabled'
             });
             $('#playerrelaytimelabel').attr({
                 'disabled': 'disabled'
             });
             $('#playerrelaytime').attr({
                 'disabled': 'disabled'
             });
             $('#GRITrigger_RelayTimeInMS').attr({
                 'disabled': 'disabled'
             });

             $('#denyrelaytime').attr({
                 'disabled': 'disabled'
             });
             $('#playerdenyrelaytimelabel').attr({
                 'disabled': 'disabled'
             });
             $('#playerdenyrelaytime').attr({
                 'disabled': 'disabled'
             });
             $('#GRITrigger_DenyRelayTimeInMS').attr({
                 'disabled': 'disabled'
             });
         }
     });
 });



/////////////////////////////////////////////////////////////////////
// Enable/Disable for Password controls (disables validation when 'clearing' pwd
/////////////////////////////////////////////////////////////////////
 $(document).ready(function () {
     $('#RemovePassword').click(function () {
         if (document.getElementById('RemovePassword').checked) {
             $('#oldpassword').attr({
                 'disabled': 'disabled'
             });
             $('#newpassword').attr({
                 'disabled': 'disabled'
             });
             $('#confirmpassword').attr({
                 'disabled': 'disabled'
             });
         }
         else {
            $('#oldpassword').removeAttr('disabled');
             $('#newpassword').removeAttr('disabled');
             $('#confirmpassword').removeAttr('disabled');
         }
     });
 });


 //////////////////////////////////////////////
 // Find containing parent tab of the provided element
 //////////////////////////////////////////////
 function FindParentTab(el, strId) {
     strId = strId.toLowerCase();

     while (el && el.parentNode)
     {
         el = el.parentNode;

         if ((el.id != "") && (el.id !== undefined))
         {
             var testId = el.id;

             if (testId.indexOf(strId) > -1)
                 return el;
         }
     }

     return null;
 }
 

 //////////////////////////////////////////////
 // Instantiate our bValidator object
 //////////////////////////////////////////////
 $(document).ready(function () {
     // Skip the password fields for a normal 'save' operation
     var webconfigoptions = {
         /// Skip all of this for now...
         onAfterElementValidation: function (element, arErrorMsgs) {

             // for some reason, the supplied element does not have a parentnode reference...
             // here we just use the passed in element id to grab the element locally and use that...
             var theElement = document.getElementById(element.attr('id'));

             var parentTab = FindParentTab(theElement, "tab-content");

             if (null != parentTab) {
                 if (arErrorMsgs.length > 0) {
                     $("#" + parentTab.id + "-label").css('background-image', 'url(/img/validationfailed.png)');
                     $("#" + parentTab.id + "-label").css('background-repeat', 'no-repeat');
                     $("#" + parentTab.id + "-label").css('background-position', '3px');
                 }
             }
         },
         //
         forceValidAttr: 'data-bvalidator-skiponsave',
         validateOn: 'keyup',
         position: { x: 'left', y: 'top' },
         offset: { x: 0, y: -4 }
     };

     // only validate the password fields when the password reset button is clicked...
     var passwordfieldsoptions = {
         validateOn: 'keyup',
         position: { x: 'left', y: 'top' },
         offset: { x: 0, y: -4 }
     };

     $('#webconfig').bValidator(webconfigoptions);
     $('#passwordfields').bValidator(passwordfieldsoptions); // for the passwords <div>
 });


///////////////////////////////////////////////
// Instantiate our ToolTip object
///////////////////////////////////////////////
// Activate our slider toop tip...
 $(document).ready(function () {
     ConfigureToolTipster();
 });

///////////////////////////////////////////////
// Handle Checkbox Events
///////////////////////////////////////////////

function setEnableNegativeMatchTimeout(element) {
	if(element.is(":checked")) {
		$('#negmatchtimeout').slider("enable");
		$('#negmatchreset').slider("enable");
		$('#Eyelock_NegativeMatchTimeout').attr("readonly", false);
		$('#Eyelock_NegativeMatchResetTimer').attr("readonly", false);
	} else {
		$('#negmatchtimeout').slider("disable");
		$('#negmatchreset').slider("disable");
		$('#Eyelock_NegativeMatchTimeout').attr("readonly", true);
		$('#Eyelock_NegativeMatchResetTimer').attr("readonly", true);
	}
}

$(document).ready(function () {
     $('#Eyelock_EnableNegativeMatchTimeout').change(function() {
        setEnableNegativeMatchTimeout($(this));
     });

     setEnableNegativeMatchTimeout($('#Eyelock_EnableNegativeMatchTimeout'));
 });


