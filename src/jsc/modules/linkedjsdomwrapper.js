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
