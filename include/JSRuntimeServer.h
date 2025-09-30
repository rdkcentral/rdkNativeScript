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

#pragma once
#include <NativeJSRenderer.h>

#ifdef USE_WEBSOCKET_MOCK
#include "websocketpp.hpp" 
#else
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#endif

#include <memory>
#include <mutex>
#include <set>

typedef websocketpp::server<websocketpp::config::asio> WsServer;
typedef WsServer::message_ptr message_ptr;

class JSRuntimeServer
{
public:
    static JSRuntimeServer *getInstance();
    ~JSRuntimeServer() = default;

    void initialize(int serverport, std::shared_ptr<JsRuntime::NativeJSRenderer> renderer);
    bool start();
    bool stop();

private:
    void send(websocketpp::connection_hdl hdl, const std::string &msg);
    void onMessage(websocketpp::connection_hdl hdl, message_ptr msg);
    void onOpen(websocketpp::connection_hdl hdl);
    void onClose(websocketpp::connection_hdl hdl);

private:
    typedef std::set<websocketpp::connection_hdl, std::owner_less<websocketpp::connection_hdl>> ConnectionSet;

    JSRuntimeServer();
    JSRuntimeServer(const JSRuntimeServer &) = delete;
    JSRuntimeServer &operator=(const JSRuntimeServer &) = delete;

    WsServer mServer;
    std::mutex mDataMutex;
    ConnectionSet mConnections;
    int mServerPort;
    std::shared_ptr<JsRuntime::NativeJSRenderer> mRenderer;
};
