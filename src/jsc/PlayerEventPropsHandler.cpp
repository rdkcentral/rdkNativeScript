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

#include <JavaScriptCore/JavaScript.h>
#include "main_aamp.h"
#include <PlayerEventPropsHandler.h>

#define SAFE_DELETE_ARRAY(array) if(array) \
{ \
        delete[] array; \
}

std::map<AAMPEventType ,void (*)(const AAMPEventPtr&, JSObjectRef event, JSContextRef context)> PlayerEventPropsHandler::sHandlers = {
	{AAMP_EVENT_TUNED , &PlayerEventPropsHandler::handleDefault},
	{AAMP_EVENT_EOS , &PlayerEventPropsHandler::handleDefault},
        {AAMP_EVENT_PLAYLIST_INDEXED, &PlayerEventPropsHandler::handleDefault},
	{AAMP_EVENT_TUNE_FAILED, &PlayerEventPropsHandler::handlePlaybackFailed},
	{AAMP_EVENT_SPEED_CHANGED, &PlayerEventPropsHandler::handleSpeedChanged},
	{AAMP_EVENT_PROGRESS, &PlayerEventPropsHandler::handleProgressUpdate},
	{AAMP_EVENT_CC_HANDLE_RECEIVED, &PlayerEventPropsHandler::handleCCHandleAvailable},
	{AAMP_EVENT_JS_EVENT, &PlayerEventPropsHandler::handleDefault},
	{AAMP_EVENT_MEDIA_METADATA, &PlayerEventPropsHandler::handleMediaMetadata},
	{AAMP_EVENT_ENTERING_LIVE, &PlayerEventPropsHandler::handleDefault},
	{AAMP_EVENT_BITRATE_CHANGED, &PlayerEventPropsHandler::handleBitrateChanged},
	{AAMP_EVENT_TIMED_METADATA, &PlayerEventPropsHandler::handleTimedMetadata},
	{AAMP_EVENT_BULK_TIMED_METADATA, &PlayerEventPropsHandler::handleBulkTimedMetadata},
	{AAMP_EVENT_STATE_CHANGED, &PlayerEventPropsHandler::handlePlaybackStateChanged},
	{AAMP_EVENT_SEEKED, &PlayerEventPropsHandler::handleSeeked},
	{AAMP_EVENT_BUFFERING_CHANGED, &PlayerEventPropsHandler::handleBufferingChanged},
	{AAMP_EVENT_DURATION_CHANGED, &PlayerEventPropsHandler::handleDefault},
	{AAMP_EVENT_AD_STARTED, &PlayerEventPropsHandler::handleDefault},
	{AAMP_EVENT_DRM_METADATA,  &PlayerEventPropsHandler::handleDRMMetadata},
	{AAMP_EVENT_REPORT_ANOMALY, &PlayerEventPropsHandler::handleAnomalyReport},
	{AAMP_EVENT_AD_RESERVATION_START, &PlayerEventPropsHandler::handleAdReservationStart},
	{AAMP_EVENT_AD_RESERVATION_END, &PlayerEventPropsHandler::handleAdReservationEnd},
	{AAMP_EVENT_AD_PLACEMENT_START,  &PlayerEventPropsHandler::handleAdPlacementStart},
	{AAMP_EVENT_AD_PLACEMENT_END,  &PlayerEventPropsHandler::handleAdPlacementEnd},
	{AAMP_EVENT_AD_PLACEMENT_ERROR,  &PlayerEventPropsHandler::handleAdPlacementError},
	{AAMP_EVENT_AD_PLACEMENT_PROGRESS,  &PlayerEventPropsHandler::handleAdProgress},
	{AAMP_EVENT_REPORT_METRICS_DATA,  &PlayerEventPropsHandler::handleMetricsData},
	{AAMP_EVENT_BLOCKED, &PlayerEventPropsHandler::handleBlocked},
	{AAMP_EVENT_CONTENT_GAP,  &PlayerEventPropsHandler::handleContentGap},
	{AAMP_EVENT_HTTP_RESPONSE_HEADER, &PlayerEventPropsHandler::handleHTTPResponseHeader},
	{AAMP_EVENT_WATERMARK_SESSION_UPDATE, &PlayerEventPropsHandler::handleWatermarkSessionUpdate}
};


static JSValueRef CStringToJSValue(JSContextRef context, const char* sz)
{
        JSStringRef str = JSStringCreateWithUTF8CString(sz);
        JSValueRef value = JSValueMakeString(context, str);
        JSStringRelease(str);
        return value;
}

static JSObjectRef CreateTimedMetadataJSObject(JSContextRef context, long long timeMS, const char* szName, const char* szContent, const char* id, double durationMS)
{
	JSStringRef name;

	JSObjectRef timedMetadata = JSObjectMake(context, NULL, NULL);

	if (timedMetadata) {
		JSValueProtect(context, timedMetadata);
		bool bGenerateID = true;

		name = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, timedMetadata, name, JSValueMakeNumber(context, std::round(timeMS)), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		// For SCTE35 tag, set id as value of key reservationId
		if(!strcmp(szName, "SCTE35") && id && *id != '\0')
		{
			name = JSStringCreateWithUTF8CString("reservationId");
			JSObjectSetProperty(context, timedMetadata, name, CStringToJSValue(context, id), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(name);
			bGenerateID = false;
		}

		if (durationMS >= 0)
		{
			name = JSStringCreateWithUTF8CString("duration");
			JSObjectSetProperty(context, timedMetadata, name, JSValueMakeNumber(context, (int)durationMS), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(name);
		}

		name = JSStringCreateWithUTF8CString("name");
		JSObjectSetProperty(context, timedMetadata, name, CStringToJSValue(context, szName), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("content");
		JSObjectSetProperty(context, timedMetadata, name, CStringToJSValue(context, szContent), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		// Force type=0 (HLS tag) for now.
		// Does type=1 ID3 need to be supported?
		name = JSStringCreateWithUTF8CString("type");
		JSObjectSetProperty(context, timedMetadata, name, JSValueMakeNumber(context, 0), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		// Force metadata as empty object
		JSObjectRef metadata = JSObjectMake(context, NULL, NULL);
		if (metadata) {
			JSValueProtect(context, metadata);
			name = JSStringCreateWithUTF8CString("metadata");
			JSObjectSetProperty(context, timedMetadata, name, metadata, kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(name);

			// Parse CUE metadata and TRICKMODE-RESTRICTION metadata
			// Parsed values are used in PlayerPlatform at the time of tag object creation
			if ((strcmp(szName, "#EXT-X-CUE") == 0) ||
			    (strcmp(szName, "#EXT-X-TRICKMODE-RESTRICTION") == 0) ||
			    (strcmp(szName, "#EXT-X-MARKER") == 0) ||
			    (strcmp(szName, "#EXT-X-SCTE35") == 0)) {
				const char* szStart = szContent;

				// Parse comma seperated name=value list.
				while (*szStart != '\0') {
					char* szSep;
					// Find the '=' seperator.
					for (szSep = (char*)szStart; *szSep != '=' && *szSep != '\0'; szSep++);

					// Find the end of the value.
					char* szEnd = (*szSep != '\0') ? szSep + 1 : szSep;
					for (; *szEnd != ',' && *szEnd != '\0'; szEnd++);

					// Append the name / value metadata.
					if ((szStart < szSep) && (szSep < szEnd)) {
						JSValueRef value;
						char chSave = *szSep;

						*szSep = '\0';
						name = JSStringCreateWithUTF8CString(szStart);
						*szSep = chSave;

						chSave = *szEnd;
						*szEnd = '\0';
						value = CStringToJSValue(context, szSep+1);
						*szEnd = chSave;

						JSObjectSetProperty(context, metadata, name, value, kJSPropertyAttributeReadOnly, NULL);
						JSStringRelease(name);

						// If we just added the 'ID', copy into timedMetadata.id
						if (szStart[0] == 'I' && szStart[1] == 'D' && szStart[2] == '=') {
							bGenerateID = false;
							name = JSStringCreateWithUTF8CString("id");
							JSObjectSetProperty(context, timedMetadata, name, value, kJSPropertyAttributeReadOnly, NULL);
							JSStringRelease(name);
						}
					}

					szStart = (*szEnd != '\0') ? szEnd + 1 : szEnd;
				}
			}
			// Parse TARGETDURATION and CONTENT-IDENTIFIER metadata
			else {
				const char* szStart = szContent;
				// Advance to the tag's value.
				for (; *szStart != ':' && *szStart != '\0'; szStart++);
				if (*szStart == ':')
					szStart++;

				// Stuff all content into DATA name/value pair.
				JSValueRef value = CStringToJSValue(context, szStart);
				if (strcmp(szName, "#EXT-X-TARGETDURATION") == 0) {
					// Stuff into DURATION if EXT-X-TARGETDURATION content.
					// Since #EXT-X-TARGETDURATION has only duration as value
					name = JSStringCreateWithUTF8CString("DURATION");
				} else {
					name = JSStringCreateWithUTF8CString("DATA");
				}
				JSObjectSetProperty(context, metadata, name, value, kJSPropertyAttributeReadOnly, NULL);
				JSStringRelease(name);
			}
			JSValueUnprotect(context, metadata);
		}

		// Generate an ID since the tag is missing one
		if (bGenerateID) {
			int hash = (int)timeMS;
			const char* szStart = szName;
			for (; *szStart != '\0'; szStart++) {
				hash = (hash * 33) ^ *szStart;
			}

			char buf[32];
			sprintf(buf, "%d", hash);
			name = JSStringCreateWithUTF8CString("id");
			JSObjectSetProperty(context, timedMetadata, name, CStringToJSValue(context, buf), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(name);
		}
		JSValueUnprotect(context, timedMetadata);
	}

        return timedMetadata;
}

void PlayerEventPropsHandler::handle(AAMPEventType type, const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
    std::map<AAMPEventType ,void (*)(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)>::iterator iter = sHandlers.find(type); 	
    if (iter != sHandlers.end())
    {
	void (*fn)(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context) = iter->second;
        fn(aampEvent, event, context);
    }
}

void PlayerEventPropsHandler::handleDefault(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
}

void PlayerEventPropsHandler::handlePlaybackFailed(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		MediaErrorEventPtr evt = std::dynamic_pointer_cast<MediaErrorEvent>(aampEvent);

		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("code");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getCode()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("description");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getDescription().c_str()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("shouldRetry");
		JSObjectSetProperty(context, event, prop, JSValueMakeBoolean(context, evt->shouldRetry()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		if(-1 != evt->getClass()) //Only send verbose error for secclient/secmanager DRM failures
{
			prop = JSStringCreateWithUTF8CString("class");
			JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getClass()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);

			prop = JSStringCreateWithUTF8CString("reason");
			JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getReason()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);

			prop = JSStringCreateWithUTF8CString("businessStatus");
			JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getBusinessStatus()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
		}
}

void PlayerEventPropsHandler::handleSpeedChanged(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		SpeedChangedEventPtr evt = std::dynamic_pointer_cast<SpeedChangedEvent>(aampEvent);

		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("speed");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getRate()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("reason");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, "unknown"), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}

void PlayerEventPropsHandler::handleProgressUpdate(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		ProgressEventPtr evt = std::dynamic_pointer_cast<ProgressEvent>(aampEvent);

		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("durationMiliseconds");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getDuration()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("positionMiliseconds");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getPosition()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("playbackSpeed");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getSpeed()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("startMiliseconds");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getStart()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("endMiliseconds");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getEnd()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("currentPTS");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getPTS()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("videoBufferedMiliseconds");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getBufferedDuration()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("timecode");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getSEITimeCode()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
	}

void PlayerEventPropsHandler::handleCCHandleAvailable(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		CCHandleEventPtr evt = std::dynamic_pointer_cast<CCHandleEvent>(aampEvent);

		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("decoderHandle");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getCCHandle()), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);

}

void PlayerEventPropsHandler::handleMediaMetadata(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		MediaMetadataEventPtr evt = std::dynamic_pointer_cast<MediaMetadataEvent>(aampEvent);

		JSStringRef prop;
		prop = JSStringCreateWithUTF8CString("durationMiliseconds");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getDuration()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		int count = evt->getLanguagesCount();
		const std::vector<std::string> &langVect = evt->getLanguages();
		JSValueRef* array = new JSValueRef[count];
		for (int32_t i = 0; i < count; i++)
{
			JSValueRef lang = CStringToJSValue(context, langVect[i].c_str());
			array[i] = lang;
		}
		JSValueRef propValue = JSObjectMakeArray(context, count, array, NULL);
		SAFE_DELETE_ARRAY(array);

		prop = JSStringCreateWithUTF8CString("languages");
		JSObjectSetProperty(context, event, prop, propValue, kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		count = evt->getBitratesCount();
		const std::vector<long> &bitrateVect = evt->getBitrates();
		array = new JSValueRef[count];
		for (int32_t i = 0; i < count; i++)
{
			array[i] = JSValueMakeNumber(context, bitrateVect[i]);
		}
		propValue = JSObjectMakeArray(context, count, array, NULL);
		SAFE_DELETE_ARRAY(array);

		prop = JSStringCreateWithUTF8CString("bitrates");
		JSObjectSetProperty(context, event, prop, propValue, kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		count = evt->getSupportedSpeedCount();
		const std::vector<float> &speedVect = evt->getSupportedSpeeds();
		array = new JSValueRef[count];
		for (int32_t i = 0; i < count; i++)
{
			array[i] = JSValueMakeNumber(context, speedVect[i]);
		}
		propValue = JSObjectMakeArray(context, count, array, NULL);
		SAFE_DELETE_ARRAY(array);

		prop = JSStringCreateWithUTF8CString("playbackSpeeds");
		JSObjectSetProperty(context, event, prop, propValue, kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("programStartTime");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getProgramStartTime()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("width");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getWidth()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("height");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getHeight()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("hasDrm");
		JSObjectSetProperty(context, event, prop, JSValueMakeBoolean(context, evt->hasDrm()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("isLive");
		JSObjectSetProperty(context, event, prop, JSValueMakeBoolean(context, evt->isLive()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("DRM");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getDrmType().c_str()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		//ratings
		if(!evt->getRatings().empty())
{
			prop = JSStringCreateWithUTF8CString("ratings");
			JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getRatings().c_str()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
		}

		//ssi
		if(evt->getSsi() >= 0 )
{
			prop = JSStringCreateWithUTF8CString("ssi");
			JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getSsi()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
		}

		//framerate
		if(evt->getFrameRate() > 0 )
{
			prop = JSStringCreateWithUTF8CString("framerate");
			JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getFrameRate()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
		}

		if(eVIDEOSCAN_UNKNOWN != evt->getVideoScanType())
{
			prop = JSStringCreateWithUTF8CString("progressive");
			JSObjectSetProperty(context, event, prop, JSValueMakeBoolean(context, ((eVIDEOSCAN_PROGRESSIVE == evt->getVideoScanType())?true:false)), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
		}
		//aspect ratio
		if((0 != evt->getAspectRatioWidth()) && (0 != evt->getAspectRatioHeight()))
{
			prop = JSStringCreateWithUTF8CString("aspectRatioWidth");
			JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getAspectRatioWidth()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);

			prop = JSStringCreateWithUTF8CString("aspectRatioHeight");
			JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getAspectRatioHeight()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
		}

		//VideoCodec
		if(!evt->getVideoCodec().empty())
{
			prop = JSStringCreateWithUTF8CString("videoCodec");
			JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getVideoCodec().c_str()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
		}

		//HdrType
		if(!evt->getHdrType().empty())
{
			prop = JSStringCreateWithUTF8CString("hdrType");
			JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getHdrType().c_str()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
		}

		//AudioBitrate
		const std::vector<long> &audioBitrateVect = evt->getAudioBitrates();
		count = audioBitrateVect.size();
		if(count > 0 )
{
			array = new JSValueRef[count];
			for (int32_t i = 0; i < count; i++)
	{
				array[i] = JSValueMakeNumber(context, audioBitrateVect[i]);
			}
			propValue = JSObjectMakeArray(context, count, array, NULL);
			delete [] array;

			prop = JSStringCreateWithUTF8CString("audioBitrates");
			JSObjectSetProperty(context, event, prop, propValue, kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
		}


		//AudioCodec
		if(!evt->getAudioCodec().empty())
{
			prop = JSStringCreateWithUTF8CString("audioCodec");
			JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getAudioCodec().c_str()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
		}

		//AudioMixType
		if(!evt->getAudioMixType().empty())
{
			prop = JSStringCreateWithUTF8CString("audioMixType");
			JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getAudioMixType().c_str()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
		}

		//AtmosInfo
		prop = JSStringCreateWithUTF8CString("isAtmos");
		JSObjectSetProperty(context, event, prop, JSValueMakeBoolean(context,evt->getAtmosInfo()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		//MediaFormat type
		prop = JSStringCreateWithUTF8CString("mediaFormat");
                JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getMediaFormat().c_str()), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);
}

void PlayerEventPropsHandler::handleBitrateChanged(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		BitrateChangeEventPtr evt = std::dynamic_pointer_cast<BitrateChangeEvent>(aampEvent);
		JSStringRef name;
		name = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, event, name, JSValueMakeNumber(context, evt->getTime()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("bitRate");
		JSObjectSetProperty(context, event, name, JSValueMakeNumber(context, evt->getBitrate()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("description");
		JSObjectSetProperty(context, event, name, CStringToJSValue(context, evt->getDescription().c_str()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("width");
		JSObjectSetProperty(context, event, name, JSValueMakeNumber(context, evt->getWidth()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("height");
		JSObjectSetProperty(context, event, name, JSValueMakeNumber(context, evt->getHeight()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("framerate");
		JSObjectSetProperty(context, event, name, JSValueMakeNumber(context, evt->getFrameRate()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("position");
		JSObjectSetProperty(context, event, name, JSValueMakeNumber(context, evt->getPosition()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("cappedProfile");
		JSObjectSetProperty(context, event, name, JSValueMakeNumber(context, evt->getCappedProfileStatus()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("displayWidth");
                JSObjectSetProperty(context, event, name, JSValueMakeNumber(context, evt->getDisplayWidth()), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(name);

		name = JSStringCreateWithUTF8CString("displayHeight");
                JSObjectSetProperty(context, event, name, JSValueMakeNumber(context, evt->getDisplayHeight()), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(name);

		if(eVIDEOSCAN_UNKNOWN != evt->getScanType())
{
			name = JSStringCreateWithUTF8CString("progressive");
			JSObjectSetProperty(context, event, name, JSValueMakeBoolean(context, ((eVIDEOSCAN_PROGRESSIVE == evt->getScanType())?true:false)), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(name);
		}

		if((0 != evt->getAspectRatioWidth()) && (0 != evt->getAspectRatioHeight()))
{
			name = JSStringCreateWithUTF8CString("aspectRatioWidth");
			JSObjectSetProperty(context, event, name, JSValueMakeNumber(context, evt->getAspectRatioWidth()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(name);

			name = JSStringCreateWithUTF8CString("aspectRatioHeight");
			JSObjectSetProperty(context, event, name, JSValueMakeNumber(context, evt->getAspectRatioHeight()), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(name);
		}
}

void PlayerEventPropsHandler::handleTimedMetadata(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		TimedMetadataEventPtr evt = std::dynamic_pointer_cast<TimedMetadataEvent>(aampEvent);

		JSObjectRef timedMetadata = CreateTimedMetadataJSObject(context, evt->getTime(), evt->getName().c_str(), evt->getContent().c_str(), evt->getId().c_str(), evt->getDuration());
		if (timedMetadata)
{
			JSValueProtect(context, timedMetadata);
			JSStringRef prop = JSStringCreateWithUTF8CString("timedMetadata");
			JSObjectSetProperty(context, event, prop, timedMetadata, kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
			JSValueUnprotect(context, timedMetadata);
		}
}

void PlayerEventPropsHandler::handleBulkTimedMetadata(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		BulkTimedMetadataEventPtr evt = std::dynamic_pointer_cast<BulkTimedMetadataEvent>(aampEvent);
		JSStringRef name = JSStringCreateWithUTF8CString("timedMetadatas");
		JSObjectSetProperty(context, event, name, CStringToJSValue(context, evt->getContent().c_str()),  kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(name);
}

void PlayerEventPropsHandler::handlePlaybackStateChanged(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		StateChangedEventPtr evt = std::dynamic_pointer_cast<StateChangedEvent>(aampEvent);

		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("state");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getState()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}


void PlayerEventPropsHandler::handleSeeked(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		SeekedEventPtr evt = std::dynamic_pointer_cast<SeekedEvent>(aampEvent);
		JSStringRef prop = JSStringCreateWithUTF8CString("position");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getPosition()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}

void PlayerEventPropsHandler::handleBufferingChanged(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		BufferingChangedEventPtr evt = std::dynamic_pointer_cast<BufferingChangedEvent>(aampEvent);

		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("buffering");
		JSObjectSetProperty(context, event, prop, JSValueMakeBoolean(context, evt->buffering()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}


void PlayerEventPropsHandler::handleDRMMetadata(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		DrmMetaDataEventPtr evt = std::dynamic_pointer_cast<DrmMetaDataEvent>(aampEvent);
		JSStringRef prop;

		int code = evt->getAccessStatusValue();
		const char* description = evt->getAccessStatus().c_str();

		prop = JSStringCreateWithUTF8CString("code");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, code), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("description");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, description), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}

void PlayerEventPropsHandler::handleAnomalyReport(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		AnomalyReportEventPtr evt = std::dynamic_pointer_cast<AnomalyReportEvent>(aampEvent);
		JSStringRef prop;

		int severity = evt->getSeverity();
		const char* description = evt->getMessage().c_str();

		prop = JSStringCreateWithUTF8CString("severity");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, severity), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("description");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, description), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}


void PlayerEventPropsHandler::handleAdReservationStart(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		AdReservationEventPtr evt = std::dynamic_pointer_cast<AdReservationEvent>(aampEvent);
		JSStringRef prop;
		prop = JSStringCreateWithUTF8CString("adbreakId");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getAdBreakId().c_str()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getPosition()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}


void PlayerEventPropsHandler::handleAdReservationEnd(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		AdReservationEventPtr evt = std::dynamic_pointer_cast<AdReservationEvent>(aampEvent);
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("adbreakId");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getAdBreakId().c_str()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getPosition()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}


void PlayerEventPropsHandler::handleAdPlacementStart(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		AdPlacementEventPtr evt = std::dynamic_pointer_cast<AdPlacementEvent>(aampEvent);
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("adId");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getAdId().c_str()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getPosition()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}


void PlayerEventPropsHandler::handleAdPlacementEnd(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		AdPlacementEventPtr evt = std::dynamic_pointer_cast<AdPlacementEvent>(aampEvent);
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("adId");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getAdId().c_str()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getPosition()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}


void PlayerEventPropsHandler::handleAdProgress(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		AdPlacementEventPtr evt = std::dynamic_pointer_cast<AdPlacementEvent>(aampEvent);
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("adId");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getAdId().c_str()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getPosition()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}


void PlayerEventPropsHandler::handleAdPlacementError(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		AdPlacementEventPtr evt = std::dynamic_pointer_cast<AdPlacementEvent>(aampEvent);
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("adId");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getAdId().c_str()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getPosition()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("error");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getErrorCode()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}


void PlayerEventPropsHandler::handleMetricsData(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
                MetricsDataEventPtr evt = std::dynamic_pointer_cast<MetricsDataEvent>(aampEvent);
                JSStringRef strJSObj;

                strJSObj = JSStringCreateWithUTF8CString("metricType");
                JSObjectSetProperty(context, event, strJSObj, JSValueMakeNumber(context, evt->getMetricsDataType()), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(strJSObj);

                strJSObj = JSStringCreateWithUTF8CString("traceID");
                JSObjectSetProperty(context, event, strJSObj, CStringToJSValue(context, evt->getMetricUUID().c_str()), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(strJSObj);

                strJSObj = JSStringCreateWithUTF8CString("metricData");
                JSObjectSetProperty(context, event, strJSObj, CStringToJSValue(context, evt->getMetricsData().c_str()), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(strJSObj);
}


void PlayerEventPropsHandler::handleBlocked(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		BlockedEventPtr evt = std::dynamic_pointer_cast<BlockedEvent>(aampEvent);
		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("reason");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getReason().c_str()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}

void PlayerEventPropsHandler::handleContentGap(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		ContentGapEventPtr evt = std::dynamic_pointer_cast<ContentGapEvent>(aampEvent);
		JSStringRef prop;

		double time = evt->getTime();
		double durationMs = evt->getDuration();

		prop = JSStringCreateWithUTF8CString("time");
		JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, std::round(time)), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		if (durationMs >= 0)
{
			prop = JSStringCreateWithUTF8CString("duration");
			JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, (int)durationMs), kJSPropertyAttributeReadOnly, NULL);
			JSStringRelease(prop);
		}
}


void PlayerEventPropsHandler::handleHTTPResponseHeader(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
		HTTPResponseHeaderEventPtr evt = std::dynamic_pointer_cast<HTTPResponseHeaderEvent>(aampEvent);

		JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("header");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getHeader().c_str()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("response");
		JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getResponse().c_str()), kJSPropertyAttributeReadOnly, NULL);
		JSStringRelease(prop);
}

void PlayerEventPropsHandler::handleWatermarkSessionUpdate(const AAMPEventPtr& aampEvent, JSObjectRef event, JSContextRef context)
{
                WatermarkSessionUpdateEventPtr evt = std::dynamic_pointer_cast<WatermarkSessionUpdateEvent>(aampEvent);
                JSStringRef prop;

		prop = JSStringCreateWithUTF8CString("sessionHandle");
                JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getSessionHandle()), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);

		prop = JSStringCreateWithUTF8CString("status");
                JSObjectSetProperty(context, event, prop, JSValueMakeNumber(context, evt->getStatus()), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);

                prop = JSStringCreateWithUTF8CString("system");
                JSObjectSetProperty(context, event, prop, CStringToJSValue(context, evt->getSystem().c_str()), kJSPropertyAttributeReadOnly, NULL);
                JSStringRelease(prop);
}
