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

#ifdef REMOTE_INSPECTOR_ENABLE

#include <libsoup/soup.h>
#include <JavaScriptCore/JavaScript.h>
#include <string>
#include <map>
#include <mutex>
#include <functional>

class InspectorHTTPServer {
public:
    static InspectorHTTPServer& singleton();
    ~InspectorHTTPServer();

    bool start(const char* address, int port);
    void stop();

    bool isRunning() const { return m_server != nullptr; }

    void registerContext(JSGlobalContextRef context, const char* title, const char* url);

    void unregisterContext(JSGlobalContextRef context);

    void sendConsoleMessage(JSContextRef context, const char* level, const char* text);

    void registerScript(const char* url, const char* source);

    // Called when frontend sends Page.reload.
    void setReloadCallback(std::function<void()> callback);

private:
    InspectorHTTPServer();

    static void onHTTPRequest(SoupServer* server, SoupServerMessage* msg,
                             const char* path, GHashTable* query,
                             gpointer userData);

    static void onWebSocketRequest(SoupServer* server, SoupServerMessage* msg,
                                   const char* path, SoupWebsocketConnection* connection,
                                   gpointer userData);

    static void onWebSocketMessage(SoupWebsocketConnection* connection,
                                   SoupWebsocketDataType dataType,
                                   GBytes* message, gpointer userData);

    static void onWebSocketClosed(SoupWebsocketConnection* connection, gpointer userData);

    std::string generateTargetListJSON();

    void handleCDPMessage(SoupWebsocketConnection* connection, const char* message);

    struct ContextInfo {
        JSGlobalContextRef context;
        std::string title;
        std::string url;
        uint64_t id;
    };

    struct ScriptInfo {
        std::string id;
        std::string url;
        std::string source;
    };

    SoupServer* m_server;
    int m_port;
    std::map<JSGlobalContextRef, ContextInfo> m_contexts;
    std::map<SoupWebsocketConnection*, JSGlobalContextRef> m_connections;
    std::map<std::string, ScriptInfo> m_scripts;
    std::mutex m_scriptsMutex;
    uint64_t m_nextContextId;
    uint64_t m_nextScriptId;
    std::function<void()> m_reloadCallback;
};

#endif // REMOTE_INSPECTOR_ENABLE

