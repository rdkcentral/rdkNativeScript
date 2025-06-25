var player = new AAMPMediaPlayer();
var url = "https://cpetestutility.stb.r53.xcal.tv/VideoTestStream/main.mpd";
player.setVideoRect(330,150,1600,920);
var t = player.load(url, false);
setTimeout(function() {
      player.play();
}, 0);

/*
const player = new Video();
player.url = "https://cpetestutility.stb.r53.xcal.tv/VideoTestStream/main.mpd";
player.autoplay = true;
player.muted = true; // Set to false to unmute
player.loop = true;
player.x = 330;
player.y = 150;
player.width = 1600;
player.height = 920;

player.load();
*/