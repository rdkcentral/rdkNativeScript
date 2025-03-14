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

global = {};
self = {}
jsruntime = {}
//fetch = undefined;
//try
//{
//  fetch = FetchLib;
//}
//catch(e)
//{
//  console.log("failed to load fetch !!");
//}
tv = {}

process = require('process')
Buffer = require('buffer').Buffer;
var encdec = require('TextEncoder');
global.TextEncoder = encdec.TextEncoder;
global.TextDecoder = encdec.TextDecoder;
URL = require('URL');
URLSearchParams = require('URLSearchParams');

function ThunderUtility()
{
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

class Performance
{
  constructor()
  {
      this.entries = {}
  }	  
  mark(name, options) 
  {
      var entries = this.entries;	  
      var hasName = (name in entries);
      if (false == hasName)
      {
          entries[name] = {}
      }
      if (options != undefined && options['startTime'] != undefined)
      {
          entries[name]['startTime'] = options['startTime'];
      }
      else
      {     
          //entries[name]['startTime'] = (new Date()).getMilliseconds();
          entries[name]['startTime'] = Date.now();
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
  now()
  {
    return Date.now();	  
  }	  
}
performance = new Performance();
