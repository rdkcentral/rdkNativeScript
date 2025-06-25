var player = new AAMPMediaPlayer();
var url = "https://cdn.bitmovin.com/content/assets/art-of-motion-dash-hls-progressive/mpds/f08e80da-bf1d-4e3d-8899-f0f6155f6efa.mpd";
player.setVideoRect(330,150,1600,920);
var t = player.load(url, false);
setTimeout(function() {
      player.play();
}, 0);

/*
const player = new Video();
player.url = "https://cdn.bitmovin.com/content/assets/art-of-motion-dash-hls-progressive/mpds/f08e80da-bf1d-4e3d-8899-f0f6155f6efa.mpd";
player.autoplay = true;
player.muted = true; // Set to false to unmute
player.loop = true;
player.x = 330;
player.y = 150;
player.width = 1600;
player.height = 920;

player.load();
*/