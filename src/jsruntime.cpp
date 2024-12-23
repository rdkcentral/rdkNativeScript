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

#include <NativeJSRenderer.h>
#if defined(ENABLE_JSRUNTIME_SERVER)
#include <JSRuntimeServer.h>
#endif

#include <thread>
#include <string>
#include <iostream>
#include <memory>
#include <string.h>
#include <thread>
#include <unistd.h>

using namespace std;
using namespace JsRuntime;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "pass the url to run" << std::endl;
	return -1;
    }

    bool runServer = false;

    std::string waylanddisplay("");
    bool enableJSDOM = false, enableWindow = false, enablePlayer = false, enableWebSocketEnhanced = false, enableFetch = false;
    int i = 1, appendindex=argc-1;
    std::vector<std::string> applications;
    ModuleSettings moduleSettings;
    bool consoleMode = false;
    while (i<argc)
    {	    
        if (strcmp(argv[i], "--display") == 0)
        {
            appendindex = i-1;
	    i++;
            waylanddisplay = argv[i];
        }
	else if (strcmp(argv[i], "--enableHttp") == 0)
        {
            moduleSettings.enableHttp = true;
        }
	else if (strcmp(argv[i], "--enableXHR") == 0)
        {
            moduleSettings.enableXHR = true;
        }
	else if (strcmp(argv[i], "--enableWebSocket") == 0)
        {
            moduleSettings.enableWebSocket = true;
        }
	else if (strcmp(argv[i], "--enableWebSocketEnhanced") == 0)
        {
            moduleSettings.enableWebSocketEnhanced = true;
        }
	else if (strcmp(argv[i], "--enableFetch") == 0)
        {
            moduleSettings.enableFetch = true;
        }
	else if (strcmp(argv[i], "--enableJSDOM") == 0)
        {
            moduleSettings.enableJSDOM = true;
        }
	else if (strcmp(argv[i], "--enableWindow") == 0)
        {
            moduleSettings.enableWindow = true;
        }
	else if (strcmp(argv[i], "--enablePlayer") == 0)
        {
            moduleSettings.enablePlayer = true;
        }
    else if (strcmp(argv[i], "--console") == 0)
        {
            consoleMode = true;
        }
#if defined(ENABLE_JSRUNTIME_SERVER)
	else if (strcmp(argv[i], "--server") == 0)
        {
            runServer = true;
        }
#endif
        else
        {
            applications.push_back(argv[i]);
        }
	i++;
    }

    std::shared_ptr<NativeJSRenderer> renderer = std::make_shared<NativeJSRenderer>(waylanddisplay);
    if (consoleMode) {
        renderer->setEnvForConsoleMode(moduleSettings);
    }
    if (!renderer)
    {
        std::cout << "unable to run application" << std::endl;
        return -1;
    }

#if defined(ENABLE_JSRUNTIME_SERVER)
    if (runServer == true)
    {
        JSRuntimeServer *server = JSRuntimeServer::getInstance();
        server->initialize(WS_SERVER_PORT, renderer);
        server->start();
    }
#endif

    for (int j=0; j<applications.size(); j++)
    {
        std::string url = applications[j];
        std::cout << "application url is " << (url.size() ? url.c_str() : "empty") << std::endl;
        renderer->launchApplication(url, moduleSettings);
    } 

    renderer->run();

    return 0;
}
