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

#ifdef USE_WEBSOCKET_MOCK
#include "websocketpp.hpp"
#else
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/common/thread.hpp>
#include <websocketpp/client.hpp>
#endif

#include <string>
#include <condition_variable>
#include <mutex>


typedef websocketpp::client<websocketpp::config::asio_client> WsClient;
typedef websocketpp::config::asio_client::message_type::ptr message_ptr;

// From helpers/WebSockets/CommunicationInterface/CommandInterface.h
template <typename Derived>
class CommandInterface
{
public:
    CommandInterface() = default;

    bool sendCommand(std::string command, std::string &response)
    {
        Derived &derived = static_cast<Derived &>(*this);
        if (derived.send(command))
        {
            std::unique_lock<std::mutex> lock(mResponseMutex);
            mResponseCondition.wait_for(lock, std::chrono::seconds(5));
            response = mLastResponse;
            return true;
        }

        return false;
    }

protected:
    ~CommandInterface() = default;

    void onMessage(const std::string &message)
    {
        std::lock_guard<std::mutex> lock(mResponseMutex);
        mLastResponse = message;
        mResponseCondition.notify_one();
    }

private:
    CommandInterface(const CommandInterface &) = delete;
    CommandInterface &operator=(const CommandInterface &) = delete;

    std::string mLastResponse;
    std::mutex mResponseMutex;
    std::condition_variable mResponseCondition;
};


class JSRuntimeClient : public CommandInterface<JSRuntimeClient>
{
public:
    static JSRuntimeClient *getInstance();
    ~JSRuntimeClient() = default;

    void initialize(int serverport);

    bool run();
    bool send(const std::string &message);
    bool close();

    std::string getState();

private:
    void onMessage(websocketpp::connection_hdl hdl, message_ptr msg);
    void setState(const std::string &state);
    void onOpen(websocketpp::connection_hdl hdl);
    void onFail(websocketpp::connection_hdl hdl);
    void onClose(websocketpp::connection_hdl hdl);

private:
    JSRuntimeClient();
    JSRuntimeClient(const JSRuntimeClient &) = delete;
    JSRuntimeClient &operator=(const JSRuntimeClient &) = delete;

    int mServerPort;
    WsClient mEndPoint;
    websocketpp::connection_hdl mConnectionHdl;

    std::string mState;
    std::mutex mStateMutex;
    std::condition_variable mStateCondition;
};
