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

#include "jsc_lib.h"
#include <JSRuntimeServer.h>
#include <iostream>
#include <sstream>
#include <string>
#include <thread>
#include <vector>
#include <cjson/cJSON.h>

class JsonWrap
{
public:
    JsonWrap(const std::string &jsonStr)
    {
        mIsRoot = true;
        mPtr = cJSON_Parse(jsonStr.c_str());
        if (mPtr == nullptr)
        {
            std::cerr << "Error parsing JSON" << std::endl;
        }
    }
    JsonWrap(JsonWrap &root, const char *name)
    {
        mIsRoot = false;
        cJSON *itm = cJSON_GetObjectItem(root.get(), name);
        if (!itm || !cJSON_IsObject(itm))
        {
            std::cerr << "Error: " << name << "is not an object" << std::endl;
            itm = nullptr;
        }
        mPtr = itm;
    }
    ~JsonWrap()
    {
        if (mIsRoot && mPtr)
            cJSON_Delete(mPtr);
    }

    std::string getString(const char *name, bool &err)
    {
        std::string res;
        cJSON *itm = cJSON_GetObjectItem(mPtr, name);
        if (!itm || !cJSON_IsString(itm))
        {
            std::cerr << "Error: " << name << "is not a string" << std::endl;
            err = true;
        }
        else
        {
            res = itm->valuestring;
            err = false;
        }
        return res;
    }

    cJSON *get() { return mPtr; }

private:
    JsonWrap(const JsonWrap &) = delete;
    JsonWrap &operator=(const JsonWrap &) = delete;

    cJSON *mPtr;
    bool mIsRoot;
};

JSRuntimeServer *JSRuntimeServer::getInstance()
{
    static JSRuntimeServer instance;
    return &instance;
}

JSRuntimeServer::JSRuntimeServer() : mServerPort(0)
{
}

void JSRuntimeServer::initialize(int serverport, std::shared_ptr<JsRuntime::NativeJSRenderer> renderer)
{
    std::cout << "Enter: " << __func__ << std::endl;

    mServerPort = serverport;
    mRenderer = renderer;
}

bool JSRuntimeServer::start()
{
    std::cout << "Enter: " << __func__ << std::endl;

    mServer.set_access_channels(websocketpp::log::alevel::all);
    mServer.clear_access_channels(websocketpp::log::alevel::frame_payload);

    // Initialize Asio
    mServer.init_asio();

    // Register our message handler
    mServer.set_message_handler(std::bind(&JSRuntimeServer::onMessage, this, websocketpp::lib::placeholders::_1, websocketpp::lib::placeholders::_2));
    mServer.set_open_handler(std::bind(&JSRuntimeServer::onOpen, this, websocketpp::lib::placeholders::_1));
    mServer.set_close_handler(std::bind(&JSRuntimeServer::onClose, this, websocketpp::lib::placeholders::_1));

    mServer.listen(mServerPort);
    mServer.start_accept();

    std::thread t(&WsServer::run, &mServer);
    t.detach();

    return true;
}

bool JSRuntimeServer::stop()
{
    std::cout << "Enter: " << __func__ << std::endl;

    mServer.stop_listening();

    // Close all existing connections
    ConnectionSet tmp;
    {
        std::lock_guard<std::mutex> lock(mDataMutex);
        tmp = mConnections;
    }
    for (auto it = tmp.begin(); it != tmp.end(); ++it)
    {
        mServer.close(*it, websocketpp::close::status::going_away, "Server shutdown");
    }

    mRenderer->terminate();
    return true;
}

void JSRuntimeServer::send(websocketpp::connection_hdl hdl, const std::string &msg)
{
    try
    {
        mServer.send(hdl, msg, websocketpp::frame::opcode::text);
    }
    catch (websocketpp::exception const &e)
    {
        std::cout << "Send failure: " << e.what() << std::endl;
    }
}

void JSRuntimeServer::onMessage(websocketpp::connection_hdl hdl, message_ptr msg)
{
    std::string msgstr = msg->get_payload();
    std::cout << "Enter: " << __func__ << ": " << msgstr << std::endl;

    // Example input:
    // {"method": "launchApplication", "params":{"url":"/opt/www/demo/player.js", "options":"player,xhr"}}

    std::string result = "";
    bool replay = true;
    bool error = false;
    do
    {
        JsonWrap jRoot(msgstr);
        if (jRoot.get() == nullptr)
        {
            break;
        }
        std::string method = jRoot.getString("method", error);
        if (error)
        {
            break;
        }

        if (method == "launchApplication")
        {
            JsonWrap jParams(jRoot, "params");
            if (jParams.get() == nullptr)
            {
                break;
            }
            std::string url = jParams.getString("url", error);
            if (error)
            {
                break;
            }
            std::string options = jParams.getString("options", error);
            ModuleSettings settings;
            settings.fromString(options);
            mRenderer->launchApplication(url, settings);
            result = "ok";
        }
        else if (method == "terminateApplication")
        {
            JsonWrap jParams(jRoot, "params");
            if (jParams.get() == nullptr)
            {
                break;
            }
            std::string url = jParams.getString("url", error);
            if (error)
            {
                break;
            }
            mRenderer->terminateApplication(url);
            result = "ok";
        }
        else if (method == "getApplications")
        {
            std::vector<std::string> apps = mRenderer->getApplications();
            std::ostringstream oss;
            for (size_t i = 0; i < apps.size(); ++i)
            {
                if (i != 0)
                {
                    oss << ' ';
                }
                oss << apps[i];
            }
            result = oss.str();
        }
        else if (method == "ping")
        {
            result = "pong";
        }
        else if (method == "stop")
        {
            stop();
            replay = false;
        }
    } while (false);

    if (replay)
    {
        if (result.empty())
        {
            result = "error";
        }
        result = "{ \"result\":\"" + result + "\"}";
        send(hdl, result);
    }
}

void JSRuntimeServer::onOpen(websocketpp::connection_hdl hdl)
{
    std::cout << "Enter: " << __func__ << std::endl;

    std::lock_guard<std::mutex> lock(mDataMutex);
    mConnections.insert(hdl);
}

void JSRuntimeServer::onClose(websocketpp::connection_hdl hdl)
{
    std::cout << "Enter: " << __func__ << std::endl;

    std::lock_guard<std::mutex> lock(mDataMutex);
    mConnections.erase(hdl);
}
