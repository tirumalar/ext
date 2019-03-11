$(function () {
    $("#repeatauth").slider({
        min: 1,
        max: 60,
        value: document.getElementById('GRI_RepeatAuthorizationPeriod').value,
        range: "min",
        animate: true,
        step: .5,
        slide: function (event, ui) {
            if (ui.value < 2)
                return false;
            setRepeatPeriod(ui.value);
        },
        create: function (event, ui) {
            setSliderTicks(event.target);
        }
    });
});


function setRepeatPeriod(myPeriod) 
{
    var periodEdit = document.getElementById('GRI_RepeatAuthorizationPeriod');
    periodEdit.value = myPeriod;
    
    // Here, we need to set this on our ui-slider-handle element
//    $("#repeatauth").find(".ui-slider-handle").tooltipster('content', myPeriod);
}

$(document).ready(function(){
    $('#GRI_RepeatAuthorizationPeriod').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#repeatauth").slider("value", parseFloat(debugvalue));
    })

})



$(function () 
{
    $("#iriswaittimeslider").slider({
        min: 1,
        max: 60,
        value: document.getElementById('GRITrigger_DualAuthNCardMatchWaitIrisTime').value,
        range: "min",
        animate: true,
        step: .5,
        slide: function (event, ui)
        {
             if (ui.value < 2)
                return false;
            setIrisWaitPeriod((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});

/////////////////////////// IrisCapture

$(function () {
    $("#iriscapturetimeout").slider({
        min: 1000,
        max: 60000,
        value: document.getElementById('Eyelock_IrisCaptureTimeout').value,
        range: "min",
        animate: true,
        step: 50,
        slide: function (event, ui) {
            if (ui.value < 1000)
                return false;
            if (ui.value > 60000)
                ui.value = 60000;
            setIrisCaptureTimeout((ui.value));
        },
        create: function (event, ui) {
            setSliderTicks(event.target);
        }
    });
});


$(document).ready(function () {
    $('#CM_IrisCaptureTimeout').change(function () {
        //do your update here
        debugvalue = this.value;
        $("#iriscapturetimeout").slider("value", parseFloat(debugvalue));
    })
})

function setIrisCaptureTimeout(myTimeout) 
{
    var timeoutEdit = document.getElementById('Eyelock_IrisCaptureTimeout');
    timeoutEdit.value = myTimeout;
}


$(function () {
    $("#iriscapturedelay").slider({
        min: 1000,
        max: 300000,
        value: document.getElementById('Eyelock_IrisCaptureResetDelay').value,
        range: "min",
        animate: true,
        step: 50,
        slide: function (event, ui) {
            if (ui.value < 1000)
                return false;
            if (ui.value > 300000)
                ui.value = 300000;
            setIrisCaptureResetDelay((ui.value));
        },
        create: function (event, ui) {
            setSliderTicks(event.target);
        }
    });
});


$(document).ready(function () {
    $('#CM_IrisCaptureResetDelay').change(function () {
        //do your update here
        debugvalue = this.value;
        $("#iriscapturedelay").slider("value", parseFloat(debugvalue));
    })
})

function setIrisCaptureResetDelay(myTimeout) 
{
    var timeoutEdit = document.getElementById('Eyelock_IrisCaptureResetDelay');
    timeoutEdit.value = myTimeout;
}


$(function () {
    $("#imagequality").slider({
        min: 1,
        max: 100,
        value: document.getElementById('Eyelock_J2KImageQuality').value,
        range: "min",
        animate: true,
        step: 1,
        slide: function (event, ui) {
            if (ui.value < 0)
                return false;
            if (ui.value > 100)
                ui.value = 100;
            setImageQualityTimeout((ui.value));
        },
        create: function (event, ui) {
            setSliderTicks(event.target);
        }
    });
});


$(document).ready(function () {
    $('#CM_ImageQuality').change(function () {
        //do your update here
        debugvalue = this.value;
        $("#imagequality").slider("value", parseFloat(debugvalue));
    })
})

function setImageQualityTimeout(myTimeout) 
{
    var timeoutEdit = document.getElementById('Eyelock_J2KImageQuality');
    timeoutEdit.value = myTimeout;
}


///////////////////////////

$(function () {
    $("#iriswaittimeslider2").slider({
        min: 10,
        max: 600,
        value: document.getElementById('GRITrigger_DualAuthNCardMatchWaitIrisTime2').value,
        range: "min",
        animate: true,
        step: 15,
        slide: function (event, ui) {
            if (ui.value < 10)
                return false;
            if (ui.value > 600)
                ui.value = 600;
            setIrisWaitPeriod2((ui.value));
        },
        create: function (event, ui) {
            setSliderTicks(event.target);
        }
    });
});

$(document).ready(function(){
    $('#GRITrigger_DualAuthNCardMatchWaitIrisTime').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#iriswaittimeslider").slider("value", parseFloat(debugvalue));
    })
})

$(document).ready(function () {
    $('#GRITrigger_DualAuthNCardMatchWaitIrisTime2').change(function () {
        //do your update here
        debugvalue = this.value;
        // $('#GRITrigger_DualAuthNCardMatchWaitIrisTime').value = debugvalue; 
        //setIrisWaitPeriod(debugvalue);
       // $("#iriswaittimeslider").slider("value", parseFloat(debugvalue))
        $("#iriswaittimeslider2").slider("value", parseFloat(debugvalue));
    })
})

function setIrisWaitPeriod(myTimeout) 
{
    var timeoutEdit = document.getElementById('GRITrigger_DualAuthNCardMatchWaitIrisTime');
    timeoutEdit.value = myTimeout;
}
function setIrisWaitPeriod2(myTimeout) 
{
    var timeoutEdit = document.getElementById('GRITrigger_DualAuthNCardMatchWaitIrisTime2');
    timeoutEdit.value = myTimeout;
}


$(document).ready(function(){
    $('#Eyelock_WaitPinTime').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#waitpintimeslider").slider("value", parseFloat(debugvalue));
    })
})


$(function () 
{
    $("#waitpintimeslider").slider({
        min: 1,
        max: 60,
        value: document.getElementById('Eyelock_WaitPinTime').value,
        range: "min",
        animate: true,
        step: .5,
        slide: function (event, ui)
        {
             if (ui.value < 2)
                return false;
            setPINWaitPeriod((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});



function setPINWaitPeriod(myTimeout) 
{
    var timeoutEdit = document.getElementById('Eyelock_WaitPinTime');
    timeoutEdit.value = myTimeout;
}



$(function () 
{
    $("#negmatchtimeout").slider({
        min: 1,
        max: 60,
        value: document.getElementById('Eyelock_NegativeMatchTimeout').value,
        range: "min",
        animate: true,
        step: .5,
        slide: function (event, ui)
        {
             if (ui.value < 2)
                return false;
            setTimeoutPeriod((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});
$(document).ready(function(){
    $('#Eyelock_NegativeMatchTimeout').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#negmatchtimeout").slider("value", parseFloat(debugvalue));
    })

})

function setTimeoutPeriod(myTimeout) 
{
    var timeoutEdit = document.getElementById('Eyelock_NegativeMatchTimeout');
    timeoutEdit.value = myTimeout;
}

$(function () 
{
    $("#negmatchreset").slider({
        min: 1,
        max: 60,
        value: document.getElementById('Eyelock_NegativeMatchResetTimer').value,
        range: "min",
        step: .5,
        animate: true,
        slide: function (event, ui)
        {
             if (ui.value < 2)
                return false;
            setNegMatchResetTimer((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});
$(document).ready(function(){
    $('#Eyelock_NegativeMatchResetTimer').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#negmatchreset").slider("value", parseFloat(debugvalue));
    })

})
function setNegMatchResetTimer(theDuration) 
{
    var NegMatchResetEdit = document.getElementById('Eyelock_NegativeMatchResetTimer');
    NegMatchResetEdit.value = theDuration;
}


$(function () 
{
    $("#volume").slider({
        min: 0,
        max: 100,
        value: document.getElementById('GRI_AuthorizationToneVolume').value,
        range: "min",
        animate: true,
        step: 10,
        slide: function (event, ui)
        {
            setVolume((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});
$(document).ready(function(){
    $('#GRI_AuthorizationToneVolume').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#volume").slider("value", parseFloat(debugvalue));
    })

})

function setVolume(myVolume) 
{
    var volEdit = document.getElementById('GRI_AuthorizationToneVolume');
    volEdit.value = myVolume;
}


/* Out
$(function () 
{
    $("#frequency").slider({
        min: 220,
        max: 880,
        value: document.getElementById('GRI_AuthorizationToneFrequency').value,
        range: "min",
        step: 10,
        animate: true,
        slide: function (event, ui)
        {
            setFrequency((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});

function setFrequency(myFrequency) 
{
    var freqEdit = document.getElementById('GRI_AuthorizationToneFrequency');
    freqEdit.value = myFrequency;
}
*/

/*
$(function () 
{
    $("#toneduration").slider({
        min: 1,
        max: 5,
        value: document.getElementById('GRI_AuthorizationToneDurationSeconds').value,
        range: "min",
        step: 1,
        animate: true,
        slide: function (event, ui)
        {
            setToneDuration((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});

function setToneDuration(theDuration) 
{
    var DurationEdit = document.getElementById('GRI_AuthorizationToneDurationSeconds');
    DurationEdit.value = theDuration;
}
*/


$(function () 
{
    $("#ledbrightness").slider({
        min: 0,
        max: 100,
        value: document.getElementById('GRI_LEDBrightness').value,
        range: "min",
        step: 1,
        animate: true,
        slide: function (event, ui)
        {
            setBrightness((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});

function setBrightness(theBrightness) 
{
    var LEDEdit = document.getElementById('GRI_LEDBrightness');
    LEDEdit.value = theBrightness;
}
$(document).ready(function(){
    $('#GRI_LEDBrightness').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#ledbrightness").slider("value", parseFloat(debugvalue));
    })

})

$(function () 
{
    $("#tampertonevol").slider({
        min: 0,
        max: 100,
        value: document.getElementById('GRI_TamperToneVolume').value,
        range: "min",
        animate: true,
        step: 10,
        slide: function (event, ui)
        {
            setTamperVolume((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});


function setTamperVolume(myVolume) 
{
    var volEdit = document.getElementById('GRI_TamperToneVolume');
    volEdit.value = myVolume;
}

$(document).ready(function(){
    $('#GRI_TamperToneVolume').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#tampertonevol").slider("value", parseFloat(debugvalue));
    })

})


$(function () 
{
    $("#dhcptimeoutplayer").slider({
        min: 10,
        max: 120,
        value: document.getElementById('DHCP_Timeout').value,
        range: "min",
        step: 1,
        animate: true,
        slide: function (event, ui)
        {
            setDHCPTimeout((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});
$(document).ready(function(){
    $('#DHCP_Timeout').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#dhcptimeoutplayer").slider("value", parseFloat(debugvalue));
    })

})

function setDHCPTimeout(theTimeout) 
{
    var TOEdit = document.getElementById('DHCP_Timeout');
    TOEdit.value = theTimeout;
}

$(function () 
{
    $("#dhcpretriesplayer").slider({
        min: 0,
        max: 5,
        value: document.getElementById('DHCP_Retries').value,
        range: "min",
        step: 1,
        animate: true,
        slide: function (event, ui)
        {
            setDHCPRetries((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});



function setDHCPRetries(theRetries) 
{
    var RetriesEdit = document.getElementById('DHCP_Retries');
    RetriesEdit.value = theRetries;
}
$(document).ready(function(){
    $('#DHCP_Retries').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#dhcpretriesplayer").slider("value", parseFloat(debugvalue));
    })

})

$(function () 
{
    $("#dhcpretrydelayplayer").slider({
        min: 0,
        max: 60,
        value: document.getElementById('DHCP_RetryDelay').value,
        range: "min",
        step: 1,
        animate: true,
        slide: function (event, ui)
        {
            setDHCPRetryDelay((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});
$(document).ready(function(){
    $('#DHCP_RetryDelay').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#dhcpretrydelayplayer").slider("value", parseFloat(debugvalue));
    })

})

function setDHCPRetryDelay(theDelay) 
{
    var DelayEdit = document.getElementById('DHCP_RetryDelay');
    DelayEdit.value = theDelay;
}



$(function () 
{
    $("#helpdelayplayer").slider({
        min: 0,
        max: 5,
        value: document.getElementById('Eyelock_PopupHelpDelay').value,
        range: "min",
        step: .05,
        animate: true,
        slide: function (event, ui)
        {
            setHelpPopupDelay((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});

$(document).ready(function(){
    $('#Eyelock_PopupHelpDelay').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#helpdelayplayer").slider("value", parseFloat(debugvalue));
    })

})

function setHelpPopupDelay(theDelay) 
{
    var DelayEdit = document.getElementById('Eyelock_PopupHelpDelay');
    DelayEdit.value = theDelay;
}

//////////////////////////////////////////////////////////////
// ACS Page
//////////////////////////////////////////////////////////////

$(function () 
{
    $("#relaytime").slider({
        min: 0,
        max: 10,
        value: document.getElementById('GRITrigger_RelayTimeInMS').value,
        range: "min",
        step: .1,
        animate: true,
        slide: function (event, ui)
        {
            setRelayTime((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});

$(document).ready(function(){
    $('#GRITrigger_RelayTimeInMS').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#relaytime").slider("value", parseFloat(debugvalue));
    })

})

function setRelayTime(theDuration) 
{
    var RelayTimeEdit = document.getElementById('GRITrigger_RelayTimeInMS');
    RelayTimeEdit.value = theDuration;
}


$(function () 
{
    $("#denyrelaytime").slider({
        min: 0,
        max: 10,
        value: document.getElementById('GRITrigger_DenyRelayTimeInMS').value,
        range: "min",
        step: .1,
        animate: true,
        slide: function (event, ui)
        {
            setDenyRelayTime((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});

$(document).ready(function(){
    $('#GRITrigger_DenyRelayTimeInMS').change(function() {
        //do your update here
        debugvalue = this.value;
        $("#denyrelaytime").slider("value", parseFloat(debugvalue));
    })

})

function setDenyRelayTime(theDuration) 
{
    var RelayTimeEdit = document.getElementById('GRITrigger_DenyRelayTimeInMS');
    RelayTimeEdit.value = theDuration;
}


/*  Below already in Authentication tab
$(function () 
{
    $("#negmatchtimeout").slider({
        min: 0,
        max: 60,
        value: document.getElementById('Eyelock_NegativeMatchTimeout').value,
        range: "min",
        step: .5,
        animate: true,
        slide: function (event, ui)
        {
            setNegMatchTimeout((ui.value));
        },
        create: function( event, ui ) {
            setSliderTicks(event.target);
            }
    });
});

function setNegMatchTimeout(theDuration) 
{
    var NegMatchEdit = document.getElementById('Eyelock_NegativeMatchTimeout');
    NegMatchEdit.value = theDuration;
}
*/



//////////////////////////////////////////////////////
// Spinners (associated with each slider for setting values
//////////////////////////////////////////////////////////////
/**
$(document).ready(function () {
    $(function () {
        $('#GRI_AuthorizationToneVolume').spinner({
        });
    });
});
**/

function setSliderTicks(el) {
    var $slider = $(el);
    var max = $slider.slider("option", "max");
    var min = $slider.slider("option", "min");
    var step = $slider.slider("option", "step");
    var steps = ((max - min) / step);

    //var spacing = 100 / (max - min);
    var spacing = 100 / steps;

    // Never want more than 10 tickmarks otherwise it looks crowded...
    if (spacing < 10) {
        steps = 10;
        spacing = 10;
    }

    $slider.find('.ui-slider-tick-mark').remove();

    for (var i = 0; i <= steps; i++) {
        $('<span class="ui-slider-tick-mark"></span>').css('left', (spacing * i) + '%').appendTo($slider);
    }
};


// Activate our slider toop tip...
// Must remain at bottom of this script...
$(document).ready(function () {
    // Create our tooltipster object for all sliders...
    $('.slidertooltip').tooltipster(
            {
                theme: 'webconfig-theme',
                touchDevices: false,
                trigger: 'hover',
                updateAnimation: false,
                positionTracker: true
            });

    // Now add the slidertooltip class to all slider handles...
    var sliderhandles = document.getElementsByClassName("ui-slider-handle");

    for (var i = 0; i < sliderhandles.length; i++) {
        var handle = sliderhandles[i];
        //handle.classList.add("slidertooltip");
    }
});
