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

#pragma once
#include <string>
#include <vector>
#include <map>
#include <mutex>
#include <IJavaScriptEngine.h>
#include <IJavaScriptContext.h>
namespace JsRuntime {

        struct MemoryStruct
        {
            MemoryStruct()
                : headerSize(0)
                , headerBuffer(NULL)
                , contentsSize(0)
                , contentsBuffer(NULL)
                , readSize(0)
            {
                headerBuffer = (char*)malloc(1);
	        contentsBuffer = (char*)malloc(1);
            }

            ~MemoryStruct()
            {
              if (headerBuffer != NULL)
              {
                free(headerBuffer);
                headerBuffer = NULL;
              }
              if (contentsBuffer != NULL)
              {
                free(contentsBuffer);
                contentsBuffer = NULL;
              }
            }

            size_t headerSize;
            char* headerBuffer;
            size_t contentsSize;
            char* contentsBuffer;
            size_t readSize;
        };

        class NativeJSRenderer
	{
            public:
	        ~NativeJSRenderer();
                NativeJSRenderer(std::string waylandDisplay="");
                bool initialize();
                bool terminate();
                void run();
                void launchApplication(std::string url);
                void terminateApplication(std::string url);
		std::vector<std::string> getApplications();

            private:
                void loadApplication(std::string url);
                void unloadApplication(std::string url);
                bool downloadFile(std::string& url, MemoryStruct& chunk);
                static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
                IJavaScriptEngine* mEngine;
		std::map<std::string, IJavaScriptContext*> mContextMap;
                bool mRunning;
                std::string mTestFileName;
                bool mEmbedThunderJS;
                bool mEmbedRdkWebBridge;
                bool mEnableWebSocketServer;
                bool mEssosInitialized;
                std::mutex mUserMutex;
	};
}
