////////////////////////////////////////////////////
// Manage showing and hiding of a 'msg box' dialog
////////////////////////////////////////////////////

var gButton1Callback;
var gButton2Callback;

function ShowMessageBox(msgTitle, msgLine1, msgLine2, msgType, btnType, button1Callback, button2Callback)
{
        // set our global callbacks
        gButton1Callback = button1Callback;
        gButton2Callback = button2Callback;

        var strImageSrc;

        switch (btnType) 
        {
            case 'CANCEL':
            {
                $('#msgbox_btn2').hide();
                $('#msgbox_btn1').prop('value', GlobalStrings.MessageBoxCancel);
                break;
            }

            case 'OKCANCEL':
            {
                $('#msgbox_btn2').prop('value', GlobalStrings.MessageBoxCancel);
                $('#msgbox_btn2').show();
                $('#msgbox_btn1').prop('value', GlobalStrings.MessageBoxOk);
                break;
            }
            case 'CONTINUELOGOUT':
            {
                 $('#msgbox_btn2').prop('value', GlobalStrings.BtnLogout);
                $('#msgbox_btn2').show();
                $('#msgbox_btn1').prop('value', GlobalStrings.BtnContinue);
                break;
            }
            case 'YESNO':
            {
                $('#msgbox_btn2').prop('value', GlobalStrings.MessageBoxNo);
                $('#msgbox_btn2').show();
                $('#msgbox_btn1').prop('value', GlobalStrings.MessageBoxYes);
                break;
            }

            default:
            case 'OK':
            {
                $('#msgbox_btn2').hide();
                $('#msgbox_btn1').prop('value', GlobalStrings.MessageBoxOk);
                break;
            }
        }


        switch (msgType)
        {
            case 'MB_ICONLOADING':
            {
                strImageSrc = "/img/flowerloader.gif";
                break;
            }

            case 'MB_ICONWARNING':
            case 'MB_ICONEXCLAMATION':
            {
                strImageSrc = "/img/messagebox_warning_big.png";
                break;
            }

            case 'MB_ICONERROR':
            {
                strImageSrc = "/img/messagebox_error_big.png";
                break;
            }

            case 'MB_ICONVALIDATEFAILED':
            {
                strImageSrc = "/img/messagebox_warning.png";
                break;
            }


            default:
            case 'MB_ICONINFORMATION':
            {
                strImageSrc = "/img/messagebox_information_big.png";
                break;
            }

            case 'MB_ICONSUCCESS':
            {
                strImageSrc = "/img/messagebox_success_big.png";
                break;
            }

            case 'MB_ICONCUSTOM':
            {
                strImageSrc = imgCustom;
                break;
            }
        }


        $('#msgbox-image').attr({ 'src': strImageSrc });
        $('#msgbox-image').hide();
        $('#msgbox-image').show(); // refresh

        if (msgTitle != "")
            $("#msgboxtitletext").text(msgTitle);
        else
            $("#msgboxtitletext").text(GlobalString.MessageBoxInformation);
        $("#msgbox-text1").text(msgLine1);
        $("#msgbox-text2").text(msgLine2);

        $("#loading-div-background").show();
        $("#msgbox-div-container").show();
    }


function HideMessageBox()
{
    $("#loading-div-background").hide();
    $("#msgbox-div-container").hide();
}

$(document).ready(function () {
    $('#msgbox_btn1').click(function () {
	 if(validateAddKey())
		return;
        HideMessageBox();

        if (null != gButton1Callback)
            gButton1Callback(); // Call our custom click handler
    });
});


$(document).ready(function()
 {
    $('#msgbox_btn2').click(function()
    {
        HideMessageBox();

        if (null != gButton2Callback)
            gButton2Callback(); // Call our custom click handler
    });
 });

var isAddKey = false;
function Show_AddKeyMsgBox(msgTitle, msgLine1, msgLine2, msgType, btnType, button1Callback, button2Callback)
{
	isAddKey = true;
	ShowMessageBox(msgTitle, msgLine1, msgLine2, msgType, btnType, button1Callback, button2Callback);
}

function validateAddKey()
{
	if(isAddKey){
		stat = checkHostName();
		stat = checkValidity()||stat;
		return stat;
	}
	return false;
}
