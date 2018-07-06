
// Function to calculate values of an audio file for playback

function playtone(freq)
{
    var data = []; // just an array
    var rate = 44100;  // Sample rate (samples per second)
    for(var i = 0; i < 10000; i++)
    {
        var time = i / rate;       
        data[i] = 128 + Math.round(127 * (Math.sin(2 * Math.PI * freq * time)));
    }

    var wave = new RIFFWAVE(data); // create the wave file
    var audio = new Audio(wave.dataURI); // create the HTML5 audio element
    audio.play(); // some noise
}

/*
 var myMedia = document.createElement('audio');
$('#player').append(myMedia);
  myMedia.id = "myMedia";
	playAudio('http://iviewsource.com/exercises/audioslider/audio/ViewSource', 0);

function playAudio(fileName, myVolume) {
  var mediaExt = (myMedia.canPlayType('audio/mp3')) ? '.mp3' 
  	: (myMedia.canPlayType('audio/ogg')) ? '.ogg' 
  	: '';
  if (mediaExt) {
    myMedia.src = fileName + mediaExt;
    myMedia.setAttribute('loop', 'loop');
    setVolume(myVolume);
    myMedia.play();
  }
}

*/