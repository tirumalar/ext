////////////////////////////////////////////////////
// Manage showing and hiding of a 'waiting' dialog
////////////////////////////////////////////////////

$(document).ready(function () {
    bStaticIPReboot = false; // flag to handle static ip reboots differently...
    bRefreshAttempt = false; // flag to handle static ip reboots differently...
    bLocateDeviceCancelled = false; // flag to handle static ip reboots differently...
    nCurrentTimeout = 1000;

    $("#loading-div-background").css({ opacity: 0.5 });
});

function ShowWaitingDlg(detailtext, detailtext2, bTimeout, nDuration) {
    if (('none' == ($('#loading-div-background').css('display'))) || ('none' == ($('#loading-div-container').css('display')))) {
        $("#loading-div-background").show();
        $("#loading-div-container").show();
    }

    if (detailtext != "")
        $("#loading-text").text(detailtext);

    if (detailtext2 != "")
        $("#loading-text-second").text(detailtext2);
    else
        $("#loading-text-second").text("");


    // Normally, code Hides the dialog... if the code fails to return
    // eventually (15 seconds) it will auto close..
    if (bTimeout) {
        setTimeout(HideWaitingDlg, nDuration);
    }
};
///Just hide the waiting dialog. don't do anything else.
function HideOnlyWaitingDlg() {
    //console.log($('#loading-div-background').css('display'));
    //console.log($('#loading-div-container').css('display'));
     if (('none' != ($('#loading-div-background').css('display'))) && ('none' != ($('#loading-div-container').css('display')))) {
        $("#loading-div-background").hide();
        $("#loading-div-container").hide();
    }
}

function HideWaitingDlg() {
    if (('none' != ($('#loading-div-background').css('display'))) && ('none' != ($('#loading-div-container').css('display')))) {
       // $("#loading-div-background").hide();
       // $("#loading-div-container").hide();
    }

    // DMOTODO this whole mech. is a hack... should be fixed...
    // If it was a static IP change... we know that we never got our
    // result from our saveconfig request and therefore we never tried to
    // refresh the page...
    if (bStaticIPReboot) {
        bStaticIPReboot = false;
        bRefreshAttempt = true;
        ShowWaitingDlg(GlobalStrings.RefreshingPage, "", true, 30000);
        if (isCurrentlyStaticIP)
            location.href = "http://" + document.getElementById('ipofboard').value; // reload with new url...
        else
            location.href = "http://" + strDeviceName + ".local";
    }
    else if (bRefreshAttempt) // On a refresh we should never get into HideWaitingDlg unless we timeout on the refresh... If we timeout out... display a message to the user...
    {
        bRefreshAttempt = false;
        //ShowMessageBox(GlobalStrings.ConnectionFailedTitle, GlobalStrings.ConnectionFailedMsg1, GlobalStrings.ConnectionFailedMsg2, "MB_ICONINFORMATION", "OK");
    }

    if(bRebootDevice)
    {
         //SubmitTimeout = setTimeout(CheckforEyelockRunning, nCurrentTimeout);
         //setTimeout(HideWaitingDlg, 60000);
          bRebootDevice = false;
        location.reload();
    }

};

function reloadMe()
{
     bRebootDevice = false;
        location.reload();
}

