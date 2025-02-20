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
Event = window.Event;
DOMParser = window.DOMParser;
navigator = window.navigator;
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
