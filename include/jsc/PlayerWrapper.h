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

#ifndef PLAYERWRAPPER_H
#define PLAYERWRAPPER_H

#include <JavaScriptCore/JavaScript.h>
#include "main_aamp.h"
#include <PlayerEventHandler.h>

struct PlayerWrapper
{
    public:
        PlayerWrapper() : ctx(), mPlayer(NULL), mPromiseCallbacks(), mPlayerEventHandler(NULL)
        {
        }
        virtual ~PlayerWrapper()
        {
        }
        PlayerWrapper(const PlayerWrapper&) = delete;
        PlayerWrapper& operator=(const PlayerWrapper&) = delete;
        void initialize(JSContextRef context);
        void terminate();
        void saveCallbackForAdId(std::string id, JSObjectRef cbObject);
        void removeCallbackForAdId(std::string id);
        void clearCallbackForAllAdIds();
        PlayerEventHandler* getPlayerEventHandler();

        JSGlobalContextRef ctx;
        PlayerInstanceAAMP* mPlayer;
    private:
        JSObjectRef getCallbackForAdId(std::string id);
        std::map<std::string, JSObjectRef> mPromiseCallbacks;
        PlayerEventHandler* mPlayerEventHandler;
};

void initializePlayer(JSGlobalContextRef context);
void deinitializePlayer(JSGlobalContextRef context);
#endif
