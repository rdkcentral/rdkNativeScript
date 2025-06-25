var player = new AAMPMediaPlayer();
var url = "https://dash.akamaized.net/dash264/TestCases/2c/qualcomm/1/MultiResMPEG2.mpd";
player.setVideoRect(330,150,1600,920);
var t = player.load(url, false);
setTimeout(function() {
      player.play();
}, 0);

/*
const player = new Video();
player.url = "https://dash.akamaized.net/dash264/TestCases/2c/qualcomm/1/MultiResMPEG2.mpd";
player.autoplay = true;
player.muted = true; // Set to false to unmute
player.loop = true;
player.x = 330;
player.y = 150;
player.width = 1600;
player.height = 920;

player.load();
*/