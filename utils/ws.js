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
*
* Includes code from pxCore which is:
* Copyright 2005-2018 John Robinson
* Licensed under the Apache License, Version 2.0
**/

"use strict";


const CONNECTING = 0;
const OPEN = 1;
const CLOSING = 2;
const CLOSED = 3;


/**
 * websocket emit class
 */
function WBEmit() {}
WBEmit.prototype.$emit = function(name) {
    var args = Array.prototype.slice.call(arguments, 1);
    if (this._events && this._events[name])
        this._events[name].forEach(function(cb) {
            cb.apply(this, args)
        }.bind(this));
    return this;
};
WBEmit.prototype.$off = function(name, cb) {
    if (!this._events) return this;
    if (!cb) delete this._events[name];
    if (this._events[name]) this._events[name] = this._events[name].filter(function(i) {
        return i != cb
    });
    return this;
};
WBEmit.prototype.$on = function(name, cb) {
    if (!this._events) this._events = {};
    if (!this._events[name]) this._events[name] = [];
    this._events[name].push(cb);
    return cb;
};

/**
 * Create a new websocket instance
 * @param {*} address  the websocket address
 * @param {*} protocols  the websocket protocols
 * @param {*} options  the websocket options  {timeout: , headers}
 */
function WebSocket(address, protocols) {
   this._instance = null;
    var uri = address;
    this.readyState = CONNECTING;
    this.emit = new WBEmit();
    var updateduri = uri;
    var wsEndIndex = uri.indexOf("ws:");
    if (wsEndIndex == 0)
    {
      if (uri.indexOf("//") == -1)
      {
          updateduri = "ws://";
          var ipport = uri.substring(3);
          updateduri = updateduri + ipport;
      }
    }

    var updatedprotocols = protocols;
    if (protocols instanceof Array)
    {
        updatedprotocols = "";
        var numprotocols = protocols.length;
        for (var i=0; i<numprotocols; i++)
        {
            updatedprotocols = updatedprotocols + protocols[i];
            if (i != numprotocols-1)
            {
                updatedprotocols = updatedprotocols + ",";      
            }
        } 
        console.log("websocket protocols is " + updatedprotocols);
    }

    var options = {};
    options.timeout = options.timeout || 60 * 1000; // 60 seconds
    options.headers = options.headers || {};
    if (updatedprotocols.length > 0)
    {
        options.headers["Sec-WebSocket-Protocol"] = updatedprotocols;
    }
    var params = {
        uri: updateduri,
        headers: options.headers,
        timeoutMs: options.timeout,
    };

    this._instance = webSocket(params);

    // next tick
    //setTimeout(() => {
    //    this._instance.connect();
    //});

    this._instance.on('open', (...args) => {
        this.readyState = OPEN;
        this.emit.$emit('open', ...args)
    })

    this._instance.on('close', (...args) => {
        this.readyState = CLOSED;
        this.emit.$emit('close', ...args)
    })

    this._instance.on('message', (...args) => {
        this.emit.$emit('message', ...args)                          
    })   
    this._instance.on('error', (...args) => {
        this.emit.$emit('error', ...args)                          
    })   
    this._instance.connect();
}

/**
 * when websocket event happened
 * @param name the event name {open,error,message,close}
 * @param fn the function
 */
WebSocket.prototype.on = function(name, fn) {
    this.emit.$on(name, fn);
}

/**
 * close websocket
 */
WebSocket.prototype.close = function() {
    this.readyState = CLOSING;
    this._instance.close();
}

/**
 * send data
 * @param data the string data
 */
WebSocket.prototype.send = function(data) {
    this._instance.send(data);
}

/**
 * remove event listener
 * @param name the event name
 * @param fn the function
 */
WebSocket.prototype.removeListener = function(name, fn) {
    this.emit.$off(name, fn);
}

/**
 * remove all event listeners
 */
WebSocket.prototype.removeAllListeners = function(name) {
    this.emit.$off(name);
}

/**
 * close websocket
 */
WebSocket.prototype.closeimmediate = function() {
    this.close();
}

WebSocket.prototype.addEventListener = function(name, fn) {
    this._instance.on(name, fn);
}

WebSocket.prototype.removeEventListener = function(name, fn) {
    this._instance.delListener(name, fn);
}

Object.defineProperty(WebSocket.prototype, 'onopen', {
    set : function(fn) {
        this.emit.$on('open', fn);
    }
})

Object.defineProperty(WebSocket.prototype, 'onerror', {
    set : function(fn) {
        this.emit.$on('error', fn);
    }
})

Object.defineProperty(WebSocket.prototype, 'onmessage', {
    set : function(fn) {
        this.emit.$on('message', fn);
    }
})
