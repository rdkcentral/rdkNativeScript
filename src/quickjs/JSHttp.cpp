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

#include "JavaScriptUtils.h"
#include <string>
#include <vector>
#include <quickjs.h>
#include <iostream>
#include <string.h>

using namespace std;

#define countof(x) (sizeof(x) / sizeof((x)[0]))

struct JSHttpRequestData
{
    //store callback data
    string mUrl;
    std::vector<string> mHeaders;
    string mMethod;
    uint8_t* mWriteData;
    size_t mWriteDataSize;
    bool mInQueue;
    bool mCompress;
    string mProxy;
    bool mDelayReply;
    //rtFileDownloadRequest* mDownloadRequest;
    uint32_t mTimeout;
};

static JSClassID js_http_class_id=0;

static void js_http_finalizer(JSRuntime *rt, JSValue val)
{
    JSHttpRequestData* s = (JSHttpRequestData*)JS_GetOpaque(val, js_http_class_id);
    /* Note: 's' can be NULL in case JS_SetOpaque() was not called */
    js_free_rt(rt, s);
}

static JSValue js_http_ctor(JSContext *ctx,
                             JSValueConst new_target,
                             int argc, JSValueConst *argv)
{
    JSHttpRequestData *s;
    JSValue obj = JS_UNDEFINED;
    JSValue proto;
    
    s = (JSHttpRequestData*)js_mallocz(ctx, sizeof(*s));
    if (!s)
        return JS_EXCEPTION;
/*
    if (JS_ToInt32(ctx, &s->x, argv[0]))
        goto fail;
    if (JS_ToInt32(ctx, &s->y, argv[1]))
        goto fail;
*/
    /* using new_target to get the prototype is necessary when the
       class is extended. */
    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto))
        goto fail;
    obj = JS_NewObjectProtoClass(ctx, proto, js_http_class_id);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))
        goto fail;
    JS_SetOpaque(obj, s);
    return obj;
 fail:
    js_free(ctx, s);
    JS_FreeValue(ctx, obj);
    return JS_EXCEPTION;
}

static JSValue js_http_end(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    /*
    JSHttpRequestData *s = JS_GetOpaque2(ctx, this_val, js_http_class_id);
    if (!s)
        return JS_EXCEPTION;
    if (magic == 0)
        return JS_NewInt32(ctx, s->x);
    else
        return JS_NewInt32(ctx, s->y);
    */
    return JS_NewInt32(ctx, 1);
}
/*
static JSValue js_http_set_xy(JSContext *ctx, JSValueConst this_val, JSValue val, int magic)
{
    JSHttpRequestData *s = JS_GetOpaque2(ctx, this_val, js_http_class_id);
    int v;
    if (!s)
        return JS_EXCEPTION;
    if (JS_ToInt32(ctx, &v, val))
        return JS_EXCEPTION;
    if (magic == 0)
        s->x = v;
    else
        s->y = v;
    return JS_UNDEFINED;
}

static JSValue js_http_norm(JSContext *ctx, JSValueConst this_val,
                             int argc, JSValueConst *argv)
{
    JSHttpRequestData *s = JS_GetOpaque2(ctx, this_val, js_http_class_id);
    if (!s)
        return JS_EXCEPTION;
    return JS_NewFloat64(ctx, sqrt((double)s->x * s->x + (double)s->y * s->y));
}
*/
static JSClassDef js_http_class = {
    "Http",
    .finalizer = js_http_finalizer,
}; 

static const JSCFunctionListEntry js_http_proto_funcs[] = {
    //JS_CGETSET_MAGIC_DEF("x", js_http_get_xy, js_http_set_xy, 0),
    //JS_CGETSET_MAGIC_DEF("y", js_http_get_xy, js_http_set_xy, 1),
    //JS_CFUNC_DEF("norm", 0, js_http_norm),
    JS_CFUNC_DEF("end", 0, js_http_end),
};

static JSValue js_print(JSContext *ctx, JSValueConst this_val,
                        int argc, JSValueConst *argv)
{
    auto str = JS_ToCString(ctx, argv[0]);
    NativeJSLogger::log(INFO, "msg: %s\n", str.c_str());
    JS_FreeCString(ctx, str);
    return JS_UNDEFINED;
}

void registerHttpClass(JSContext *ctx){
    JSClassID id=0;
    js_http_class_id = JS_NewClassID(&id);
    auto rt = JS_GetRuntime(ctx);
    char* className ="Http";

    JSClassDef  classDef;
    classDef.class_name=className;
    classDef.finalizer=[](JSRuntime* rt, JSValue val){
        auto s  = (JSHttpRequestData*)JS_GetOpaque(val, js_http_class_id);
/*
        if(s!=NULL){
            free(s->type);
            js_free_rt(rt, s);
        }
*/
    };
    classDef.gc_mark=NULL;
    classDef.exotic=NULL;
    classDef.call=NULL;

    JS_NewClass(rt,js_http_class_id,&classDef);

    JSValue prototype =JS_NewObject(ctx);
    //adding instance methods
    JS_SetPropertyFunctionList(ctx,prototype,js_http_proto_funcs,1);

    auto new_class = JS_NewCFunction2(ctx,js_http_ctor,className,0,JS_CFUNC_constructor,0);
    auto global_obj=JS_GetGlobalObject(ctx);
    JS_DefinePropertyValueStr(ctx, global_obj, className,
                        JS_DupValue(ctx, new_class),
                        JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
    JS_SetConstructor(ctx, new_class, prototype);
    JS_SetClassProto(ctx,js_http_class_id,prototype);

    JS_FreeValue(ctx, new_class);
    JS_FreeValue(ctx,global_obj);

}
