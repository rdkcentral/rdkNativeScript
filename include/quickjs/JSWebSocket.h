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

#include "JavaScriptUtils.h"
#include <string>
#include <vector>
#include <quickjs.h>
#include <iostream>
#include <string.h>
#include <map>
#include <uWS.h>

using namespace std;

class JSWebSocket
{
    public:
        JSWebSocket();
        ~JSWebSocket();
        void initialize(JSContext* ctx, const char* url, int timeout=5000);
        void deinitialize();
        bool addListener(string eventName, JSValue& f);
        bool delListener(string eventName, JSValue& f);
        bool close();
        bool connect();
        bool clearListeners();
        bool send(const string& chunk);
        void poll();
    private:
        void sendCallback();
        int mTimeoutMs;
        uWS::Hub* mWSHub;
        uWS::WebSocket <uWS::CLIENT>* mWs;
        std::map<std::string, std::string> mHeaders;
        std::map<std::string, std::vector<JSValue>> mEventHandlers;
        std::string mUri;
        JSContext* mCtx;
};
