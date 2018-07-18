// Our ajax object
//var loghttp = createRequestObject();

var nCheckEyelockRunningCount = 5;

var bStillWaitingForResponse = false;
var nCurrentStatusTimeout = 5000;
var bRebootDevice = false;



// When the DOM is loaded we start tailing our log
$(document).ready(function () {
    // Check for and update the copyright text...
    var today = new Date();
    var y = today.getFullYear();

    if (y > 2014)
        $('#copyright').html("Copyright © 2014-" + y + ". All Rights Reserved.").text();

    refreshTimeZone();

    tailLog(false);
    // We also grab the device time
    sndReq('/scripts/rpc.php', 'gettime', "", false);
    // We also grab our database template count...
    //sndReq('/scripts/rpc.php', 'getdb', "", false);
    // And our ACS test details...
     sndReq('/scripts/rpc.php', 'getacstestdata', "", false); 
      setTimeout(function (){  
      setTimeout(function (){  sndReq('/scripts/rpc.php', 'getacstestdata', "", false); }, 15000);// every 15 seconds we update this, just in case we missed it the first time
      sndReq('/scripts/rpc.php', 'getacstestdata', "", false); 
      
      }, 5000); //wait for the app to start (it probably isn't started)

  
    // and our restore points
    sndReq('/scripts/rpc.php', 'getrestorepoints', "", false);
    CheckforEyelockRunning(5000); // starts a timer and keeps pining for eyelock running...
    CheckDeviceStatus(5000);

    $('#templatesavailable').text(GlobalStrings.MsgUpdating);
    $('#templatesavailable2').text(GlobalStrings.MsgUpdating);
    CheckforDBTemplates(10000);
    CheckUpgrade(); // Check for nano upgrade...

    ProcessLoggedOnuser(); // configure elements as read only if logged on as admin...
});

var bLoadLogDlgShow = false;

function tailLog(bShowWaiting)
{
    var url = "/scripts/logtail.php";

    bLoadLogDlgShown = bShowWaiting;

    if (bShowWaiting) {
        ShowWaitingDlg(GlobalStrings.LoadingLogDetail, "", true, 30000);
    }

     $.ajax({

         cache: false,
        url: url,
        type: 'GET',
        success: function (data, textStatus, jqXHR) {
            //data - response from server
            updateDisplay(data);
			HideOnlyWaitingDlg();
        },
        error: function (jqXHR, textStatus, errorThrown) {

        }
    });

}


function autotailLog()
{
    tailLog(false);

    // As long the checkbox is checked, keep on refreshing...
    if (document.getElementById('autorefreshlog').checked)
        setTimeout(autotailLog, document.getElementById('autorefreshseconds').value*1000);
}

function refreshTimeZone()
{
    tmp = new Date().toString();
    tmp = tmp.split(':')[2];
    timeZone = tmp.substring(tmp.indexOf(' ')+1);
    $('#logTimeZone').html(timeZone);
}

function updateDisplay(theResponse)
{
	 refreshTimeZone();

        var currentLogValue = new Array();

        currentLogValue = theResponse.trim().split("\n");

        logDiv = document.getElementById("logdisplay");

        var logLine = ' ';
        var imgLine = ' ';

        logLine = "<div id='logheaderdiv' style='border-top: solid 1px #a6b969; border-left: solid 1px #a6b969; border-right: solid 1px #a6b969; margin-left: 2%; font-size: 12px; margin-top: 10px; margin-bottom: 0px; width: 96%; text-align: left;overflow: auto'>";
        logLine += "<table id='logheadertable' class='sizeabletable' width='100%' border='0' cellpadding='0' cellspacing='0' style='table-layout:fixed;resize:horizontal'>";
		logLine += "<tr><th style='width:17em' class='right'>" + GlobalStrings.LogHeaderStatus + "</th><th style='width:12em' class='right'>" + GlobalStrings.LogHeaderDate + "</th><th style='width:9em' class='right'>" + GlobalStrings.LogHeaderName + "</th><th style='width:7em' class='right'>" + GlobalStrings.LogHeaderCode + "</th><th>" + GlobalStrings.LogHeaderMessage +"/GUID" + "</th></tr>";
        logLine += "</table></div>"

            
        logLine += "<div style='overflow-y: auto; border-bottom: solid 1px #a6b969; border-left: solid 1px #a6b969; border-right: solid 1px #a6b969; height: 310px; margin-left: 2%; font-size: 12px; margin-top: 0px; margin-bottom: 10px; width: 96%; text-align: left;'>"

        if (currentLogValue.length <= 0)
            logLine += Globalstrings.logNoInfo;
        else {
            logLine += "<table id='logtable' class='sizeabletable' tabindex='0' width='100%' border='0' cellpadding='0' cellspacing='0' style='table-layout:fixed;resize:horizontal'>";

            for (i = currentLogValue.length-1; i >= 0; i--)
            {
                var strCurrentLogValue = currentLogValue[i].trim();

                // Skip any blank lines
                if (strCurrentLogValue === "")
                    continue;

                //skip entries for when tail can't open a file
                if (strCurrentLogValue === "tail: no files")
                    continue;
                if (strCurrentLogValue === "tail: can't open '/home/root/nxtEvent.log.1': No such file or directory")
                    continue;

                
                // Process for correct ICON and ICON area Text
                var bSuccess = false;
                //Match success(Duress) ID is Fang  J|132 2253|01b75e30-0f28-4da6-bbbe-02e287c30027
                if ((strCurrentLogValue.indexOf("Success(Duress)") > -1) || (strCurrentLogValue.indexOf("success(Duress)") > -1)) {
                    bSuccess = true;
                    imgLine = "<img src='/img/icon-success.png' alt='Success' style='margin-left:.5em;vertical-align:middle'><span style='margin-left:.5em;'>" + GlobalStrings.matchSuccessDuress + "</span>";
                }
                else if ((strCurrentLogValue.indexOf("Match Success") > -1) || (strCurrentLogValue.indexOf("Match success") > -1))
                {
                    bSuccess = true;
                    imgLine = "<img src='/img/icon-success.png' alt='Success' style='margin-left:.5em;vertical-align:middle'><span style='margin-left:.5em;'>" + GlobalStrings.matchSuccess + "</span>";
                }
                else if ((strCurrentLogValue.indexOf("Match failure, no iris present") > -1))
                    imgLine = "<img src='/img/icon-warning.png' alt='Failed' style='margin-left:.5em;vertical-align:middle'><span style='margin-left:.5em;'>" + GlobalStrings.matchFailNoIris + "</span>";
                else if ((strCurrentLogValue.indexOf("Match failure, iris mismatch") > -1))
                    imgLine = "<img src='/img/icon-warning.png' alt='Failed' style='margin-left:.5em;vertical-align:middle'><span style='margin-left:.5em;'>" + GlobalStrings.matchFailMismatch + "</span>";
                else if ((strCurrentLogValue.indexOf("Match failure, invalid card") > -1))
                    imgLine = "<img src='/img/icon-warning.png' alt='Failed' style='margin-left:.5em;vertical-align:middle'><span style='margin-left:.5em;'>" + GlobalStrings.matchFailInvalidCard + "</span>";
                //   Match failure, invalid PIN number - ID:  Invalid PIN number: 01-02-03-04
                else if ((strCurrentLogValue.indexOf("Match failure, invalid PIN number - ID") > -1))
                    imgLine = "<img src='/img/icon-warning.png' alt='Failed' style='margin-left:.5em;vertical-align:middle'><span style='margin-left:.5em;'>" + GlobalStrings.matchFailInvalidPIN + "</span>";
                //Match failure, invalid PIN number - Fang  J|132 2253|01b75e30-0f28-4da6-bbbe-02e287c30027 Invalid PIN number: 01-02-03-05
                else if ((strCurrentLogValue.indexOf("Match failure, invalid PIN number") > -1))
                    imgLine = "<img src='/img/icon-warning.png' alt='Failed' style='margin-left:.5em;vertical-align:middle'><span style='margin-left:.5em;'>" + GlobalStrings.matchFailInvalidPIN + "</span>";
                    //Match failure, no PIN present - Fang  J|132 2253|01b75e30-0f28-4da6-bbbe-02e287c30027
                else if ((strCurrentLogValue.indexOf("Match failure, no PIN present") > -1))
                    imgLine = "<img src='/img/icon-warning.png' alt='Failed' style='margin-left:.5em;vertical-align:middle'><span style='margin-left:.5em;'>" + GlobalStrings.matchFailNoPIN + "</span>";
                else if ((strCurrentLogValue.indexOf("failed") > -1) || (strCurrentLogValue.indexOf("Failed") > -1))
                    imgLine = "<img src='/img/icon-warning.png' alt='Failed' style='margin-left:.5em;vertical-align:middle'><span style='margin-left:.5em;'>" + GlobalStrings.MessageBoxFailed + "</span>";
                else if ((strCurrentLogValue.indexOf("TAMPER") > -1) || (strCurrentLogValue.indexOf("tamper") > -1))
                    imgLine = "<img src='/img/icon-tamperinformation.png' alt='Tampered' style='margin-left:.5em;vertical-align:middle'><span style='margin-left:.5em;'>" + GlobalStrings.MessageBoxTampered + "</span>";
                else
                    imgLine = "<img src='/img/icon-information.png' alt='Failed' style='margin-left:.5em;vertical-align:middle'><span style='margin-left:.5em;'>" + GlobalStrings.MessageBoxInfo + "</span>";


                var subStrings = strCurrentLogValue.split(">");
                var dateLine = subStrings[0].split(',')[0];
		  		dateLine = dateLine.replace(/-/g,'/')+' UTC';
		 		d = new Date(dateLine);
		  		dateLine =  twoDigit(d.getMonth()+1)  + '-';
		  		dateLine += twoDigit(d.getDate()) + '-';
    		  	dateLine += d.getFullYear()  + ' ';
    		  	dateLine += twoDigit(d.getHours()) + ':';
    		  	dateLine += twoDigit(d.getMinutes()) + ':';
    		  	dateLine += twoDigit(d.getSeconds());

                // Strip the 'day of week' off the front of the date
         //       var subDateLine = dateLine.split(", ");

                var msgLine = subStrings[1];

                //saw this anomaly:  Bob => (version)
                if (msgLine === " BoB =")
                    msgLine += subStrings[2];

                // Always start with icon and date...
                logLine += "<tr><td style='width:17em; vertical-align:middle' class='right'>";
                logLine += imgLine;
                logLine += "</td><td style='width:12em;' class='right center'>";
                logLine += dateLine;//subDateLine[1];


                // Ok, now for the 'success' case we need to subdivde the msgLine
                if (bSuccess)
                {
                    //Match success(Duress) ID is F  J|132 2253|01b75e30-0f28-4da6-bbbe-02e287c30027
                    var subSuccessStrings = msgLine.split("|");
                    var subMatchStrings = subSuccessStrings[0].split("ID is ");

                    // Ok, we will either have 3 or only 2...
                    // name | access | GUID   name | GUID
                    if (subSuccessStrings.length == 3)
                    {
                 //       logLine += "</td><td style='width:15em; text-overflow:ellipsis;overflow:hidden;white-space: nowrap;' class='right'>";
                        logLine += "</td><td style='width:9em;' class='right indent'>";
                        logLine += subMatchStrings[1];
                        logLine += "</td><td style='width:7em;' class='right center'>";
                        logLine += subSuccessStrings[1];
                        logLine += "</td><td class='indent'>";
                        logLine += subSuccessStrings[2];
                        logLine += "</td></tr>";
                    }
                    else if (subSuccessStrings.length == 2)
                    {
                        logLine += "</td><td style='width:9em;' class='right indent'>";
                        logLine += subMatchStrings[1];
                        logLine += "</td><td style=style='width:7em;' class='right center'>";
                        logLine += GlobalStrings.LogUnavailable;
                        logLine += "</td><td class='indent'>";
                        logLine += subSuccessStrings[1];
                        logLine += "</td></tr>";
                    }
                    else
                        bSuccess = false;
                }

                // Just the message
                if (!bSuccess)
                {
                    var name = "";
                    var msg = "";
                    var card = "";

                    
                    if (((strCurrentLogValue.indexOf("Match failure, invalid card") > -1)) || ((strCurrentLogValue.indexOf("Match failure, invalid PIN number") > -1) && (strCurrentLogValue.indexOf("|") == -1)) ||
                    (strCurrentLogValue.indexOf("Match failure, invalid PIN number - ID") > -1))
                     {
                        var stattok = msgLine.split('-');
                        for (xi = 0; xi < stattok.length; xi++) {
                            if (xi > 0) {
                                msg += stattok[xi];

                                if (xi != stattok.length - 1)
                                    msg += "-";
                            }
                        }
                    }
                    //Match failure, invalid PIN number - F  J|132 2253|01b75e30-0f28-4da6-bbbe-02e287c30027 Invalid PIN number: 01-02-03-04
                    else if (((strCurrentLogValue.indexOf("Match failure, no iris present") > -1)) ||
                            ((strCurrentLogValue.indexOf("Match failure, iris mismatch") > -1)) ||
                            (strCurrentLogValue.indexOf("Match failure, invalid PIN number") > -1) ||
                            (strCurrentLogValue.indexOf("Match failure, no PIN present") > -1))
                    {
                        var tokens = msgLine.split('|');
                        //status - name|card|id
                        var stattok = tokens[0].split('-');
                        name = stattok[1];

                        if ((strCurrentLogValue.indexOf("Match failure, no iris present") == -1) || ((strCurrentLogValue.indexOf("Match failure, no iris present") > -1) &&  (strCurrentLogValue.indexOf("|") > -1)))
                        {
                            card = tokens[1];

                            // special case, must strip GUID
                            if (strCurrentLogValue.indexOf("Match failure, invalid PIN number") > -1)
                            {
                                if (tokens.length > 2)
                                {
                                    var PINtok = tokens[2].split(' ');

                                    // grab the reset of the line minus the GUID
                                    for (yi = 1; yi < PINtok.length; yi++)
                                    {
                                        msg += PINtok[yi];
                                        msg += " ";
                                    }
                                }
                            }
                            else
                                msg = tokens[2];
                        }
                    }
                    else
                        msg = msgLine;


                    logLine += "</td><td style='width:9em;' class='right indent'>" + name; //name
                    logLine += "</td><td style='width:7em;' class='right center'>" + card; // code
                    logLine += "</td><td class='indent''>" + msg; //msg
                    logLine += "</td></tr>";
                }
            }

            logLine += "</table>";
        }

        logLine += "</div>";

        logDiv.innerHTML = logLine;

        if (bLoadLogDlgShown)
            HideWaitingDlg();
}

