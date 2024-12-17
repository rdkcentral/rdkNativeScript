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

#ifndef RT_WEB_SOCKET_CLIENT_H
#define RT_WEB_SOCKET_CLIENT_H

#include <map>
#include <uWS.h>

//#include "headers.h"
#include "rtObject.h"
#include "rtString.h"

/**
 * this rtWebSocketClient used to support webSocket in v8 engine
 * base on lib uWebSockets-0.14.8 and use uv_default_loop (compatible with v8 uv loop)
 */
class rtWebSocketClient : public rtObject
{
public:
  enum WebSocketState
  {
    CONNECTING,
    OPEN,
    CLOSING,
    CLOSED
  };

  rtDeclareObject(rtWebSocketClient, rtObject);

  /**
   * create new webSocket instance
   * @param options the webSocket options like uri, timeout,headers
   * @param loop the v8 event loop
   */
  rtWebSocketClient(uWS::WebSocket <uWS::SERVER>*ws);

  ~rtWebSocketClient();

  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);

  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);

  rtMethodNoArgAndNoReturn("close", close);

  rtMethodNoArgAndNoReturn("clearListeners", clearListeners);

  rtMethod2ArgAndNoReturn("send", send, rtString, rtObjectRef);
  rtReadOnlyProperty(readyState, readyState, int32_t);

  rtError readyState(int32_t&) const;

  /**
   * add webSocket event listener
   * @param eventName the event name
   * @param f the callback function
   */
  rtError addListener(rtString eventName, const rtFunctionRef& f);

  /**
   * del  webSocket event listener
   * @param eventName the event name
   * @param f the callback function
   */
  rtError delListener(rtString eventName, const rtFunctionRef& f);

  /**
   * close webSocket
   */
  rtError close();

  /**
   * remove all event listeners
   */
  rtError clearListeners();


  /**
   * send string type message to webSocket server
   * @param chunk  the string data
   */
  rtError send(const rtString& chunk, rtObjectRef properties);
  void sendEvent(std::string name, char* data, size_t length);

private:
  rtEmitRef mEmit;
  uWS::WebSocket <uWS::SERVER>* mWs;
  WebSocketState mReadyState;
};

#endif //RT_WEB_SOCKET_CLIENT_H
