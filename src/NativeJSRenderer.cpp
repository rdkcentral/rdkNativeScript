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
        std::cout << "Context could not be initialized - exiting" << std::endl;
        return;
    }
    mConsoleState->consoleContext = context;

    std::cout << "Running developer console..." << std::endl;
    std::thread consoleThread(&JsRuntime::NativeJSRenderer::runDeveloperConsole, this, std::ref(mConsoleState->moduleSettings));
    consoleThread.detach();

    mConsoleMode = true;
}

uint32_t createID()
{
    static uint32_t id = 1;
    std::cout<<"Creating id: "<<id<<std::endl;
    return id++;
}

bool NativeJSRenderer::createApplication(ModuleSettings& moduleSettings)
{
	mUserMutex.lock();
    	mId = createID();
	ApplicationRequest request(mId, CREATE, "", moduleSettings.enableHttp, moduleSettings.enableXHR, moduleSettings.enableWebSocket, moduleSettings.enableWebSocketEnhanced, moduleSettings.enableFetch, moduleSettings.enableJSDOM, moduleSettings.enableWindow, moduleSettings.enablePlayer);
	gPendingRequests.push_back(request);
	mUserMutex.unlock();
	return true;
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
		std::cout<<"No application found.. "<<std::endl;
	}
	else {
		for(const auto& [key, value] : mContextMap)
		{
			ApplicationDetails appData;
			appData.id = key;
			appData.url = value.url;
			std::cout<<"Found application with ID: "<<key<<" and url:"<<value.url<<std::endl;
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
	
	JavaScriptContextFeatures features(mEmbedThunderJS, mEmbedRdkWebBridge, mEnableWebSocketServer, settings);
        JavaScriptContext* context = new JavaScriptContext(features, " " , mEngine);
        if(NULL == context)
        {
        	std::cerr<<"context not created for id: "<< id <<std::endl;
        	return ;
        }
        std::cout<<"Context created for id: "<<id<<std::endl;
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
		std::cout<<"Before launching app"<<std::endl;
		std::string urlPattern = url.substr(0, 4);
		if (urlPattern.compare(0, 4, "http") == 0)
		{
			MemoryStruct chunk;
			bool ret = downloadFile(url, chunk);
			if (!ret)
			{
			    return ;
			}
			IJavaScriptContext* context = mContextMap[id].context;
			std::cout << "nativeJS application thunder execution url " << url << " result " << ret << std::endl;
			ret = context->runScript(chunk.contentsBuffer, true, url, nullptr, true);
			std::cout << "nativeJS application execution result " << ret << std::endl; 
		}
		else
		{	    
			std::cout << "about to launch local app " <<std::endl;
			IJavaScriptContext* context = mContextMap[id].context;
			std::cout << "running test application " << url << std::endl;
			bool ret = context->runFile(url.c_str(), nullptr, true);
			std::cout << "test application execution result " << ret << std::endl;
		}	    
	}
	else{
	    std::cout << "nativeJS application url not proper" << std::endl;
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
	
	mContextMap[id].url = code;
	if(!code.empty())
	{
		std::cout<<"Running the JavaScript code "<<std::endl;
		IJavaScriptContext* context = mContextMap[id].context;
		std::string rawcode = code ;
		bool ret = context-> runScript(rawcode.c_str(),true,"JavaScriptCode",nullptr,true);
	}
	else{
		std::cout<<"Empty/Invalid JavaScript Code"<<std::endl;
	}
}

void NativeJSRenderer::terminateApplicationInternal(ApplicationRequest& AppRequest)
{
	uint32_t id = AppRequest.mId;
	std::map<uint32_t, ApplicationData>::iterator mapEntry = mContextMap.find(id);
	if (mapEntry != mContextMap.end())
	{
		JavaScriptContext* context = (JavaScriptContext*)mContextMap[id].context;
		std::string* Url = &mContextMap[id].url;
		std::cout<<"terminating the application with id "<<id<<std::endl;
		if (NULL != context)	{     	
			std::cout << "deleted context " <<std::endl;
			delete context;
		}	
        mContextMap.erase(mapEntry);	
		std::cout<<"Application is terminated"<<std::endl;
	}
	
	else
	{
		std::cout << "unable to find application" << id <<" with url: "<<mContextMap[id].url << std::endl;
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
            if(request.mRequestType == 0)
            {
		NativeJSRenderer::createApplicationInternal(request);
            }
            else if(request.mRequestType == 1)
            {
		NativeJSRenderer::runApplicationInternal(request);
		
            }
            else if(request.mRequestType == 2)
            {           	
		NativeJSRenderer::terminateApplicationInternal(request);
            }
            else if(request.mRequestType == 3)
            {
            	NativeJSRenderer::runJavaScriptInternal(request);
            }
            else 
            {
            	std::cerr<<"Invalid Request Type"<<std::endl;
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
