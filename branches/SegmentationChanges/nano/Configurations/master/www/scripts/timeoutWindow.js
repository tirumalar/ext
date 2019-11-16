



 var ignoreTimeout = false;
        var StringHelpers = function () {
            return {
                // Pad string using padMask.  string '1' with padMask '000' will produce '001'.
                padLeft: function (string, padMask) {
                    string = '' + string;
                    return (padMask.substr(0, (padMask.length - string.length)) + string);
                }
            };
        } ();

        var SessionManager = function () {
            // NOTE:  I use @Session.Timeout here, which is Razor syntax, and I am pulling that value
            //        right from the ASP.NET MVC Session variable.  Dangerous!  Reckless!  Awesome-sauce!
            //        You can just hard-code your timeout here if you feel like it.  But I might cry.
            var sessionTimeoutSeconds = 300,
                    countdownSeconds = 60,
                    secondsBeforePrompt = sessionTimeoutSeconds - countdownSeconds,
                    $dlg,
                    displayCountdownIntervalId,
                    promptToExtendSessionTimeoutId,
                    originalTitle = document.title,
                    count = countdownSeconds,
                    extendSessionUrl = '',
                    expireSessionUrl = '/scripts/logout.php';

            var endSession = function () {
                HideMessageBox();
                location.href = expireSessionUrl;
            };

            var displayCountdown = function () {
                var countdown = function () {
                    var cd = new Date(count * 1000),
                            minutes = cd.getUTCMinutes(),
                            seconds = cd.getUTCSeconds(),
                            minutesDisplay = minutes === 1 ? '1 minute ' : minutes === 0 ? '' : minutes + ' minutes ',
                            secondsDisplay = seconds === 1 ? '1 second' : seconds + ' seconds',
                            cdDisplay = minutesDisplay + secondsDisplay;

                   // document.title = 'Expire in ' +
                          //  StringHelpers.padLeft(minutes, '00') + ':' +
                               // StringHelpers.padLeft(seconds, '00');
                    $("msgbox-text1").text("Click Continue to stay logged in. Your session will expire in...");
                    $("#msgbox-text2").text(cdDisplay);
                    // $('#sm-countdown').html(cdDisplay);
                    if (count === 0) {
                        //document.title = 'Session Expired';
                        endSession();
                    }
                    count--;
                };
                countdown();
                displayCountdownIntervalId = window.setInterval(countdown, 1000);
            };

            var promptToExtendSession = function () {
                // $dlg = $('#sm-countdown-dialog')
                //    .dialog({
                //      title: 'Session Timeout Warning',
                ///      height: 205,
                //    width: 250,
                //   bgiframe: true,
                //  modal: true,
                // dialogClass: "msgbox",
                //buttons: {
                //  'Continue': function() {
                //     $(this).dialog('close');
                //    refreshSession();
                //   document.title = originalTitle;
                //},
                //'Log Out': function() {
                //   endSession(false);
                //}
                //}
                //});
                if (ignoreTimeout)
                    return;
                ShowMessageBox(GlobalStrings.SessionTimeoutWarning, GlobalStrings.SessionTimeoutContinue, "", 'MB_ICONWARNING', 'CONTINUELOGOUT', refreshSession, endSession);

                count = countdownSeconds;
                displayCountdown();
            };

            var refreshSession = function () {
                window.clearInterval(displayCountdownIntervalId);
                var img = new Image(1, 1);
                img.src = extendSessionUrl;
                window.clearTimeout(promptToExtendSessionTimeoutId);

                //trigger the cookie refresh
                 sndReq('/scripts/rpc.php', 'extendcookie', "", false);
               // document.title = originalTitle;
                startSessionManager();
            };

            var startSessionManager = function () {
                promptToExtendSessionTimeoutId =
                        window.setTimeout(promptToExtendSession, secondsBeforePrompt * 1000);
            };

            // Public Functions
            return {
                start: function () {
                    startSessionManager();
                },

                extend: function () {
                    refreshSession();
                }
            };
        } ();

        SessionManager.start();

        // Whenever an input changes, extend the session,
        // since we know the user is interacting with the site.
        $(':input').change(function () {
            SessionManager.extend();
        });


$(document).ready(function () {

    var str = '';
    var elem = document.getElementById('webconfig').elements;
    for (var i = 0; i < elem.length; i++) {
        $('#' + elem[i].id).change(function () {
            SessionManager.extend();
        });
         $('#' + elem[i].id).mouseup(function () {
            SessionManager.extend();
        });
    }
});
   