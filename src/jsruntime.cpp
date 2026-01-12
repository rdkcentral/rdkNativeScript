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
#include <NativeJSLogger.h>
#if defined(ENABLE_JSRUNTIME_SERVER)
#include <JSRuntimeServer.h>
#endif

#include <thread>
#include <string>
#include <iostream>
#include <memory>
#include <string.h>
#include <vector>
#include <unistd.h>

using namespace std;
using namespace JsRuntime;

#ifndef UNIT_TEST_BUILD
int main(int argc, char* argv[])
{
    try {
    if (argc < 2)
    {
        NativeJSLogger::log(WARN, "Pass the URL to run\n");
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

    //CID:430751:Use of untrusted string value (TAINTED_STRING)
    // Validate waylanddisplay to prevent injection attacks
    if (!waylanddisplay.empty()) {
        if (waylanddisplay.length() > 256) {
            NativeJSLogger::log(ERROR, "Invalid Wayland display name: too long\n");
            waylanddisplay = "";
        } else {
            bool valid = true;
            for (char c : waylanddisplay) {
                if (!isalnum(c) && c != '-' && c != '_' && c != '/' && c != '.') {
                    valid = false;
                    break;
                }
            }
            if (!valid) {
                NativeJSLogger::log(ERROR, "Invalid Wayland display name: contains invalid characters\n");
                waylanddisplay = "";
            }
        }
    }


    std::shared_ptr<NativeJSRenderer> renderer = std::make_shared<NativeJSRenderer>(waylanddisplay);
    if (consoleMode) {
        renderer->setEnvForConsoleMode(moduleSettings);
    }

#if defined(ENABLE_JSRUNTIME_SERVER)
    if (runServer == true)
    {
        JSRuntimeServer *server = JSRuntimeServer::getInstance();
        server->initialize(WS_SERVER_PORT, renderer, nullptr);
        server->start();
    }
#endif

    std::vector<std::thread> applicationThreads;

    for (int j = 0; j < applications.size(); j++) {
        std::string url = applications[j];
        
        applicationThreads.emplace_back([renderer, url, &moduleSettings]() {
        NativeJSLogger::log(INFO, "Application URL is %s\n", (url.size() ? url.c_str() : "empty"));
	uint32_t id = renderer->createApplication(moduleSettings);
        renderer->runApplication(id, url);
        //renderer->runJavaScript(id,url);
#if defined(NATIVEJS_DEVELOPER_MODE)
        std::this_thread::sleep_for(std::chrono::milliseconds(500)); 
        renderer->getApplications();
        
    #ifdef NATIVEJS_L2_BUILD
        int waitTime = 10;
        if (moduleSettings.enableWebSocket || moduleSettings.enableWebSocketEnhanced) {
            waitTime = 30;
            NativeJSLogger::log(INFO, "WebSocket enabled - using extended wait time: %d seconds\n", waitTime);
        }
        sleep(waitTime);
    #else
        sleep(10);
    #endif

    renderer->terminateApplication(id);

    #ifdef NATIVEJS_L2_BUILD
        std::this_thread::sleep_for(std::chrono::milliseconds(2000));
        renderer->terminate();
    #endif

#endif
        });
    }

    renderer->run(); 

    for (auto& t : applicationThreads) {
        if (t.joinable()) {
        t.join();
        }
    }
    
    return 0;
    } catch (const std::exception& e) {
        NativeJSLogger::log(ERROR, "Uncaught exception in main: %s\n", e.what());
        return -1;
    } catch (...) {
        NativeJSLogger::log(ERROR, "Unknown exception caught in main\n");
        return -1;
    }
}

#endif
