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

#include "rtWebSocketClient.h"
#include <iostream>

using std::string;

rtDefineObject(rtWebSocketClient, rtObject);

rtDefineMethod(rtWebSocketClient, addListener);
rtDefineMethod(rtWebSocketClient, close);
rtDefineMethod(rtWebSocketClient, send);
rtDefineMethod(rtWebSocketClient, delListener);
rtDefineMethod(rtWebSocketClient, clearListeners);
rtDefineProperty(rtWebSocketClient, readyState);

rtWebSocketClient::rtWebSocketClient(uWS::WebSocket <uWS::SERVER>* ws)
    : mEmit(new rtEmit()), mWs(ws), mReadyState(OPEN)
{
}

rtWebSocketClient::~rtWebSocketClient()
{
}

rtError rtWebSocketClient::addListener(rtString eventName, const rtFunctionRef& f)
{
  mEmit->addListener(eventName, f);
  return RT_OK;
}

rtError rtWebSocketClient::delListener(rtString eventName, const rtFunctionRef& f)
{
  mEmit->delListener(eventName, f);

  return RT_OK;
}

rtError rtWebSocketClient::clearListeners()
{
  mEmit->clearListeners();
  return RT_OK;
}

rtError rtWebSocketClient::close()
{
/*
  if (mWs != nullptr)
  {
    int code = 0;
    const char* message = "closed";
    mWs->close(code, message, strlen(message));
    mWs = nullptr;
  }
*/
  mReadyState = CLOSED;
  return RT_OK;
}

rtError rtWebSocketClient::send(const rtString& chunk, rtObjectRef properties)
{
  if (mWs == nullptr)
  {
    rtLogError("webSocket still in connecting, cannot send message now.");
    return RT_ERROR;
  }

  //TODO only support text for now
  bool isBinary = properties.get<bool>("binary");
  if (isBinary)
  {	  
    mWs->send(chunk.cString(), uWS::OpCode::BINARY);
  }
  else
  {
    mWs->send(chunk.cString(), uWS::OpCode::TEXT);
  }
  return RT_OK;
}

void rtWebSocketClient::sendEvent(std::string name, char* data, size_t length)
{
  rtString message(data, length);
  mEmit.send(name.c_str(), message);
}

rtError rtWebSocketClient::readyState(int32_t& v) const
{
  v = (int32_t)mReadyState;	 
  return RT_OK;
}
