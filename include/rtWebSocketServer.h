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

#ifndef RT_WEB_SOCKET_SERVER_H
#define RT_WEB_SOCKET_SERVER_H

#include <map>
#include <uWS.h>

//#include "headers.h"
#include "rtObject.h"
#include "rtString.h"
#include "rtWebSocketClient.h"
/**
 * this rtWebSocket used to support webSocket in v8 engine
 * base on lib uWebSockets-0.14.8 and use uv_default_loop (compatible with v8 uv loop)
 */
class rtWebSocketServer : public rtObject
{
public:
  rtDeclareObject(rtWebSocketServer, rtObject);

  /**
   * create new webSocket instance
   * @param options the webSocket options like uri, timeout,headers
   * @param loop the v8 event loop
   */
  rtWebSocketServer(const rtObjectRef& options);

  ~rtWebSocketServer();

  rtMethod2ArgAndNoReturn("on", addListener, rtString, rtFunctionRef);

  rtMethod2ArgAndNoReturn("delListener", delListener, rtString, rtFunctionRef);

  rtMethodNoArgAndNoReturn("close", close);

  rtMethodNoArgAndNoReturn("clearListeners", clearListeners);

  rtMethodNoArgAndReturn("address", address, rtObjectRef);
  rtMethod1ArgAndNoReturn("send", send, rtString);
  rtMethod1ArgAndReturn("shouldHandle", shouldHandle, rtObjectRef, bool);
  rtReadOnlyProperty(clients, clients, rtObjectRef);

  rtError clients(rtObjectRef& v) const;
  rtError address(rtObjectRef& v) const;
  rtError shouldHandle(rtObjectRef v, bool& ret) const;
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
  rtError send(const rtString& chunk);

  /* run event loop */
  void poll();

private:
  rtError init();

  rtEmitRef mEmit;
  int mTimeoutMs;
  uWS::Hub* mWSHub;
  uWS::WebSocket <uWS::SERVER>* mWs;
  std::map<std::string, std::string>* mHeaders;
  std::string mHost;
  int mPort;
  std::map<uWS::WebSocket <uWS::SERVER>*, rtWebSocketClient*> mClients;
  bool mClosed;
  bool mClientTracking;
  int mBacklog;
  unsigned int mMaxPayload;
  bool mNoServer;
  std::string mPath;
  rtObjectRef mDeflateOptions;
};

#endif //RT_WEB_SOCKET_SERVER_H
