// Globals for restore points...
var arRestoreList = new Array();
var arBobRestoreList = new Array();


function GenerateRestoreTable(strRestoreList, strBobRestoreList) {
    arRestoreList = strRestoreList.split("*");
    arBobRestoreList = strBobRestoreList.split("*");

    restoreDiv = document.getElementById("restoredisplay");
    bobrestoreDiv = document.getElementById("bobrestoredisplay");


    var restoreLine = ' ';
    var bobrestoreLine = ' ';

    // We could do this using the DOM, we'll just write the html instead...
    restoreLine = "<div id='restoreheaderdiv' style='border-top: solid 1px #a6b969; border-left: solid 1px #a6b969; border-right: solid 1px #a6b969; margin-left: 2%; font-size: 12px; margin-top: 1em; margin-bottom: 0px; width: 100%; text-align: left;'>";
    restoreLine += "<table id='restoreheadertable' class='sizeabletable' width='100%' border='0' cellpadding='0' cellspacing='0'>";
	restoreLine += "<tr><th style='width:5em' class='right'>" + GlobalStrings.RestoreHeaderSelect + "</th><th style='width:8em' class='right'>"+GlobalStrings.VersionHeaderSelect+"</th><th>"+GlobalStrings.RestorePointsTimestampHeader+"</th>";
	restoreLine += "</table></div>";


	restoreLine += "<div style='overflow-y: auto; border-bottom: solid 1px #a6b969; border-left: solid 1px #a6b969; border-right: solid 1px #a6b969; height: 150px; margin-left: 2%; font-size: 12px; margin-top: 0px; margin-bottom: 10px; width: 100%; text-align: left;'>";

    if ((arRestoreList.length <= 0) || (arRestoreList[0] == ""))
        restoreLine += GlobalStrings.SoftwareUpdateNoNanoRestorePoints;
    else {
        restoreLine += "<table id='restoretable' class='sizeabletable' width='100%' border='0' cellpadding='0' cellspacing='0'>";


        // Ok, for each restore point, we need to format the filename for display
        // and then create a line item for our list
        // we also store the full filename in a hidden table cell so that we can
        // easily retrieve it on a delete or restore action...
        for (var i = 0, nIndex = arRestoreList.length-1; i < arRestoreList.length; i++, nIndex--) {
            var arDatePart = new Array();

            if (arRestoreList[nIndex] == "")
                continue;

            arDatePart = arRestoreList[nIndex].replace("root_", "").split(".");

            var strDateSplit = arDatePart[0].split("_")[0];
            var strDate = strDateSplit.substring(0, 4) + "/" + strDateSplit.substring(4, 6) + "/" + strDateSplit.substring(6, 8);

            var strTimeSplit = arDatePart[0].split("_")[1];
            var strTime = strTimeSplit.substring(0, 2) + ":" + strTimeSplit.substring(2, 4) + ":" + strTimeSplit.substring(4, 6);

			var date = new Date(strDate + " " + strTime);
			var dateInfo = dateFormat(date, "ddd ddS mmm yyyy hh:MM:ss TT")+"  (UTC)";

			var strVersion = arDatePart[0].split("_")[2];
			if(strVersion)
			 	strVersion += '.' + arDatePart[1] + '.' + arDatePart[2];
			else
				strVersion = '';
				
            // Ok, have our strings... Now generate our table rows...
            strCbxId = "restore_" + nIndex;
            restoreLine += "<td style='width:5em' class='right center'><label><input type='checkbox' id='" + strCbxId + "'/></label>";
            restoreLine += "</td><td style='width:8em' class='right center'>";
            restoreLine += strVersion;
            restoreLine += "</td><td class='indent' style='padding-left:20px'>";
            restoreLine += dateInfo;
            restoreLine += "</td></tr>";
        }

        restoreLine += "</table>";
    }

    restoreLine += "</div>";
    restoreDiv.innerHTML = restoreLine;

    //////////////////////////////////////////////
    // Now populate the bob list
    // We could do this using the DOM, we'll just write the html instead...
    bobrestoreLine = "<div id='restoreheaderdiv' style='border-top: solid 1px #a6b969; border-left: solid 1px #a6b969; border-right: solid 1px #a6b969; margin-left: 2%; font-size: 12px; margin-top: 0px; margin-bottom: 0px; width: 100%; text-align: left;'>";
    bobrestoreLine += "<table id='restoreheadertable' class='sizeabletable' width='100%' border='0' cellpadding='0' cellspacing='0'>";
	bobrestoreLine += "<tr><th style='width:5em' class='right'>" + GlobalStrings.RestoreHeaderSelect + "</th><th>" + GlobalStrings.BobHeaderRestorePoints + "</th>";
	bobrestoreLine += "</table></div>";


	bobrestoreLine += "<div style='overflow-y: auto; border-bottom: solid 1px #a6b969; border-left: solid 1px #a6b969; border-right: solid 1px #a6b969; height: 130px; margin-left: 2%; font-size: 12px; margin-top: 0px; margin-bottom: 10px; width: 100%; text-align: left;'>";

    if ((arBobRestoreList.length <= 0) || (arBobRestoreList[0] == ""))
       bobrestoreLine += GlobalStrings.SoftwareUpdateNoBobRestorePoints;
    else {
        bobrestoreLine += "<table id='restoretable' class='sizeabletable' width='100%' border='0' cellpadding='0' cellspacing='0'>";


        // Ok, for each restore point, we need to format the filename for display
        // and then create a line item for our list
        // we also store the full filename in a hidden table cell so that we can
        // easily retrieve it on a delete or restore action...
        for (var i = 0, nIndex = arRestoreList.length-1; i < arBobRestoreList.length; i++, nIndex--) {
            var arDatePart = new Array();

            if (arBobRestoreList[nIndex] == "")
                continue;

            arDatePart = arBobRestoreList[nIndex].replace("root_", "").split(".");

            var strDateSplit = arDatePart[0].split("_")[0];
            var strDate = strDateSplit.substring(0, 4) + "/" + strDateSplit.substring(4, 6) + "/" + strDateSplit.substring(6, 8);

            var strTimeSplit = arDatePart[0].split("_")[1];
            var strTime = strTimeSplit.substring(0, 2) + ":" + strTimeSplit.substring(2, 4) + ":" + strTimeSplit.substring(4, 6);

            // Ok, have our strings... Now generate our table rows...
            strCbxId = "bobrestore_" + nIndex;
            bobrestoreLine += "<td style='width:5em' class='right center'><label><input type='checkbox' id='" + strCbxId + "'/></label>";
            bobrestoreLine += "</td><td class='indent'>";
            bobrestoreLine += strDate + " " + strTime;
            bobrestoreLine += "</td></tr>";
        }

        bobrestoreLine += "</table>";
    }

    bobrestoreLine += "</div>";
    bobrestoreDiv.innerHTML = bobrestoreLine;

    // Show/hide buttons as appropriate...
    if (true)//document.getElementById('nanorestoreradio').checked)
    {
        if ((arRestoreList.length <= 0) || (arRestoreList[0] == "")) {
            $('#deletefirmwarebutton').hide();
            $('#restorefirmwarebutton').hide();
        }
        else //if (LoggedonUser === "installer") 
		{
            $('#deletefirmwarebutton').show();
            $('#restorefirmwarebutton').show();
        }
    }
    else {
        if ((arBobRestoreList.length <= 0) || (arBobRestoreList[0] == "")) {
            $('#deletefirmwarebutton').hide();
            $('#restorefirmwarebutton').hide();
        }
        else {
            $('#deletefirmwarebutton').show();
            $('#restorefirmwarebutton').show();
        }
    }
	setTimeout(addCheckboxControl, 1000);
}

function addCheckboxControl()
{
	var  arList = arRestoreList;
	 for (var i = arList.length-1; i >= 0; i--)
         {
             var theCheckbox =  "#restore_" + i;

             $(theCheckbox).click(function (event) { //dynamically generated event function!
				 theCheckbox = event.target.id;
				  if (document.getElementById(theCheckbox).checked)
						 {
						  for (var j = arList.length-1; j >= 0; j--)
							 {
								 var theOtherCheckbox = "restore_"  + j;
								 if(theOtherCheckbox == theCheckbox) continue;
								 document.getElementById(theOtherCheckbox).checked = false;
							 }
						 }
			 });
         }
}

/////////////////////////////
// Handle events...
//////////////////////////////
$(document).ready(function () {
    $('#restoredisplay').show();
    $('#bobrestoredisplay').hide();
    if ((arRestoreList.length <= 0) || (arRestoreList[0] == "")) {
        $('#deletefirmwarebutton').hide();
        $('#restorefirmwarebutton').hide();
    }
    else //if (LoggedonUser === "installer") 
	{
        $('#deletefirmwarebutton').show();
        $('#restorefirmwarebutton').show();
    }


    /*
    $('#nanorestoreradio').click(function () {
    $('#restoredisplay').show();
    $('#bobrestoredisplay').hide();
    if ((arRestoreList.length <= 0) || (arRestoreList[0] == "")){
    $('#deletefirmwarebutton').hide();
    $('#restorefirmwarebutton').hide();
    }
    else {
    $('#deletefirmwarebutton').show();
    $('#restorefirmwarebutton').show();
    }
    });
    */
});


 // Set enabled/disabled state of config dependant controls...
 $(document).ready(function () {
     $('#bobrestoreradio').click(function () {
        $('#bobrestoredisplay').show();
        $('#restoredisplay').hide();
        if ((arBobRestoreList.length <= 0) || (arBobRestoreList[0] == "")){
            $('#deletefirmwarebutton').hide();
            $('#restorefirmwarebutton').hide();
        }
        else {
            $('#deletefirmwarebutton').show();
            $('#restorefirmwarebutton').show();
        }
     });
 });

