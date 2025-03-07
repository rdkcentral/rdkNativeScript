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
#include <JSRuntimeClient.h>
#include "jsc_lib.h"
#include <iostream>
#include <sstream>
#include <thread>

JSRuntimeClient *JSRuntimeClient::getInstance()
{
    static JSRuntimeClient instance;
    return &instance;
}

JSRuntimeClient::JSRuntimeClient() : mServerPort(0), mState("none")
{
}

void JSRuntimeClient::initialize(int serverPort)
{
    mServerPort = serverPort;
    mState = "none";

    mEndPoint.clear_access_channels(websocketpp::log::alevel::all);
    mEndPoint.clear_error_channels(websocketpp::log::elevel::all);

    mEndPoint.init_asio();

    mEndPoint.set_message_handler(std::bind(&JSRuntimeClient::onMessage, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
    mEndPoint.set_open_handler(std::bind(&JSRuntimeClient::onOpen, this, websocketpp::lib::placeholders::_1));
    mEndPoint.set_close_handler(std::bind(&JSRuntimeClient::onClose, this, websocketpp::lib::placeholders::_1));
    mEndPoint.set_fail_handler(std::bind(&JSRuntimeClient::onFail, this, websocketpp::lib::placeholders::_1));
    mEndPoint.set_open_handshake_timeout(5000);
    mEndPoint.set_close_handshake_timeout(5000);
}

bool JSRuntimeClient::run()
{
    websocketpp::lib::error_code ec;

    std::string uri = "ws://localhost:" + std::to_string(mServerPort);
    WsClient::connection_ptr con = mEndPoint.get_connection(uri, ec);
    if (ec)
    {
        std::cout << "Could not create connection because: " << ec.message() << std::endl;
        return false;
    }

    mConnectionHdl = con->get_handle();
    mEndPoint.connect(con);

    std::thread t(&WsClient::run, &mEndPoint);
    t.detach();

    std::unique_lock<std::mutex> lock(mStateMutex);
    mStateCondition.wait_for(lock, std::chrono::seconds(5));
    return mState == "open";
}

bool JSRuntimeClient::send(const std::string &message)
{
    std::cout << "Enter: " << __func__ << " : " << message << std::endl;

    if (message.empty())
    {
        std::cout << "Can't send empty message\n";
        return false;
    }

    try
    {
        mEndPoint.send(mConnectionHdl, message, websocketpp::frame::opcode::text);
    }
    catch (websocketpp::exception const &e)
    {
        std::cout << "Send failure: " << e.what() << std::endl;
    }

    return true;
}

bool JSRuntimeClient::close()
{
    std::cout << "Enter: " << __func__ << std::endl;

    websocketpp::lib::error_code ec;
    mEndPoint.close(mConnectionHdl, websocketpp::close::status::going_away, "", ec);
    return true;
}

std::string JSRuntimeClient::getState()
{
    std::lock_guard<std::mutex> lock(mStateMutex);
    return mState;
}

void JSRuntimeClient::setState(const std::string &state)
{
    std::lock_guard<std::mutex> lock(mStateMutex);
    mState = state;
    mStateCondition.notify_one();
}

void JSRuntimeClient::onMessage(websocketpp::connection_hdl hdl, message_ptr msg)
{
    std::string msgstr = msg->get_payload();
    std::cout << "Enter: " << __func__ << " : " << msgstr << std::endl;

    CommandInterface<JSRuntimeClient>::onMessage(msgstr);
}

void JSRuntimeClient::onOpen(websocketpp::connection_hdl hdl)
{
    std::cout << "Enter: " << __func__ << std::endl;
    setState("open");
}

void JSRuntimeClient::onFail(websocketpp::connection_hdl hdl)
{
    std::cout << "Enter: " << __func__ << std::endl;
    setState("fail");
}

void JSRuntimeClient::onClose(websocketpp::connection_hdl hdl)
{
    std::cout << "Enter: " << __func__ << std::endl;
    setState("close");
}

int main(int argc, char **argv)
{
    std::string command;
    std::string response;

    if (argc > 1)
    {
        std::cout << "Send input commands at ws://localhost:" << std::to_string(WS_SERVER_PORT) << std::endl;
        return -1;
    }

    JSRuntimeClient *client = JSRuntimeClient::getInstance();
    client->initialize(WS_SERVER_PORT);
    if (!client->run())
    {
        std::cout << "Unable to connect server" << std::endl;
        return -1;
    }

    while (client->getState() == "open" && std::getline(std::cin, command))
    {
        client->sendCommand(command, response);
        if (!response.empty())
        {
            std::cout << "Response: " << response << std::endl;
        }
        else
        {
            std::cout << "Missing response" << std::endl;
            break;
        }
    }

    return 0;
}
