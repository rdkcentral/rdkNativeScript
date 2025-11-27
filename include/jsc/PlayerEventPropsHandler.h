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

#ifndef __EVENTPROPSHANDLER__H__
#define __EVENTPROPSHANDLER__H__

#include <JavaScriptCore/JavaScript.h>
#include "main_aamp.h"
#include <map>

class PlayerEventPropsHandler
{
    public:
        static void handle(AAMPEventType type, const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static std::map<AAMPEventType ,void (*)(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)> sHandlers;
    private:
        static void handleDefault(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);;
        static void handlePlaybackFailed(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleSpeedChanged(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleProgressUpdate(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleCCHandleAvailable(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleMediaMetadata(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleBitrateChanged(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleTimedMetadata(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleBulkTimedMetadata(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handlePlaybackStateChanged(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleSeeked(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleBufferingChanged(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleDRMMetadata(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleAnomalyReport(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleAdReservationStart(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleAdReservationEnd(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleAdPlacementStart(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleAdPlacementEnd(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleAdProgress(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleAdPlacementError(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleMetricsData(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleBlocked(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleContentGap(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleHTTPResponseHeader(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
        static void handleWatermarkSessionUpdate(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context);
};

#endif /** __EVENTPROPSHANDLER__H__ **/
