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

#ifdef USE_JSCLIB_MOCK
#include "jsc_lib_mock.h"
#else
#include "jsc_lib.h"
#endif

#include <JSRuntimeServer.h>
#include <NativeJSLogger.h>
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
             NativeJSLogger::log(ERROR, "Error parsing JSON\n");
	}
    }
    JsonWrap(JsonWrap &root, const char *name)
    {
        mIsRoot = false;
        cJSON *itm = cJSON_GetObjectItem(root.get(), name);
        if (!itm || !cJSON_IsObject(itm))
        {
            NativeJSLogger::log(ERROR, "Error: %s is not an object\n", name);
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
	     NativeJSLogger::log(ERROR, "Error: %s is not a string\n", name);
	     err = true;
        }
        else
        {
            res = itm->valuestring;
            err = false;
        }
        return res;
    }

    uint32_t getUint32(const char *name, bool &err)
    {
        uint32_t res;
        cJSON *itm = cJSON_GetObjectItem(mPtr, name);
        if (!itm || !cJSON_IsNumber(itm))
        {
            std::cerr << "Error: " << name << "is not a Uint32_t" << std::endl;
            err = true;
        }
        else
        {
            res = (uint32_t) itm->valuedouble;
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

void JSRuntimeServer::initialize(int serverport, std::shared_ptr<JsRuntime::NativeJSRenderer> renderer, std::shared_ptr<IExternalApplicationHandler> externalHandler)
{
    NativeJSLogger::log(INFO, "JSRuntimeServer::initialize - port=%d, externalHandler=%p\n", serverport, externalHandler.get());

    mServerPort = serverport;
    mRenderer = renderer;
    mExternalHandler = externalHandler;
    
    NativeJSLogger::log(INFO, "JSRuntimeServer initialized - mExternalHandler=%p\n", mExternalHandler.get());
}

bool JSRuntimeServer::start()
{
    NativeJSLogger::log(INFO, "Enter: %s\n", __func__);

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
    NativeJSLogger::log(INFO, "Enter: %s\n", __func__);
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
	NativeJSLogger::log(ERROR, "Send failure: %s\n", e.what());
    }

}

void JSRuntimeServer::onMessage(websocketpp::connection_hdl hdl, message_ptr msg)
{
    std::string msgstr = msg->get_payload();
    NativeJSLogger::log(INFO, "Enter: %s : %s\n", __func__, msgstr.c_str());

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
        
        if (method == "runExternalApplication")
        {
            JsonWrap jParams(jRoot, "params");
            if (jParams.get() == nullptr)
            {
                result = "error: missing params";
                break;
            }
            
            uint32_t id = jParams.getUint32("id", error);
            if (error)
            {
                result = "error: invalid or missing id";
                break;
            }
            
            std::string url = jParams.getString("url", error);
            if (error)
            {
                result = "error: invalid or missing url";
                break;
            }
            
            if (!mExternalHandler)
            {
                result = "error: external handler not available";
                NativeJSLogger::log(ERROR, "External handler not initialized\n");
                break;
            }
            
            mExternalHandler->runExternalApplication(url, id);
            result = "ok";
            NativeJSLogger::log(INFO, "Queued external application: id=%d, url=%s\n", id, url.c_str());
        }
        else if (method == "launchApplication")
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
            std::string options = jParams.getString("moduleSettings", error);
            ModuleSettings moduleSettings;
            moduleSettings.fromString(options);
	        uint32_t id = mRenderer->createApplication(moduleSettings);
            
            NativeJSLogger::log(INFO, "launchApplication: checking URL=%s for HTML extension\n", url.c_str());
            if (url.find(".html") != std::string::npos || url.find(".htm") != std::string::npos)
            {
                NativeJSLogger::log(INFO, "Detected HTML file, mExternalHandler=%p\n", mExternalHandler.get());
                if (mExternalHandler)
                {
                    mExternalHandler->runExternalApplication(url, id);
                }
            }
            else
            {
                mRenderer->runApplication(id, url);
            }
            std::ostringstream oss;
	        oss<< "ID : " << id;
	        result = oss.str();
        }
        else if (method == "createApplication")
        {
            JsonWrap jParams(jRoot, "params");
            if (jParams.get() == nullptr)
            {
                break;
            }
	    std::string options = jParams.getString("moduleSettings", error);
            if (error)
            {
                break;
            }
            ModuleSettings moduleSettings;
            moduleSettings.fromString(options);
            uint32_t id = mRenderer->createApplication(moduleSettings);
            std::ostringstream oss;
            oss<< "ID : " << id;
            result = oss.str();
        }
        else if (method == "runApplication")
        {
            JsonWrap jParams(jRoot, "params");
            if (jParams.get() == nullptr)
            {
                break;
            }
            uint32_t id = jParams.getUint32("id", error);
	    if (error)
            {
                break;
            }
            std::string url = jParams.getString("url", error);
            if (error)
            {
                break;
            }
            mRenderer->runApplication(id, url);
            std::ostringstream oss;
            result = "ok";
        }
	
	else if (method == "runJavaScript")
	{
	    JsonWrap jParams(jRoot, "params");
	    if ( jParams.get() == nullptr)
	    {
	        break;
	    }
            uint32_t id = jParams.getUint32("id", error);
            if (error)
            {
                break;
            }
            std::string code = jParams.getString("code", error);
            if (error)
            {
                break;
            }
	    mRenderer->runJavaScript(id, code);
            result = "ok";

	}
        else if (method == "destroyApplication")
        {
            JsonWrap jParams(jRoot, "params");
            if (jParams.get() == nullptr)
            {
                break;
            }
            uint32_t id = jParams.getUint32("id", error);  
 	    if (error)
            {
                break;
            }
            mRenderer->terminateApplication(id);
            result = "ok";
        }

        else if (method == "getApplications")
        {
            std::list<JsRuntime::ApplicationDetails> apps = mRenderer->getApplications();
	    if(!apps.empty())
	    {
                std::ostringstream oss;
                for (const auto& app : apps) 
	        {
                    if (!oss.str().empty()) 
		    { 
                        oss << " ";
                    }
                    oss << "ID: " << app.id << ", URL: " << app.url;
                } 
	        result = oss.str();
	    }
	    else
		result = "No ID found";

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
    NativeJSLogger::log(INFO, "Enter: %s\n", __func__);

    std::lock_guard<std::mutex> lock(mDataMutex);
    mConnections.insert(hdl);
}

void JSRuntimeServer::onClose(websocketpp::connection_hdl hdl)
{
    NativeJSLogger::log(INFO, "Enter: %s\n", __func__);

    std::lock_guard<std::mutex> lock(mDataMutex);
    mConnections.erase(hdl);
}
