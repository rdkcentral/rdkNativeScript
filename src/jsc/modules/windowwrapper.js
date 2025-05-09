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

try
{
    if (undefined != jsdom)
    {
        window = jsdom.window;
        window.location = {"href":"", "host":"127.0.0.1", "protocol":"http"}
	/*
        window.frames = []
        window.screen = {
        "width":1920,
        "height":1080,
        "availWidth":1920,
        "availHeight":1080
        }
        screen = window.screen;
        */
    }
}
catch(err)
{
    try
    {	
        if (WindowLib)
        {
            window = new WindowLib({});
            document = window.document;
        }
    }	    
    catch(err)
    {
    }
}
try
{	
    if (window)
    {	    
        Event = window.Event;
        DOMParser = window.DOMParser;
        navigator = window.navigator;
        window.tv = tv
    }
}
catch(err)
{
}
//console.log(window.Event);
//console.log(window.DOMParser);
//console.log(window.navigator);
//console.log(window.setTimeout);
//console.log(window.clearTimeout);
//console.log(window.setInterval);
//console.log(window.clearInterval);
//console.log(window.navigator.userAgent);
//console.log(window.Document);
//console.log(window.document.createElement);
//console.log(window.document.getElementsByTagName);
//console.log(window.document.getElementById);
//console.log(window.DOMParser);
//const parser = new DOMParser();
//console.log(parser.parseFromString);

//below are varying between implementations
//console.log(window.localStorage); //not working on both paths
//console.log(window.location);
//console.log(window.parent);
//console.log(window.top);
//console.log(window.screen.width);
//console.log(window.screen.height);
//console.log(window.screen.availWidth);
//console.log(window.screen.availHeight);
//console.log(window.navigator.appCodeName);
//console.log(window.navigator.appName);
//console.log(window.navigator.appVersion);
//console.log(window.navigator.cookieEnabled);
//console.log(window.navigator.gelocation);
//console.log(window.navigator.language);
//console.log(window.navigator.onLine);
//console.log(window.navigator.platform);
//console.log(window.navigator.product);

//console.log(window.origin);
//console.log(window.Navigator);
//console.log(window.Storage);
