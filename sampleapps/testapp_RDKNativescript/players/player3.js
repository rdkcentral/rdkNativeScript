var player = new AAMPMediaPlayer();
var url = "https://demo.unified-streaming.com/k8s/features/stable/video/tears-of-steel/tears-of-steel.ism/.m3u8";
player.setVideoRect(330,150,1600,920);
var t = player.load(url, false);
setTimeout(function() {
      player.play();
}, 0);

/*
const player = new Video();
player.url = "https://demo.unified-streaming.com/k8s/features/stable/video/tears-of-steel/tears-of-steel.ism/.m3u8";
player.autoplay = true;
player.muted = true; // Set to false to unmute
player.loop = true;
player.x = 330;
player.y = 150;
player.width = 1600;
player.height = 920;

player.load();
*/