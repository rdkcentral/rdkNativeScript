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

#include "JSWebSocket.h"
#include <quickjs.h>

using namespace std;

JSWebSocket::JSWebSocket(): mTimeoutMs(30), mWSHub(new uWS::Hub(0, true)), mWs(nullptr), mHeaders(), mEventHandlers(), mUri(""), mCtx(nullptr)
{
}

void JSWebSocket::initialize(JSContext* ctx, const char* url, int timeout)
{
    mCtx = ctx;	
    mUri = url; 
    mTimeoutMs = timeout;
}

void JSWebSocket::deinitialize()
{
    mEventHandlers.clear();
/*
    if (mHeaders)
    {	  
        delete mHeaders;
    }
*/
    mHeaders.clear();
    if (mWSHub)
    {	  
        delete mWSHub;
    }
}

JSWebSocket::~JSWebSocket()
{
}

bool JSWebSocket::addListener(string eventName, JSValue& f)
{
    std::map<std::string, std::vector<JSValue>>::iterator iter = mEventHandlers.find(eventName);
    if (iter == mEventHandlers.end())
    {
        mEventHandlers[eventName] = std::vector<JSValue>();
	mEventHandlers[eventName].push_back(f);
    }
    else
    {
        iter->second.push_back(f);
    }	    
    return true;
}

bool JSWebSocket::delListener(string eventName, JSValue& f)
{
    std::map<std::string, std::vector<JSValue>>::iterator iter = mEventHandlers.find(eventName);
    if ((iter != mEventHandlers.end())/* && (iter->second == f)*/)
    {
        std::vector<JSValue>& callbacks = iter->second;
	bool deleted = false;
        int index = -1;
        for (int i=0; i<callbacks.size(); i++)
	{
            //if (callbacks[i] == f)
            //{		    
                JS_FreeValue(mCtx, callbacks[i]);
		deleted = true;
                index = i;
		break;
            //}
	}
        if (deleted)
        {
            callbacks.erase(callbacks.begin()+index);
        }		    
        if (callbacks.size() == 0)
        {
            mEventHandlers.erase(iter);
        }
    }
    return true;
}

bool JSWebSocket::clearListeners()
{
    std::map<std::string, std::vector<JSValue>>::iterator iter = mEventHandlers.end();
    for (iter = mEventHandlers.begin(); iter != mEventHandlers.end(); iter++)
    {
        std::vector<JSValue>& callbacks = iter->second;
        for (int i=0; i<callbacks.size(); i++)
	{
            JS_FreeValue(mCtx, callbacks[i]);
        }
        callbacks.clear();
    }
    mEventHandlers.clear();
    return true;
}

void JSWebSocket::sendCallback()
{
}

bool JSWebSocket::connect()
{
    // bind callback to webSocket hub
    uWS::Hub& hub = *mWSHub;
  
    hub.onConnection(
        [&](uWS::WebSocket <uWS::CLIENT>* ws, uWS::HttpRequest /*httpRequest*/)
        {
            mWs = ws;
            std::map<string, std::vector<JSValue>>::iterator iter = mEventHandlers.find("open");
            if (iter != mEventHandlers.end())
	    { 
                std::vector<JSValue>& callbacks = iter->second;
                for (int i=0; i<callbacks.size(); i++)
		{
                    JSValue ret = JS_Call(mCtx, callbacks[i], JS_UNDEFINED, 0, NULL);
                }
            }
            else
	    {
                printf("no event handlers registered for open\n");
	        fflush(stdout);
	    } 
	});
  
    hub.onMessage(
        [&](uWS::WebSocket <uWS::CLIENT>* ws, char* data, size_t length, uWS::OpCode opCode)
        {
            std::map<string, std::vector<JSValue>>::iterator iter = mEventHandlers.find("message");
            if (iter != mEventHandlers.end())
	    { 
                std::vector<JSValue>& callbacks = iter->second;
                JSValue dataObject = JS_NewObject(mCtx);
                JSValue dataValue = JS_NewStringLen(mCtx, (const char*) data, length);
                JSValue isBinary = JS_NewBool(mCtx, uWS::BINARY==opCode);
                JS_SetPropertyStr(mCtx, dataObject, "data", dataValue);
                JS_SetPropertyStr(mCtx, dataObject, "isBinary", isBinary);
                for (int i=0; i<callbacks.size(); i++)
		{
                    JSValue ret = JS_Call(mCtx, callbacks[i], JS_UNDEFINED, 1, &dataObject);
		}
                JS_FreeValue(mCtx, isBinary);
                JS_FreeValue(mCtx, dataValue);
                JS_FreeValue(mCtx, dataObject);
            }
            else
	    {
                printf("no event handlers registered for message\n");
	        fflush(stdout);
	    } 
        });
  
    hub.onError(
        [&](void*)
        {
            std::map<string, std::vector<JSValue>>::iterator iter = mEventHandlers.find("error");
            if (iter != mEventHandlers.end())
	    { 
                std::vector<JSValue>& callbacks = iter->second;
                JSValue dataObject = JS_NewObject(mCtx);
                JSValue dataValue = JS_NewString(mCtx, "connection failed");
                for (int i=0; i<callbacks.size(); i++)
		{
                    JSValue ret = JS_Call(mCtx, callbacks[i], JS_UNDEFINED, 1, &dataObject);
                }
                JS_FreeValue(mCtx, dataValue);
                JS_FreeValue(mCtx, dataObject);
            }
            else
	    {
                printf("no event handlers registered for error\n");
	        fflush(stdout);
	    } 
        });
  
    hub.onDisconnection(
        [&](uWS::WebSocket <uWS::CLIENT>* /*ws*/, int code, char* msg, size_t length)
        {
            std::map<string, std::vector<JSValue>>::iterator iter = mEventHandlers.find("close");
            if (iter != mEventHandlers.end())
	    { 
                std::vector<JSValue>& callbacks = iter->second;
                JSValue dataObject = JS_NewObject(mCtx);
                JSValue dataValue = JS_NewStringLen(mCtx, (const char*) msg, length);
                JSValue codeValue = JS_NewInt32(mCtx, code);
                JS_SetPropertyStr(mCtx, dataObject, "reason", dataValue);
                JS_SetPropertyStr(mCtx, dataObject, "code", codeValue);
                for (int i=0; i<callbacks.size(); i++)
		{
                    JSValue ret = JS_Call(mCtx, callbacks[i], JS_UNDEFINED, 1, &dataObject);
                }
                JS_FreeValue(mCtx, codeValue);
                JS_FreeValue(mCtx, dataValue);
                JS_FreeValue(mCtx, dataObject);
            }
            else
	    {
                printf("no event handlers registered for close\n");
	        fflush(stdout);
	    } 
        });
  
    printf("[%s]\n", mUri.c_str());
    hub.connect(mUri, nullptr, mHeaders);
    return true;
}

bool JSWebSocket::close()
{
    if (mWs != nullptr)
    {
        int code = 0;
        const char* message = "closed";
        mWs->close(code, message, strlen(message));
        mWs = nullptr;
    }
    return true;
}

bool JSWebSocket::send(const string& chunk)
{
    if (mWs == nullptr)
    {
        printf("webSocket still in connecting, cannot send message now.");
	fflush(stdout);
        return false;
    }
  
    //TODO only support text for now
    mWs->send(chunk.c_str(), uWS::OpCode::TEXT);
    return true;
}

void JSWebSocket::poll()
{
    if (nullptr != mWSHub)
    {
        mWSHub->poll();
    }	   
}

JSClassID js_websocket_class_id=0;
static void js_websocket_finalizer(JSRuntime *rt, JSValue val)
{
    JSWebSocket* s = (JSWebSocket*)JS_GetOpaque(val, js_websocket_class_id);
    /* Note: 's' can be NULL in case JS_SetOpaque() was not called */
    js_free_rt(rt, s);
}

static JSValue js_websocket_ctor(JSContext *ctx,
                             JSValueConst new_target,
                             int argc, JSValueConst *argv)
{
    JSWebSocket *s;
    JSValue obj = JS_UNDEFINED;
    JSValue proto, uri, timeoutMs;
    const char* urivalue;
    
    s = (JSWebSocket *) js_mallocz(ctx, sizeof(*s));
    if (!s)
        return JS_EXCEPTION;
    new (s) JSWebSocket();
    /* using new_target to get the prototype is necessary when the
       class is extended. */
    if (argc == 0)
    {
        printf("insufficient number of arguments !!");
	fflush(stdout);
        goto fail;
    }
    if (!JS_IsObject(argv[0]) && !JS_IsString(argv[0]))
    {
        printf("invalid type for constructor !!");
	fflush(stdout);
        goto fail;
    }
    if (JS_IsString(argv[0]))
    {
      urivalue = JS_ToCString(ctx, argv[0]);
      s->initialize(ctx, urivalue);
    }    
    else
    { 
      uri = JS_GetPropertyStr(ctx, argv[0], "uri");
      urivalue = JS_ToCString(ctx, uri);
      s->initialize(ctx, urivalue);
    }
    //timeoutMs = JS_GetPropertyStr(ctx, argv[0], "timeoutMs");
    
    proto = JS_GetPropertyStr(ctx, new_target, "prototype");
    if (JS_IsException(proto))
        goto fail;
    obj = JS_NewObjectProtoClass(ctx, proto, js_websocket_class_id);
    JS_FreeValue(ctx, proto);
    if (JS_IsException(obj))
        goto fail;
    JS_SetOpaque(obj, s);
    return obj;
 fail:
    if (s)
    {
        s->~JSWebSocket();
    }	    
    js_free(ctx, s);
    JS_FreeValue(ctx, obj);
    JS_FreeValue(ctx, uri);
    JS_FreeCString(ctx, urivalue);

    return JS_EXCEPTION;
}

static JSValue js_websocket_send(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSWebSocket *s = (JSWebSocket*) JS_GetOpaque2(ctx, this_val, js_websocket_class_id);
    if (!s || (argc == 0))
        return JS_EXCEPTION;
    string data = JS_ToCString(ctx, argv[0]);
    s->send(data);
    return JS_NewInt32(ctx, 1);
}

static JSValue js_websocket_connect(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSWebSocket *s = (JSWebSocket*) JS_GetOpaque2(ctx, this_val, js_websocket_class_id);
    if (!s)
        return JS_EXCEPTION;
    
    s->connect();
    return JS_NewInt32(ctx, 1);
}

static JSValue js_websocket_addlistener(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSWebSocket *s = (JSWebSocket*) JS_GetOpaque2(ctx, this_val, js_websocket_class_id);
    if (!s)
        return JS_EXCEPTION;

    string eventname = JS_ToCString(ctx, argv[0]);
    JSValue func = JS_DupValue(ctx, argv[1]);
    s->addListener(eventname, func);
    return JS_NewInt32(ctx, 1);
}

static JSValue js_websocket_dellistener(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSWebSocket *s = (JSWebSocket*) JS_GetOpaque2(ctx, this_val, js_websocket_class_id);
    if (!s)
        return JS_EXCEPTION;
    string eventname = JS_ToCString(ctx, argv[0]);
    s->delListener(eventname, argv[1]);
    return JS_NewInt32(ctx, 1);
}

static JSValue js_websocket_clearlisteners(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSWebSocket *s = (JSWebSocket*) JS_GetOpaque2(ctx, this_val, js_websocket_class_id);
    if (!s)
        return JS_EXCEPTION;
    s->clearListeners();
    return JS_NewInt32(ctx, 1);
}

static JSValue js_websocket_close(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
    JSWebSocket *s = (JSWebSocket*) JS_GetOpaque2(ctx, this_val, js_websocket_class_id);
    if (!s)
        return JS_EXCEPTION;
    s->close();
    return JS_NewInt32(ctx, 1);
}
static JSClassDef js_websocket_class = {
    "WebSocket",
    .finalizer = js_websocket_finalizer,
}; 

static const JSCFunctionListEntry js_websocket_proto_funcs[] = {
    JS_CFUNC_DEF("close", 0, js_websocket_close),
    JS_CFUNC_DEF("send", 1, js_websocket_send),
    JS_CFUNC_DEF("connect", 0, js_websocket_connect),
    JS_CFUNC_DEF("on", 2, js_websocket_addlistener),
    JS_CFUNC_DEF("delListener", 2, js_websocket_dellistener),
    JS_CFUNC_DEF("clearListeners", 0, js_websocket_clearlisteners),
};


void registerWebSocketInterface(JSContext *ctx){
    JSClassID id=0;
    js_websocket_class_id = JS_NewClassID(&id);
    auto rt = JS_GetRuntime(ctx);
    const char* className ="WebSocketInternal";

    JSClassDef  classDef;
    classDef.class_name=className;
    classDef.finalizer=[](JSRuntime* rt, JSValue val){
        auto s  = (JSWebSocket*)JS_GetOpaque(val, js_websocket_class_id);
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

    JS_NewClass(rt,js_websocket_class_id,&classDef);

    JSValue prototype =JS_NewObject(ctx);
    //adding instance methods
    JS_SetPropertyFunctionList(ctx,prototype,js_websocket_proto_funcs,6);

    auto new_class = JS_NewCFunction2(ctx,js_websocket_ctor,className,0,JS_CFUNC_constructor,0);
    auto global_obj=JS_GetGlobalObject(ctx);
    JS_DefinePropertyValueStr(ctx, global_obj, className,
                        JS_DupValue(ctx, new_class),
                        JS_PROP_WRITABLE | JS_PROP_CONFIGURABLE);
    JS_SetConstructor(ctx, new_class, prototype);
    JS_SetClassProto(ctx,js_websocket_class_id,prototype);

    JS_FreeValue(ctx, new_class);
    JS_FreeValue(ctx,global_obj);

}
