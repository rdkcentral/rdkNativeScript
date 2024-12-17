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

#include <unistd.h>
#include <errno.h>

#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include <iostream>

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
#ifndef ENABLE_AAMP_JSBINDINGS
#include <PlayerWrapper.h>
#endif

extern "C" JS_EXPORT void JSSynchronousGarbageCollectForDebugging(JSContextRef);
#ifdef ENABLE_AAMP_JSBINDINGS
extern void AAMPPlayer_LoadJS(void* context);
extern void AAMPPlayer_UnloadJS(void* context);
#endif

JSContextGroupRef globalContextGroup()
{
    static JSContextGroupRef gGroupRef = JSContextGroupCreate();
    return gGroupRef;
}

JSGlobalContextRef gTopLevelContext = nullptr;

JavaScriptContext::JavaScriptContext(bool embedThunderJS, bool embedWebBridge, bool enableWebSockerServer, std::string url, IJavaScriptEngine* jsEngine):JavaScriptContextBase(embedThunderJS, embedWebBridge, enableWebSockerServer, url, jsEngine)
{
    rtLogInfo("%s", __FUNCTION__);
    mContextGroup = JSContextGroupRetain(globalContextGroup());
    mContext = JSGlobalContextCreateInGroup(mContextGroup, nullptr);
    mPriv = rtJSCContextPrivate::create(mContext);
    if (!gTopLevelContext)
      gTopLevelContext = mContext;
    registerUtils();
    registerCommonUtils();
}

JavaScriptContext::~JavaScriptContext()
{
    rtLogInfo("%s begin", __FUNCTION__);
  
#ifdef ENABLE_AAMP_JSBINDINGS
    AAMPPlayer_UnloadJS(mContext);
#else
    deinitializePlayer(mContext);
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

bool JavaScriptContext::evaluateScript(const char* script, const char* name, const char *args)
{
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
                std::cout << "received exception during key handling ";
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
    m_httpGetBinding = new rtFunctionCallback(rtHttpGetBinding, nullptr);
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
  
#ifdef ENABLE_AAMP_JSBINDINGS
    AAMPPlayer_LoadJS(mContext);
#else
    initializePlayer(mContext);
#endif
    runFile("modules/window.js", nullptr);
    runFile("modules/ws.js", nullptr);
#ifdef WS_SERVER_ENABLED
    if (mEnableWebSockerServer)
    {
        std::cout << "enabling websocket server " << std::endl;
        runFile("modules/wsserver.js", nullptr);
    }
#endif
    runFile("modules/http.js", nullptr);
    runFile("modules/https.js", nullptr);
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
    runFile("modules/xhr.js", nullptr);
    runFile("modules/url.js", nullptr);
}
