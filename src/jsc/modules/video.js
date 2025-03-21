/**
* If not stated otherwise in this file or this component's LICENSE
* file the following copyright and licenses apply:
*
* Copyright 2024 RDK Management
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
**/

/* Video is a wrapper class for AAMPMediaPlayer component */
class Video {
    /* constructor to initialize the objects created by class Video */
    
    constructor() {
    
        /* Declaring the property player and assign it with AAMMediaPlayer instance */
        this.player = new AAMPMediaPlayer(); 
        
        /* Method called to add event listeners */
        this.addEventListeners(); 
        
        /* Initialize the player properties */
        this._loop = false;
        this._muted = false;
        this._autoplay = false;
        this._url = "";
        
}
    get url() {
        return this._url;
    }

    set url(value) {
         this._url = value;
     }

    get autoplay() {
        return this._autoplay;
    }

    set autoplay(value) {
        this._autoplay = value;
    }
    get muted() {
        return this._muted;
    }

    set muted(value) {
        this._muted = value;
    }
    get loop() {
        return this._loop;
    }

    set loop(value) {
        this._loop = value;
    }

/* Application can receive events from player for various state machine */
    addEventListeners() {
        /* Fired when playback starts */
        this.player.addEventListener("playbackStarted", this.handlePlay.bind(this));
         
        /* Event when player state changes. Contains codes for each states */
        this.player.addEventListener("playbackStateChanged", this.handleStateChanged.bind(this)); 
        
        /* Fired when an error occurs */
        this.player.addEventListener("playbackFailed", this.handleFailed.bind(this)); 
        
        /* Fired when video profile is switched by ABR with the metadata associated with newly selected profile */
        this.player.addEventListener("bitrateChanged", this.handleBitrateChanged.bind(this)); 
        
        /* Fired with metadata of the asset currently played */
        this.player.addEventListener("mediaMetadata", this.handleMediaMetadata.bind(this)); 
    }
/* Method mutes the volume when playback starts if the player is muted */
    handlePlay() {
        console.log('Playback started...!!!');
        this.getVolume();
        if (this.muted) {
            this.setVolume(0);
        }
    }

/* Method will carry looping when EOS is reached */
    handleStateChanged(event) {
        console.log("Playback state changed:", event.state);
        if ((event.state === 10 || event.state === 11) && this.loop) { // Checking for EOS / media playback is stopped
            console.log("End of stream reached, looping...");
            this.seek(0, true); // Seeks to beginning of media and pause
            this.play();
        }
    }

    handleFailed(event) {
        console.error("Playback failed:", event.code, event.description);
    }

    handleBitrateChanged(event) {
        console.log("Bitrate changed:", event.bitRate, "Resolution:", event.width, "x", event.height);
    }

    handleMediaMetadata(event) {
        console.log("Media metadata:", event);
    }
    
/* Method loads the media from given url, sets the autoplay property and sets volume to 0 if player is muted */
    async load() {
        console.log(`Loading media from URL: ${this.url}`);
        console.log(`Autoplay is set to: ${this.autoplay}`);
        
        await this.player.load(this.url, this.autoplay);
        if (this.autoplay) {
            console.log('Media loaded and autoplaying:', this.url);
        } else {
            console.log("Autoplay is set to OFF");
        }
        if (this.muted) {
            this.setVolume(0);
            console.log("Player is muted");
        } else {
            console.log("Player is not muted");
        }
    }
    
/* Sets the current volume to value between 0 and 100. Updated value is reflected in subsequent calls of getVolume() */
    setVolume(volume) {
        this.player.setVolume(volume);
        console.log(`Volume set to: ${volume}`);
    }

    getVolume() {
        console.log("Current volume level:", this.player.getVolume());
    }

/* Specify new playback position to start playback. Can be called prior to load() or during playback */  
    seek(offset, keepPause = false) {
        this.player.seek(offset, keepPause);
        console.log(`Seeking to offset: ${offset}, keepPause: ${keepPause}`);
    }
    
 /* Start playback (if stream is in prebuffered state), or resume playback at normal speed. Equivalent to setPlaybackRate(1) */
    play() {
        this.player.play();
        console.log("Playback started/resumed");
    }

/* Pauses playback. Equivalent to setPlaybackRate(0) */
    pause() {
        this.player.pause();
        console.log("Playback paused");
    }

/* Stop playback immediately and free resources associated with playback */
    stop() {
        this.player.stop();
        console.log("Playback stopped and resources freed");
    }
}
