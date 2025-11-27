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

#include <quickjs-libc.h>
#include <JavaScriptContext.h>
#include <JavaScriptEngine.h>
#include <JavaScriptUtils.h>
#ifdef ENABLE_ESSOS
#include <EssosInstance.h>
#endif
#include <string.h>
/* also used to initialize the worker context */
static JSContext *JS_NewCustomContext(JSRuntime *rt)
{
    JSContext *ctx;
    ctx = JS_NewContext(rt);
    if (!ctx)
    {
        return NULL;
    }
    js_init_module_std(ctx, "std");
    js_init_module_os(ctx, "os");
    return ctx;
}

JavaScriptContext::JavaScriptContext(JavaScriptContextFeatures& features, std::string url, IJavaScriptEngine* jsEngine):JavaScriptContextBase(features, url, jsEngine)
{
    mRuntime = ((JavaScriptEngine*)jsEngine)->getRuntime();
    js_std_set_worker_new_context_func(JS_NewCustomContext);
    mContext = JS_NewCustomContext(mRuntime);
    JS_SetContextOpaque(mContext, this);
    registerWebSocketInterface(mContext);
    registerPlayerInterface(mContext);
    #ifdef ENABLE_JSRUNTIME_THUNDER_SECURITYAGENT
    registerThunderTokenInterface(mContext, this);
    #endif
    registerRequireInterface(mContext, this);
    js_std_init_handlers(mRuntime);
    JS_SetModuleLoaderFunc(mRuntime, NULL, js_module_loader, NULL);
    js_std_add_helpers(mContext, 0, nullptr);
    const char *str = "import * as std from 'std';\n"
        "import * as os from 'os';\n"
        "globalThis.std = std;\n"
        "globalThis.os = os;\n";
    JSValue retValue = JS_Eval(mContext, str, strlen(str), "<input>", JS_EVAL_TYPE_MODULE);
    //runFile("modules/init.js", nullptr);
    //runFile("modules/timers.js", nullptr);
    runFile("modules/utils.js", nullptr);
    if (mModuleSettings.enableXHR)
    {
        runFile("modules/xhr.js", nullptr);
    }
    if(mModuleSettings.enableWebSocket)
    {
        runFile("modules/ws.js", nullptr);
    }
    if (mModuleSettings.enableWindow)
    {
        runFile("modules/window.js", nullptr/*, true*/);
        runFile("modules/windowwrapper.js", nullptr/*, true*/);
    }
    if (mModuleSettings.enableJSDOM)
    {
        runFile("modules/linkedjsdom.js", nullptr/*, true*/);
        runFile("modules/linkedjsdomwrapper.js", nullptr/*, true*/);
        runFile("modules/windowwrapper.js", nullptr/*, true*/);
    }
    registerCommonUtils();
    JavaScriptEngine* engine = (JavaScriptEngine*)mEngine;
    engine->addContext(this);
}

JavaScriptContext::~JavaScriptContext()
{
    if (mEngine)
    {
        ((JavaScriptEngine*)mEngine)->removeContext(this);
    }
    JS_FreeContext(mContext);
}

bool JavaScriptContext::evaluateScript(const char* script, const char* name, const char *args, bool module)
{
    if (nullptr != name)
    {	  
      NativeJSLogger::log(INFO, "JavaScriptContext::evaluateScript name=%s\n", name);
      fflush(stdout);
    }
    //JSValue retValue = JS_Eval(mContext, script, strlen(script), name==nullptr?"":name, JS_EVAL_TYPE_MODULE);
    JSValue retValue = JS_Eval(mContext, script, strlen(script), name==nullptr?"":name, 0);
    if (JS_IsException(retValue))
    {
	NativeJSLogger::log(ERROR, "Error evaluating script\n");
        js_std_dump_error(mContext);
        return false;
    }
    js_std_loop(mContext);
    return true;
}

void JavaScriptContext::processKeyEvent(struct JavaScriptKeyDetails& details, bool keyPress)
{
    std::string keyFunctionString = keyPress?"jsruntime.onKeyDown":"jsruntime.onKeyUp"; 
    JSValue keyFunction = JS_Eval(mContext, keyFunctionString.c_str(), keyFunctionString.length(), "", 0);
    if (JS_IsException(keyFunction))
    {
	NativeJSLogger::log(ERROR, "Error evaluating key function\n");
        js_std_dump_error(mContext);
        return;
    }
    if (JS_IsFunction(mContext, keyFunction))
    {
        JSValue dataObject = JS_NewObject(mContext);
        JSValue type = JS_NewStringLen(mContext, details.type.c_str(), details.type.length());
        JSValue key = JS_NewStringLen(mContext, details.key.c_str(), details.key.length());
        JSValue code = JS_NewStringLen(mContext, details.code.c_str(), details.code.length());
        JSValue shiftKey = JS_NewBool(mContext, details.shiftKey);
        JSValue ctrlKey = JS_NewBool(mContext, details.ctrlKey);
        JSValue altKey = JS_NewBool(mContext, details.altKey);
        JSValue metaKey = JS_NewBool(mContext, details.metaKey);
        JSValue repeat = JS_NewBool(mContext, details.repeat);
        JSValue keyCode = JS_NewInt32(mContext, details.keyCode);

        JS_SetPropertyStr(mContext, dataObject, "type", type);
        JS_SetPropertyStr(mContext, dataObject, "key", key);
        JS_SetPropertyStr(mContext, dataObject, "code", code);
        JS_SetPropertyStr(mContext, dataObject, "shiftKey", shiftKey);
        JS_SetPropertyStr(mContext, dataObject, "ctrlKey", ctrlKey);
        JS_SetPropertyStr(mContext, dataObject, "altKey", altKey);
        JS_SetPropertyStr(mContext, dataObject, "metaKey", metaKey);
        JS_SetPropertyStr(mContext, dataObject, "repeat", repeat);
        JS_SetPropertyStr(mContext, dataObject, "keyCode", keyCode);

        JSValue ret = JS_Call(mContext, keyFunction, JS_UNDEFINED, 1, &dataObject);

        JS_FreeValue(mContext, type);
        JS_FreeValue(mContext, key);
        JS_FreeValue(mContext, code);
        JS_FreeValue(mContext, shiftKey);
        JS_FreeValue(mContext, altKey);
        JS_FreeValue(mContext, ctrlKey);
        JS_FreeValue(mContext, metaKey);
        JS_FreeValue(mContext, repeat);
        JS_FreeValue(mContext, keyCode);
        //JS_FreeValue(mContext, dataObject);

        if (JS_IsException(ret))
        {
	   NativeJSLogger::log(ERROR, "received exception during key handling\n");
        }
    }
}

void JavaScriptContext::run()
{
    if (mContext)
    {
        js_std_loop(mContext);
    }
}
