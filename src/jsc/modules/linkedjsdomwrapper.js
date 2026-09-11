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


if (typeof Error.captureStackTrace !== 'function') {
    Error.captureStackTrace = function(targetObject, constructorOpt) {
        try {
            var stack = new Error().stack;
            if (stack && targetObject) {
                var lines = stack.split('\n');
                targetObject.stack = lines.slice(1).join('\n');
            }
            if (typeof console !== 'undefined' && typeof console.log === 'function') {
                var ctorName = (constructorOpt && constructorOpt.name) ? constructorOpt.name : 'unknown';
                console.log('Error.captureStackTrace called for ' + ctorName + ' (polyfilled no-op, previously would have thrown and hung the fetch promise)');

                if (targetObject && targetObject.message) {
                    console.log('message: ' + targetObject.message);
                }
                if (targetObject && targetObject.type) {
                    console.log('type: ' + targetObject.type);
                }
                console.log('stack:\n' + (targetObject && targetObject.stack ? targetObject.stack : stack));
            }
        } catch (e) {
        }
    };
}

(function ppGuardGetOwnPropertySymbols() {
    if (typeof Object.getOwnPropertySymbols !== 'function') {
        return;
    }
    var originalGetOwnPropertySymbols = Object.getOwnPropertySymbols;
    if (originalGetOwnPropertySymbols.__ppGuarded) {
        return;
    }
    var guarded = function(obj) {
        if (obj === undefined || obj === null) {
            try {
                if (typeof console !== 'undefined' && typeof console.log === 'function') {
                    console.log('Object.getOwnPropertySymbols called with ' + obj + ', returning [] instead of throwing');
                }
            } catch (e) {
            }
            return [];
        }
        return originalGetOwnPropertySymbols(obj);
    };
    guarded.__ppGuarded = true;
    Object.getOwnPropertySymbols = guarded;
})();

LinkedJSDOM = LinkedJSDOMLib;
function JSDOM(html)
{
    return LinkedJSDOM.parseHTML(html);
}
var jsdom = new JSDOM('<html></html>');
//const {document, window} = new JSDOM('<!DOCTYPE html><p>Hello world</p>');
document = jsdom.document;
global.document = document;
window = jsdom.window;
global.window = window;
Event = window.Event;
DOMParser = window.DOMParser;
navigator = window.navigator;
global.navigator = navigator;
tv = window.tv = {}
//fetch = FetchLib;
try
{
    EventLib.install(window);
    ProgressEventLib.install(window);
}
catch(e)
{
    console.log("disabled with event");
}
XMLHttpRequest = window.XMLHttpRequest;
HTMLElement = window.HTMLElement;
window.location = {"href":"", "host":"127.0.0.1", "protocol":"http"}
Image = window.Image;


//below all are undefined
/*
console.log(window.Storage);
console.log(window.localStorage);
console.log(window.location);
console.log(window.parent);
console.log(window.top);
console.log(window.screen);
console.log(window.URL);
console.log(window.URLSearchParams);
*/
document.location = window.location;

function getRandom(arr) {
    getRandomValuesCpp(arr);
}

crypto = {}
crypto.getRandomValues = getRandom;

(function setupUrlChangeDetection() {
    let currentHref = window.location.href;

    const checkUrlChange = () => {
        let newHref = window.location.href;
        if (document && document.location && document.location.href && newHref !== document.location.href) {
            newHref = document.location.href;
        }

        if (newHref !== currentHref) {
        if (typeof setExternalAppHandler === 'function') {
                    setExternalAppHandler(newHref);
            }
                currentHref = newHref;
        }
    };

    setInterval(checkUrlChange, 500);
})();

(function() {

    function run(code, url) {
        try {
            (0, eval)(code + "\n//# sourceURL=" + url);
            return null;
        } catch (e) {
            return e;
        }
    }

    function load(url, ok, fail) {
        var x = new XMLHttpRequest();
        x.open("GET", url, true);

        x.onreadystatechange = function() {
            if (x.readyState !== 4) return;

            if (x.status >= 200 && x.status < 300)
                ok(x.responseText || "");
            else
                fail(new Error("HTTP " + x.status));
        };

        x.onerror = fail;
        x.send();
    }

    var orig = HTMLElement.prototype.appendChild;

    HTMLElement.prototype.appendChild = function(node) {

        var ret = orig.call(this, node);

        if (
            node &&
            node.tagName &&
            node.tagName.toLowerCase() === "script" &&
            node.src
        ) {
            load(
                node.src,
                function(code) {

                    var err = run(code, node.src);

                    if (err)
                        return node.onerror && node.onerror(err);

                    node.readyState = "complete";

                    if (typeof node.onreadystatechange === "function")
                        node.onreadystatechange.call(node);

                    if (typeof node.dispatchEvent === "function" && typeof Event === "function") {
                        node.dispatchEvent(new Event("load"));
                    } else if (
                        typeof node.onload === "function" &&
                        node.onload !== node.onreadystatechange
                    ) {
                        node.onload.call(node);
                    }
                },
                function(err) {
                    if (typeof node.dispatchEvent === "function" && typeof Event === "function") {
                        node.dispatchEvent(new Event("error"));
                    } else if (typeof node.onerror === "function") {
                        node.onerror(err);
                    }
                }
            );
        }

        return ret;
    };
})();

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

clearTimeout = (function(originalClearTimeout) {
    return function(id) {
        if (id === null || typeof id === "undefined") {
            return undefined;
        }
        return originalClearTimeout(id);
    };
})(clearTimeout);

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

clearInterval = (function(originalClearInterval) {
    return function(id) {
        if (id === null || typeof id === "undefined") {
            return undefined;
        }
        return originalClearInterval(id);
    };
})(clearInterval);

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

