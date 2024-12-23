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
