/*

 pxCore Copyright 2005-2018 John Robinson

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#include "rtWebSocketServer.h"
#include "rtWebSocketClient.h"
#include <iostream>

using std::string;


rtDefineObject(rtWebSocketServer, rtObject);

rtDefineMethod(rtWebSocketServer, address);
rtDefineMethod(rtWebSocketServer, addListener);
rtDefineMethod(rtWebSocketServer, close);
rtDefineMethod(rtWebSocketServer, send);
rtDefineMethod(rtWebSocketServer, delListener);
//rtDefineMethod(rtWebSocketServer, connect);
rtDefineMethod(rtWebSocketServer, clearListeners);
rtDefineMethod(rtWebSocketServer, shouldHandle);
rtDefineProperty(rtWebSocketServer, clients);

rtWebSocketServer::rtWebSocketServer(const rtObjectRef& options)
//rtWebSocketServer::rtWebSocketServer(rtString options)
    : mEmit(new rtEmit()), mWSHub(nullptr), mWs(nullptr),
      mHeaders(new std::map<string, string>()), mClients(), mClosed(true), mBacklog(512), mMaxPayload(16777216), mNoServer(false), mPath(""), mDeflateOptions()
{
 // mUri = options.cString();
  mHost = string(options.get<rtString>("host").cString());
  mPort = options.get<int>("port");
  mBacklog = options.get<int>("backlog");
  mMaxPayload = options.get<int>("maxPayload"); 
  mDeflateOptions = options.get<rtObjectRef>("deflateOptions");
  mWSHub = new uWS::Hub(0, true, mMaxPayload);
  mHeaders->clear();
  //mHost = "0.0.0.0";
  rtObjectRef headers = options.get<rtObjectRef>("headers");
  if (headers)
  {
    rtValue allKeys;
    headers->Get("allKeys", &allKeys);
    rtArrayObject* arr = (rtArrayObject*) allKeys.toObject().getPtr();
    for (uint32_t i = 0, l = arr->length(); i < l; ++i)
    {
      rtValue key;
      if (arr->Get(i, &key) == RT_OK && !key.isEmpty())
      {
        rtString s = key.toString();
        string key(s.cString());
        string val(headers.get<rtString>(s).cString());
        mHeaders->insert(std::pair<string, string>(key, val));
      }
    } 
  }
  init();
}

rtWebSocketServer::~rtWebSocketServer()
{
  // clear all
  delete mHeaders;
  delete mWSHub;
}

rtError rtWebSocketServer::addListener(rtString eventName, const rtFunctionRef& f)
{
  mEmit->addListener(eventName, f);
  return RT_OK;
}

rtError rtWebSocketServer::delListener(rtString eventName, const rtFunctionRef& f)
{
  mEmit->delListener(eventName, f);

  return RT_OK;
}

rtError rtWebSocketServer::clearListeners()
{
  mEmit->clearListeners();
  return RT_OK;
}

rtError rtWebSocketServer::init()
{

  // bind callback to webSocket hub
  uWS::Hub& hub = *mWSHub;

  hub.onConnection(
      [&](uWS::WebSocket <uWS::SERVER>* ws, uWS::HttpRequest /*httpRequest*/)
      {
        mWs = ws;
        rtWebSocketClient* client = new rtWebSocketClient(ws);
        mClients[ws] = client;
        rtObjectRef e = new rtMapObject;
        e.set("websocket", client);
        e.set("request", 0);
        mEmit.send("connection", e);
      });


/*
  hub.onHttpUpgrade([](uWS::HttpSocket<uWS::SERVER> *s, uWS::HttpRequest req) {
    //s->terminate();
  });
*/

  hub.onMessage(
      [&](uWS::WebSocket <uWS::SERVER>* ws, char* data, size_t length, uWS::OpCode opCode)
      {
        for (int i=0; i<length; i++)
	{
          printf("%c", *(data+i));
	}
	printf("\n");
        if (mClients.find(ws) != mClients.end())
        {
          rtWebSocketClient* client = mClients[ws];
	  client->sendEvent("message", data, length);
        }	
        /*
        rtString message(data, length);
        rtObjectRef e = new rtMapObject;
        e.set("data", message);
        mEmit.send("message", e);
        */
      });

/*
    hub.onMessage([&](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode opCode) {
    });
*/
  hub.onError(
      [&](void*)
      {
        rtObjectRef e = new rtMapObject;
        e.set("message", rtValue("connection failed"));
        e.set("code", rtValue("connection failed"));
        //mEmit.send("error", rtValue("connection failed"));
        mEmit.send("error", e);
      });

  hub.onDisconnection(
      [&](uWS::WebSocket <uWS::SERVER>* /*ws*/, int code, char* msg, size_t length)
      {
        rtString errorMsg(msg, length);
        rtObjectRef e = new rtMapObject;
        e.set("code", code);
        e.set("reason", errorMsg);
        //mEmit.send("close", rtValue(code), errorMsg);
        mEmit.send("close", e);
      });

  //hub.connect(mUri, nullptr, *mHeaders);
  hub.listen(mHost.c_str(), mPort, nullptr, 0, nullptr, mBacklog);
  mClosed = false;
  mEmit.send("listening", "");
  return RT_OK;
}

rtError rtWebSocketServer::close()
{
  if (mClosed)
  {
    rtObjectRef e = new rtMapObject;
    e.set("message", rtValue("server already in closed state"));
    e.set("code", rtValue("server already in closed state"));
    mEmit.send("error", e);
    return RT_OK;
  }
  mClosed = true;
  if (mWs != nullptr)
  {
    int code = 0;
    const char* message = "closed";
    mWs->close(code, message, strlen(message));
    mWs = nullptr;
  }
  delete mWSHub;
  mWSHub = nullptr;
  mEmit.send("close", "");
  return RT_OK;
}

rtError rtWebSocketServer::send(const rtString& chunk)
{
  if (mWs == nullptr)
  {
    rtLogError("webSocket still in connecting, cannot send message now.");
    return RT_ERROR;
  }

  //TODO only support text for now
  mWs->send(chunk.cString(), uWS::OpCode::TEXT);
  return RT_OK;
}

void rtWebSocketServer::poll()
{
   if (nullptr != mWSHub)
   {
       mWSHub->poll();
   }	   
}

rtError rtWebSocketServer::address(rtObjectRef& v) const
{
    rtObjectRef e = new rtMapObject;
    e.set("port", mPort);
    e.set("address", mHost.c_str());
    v = e;
    return RT_OK;
}

rtError rtWebSocketServer::clients(rtObjectRef& v) const
{
    rtRef<rtArrayObject> array = new rtArrayObject;
    std::map<uWS::WebSocket <uWS::SERVER>*, rtWebSocketClient*>::const_iterator iter = mClients.begin();
    for(iter = mClients.begin(); iter != mClients.end(); iter++)
    {
        array->pushBack(iter->second);
    }
    v = array;
    return RT_OK;
}

rtError rtWebSocketServer::shouldHandle(rtObjectRef v, bool& ret) const
{
    ret = false;
    std::string url = string(v.get<rtString>("url").cString());
    size_t index = url.find("?");
    if (index != std::string::npos)
    {
        std::string path = url.substr(0, index);
        if (path.compare(mPath) == 0)
        {
            ret = true;
        }		
    }	    
    return RT_OK;
}
