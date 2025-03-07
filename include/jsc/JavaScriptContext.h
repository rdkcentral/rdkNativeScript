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

#ifndef JAVASCRIPTCONTEXT_H
#define JAVASCRIPTCONTEXT_H

#include <unistd.h>
#include <errno.h>

#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include <rtValue.h>
#include <rtObject.h>

#include <JavaScriptCore/JavaScript.h>
#include <JavaScriptContextBase.h>
#include "rtScriptJSCPrivate.h"
#include <KeyListener.h>
#include <KeyInput.h>

#include <JavaScriptCore/JavaScript.h>
#ifdef ENABLE_JSRUNTIME_PLAYER
#ifdef ENABLE_AAMP_JSBINDINGS_STATIC
#include <PlayerWrapper.h>
#endif
#endif

#ifdef ENABLE_JSRUNTIME_PLAYER
#ifdef ENABLE_AAMP_JSBINDINGS
struct AAMPJSBindings
{
    void *PlayerLibHandle = nullptr;
    void (*fnLoadJS)(JSGlobalContextRef context) = nullptr;
    void (*fnUnloadJS)(JSGlobalContextRef context) = nullptr;
};
#endif
#endif

extern "C" JS_EXPORT void JSSynchronousGarbageCollectForDebugging(JSContextRef);

class JavaScriptContext: public JavaScriptContextBase
{
  public:
    JavaScriptContext(JavaScriptContextFeatures& features, std::string url, IJavaScriptEngine* jsEngine);
    virtual ~JavaScriptContext();
  
    rtValue get(const char *name);
    rtError add(const char *name, rtValue  const& val);
    bool    has(const char *name);
    JSGlobalContextRef getContext() { return mContext; }

  private:
    bool evaluateScript(const char *script, const char *name, const char *args = nullptr, bool module = false);
    void processKeyEvent(struct JavaScriptKeyDetails& details, bool keyPress);
    void registerUtils();
#ifdef ENABLE_JSRUNTIME_PLAYER
#ifdef ENABLE_AAMP_JSBINDINGS_DYNAMIC
    void loadAAMPJSBindingsLib();
    void unloadAAMPJSBindingsLib();
#endif
#endif
    JSContextGroupRef mContextGroup;
    JSGlobalContextRef mContext;
    rtRef<rtJSCContextPrivate> mPriv;
    rtRef<rtFunctionCallback> m_webSocketBinding;
    rtRef<rtFunctionCallback> m_webSocketServerBinding;
    rtRef<rtFunctionCallback> m_setTimeoutBinding;
    rtRef<rtFunctionCallback> m_clearTimeoutBinding;
    rtRef<rtFunctionCallback> m_setIntervalBinding;
    rtRef<rtFunctionCallback> m_clearIntervalBinding;
    rtRef<rtFunctionCallback> m_thunderTokenBinding;
    rtRef<rtFunctionCallback> m_httpGetBinding;
    rtRef<rtFunctionCallback> m_readBinaryBinding;
};
#endif
