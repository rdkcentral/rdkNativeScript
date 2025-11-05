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

#ifndef __PLAYEREVENTLISTENER__H__
#define __PLAYEREVENTLISTENER__H__

#include <JavaScriptCore/JavaScript.h>
#include "main_aamp.h"
#include <map>
#include <vector>

struct EventHandlerData
{
    EventHandlerData(AAMPEventType type, JSObjectRef callback): mType(type), mCallback(callback) {}
    AAMPEventType mType;
    JSObjectRef mCallback;
};

class PlayerEventHandler : public AAMPEventObjectListener
{
    public:
	PlayerEventHandler(JSContextRef context, PlayerInstanceAAMP* player): mContext(context), mPlayer(player), mEventListeners() {}
	~PlayerEventHandler();
        void addEventListener(AAMPEventType type, JSObjectRef callback);
        void removeEventListener(AAMPEventType type, JSObjectRef callback);
        void removeAllEventListeners();
	void Event(const AAMPEventPtr& e);
#ifdef UNIT_TEST_BUILD	
	void sendEvent(AAMPEventType type, JSObjectRef event);
#endif
    private:
#ifndef UNIT_TEST_BUILD  
        void sendEvent(AAMPEventType type, JSObjectRef event);
#endif        
        JSContextRef mContext;
        PlayerInstanceAAMP* mPlayer;
        std::map<AAMPEventType, std::vector<EventHandlerData>> mEventListeners;
};

#endif /** __PLAYEREVENTLISTENER__H__ **/
