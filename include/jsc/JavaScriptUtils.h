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

#ifndef JAVASCRIPTMISC_H
#define JAVASCRIPTMISC_H

#include <JavaScriptCore/JavaScript.h>
#include "rtHttpRequest.h"
#include "rtString.h"
#include "rtAtomic.h"
#include <rtError.h>
#include <rtValue.h>
#include <string>
#include <memory>
#include <iostream>
#include <functional>
#include <vector>
#include <thread>
#include <queue>
#include <chrono>
#include <cstdint>

namespace jsruntime
{
template<typename T>
class RefCounted: public T
{
protected:
  int m_refCount { 0 };
  unsigned long AddRef() final {
    return rtAtomicInc(&m_refCount);
  }
  unsigned long Release() final {
    long l = rtAtomicDec(&m_refCount);
    if (l == 0) {
      delete this;
    }
    return l;
  }
  virtual ~RefCounted() {}
};
}

class rtHttpRequestEx : public rtHttpRequest
{
public:
  rtDeclareObject(rtHttpRequestEx, rtHttpRequest);
  rtHttpRequestEx(const rtString& url);
  rtHttpRequestEx(const rtObjectRef& options);
  void onDownloadProgressImpl(double progress) final;
  void onDownloadCompleteImpl(rtFileDownloadRequest* downloadRequest) final;
};

struct TimeoutInfo
{
    std::function<int ()> callback;
    std::chrono::time_point<std::chrono::steady_clock> fireTime;
    std::chrono::milliseconds interval;
    bool repeat;
    uint32_t tag;
    bool canceled;
};

struct TimeoutInfoComparator
{
    constexpr bool operator()(const TimeoutInfo *lhs, const TimeoutInfo *rhs) const {
        return !((lhs->fireTime < rhs->fireTime) ||
                 ((lhs->fireTime == rhs->fireTime) && (lhs->tag < rhs->tag)));
    }
};

class TimeoutQueue : public std::priority_queue<TimeoutInfo*, std::vector<TimeoutInfo*>, TimeoutInfoComparator>
{
public:
    void pushTimeouts(const std::vector<TimeoutInfo*>& timerVec);
    bool updateForInfo(const TimeoutInfo* info);
};

void dispatchOnMainLoop(std::function<void ()>&& fun);
void dispatchPending();
void dispatchTimeouts();
uint32_t installTimeout(double intervalMs, bool repeat, std::function<int ()>&& fun);
void clearTimeout(uint32_t tag);

void printException(JSContextRef ctx, JSValueRef exception);
rtString jsToRtString(JSStringRef str);

void assertIsMainThread();
rtError rtWebSocketBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtWebSocketServerBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtClearTimeoutBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtSetTimeoutBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtSetItervalBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError getThunderTokenBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtHttpGetBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtReadBinaryBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtSetVideoStartTimeBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtJSRuntimeDownloadMetrics(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtSetExternalAppHandlerBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtGetRandomValuesBinding(int numArgs, const rtValue* args, rtValue* result, void* context);
rtError rtInstallTimeout(int numArgs, const rtValue* args, rtValue* result, bool repeat);
JSValueRef requireCallback(JSContextRef ctx, JSObjectRef, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception);

#endif /* JAVASCRIPTMISC_H */
