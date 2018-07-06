function keymgmtReq(action, displaytext, nDialogTimeout)
{
    var arParameters = [];

    nDefinedTimeout = typeof nDialogTimeout !== 'undefined' ? nDialogTimeout : 30000;

    keymgmtReqParams(action, arParameters, displaytext, nDefinedTimeout);
}

function keymgmtReqParams(action, arParameters, displaytext, nDialogTimeout)
{
    if(displaytext !== 'undefined'){
    	nDefinedTimeout = typeof nDialogTimeout !== 'undefined' ? nDialogTimeout : 30000;

    	// Handle default value...
    	bShowWaitDialog = typeof bShowWaitDialog !== 'undefined' ? bShowWaitDialog : true;
    	//bShowWaitDialog = false; //hide the waiting dialog, we don't use it anym
    	// In here, depending up the 'action' we may want to display
    	// wait dialogs or other such things to update the user
    	// as to the status...
    	if (bShowWaitDialog)
        	ShowWaitingDlg(displaytext, "", true, nDefinedTimeout);
    	else if (typeof displaytext !== 'undefined')
    	{
        	if (displaytext != "")
        	    $("#loading-text").text(displaytext);
    	}
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
        url: '/scripts/keymgmt.php?' + strParameters,
        type: 'GET',
        success: function (data, textStatus, jqXHR) {
            //data - response from server
	     if(data.indexOf('loadallkeys') != -1){
		  handleLoadAllKeys(data.split(':')[1]);
		  HideOnlyWaitingDlg();
	     }
	     else
            	  handleKeyMgmtResponse(data);
        },
        error: function (jqXHR, textStatus, errorThrown) {
        //    console.log(jqXHR.statusText);
        }
    });
}


function handleKeyMgmtResponse(response)
{
    var update = new Array();
    
    if (response.indexOf('|') != -1)
    {
        update = response.split('|');

	 if (update[0] === 'hostexists') {
            if (update[1] === '0') {
   		   addNewKey();
            }else{
		   $('#keyHostName').css('border','1px solid red');
		   ShowMessageBox(GlobalStrings.MessageBoxFailed, 'Invalid host name or host already has a key.', 'Try again with different host name.' , "MB_ICONERROR", "OK");
	     }
        }
        else if (update[0] === 'addnewkey') {
            if (update[1] === 'success') {
                finishSubmit();
            }
        }
	 else if(update[0]=='downloadkey') {
	 	filepath = "4";
		dlname = update[2];
		dlname = dlname.replace(".pfx", ".pfx");
		var ifrm = document.getElementById('downloadframe');
       	ifrm.src = "/scripts/logdownload.php?logfiletype=" + filepath + "&" + "dlfilename=" + dlname;
	 }
	 else if (update[0] === 'regeneratenanokey') {
            if (update[1] === 'success') {
   
            }
        }
        else if(update[0] === 'deleteallkeys')
        {
            if (update[1] === 'success')
                finishSubmit();

        }
	 if(update[0] != 'hostexists')
	  	loadAllKeys();
    }
     tryHideAdd();
}

function tryHideAdd() {
   
}

function handleLoadAllKeys(data)
{
    if(data.indexOf('fail') != -1)
	return;
    localvalidity = 0;
    defaultValidity = 0;
    var html='<table id="keytable" class="sizeabletable" border="0" cellpadding="0" cellspacing="0">';    
    rows = data.trim().split("\\n");
    for (i = 0; i < rows.length; i++)
    {
	if(rows[i]!=''){
		cols = rows[i].split("|");
	
		// cols[0]-->isDevice, cols[1]-->id, cols[2]-->host, cols[3]-->validity
		html += '<tr>';
		html += htmlDeviceAndKey(cols[0], cols[3]); 
	
		if(isKeyValid(cols[3])){
	    	    html += '<td>' + cols[2] + '</td>';
	    	    html += '<td>' + getDateTime(cols[3]) + '</td>';
		}else{
	    	    html += '<td style="color:red">' + cols[2] + '</td>';
	    	    html += '<td style="color:red">' + getDateTime(cols[3]) + '</td>';
		}
	
		html += htmlDloadAndDeleteKey(cols[0], cols[1], cols[2]);	
		html += '</tr>';
		if (i == 1)
		    defaultValidity = cols[3];
		localvalidity = cols[3];
	}
    }
    document.getElementById('keytableparent').innerHTML = html+'</table>';
    if (rows.length >= 3) {

         document.getElementById('keyMgmtDefault').checked = false;
         document.getElementById('keyMgmtCustom').checked = true;
         $('#keymgmtbutton').show();
        $('#addkeybutton').hide();
        $('#deleteallkeysbutton').show();
        
         document.getElementById('validitytag').textContent = "Key Expires: " + getDateTimeUS(localvalidity);
    }
    else
       {
        $('#addkeybutton').show();
        $('#deleteallkeysbutton').hide();
         $('#keymgmtbutton').hide();
         document.getElementById('keyMgmtDefault').checked = true;
         document.getElementById('keyMgmtCustom').checked = false;
       //   document.getElementById('validitytag').textContent = "Key Expires: " + getDateTime(localvalidity);
    }
    //setTimeout(function () { document.getElementById('DefaultKeyExpiry').textContent = "Key Expires: " + getDateTime(defaultValidity); }, 3000);
    
    updateDefaultKeyManagementState();
   
}

function htmlDeviceAndKey(isDevice, validity)
{
	html = '<td><img class="tooltip" style="padding-right: 12px;'; 
	
	if(isDevice=='1')
		html += '" src="/img/simple-nxt-unit.png" alt="NanoNXT Device" height="24" width="40" title="NanoNXT Device">';
	else
		html += 'padding-left:4px" src="/img/system.png" alt="Host System" height="18" width="36" title="Host System">';

	html += '<img class="tooltip" style="padding-right: 12px;" ';
	
	if(isKeyValid(validity))
		html += 'src="/img/valid_key.png" alt="Valid Key" height="22" width="20" title="Valid Key">';
	else
		html += 'src="/img/invalid_key.png" alt="Invalid/Expired Key!" height="22" width="20" title="Invalid/Expired Key!">';

	html += "</td>";
	return html;	
}

function htmlDloadAndDeleteKey(isDevice, id, host)
{
	html = '<td>';
	if(isDevice!='1' && (host != "eyelock-pc"))

		html += '<img style="cursor:pointer;padding-right: 12px;" src="/img/download_key.png" alt="Download Key" height="24" width="18" class="tooltip" title="Download Key" onclick="downloadKey('+id+',\''+host+'\')">';

	html += '<img class="tooltip" style="cursor:pointer;';
	
	if(isDevice=='1')
		html += 'padding-left:36px;" src="/img/spacer.png" alt="Regenerate Key" height="20" width="22">';
	else if(host != "eyelock-pc")
		html += 'padding-left:6px;" src="/img/delete.png" alt="Delete Key" height="22" width="18" title="Delete Key" onclick="deleteKey('+id+')">';
        else
        html += 'padding-left:6px;"  src="/img/spacer.png" alt="" height="22" width="18">';

	html += '</td>';
	return html;	
}

function isKeyValid(validity)
{
	keyDate = new Date(parseInt(validity)*1000);
	curDate = new Date(); 
	if(keyDate<curDate)
		return false;
	else
		return true;
}

function twoDigit(num)
{
   if(num<10)
	return '0'+num;
   return num;
}

function getDateTime(validity)
{
    d = new Date(parseInt(validity)*1000);
    tmp = d.getFullYear() + '-';
    tmp += twoDigit(d.getMonth()+1) + '-';
    tmp += twoDigit(d.getDate()) + ' ';
    tmp += twoDigit(d.getHours()) + ':';
    tmp += twoDigit(d.getMinutes()) + ':';
    tmp += twoDigit(d.getSeconds());
    return tmp;
}

function getDateTimeUS(validity)
{
    d = new Date(parseInt(validity)*1000);

    tmp =  twoDigit(d.getMonth()+1)  + '-';
    tmp +=  twoDigit(d.getDate()) + '-';
    tmp +=  d.getFullYear() + ' ';
    tmp += twoDigit(d.getHours()) + ':';
    tmp += twoDigit(d.getMinutes()) + ':';
    tmp += twoDigit(d.getSeconds());
    return tmp;
}
