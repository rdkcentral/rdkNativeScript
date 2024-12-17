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

function ThunderUtility() {
   this.token = function() {
       if (undefined != thunderToken)
       {	   
           return thunderToken();
       }
       else
       {
           return "";
       }	       
   }
}
self = {}
window = {};
window.localStorage = {}
window.parent = self;
self.location = {"href":"mainapp"}
window.setTimeout = setTimeout;
window.clearTimeout = clearTimeout;
global = {};
window.location = {"href":"", "host":"192.168.0.102", "protocol":"http"}
window.thunder = new ThunderUtility();
top = window.top = self;
jsruntime = {}
var self=window;
globalThis=global
globalThis.performance = undefined
Reflect = global.Reflect = {}
class Div
{
  constructor(name)
  {
      this.childrens = Array();
      this.id = name;
      this.classList = []
      this.attrs = {}
      this.style = {}
  }	    
  appendChild(element)
  {
      this.childrens.push(element);
  }	    
  addEventListener(name, fn)
  {

  }	   
  setAttribute(name, value)
  {
      if (name == "id"){
          this.id = value;
      }	       
      this.attrs[name] = value;	  
  }	    
  hasAttribute(name)
  {
      for (var key in this.attrs)
      {	  
      if (key == name){
          return true;
      }
      }	      
      return false;	  
  }	    
  getElementsByTagName(name)
  {
      var els = []
      for (var key in this.childrens)
      {	  
          if (key.id == name)
	  {
              els.push(key);
          }		  
      }
      return els;	  
  }
}

class Document
{
   constructor() 
   {
       this.body = new Div();
       this.elements = Array();
   }

   createElement(id)
   {
      console.log(id);
      if (id == "div")
      {	   
          var div = new Div(id);
          this.elements.push(div);
          return div;
      }
      if (id == "video")
      {	   
          var div = new Div(id);
          this.elements.push(div);
          return div;
      }
      if (id == "script")
      {
          var div = new Div(id);
          this.elements.push(div);
          return div;
      }	      
      if (id == "span")
      {	   
          var div = new Div(id);
          this.elements.push(div);
          return div;
      }
   }
   getElementsByTagName(name)
   {
       var el = undefined;	   
       for (var i=0; i<this.elements.length; i++)
       {
           if (this.elements[i].id == name)
	   {
               el = this.elements[i];
	       break;
           }		   
       }	     
       return el;	   
   }	   
   getElementById(id)
   {
       var el = undefined;	   
       for (var i=0; i<this.elements.length; i++)
       {
           if (this.elements[i].id == id)
	   {
               el = this.elements[i];
	       break;
           }		   
       }	     
       return el;	   
   }
};
document = new Document();
class Performance
{
  constructor()
  {
      this.entries = {}
  }	  
  mark(name, options) 
  {
      console.log("KRISHNA MARKING " + name);
      var entries = this.entries;	  
      var hasName = (name in entries);
      if (false == hasName)
      {
          entries[name] = {}
      }
      if (options != undefined && options['startTime'] != undefined)
      {
          entries[name]['startTime'] = options['startTime'];
          console.log("KRISHNA MARKING1 " + name);
      }
      else
      {     
          //entries[name]['startTime'] = (new Date()).getMilliseconds();
          entries[name]['startTime'] = Date.now();
      console.log("KRISHNA MARKING2 " + name);
      }
      if (options != undefined && option['detail'] != undefined)
      {
          entries[name]['detail'] = options['detail'];
      }
      return entries[name];
  }

  clearMarks()
  {
    for (var key in entries)
    {
        entries[key] = null;
	delete entries[key];    
    }
    entries = {}
  }	  

  getEntriesByName(name)
  { 
      return []
  }
  getEntriesByType(type)
  { 
      return []
  }
  measure(name, startMark, endMark)
  {
      console.log("KRISHNA " + name);
      var hasStart = (startMark in this.entries);
      var hasEnd = (endMark in this.entries);
      var ret = {};
      if (hasStart && hasEnd)
      {
          ret["name"] = name;
	  ret["duration"] = this.entries[endMark].startTime - this.entries[startMark].startTime;
	  ret["startTime"] = this.entries[startMark].startTime;
      }
      return ret;
  }
}
performance = new Performance();
global.AAMPMediaPlayer = AAMPMediaPlayer;

//Error shown but execution not stopped
class DOMParser
{
  parseFromString()
  {
      return document;
  }	  
}
class Event
{
   constructor() 
   {
   }
}

class Navigator
{
   constructor()
   {
       this.appCodeName = "";
       this.appName = "jsruntime";
       this.appVersion = "1";
       this.cookieEnabled = false;
       this.geolocation = undefined;
       this.language = "eng";
       this.onLine = true;
       this.platform = "linux";
       this.product = undefined;
       this.userAgent = "jsruntime";
   }	   
   javaEnabled()
   {
       return false;	   
   }
}
navigator = window.navigator = new Navigator();
tv = window.tv = {}
window.frames = []
window.screen = {
"width":1920,
"height":1080,
"availWidth":1920,
"availHeight":1080
}
screen = window.screen;
