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
#ifdef UNIT_TEST_BUILD
        PlayerEventHandler* mPlayerEventHandler;
#endif
           	
    private:
        JSObjectRef getCallbackForAdId(std::string id);
        std::map<std::string, JSObjectRef> mPromiseCallbacks;
#ifndef UNIT_TEST_BUILD 
        PlayerEventHandler* mPlayerEventHandler;
#endif
};

void initializePlayer(JSGlobalContextRef context);
void deinitializePlayer(JSGlobalContextRef context);

#ifdef UNIT_TEST_BUILD
std::vector<std::string> JSStringArrayToCStringArray(JSContextRef context, JSValueRef arrayRef);

JSValueRef AAMPMediaPlayerJS_initConfig(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_load(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setVolume(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_addEventListener(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_removeEventListener(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_subscribeResponseHeaders(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setSubscribedTags(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_addCustomHTTPHeader(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_removeCustomHTTPHeader(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setClosedCaptionStatus(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setVideoMute(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_play(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_detach(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_pause(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_release(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_stop(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_seek(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_getAudioTrackInfo(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_getTextTrackInfo(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_getDurationSec(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_getCurrentPosition(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_getAudioTrack(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_getTextTrack(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_getVolume(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setAudioLanguage(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setPlaybackRate(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setVideoRect(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setVideoZoom(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setPreferredAudioLanguage(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setPreferredTextLanguage(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setTextStyleOptions(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setAuxiliaryLanguage(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_xreSupportedTune(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);

JSValueRef AAMPMediaPlayerJS_getAvailableAudioTracks(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_getAvailableTextTracks(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_getVideoRectangle(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_setAlternateContent(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
JSValueRef AAMPMediaPlayerJS_notifyReservationCompletion(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
#endif

#endif
