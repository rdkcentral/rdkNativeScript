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
#include <signal.h>

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
            std::cout << "failed to load " << aampJSBindingsLib << " and error is " << dlerror();
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
    m_readBinaryBinding = new rtFunctionCallback(rtReadBinaryBinding, nullptr);
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
	runFile("modules/video.js", nullptr);
    }
    if (mModuleSettings.enableXHR)
    {
        runFile("modules/xhr.js", nullptr);
    }
    if (mModuleSettings.enableHttp)
    {
        runFile("modules/http.js", nullptr);
        runFile("modules/https.js", nullptr);
    }
    if (mModuleSettings.enableFetch)
    {
        runFile("modules/node-fetch.js", nullptr/*, true*/);
    }
    runFile("modules/utils.js", nullptr);
    if (mModuleSettings.enableWebSocketEnhanced)
    {
        runFile("modules/event.js", nullptr);
        runFile("modules/wsenhanced.js", nullptr);
    }
    else if(mModuleSettings.enableWebSocket)
    {
        runFile("modules/ws.js", nullptr);
    }
#ifdef WS_SERVER_ENABLED
    if (mEnableWebSockerServer)
    {
        std::cout << "enabling websocket server " << std::endl;
        runFile("modules/wsserver.js", nullptr);
    }
#endif
    if (mModuleSettings.enableWindow)
    {
        runFile("modules/window.js", nullptr/*, true*/);
        runFile("modules/windowwrapper.js", nullptr/*, true*/);
    }
    else if (mModuleSettings.enableJSDOM)
    {
        runFile("modules/linkedjsdom.js", nullptr/*, true*/);
        runFile("modules/linkedjsdomwrapper.js", nullptr/*, true*/);
        runFile("modules/windowwrapper.js", nullptr/*, true*/);
    }
}
