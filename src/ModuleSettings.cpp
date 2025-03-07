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
#include <ModuleSettings.h>

ModuleSettings::ModuleSettings(): enableHttp(false), enableXHR(false), enableWebSocket(false), enableWebSocketEnhanced(false)
				  , enableFetch(false), enableJSDOM(false), enableWindow(false), enablePlayer(false)
{
}

ModuleSettings::ModuleSettings(ModuleSettings& settings): enableHttp(settings.enableHttp),
	                                                  enableXHR(settings.enableXHR), 
	                                                  enableWebSocket(settings.enableWebSocket), 
							  enableWebSocketEnhanced(settings.enableWebSocketEnhanced), 
							  enableFetch(settings.enableFetch),
							  enableJSDOM(settings.enableJSDOM), 
							  enableWindow(settings.enableWindow),
							  enablePlayer(settings.enablePlayer)
{
}

void ModuleSettings::fromString(std::string& options)
{
    if (options.find("http") != std::string::npos)
    {
        enableHttp = true;
    }	    
    if (options.find("xhr") != std::string::npos)
    {
        enableXHR = true;
    }	    
    if (options.find("ws") != std::string::npos)
    {
        enableWebSocket = true;
    }	    
    if (options.find("wsenhanced") != std::string::npos)
    {
        enableWebSocketEnhanced = true;
    }	    
    if (options.find("fetch") != std::string::npos)
    {
        enableFetch = true;
    }	    
    if (options.find("jsdom") != std::string::npos)
    {
        enableJSDOM = true;
    }	    
    if (options.find("window") != std::string::npos)
    {
        enableWindow = true;
    }	    
    if (options.find("player") != std::string::npos)
    {
        enablePlayer = true;
    }	    
}
