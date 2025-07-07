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
#include <unistd.h>
#include <errno.h>

#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include <iostream>
#include <signal.h>

#include <fstream>
#include <sstream>
#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptContext.h>
#include <JavaScriptUtils.h>
#include <rtLog.h>
#include <rtString.h>
#include "rtWebSocket.h"
#ifdef WS_SERVER_ENABLED
#include "rtWebSocketServer.h"
#endif
#include "JavaScriptWrapper.h"
#ifdef ENABLE_JSRUNTIME_PLAYER
#ifndef ENABLE_AAMP_JSBINDINGS
#include <PlayerWrapper.h>
#endif
#if ENABLE_AAMP_JSBINDINGS_DYNAMIC
#include <dlfcn.h>
#endif
#endif
#include <sstream>
#include <fstream>

extern "C" JS_EXPORT void JSSynchronousGarbageCollectForDebugging(JSContextRef);
#ifdef ENABLE_AAMP_JSBINDINGS_STATIC
extern void AAMPPlayer_LoadJS(void* context);
extern void AAMPPlayer_UnloadJS(void* context);
#endif
#ifdef ENABLE_AAMP_JSBINDINGS_DYNAMIC
static AAMPJSBindings* gAAMPJSBindings = nullptr;
#endif
//BIG CHANGE
extern void functionLoadModule(JSGlobalContextRef ref, JSObjectRef globalObjectRef, char* buffer, int len, char* name);

static const char* envValue = std::getenv("NATIVEJS_DUMP_NETWORKMETRIC");

JSContextGroupRef globalContextGroup()
{
    static JSContextGroupRef gGroupRef = JSContextGroupCreate();
    return gGroupRef;
}

JSGlobalContextRef gTopLevelContext = nullptr;

JavaScriptContext::JavaScriptContext(JavaScriptContextFeatures& features, std::string url, IJavaScriptEngine* jsEngine):JavaScriptContextBase(features, url, jsEngine)
{
    rtLogInfo("%s", __FUNCTION__);
    mContextGroup = JSContextGroupRetain(globalContextGroup());
    mContext = JSGlobalContextCreateInGroup(mContextGroup, nullptr);
    mPriv = rtJSCContextPrivate::create(mContext);
    if (!gTopLevelContext)
      gTopLevelContext = mContext;
    registerUtils();
    registerCommonUtils();
    mNetworkMetricsData = new rtMapObject();
}

JavaScriptContext::~JavaScriptContext()
{
    rtLogInfo("%s begin", __FUNCTION__);
  
#ifdef ENABLE_JSRUNTIME_PLAYER
if (mModuleSettings.enablePlayer)
{
#ifdef ENABLE_AAMP_JSBINDINGS
#ifdef ENABLE_AAMP_JSBINDINGS_STATIC
    AAMPPlayer_UnloadJS(mContext);
#else
    if (gAAMPJSBindings->fnUnloadJS)
    {
        gAAMPJSBindings->fnUnloadJS(mContext);
    }
    if ((nullptr != gAAMPJSBindings) && (gTopLevelContext == mContext))
    {
        unloadAAMPJSBindingsLib();
        delete gAAMPJSBindings;
        gAAMPJSBindings = nullptr;
    }
#endif
#else
    deinitializePlayer(mContext);
#endif
}
#endif
    if (gTopLevelContext == mContext)
    {
        if (gTopLevelContext != nullptr)
        {
            JSSynchronousGarbageCollectForDebugging(gTopLevelContext);
        }
        gTopLevelContext = nullptr;
    }
    mPriv->releaseAllProtected();
    JSGlobalContextRelease(mContext);
    JSContextGroupRelease(mContextGroup);
    rtLogInfo("%s end", __FUNCTION__);
}

#ifdef ENABLE_JSRUNTIME_PLAYER
#ifdef ENABLE_AAMP_JSBINDINGS_DYNAMIC
void JavaScriptContext::loadAAMPJSBindingsLib()
{
    std::cout<<"Dynamic mode is enabled"<<std::endl;
    if (nullptr == gAAMPJSBindings->PlayerLibHandle)
    {
        static const char *aampJSBindingsLib = "libaampjsbindings.so";
	static const char *jscLib = "libJavaScriptCore.so";
	void *jscLibHandle = dlopen(jscLib, RTLD_NOW | RTLD_GLOBAL);
	if (!jscLibHandle)
	{
	    std::cout<<"dlopen error for jsc library " << dlerror() << std::endl;
	}
        void *aampJSBindingsLibHandle = dlopen(aampJSBindingsLib, RTLD_NOW | RTLD_GLOBAL);
        if (aampJSBindingsLibHandle)
        {
            gAAMPJSBindings->PlayerLibHandle = aampJSBindingsLibHandle;

            gAAMPJSBindings->fnLoadJS =
                    reinterpret_cast<typeof AAMPJSBindings::fnLoadJS>(
                            dlsym(aampJSBindingsLibHandle, "_Z17AAMPPlayer_LoadJSPv"));
            gAAMPJSBindings->fnUnloadJS =
                    reinterpret_cast<typeof AAMPJSBindings::fnUnloadJS>(
                            dlsym(aampJSBindingsLibHandle, "AAMPPlayer_UnloadJS"));
        }
        else
        {
            NativeJSLogger::log(ERROR, "failed to load %s and error is %s\n", aampJSBindingsLib, dlerror());
        }
	
	dlclose(jscLibHandle);
    }
}

void JavaScriptContext::unloadAAMPJSBindingsLib()
{
    if (nullptr != gAAMPJSBindings->PlayerLibHandle)
    {
        dlclose(gAAMPJSBindings->PlayerLibHandle);
    }
}
#endif
#endif

rtError JavaScriptContext::add(const char *name, rtValue const& val)
{
    JSStringRef jsName = JSStringCreateWithUTF8CString(name);
    JSValueRef jsVal = rtToJs(mContext, val);
    JSObjectRef globalObj = JSContextGetGlobalObject(mContext);
    JSValueRef exception = nullptr;
    JSObjectSetProperty(mContext, globalObj, jsName, jsVal, kJSPropertyAttributeDontEnum, &exception);
    JSStringRelease(jsName);
  
    if (exception)
    {
        JSStringRef exceptStr = JSValueToStringCopy(mContext, exception, nullptr);
        rtString errorStr = jsToRtString(exceptStr);
        JSStringRelease(exceptStr);
        rtLogError("Failed to add to rtScript context, error='%s'\n", errorStr.cString());
        return RT_FAIL;
    }
    return RT_OK;
}

rtValue JavaScriptContext::get(const char *name)
{
    if (!name)
      return {};
  
    JSStringRef jsName = JSStringCreateWithUTF8CString(name);
    JSObjectRef globalObj = JSContextGetGlobalObject(mContext);
    JSValueRef exception = nullptr;
    rtValue retVal;
  
    do
    {
        JSValueRef valueRef = JSObjectGetProperty(mContext, globalObj, jsName, &exception);
        if (exception)
          break;
        jsToRt(mContext, valueRef, retVal, &exception);
        if (exception)
          break;
    } while(0);
    JSStringRelease(jsName);
  
    if (exception)
    {
        JSStringRef exceptStr = JSValueToStringCopy(mContext, exception, nullptr);
        rtString errorStr = jsToRtString(exceptStr);
        JSStringRelease(exceptStr);
        rtLogError("Failed to get %s from rtScript context, error='%s'\n", name, errorStr.cString());
        return rtValue();
    }
  
    return retVal;
}

bool JavaScriptContext::has(const char *name)
{
    if (!name)
      return {};
    JSStringRef jsName = JSStringCreateWithUTF8CString(name);
    JSObjectRef globalObj = JSContextGetGlobalObject(mContext);
    bool ret = JSObjectHasProperty(mContext, globalObj, jsName);
    JSStringRelease(jsName);
    return ret;
}

bool JavaScriptContext::evaluateScript(const char* script, const char* name, const char *args, bool module)
{
    //execution start time
    mPerformanceMetrics.executionStartTime = getTimeInMilliSec();

    if (nullptr != name)
    {	  
      rtLogInfo("JavaScriptContext::evaluateScript name=%s", name);
    }
    JSValueRef exception = nullptr;
    JSStringRef codeStr = JSStringCreateWithUTF8CString(script);
    JSObjectRef globalObj = JSContextGetGlobalObject(mContext);
    JSStringRef fileStr = nullptr;
  
    if (name)
    {
        fileStr = JSStringCreateWithUTF8CString(name);
        JSGlobalContextSetName(mContext, fileStr);
    } 
  
    //MADANA BIG CHANGE
    if (module)
    {
          JSStringRef fileNameProperty = JSStringCreateWithUTF8CString("entryPointModuleName");
          JSStringRef fileName = JSStringCreateWithUTF8CString(name);
          JSValueRef value = JSValueMakeString(mContext, fileName);
          JSObjectSetProperty(mContext, globalObj, fileNameProperty, value, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, nullptr);
          JSStringRelease(fileNameProperty);
          JSStringRelease(fileName);
        functionLoadModule(mContext, globalObj, (char *) script, strlen(script), (char*)name);
    }
    else
    {	    
        JSValueRef result = JSEvaluateScript(mContext, codeStr, globalObj, fileStr, 0, &exception);
        JSStringRelease(codeStr);
        if (nullptr != fileStr)
        {	  
            JSStringRelease(fileStr);
        }
        if (exception)
        {
            JSStringRef exceptStr = JSValueToStringCopy(mContext, exception, nullptr);
            rtString errorStr = jsToRtString(exceptStr);
            JSStringRelease(exceptStr);
            rtLogError("Failed to eval, error='%s'", errorStr.cString());
            return false;
        }
    }

    //execution end time
    mPerformanceMetrics.executionEndTime = getTimeInMilliSec();

    return true;
}

void JavaScriptContext::processKeyEvent(struct JavaScriptKeyDetails& details, bool keyPress)
{
    JSStringRef str = JSStringCreateWithUTF8CString(keyPress?"jsruntime.onKeyDown":"jsruntime.onKeyUp");
  
    JSValueRef func = JSEvaluateScript(mContext, str, 0, 0, 0, 0);
  
    if (JSValueIsObject(mContext, func))
    {
        JSObjectRef funcObj = JSValueToObject(mContext, func, 0);
  
        if (funcObj && JSObjectIsFunction(mContext, funcObj))
	{
            rtObjectRef e = new rtMapObject;
            rtString type(details.type.c_str());
            rtString key(details.key.c_str());
            rtString code(details.code.c_str());
            e.set("type", type);
            e.set("key", key);
            e.set("code", code);
            e.set("shiftKey", details.shiftKey);
            e.set("ctrlKey", details.ctrlKey);
            e.set("altKey", details.altKey);
            e.set("metaKey", details.metaKey);
            e.set("repeat", details.repeat);
            e.set("keyCode", details.keyCode);
            
  
            const JSValueRef args[] = { rtToJs(mContext, e) };
  
            size_t num_args = sizeof(args) / sizeof(JSValueRef*);
  
            JSValueRef exception = 0;
  
            JSValueRef result = JSObjectCallAsFunction(mContext, funcObj, 0, 
                                                       num_args, args, 
                                                       &exception);
  
            if (exception)
	    {
		 NativeJSLogger::log(ERROR, "received exception during key handling\n");
            }
            
            if (result)
	    {
                // Handle result (if any) here.
            }
        }
    }
}

void JavaScriptContext::registerUtils()
{
    m_webSocketBinding = new rtFunctionCallback(rtWebSocketBinding, nullptr);
#ifdef WS_SERVER_ENABLED
    if (mEnableWebSockerServer)
    {	  
        m_webSocketServerBinding = new rtFunctionCallback(rtWebSocketServerBinding, nullptr);
    }
#endif
    m_setTimeoutBinding = new rtFunctionCallback(rtSetTimeoutBinding, nullptr);
    m_clearTimeoutBinding = new rtFunctionCallback(rtClearTimeoutBinding, nullptr);
    m_setIntervalBinding = new rtFunctionCallback(rtSetItervalBinding, nullptr);
    m_clearIntervalBinding = new rtFunctionCallback(rtClearTimeoutBinding, nullptr);
    m_thunderTokenBinding = new rtFunctionCallback(getThunderTokenBinding, this);
    m_httpGetBinding = new rtFunctionCallback(rtHttpGetBinding, this);
    m_readBinaryBinding = new rtFunctionCallback(rtReadBinaryBinding, nullptr);
    m_setVideoStartTimeBinding = new rtFunctionCallback(rtSetVideoStartTimeBinding, this);
    m_JSRuntimeDownloadMetrics = new rtFunctionCallback(rtJSRuntimeDownloadMetrics, this);
    add("webSocket", m_webSocketBinding.getPtr());
#ifdef WS_SERVER_ENABLED
    if (mEnableWebSockerServer)
    {	  
        add("webSocketServer", m_webSocketServerBinding.getPtr());
    }
#endif
    add("setTimeout", m_setTimeoutBinding.getPtr());
    add("clearTimeout", m_clearTimeoutBinding.getPtr());
    add("setInterval", m_setIntervalBinding.getPtr());
    add("clearInterval", m_clearIntervalBinding.getPtr());
    add("thunderToken", m_thunderTokenBinding.getPtr());
    add("httpGet", m_httpGetBinding.getPtr());
    add("readBinary", m_readBinaryBinding.getPtr());
    add("setVideoStartTime", m_setVideoStartTimeBinding.getPtr());
    add("JSRuntimeDownloadMetrics", m_JSRuntimeDownloadMetrics.getPtr());
  
#ifdef ENABLE_JSRUNTIME_PLAYER
if (mModuleSettings.enablePlayer)
{
#ifdef ENABLE_AAMP_JSBINDINGS
#ifdef ENABLE_AAMP_JSBINDINGS_STATIC
    AAMPPlayer_LoadJS(mContext);
#else
    if ((nullptr == gAAMPJSBindings) && (gTopLevelContext == mContext))
    {
        gAAMPJSBindings = new AAMPJSBindings();
        loadAAMPJSBindingsLib();
    }
    if (gAAMPJSBindings->fnLoadJS)
    {
        gAAMPJSBindings->fnLoadJS(mContext);
    }
#endif
#else
    initializePlayer(mContext);
#endif
}
#endif
    auto injectFun =
      [](JSContextRef jsContext, const char* name, JSObjectCallAsFunctionCallback callback) {
          JSContextRef globalCtx = JSContextGetGlobalContext(jsContext);
          JSObjectRef globalObj = JSContextGetGlobalObject(globalCtx);
          JSStringRef funcName = JSStringCreateWithUTF8CString(name);
          JSObjectRef funcObj = JSObjectMakeFunctionWithCallback(jsContext, funcName, callback);
          JSObjectSetProperty(jsContext, globalObj, funcName, funcObj, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, nullptr);
          JSStringRelease(funcName);
      };
    injectFun(mContext, "require", requireCallback);
    if(mModuleSettings.enablePlayer)
    {
       runFile("video.js", nullptr);
    }
    if (mModuleSettings.enableXHR)
    {
        runFile("xhr.js", nullptr);
    }
    if (mModuleSettings.enableHttp)
    {
        runFile("http.js", nullptr);
        runFile("https.js", nullptr);
    }
    if (mModuleSettings.enableFetch)
    {
        runFile("node-fetch.js" , nullptr/*, true*/);
    }
    runFile("utils.js", nullptr);
    if (mModuleSettings.enableWebSocketEnhanced)
    {
        runFile("event.js", nullptr);
        runFile("wsenhanced.js", nullptr);
}
    else if(mModuleSettings.enableWebSocket)
    {
        runFile("ws.js", nullptr);
    }
#ifdef WS_SERVER_ENABLED
    if (mEnableWebSockerServer)
    {
        NativeJSLogger::log(INFO, "enabling websocket server\n");
        runFile("wsserver.js", nullptr);
    }
#endif
    if (mModuleSettings.enableWindow)
    {
        runFile("window.js", nullptr/*, true*/);
        runFile("windowwrapper.js", nullptr/*, true*/);
    }
    else if (mModuleSettings.enableJSDOM)
    {
        runFile("linkedjsdom.js", nullptr/*, true*/);
        runFile("linkedjsdomwrapper.js", nullptr/*, true*/);
        runFile("windowwrapper.js", nullptr/*, true*/);
        if(getenv("FIREBOLT_ENDPOINT")!=NULL)
        {
            auto FireboltEndpoint = std::string(getenv("FIREBOLT_ENDPOINT"));
            std::stringstream ss;
            ss << "window.__firebolt = {\"endpoint\":\"" << FireboltEndpoint << "\"};";
            NativeJSLogger::log(INFO, "Adding the Firebolt EndPoint value: %s to window.js file\n", FireboltEndpoint.c_str());
            ss << "var self = window;";
            ss << "let videoDiv = document.createElement(\"div\");";
            ss << "videoDiv.id = \"videoDiv\";";
            ss << "document.body.appendChild(videoDiv)";
            evaluateScript(ss.str().c_str(),nullptr);
        }
    }
}

void JavaScriptContext::onMetricsData (NetworkMetrics *net)
{
   if (!net) {
        rtLogError("onMetricsData: Received null NetworkMetrics structure.");
        return;
    }
    rtString key = net->url;
    mNetworkMetricsData->set(key, rtValue((void *)net));

    if (envValue) {
        dumpNetworkMetricData(net, this->getUrl());
    }
}

void JavaScriptContext::dumpNetworkMetricData(NetworkMetrics *metrics, std::string appUrl)
{
    std::ofstream file("/tmp/jsruntimenetworkmetrics.log", std::ios::app);

    if (!file.is_open()) 
    {
        rtLogError("Failed to open jsruntimenetworkmetrics log file");
        return;
    }
    static bool isAppUrlLogged = false; 
    if(!isAppUrlLogged)
    {
	isAppUrlLogged = true;    
	    file << appUrl << ":";
    }
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&t);
    char timestampBuffer[20];
    std::strftime(timestampBuffer, sizeof(timestampBuffer), "%Y-%m-%d %H:%M:%S", &tm);
    std::stringstream ss;
    ss << "  [" << std::endl;
    ss << "  timestamp: " << timestampBuffer << std::endl;
    ss << "  url: " << metrics->url << "," << std::endl;
    ss << "  method: " << metrics->method << "," << std::endl;
    ss << "  headers: [";
    for (size_t j = 0; j < metrics->headers.size(); ++j) 
    {
	ss << metrics->headers[j];
        if (j != metrics->headers.size() - 1) 
        {
           ss << ", ";
        }
    }
    ss << "]," << std::endl;
    ss << "  statusCode: " << metrics->statusCode << std::endl;
    for (const auto& pair : metrics->timeMetricsData) 
    {              
	ss << "  " << pair.first.cString() << ": " << pair.second.toString().cString() << "\n";
    }
    ss << "]" << std::endl;
    file << ss.str();
    file.close();
}

void JavaScriptContext::setCreateApplicationStartTime(double time)
{
    mPerformanceMetrics.createApplicationStartTime = time;
}

void JavaScriptContext::setCreateApplicationEndTime(double time, uint32_t id)
{
    mPerformanceMetrics.createApplicationEndTime = time;
    double duration = time-mPerformanceMetrics.createApplicationStartTime;
    
    NativeJSLogger::log(INFO, "createApplicationDuration for ID %d: %.3f ms\n",id, duration);
}

double JavaScriptContext::getExecutionDuration() const
{
    double executionDuration = mPerformanceMetrics.executionEndTime - mPerformanceMetrics.executionStartTime;
    return executionDuration;
}

void JavaScriptContext::setAppdata(uint32_t id, const std::string& url)
{
        mIds = id;
        mUrls = url;
} 

void JavaScriptContext::setPlaybackStartTime(double time)
{
    mPerformanceMetrics.playbackStartTime = time;
    double launchTime = mPerformanceMetrics.playbackStartTime - mPerformanceMetrics.createApplicationStartTime;
    NativeJSLogger::log(INFO, "Launch_Duration for ID %d | URL %s : %.3f ms\n", mIds, mUrls.c_str(), launchTime);
}
