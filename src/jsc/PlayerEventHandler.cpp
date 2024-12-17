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

#include "PlayerEventHandler.h"
#include "PlayerEventPropsHandler.h"
/*
#include "jseventlistener.h"
#include "jsevent.h"
#include "jsutils.h"
#include "vttCue.h"
*/

static const JSClassDefinition AAMPJSEvent_object_def =
{
    0,
    kJSClassAttributeNone,
    "__Event_JS",
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};

static JSClassRef JSEvent_class_ref()
{
    static JSClassRef classDef = NULL;
    if (!classDef)
    {
        classDef = JSClassCreate(&AAMPJSEvent_object_def);
    }

    return classDef;
}

PlayerEventHandler::~PlayerEventHandler()
{
    removeAllEventListeners();
}

void PlayerEventHandler::addEventListener(AAMPEventType type, JSObjectRef callback)
{
    std::map<AAMPEventType, std::vector<EventHandlerData>>::iterator iter = mEventListeners.find(type);
    bool found = false;
    if (iter != mEventListeners.end())
    {
        std::vector<EventHandlerData>& handlers = iter->second;
	for (int i=0; i<handlers.size(); i++)
	{
	    EventHandlerData& data = handlers[i];	
            if (data.mCallback == callback)
            {
		found = true;
		break;
            }
	}
    }
    else
    {
        mEventListeners[type] = std::vector<EventHandlerData>();
	iter = mEventListeners.find(type);
        mPlayer->AddEventListener(type, this);
    }	    
    if (!found)
    {
        std::vector<EventHandlerData>& handlers = iter->second;
	EventHandlerData data(type, callback);
	handlers.push_back(data);
    }
}

void PlayerEventHandler::removeEventListener(AAMPEventType type, JSObjectRef callback)
{
    std::map<AAMPEventType, std::vector<EventHandlerData>>::iterator iter = mEventListeners.find(type);
    int index = -1;
    if (iter != mEventListeners.end())
    {
        std::vector<EventHandlerData>& handlers = iter->second;
	for (int i=0; i<handlers.size(); i++)
	{
	    EventHandlerData& data = handlers[i];	
            if (data.mCallback == callback)
            {
                index = i;
		break;
            }
	}
    }
    if (index != -1)
    {
        std::vector<EventHandlerData>& handlers = iter->second;
	handlers.erase(handlers.begin()+index);
        if (handlers.size() == 0)
	{
	    mEventListeners.erase(iter);	
            mPlayer->RemoveEventListener(type, this);
	}
    }
}

void PlayerEventHandler::removeAllEventListeners()
{
    std::map<AAMPEventType, std::vector<EventHandlerData>>::iterator iter = mEventListeners.begin();
    while (iter != mEventListeners.end())
    {	    
        mPlayer->RemoveEventListener(iter->first, this);
        std::vector<EventHandlerData>& handlers = iter->second;
        handlers.clear();
        iter = mEventListeners.erase(iter);
    }
    mEventListeners.clear();
}

void PlayerEventHandler::sendEvent(AAMPEventType type, JSObjectRef event)
{
    std::map<AAMPEventType, std::vector<EventHandlerData>>::iterator iter = mEventListeners.find(type);
    if (iter != mEventListeners.end())
    {
        std::vector<EventHandlerData>& handlers = iter->second;
	for (int i=0; i<handlers.size(); i++)
	{
	    EventHandlerData& data = handlers[i];	
            if (data.mCallback != NULL)
            {
                JSValueRef args[1] = { event };
                JSObjectCallAsFunction(mContext, data.mCallback, NULL, 1, args, NULL);
            }
	}
    }
}

void PlayerEventHandler::Event(const AAMPEventPtr& e)
{
    AAMPEventType eventType = e->getType();
    if (eventType < 0 || eventType >= AAMP_MAX_NUM_EVENTS)
    {
    	return;
    }

    JSObjectRef event = JSObjectMake(mContext, JSEvent_class_ref(), NULL);
    if (event)
    {
    	JSValueProtect(mContext, event);
        PlayerEventPropsHandler::handle(eventType, e, event, mContext);
        sendEvent(eventType, event);
    	JSValueUnprotect(mContext, event);
    }
}
