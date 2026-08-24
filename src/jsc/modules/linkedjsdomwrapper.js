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
