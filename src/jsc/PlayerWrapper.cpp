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

#include <PlayerWrapper.h>
#include <NativeJSLogger.h>

struct EventTypeMap
{
    AAMPEventType eventType;
    const char* szName;
};

#define SAFE_DELETE_ARRAY(array) if(array) \
{ \
	delete[] array; \
}

static EventTypeMap aampPlayer_eventTypes[] =
{
//TODO: Need separate event list to avoid breaking existing viper impl. Unify later.
	{ (AAMPEventType)0, "onEvent"},
	{ AAMP_EVENT_TUNED, "playbackStarted"},
	{ AAMP_EVENT_TUNE_FAILED, "playbackFailed"},
	{ AAMP_EVENT_SPEED_CHANGED, "playbackSpeedChanged"},
	{ AAMP_EVENT_EOS, "playbackCompleted"},
	{ AAMP_EVENT_PLAYLIST_INDEXED, "playlistIndexed"},
	{ AAMP_EVENT_PROGRESS, "playbackProgressUpdate"},
	{ AAMP_EVENT_CC_HANDLE_RECEIVED, "decoderAvailable"},
	{ AAMP_EVENT_JS_EVENT, "jsEvent"},
	{ AAMP_EVENT_MEDIA_METADATA, "mediaMetadata"},
	{ AAMP_EVENT_ENTERING_LIVE, "enteringLive"},
	{ AAMP_EVENT_BITRATE_CHANGED, "bitrateChanged"},
	{ AAMP_EVENT_TIMED_METADATA, "timedMetadata"},
	{ AAMP_EVENT_BULK_TIMED_METADATA, "bulkTimedMetadata"},
	{ AAMP_EVENT_STATE_CHANGED, "playbackStateChanged"},
	{ AAMP_EVENT_SEEKED, "seeked"},
	{ AAMP_EVENT_BUFFERING_CHANGED, "bufferingChanged"},
	{ AAMP_EVENT_DURATION_CHANGED, "durationChanged"},
	{ AAMP_EVENT_AD_STARTED, "contentStarted"},
	{ AAMP_EVENT_DRM_METADATA, "drmMetadata"},
	{ AAMP_EVENT_REPORT_ANOMALY, "anomalyReport" },
	{ AAMP_EVENT_AD_RESERVATION_START, "reservationStart" },
	{ AAMP_EVENT_AD_RESERVATION_END, "reservationEnd" },
	{ AAMP_EVENT_AD_PLACEMENT_START, "placementStart" },
	{ AAMP_EVENT_AD_PLACEMENT_END, "placementEnd" },
	{ AAMP_EVENT_AD_PLACEMENT_ERROR, "placementError" },
	{ AAMP_EVENT_AD_PLACEMENT_PROGRESS, "placementProgress" },
	{ AAMP_EVENT_REPORT_METRICS_DATA, "metricsData" },
	{ AAMP_EVENT_BLOCKED, "blocked" },
	{ AAMP_EVENT_CONTENT_GAP, "contentGap" },
	{ AAMP_EVENT_HTTP_RESPONSE_HEADER, "httpResponseHeader"},
	{ AAMP_EVENT_WATERMARK_SESSION_UPDATE, "watermarkSessionUpdate" },
	{ (AAMPEventType)0, "" }
};

static AAMPEventType getEventTypeFromName(const char* szName)
{
        AAMPEventType eventType = AAMP_MAX_NUM_EVENTS;
        int numEvents = sizeof(aampPlayer_eventTypes) / sizeof(aampPlayer_eventTypes[0]);

        for (int i=0; i<numEvents; i++)
        {
                if (strcasecmp(aampPlayer_eventTypes[i].szName, szName) == 0)
                {
                        eventType = aampPlayer_eventTypes[i].eventType;
                        break;
                }
        }

        return eventType;
}

void PlayerWrapper::initialize(JSContextRef context)
{
    ctx = JSContextGetGlobalContext(context);
    mPlayer = new PlayerInstanceAAMP(NULL, NULL);
    mPlayerEventHandler = new PlayerEventHandler(ctx, mPlayer);
}

PlayerEventHandler* PlayerWrapper::getPlayerEventHandler()
{
    return mPlayerEventHandler;	
}

JSObjectRef PlayerWrapper::getCallbackForAdId(std::string id)
{
        std::map<std::string, JSObjectRef>::const_iterator it = mPromiseCallbacks.find(id);
        if (it != mPromiseCallbacks.end())
        {
            return it->second;
        }
        else
        {
            return NULL;
        }
}

void PlayerWrapper::removeCallbackForAdId(std::string id)
{
        std::map<std::string, JSObjectRef>::const_iterator it = mPromiseCallbacks.find(id);
        if (it != mPromiseCallbacks.end())
        {
            JSValueUnprotect(ctx, it->second);
            mPromiseCallbacks.erase(it);
        }
}

void PlayerWrapper::saveCallbackForAdId(std::string id, JSObjectRef cbObject)
{
        JSObjectRef savedObject = getCallbackForAdId(id);
        if (savedObject != NULL)
        {
                JSValueUnprotect(ctx, savedObject); //remove already saved callback
        }

        JSValueProtect(ctx, cbObject);
        mPromiseCallbacks[id] = cbObject;
}


void PlayerWrapper::clearCallbackForAllAdIds()
{
        if (!mPromiseCallbacks.empty())
        {
            for (std::map<std::string, JSObjectRef>::iterator it = mPromiseCallbacks.begin(); it != mPromiseCallbacks.end(); )
            {
                JSValueUnprotect(ctx, it->second);
                it = mPromiseCallbacks.erase(it);
            }
        }
}

void PlayerWrapper::terminate()
{
    clearCallbackForAllAdIds();
    if (NULL != mPlayerEventHandler)
    {
        delete mPlayerEventHandler;
        mPlayerEventHandler = NULL;	
    }
    if (NULL != mPlayer)
    {
        delete mPlayer;
	mPlayer = NULL;
    }
}

extern "C"
{
    JS_EXPORT JSGlobalContextRef JSContextGetGlobalContext(JSContextRef);
}

/*
JSObjectRef AAMPMediaPlayer_JS_class_constructor(JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* wrapperObj = new PlayerWrapper();

        wrapperObj->ctx = JSContextGetGlobalContext(ctx);
        wrapperObj->mPlayer = new PlayerInstanceAAMP(NULL, NULL);

        // NOTE : Association of JSObject and AAMPMediaPlayer_JS native object will be deleted only in
        // AAMPMediaPlayerJS_release ( APP initiated )  or AAMPMediaPlayer_JS_finalize ( GC initiated)
        // There is chance that aamp_UnloadJS is called then functions on aamp object is called from JS script.
        // In this case AAMPMediaPlayer_JS should be available to access
        JSObjectRef newObj = JSObjectMake(ctx, AAMPMediaPlayer_object_ref(), wrapperObj);

        // Required for viper-player
	*/
	/*
        JSStringRef fName = JSStringCreateWithUTF8CString("toString");
        JSStringRef fString = JSStringCreateWithUTF8CString("return \"[object __AAMPMediaPlayer]\";");
        JSObjectRef toStringFunc = JSObjectMakeFunction(ctx, NULL, 0, NULL, fString, NULL, 0, NULL);

        JSObjectSetProperty(ctx, newObj, fName, toStringFunc, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete, NULL);
        JSStringRelease(fName);
        JSStringRelease(fString);
        *//* 
        return newObj;
}

void AAMPMediaPlayer_JS_finalize(JSObjectRef object)
{
        PlayerWrapper *wrapperObj = (PlayerWrapper *) JSObjectGetPrivate(object);
        if (wrapperObj != NULL)
        {
                //unlink native object from JS object
                JSObjectSetPrivate(object, NULL);
                wrapperObj->terminate();
                // Delete native object
		delete wrapperObj;
        }
        else
        {
                printf("called finalize on empty object\n");
		fflush(stdout);
        }
}
*/


static JSValueRef CStringToJSValue(JSContextRef context, const char* sz)
{
        JSStringRef str = JSStringCreateWithUTF8CString(sz);
        JSValueRef value = JSValueMakeString(context, str);
        JSStringRelease(str);
        return value;
}

static JSValueRef GetException(JSContextRef context,const char *exceptionMsg)
{
        const JSValueRef arguments[] = { CStringToJSValue(context, exceptionMsg) };
        JSValueRef exception = NULL;
        JSValueRef retVal = JSObjectMakeError(context, 1, arguments, &exception);
        if (exception)
        {
                return NULL;
        }
        return retVal;
}


static char* JSValueToCString(JSContextRef context, JSValueRef value, JSValueRef* exception)
{
        JSStringRef jsstr = JSValueToStringCopy(context, value, exception);
        size_t len = JSStringGetMaximumUTF8CStringSize(jsstr);
        char* src = new char[len];
        JSStringGetUTF8CString(jsstr, src, len);
        JSStringRelease(jsstr);
        return src;
}

static char* JSValueToJSONCString(JSContextRef context, JSValueRef value, JSValueRef* exception)
{
        JSStringRef jsstr = JSValueCreateJSONString(context, value, 0, exception);
        size_t len = JSStringGetMaximumUTF8CStringSize(jsstr);
        char* src = new char[len];
        JSStringGetUTF8CString(jsstr, src, len);
        JSStringRelease(jsstr);
        return src;
}

std::vector<std::string> JSStringArrayToCStringArray(JSContextRef context, JSValueRef arrayRef)
{
    std::vector<std::string> retval;
    JSValueRef exception = NULL;

    if(!arrayRef)
    {
	NativeJSLogger::log(ERROR, "Error: array is null\n");
	fflush(stdout);
        return retval;
    }
    if (!JSValueIsObject(context, arrayRef))
    {
	NativeJSLogger::log(ERROR, "Error: value is not an object\n");
	fflush(stdout);
        return retval;
    }
    if(!JSValueIsArray(context, arrayRef))
    {
	NativeJSLogger::log(ERROR, "Error: value is not an array\n");
	fflush(stdout);
        return retval;
    }
    JSObjectRef arrayObj = JSValueToObject(context, arrayRef, &exception);
    if(exception)
    {
	NativeJSLogger::log(ERROR, "Error: exception accessing array object\n");
	fflush(stdout);
        return retval;
    }

    JSStringRef lengthStrRef = JSStringCreateWithUTF8CString("length");
    JSValueRef lengthRef = JSObjectGetProperty(context, arrayObj, lengthStrRef, &exception);
    if(exception)
    {
	NativeJSLogger::log(ERROR, "Error: exception accessing array length\n");
	fflush(stdout);
        return retval;
    }
    int length = JSValueToNumber(context, lengthRef, &exception);
    if(exception)
    {
	NativeJSLogger::log(ERROR, "Error: exception array length is not a number\n");
	fflush(stdout);
        return retval;
    }

    retval.reserve(length);
    for(int i = 0; i < length; i++)
    {
        JSValueRef strRef = JSObjectGetPropertyAtIndex(context, arrayObj, i, &exception);
        if(exception)
            continue;

        char* str = JSValueToCString(context, strRef, NULL);
        retval.push_back(str);
        if (str)
        {
            delete[] str;
        }
    }
    JSStringRelease(lengthStrRef);
    return retval;
}

JSValueRef AAMPMediaPlayerJS_initConfig (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
	if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx, "Can only call initConfig() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	
	if (argumentCount == 1 && JSValueIsObject(ctx, arguments[0]))
	{
		bool success = false;
                char* configData = JSValueToJSONCString(ctx, arguments[0], exception);
                if (configData != NULL)
                {
                    success = privObj->mPlayer->InitAAMPConfig(configData);
                }
		else
		{
		    NativeJSLogger::log(ERROR, "Failed to create JSON String\n");
		    fflush(stdout);
		}
		if (!success)
		{
		    NativeJSLogger::log(ERROR, "Failed to parse JSON string [%s]\n", configData);
		    fflush(stdout);
	        }
                if (configData)
                {
                    delete[] configData;		 
                }
	}
	else
	{
		NativeJSLogger::log(ERROR, "wrong number of arguments or type to initConfig\n");
		fflush(stdout);
		*exception = GetException(ctx, "wrong number of arguments or type to initConfig");
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_load (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
	if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx, "Can only call load() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	bool autoPlay = true;
	bool bFinalAttempt = false;
	bool bFirstAttempt = true;
	char* url = NULL;
	char* contentType = NULL;
	char* strTraceId = NULL;

	switch(argumentCount)
	{
		case 3:
		{
			JSObjectRef argument = JSValueToObject(ctx, arguments[2], NULL);
			JSStringRef paramName = JSStringCreateWithUTF8CString("contentType");
			JSValueRef paramValue = JSObjectGetProperty(ctx, argument, paramName, NULL);
			if (JSValueIsString(ctx, paramValue))
			{
		            contentType = JSValueToCString(ctx, paramValue, NULL);
			}
			JSStringRelease(paramName);
			paramName = JSStringCreateWithUTF8CString("traceId");
			paramValue = JSObjectGetProperty(ctx, argument, paramName, NULL);
			if (JSValueIsString(ctx, paramValue))
			{
			    strTraceId = JSValueToCString(ctx, paramValue, NULL);
			}
			JSStringRelease(paramName);
			paramName = JSStringCreateWithUTF8CString("isInitialAttempt");
			paramValue = JSObjectGetProperty(ctx, argument, paramName, NULL);
			if (JSValueIsBoolean(ctx, paramValue))
			{
			    bFirstAttempt = JSValueToBoolean(ctx, paramValue);
			}
			JSStringRelease(paramName);
			paramName = JSStringCreateWithUTF8CString("isFinalAttempt");
			paramValue = JSObjectGetProperty(ctx, argument, paramName, NULL);
			if (JSValueIsBoolean(ctx, paramValue))
			{
			    bFinalAttempt = JSValueToBoolean(ctx, paramValue);
			}
			JSStringRelease(paramName);
		}
		case 2:
			autoPlay = JSValueToBoolean(ctx, arguments[1]);
		case 1:
		{
			url = JSValueToCString(ctx, arguments[0], exception);
			//aamp_ApplyPageHttpHeaders(privObj->mPlayer);
			{
				char* url = JSValueToCString(ctx, arguments[0], exception);
				privObj->mPlayer->Tune(url, autoPlay, contentType, bFirstAttempt, bFinalAttempt,strTraceId);
			}
			break;
		}
		default:
			*exception = GetException(ctx, "Failed to execute load() <= 3 arguments required");

	}
        if (url)
        {
            delete[] url;		 
        }		
        if (contentType)
        {
            delete[] contentType;		 
        }		
        if (strTraceId)
        {
            delete[] strTraceId;		 
        }		
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_setVolume (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
	if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx, "Can only call setVolume() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	
	if (argumentCount == 1)
	{
		int volume = (int) JSValueToNumber(ctx, arguments[0], exception);
		if (volume >= 0)
		{
			NativeJSLogger::log(INFO, "Setting AudioVolume(%d)\n", volume);
			fflush(stdout);
			privObj->mPlayer->SetAudioVolume(volume);
		}
		else
		{
			*exception = GetException(ctx, "Volume should not be a negative number");
		}
	}
	else
	{
		*exception = GetException(ctx, "Failed to execute setVolume() - 1 argument required");
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_addEventListener (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
    if (!privObj || !privObj->mPlayer)
    {
        *exception = GetException(ctx, "Can only call addEventListener() on instances of AAMPPlayer");
        return JSValueMakeUndefined(ctx);
    }

    if (ctx != privObj->ctx)
    {
    }

    if (argumentCount >= 2)
    {
        char* type = JSValueToCString(ctx, arguments[0], NULL);
        JSObjectRef callbackObj = JSValueToObject(ctx, arguments[1], NULL);

        if ((callbackObj != NULL) && JSObjectIsFunction(ctx, callbackObj))
        {
            AAMPEventType eventType = getEventTypeFromName(type);
            if ((eventType >= 0) && (eventType < AAMP_MAX_NUM_EVENTS))
            {
                privObj->getPlayerEventHandler()->addEventListener(eventType, callbackObj);
                //privObj->mPlayer->AddEventListener(eventType, sPlayerListener);
                //AAMP_JSEventListener::AddEventListener(privObj, eventType, callbackObj);
            }
        }
        else
        {
            char errMsg[512];
            memset(errMsg, '\0', 512);
            snprintf(errMsg, 511, "Failed to execute addEventListener for %s, parameter 2 is not a function", type);
            *exception = GetException(ctx, (const char*)errMsg);
        }
        if (type)
        {
            delete[] type;
        }
    }
    else
    {
        *exception = GetException(ctx, "Failed to execute addEventListener() - 2 arguments required.");
    }
    return JSValueMakeUndefined(ctx);
}


JSValueRef AAMPMediaPlayerJS_removeEventListener (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
	if (!privObj || !privObj->mPlayer)
	{
	    *exception = GetException(ctx, "Can only call removeEventListener() on instances of AAMPPlayer");
	    return JSValueMakeUndefined(ctx);
	}

	if (argumentCount >= 2)
	{
	    char* type = JSValueToCString(ctx, arguments[0], NULL);
	    JSObjectRef callbackObj = JSValueToObject(ctx, arguments[1], NULL);

	    if ((callbackObj != NULL) && JSObjectIsFunction(ctx, callbackObj))
	    {
	    	AAMPEventType eventType = getEventTypeFromName(type);
	    	if ((eventType >= 0) && (eventType < AAMP_MAX_NUM_EVENTS))
	    	{
                    privObj->getPlayerEventHandler()->removeEventListener(eventType, callbackObj);
                    //privObj->mPlayer->RemoveEventListener(eventType, sPlayerListener);
	    	    //AAMP_JSEventListener::RemoveEventListener(privObj, eventType, callbackObj);
	    	}
	    }
	    else
	    {
	    	char errMsg[512];
	    	memset(errMsg, '\0', 512);
	    	snprintf(errMsg, 511, "Failed to execute removeEventListener() for event %s - parameter 2 is not a function", type);
	    	*exception = GetException(ctx, (const char*)errMsg);
	    }
            if (type)
            {
                delete[] type;
            }		
	}
	else
	{
	    *exception = GetException(ctx, "Failed to execute removeEventListener() - 1 argument required");
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_subscribeResponseHeaders (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
    if (!privObj || !privObj->mPlayer)
    {
            *exception = GetException(ctx, "Can only call subscribeResponseHeaders() on instances of AAMPPlayer");
            return JSValueMakeUndefined(ctx);
    }
    
    if (argumentCount != 1)
    {
            *exception = GetException(ctx, "Failed to execute subscribeResponseHeaders() - 1 argument required");
    }
    else if (!JSValueIsArray(ctx, arguments[0]))
    {
            *exception = GetException(ctx, "Failed to execute subscribeResponseHeaders() - parameter 1 is not an array");
    }
    else
    {
            std::vector<std::string> responseHeaders = JSStringArrayToCStringArray(ctx, arguments[0]);
            privObj->mPlayer->SubscribeResponseHeaders(responseHeaders);
    }
    return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_setSubscribedTags (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
    if (!privObj || !privObj->mPlayer)
    {
        *exception = GetException(ctx, "Can only call setSubscribedTags() on instances of AAMPPlayer");
        return JSValueMakeUndefined(ctx);
    }

    if (argumentCount != 1)
    {
        *exception = GetException(ctx, "Failed to execute setSubscribedTags() - 1 argument required");
    }
    else if (!JSValueIsArray(ctx, arguments[0]))
    {
        *exception = GetException(ctx, "Failed to execute setSubscribeTags() - parameter 1 is not an array");
    }
    else
    {
        std::vector<std::string> subscribedTags = JSStringArrayToCStringArray(ctx, arguments[0]);
        privObj->mPlayer->SetSubscribedTags(subscribedTags);
    }
    return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_addCustomHTTPHeader (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
            *exception = GetException(ctx, "Can only call addCustomHTTPHeader() on instances of AAMPPlayer");
            return JSValueMakeUndefined(ctx);
	}

	//optional parameter 3 for identifying if the header is for a license request
	if (argumentCount == 2 || argumentCount == 3)
	{
	    char *name = JSValueToCString(ctx, arguments[0], exception);
	    std::string headerName(name);
	    std::vector<std::string> headerVal;
	    bool isLicenseHeader = false;

            if (name)
            {
                delete[] name;
            }		

	    if (JSValueIsArray(ctx, arguments[1]))
	    {
	    	headerVal = JSStringArrayToCStringArray(ctx, arguments[1]);
	    }
	    else if (JSValueIsString(ctx, arguments[1]))
	    {
	    	headerVal.reserve(1);
	    	char *value =  JSValueToCString(ctx, arguments[1], exception);
	    	headerVal.push_back(value);
                if (value)
                {
                    delete[] value;
                }		
	    }

	    // Don't support empty values now
	    if (headerVal.size() == 0)
	    {
	    	*exception = GetException(ctx, "Failed to execute addCustomHTTPHeader() - 2nd argument should be a string or array of strings");
	    	return JSValueMakeUndefined(ctx);
	    }

	    if (argumentCount == 3)
	    {
	    	isLicenseHeader = JSValueToBoolean(ctx, arguments[2]);
	    }
	    privObj->mPlayer->AddCustomHTTPHeader(headerName, headerVal, isLicenseHeader);
	}
	else
	{
	    *exception = GetException(ctx, "Failed to execute addCustomHTTPHeader() - 2 arguments required");
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_removeCustomHTTPHeader (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
	    *exception = GetException(ctx, "Can only call removeCustomHTTPHeader() on instances of AAMPPlayer");
            return JSValueMakeUndefined(ctx);
	}

	if (argumentCount == 1)
	{
	    char *name = JSValueToCString(ctx, arguments[0], exception);
	    std::string headerName(name);
	    privObj->mPlayer->AddCustomHTTPHeader(headerName, std::vector<std::string>());
            if (name)
            {
                delete[] name;
            }		    
	}
	else
	{
	    *exception = GetException(ctx, "Failed to execute removeCustomHTTPHeader() - 1 argument required");
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_setClosedCaptionStatus(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
        {
            *exception = GetException(ctx, "Can only call setClosedCaptionStatus() on instances of AAMPPlayer");
            return JSValueMakeUndefined(ctx);
        }

        if (argumentCount != 1)
        {
            *exception = GetException(ctx, "Failed to execute setClosedCaptionStatus() - 1 argument required");
        }
        else
        {
            bool enabled = JSValueToBoolean(ctx, arguments[0]);
            privObj->mPlayer->SetCCStatus(enabled);
        }
        return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_setVideoMute (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
    if (!privObj || !privObj->mPlayer)
    {
            *exception = GetException(ctx, "Can only call setVideoMute() on instances of AAMPPlayer");
            return JSValueMakeUndefined(ctx);
    }
    
    if (argumentCount == 1)
    {
            bool videoMute = JSValueToBoolean(ctx, arguments[0]);
            privObj->mPlayer->SetVideoMute(videoMute);
    }
    else
    {
            *exception = GetException(ctx, "Failed to execute setVideoMute() - 1 argument required");
    }
    return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_play (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call play() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	{
		privObj->mPlayer->SetRate(AAMP_NORMAL_PLAY_RATE);
	}
	return JSValueMakeUndefined(ctx);
}

/**
 * @brief API invoked from JS when executing AAMPMediaPlayer.detach()
 * @param[in] ctx JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
JSValueRef AAMPMediaPlayerJS_detach (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call play() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	privObj->mPlayer->detach();

	return JSValueMakeUndefined(ctx);
}

/**
 * @brief API invoked from JS when executing AAMPMediaPlayer.pause()
 * @param[in] ctx JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
JSValueRef AAMPMediaPlayerJS_pause (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call pause() on instances of AAMPPlayer");
	}
	else
	{
		if (argumentCount == 0)
		{
			privObj->mPlayer->SetRate(0);
		}
		else if (argumentCount == 1)
		{
			double position = (double)JSValueToNumber(ctx, arguments[0], exception);
			privObj->mPlayer->PauseAt(position);
		}
		else
		{
			*exception = GetException(ctx,  "Failed to execute pause() - 0 or 1 argument required");
		}
	}
	return JSValueMakeUndefined(ctx);
}


JSValueRef AAMPMediaPlayerJS_release (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
    PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
    if (!privObj)
    {
        *exception = GetException(ctx, "Can only call release() on instances of AAMPPlayer");
        return JSValueMakeUndefined(ctx);
    }
    JSObjectSetPrivate(thisObject, NULL);
    if (NULL != privObj)
    {
	delete privObj;
    }
    return JSValueMakeUndefined(ctx);
}


/**
 * @brief API invoked from JS when executing AAMPMediaPlayer.stop()
 * @param[in] ctx JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
JSValueRef AAMPMediaPlayerJS_stop (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call stop() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	privObj->mPlayer->Stop();
	return JSValueMakeUndefined(ctx);
}


/**
 * @brief API invoked from JS when executing AAMPMediaPlayer.seek()
 * @param[in] ctx JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
JSValueRef AAMPMediaPlayerJS_seek (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call seek() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	if (argumentCount == 1 || argumentCount == 2)
	{
		double newSeekPos = JSValueToNumber(ctx, arguments[0], exception);
		bool keepPaused = (argumentCount == 2)? JSValueToBoolean(ctx, arguments[1]) : false;

		{
			privObj->mPlayer->Seek(newSeekPos, keepPaused);
		}
	}
	else
	{
		*exception = GetException(ctx,  "Failed to execute seek() - 1 or 2 arguments required");
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_getAudioTrackInfo (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
	
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call getAudioTrackInfo() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	std::string value = privObj->mPlayer->GetAudioTrackInfo();
	if (!value.empty())
	{
		return CStringToJSValue(ctx, value.c_str());
	}
	else
        {
		NativeJSLogger::log(ERROR, "GetAudioTrackInfo() value is NULL\n");
		fflush(stdout);
        }
	return JSValueMakeUndefined(ctx);
}

/**
 * @brief API invoked from JS when executing AAMPMediaPlayer.getTextTrackInfo()
 * @param[in] ctx JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
JSValueRef AAMPMediaPlayerJS_getTextTrackInfo (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call getTextTrackInfo() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	std::string value = privObj->mPlayer->GetTextTrackInfo();
	if (!value.empty())
	{
		return CStringToJSValue(ctx, value.c_str());
	}
	else
	{
		NativeJSLogger::log(ERROR, "GetTextTrackInfo() value is NULL\n");
		fflush(stdout);
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_getDurationSec (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call getDurationSec() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	double duration = 0;
	duration = privObj->mPlayer->GetPlaybackDuration();
	if (duration < 0)
	{
		duration = 0;
	}

	return JSValueMakeNumber(ctx, duration);
}


/**
 * @brief API invoked from JS when executing AAMPMediaPlayer.getCurrentPosition()
 * @param[in] ctx JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
JSValueRef AAMPMediaPlayerJS_getCurrentPosition (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call getCurrentPosition() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	double currPosition = 0;
	currPosition = privObj->mPlayer->GetPlaybackPosition();
	if (currPosition < 0)
	{
		currPosition = 0;
	}

	return JSValueMakeNumber(ctx, currPosition);
}

JSValueRef AAMPMediaPlayerJS_getAudioTrack (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call getAudioTrack() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	return JSValueMakeNumber(ctx, privObj->mPlayer->GetAudioTrack());
}

JSValueRef AAMPMediaPlayerJS_getTextTrack (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call getTextTrack() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	return JSValueMakeNumber(ctx, privObj->mPlayer->GetTextTrack());
}

JSValueRef AAMPMediaPlayerJS_getVolume (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call getVolume() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	
	return JSValueMakeNumber(ctx, privObj->mPlayer->GetAudioVolume());
}

JSValueRef AAMPMediaPlayerJS_setAudioLanguage (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call setAudioLanguage() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}

	if (argumentCount == 1)
	{
		const char *lang = JSValueToCString(ctx, arguments[0], exception);
		{
			privObj->mPlayer->SetLanguage(lang);
		}
		SAFE_DELETE_ARRAY(lang);
	}
	else
	{
		*exception = GetException(ctx,  "Failed to execute setAudioLanguage() - 1 argument required");
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_setPlaybackRate (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call setPlaybackRate() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}

	if (argumentCount == 1 || argumentCount == 2)
	{
		int overshootCorrection = 0;
		float rate = (float) JSValueToNumber(ctx, arguments[0], exception);
		if (argumentCount == 2)
		{
			overshootCorrection = (int) JSValueToNumber(ctx, arguments[1], exception);
		}
		{
			privObj->mPlayer->SetRate(rate, overshootCorrection);
		}
	}
	else
	{
		*exception = GetException(ctx,  "Failed to execute setPlaybackRate() - atleast 1 argument required");
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_setVideoRect (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call setVideoRect() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}

	if (argumentCount == 4)
	{
		int x = (int) JSValueToNumber(ctx, arguments[0], exception);
		int y = (int) JSValueToNumber(ctx, arguments[1], exception);
		int w = (int) JSValueToNumber(ctx, arguments[2], exception);
		int h = (int) JSValueToNumber(ctx, arguments[3], exception);
		{
			privObj->mPlayer->SetVideoRectangle(x, y, w, h);
		}
	}
	else
	{
		*exception = GetException(ctx,  "Failed to execute setVideoRect() - 1 argument required");
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_setVideoZoom (JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call setVideoZoom() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}

	if (argumentCount == 1)
	{
		VideoZoomMode zoom;
		char* zoomStr = JSValueToCString(ctx, arguments[0], exception);
		if (0 == strcmp(zoomStr, "none"))
		{
			zoom = VIDEO_ZOOM_NONE;
		}
		else
		{
			zoom = VIDEO_ZOOM_FULL;
		}
		privObj->mPlayer->SetVideoZoom(zoom);
		SAFE_DELETE_ARRAY(zoomStr);
	}
	else
	{
		*exception = GetException(ctx,  "Failed to execute setVideoZoom() - 1 argument required");
	}
	return JSValueMakeUndefined(ctx);
}

#ifdef UNIT_TEST_BUILD
JSValueRef AAMPMediaPlayerJS_getAvailableAudioTracks(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
#else
static JSValueRef AAMPMediaPlayerJS_getAvailableAudioTracks(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
#endif
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call getAvailableAudioTracks() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	bool allTrack = false;
	if (argumentCount == 1)
	{
		allTrack = JSValueToBoolean(ctx, arguments[0]);
	}
	std::string tracks = privObj->mPlayer->GetAvailableAudioTracks(allTrack);
	if (!tracks.empty())
	{
		return CStringToJSValue(ctx, tracks.c_str());
	}
	else
	{
		return JSValueMakeUndefined(ctx);
	}
}


/**
 * @brief API invoked from JS when executing AAMPMediaPlayer.getAvailableTextTracks()
 * @param[in] ctx JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
 #ifdef UNIT_TEST_BUILD
JSValueRef AAMPMediaPlayerJS_getAvailableTextTracks(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
#else
static JSValueRef AAMPMediaPlayerJS_getAvailableTextTracks(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
#endif
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call getAvailableTextTracks() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	bool allTrack = false;
	if (argumentCount == 1)
	{
		allTrack = JSValueToBoolean(ctx, arguments[0]);
	}
	std::string tracks = privObj->mPlayer->GetAvailableTextTracks(allTrack);
	if (!tracks.empty())
	{
		return CStringToJSValue(ctx, tracks.c_str());
	}
	else
	{
		return JSValueMakeUndefined(ctx);
	}
}


/**
 * @brief API invoked from JS when executing AAMPMediaPlayer.getVideoRectangle()
 * @param[in] ctx JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
#ifdef UNIT_TEST_BUILD
JSValueRef AAMPMediaPlayerJS_getVideoRectangle(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
#else
static JSValueRef AAMPMediaPlayerJS_getVideoRectangle(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
#endif
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call getVideoRectangle() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	
	std::string strRect = privObj->mPlayer->GetVideoRectangle();
	return CStringToJSValue(ctx, strRect.c_str());
}

/**	
 * @brief API invoked from JS when executing AAMPMediaPlayer.setAlternateContent()	
 * @param[in] ctx JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
#ifdef UNIT_TEST_BUILD
JSValueRef AAMPMediaPlayerJS_setAlternateContent(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)	
#else
static JSValueRef AAMPMediaPlayerJS_setAlternateContent(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
#endif	
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call setAlternateContent() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	
	if (argumentCount == 2)
	{
		/*
		 * Parmater format
		 * "reservationObject": object {
		 *   "reservationId": "773701056",
		 *    "reservationBehavior": number
		 *    "placementRequest": {
		 *      "id": string,
		 *      "pts": number,
		 *      "url": "",
		 *    },
		 * },
		 * "promiseCallback": function
		 */
		char *reservationId = NULL;
		int reservationBehavior = -1;
		char *adId = NULL;
		long adPTS = -1;
		char *adURL = NULL;
		if (JSValueIsObject(ctx, arguments[0]))
		{
			//Parse the ad object
			JSObjectRef reservationObject = JSValueToObject(ctx, arguments[0], NULL);
			if (reservationObject == NULL)
			{
				return JSValueMakeUndefined(ctx);
			}
			JSStringRef propName = JSStringCreateWithUTF8CString("reservationId");
			JSValueRef propValue = JSObjectGetProperty(ctx, reservationObject, propName, NULL);
			if (JSValueIsString(ctx, propValue))
			{
				reservationId = JSValueToCString(ctx, propValue, NULL);
			}
	
			JSStringRelease(propName);
			propName = JSStringCreateWithUTF8CString("reservationBehavior");
			propValue = JSObjectGetProperty(ctx, reservationObject, propName, NULL);
			if (JSValueIsNumber(ctx, propValue))
			{
				reservationBehavior = JSValueToNumber(ctx, propValue, NULL);
			}
			JSStringRelease(propName);
	
			propName = JSStringCreateWithUTF8CString("placementRequest");
			propValue = JSObjectGetProperty(ctx, reservationObject, propName, NULL);
			if (JSValueIsObject(ctx, propValue))
			{
				JSObjectRef adObject = JSValueToObject(ctx, propValue, NULL);
				JSStringRef adPropName = JSStringCreateWithUTF8CString("id");
				JSValueRef adPropValue = JSObjectGetProperty(ctx, adObject, adPropName, NULL);
				if (JSValueIsString(ctx, adPropValue))
				{
					adId = JSValueToCString(ctx, adPropValue, NULL);
				}
				JSStringRelease(adPropName);
				adPropName = JSStringCreateWithUTF8CString("pts");
				adPropValue = JSObjectGetProperty(ctx, adObject, adPropName, NULL);
				if (JSValueIsNumber(ctx, adPropValue))
				{
					adPTS = (long) JSValueToNumber(ctx, adPropValue, NULL);
				}
				JSStringRelease(adPropName);
				adPropName = JSStringCreateWithUTF8CString("url");
				adPropValue = JSObjectGetProperty(ctx, adObject, adPropName, NULL);
				if (JSValueIsString(ctx, adPropValue))
				{
					adURL = JSValueToCString(ctx, adPropValue, NULL);
				}
				JSStringRelease(adPropName);
			}
			JSStringRelease(propName);
		}
	
		JSObjectRef callbackObj = JSValueToObject(ctx, arguments[1], NULL);
		if ((callbackObj) && JSObjectIsFunction(ctx, callbackObj) && adId && reservationId && adURL)
		{
			std::string adIdStr(adId);  //CID:115002 - Resolve Forward null
			std::string adBreakId(reservationId);  //CID:115001 - Resolve Forward null
			std::string url(adURL);  //CID:115000 - Resolve Forward null

			privObj->saveCallbackForAdId(adIdStr, callbackObj); //save callback for sending status later, if ad can be played or not
			privObj->mPlayer->SetAlternateContents(adBreakId, adIdStr, url);
		}
		else
		{
			NativeJSLogger::log(ERROR, "Unable to parse the promiseCallback argument\n");
			fflush(stdout);
		}
	
		SAFE_DELETE_ARRAY(reservationId);
		SAFE_DELETE_ARRAY(adURL);
		SAFE_DELETE_ARRAY(adId);
	}
	else
	{
		*exception = GetException(ctx,  "Failed to execute setAlternateContent() - 2 argument required");
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_setPreferredAudioLanguage(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call setPreferredAudioLanguage() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}

	if( argumentCount>=1 && argumentCount<=5)
	{
		char* lanList = JSValueToCString(ctx,arguments[0], NULL);
		char *rendition = NULL;
		char *type = NULL;
		char *codecList = NULL;
		char *labelList=NULL;
		if(argumentCount >= 2) {
			rendition = JSValueToCString(ctx,arguments[1], NULL);
		}
		if(argumentCount >= 3) {
			type = JSValueToCString(ctx,arguments[2], NULL);
		}
		if(argumentCount >= 4) {
			codecList = JSValueToCString(ctx,arguments[3], NULL);
		}
		if(argumentCount >= 5) {
			labelList = JSValueToCString(ctx,arguments[4], NULL);
		}
		privObj->mPlayer->SetPreferredLanguages(lanList, rendition, type, codecList, labelList);
		SAFE_DELETE_ARRAY(type);
		SAFE_DELETE_ARRAY(rendition);
		SAFE_DELETE_ARRAY(lanList);
		SAFE_DELETE_ARRAY(codecList);
		SAFE_DELETE_ARRAY(labelList);
	}
	else
	{
		*exception = GetException(ctx,  "Failed to execute setPreferredAudioLanguage() - 1, 2 or 3 arguments required");
	}
	return JSValueMakeUndefined(ctx);
}

/**
 * @brief API invoked from JS when executing AAMPMediaPlayer.setPreferredAudioLanguage()
 * @param[in] ctx JS execution context
 * @param[in] function JSObject that is the function being called
 * @param[in] thisObject JSObject that is the 'this' variable in the function's scope
 * @param[in] argumentCount number of args
 * @param[in] arguments[] JSValue array of args
 * @param[out] exception pointer to a JSValueRef in which to return an exception, if any
 * @retval JSValue that is the function's return value
 */
JSValueRef AAMPMediaPlayerJS_setPreferredTextLanguage(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call setPreferredAudioLanguage() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}

	if( argumentCount==1)
	{
		char* parameters = JSValueToCString(ctx,arguments[0], NULL);
		privObj->mPlayer->SetPreferredTextLanguages(parameters);
		SAFE_DELETE_ARRAY(parameters);
	}
	else
	{
		*exception = GetException(ctx,  "Failed to execute setPreferredAudioLanguage() - 1 argument required");
	}
	return JSValueMakeUndefined(ctx);
}

#ifdef UNIT_TEST_BUILD
JSValueRef AAMPMediaPlayerJS_notifyReservationCompletion(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
#else
static JSValueRef AAMPMediaPlayerJS_notifyReservationCompletion(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
#endif
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call notifyReservationCompletion() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	
	if (argumentCount == 2)
	{
		const char * reservationId = JSValueToCString(ctx, arguments[0], exception);
		long time = (long) JSValueToNumber(ctx, arguments[1], exception);
		//Need an API in AAMP to notify that placements for this reservation are over and AAMP might have to trim
		//the ads to the period duration or not depending on time param
		SAFE_DELETE_ARRAY(reservationId);
	}
	else
	{
		*exception = GetException(ctx,  "Failed to execute notifyReservationCompletion() - 2 argument required");
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_setTextStyleOptions(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call setTextStyleOptions() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}

	if (argumentCount != 1)
	{
		*exception = GetException(ctx,  "Failed to execute setTextStyleOptions() - 1 argument required");
	}
	else
	{
		if (JSValueIsString(ctx, arguments[0]))
		{
			const char *options = JSValueToCString(ctx, arguments[0], NULL);
			privObj->mPlayer->SetTextStyle(std::string(options));
			SAFE_DELETE_ARRAY(options);
		}
		else
		{
			*exception = GetException(ctx,  "Argument should be a JSON formatted string!");
		}
	}
	return JSValueMakeUndefined(ctx);
}

#ifdef UNIT_TEST_BUILD
JSValueRef AAMPMediaPlayerJS_setAuxiliaryLanguage(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
#else
static JSValueRef AAMPMediaPlayerJS_setAuxiliaryLanguage(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
#endif
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call setAuxiliaryLanguage() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}

	if (argumentCount == 1)
	{
		const char *lang = JSValueToCString(ctx, arguments[0], NULL);
		privObj->mPlayer->SetAuxiliaryLanguage(std::string(lang));
		SAFE_DELETE_ARRAY(lang);
	}
	else
	{
		*exception = GetException(ctx,  "Failed to execute setAuxiliaryLanguage() - 1 argument required");
	}
	return JSValueMakeUndefined(ctx);
}

JSValueRef AAMPMediaPlayerJS_xreSupportedTune(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* privObj = (PlayerWrapper*)JSObjectGetPrivate(thisObject);
        if (!privObj || !privObj->mPlayer)
	{
		*exception = GetException(ctx,  "Can only call xreSupportedTune() on instances of AAMPPlayer");
		return JSValueMakeUndefined(ctx);
	}
	if (argumentCount == 1)
	{
		bool xreSupported = JSValueToBoolean(ctx, arguments[0]);
		privObj->mPlayer->XRESupportedTune(xreSupported);
	}
	else
	{
		*exception = GetException(ctx,  "Failed to execute xreSupportedTune() - 1 argument required");
	}
	return JSValueMakeUndefined(ctx);
}

void AAMPMediaPlayer_JS_finalize(JSObjectRef object)
{
        PlayerWrapper *wrapperObj = (PlayerWrapper *) JSObjectGetPrivate(object);
        if (wrapperObj != NULL)
        {
                //unlink native object from JS object
                JSObjectSetPrivate(object, NULL);
                wrapperObj->terminate();
                // Delete native object
                delete wrapperObj;
        }
        else
        {
                NativeJSLogger::log(WARN, "called finalize on empty object\n");
		fflush(stdout);
        }
}

static const JSStaticFunction AAMPMediaPlayerJS_static_functions[] = {
        { "load", AAMPMediaPlayerJS_load, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "initConfig", AAMPMediaPlayerJS_initConfig, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
        { "setVolume", AAMPMediaPlayerJS_setVolume, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
        { "addEventListener", AAMPMediaPlayerJS_addEventListener, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
        { "removeEventListener", AAMPMediaPlayerJS_removeEventListener, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
        { "subscribeResponseHeaders", AAMPMediaPlayerJS_subscribeResponseHeaders, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
        { "setSubscribedTags", AAMPMediaPlayerJS_setSubscribedTags, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
        { "addCustomHTTPHeader", AAMPMediaPlayerJS_addCustomHTTPHeader, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
        { "removeCustomHTTPHeader", AAMPMediaPlayerJS_removeCustomHTTPHeader, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setClosedCaptionStatus", AAMPMediaPlayerJS_setClosedCaptionStatus, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
        { "setVideoMute", AAMPMediaPlayerJS_setVideoMute, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "play", AAMPMediaPlayerJS_play, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "detach", AAMPMediaPlayerJS_detach, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "pause", AAMPMediaPlayerJS_pause, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "stop", AAMPMediaPlayerJS_stop, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "release", AAMPMediaPlayerJS_release, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "seek", AAMPMediaPlayerJS_seek, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "getDurationSec", AAMPMediaPlayerJS_getDurationSec, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "getCurrentPosition", AAMPMediaPlayerJS_getCurrentPosition, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "getAudioTrack", AAMPMediaPlayerJS_getAudioTrack, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "getAudioTrackInfo", AAMPMediaPlayerJS_getAudioTrackInfo, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "getTextTrackInfo", AAMPMediaPlayerJS_getTextTrackInfo, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "getTextTrack", AAMPMediaPlayerJS_getTextTrack, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "getVolume", AAMPMediaPlayerJS_getVolume, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "setAudioLanguage", AAMPMediaPlayerJS_setAudioLanguage, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "setPlaybackRate", AAMPMediaPlayerJS_setPlaybackRate, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "setVideoRect", AAMPMediaPlayerJS_setVideoRect, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setVideoZoom", AAMPMediaPlayerJS_setVideoZoom, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "getAvailableAudioTracks", AAMPMediaPlayerJS_getAvailableAudioTracks, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "getAvailableTextTracks", AAMPMediaPlayerJS_getAvailableTextTracks, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "getVideoRectangle", AAMPMediaPlayerJS_getVideoRectangle, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "setAlternateContent", AAMPMediaPlayerJS_setAlternateContent, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "notifyReservationCompletion", AAMPMediaPlayerJS_notifyReservationCompletion, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setTextStyleOptions", AAMPMediaPlayerJS_setTextStyleOptions, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "setPreferredAudioLanguage", AAMPMediaPlayerJS_setPreferredAudioLanguage, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "setPreferredTextLanguage", AAMPMediaPlayerJS_setPreferredTextLanguage, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ "setAuxiliaryLanguage", AAMPMediaPlayerJS_setAuxiliaryLanguage, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly },
	{ "xreSupportedTune", AAMPMediaPlayerJS_xreSupportedTune, kJSPropertyAttributeDontDelete | kJSPropertyAttributeReadOnly},
	{ NULL, NULL, 0 },
};


static JSClassDefinition AAMPMediaPlayer_JS_object_def {
        0, /* current (and only) version is 0 */
        kJSClassAttributeNone,
        "__AAMPMediaPlayer",
        NULL,

        NULL,
        AAMPMediaPlayerJS_static_functions,

        NULL,
        AAMPMediaPlayer_JS_finalize,
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
static JSClassRef AAMPMediaPlayer_object_ref() {
        static JSClassRef _mediaPlayerObjRef = NULL;
        if (!_mediaPlayerObjRef) {
                _mediaPlayerObjRef = JSClassCreate(&AAMPMediaPlayer_JS_object_def);
        }
        return _mediaPlayerObjRef;
}

JSObjectRef AAMPMediaPlayer_JS_class_constructor(JSContextRef ctx, JSObjectRef constructor, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception)
{
        PlayerWrapper* wrapperObj = new PlayerWrapper();
        wrapperObj->initialize(ctx);
/*
        wrapperObj->ctx = JSContextGetGlobalContext(ctx);
        wrapperObj->mPlayer = new PlayerInstanceAAMP(NULL, NULL);
        mPlayerEventHandler = new PlayerEventHandler(ctx, wrapperObj->mPlayer);
*/
        // NOTE : Association of JSObject and AAMPMediaPlayer_JS native object will be deleted only in
        // AAMPMediaPlayerJS_release ( APP initiated )  or AAMPMediaPlayer_JS_finalize ( GC initiated)
        // There is chance that aamp_UnloadJS is called then functions on aamp object is called from JS script.
        // In this case AAMPMediaPlayer_JS should be available to access
        JSObjectRef newObj = JSObjectMake(ctx, AAMPMediaPlayer_object_ref(), wrapperObj);

        // Required for viper-player
        /*
        JSStringRef fName = JSStringCreateWithUTF8CString("toString");
        JSStringRef fString = JSStringCreateWithUTF8CString("return \"[object __AAMPMediaPlayer]\";");
        JSObjectRef toStringFunc = JSObjectMakeFunction(ctx, NULL, 0, NULL, fString, NULL, 0, NULL);

        JSObjectSetProperty(ctx, newObj, fName, toStringFunc, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontEnum | kJSPropertyAttributeDontDelete, NULL);
        JSStringRelease(fName);
        JSStringRelease(fString);
        */
        return newObj;
}
static JSClassDefinition AAMPMediaPlayer_JS_class_def {
        0, /* current (and only) version is 0 */
        kJSClassAttributeNone,
        "__AAMPMediaPlayer__class",
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
        AAMPMediaPlayer_JS_class_constructor,
        NULL,
        NULL
};
void initializePlayer(JSGlobalContextRef context)
{
    JSGlobalContextRef jsContext = (JSGlobalContextRef)context;
    JSObjectRef globalObj = JSContextGetGlobalObject(jsContext);
    JSClassRef mediaPlayerClass = JSClassCreate(&AAMPMediaPlayer_JS_class_def);
    JSObjectRef classObj = JSObjectMakeConstructor(jsContext, mediaPlayerClass, AAMPMediaPlayer_JS_class_constructor);
    JSValueProtect(jsContext, classObj);
    JSStringRef str = JSStringCreateWithUTF8CString("AAMPMediaPlayer");
    JSObjectSetProperty(jsContext, globalObj, str, classObj, kJSPropertyAttributeReadOnly | kJSPropertyAttributeDontDelete, NULL);
    JSClassRelease(mediaPlayerClass);
    JSStringRelease(str);
}

void deinitializePlayer(JSGlobalContextRef context)
{
    JSValueRef exception = NULL;
    JSGlobalContextRef jsContext = (JSGlobalContextRef)context;
    JSObjectRef globalObj = JSContextGetGlobalObject(jsContext);
    JSStringRef str = JSStringCreateWithUTF8CString("AAMPMediaPlayer");
    JSValueRef playerConstructor = JSObjectGetProperty(jsContext, globalObj, str, &exception);
    if (playerConstructor == NULL || exception != NULL)
    {
            JSStringRelease(str);
            return;
    }
    JSObjectRef playerObj = JSValueToObject(jsContext, playerConstructor, &exception);
    if (playerObj == NULL || exception != NULL)
    {
            JSStringRelease(str);
            return;
    }
    if (!JSObjectIsConstructor(jsContext, playerObj))
    {
            JSStringRelease(str);
            return;
    }
    
    JSValueUnprotect(jsContext, playerConstructor);
    JSObjectSetProperty(jsContext, globalObj, str, JSValueMakeUndefined(jsContext), kJSPropertyAttributeReadOnly, NULL);
    JSStringRelease(str);
    
    JSGarbageCollect(jsContext);
}
