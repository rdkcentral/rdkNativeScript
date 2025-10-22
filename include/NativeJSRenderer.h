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
#include <deque>
#include <atomic>
#include <memory>
#include <IJavaScriptEngine.h>
#include <IJavaScriptContext.h>
#include <IExternalApplicationHandler.h>
#include <ModuleSettings.h>
#include <condition_variable>
#include <list>

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

        struct ConsoleState {
          std::atomic_bool isProcessing = false;
          std::condition_variable isProcessing_cv{};
          std::mutex isProcessing_cv_m{};
          std::deque<std::string> codeToExecute{};
          std::mutex inputMutex{};
          IJavaScriptContext* consoleContext = nullptr;
          ModuleSettings moduleSettings{};
        };

        struct ApplicationDetails{
        	uint32_t id;
        	std::string url;
        };

        enum RequestType{
          CREATE=0,
          RUN,
          TERMINATE,
          RUNSCRIPT
		    };

        struct ApplicationRequest
        {
          ApplicationRequest(uint32_t id, RequestType requestType, std::string url="", bool enableHttp=false, bool enableXHR=false, bool enableWebSocket=false, bool enableWebSocketEnhanced=false, bool enableFetch=false, bool enableJSDOM=false, bool enableWindow=false, bool enablePlayer=false, std::string userAgent=""): mId(id), mRequestType(requestType), mUrl(url), mEnableHttp(enableHttp), mEnableXHR(enableXHR), mEnableWebSocket(enableWebSocket), mEnableWebSocketEnhanced(enableWebSocketEnhanced), mEnableFetch(enableFetch), mEnableJSDOM(enableJSDOM), mEnableWindow(enableWindow), mEnablePlayer(enablePlayer), mUserAgent(userAgent)
          {
          }
          uint32_t mId;
	  RequestType mRequestType;
          std::string mUrl;
          bool mEnableHttp;
          bool mEnableXHR;
          bool mEnableWebSocket;
          bool mEnableWebSocketEnhanced;
          bool mEnableFetch;
          bool mEnableJSDOM;
          bool mEnableWindow;
          bool mEnablePlayer;
          std::string mUserAgent;

        };
        struct ApplicationData{
          std::string url;
          IJavaScriptContext* context;
        };

        class NativeJSRenderer
        {
            public:
	              ~NativeJSRenderer();
                NativeJSRenderer(std::string waylandDisplay="");
                bool initialize();
                bool terminate();
                void run();
                void setEnvForConsoleMode(ModuleSettings& moduleSettings);
                bool runApplication(uint32_t id, std::string url);
                bool runJavaScript(uint32_t id, std::string code);
                uint32_t createApplication(ModuleSettings& moduleSettings, std::string userAgent) ;
                bool terminateApplication(uint32_t id);
                std::list<ApplicationDetails> getApplications();
                void setExternalApplicationHandler(std::shared_ptr<IExternalApplicationHandler> handler);
                std::string getBaseUserAgent();
            private:
		bool downloadFile(std::string& url, MemoryStruct& chunk);
                void processDevConsoleRequests();
                void runDeveloperConsole(ModuleSettings moduleSettings);
                void createApplicationInternal(ApplicationRequest& appRequest);
                void runApplicationInternal(ApplicationRequest& appRequest);
                void terminateApplicationInternal(ApplicationRequest& appRequest);
                void runJavaScriptInternal(ApplicationRequest& appRequest);
                uint32_t createApplicationIdentifier();
                static size_t write_data(void *ptr, size_t size, size_t nmemb, FILE *stream);
                IJavaScriptEngine* mEngine;
                bool mRunning;
                std::string mTestFileName;
                std::unique_ptr<ConsoleState> mConsoleState;
                bool mEnableTestFileDOMSupport;
                bool mEmbedThunderJS;
                bool mEmbedRdkWebBridge;
                bool mEnableWebSocketServer;
                bool mEssosInitialized;
                bool mConsoleMode;
                std::mutex mUserMutex;
                std::map<uint32_t, ApplicationData> mContextMap;
                std::vector<ApplicationRequest> gPendingRequests;
                std::shared_ptr<IExternalApplicationHandler> mExternalApplicationHandler;
                std::string mBaseUserAgent;
	};
};
