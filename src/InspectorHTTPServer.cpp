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

#ifdef REMOTE_INSPECTOR_ENABLE

#include "InspectorHTTPServer.h"
#include <NativeJSLogger.h>
#include <sstream>
#include <fstream>
#include <array>
#include <cstring>
#include <ctime>
#include <unistd.h>
#include <limits.h>
#include <stdlib.h>
#include <utility>

static std::string escapeJSON(const char* str)
{
    if (!str) {
        return {};
    }

    static const std::array<const char*, 256> kEscapes = [] {
        std::array<const char*, 256> table{};
        table['"'] = "\\\"";
        table['\\'] = "\\\\";
        table['\b'] = "\\b";
        table['\f'] = "\\f";
        table['\n'] = "\\n";
        table['\r'] = "\\r";
        table['\t'] = "\\t";
        return table;
    }();
    static const char kHexDigits[] = "0123456789abcdef";

    std::string escaped;
    escaped.reserve(std::strlen(str) + 8);

    for (const unsigned char* p = reinterpret_cast<const unsigned char*>(str); *p; ++p) {
        const char* replacement = kEscapes[*p];
        if (replacement) {
            escaped.append(replacement);
            continue;
        }

        if (*p < 0x20) {
            escaped += "\\u00";
            escaped.push_back(kHexDigits[(*p >> 4) & 0x0f]);
            escaped.push_back(kHexDigits[*p & 0x0f]);
        } else {
            escaped.push_back(static_cast<char>(*p));
        }
    }

    return escaped;
}

InspectorHTTPServer& InspectorHTTPServer::singleton()
{
    static InspectorHTTPServer instance;
    return instance;
}

InspectorHTTPServer::InspectorHTTPServer()
    : m_server(nullptr)
    , m_port(0)
    , m_nextContextId(1)
    , m_nextScriptId(1)
{
}

InspectorHTTPServer::~InspectorHTTPServer()
{
    stop();
}

bool InspectorHTTPServer::start(const char* address, int port)
{
    if (m_server) {
        NativeJSLogger::log(DEBUG, "InspectorHTTPServer: Already running\n");
        return false;
    }
    
    m_server = soup_server_new(nullptr, nullptr);
    
    if (!m_server) {
        NativeJSLogger::log(ERROR, "InspectorHTTPServer: Failed to create server\n");
        return false;
    }
    
    soup_server_add_handler(m_server, "/", onHTTPRequest, this, nullptr);
    soup_server_add_handler(m_server, "/json", onHTTPRequest, this, nullptr);
    soup_server_add_handler(m_server, "/json/list", onHTTPRequest, this, nullptr);
    soup_server_add_handler(m_server, "/json/version", onHTTPRequest, this, nullptr);
    
    soup_server_add_websocket_handler(m_server, "/devtools", nullptr, nullptr,
                                      onWebSocketRequest, this, nullptr);
    
    // Bind requested port; if busy, retry next few ports.
    static const int kMaxPortTries = 10;
    int boundPort = -1;
    for (int tryPort = port; tryPort < port + kMaxPortTries; tryPort++) {
        GError* error = nullptr;
        gboolean success = soup_server_listen_all(m_server, tryPort,
                                                  static_cast<SoupServerListenOptions>(0),
                                                  &error);
        if (success) {
            boundPort = tryPort;
            break;
        }
        NativeJSLogger::log(DEBUG, "InspectorHTTPServer: Port %d busy (%s), trying %d\n",
                            tryPort, error ? error->message : "unknown", tryPort + 1);
        if (error) g_error_free(error);
    }
    
    if (boundPort < 0) {
        NativeJSLogger::log(WARN, "InspectorHTTPServer: No free port found in range %d-%d\n",
                            port, port + kMaxPortTries - 1);
        g_object_unref(m_server);
        m_server = nullptr;
        return false;
    }
    
    m_port = boundPort;
    
    NativeJSLogger::log(INFO, "InspectorHTTPServer: Started on %s:%d\n", address, boundPort);
    NativeJSLogger::log(DEBUG, "InspectorHTTPServer: Open chrome://inspect in Chrome to connect\n");
    
    return true;
}

void InspectorHTTPServer::stop()
{
    if (m_server) {
        soup_server_disconnect(m_server);
        g_object_unref(m_server);
        m_server = nullptr;
        NativeJSLogger::log(INFO, "InspectorHTTPServer: Stopped\n");
    }
    
    for (auto& pair : m_connections) {
        g_object_unref(pair.first);
    }
    
    m_contexts.clear();
    m_connections.clear();
}

void InspectorHTTPServer::registerContext(JSGlobalContextRef context, const char* title, const char* url)
{
    NativeJSLogger::log(DEBUG, "InspectorHTTPServer: registerContext called - context=%p, title=%s, url=%s\n",
                       context, title ? title : "null", url ? url : "null");
    
    if (!context) {
        NativeJSLogger::log(WARN, "InspectorHTTPServer: registerContext - context is NULL, aborting\n");
        return;
    }
    
    ContextInfo info;
    info.context = context;
    info.title = title ? title : "JavaScript Context";
    info.url = url ? url : "rdknativescript://context";
    info.id = m_nextContextId++;
    
    m_contexts[context] = info;
    
    NativeJSLogger::log(INFO, "InspectorHTTPServer: Registered context '%s' (id=%llu), total contexts=%zu\n",
                       info.title.c_str(), info.id, m_contexts.size());
}

void InspectorHTTPServer::unregisterContext(JSGlobalContextRef context)
{
    auto it = m_contexts.find(context);
    if (it != m_contexts.end()) {
        NativeJSLogger::log(INFO, "InspectorHTTPServer: Unregistered context (id=%llu)\n", it->second.id);
        m_contexts.erase(it);
    }
}

void InspectorHTTPServer::sendConsoleMessage(JSContextRef context, const char* level, const char* text)
{
    if (!context || !level || !text) return;
    
    JSGlobalContextRef globalContext = const_cast<JSGlobalContextRef>(context);
    
    SoupWebsocketConnection* targetConnection = nullptr;
    for (const auto& pair : m_connections) {
        if (pair.second == globalContext) {
            targetConnection = pair.first;
            break;
        }
    }
    
    if (!targetConnection) {
        NativeJSLogger::log(DEBUG, "InspectorHTTPServer: No WebSocket connection found for this context\n");
        return;
    }
    
    SoupWebsocketState state = soup_websocket_connection_get_state(targetConnection);
    if (state != SOUP_WEBSOCKET_STATE_OPEN) {
        NativeJSLogger::log(DEBUG, "InspectorHTTPServer: WebSocket connection is not open (state=%d)\n", state);
        return;
    }
    
    const std::string escapedText = escapeJSON(text);
    
    std::ostringstream event;
    event << "{\"method\":\"Runtime.consoleAPICalled\",\"params\":{"
          << "\"type\":\"" << level << "\","
          << "\"args\":[{\"type\":\"string\",\"value\":\"" << escapedText << "\"}],"
          << "\"executionContextId\":1,"
          << "\"timestamp\":" << (long long)(time(nullptr) * 1000) 
          << "}}";
    
    std::string eventStr = event.str();
    soup_websocket_connection_send_text(targetConnection, eventStr.c_str());
    
}

void InspectorHTTPServer::setReloadCallback(std::function<void()> callback)
{
    m_reloadCallback = std::move(callback);
}

void InspectorHTTPServer::registerScript(const char* url, const char* source)
{
    if (!url || !source) return;

    // Skip tiny internal snippets.
    if (strlen(source) < 10) return;

    std::lock_guard<std::mutex> lock(m_scriptsMutex);

    for (const auto& pair : m_scripts) {
        if (pair.second.url == url) return;
    }

    std::string scriptId = std::to_string(m_nextScriptId++);

    ScriptInfo info;
    info.id     = scriptId;
    info.url    = url;
    info.source = source;
    m_scripts[scriptId] = info;

    std::ostringstream evt;
    evt << "{\"method\":\"Debugger.scriptParsed\",\"params\":{"
        << "\"scriptId\":\"" << scriptId << "\","
        << "\"url\":\"" << escapeJSON(url) << "\","
        << "\"startLine\":0,\"startColumn\":0,"
        << "\"endLine\":0,\"endColumn\":0,"
        << "\"executionContextId\":1,\"hash\":\"\""
        << "}}";
    std::string evtStr = evt.str();

    for (const auto& pair : m_connections) {
        SoupWebsocketConnection* conn = pair.first;
        if (soup_websocket_connection_get_state(conn) == SOUP_WEBSOCKET_STATE_OPEN) {
            soup_websocket_connection_send_text(conn, evtStr.c_str());
        }
    }
}

std::string InspectorHTTPServer::generateTargetListJSON()
{
    std::ostringstream json;
    json << "[";
    
    bool first = true;
    for (const auto& pair : m_contexts) {
        const ContextInfo& info = pair.second;
        
        if (!first) json << ",";
        first = false;
        
        json << "{";
        json << "\"id\":\"" << info.id << "\",";
        json << "\"type\":\"page\",";
        json << "\"title\":\"" << info.title << "\",";
        json << "\"url\":\"" << info.url << "\",";
        json << "\"webSocketDebuggerUrl\":\"ws://127.0.0.1:" << m_port << "/devtools/page/" << info.id << "\",";
        json << "\"devtoolsFrontendUrl\":\"devtools://devtools/bundled/inspector.html?ws=127.0.0.1:" << m_port << "/devtools/page/" << info.id << "\"";
        json << "}";
    }
    
    json << "]";
    
    std::string result = json.str();
    return result;
}

void InspectorHTTPServer::onHTTPRequest(SoupServer* server, SoupServerMessage* msg,
                                       const char* path, GHashTable* query,
                                       gpointer userData)
{
    InspectorHTTPServer* self = static_cast<InspectorHTTPServer*>(userData);
    
    if (strcmp(path, "/") == 0 || strcmp(path, "/inspector") == 0) {
        const char* possiblePaths[] = {
            "src/nativevjsinspector.html",
            "../src/nativevjsinspector.html",
            "/runtime/modules/nativevjsinspector.html",
            nullptr
        };
        
        std::string htmlContent;
        bool loaded = false;
        
        char cwdBuf[1024] = {};
        getcwd(cwdBuf, sizeof(cwdBuf));
        
        for (int i = 0; possiblePaths[i] != nullptr; i++) {
            char resolvedPath[4096] = {};
            realpath(possiblePaths[i], resolvedPath);
            
            std::ifstream file(possiblePaths[i]);
            if (file.is_open()) {
                htmlContent.assign((std::istreambuf_iterator<char>(file)),
                                 std::istreambuf_iterator<char>());
                file.close();
                loaded = true;
                char absPath[4096] = {};
                realpath(possiblePaths[i], absPath);
                break;
            }
        }
        
        if (loaded) {
            SoupMessageHeaders* headers = soup_server_message_get_response_headers(msg);
            soup_message_headers_set_content_type(headers, "text/html", nullptr);
            soup_server_message_set_response(msg, "text/html", SOUP_MEMORY_COPY,
                                            htmlContent.c_str(), htmlContent.length());
            soup_server_message_set_status(msg, SOUP_STATUS_OK, nullptr);
            return;
        }
        
        NativeJSLogger::log(WARN, "InspectorHTTPServer: nativevjsinspector.html not found, using fallback\n");
        
        const char* fallbackHTML = R"HTML(<!DOCTYPE html>
<html><head><title>Inspector Error</title></head>
<body><h1>Inspector UI Not Found</h1><p>Could not load nativevjsinspector.html</p></body></html>)HTML";
        
        SoupMessageHeaders* headers = soup_server_message_get_response_headers(msg);
        soup_message_headers_set_content_type(headers, "text/html", nullptr);
        soup_server_message_set_response(msg, "text/html", SOUP_MEMORY_STATIC,
                                        fallbackHTML, strlen(fallbackHTML));
        soup_server_message_set_status(msg, SOUP_STATUS_OK, nullptr);
        return;
    }
    
    if (strcmp(path, "/json/version") == 0) {
        std::ostringstream version;
        version << "{"
                << "\"Browser\":\"RDK NativeScript/1.0\","
                << "\"Protocol-Version\":\"1.3\","
                << "\"User-Agent\":\"RDK NativeScript\","
                << "\"V8-Version\":\"JavaScriptCore\","
                << "\"WebKit-Version\":\"JavaScriptCore\","
                << "\"webSocketDebuggerUrl\":\"ws://127.0.0.1:" << self->m_port << "/devtools\""
                << "}";
        
        std::string versionStr = version.str();
        
        SoupMessageHeaders* headers = soup_server_message_get_response_headers(msg);
        soup_message_headers_set_content_type(headers, "application/json", nullptr);
        soup_message_headers_append(headers, "Access-Control-Allow-Origin", "*");
        soup_server_message_set_response(msg, "application/json", SOUP_MEMORY_COPY,
                                        versionStr.c_str(), versionStr.length());
        soup_server_message_set_status(msg, SOUP_STATUS_OK, nullptr);
        return;
    }
    
    std::string jsonResponse = self->generateTargetListJSON();
    
    SoupMessageHeaders* headers = soup_server_message_get_response_headers(msg);
    soup_message_headers_set_content_type(headers, "application/json", nullptr);
    soup_message_headers_append(headers, "Access-Control-Allow-Origin", "*");
    
    soup_server_message_set_response(msg, "application/json", SOUP_MEMORY_COPY,
                                    jsonResponse.c_str(), jsonResponse.length());
    soup_server_message_set_status(msg, SOUP_STATUS_OK, nullptr);
}

void InspectorHTTPServer::onWebSocketRequest(SoupServer* server, SoupServerMessage* msg,
                                            const char* path, SoupWebsocketConnection* connection,
                                            gpointer userData)
{
    InspectorHTTPServer* self = static_cast<InspectorHTTPServer*>(userData);
    
    uint64_t contextId = 0;
    if (sscanf(path, "/devtools/page/%llu", &contextId) == 1) {
        JSGlobalContextRef targetContext = nullptr;
        for (const auto& pair : self->m_contexts) {
            if (pair.second.id == contextId) {
                targetContext = pair.first;
                break;
            }
        }
        
        if (targetContext) {
            g_object_ref(connection);
            self->m_connections[connection] = targetContext;
            
            g_signal_connect(connection, "message", G_CALLBACK(onWebSocketMessage), userData);
            g_signal_connect(connection, "closed", G_CALLBACK(onWebSocketClosed), userData);
            
            NativeJSLogger::log(DEBUG, "InspectorHTTPServer: WebSocket connected to context %llu\n", contextId);
        } else {
            NativeJSLogger::log(WARN, "InspectorHTTPServer: Context %llu not found\n", contextId);
            soup_websocket_connection_close(connection, SOUP_WEBSOCKET_CLOSE_NORMAL, "Context not found");
        }
    } else {
        NativeJSLogger::log(WARN, "InspectorHTTPServer: Invalid WebSocket path: %s\n", path);
        soup_websocket_connection_close(connection, SOUP_WEBSOCKET_CLOSE_NORMAL, "Invalid path");
    }
}

void InspectorHTTPServer::onWebSocketMessage(SoupWebsocketConnection* connection,
                                            SoupWebsocketDataType dataType,
                                            GBytes* message, gpointer userData)
{
    InspectorHTTPServer* self = static_cast<InspectorHTTPServer*>(userData);
    
    if (dataType != SOUP_WEBSOCKET_DATA_TEXT) {
        NativeJSLogger::log(DEBUG, "InspectorHTTPServer: Received non-text WebSocket message\n");
        return;
    }
    
    gsize size;
    const char* data = static_cast<const char*>(g_bytes_get_data(message, &size));
    
    if (data && size > 0) {
        std::string messageStr(data, size);
        self->handleCDPMessage(connection, messageStr.c_str());
    }
}

void InspectorHTTPServer::onWebSocketClosed(SoupWebsocketConnection* connection, gpointer userData)
{
    InspectorHTTPServer* self = static_cast<InspectorHTTPServer*>(userData);
    
    auto it = self->m_connections.find(connection);
    if (it != self->m_connections.end()) {
        NativeJSLogger::log(DEBUG, "InspectorHTTPServer: WebSocket connection closed\n");
        self->m_connections.erase(it);
        g_object_unref(connection);
    }
}

void InspectorHTTPServer::handleCDPMessage(SoupWebsocketConnection* connection, const char* message)
{
    auto it = m_connections.find(connection);
    if (it == m_connections.end()) {
        NativeJSLogger::log(DEBUG, "InspectorHTTPServer: No context for connection\n");
        return;
    }
    
    JSGlobalContextRef context = it->second;
    
    std::string msg(message);
    
    size_t idPos = msg.find("\"id\":");
    int messageId = 1;
    if (idPos != std::string::npos) {
        sscanf(msg.c_str() + idPos + 5, "%d", &messageId);
    }
    
    std::string method;
    size_t methodPos = msg.find("\"method\":\"");
    if (methodPos != std::string::npos) {
        size_t methodStart = methodPos + 10;
        size_t methodEnd = msg.find("\"", methodStart);
        if (methodEnd != std::string::npos) {
            method = msg.substr(methodStart, methodEnd - methodStart);
        }
    }
    
    std::ostringstream response;
    
    if (method == "Runtime.enable") {
        response << "{\"id\":" << messageId << ",\"result\":{}}";
        
        std::ostringstream contextCreated;
        contextCreated << "{\"method\":\"Runtime.executionContextCreated\",\"params\":{"
                      << "\"context\":{"
                      << "\"id\":1,"
                      << "\"origin\":\"\","
                      << "\"name\":\"RDK NativeScript\""
                      << "}}}";
        std::string eventStr = contextCreated.str();
        soup_websocket_connection_send_text(connection, eventStr.c_str());
    }
    else if (method == "Debugger.enable" || 
        method == "Console.enable" || method == "Runtime.runIfWaitingForDebugger" ||
        method == "Profiler.enable" || method == "Profiler.setSamplingInterval" ||
        method == "HeapProfiler.enable" || method == "Debugger.setAsyncCallStackDepth" ||
        method == "Debugger.setPauseOnExceptions" || method == "Debugger.setBlackboxPatterns" ||
        method == "Runtime.compileScript" || method == "Page.enable" ||
        method == "Page.getResourceTree" || method == "Network.enable" ||
        method == "Log.enable" || method == "Log.startViolationsReport") {
        response << "{\"id\":" << messageId << ",\"result\":{}}";
        
        // Replay known scripts after Debugger.enable.
        if (method == "Debugger.enable") {
            std::lock_guard<std::mutex> lock(m_scriptsMutex);
            for (const auto& pair : m_scripts) {
                const ScriptInfo& info = pair.second;
                std::ostringstream scriptParsed;
                scriptParsed << "{\"method\":\"Debugger.scriptParsed\",\"params\":{"
                             << "\"scriptId\":\"" << info.id << "\","
                             << "\"url\":\"" << escapeJSON(info.url.c_str()) << "\","
                             << "\"startLine\":0,\"startColumn\":0,"
                             << "\"endLine\":0,\"endColumn\":0,"
                             << "\"executionContextId\":1,\"hash\":\"\""
                             << "}}";
                std::string evtStr = scriptParsed.str();
                soup_websocket_connection_send_text(connection, evtStr.c_str());
            }
        }
    }
    else if (method == "Runtime.evaluate") {
        // Parse expression while honoring escaped quotes.
        size_t exprPos = msg.find("\"expression\":\"");
        std::string expression;
        if (exprPos != std::string::npos) {
            size_t i = exprPos + 14; // skip past "expression":"
            while (i < msg.size()) {
                if (msg[i] == '\\' && i + 1 < msg.size()) {
                    char next = msg[i + 1];
                    switch (next) {
                        case '"':  expression += '"';  break;
                        case '\\': expression += '\\'; break;
                        case 'n':  expression += '\n'; break;
                        case 'r':  expression += '\r'; break;
                        case 't':  expression += '\t'; break;
                        default:   expression += '\\'; expression += next; break;
                    }
                    i += 2;
                } else if (msg[i] == '"') {
                    break;
                } else {
                    expression += msg[i++];
                }
            }
        }

        if (!expression.empty()) {
            // fullScript=true: evaluate script as-is.
            bool isFullScript = (msg.find("\"fullScript\":true") != std::string::npos);

            JSStringRef script = nullptr;
            if (isFullScript) {
                script = JSStringCreateWithUTF8CString(expression.c_str());
            } else {
                // Wrap expression to normalize result as a string.
                std::string evalWrapper =
                    "(function(){"
                    "  var __r = (" + expression + ");"
                    "  if (__r === null)      return 'null';"
                    "  if (__r === undefined) return 'undefined';"
                    "  if (typeof __r === 'object' || typeof __r === 'function') {"
                    "    try { return JSON.stringify(__r, null, 2); }"
                    "    catch(e) { return String(__r); }"
                    "  }"
                    "  return String(__r);"
                    "})()";
                script = JSStringCreateWithUTF8CString(evalWrapper.c_str());
            }

            JSValueRef exception = nullptr;
            JSValueRef result = JSEvaluateScript(context, script, nullptr, nullptr, 0, &exception);
            JSStringRelease(script);

            if (exception) {
                JSStringRef exStr = JSValueToStringCopy(context, exception, nullptr);
                size_t maxSize = JSStringGetMaximumUTF8CStringSize(exStr);
                char* buffer = new char[maxSize];
                JSStringGetUTF8CString(exStr, buffer, maxSize);
                JSStringRelease(exStr);

                response << "{\"id\":" << messageId
                         << ",\"result\":{\"exceptionDetails\":{\"text\":\""
                         << escapeJSON(buffer) << "\"}}}";
                delete[] buffer;
            } else if (!isFullScript && result) {
                JSStringRef resultStr = JSValueToStringCopy(context, result, nullptr);
                size_t maxSize = JSStringGetMaximumUTF8CStringSize(resultStr);
                char* buffer = new char[maxSize];
                JSStringGetUTF8CString(resultStr, buffer, maxSize);
                JSStringRelease(resultStr);

                response << "{\"id\":" << messageId
                         << ",\"result\":{\"result\":{\"type\":\"string\",\"value\":\""
                         << escapeJSON(buffer) << "\"}}}";
                delete[] buffer;
            } else {
                response << "{\"id\":" << messageId
                         << ",\"result\":{\"result\":{\"type\":\"undefined\",\"value\":\"undefined\"}}}";
            }
        } else {
            response << "{\"id\":" << messageId << ",\"result\":{}}";
        }
    }
    else if (method == "Page.reload") {
        NativeJSLogger::log(INFO, "InspectorHTTPServer: Page.reload requested\n");

        // Ack first so frontend does not time out.
        response << "{\"id\":" << messageId << ",\"result\":{}}";
        std::string ackStr = response.str();
        soup_websocket_connection_send_text(connection, ackStr.c_str());

        const char* destroyedEvt =
            "{\"method\":\"Runtime.executionContextDestroyed\","
            "\"params\":{\"executionContextId\":1}}";
        soup_websocket_connection_send_text(connection, destroyedEvt);

        {
            std::lock_guard<std::mutex> lock(m_scriptsMutex);
            m_scripts.clear();
            m_nextScriptId = 1;
        }

        if (m_reloadCallback) {
            NativeJSLogger::log(DEBUG, "InspectorHTTPServer: Invoking reload callback\n");
            m_reloadCallback();
        } else {
            NativeJSLogger::log(WARN, "InspectorHTTPServer: No reload callback registered\n");
        }

        const char* createdEvt =
            "{\"method\":\"Runtime.executionContextCreated\",\"params\":{\"context\":{"
            "\"id\":1,\"origin\":\"\",\"name\":\"RDK NativeScript\"}}}";
        soup_websocket_connection_send_text(connection, createdEvt);

        NativeJSLogger::log(INFO, "InspectorHTTPServer: Page.reload complete\n");
        return;
    }
    else if (method == "Runtime.getProperties") {
        response << "{\"id\":" << messageId << ",\"result\":{\"result\":[]}}";
    }
    else if (method == "Runtime.getIsolateId") {
        response << "{\"id\":" << messageId << ",\"result\":{\"id\":\"isolate-rdknativescript-1\"}}";
    }
    else if (method == "Runtime.getHeapUsage") {
        response << "{\"id\":" << messageId << ",\"result\":{\"usedSize\":1048576,\"totalSize\":10485760}}";
    }
    else if (method == "Inspector.readFile") {
        size_t pathPos = msg.find("\"path\":\"");
        std::string filePath;
        if (pathPos != std::string::npos) {
            size_t pathStart = pathPos + 8;
            size_t pathEnd = msg.find("\"", pathStart);
            if (pathEnd != std::string::npos) {
                filePath = msg.substr(pathStart, pathEnd - pathStart);
                
                size_t pos = 0;
                while ((pos = filePath.find("\\\\", pos)) != std::string::npos) {
                    filePath.replace(pos, 2, "\\");
                    pos += 1;
                }
            }
        }
        
        if (!filePath.empty()) {
            NativeJSLogger::log(DEBUG, "InspectorHTTPServer: Reading file: %s\n", filePath.c_str());
            
            std::ifstream file(filePath);
            if (file.is_open()) {
                std::string content((std::istreambuf_iterator<char>(file)),
                                   std::istreambuf_iterator<char>());
                file.close();
                
                std::string escapedContent = escapeJSON(content.c_str());
                std::string escapedPath = escapeJSON(filePath.c_str());
                
                response << "{\"id\":" << messageId 
                        << ",\"result\":{\"filename\":\"" << escapedPath 
                        << "\",\"content\":\"" << escapedContent << "\"}}";
            } else {
                response << "{\"id\":" << messageId 
                        << ",\"result\":{\"filename\":\"" << escapeJSON(filePath.c_str()) 
                        << "\",\"error\":\"File not found or cannot be opened\"}}";
            }
        } else {
            response << "{\"id\":" << messageId << ",\"result\":{\"error\":\"No path specified\"}}";
        }
    }
    else if (method == "Debugger.getScriptSource") {
        size_t scriptIdPos = msg.find("\"scriptId\":\"");
        std::string scriptId;
        if (scriptIdPos != std::string::npos) {
            size_t idStart = scriptIdPos + 12;
            size_t idEnd = msg.find("\"", idStart);
            if (idEnd != std::string::npos) {
                scriptId = msg.substr(idStart, idEnd - idStart);
            }
        }
        
        if (!scriptId.empty()) {
            std::lock_guard<std::mutex> lock(m_scriptsMutex);
            auto sit = m_scripts.find(scriptId);
            if (sit != m_scripts.end()) {
                response << "{\"id\":" << messageId
                         << ",\"result\":{\"scriptSource\":\""
                         << escapeJSON(sit->second.source.c_str()) << "\"}}";
            } else {
                response << "{\"id\":" << messageId
                         << ",\"result\":{\"scriptSource\":\"// Script id "
                         << scriptId << " not found\"}}";
            }
        } else {
            response << "{\"id\":" << messageId << ",\"result\":{\"scriptSource\":\"\"}}";
        }
    }
    else {
        response << "{\"id\":" << messageId << ",\"result\":{}}";
    }
    
    std::string responseStr = response.str();
    soup_websocket_connection_send_text(connection, responseStr.c_str());
    
}

#endif // REMOTE_INSPECTOR_ENABLE
