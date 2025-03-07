# RDK Native Script #

Application wrapper for JavaScript Runtime

<br>

#### Test examples
Deploy hello world example and simple player example in /opt/www/demo

helloworld.js:
console.log("hello world");

player.js:
var player = new AAMPMediaPlayer();
var url = <"use some video url">;
var t = player.load(url, false);
setTimeout(function() {
	player.play();
}, 0);

playerq.js:
var player = new AAMPMediaPlayer();
var url = <"use some video url">;;
var t = player.load(url);

#### Hello world example:
curl --header "Content-Type: application/json"  --request POST --data '{"jsonrpc":"2.0","id":"3","method": "Controller.1.clone", "params":{"callsign":"org.rdk.jsruntime", "newcallsign":"jsruntime1"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json"  -H "$token" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "Controller.1.activate", "params":{"callsign":"jsruntime1"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json"  -H "$token" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "jsruntime1.1.launchApplication", "params":{"url":"http://127.0.0.1:50050/demo/helloworld.js"}}' http://127.0.0.1:9998/jsonrpc

#### Player example:
curl --header "Content-Type: application/json"  --request POST --data '{"jsonrpc":"2.0","id":"3","method": "Controller.1.clone", "params":{"callsign":"org.rdk.jsruntime", "newcallsign":"jsruntime2"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json"  -H "$token" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "Controller.1.activate", "params":{"callsign":"jsruntime2"}}' http://127.0.0.1:9998/jsonrpc
curl --header "Content-Type: application/json"  -H "$token" --request POST --data '{"jsonrpc":"2.0","id":"3","method": "jsruntime2.1.launchApplication", "params":{"url":"http://127.0.0.1:50050/demo/player.js", "options":"player"}}' http://127.0.0.1:9998/jsonrpc

#### Run as command line:
./JSRuntime http://127.0.0.1:50050/demo/helloworld.js
./JSRuntime --enablePlayer http://127.0.0.1:50050/demo/player.js

