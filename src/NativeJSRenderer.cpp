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
//#include <IJavaScriptContext.h>
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

struct ApplicationRequest
{
    ApplicationRequest(std::string url, bool enableHttp=false, bool enableXHR=false, bool enableWebSocket=false, bool enableWebSocketEnhanced=false, bool enableFetch=false, bool enableJSDOM=false, bool enableWindow=false, bool enablePlayer=false): mUrl(url), mEnableHttp(enableHttp), mEnableXHR(enableXHR), mEnableWebSocket(enableWebSocket), mEnableWebSocketEnhanced(enableWebSocketEnhanced), mEnableFetch(enableFetch), mEnableJSDOM(enableJSDOM), mEnableWindow(enableWindow), mEnablePlayer(enablePlayer)
    {
    }
    std::string mUrl;
    bool mEnableHttp;
    bool mEnableXHR;
    bool mEnableWebSocket;
    bool mEnableWebSocketEnhanced;
    bool mEnableFetch;
    bool mEnableJSDOM;
    bool mEnableWindow;
    bool mEnablePlayer;
    //ModuleSettings mModuleSettings;
};

std::vector<ApplicationRequest> gPendingUrlRequests;
std::vector<std::string> gPendingUrlTerminateRequests;
static size_t HeaderCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t downloadSize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->headerBuffer = (char*)realloc(mem->headerBuffer, mem->headerSize + downloadSize + 1);
  if(mem->headerBuffer == NULL) {
    /* out of memory! */
    std::cout << "out of memory when downloading image\n";
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
    std::cout << "out of memory when downloading image\n";
    return 0;
  }

  memcpy(&(mem->contentsBuffer[mem->contentsSize]), contents, downloadSize);
  mem->contentsSize += downloadSize;
  mem->contentsBuffer[mem->contentsSize] = 0;

  return downloadSize;
}

NativeJSRenderer::NativeJSRenderer(std::string waylandDisplay): mEngine(nullptr), mRunning(true), mEnableTestFileDOMSupport(false), mEmbedThunderJS(false), mEmbedRdkWebBridge(false), mEnableWebSocketServer(false), mContextMap(), mEssosInitialized(false), mConsoleMode(false)
{
    if (waylandDisplay.size() > 0)
    {
        setenv("WAYLAND_DISPLAY", waylandDisplay.c_str(), 1);
#ifdef ENABLE_ESSOS
        mEssosInitialized = EssosInstance::instance()->initialize(true);
#endif
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
        std::cout << "thunderjs enabled via environment " << std::endl;
    }
    else
    {
	std::ifstream f(NATIVEJS_EMBED_THUNDERJS);
        if (f.good())
        {
            mEmbedThunderJS = true;
            std::cout << "thunderjs enabled via file presence " << std::endl;
        }			
    }		    

    std::ifstream f(NATIVEJS_EMBED_WEBBRIDGE);
    if (f.good())
    {
        mEmbedRdkWebBridge = true;
        std::cout << "rdk webbridge enabled via file presence " << std::endl;
    }			
    char* enableWebSocketServer = getenv("NATIVEJS_ENABLE_WEBSOCKET_SERVER");
    if (enableWebSocketServer)
    {
        mEnableWebSocketServer = true;
        std::cout << "websocket server enabled via environment " << std::endl;
    }
    else
    {
	std::ifstream f(NATIVEJS_ENABLE_WEBSOCKET_SERVER);
        if (f.good())
        {
            mEnableWebSocketServer = true;
            std::cout << "websocket server enabled via file presence " << std::endl;
        }			
    }		    
}

NativeJSRenderer::~NativeJSRenderer()
{
    gPendingUrlRequests.clear();
    gPendingUrlTerminateRequests.clear();
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
        std::cout << "Context could not be initialized - exiting" << std::endl;
        return;
    }
    mConsoleState->consoleContext = context;

    std::cout << "Running developer console..." << std::endl;
    std::thread consoleThread(&JsRuntime::NativeJSRenderer::runDeveloperConsole, this, std::ref(mConsoleState->moduleSettings));
    consoleThread.detach();

    mConsoleMode = true;
}

void NativeJSRenderer::launchApplication(std::string url, ModuleSettings& moduleSettings)
{
    metrics.startTime = getTimeInMilliSec();
    std::cout << "\n-----START TIME-----: " << std::fixed << std::setprecision(3) << metrics.startTime << " ms\n";

    mUserMutex.lock();
    ApplicationRequest request(url, moduleSettings.enableHttp, moduleSettings.enableXHR, moduleSettings.enableWebSocket, moduleSettings.enableWebSocketEnhanced, moduleSettings.enableFetch, moduleSettings.enableJSDOM, moduleSettings.enableWindow, moduleSettings.enablePlayer);
    gPendingUrlRequests.push_back(request);
    mUserMutex.unlock();
}

void NativeJSRenderer::terminateApplication(std::string url)
{
    mUserMutex.lock();
    gPendingUrlTerminateRequests.push_back(url);
    mUserMutex.unlock();
}

std::vector<std::string> NativeJSRenderer::getApplications()
{
    std::vector<std::string> applications;
    mUserMutex.lock();
    std::map<std::string, IJavaScriptContext*>::iterator mapIter = mContextMap.end();
    for (mapIter = mContextMap.begin(); mapIter != mContextMap.end(); mapIter++)
    {
        applications.push_back(mapIter->first);
    }	    
    mUserMutex.unlock();
    return applications;
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
	mUserMutex.lock();
        if (mConsoleMode) {
            processDevConsoleRequests();
        }

        for (int i=0; i<gPendingUrlTerminateRequests.size(); i++)
        {
            unloadApplication(gPendingUrlTerminateRequests[i]);
        }
        gPendingUrlTerminateRequests.clear();
        for (int i=0; i<gPendingUrlRequests.size(); i++)
        {
            ModuleSettings settings;
	    settings.enableHttp = gPendingUrlRequests[i].mEnableHttp;
	    settings.enableXHR = gPendingUrlRequests[i].mEnableXHR;
	    settings.enableWebSocket = gPendingUrlRequests[i].mEnableWebSocket;
	    settings.enableWebSocketEnhanced = gPendingUrlRequests[i].mEnableWebSocketEnhanced;
	    settings.enableFetch = gPendingUrlRequests[i].mEnableFetch;
	    settings.enableJSDOM = gPendingUrlRequests[i].mEnableJSDOM;
	    settings.enableWindow = gPendingUrlRequests[i].mEnableWindow;
	    settings.enablePlayer = gPendingUrlRequests[i].mEnablePlayer;
            loadApplication(gPendingUrlRequests[i].mUrl, settings);
        }
        gPendingUrlRequests.clear();
	mUserMutex.unlock();
        if(!mTestFileName.empty())
        {			
            ModuleSettings settings;
	    settings.enableJSDOM = mEnableTestFileDOMSupport;
            loadApplication(mTestFileName, settings);
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

void NativeJSRenderer::loadApplication(std::string url, ModuleSettings& moduleSettings)
{
    if (mContextMap.find(url) != mContextMap.end())
    {
        unloadApplication(url);
    }
    if (!url.empty())
    {
        std::cout << "before launching app " <<std::endl;
        std::string urlPattern = url.substr(0, 4);
        if (urlPattern.compare(0, 4, "http") == 0)
        {
            MemoryStruct chunk;
            bool ret = downloadFile(url, chunk);
            if (!ret)
            {
                return;
            }
            JavaScriptContextFeatures features(mEmbedThunderJS, mEmbedRdkWebBridge, mEnableWebSocketServer, moduleSettings);
            JavaScriptContext* context = new JavaScriptContext(features, url, mEngine);
            if (NULL == context)
            {
                std::cout << "nativeJS application " << url << " cannot be loaded" << std::endl;
	        return;
            }
            mContextMap[url] = context;
            std::cout << "nativeJS application thunder execution url " << url << " result " << ret << std::endl;
            ret = context->runScript(chunk.contentsBuffer, true, url, nullptr, true);
            std::cout << "nativeJS application execution result " << ret << std::endl;
        }
        else
        {	    
            std::cout << "about to launch local app " <<std::endl;
            JavaScriptContextFeatures features(mEmbedThunderJS, mEmbedRdkWebBridge, mEnableWebSocketServer, moduleSettings);
            JavaScriptContext* context = new JavaScriptContext(features, url, mEngine);
            if (NULL == context)
            {
                std::cout << "nativeJS application " << url << " cannot be loaded" << std::endl;
                return;
            }
            mContextMap[url] = context;
            std::cout << "running test application " << url << std::endl;
            bool ret = context->runFile(url.c_str(), nullptr, true);
            std::cout << "test application execution result " << ret << std::endl;
        }
    }
    else
    {
        std::cout << "nativeJS application url not proper" << std::endl;
	return;
    }
    return;
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

    std::cout << "\nJSRuntime Developer Console\n";
    std::cout << "Type 'exit' or press CTRL+C and ENTER to quit.\n\n";

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
            break;
        }

        mConsoleState->inputMutex.lock();
        mConsoleState->codeToExecute.push_back(input);
        mConsoleState->inputMutex.unlock();
    }

    signal(SIGINT, SIG_DFL);
}

void NativeJSRenderer::unloadApplication(std::string url)
{
    std::map<std::string, IJavaScriptContext*>::iterator mapEntry = mContextMap.find(url);
    if (mapEntry != mContextMap.end())
    {
        JavaScriptContext* context = (JavaScriptContext*)mapEntry->second;
	if (NULL != context)
	{     	
            std::cout << "deleted context " <<std::endl;
            delete context;
        }
        mContextMap.erase(mapEntry);
    }
    else
    {
        std::cout << "unable to find application with url " << url << std::endl;
    }	    
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
            std::cout << "download operation success" << std::endl;
            ret = true;
        }
        else
        {
            std::cout << "download operation failed" << std::endl;
        }
    }
    else
    {
        std::cout << "unable to perform download " << std::endl;
    } 
    return ret;
}
