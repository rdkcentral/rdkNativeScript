var document = {
    createElement: function() {
        document.getElementsByTagName = function(tag) {
            tag = String(tag).toLowerCase();

            if (tag === "head") return [document.head];
            if (tag === "body") return [document.body];
            if (tag === "html") return [document.documentElement];

            return []; 
        };

        // ensure tagName exists on head/body/html
        document.head.tagName = "HEAD";
        document.body.tagName = "BODY";

        document.documentElement = {
            tagName: "HTML",
            children: [document.head, document.body],
            childNodes: [document.head, document.body]
        };

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

function HTMLElement() {
    this.style = {};
    this.classList = { add: function() {}, remove: function() {}, toggle: function() {}, contains: function() { return false; } };
    this.children = [];
    this.childNodes = [];
}
HTMLElement.prototype.setAttribute = function() {};
HTMLElement.prototype.getAttribute = function() { return null; };
HTMLElement.prototype.hasAttribute = function() { return false; };
HTMLElement.prototype.removeAttribute = function() {};
HTMLElement.prototype.appendChild = function(c) { this.children.push(c); this.childNodes.push(c); return c; };
HTMLElement.prototype.removeChild = function(c) { return c; };
HTMLElement.prototype.addEventListener = function() {};
HTMLElement.prototype.removeEventListener = function() {};
HTMLElement.prototype.dispatchEvent = function() {};
HTMLElement.prototype.getBoundingClientRect = function() { return {top:0,left:0,right:0,bottom:0,width:0,height:0,x:0,y:0}; };
HTMLElement.prototype.focus = function() {};
HTMLElement.prototype.blur = function() {};
HTMLElement.prototype.click = function() {};

function DOMParser() {}
DOMParser.prototype.parseFromString = function() { return document; };

function Blob(parts, options) { this.size = 0; this.type = (options && options.type) || ''; }
Blob.prototype.slice = function() { return new Blob([], {}); };

var window = {
    document: document,
    location: {href:'', host:'127.0.0.1', hostname:'127.0.0.1'},
    navigator: {userAgent:'Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Safari/537.36', platform:'Linux'},
    console: console,
    addEventListener: function() {},
    Event: Event,
    HTMLElement: HTMLElement,
    DOMParser: DOMParser,
    Blob: Blob,
    Promise: Promise,
    frames: { length: 0 }
};

window.window = window;
window.self = window;
var navigator = window.navigator;
var location = window.location;
window.top = window;
var tv = window.tv = {};
var HTMLElement = window.HTMLElement;
var DOMParser = window.DOMParser;
var Event = window.Event;
var Blob = window.Blob;
var Promise = window.Promise;
var top = window;

// Add .apply(), .call(), .bind() support to native timer functions for JSR compatibility
setTimeout.apply = function(thisArg, args) {
    if (!args || args.length === 0) return setTimeout(function() {}, 0);
    var callback = args[0];
    var delay = args[1];
    var callbackArgs = args.slice(2);
    return setTimeout(function() {
        callback.apply(null, callbackArgs);
    }, delay);
};
setTimeout.call = function(thisArg, callback, delay) {
    var args = Array.prototype.slice.call(arguments, 3);
    return setTimeout(function() {
        callback.apply(null, args);
    }, delay);
};
setTimeout.bind = function(thisArg) {
    var boundArgs = Array.prototype.slice.call(arguments, 1);
    return function() {
        var args = boundArgs.concat(Array.prototype.slice.call(arguments));
        return setTimeout.apply(null, args);
    };
};

clearTimeout.apply = function(thisArg, args) {
    return clearTimeout(args && args[0]);
};
clearTimeout.call = function(thisArg, id) {
    return clearTimeout(id);
};
clearTimeout.bind = function(thisArg) {
    return function(id) { return clearTimeout(id); };
};

setInterval.apply = function(thisArg, args) {
    if (!args || args.length === 0) return setInterval(function() {}, 0);
    var callback = args[0];
    var delay = args[1];
    var callbackArgs = args.slice(2);
    return setInterval(function() {
        callback.apply(null, callbackArgs);
    }, delay);
};
setInterval.call = function(thisArg, callback, delay) {
    var args = Array.prototype.slice.call(arguments, 3);
    return setInterval(function() {
        callback.apply(null, args);
    }, delay);
};
setInterval.bind = function(thisArg) {
    var boundArgs = Array.prototype.slice.call(arguments, 1);
    return function() {
        var args = boundArgs.concat(Array.prototype.slice.call(arguments));
        return setInterval.apply(null, args);
    };
};

clearInterval.apply = function(thisArg, args) {
    return clearInterval(args && args[0]);
};
clearInterval.call = function(thisArg, id) {
    return clearInterval(id);
};
clearInterval.bind = function(thisArg) {
    return function(id) { return clearInterval(id); };
};

window.setInterval = setInterval;
window.clearTimeout = clearTimeout;
window.setTimeout = setTimeout;
window.clearInterval = clearInterval;

XMLHttpRequest = window.XMLHttpRequest;

if (!window.screen) {
    window.screen = {
        width: 1920,
        height: 1080
    };
}
var screen = window.screen;

var LinkedJSDOMLib = {
    parseHTML: function() {
        return {window:window, document:document, defaultView:window};
    }
};

var BlobPolyfill = function(parts, options) {
    parts = parts || [];
    options = options || {};
    this.size = 0;
    this.type = options.type || '';
    this._parts = parts;

    for (var i = 0; i < parts.length; i++) {
        if (typeof parts[i] === 'string') {
            this.size += parts[i].length;
        } else if (parts[i] && parts[i].byteLength) {
            this.size += parts[i].byteLength;
        }
    }
};

BlobPolyfill.prototype.slice = function(start, end, contentType) {
    return new BlobPolyfill(this._parts, { type: contentType || this.type });
};

BlobPolyfill.prototype.text = function() {
    var text = '';
    for (var i = 0; i < this._parts.length; i++) {
        if (typeof this._parts[i] === 'string') {
            text += this._parts[i];
        }
    }
    return Promise.resolve(text);
};

BlobPolyfill.prototype.arrayBuffer = function() {
    return Promise.resolve(new ArrayBuffer(0));
};

if (typeof global.Blob === 'undefined') {
    global.Blob = BlobPolyfill;
}
if (typeof window !== 'undefined' && typeof window.Blob === 'undefined') {
    window.Blob = BlobPolyfill;
}
if (typeof self !== 'undefined' && typeof self.Blob === 'undefined') {
    self.Blob = BlobPolyfill;
}
if (typeof this !== 'undefined' && typeof this.Blob === 'undefined') {
    this.Blob = BlobPolyfill;
}

if (typeof window !== 'undefined') {
    if (!window.top) window.top = window;
    if (!window.parent) window.parent = window;

    if (!window.__tcfapi) {
        window.__tcfapi = function(cmd, ver, callback) {
            if (callback) callback({gdprApplies: false}, true);
        };
    }
    if (!window.__uspapi) {
        window.__uspapi = function(cmd, ver, callback) {
            if (callback) callback({uspString: '1---'}, true);
        };
    }
}
