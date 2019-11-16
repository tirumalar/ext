// JavaScript Document

function lerp(a1, a2, b1, b2, a)
{
    return b1 + (b2 - b1) / (a2 - a1) * (a - a1);
}

function setBLProgress(step)
{
    var imgelement = document.getElementById("uploading");
    var imgdiv = document.getElementById("progressloader");
   imgelement.src = "/img/progressbar.png";
    var width = lerp(0, 10, 0, 250, step);
   imgelement.width = width;
   imgelement.height = 15;
   document.getElementById("uploading").setAttribute("style", "margin-left: 25px");
   document.getElementById("uploading").setAttribute("align", "left");
  
}
function handleBootLoadFileSelect(evt) {
        var files = evt.target.files; // FileList object

        ignoreTimeout = true;

        // DMOTODO!!!! VALIDATE FILE HERE BEFORE SENDING IT!!!!
        var data = new FormData();
        $.each(files, function (key, value) {
            data.append(key, value);
        });

        UploadBootLoader(data);
}

function progressBarTest()
{
	 $.ajax({
             cache: false,
            xhr: function () {
				var xhr = new window.XMLHttpRequest();
//These events monitor the upload itself

                xhr.addEventListener("loadstart", function (evt) {
                    $('#progressloader').show();
                }, false);

                xhr.addEventListener("loadend", function (evt) {
                    $('#progressloader').hide();
                }, false);

                xhr.addEventListener("progress", function (evt) {
                    if (evt.lengthComputable) {
                        $('#loading-text-second').text((evt.loaded * 100) / evt.total + "%");
                        $('#loading-text-second').show();
                    }

					//progress events are responses from the PHP while it runs:
					//
                    var response = evt.currentTarget.response;
                    var lines = response.split("|");
                    if (lines.length)
                        lastStatus = lines[lines.length - 1];
                    else
                        lastStatus = "";

                    var statusText = "";
                   // console.log(lastStatus);
                    switch (lastStatus) {
                       


                        default:
                            {
                                statusText = "";

                                break;
                            }
                    }

                    
                    {
                      //progress test: lines are the progress number
                       
                        var progressText = lines[1];
                       {
                           

                            var x = parseInt(lastStatus);
                            // var y = parseInt(xofy[1]);
                            var complete = lerp(0, 100, 0, 100, x);
                            //clamp:  f(x, a, b)...  clamp = (a1 - b) / (a1 - a0)... if clamp > 1 clamp = a1, if clamp < 0 clamp = a0
                            var clamp = complete / 100;
                            if (clamp < 0)
                                complete = 0;
                            if (clamp > 1)
                                complete = 100;
								document.getElementById('testLabel').innerHTML = complete
								
							//document.getElementById('testlabel').text =complete;
                           // setProgress(lerp(0, 100, 9, 10, complete));

                            //statusText = GlobalStrings.FirmwareUpdateStatusUpdatingBob + " " + complete.toString() + "% complete.";
                            // $('#loading-text-second').text(complete + "%");
                        }

                    }

                  
						
						
                }, false);

                xhr.upload.addEventListener("progress", function (evt) {
                    if (evt.lengthComputable) {
                        setProgress((evt.loaded * 10) / evt.total);
                       // $('#loading-text-second').text((evt.loaded * 100) / evt.total + "%");
                        $('#loading-text-second').show();
                    }
                }, false);
				xhr.onreadystatechange = function(){
					document.getElementById('testLabel').innerHTML = this.responseText;
					
				}
                return xhr;
            },
				 type: 'POST',
            url: '/scripts/testprogress.php',
            data: null,
            processData: false, // Don't process the files
            contentType: false, // Set content type to false as jQuery will tell the server its a query 
			progress: function (xhr, status) {
				document.getElementById('testLabel').innerHTML = status;
			},
            complete: function (xhr, status) {
				$('#testLabel').html( "success");
            },
            error: function (xhr, ajaxOptions, thrownError) {
               $('#testLabel').html("error");
            }
        }); 
				
				
}
uploadBootLoaderSystemUpdate = false;
//this is basically the same as the firmware upgrade
function UploadBootLoader(data)
{
	 
	if(uploadBootLoaderSystemUpdate) 
	{
		uploadBootLoaderSystemUpdate = false;
		return;
	}
        var lastStatus = "";
		
		var uploadSize = 0;
        //We always append the current nano and ICM versions.
       
	   //todo: add the correct titles
        ShowWaitingDlg(GlobalStrings.PTFirmwareUpdateNanoTitle, GlobalStrings.PTFirmwareUpdateStatusUpload, false);  

        $.ajax({
             cache: false,
            xhr: function () {
                var xhr = new window.XMLHttpRequest();
//These events monitor the upload itself

                xhr.addEventListener("loadstart", function (evt) {
                    $('#progressloader').show();
                }, false);

                xhr.addEventListener("loadend", function (evt) {
                    $('#progressloader').hide();
                }, false);

                xhr.addEventListener("progress", function (evt) {
                    if (evt.lengthComputable) {
						if(evt.total > uploadSize)
							uploadSize = evt.total; //find out how big the file is so we can track the load progress
                        $('#loading-text-second').text((evt.loaded * 100) / evt.total + "%");
                        $('#loading-text-second').show();
                    }

					//progress events are responses from the PHP while it runs:
					//
                    var response = evt.currentTarget.response;
                    var lines = response.split("|");
                    if (lines.length)
                        lastStatus = lines[lines.length - 1];
                    else
                        lastStatus = "";

                    var statusText = "";
                   // console.log(lastStatus);
                   if(lastStatus.indexOf("waiting") != -1)
				   {
					    $('#loading-text-second').html(GlobalStrings.PTFirmwareUpdateStatusUpdatingBobWaiting);
				   }
					////Total 135210 bytes sent out, packet 1082
                    if (lastStatus.indexOf("Total") != -1)  //bobProgress|<some text from the ICM communicator>
                    {
                       // console.log(response);
                        //response was already split into the object Lines. lines[0] is "bobProgress", lines[1] is ICM communicator output
                        //there's a start indicator then the ICM will mark the register it wrote to from 33 to 133, then 255.  We'll use a simple
                        //scale for 33-133 equaling 0 to 100, ignoring other output.  
                        //lines we want look llike this:
                        //Programming row: 37 

                        //todo: lines are Programming Row: x of y
                        statusText = GlobalStrings.PTFirmwareUpdateStatusUpdatingBob;  
                        var progressText = lastStatus;
                        {
                            //lerp:  f(x) = f0 + (f1 - f0) /(x1-x0) * (x - x0)
                            var parts = progressText.split(" ");
                          

                            var x = parseInt(parts[1]);
                            // var y = parseInt(xofy[1]);
                            var complete = lerp(0, uploadSize, 0, 100, x);
                            //clamp:  f(x, a, b)...  clamp = (b - a0) / (a1 - a0)... if clamp > 1 clamp = a1, if clamp < 0 clamp = a0
                            var clamp = complete / 100;
                            if (clamp < 0)
                                complete = 0;
                            if (clamp > 1)
                                complete = 100;

                            setProgress(lerp(0, 100, 0, 10, complete));

                            //statusText = GlobalStrings.FirmwareUpdateStatusUpdatingBob + " " + complete.toString() + "% complete.";
                            // $('#loading-text-second').text(complete + "%");
                        }

                    }

                    if ((evt.lengthComputable && (evt.loaded < evt.total)) || !evt.lengthComputable)
                        $('#loading-text-second').html(statusText);
						
						
                }, false);

                xhr.upload.addEventListener("progress", function (evt) {
                    if (evt.lengthComputable) {
						if(evt.total > uploadSize)
							uploadSize = evt.total; 
                        setProgress((evt.loaded * 10) / evt.total);
                       // $('#loading-text-second').text((evt.loaded * 100) / evt.total + "%");
                        $('#loading-text-second').show();
                    }
                }, false);
				//just having this seemed to trigger everything
				xhr.onreadystatechange = function(){
					
					
				}
                return xhr;
            },

            type: 'POST',
            url: '/scripts/testprogress.php',
            data: data,
            processData: false, // Don't process the files
            contentType: false, // Set content type to false as jQuery will tell the server its a query 
			
            complete: function (xhr, status) {
                HideWaitingDlg();
				//Programming Reader Start ...
                // Now we want to show our success/failure dialog...
                switch (lastStatus) {
                    case "success":
                        {
                            // Reset all text strings in Software Tab on success...
                          //TODO:  update versions for firmware
						  
						  
						  
                           HideOnlyWaitingDlg();
                             ShowMessageBox(GlobalStrings.FirmwareUpdateTitle, GlobalStrings.PTFirmwareUpdateSuccess, GlobalStrings.PTFirmwareUpdateReload, "MB_ICONSUCCESS", "OK", okRefresh); //TODO:  update messages
							 resetUploadFileName();
                            break;
                        }


                        // On Failure we need to figure out what string to display        
                    default:
                        {
                            var strResult = "";

                            // Map the error code to an error string...
                            switch (lastStatus) {
                                //Unsupported File Type!                               
                               
                                default:
                                    {
                                        strResult = GlobalStrings.PTFirmwareUpdateErrorFailed; //TODO:  update messages
                                        break;
                                    }

                            }
                            HideOnlyWaitingDlg();
                           
                            ShowMessageBox(GlobalStrings.PTFirmwareUpdateFailedTitle, GlobalStrings.PTFirmwareUpdateFailed, "", "MB_ICONERROR", "OK", okRefresh); //TODO:  update messages
							 resetUploadFileName();
							
                        }
                }
            },
            error: function (xhr, ajaxOptions, thrownError) {
                HideOnlyWaitingDlg();
                ShowMessageBox(GlobalStrings.PTFirmwareUpdateFailedTitle, thrownError, "", "MB_ICONERROR", "OK", okRefresh); //TODO:  update messages
				 resetUploadFileName();
            }
        }); 

    }
	function okRefresh()
	{
		 bRefreshAttempt = true;
                    ShowWaitingDlg(GlobalStrings.RefreshingPage, "", true, 30000);
                    sleep(5000); // give the app a chance to restart
                    location.reload(true); // reload with same url...
	}
	function resetUploadFileName()
	{
		uploadBootLoaderSystemUpdate = true;
		$('#bootLoaderUpload').val("");
		
		
	}
