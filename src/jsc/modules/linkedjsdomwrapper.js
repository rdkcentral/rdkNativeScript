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

	(function (global) {
		'use strict';

		var nativeSetTimeout = setTimeout;
		var nativeClearTimeout = clearTimeout;
		var nativeSetInterval = setInterval;
		var nativeClearInterval = clearInterval

		function invokeCallback(callback, args) {

			if (typeof callback !== "function") {
				return;
			}

			switch (args.length) {
				case 0:
					if (callback.call) {
						callback.call(global);
					} else {
						callback();
					}
					break;

				case 1:
					if (callback.call) {
						callback.call(global, args[0]);
					} else {
						callback(args[0]);
					}
					break;

				case 2:
					if (callback.call) {
						callback.call(global, args[0], args[1]);
					} else {
						callback(args[0], args[1]);
					}
					break;

				case 3:
					if (callback.call) {
						callback.call(global, args[0], args[1], args[2]);
					} else {
						callback(args[0], args[1], args[2]);
					}
					break;

				default:
					if (callback.apply) {
						callback.apply(global, args);
					} else {
						callback();
					}
			}
		}

		function createTimerWrapper(nativeFn) {

			var wrapper = function (callback, delay) {

				var args =
					Array.prototype.slice.call(arguments, 2);

				return nativeFn(function () {

					try {

						invokeCallback(callback, args);

					} catch (e) {

						console.error(
							"Timer callback error:",
							e
						);

						if (e && e.stack) {
							console.error(e.stack);
						}
					}

				}, delay);
			};

			wrapper.apply = function (thisArg, args) {

				args = args || [];

				switch (args.length) {
					case 0: return wrapper();
					case 1: return wrapper(args[0]);
					case 2: return wrapper(args[0], args[1]);
					case 3: return wrapper(args[0], args[1], args[2]);
					case 4: return wrapper(args[0], args[1], args[2], args[3]);
					case 5: return wrapper(args[0], args[1], args[2], args[3], args[4]);
					default:
						throw new Error(
							"setTimeout/setInterval.apply: too many arguments"
						);
				}
			};

			wrapper.call = function (thisArg) {

				var args =
					Array.prototype.slice.call(arguments, 1);

				return wrapper.apply(null, args);
			};

			wrapper.bind = function (thisArg) {

				var boundArgs =
					Array.prototype.slice.call(arguments, 1);

				return function () {

					return wrapper.apply(
						null,
						boundArgs.concat(
							Array.prototype.slice.call(arguments)
						)
					);
				};
			};

			return wrapper;
		}

		function createClearWrapper(nativeFn) {

			var wrapper = function (id) {
				return nativeFn(id);
			};

			wrapper.apply = function (thisArg, args) {
				return nativeFn(args && args[0]);
			};

			wrapper.call = function (thisArg, id) {
				return nativeFn(id);
			};

			wrapper.bind = function () {
				return function (id) {
					return nativeFn(id);
				};
			};

			return wrapper;
		}

		global.setTimeout =
			createTimerWrapper(nativeSetTimeout);

		global.setInterval =
			createTimerWrapper(nativeSetInterval);

		global.clearTimeout =
			createClearWrapper(nativeClearTimeout);

		global.clearInterval =
			createClearWrapper(nativeClearInterval);

	})(window);
