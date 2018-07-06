function ConfigureToolTipster() {
    var bEnabled = true;

    if (document.getElementById('Eyelock_EnablePopupHelp') != null)
        bEnabled = document.getElementById('Eyelock_EnablePopupHelp').checked;

    var DelayEdit = document.getElementById('Eyelock_PopupHelpDelay');


    // Always destroy the current instance (if it exists)
    if (!bEnabled) {
        try {
            $('.tooltip').tooltipster('disable'); // Only disable, if we destroy here, the jQuery tooltips popup and they're a pain to suppress
        }
        catch (exception) {
        }
/*
        $('.normallabel').removeattr('cursor');
        $('.mediumlabel').removeattr('cursor');
        $('.widelabel').removeattr('cursor');
*/
    }
    else {
        try {
            $('.tooltip').tooltipster('destroy');
        }
        catch (exception) {
        }

        var cSelect = document.getElementById("helptrigger");

        // If still turned on, go ahead and configure it...
        // Change the timing and enabled status of the popup help...
        $('.tooltip').tooltipster(
            {
                contentAsHTML: true,
                animation: 'grow',
                maxWidth: 400,
                speed: 300,
                interactive: true,
                delay: (DelayEdit != null) ? ((cSelect.options[cSelect.selectedIndex].value == "hover") ? DelayEdit.value * 1000 : 0) : 1000,
                theme: 'webconfig-theme',
                touchDevices: false,
                trigger: (DelayEdit != null) ? ((cSelect.options[cSelect.selectedIndex].value == "hover") ? 'hover' : 'click') : 'hover'
            });

/*
        $('.normallabel').attr({
            'cursor': 'help'
        });
        $('.mediumlabel').attr({
            'cursor': 'help'
        });
        $('.widelabel').attr({
            'cursor': 'help'
        });
*/
    }
}


function ProcessLoggedOnuser()
{
    if (LoggedonUser !== "installer") {
        // disable sliders...
        $('.slidey').css('pointer-events', 'none');
        // allow ptr events on these sliders...
        $('#repeatauth').css('pointer-events', 'auto');
        $('#negmatchtimeout').css('pointer-events', 'auto');
        $('#negmatchreset').css('pointer-events', 'auto');
        $('#helpdelayplayer').css('pointer-events', 'auto');

          $('#volume').css('pointer-events', 'auto');
            $('#ledbrightness').css('pointer-events', 'auto');
              $('#tampertonevol').css('pointer-events', 'auto');

     //   $('#resetpwdbutton').hide();
        $('#factoryresetbutton').hide();
	if(!bAllowSiteAdminUpdates) //allow restore points as admin
	{
        $('#restorefirmwarebutton').hide();
        $('#deletefirmwarebutton').hide();
	}
        $('#acstestbutton').hide();

        // disable all buttons... to be re-enabled individually
       // $('.submit').css('pointer-events', 'none');
        $('#identifydevicebutton').css('pointer-events', 'auto');
        $('#factoryresetbutton').css('pointer-events', 'auto');
        $('#rebootdevicebutton').css('pointer-events', 'auto');
        $('#dhcpsettingsbutton').css('pointer-events', 'auto');
        $('#dhcpsettingsOKbutton').css('pointer-events', 'auto');
        $('#updatetimebutton').css('pointer-events', 'auto');
        $('#refreshlog').css('pointer-events', 'auto');
        $('#downloadlog').css('pointer-events', 'auto');
        $('#helpsettingsOKbutton').css('pointer-events', 'auto');
        $('#msgbox_btn1').css('pointer-events', 'auto');
        $('#msgbox_btn2').css('pointer-events', 'auto');

        if (bAllowSiteAdminUpdates) {
            $('#detailsbutton').css('pointer-events', 'auto');
            $('#manualnano').css('pointer-events', 'auto');
            $('#localfilebutton').css('pointer-events', 'auto');
            $('#updatenowbutton').css('pointer-events', 'auto');
        }

        var str = '';
        var elem = document.getElementById('webconfig').elements;
        for(var i = 0; i < elem.length; i++)
        {
            // Exclude the tabs themselves...
            if (elem[i].id.indexOf('tab') != -1)
                continue;

            // Now exclude all of the 'Help' Settings items...
            if ((elem[i].id === "countrycode") ||
                (elem[i].id === "Eyelock_EnablePopupHelp") ||
                (elem[i].id === "helptrigger") ||
                (elem[i].id === "Eyelock_PopupHelpDelay") ||
                (elem[i].id === "helpsettingsOKbutton"))
                continue;

            //security page: allow the admin user to change his own password
            if(elem[i].id == "oldpassword" || 
            elem[i].id == "newpassword" ||
            elem[i].id == "confirmpassword" ||
            elem[i].id == "resetpwdbutton")
             continue;
            // Now exclude all of the 'Help' Settings items...
            if ((elem[i].id === "GRI_RepeatAuthorizationPeriod") ||
                (elem[i].id === "Eyelock_EnableNegativeMatchTimeout") ||
                (elem[i].id === "Eyelock_NegativeMatchTimeout") ||
                (elem[i].id === "Eyelock_NegativeMatchResetTimer"))
                continue;

            // Now exclude all of the 'Log' tab items...
            if ((elem[i].id === "refreshlog") ||
                (elem[i].id === "downloadlog"))
                continue;
                //device settings tab
            if((elem[i].id === "GRI_AuthorizationToneVolume" ||
            elem[i].id === "volume" ||
            elem[i].id === "ledbrightness" ||
            elem[i].id === "tampertonevol" ||
            elem[i].id === "GRI_TamperToneVolume" ||
            elem[i].id === "identifydevicebutton" ||
            elem[i].id === "GRI_InternetTimeAddr" ||
            elem[i].id === "GRI_InternetTimeSync" ||
            elem[i].id === "Eyelock_HttpPostSenderDestAddress" ||
            elem[i].id === "Eyelock_DeviceLocation" ||
            elem[i].id === "Eyelock_WelcomeMessage" ||
            elem[i].id === "updatetimebutton" ||
            elem[i].id === "updatelocaltimebutton" ||
            elem[i].id === "rebootdevicebutton" ||
            elem[i].id === "GRI_LEDBrightness"))
            continue;



            if ((elem[i].id === "identifydevicebutton") ||
                (elem[i].id === "factoryresetbutton") ||
                (elem[i].id === "rebootdevicebutton") ||
                (elem[i].id === "dhcpsettingsbutton") ||
                (elem[i].id === "dhcpsettingsOKbutton") ||
                (elem[i].id === "helpsettingsOKbutton") ||
                (elem[i].id === "msgbox_btn1") ||
                (elem[i].id === "msgbox_btn2") ||
                (elem[i].id === "updatetimebutton"))
                continue;

            if (bAllowSiteAdminUpdates) {
                if ((elem[i].id === "detailsbutton") ||
                    (elem[i].id === "manualnano") ||
                    (elem[i].id === "updatenowbutton") ||
					 (elem[i].id === "restorefirmwarebutton") ||
					  (elem[i].id === "deletefirmwarebutton") ||
                    (elem[i].id === "localfilebutton"))
                    continue;
            }

            $('#' + elem[i].id).prop("disabled", true);
             $('#submitbutton').prop("disabled", false);
        }
    }
}

function setPlaceHolder(element)
{
    if(element.val() == '')
    {
       if(element.attr('type')=='password')
           element.data('type','password').get(0).type='text';
	
    	element.val(element.attr("placeholder")).addClass('placeholder');          
     	element.css('color','gray');
    }
}

function resetPlaceHolder(element)
{
    if(element.data('type')=='password')
    	element.get(0).type='password';
    if(element.hasClass('placeholder'))
    { 
       element.val('').removeClass('placeholder');
       element.val('').css('color','black');
    }
}



$(document).ready(function () {
    $('#newpassword').change(function () {
        $('#passwordfields').data('bValidator').validate();

    });
    $('#confirmpassword').change(function () {
        $('#passwordfields').data('bValidator').validate();

    });
    $('#oldpassword').change(function () {
        $('#passwordfields').data('bValidator').validate();

    });

});


$(document).ready(function(){
  if(navigator.appVersion.match(/MSIE [\d.]+/)){
    $(document).find("input[placeholder]").each(function(){
        if($.trim($(this).val()) == ""){
           setPlaceHolder($(this));
        }
        $(this).on("focus",function(){
	     resetPlaceHolder($(this));
        }).on("blur",function(){
    	     setPlaceHolder($(this));
        });
    }); 
  }
});
