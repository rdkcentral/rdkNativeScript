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

#include<TimeUtils.h>
#include "NativeJSRenderer.h"
//#include "utils.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <vector>
#include <thread>
#include <signal.h>
#ifndef __APPLE__
#include <linux/input.h>
#endif
#include <map>
#include <curl/curl.h>
#include <JavaScriptEngine.h>
#include <JavaScriptContext.h>
#ifdef ENABLE_ESSOS
#include <EssosInstance.h>
#endif
#ifndef DISABLE_RTUTILS
#include <rtThreadQueue.h>
#endif

#define NATIVEJS_EMBED_THUNDERJS "/tmp/nativejsEmbedThunder"
#define NATIVEJS_EMBED_WEBBRIDGE "/tmp/nativejsRdkWebBridge"
#define NATIVEJS_ENABLE_WEBSOCKET_SERVER "/tmp/nativejsEnableWebSocketServer"

using namespace JsRuntime;

#ifndef DISABLE_RTUTILS
rtThreadQueue* gUIThreadQueue = NULL;
#endif

namespace JsRuntime {
        std::string DEFAULT_USER_AGENT = "Mozilla/5.0 (X11; Linux armv7l) AppleWebKit/605.1.15 (KHTML, like Gecko) Version/16.0 Safari/605.1.15 ";
}

static size_t HeaderCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t downloadSize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->headerBuffer = (char*)realloc(mem->headerBuffer, mem->headerSize + downloadSize + 1);
  if(mem->headerBuffer == NULL) {
    /* out of memory! */
    NativeJSLogger::log(WARN, "Out of memory when downloading image\n");
    return 0;
  }

  memcpy(&(mem->headerBuffer[mem->headerSize]), contents, downloadSize);
  mem->headerSize += downloadSize;
  mem->headerBuffer[mem->headerSize] = 0;

  return downloadSize;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t downloadSize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->contentsBuffer = (char*)realloc(mem->contentsBuffer, mem->contentsSize + downloadSize + 1);
  if(mem->contentsBuffer == NULL) {
    /* out of memory! */
    NativeJSLogger::log(WARN, "Out of memory when downloading image\n");
    return 0;
  }

  memcpy(&(mem->contentsBuffer[mem->contentsSize]), contents, downloadSize);
  mem->contentsSize += downloadSize;
  mem->contentsBuffer[mem->contentsSize] = 0;

  return downloadSize;
}

NativeJSRenderer::NativeJSRenderer(std::string waylandDisplay): mEngine(nullptr), mRunning(true), mEnableTestFileDOMSupport(false), mEmbedThunderJS(false), mEmbedRdkWebBridge(false), mEnableWebSocketServer(false), mContextMap(), mEssosInitialized(false), mConsoleMode(false), mBaseUserAgent(DEFAULT_USER_AGENT)
{
    if (waylandDisplay.size() > 0)
    {
        setenv("WAYLAND_DISPLAY", waylandDisplay.c_str(), 1);
#ifdef ENABLE_ESSOS
        mEssosInitialized = EssosInstance::instance()->initialize(true);
#endif
    }

    const char* levelFromEnv = getenv("NATIVEJS_LOG_LEVEL");

    // checking for ethan log env 
    #ifdef USE_ETHANLOG
    NativeJSLogger::isEthanLogEnabled();
    NativeJSLogger::log(INFO, "EthanLog is enabled");
    #endif
    if(levelFromEnv)
    {
    	NativeJSLogger::setLogLevel(levelFromEnv);
    	NativeJSLogger::log(INFO, "Log level is set to: %s", levelFromEnv);
    }
    else
    {
    	NativeJSLogger::log(INFO, "Log level isn't set. Using default loglevel: INFO");
    }

    mEngine = new JavaScriptEngine();
    mEngine->initialize();
    char* testfile = getenv("NATIVEJS_TEST_FILE");
    if (nullptr != testfile)
    {
        mTestFileName = testfile;
    }
    char* testfiledomsupport = getenv("NATIVEJS_ENABLE_TEST_FILE_DOMSUPPORT");
    if (nullptr != testfiledomsupport)
    {
        if (strcmp(testfiledomsupport, "1") == 0)
	{
            mEnableTestFileDOMSupport = true;
        }
    }
    char* embedThunderJS = getenv("NATIVEJS_EMBED_THUNDERJS");
    if (embedThunderJS)
    {
        mEmbedThunderJS = true;
        NativeJSLogger::log(INFO, "ThunderJS enabled via environment\n");
    }
    else
    {
	std::ifstream f(NATIVEJS_EMBED_THUNDERJS);
        if (f.good())
        {
            mEmbedThunderJS = true;
            NativeJSLogger::log(INFO, "ThunderJS enabled via file presence\n");
        }
    }

    std::ifstream f(NATIVEJS_EMBED_WEBBRIDGE);
    if (f.good())
    {
        mEmbedRdkWebBridge = true;
        NativeJSLogger::log(INFO, "rdk WebBridge enabled via file presence\n");
    }
    char* enableWebSocketServer = getenv("NATIVEJS_ENABLE_WEBSOCKET_SERVER");
    if (enableWebSocketServer)
    {
        mEnableWebSocketServer = true;
        NativeJSLogger::log(INFO, "WebSocket server enabled via environment\n");
    }
    else
    {
	std::ifstream f(NATIVEJS_ENABLE_WEBSOCKET_SERVER);
        if (f.good())
        {
            mEnableWebSocketServer = true;
            NativeJSLogger::log(INFO, "WebSocket server enabled via file presence\n");
        }
    }
}

NativeJSRenderer::~NativeJSRenderer()
{
    gPendingRequests.clear();
    if (mEngine)
    {
        delete mEngine;
        mEngine = nullptr;
    }
    if (mConsoleState && mConsoleState->consoleContext) {
        delete mConsoleState->consoleContext;
    }
}

void NativeJSRenderer::setEnvForConsoleMode(ModuleSettings& moduleSettings)
{
    mConsoleState = std::make_unique<ConsoleState>();
    mConsoleState->moduleSettings = moduleSettings;

    JavaScriptContextFeatures features(mEmbedThunderJS, mEmbedRdkWebBridge, mEnableWebSocketServer, mConsoleState->moduleSettings);
    JavaScriptContext* context = new JavaScriptContext(features, "", mEngine);
    if (!context)
    {
        NativeJSLogger::log(ERROR, "Context could not be initialized - exiting\n");
	return;
    }
    mConsoleState->consoleContext = context;

    NativeJSLogger::log(INFO, "Running developer console...\n");
    std::thread consoleThread(&JsRuntime::NativeJSRenderer::runDeveloperConsole, this, std::ref(mConsoleState->moduleSettings));
    consoleThread.detach();

    mConsoleMode = true;
}

uint32_t NativeJSRenderer::createApplicationIdentifier()
{
    static uint32_t id = 1;
    uint32_t ret = id;
    id++;
    return ret;
}

uint32_t  NativeJSRenderer::createApplication(ModuleSettings& moduleSettings, std::string userAgent)
{
    uint32_t id=0;
	mUserMutex.lock();
    id = createApplicationIdentifier();
	ApplicationRequest request(id, CREATE, "", moduleSettings.enableHttp, moduleSettings.enableXHR, moduleSettings.enableWebSocket, moduleSettings.enableWebSocketEnhanced, moduleSettings.enableFetch, moduleSettings.enableJSDOM, moduleSettings.enableWindow, moduleSettings.enablePlayer, userAgent);
	gPendingRequests.push_back(request);
	mUserMutex.unlock();
	return id;
}

bool NativeJSRenderer::runApplication(uint32_t id, std::string url)
{
	mUserMutex.lock();
	ApplicationRequest request(id,RUN,url);
	gPendingRequests.push_back(request);
	mUserMutex.unlock();
	return true;
}

bool NativeJSRenderer::runJavaScript(uint32_t id, std::string code)
{
	mUserMutex.lock();
	ApplicationRequest request(id,RUNSCRIPT,code);
	gPendingRequests.push_back(request);
	mUserMutex.unlock();
	return true;
}

std::list<ApplicationDetails> NativeJSRenderer::getApplications()
{
	mUserMutex.lock();
	std::list<ApplicationDetails> runningApplication;
	if(mContextMap.empty())
	{
		NativeJSLogger::log(WARN, "No application found..\n");
	}
	else {
		for(const auto& [key, value] : mContextMap)
		{
			ApplicationDetails appData;
			appData.id = key;
			appData.url = value.url;
			NativeJSLogger::log(DEBUG, "Found application with ID: %d and URL: %s\n", key, value.url.c_str());
			runningApplication.push_back(appData);
		}
	}
	mUserMutex.unlock();
	return runningApplication;

}

bool NativeJSRenderer::terminateApplication(uint32_t id)
{
	mUserMutex.lock();
	ApplicationRequest request(id, TERMINATE);
	gPendingRequests.push_back(request);
	mUserMutex.unlock();
	return true;
}

void NativeJSRenderer::createApplicationInternal(ApplicationRequest& appRequest)
{
        double startTime = getTimeInMilliSec();

	ModuleSettings settings;
	settings.enableHttp = appRequest.mEnableHttp;
	settings.enableXHR = appRequest.mEnableXHR;
	settings.enableWebSocket = appRequest.mEnableWebSocket;
	settings.enableWebSocketEnhanced = appRequest.mEnableWebSocketEnhanced;
	settings.enableFetch = appRequest.mEnableFetch;
	settings.enableJSDOM = appRequest.mEnableJSDOM;
	settings.enableWindow = appRequest.mEnableWindow;
	settings.enablePlayer = appRequest.mEnablePlayer;
	uint32_t id= appRequest.mId;
	std::string userAgent = appRequest.mUserAgent;
	JavaScriptContextFeatures features(mEmbedThunderJS, mEmbedRdkWebBridge, mEnableWebSocketServer, settings);
        JavaScriptContext* context = new JavaScriptContext(features, "" , mEngine);
        if(NULL == context)
        {
        	NativeJSLogger::log(DEBUG, "Context not created for ID: %d\n", id);
        	return ;
        }
	
	std::stringstream uagent;
        uagent << "window.navigator.userAgent = \"" << userAgent << "\";";
        context->runScript(uagent.str().c_str(),true, userAgent, nullptr, true);

	NativeJSLogger::log(INFO, "UserAgent set to : %s", userAgent.c_str());         
	NativeJSLogger::log(DEBUG, "Context created for ID: %d\n", id);
	 if (mExternalApplicationHandler) {
        context->setExternalApplicationHandler(mExternalApplicationHandler);
    }

        double endTime = getTimeInMilliSec();
        context->setCreateApplicationStartTime(startTime);
        context->setCreateApplicationEndTime(endTime, id);

        mContextMap[id].context=context;
        mUserMutex.unlock();
}

void NativeJSRenderer::runApplicationInternal(ApplicationRequest& appRequest)
{
	uint32_t id = appRequest.mId;
	std::string url = appRequest.mUrl;

	if (mContextMap.find(id) == mContextMap.end())
	{
		return;
	}
	mContextMap[id].url = url;

	if(!url.empty())
	{
		NativeJSLogger::log(INFO, "Before launching app\n");
		std::string urlPattern = url.substr(0, 4);
		if (urlPattern.compare(0, 4, "http") == 0)
		{
			MemoryStruct chunk;
			bool ret = downloadFile(url, chunk);
			if (!ret)
			{
			    return ;
			}
			JavaScriptContext* context = (JavaScriptContext*)mContextMap[id].context;
			context->setUrl(mContextMap[id].url);
			if(context->getModuleSettings().enableJSDOM)
			{
				std::stringstream window;
            	    		window<<"window.location = {\"href\":\"" << url << "\"};";
           			NativeJSLogger::log(INFO, "Adding the window location: %s to js file\n", window.str().c_str());
            			context->runScript(window.str().c_str(),true, url, nullptr, true);
			}
			NativeJSLogger::log(INFO, "nativeJS application thunder execution url: %s, result: %d\n", url.c_str(), ret ? 1 : 0);
			ret = context->runScript(chunk.contentsBuffer, true, url, nullptr, true);
			NativeJSLogger::log(INFO, "nativeJS application execution result: %d\n", ret ? 1 : 0);
			double duration = context->getExecutionDuration();
            		context->setAppdata(id, url);
			NativeJSLogger::log(INFO, "Execution duration(runApplicationDuration) for ID %d | URL %s : %.3f ms\n", id, url.c_str(), duration);
		}
		else
		{
			NativeJSLogger::log(INFO, "About to launch local app\n");
			JavaScriptContext* context = (JavaScriptContext*)mContextMap[id].context;
                       context->setUrl(mContextMap[id].url);
			if(context->getModuleSettings().enableJSDOM)
            		{
			    std::stringstream window;
                	    window<<"window.location = {\"href\":\"file:/" << url << "\"};";
                	    NativeJSLogger::log(INFO, "Adding the window location: %s to js file\n", window.str().c_str());
                	    context->runScript(window.str().c_str(),true, url, nullptr, true);
			}
			NativeJSLogger::log(INFO, "Running test application: %s\n", url.c_str());
			bool ret = context->runFile(url.c_str(), nullptr, true);
			NativeJSLogger::log(INFO, "Test application execution result: %d\n", ret ? 1 : 0);
			double duration = context->getExecutionDuration();
			context->setAppdata(id, url);
			NativeJSLogger::log(INFO, "Execution duration(runApplicationDuration) for ID %d | URL %s : %.3f ms\n", id, url.c_str(), duration);
		}
	}
	else{
	    NativeJSLogger::log(WARN, "nativeJS application url not proper\n");
	    return ;
	}
}

void NativeJSRenderer::runJavaScriptInternal(ApplicationRequest& appRequest)
{
	uint32_t id = appRequest.mId;
	std::string code = appRequest.mUrl;

	if (mContextMap.find(id) == mContextMap.end())
	{
		return;
	}

        if (mContextMap[id].url.length() == 0)
	{
	   mContextMap[id].url = "JavaScriptCode";
	}
        bool isApplication = true, isModule = false;
	if(!code.empty())
	{
		NativeJSLogger::log(INFO, "Running the JavaScript code\n");
		JavaScriptContext* context = (JavaScriptContext*)mContextMap[id].context;
		std::string rawcode = code ;
		bool ret = context->runScript(rawcode.c_str(),isModule,mContextMap[id].url,nullptr,isApplication);
		double duration = context->getExecutionDuration();
		NativeJSLogger::log(INFO, "Execution duration(runJavaScriptDuration) for ID %d | %s : %.3f ms\n", id, code.c_str(), duration);
	}
	else{
		NativeJSLogger::log(ERROR, "Empty or Invalid JavaScript Code\n");
	}
}

void NativeJSRenderer::terminateApplicationInternal(ApplicationRequest& AppRequest)
{
	uint32_t id = AppRequest.mId;
	std::map<uint32_t, ApplicationData>::iterator mapEntry = mContextMap.find(id);
	if (mapEntry != mContextMap.end())
	{
		JavaScriptContext* context = (JavaScriptContext*)mContextMap[id].context;
		NativeJSLogger::log(INFO, "Terminating the application with id: %d\n", id);
		if (NULL != context)	{
			NativeJSLogger::log(INFO, "Deleted context\n");
			delete context;
		}
        mContextMap.erase(mapEntry);
		NativeJSLogger::log(INFO, "Application is terminated\n");
	}

	else
	{
		NativeJSLogger::log(ERROR, "Unable to find application with id: %d and url: %s\n", id, mContextMap[id].url);
		return ;
	}


}

size_t NativeJSRenderer::write_data(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    return 0;
}

bool NativeJSRenderer::terminate()
{
    mRunning = false;
    return true;
}


void NativeJSRenderer::run()
{
    while(mRunning)
    {
	uint32_t id;
	mUserMutex.lock();
        if (mConsoleMode) {
            processDevConsoleRequests();
        }
        for (int i=0; i<gPendingRequests.size(); i++)
        {

            ApplicationRequest& request = gPendingRequests[i];
            if(request.mRequestType == CREATE)
            {
		createApplicationInternal(request);
            }
            else if(request.mRequestType == RUN)
            {
		runApplicationInternal(request);

            }
            else if(request.mRequestType == TERMINATE)
            {
		terminateApplicationInternal(request);
            }
            else if(request.mRequestType == RUNSCRIPT)
            {
            	runJavaScriptInternal(request);
            }
            else
            {
            	NativeJSLogger::log(ERROR, "Invalid Request Type\n");
            }

        }
        gPendingRequests.clear();
	mUserMutex.unlock();

        if(!mTestFileName.empty())
        {
            ModuleSettings settings;
	    settings.enableJSDOM = mEnableTestFileDOMSupport;
	    ApplicationRequest appRequest(id, RUN, mTestFileName, settings.enableHttp, settings.enableXHR, settings.enableWebSocket, settings.enableWebSocketEnhanced, settings.enableFetch, settings.enableJSDOM, settings.enableWindow, settings.enablePlayer);
	    NativeJSRenderer::createApplicationInternal(appRequest);
	    NativeJSRenderer::runApplicationInternal(appRequest);
	    mTestFileName = "";
        }

#ifdef ENABLE_ESSOS
        if (mEssosInitialized)
	{
            EssosInstance::instance()->update();
        }
#endif
        mEngine->run();
        double maxSleepTime = (1000 / 40) * 1000;
        usleep(maxSleepTime);
    }
    if (mEngine)
    {
	mEngine->terminate();
    }
}

void NativeJSRenderer::processDevConsoleRequests()
{
    mConsoleState->inputMutex.lock();

    if (mConsoleState->codeToExecute.empty()) {
        mConsoleState->inputMutex.unlock();
        return;
    }

    std::lock_guard<std::mutex> lockg(mConsoleState->isProcessing_cv_m);
    bool dataProcessed = false;

    for (; !mConsoleState->codeToExecute.empty(); mConsoleState->codeToExecute.pop_front()) {
        bool ret = mConsoleState->consoleContext->runScript(mConsoleState->codeToExecute.front().c_str(), false);
        dataProcessed = true;
    }

    if (dataProcessed) {
        mConsoleState->isProcessing = false;
        mConsoleState->isProcessing_cv.notify_one();
    }

    mConsoleState->inputMutex.unlock();
}

namespace {
    bool consoleLoop = true;
    void handleDevConsoleSigInt(int /*sig*/){
        consoleLoop = false;
    }
} // namespace

void NativeJSRenderer::runDeveloperConsole(ModuleSettings moduleSettings)
{
    std::string input;

    NativeJSLogger::log(INFO, "\nJSRuntime Developer Console\n");
    NativeJSLogger::log(INFO, "Type 'exit' or press CTRL+C and ENTER to quit.\n");

    signal(SIGINT, handleDevConsoleSigInt);
    while (consoleLoop) {
        // Don't display another input prompt until previous code is processed (prevents)
        {
            std::unique_lock<std::mutex> lk(mConsoleState->isProcessing_cv_m);
            mConsoleState->isProcessing_cv.wait(lk, [&](){ return mConsoleState->isProcessing == false;});
            std::cout << ">> ";
            mConsoleState->isProcessing = true;
        }

        std::getline(std::cin, input);

        // Short-cirtuit: in case consoleLoop was altered by signal handler we shouldn't execute lines below
        if (!consoleLoop || input == "exit") {
            delete mConsoleState->consoleContext;
            
          #ifdef NATIVEJS_L2_BUILD
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            terminate();
          #endif
            break;
        }

        mConsoleState->inputMutex.lock();
        mConsoleState->codeToExecute.push_back(input);
        mConsoleState->inputMutex.unlock();
    }

    signal(SIGINT, SIG_DFL);
}

bool NativeJSRenderer::downloadFile(std::string& url, MemoryStruct& chunk)
{
    bool ret = false;
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, HeaderCallback);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(curl, CURLOPT_PROXY, "");


        //curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);
        if ((res == 0) && (httpCode == 200))
        {
            NativeJSLogger::log(INFO, "Download operation success\n");
	    ret = true;
        }
        else
        {
            NativeJSLogger::log(ERROR, "Download operation failed\n");
        }
    }
    else
    {
        NativeJSLogger::log(ERROR, "Unable to perform download\n");
    }
    return ret;
}

void NativeJSRenderer::setExternalApplicationHandler(std::shared_ptr<IExternalApplicationHandler> handler)
{
    mExternalApplicationHandler = handler;
}

std::string NativeJSRenderer::getBaseUserAgent()
{
        return mBaseUserAgent;
}

