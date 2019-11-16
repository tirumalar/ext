////////////////////////////////////////////////////
// Manage showing and hiding of a 'dhcp' dialog
////////////////////////////////////////////////////

$(document).ready(function ()
{
    $("#loading-div-background").css({ opacity: 0.5 });
});

function ShowDHCPDlg()
{
    $("#loading-div-background").show();
    $("#dhcp-div-container").show();

//    setTimeout(HideDHCPDlg, 3000);
}


function HideDHCPDlg()
{
    $("#loading-div-background").hide();
    $("#dhcp-div-container").hide();
}


// Validation routine for hostname contents
function validatehostname(hostname)
{
    //first make sure we are only numbers or letters,
     if( /[^a-zA-Z0-9]/.test( hostname) ) 
       return false;
    return true;     
}
