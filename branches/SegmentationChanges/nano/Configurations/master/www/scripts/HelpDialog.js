function ShowHelpDlg() {
    ProcessHelpDialogLayout();
    $("#loading-div-background").show();
    $("#help-div-container").show();
}


function HideHelpDlg()
{
    $("#loading-div-background").hide();
    $("#help-div-container").hide();
}


$(document).ready(function () {
    $('#helpsettingsOKbutton').click(function () {
        HideHelpDlg();
        ConfigureToolTipster();
    });
});





 $(document).ready(function () {
     $('#helptrigger').change(function () {
         ProcessHelpDialogLayout();
      });
});


function ProcessHelpDialogLayout() {
         var cSelect = document.getElementById("helptrigger");

        if (cSelect.options[cSelect.selectedIndex].value == "hover")
             $('#helpdelayline').show();
        else
            $('#helpdelayline').hide();
}

