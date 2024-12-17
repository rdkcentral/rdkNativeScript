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
#include <JSPlayer.h>
#include <quickjs.h>

static JSClassID js_player_class_id;

static void js_player_finalizer(JSRuntime *rt, JSValue val)
{
    JSPlayerData* s = (JSPlayerData*)JS_GetOpaque(val, js_player_class_id);
    /* Note: 's' can be NULL in case JS_SetOpaque() was not called */
    if (NULL != s->mPlayer)
    {
        delete s->mPlayer;
        s->mPlayer = NULL;
    }
    js_free_rt(rt, s);
}

static JSValue js_player_ctor(JSContext *ctx,
                             JSValueConst new_target,
                             int argc, JSValueConst *argv)
{
    JSPlayerData *s;
    JSValue obj = JS_UNDEFINED;
    JSValue proto;
    
    s = (JSPlayerData*)js_mallocz(ctx, sizeof(*s));
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
    s->mPlayer = new PlayerInstanceAAMP(NULL, NULL);
    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto))
        goto fail;
    obj = JS_NewObjectProtoClass(ctx, proto, js_player_class_id);
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

static JSValue js_player_load(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    const char* url;
    url = JS_ToCString(ctx, argv[0]);
    if (!url)
        return JS_EXCEPTION;
    JSPlayerData *s = (JSPlayerData*) JS_GetOpaque2(ctx, this_val, js_player_class_id);
    if (!s || !s->mPlayer)
        return JS_EXCEPTION;
    s->mUrl = url;
    bool autoPlay = true;
    bool bFinalAttempt = false;
    bool bFirstAttempt = true;
    const char* contentType = NULL;
    const char* strTraceId = NULL;
    
    switch(argc)
    {
    	case 3:
    	{
    	    JSValue contentTypeParam = JS_GetPropertyStr(ctx, argv[2], "contentType");
    	    contentType = JS_ToCString(ctx, contentTypeParam);
    	    JSValue traceIdParam = JS_GetPropertyStr(ctx, argv[2], "traceId");
    	    strTraceId = JS_ToCString(ctx, traceIdParam);
    	    JSValue isInitialAttemptParam = JS_GetPropertyStr(ctx, argv[2], "isInitialAttempt");
    	    bFirstAttempt = JS_ToBool(ctx, isInitialAttemptParam);
    	    JSValue isFinalAttemptParam = JS_GetPropertyStr(ctx, argv[2], "isFinalAttempt");
    	    bFinalAttempt = JS_ToBool(ctx, isFinalAttemptParam);
    	}
    	case 2:
    	    autoPlay = JS_ToBool(ctx, argv[1]);
    	case 1:
    	    {
    	        s->mPlayer->Tune(url, autoPlay, contentType, bFirstAttempt, bFinalAttempt,strTraceId);
    	        break;
    	    }
    	default:
            return JS_ThrowTypeError(ctx, "Failed to execute load() <= 3 arguments required");
    }
    if (url)
    {
        JS_FreeCString(ctx, url); 
    }		
    if (contentType)
    {
        JS_FreeCString(ctx, contentType); 
    }		
    if (strTraceId)
    {
        JS_FreeCString(ctx, strTraceId); 
    }		
    return JS_NewInt32(ctx, 1);
}

static JSValue js_player_initconfig(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    if (argc != 1)
    {
        return JS_ThrowTypeError(ctx, "Failed to execute init config() = one argument required");
    }
/*
    const char* configData;
    configData = JS_ToCString(ctx, argv[0]);
    if (!configData)
        return JS_EXCEPTION;
*/
    JSValue jsonData = JS_JSONStringify(ctx, argv[0], JSValue(), JSValue());
    //handle exception
    const char* configData;
    configData = JS_ToCString(ctx, jsonData);
    if (!configData)
        return JS_EXCEPTION;
    JSPlayerData *s = (JSPlayerData*) JS_GetOpaque2(ctx, this_val, js_player_class_id);
    if (!s || !s->mPlayer)
        return JS_EXCEPTION;
    bool ret = s->mPlayer->InitAAMPConfig((char*) configData);
    if (configData)
    {
        JS_FreeCString(ctx, configData);
    }
    JS_FreeValue(ctx, jsonData);
    return JS_NewInt32(ctx, ret?1:0);
}

static JSClassDef js_player_class = {
    "AAMPMediaPlayer",
    .finalizer = js_player_finalizer,
}; 

static const JSCFunctionListEntry js_player_proto_funcs[] = {
    JS_CFUNC_DEF("load", 1, js_player_load),
    JS_CFUNC_DEF("initConfig", 1, js_player_initconfig),
};

static JSClassID playerClassId=0;

void registerPlayerInterface(JSContext *ctx)
{
    JSClassID id=0;
    playerClassId = JS_NewClassID(&id);
    auto rt = JS_GetRuntime(ctx);
    const char* className ="AAMPMediaPlayer";

    JSClassDef  classDef;
    classDef.class_name=className;
    classDef.finalizer=[](JSRuntime* rt, JSValue val)
    {
        auto s  = (JSPlayerData*)JS_GetOpaque(val, playerClassId);
        if(s!=NULL)
	{
            js_free_rt(rt, s);
        }
    };
    classDef.gc_mark=NULL;
    classDef.exotic=NULL;
    classDef.call=NULL;

    JS_NewClass(rt,playerClassId,&classDef);

    JSValue prototype =JS_NewObject(ctx);
    //adding instance methods
    JS_SetPropertyFunctionList(ctx,prototype,js_player_proto_funcs,2);

    auto new_class = JS_NewCFunction2(ctx,js_player_ctor,className,0,JS_CFUNC_constructor,0);
    auto global_obj=JS_GetGlobalObject(ctx);
    JS_DefinePropertyValueStr(ctx, global_obj, className,
                        JS_DupValue(ctx, new_class),
                        JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
    JS_SetConstructor(ctx, new_class, prototype);
    JS_SetClassProto(ctx,playerClassId,prototype);

    JS_FreeValue(ctx, new_class);
    JS_FreeValue(ctx,global_obj);
}
