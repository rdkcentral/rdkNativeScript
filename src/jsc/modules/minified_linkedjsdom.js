var document = {
    createElement: function() { 
        return {
            style: {},
            setAttribute: function() {},
            getAttribute: function() { return null; },
			hasAttribute: function() { return false; },
            appendChild: function(c) { return c; },
            removeChild: function(c) { return c; },
            addEventListener: function() {},
            getBoundingClientRect: function() { return {top:0,left:0,right:0,bottom:0,width:0,height:0,x:0,y:0}; }
        }; 
    },
	createEvent: function(type) {
        return {
            initEvent: function(t, bubbles, cancelable) { this.type = t; this.bubbles = bubbles; this.cancelable = cancelable; },
            type: '',
            bubbles: false,
            cancelable: false,
            preventDefault: function() {}
        };
    },
    getElementById: function(id) { return id === 'videoDiv' ? this.createElement() : null; },
    querySelector: function(s) { return s === '#videoDiv' ? this.getElementById('videoDiv') : null; },
    body: { appendChild: function(c) { return c; } },
    head: { appendChild: function(c) { return c; } }
};

function Event(t) { this.type = t; }
Event.prototype.preventDefault = function() {};

function DOMParser() {}
DOMParser.prototype.parseFromString = function() { return document; };

function Blob(parts, options) { this.size = 0; this.type = (options && options.type) || ''; }
Blob.prototype.slice = function() { return new Blob([], {}); };

// AbortController for fetch API
function AbortController() {
    this.signal = { aborted: false, addEventListener: function() {} };
}
AbortController.prototype.abort = function() {
    this.signal.aborted = true;
};

// Minimal fetch stub
function fetch(url, options) {
    return new Promise(function(resolve, reject) {
        if (options && options.signal && options.signal.aborted) {
            var error = new Error('Aborted');
            error.name = 'AbortError';
            reject(error);
            return;
        }
        // Stub response
        resolve({
            ok: true,
            status: 200,
            json: function() { return Promise.resolve({}); },
            text: function() { return Promise.resolve(''); },
            blob: function() { return Promise.resolve(new Blob([], {})); }
        });
    });
}

// Wrap timer functions to ensure .apply() works
var _setTimeout = setTimeout;
var _setInterval = setInterval;
var _clearTimeout = clearTimeout;
var _clearInterval = clearInterval;

function wrappedSetTimeout(fn, delay) { 
    var args = Array.prototype.slice.call(arguments, 2);
    return _setTimeout(function() { fn.apply(null, args); }, delay); 
}
wrappedSetTimeout.apply = function(thisArg, argArray) {
    if (!argArray || argArray.length === 0) return _setTimeout();
    if (argArray.length === 1) return _setTimeout(argArray[0]);
    if (argArray.length === 2) return _setTimeout(argArray[0], argArray[1]);
    var args = Array.prototype.slice.call(argArray, 2);
    return _setTimeout(function() { argArray[0].apply(null, args); }, argArray[1]);
};

function wrappedSetInterval(fn, delay) { 
    var args = Array.prototype.slice.call(arguments, 2);
    return _setInterval(function() { fn.apply(null, args); }, delay); 
}
wrappedSetInterval.apply = function(thisArg, argArray) {
    if (!argArray || argArray.length === 0) return _setInterval();
    if (argArray.length === 1) return _setInterval(argArray[0]);
    if (argArray.length === 2) return _setInterval(argArray[0], argArray[1]);
    var args = Array.prototype.slice.call(argArray, 2);
    return _setInterval(function() { argArray[0].apply(null, args); }, argArray[1]);
};

function wrappedClearTimeout(id) { return _clearTimeout(id); }
wrappedClearTimeout.apply = function(thisArg, argArray) {
    return _clearTimeout(argArray ? argArray[0] : undefined);
};

function wrappedClearInterval(id) { return _clearInterval(id); }
wrappedClearInterval.apply = function(thisArg, argArray) {
    return _clearInterval(argArray ? argArray[0] : undefined);
};

var window = {
    document: document,
    location: {href:'',host:'127.0.0.1',hostname:'127.0.0.1'},
    navigator: {userAgent:'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Safari/537.36',platform:'Linux'},
    console: console,
    setTimeout: wrappedSetTimeout,
    setInterval: wrappedSetInterval,
    clearTimeout: wrappedClearTimeout,
    clearInterval: wrappedClearInterval,
    addEventListener: function() {},
    Event: Event,
    DOMParser: DOMParser,
    Blob: Blob,
    AbortController: AbortController,
    fetch: fetch,
    Promise: Promise
};

window.window = window;
window.self = window;
var navigator = window.navigator;
var location = window.location;
window.top = window;
var tv = window.tv = {};
var DOMParser = window.DOMParser;
var Event = window.Event;
var Blob = window.Blob;
var AbortController = window.AbortController;
var fetch = window.fetch;
var Promise = window.Promise;
var top = window;
var setTimeout = wrappedSetTimeout;
var setInterval = wrappedSetInterval;
var clearTimeout = wrappedClearTimeout;
var clearInterval = wrappedClearInterval;
var LinkedJSDOMLib = {parseHTML: function() { return {window:window,document:document,defaultView:window}; }};
