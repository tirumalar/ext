$(document).ready(function ($) {
    // size = flag size + spacing
    var default_size = {
        w: 20,
        h: 15
    };

    function calcPos(letter, size) {
        return -(letter.toLowerCase().charCodeAt(0) - 97) * size;
    }

    function getFlagPosition(iso, size)
    {
	size || (size = default_size);
       
       var pos = {};
       pos[0] = calcPos(iso[1], size.w);
       pos[1] = calcPos(iso[0], size.h);
       return pos;
    }

    $.fn.setFlagPosition = function (iso, size) {
        var pos = getFlagPosition(iso, size);
        return $(this).css('background-position', pos[0]+'px '+pos[1]+'px');
    };

    // support for IE	
    if(navigator.appVersion.match(/MSIE [\d.]+/)){
	$('.country').find('i').setFlagPosition(currentLanguage);

	$('#countrycode').change(function () {
		pos = getFlagPosition(this.value);
		$('.country').find('i').css('background-position-x', pos[0]+'px\9');
		$('.country').find('i').css('background-position-y', pos[1]+'px\9');
	});
    }
});

// USAGE:
$(document).ready(function ($) {
    $(function () {
        var $target = $('.country');

        // on load:
        $target.find('i').setFlagPosition(currentLanguage);

        $('#countrycode').change(function () {
            $target.find('i').setFlagPosition(this.value);
            //   $target.find('b').text($(this).find(':selected').text());
        });
    });
});