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
#include "JavaScriptWrapper.h"
#include "rtLog.h"

#ifdef USE_JSCLIB_MOCK
#include "jsc_lib_mock.h"
#else
#include "jsc_lib.h"
#endif

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <algorithm>
#include <cassert>
#include <chrono>
#include <random>
#include <fstream>
#include <iostream>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <queue>
#include <sstream>
#include <thread>
#include <rtObject.h>
#include <rtWebSocket.h>
#include <uv.h>
#ifdef WS_SERVER_ENABLED
#include <rtWebSocketServer.h>
#endif
#ifdef ENABLE_JSRUNTIME_THUNDER_SECURITYAGENT
#include <securityagent/securityagent.h>
#endif
#include "rtHttpRequest.h"
#include "rtHttpResponse.h"

#include <glib.h>

#include <uv.h>
#include <JavaScriptContext.h>
#define MAX_TOKEN_BUFFER_LENGTH 2048

std::thread::id gMainThreadId;

static std::list<std::function<void ()>> gPendingFun;
static std::mutex gDispatchMutex;
static const char* envValue = std::getenv("NATIVEJS_DUMP_NETWORKMETRIC");

void TimeoutQueue::pushTimeouts(const std::vector<TimeoutInfo*>& timerVec)
{
    if (!timerVec.size())
        return;
    this->c.reserve(this->c.size() + timerVec.size());
    this->c.insert(this->c.end(), timerVec.begin(), timerVec.end());
    std::make_heap(this->c.begin(), this->c.end(), this->comp);
}

bool TimeoutQueue::updateForInfo(const TimeoutInfo* info)
{
    auto it = std::find(this->c.begin(), this->c.end(), info);
    if (it != this->c.end()) {
        std::make_heap(this->c.begin(), this->c.end(), this->comp);
        return true;
    }
    return false;
}

static std::map<uint32_t, TimeoutInfo*> gTimeoutMap;
static uint32_t gTimeoutIdx = 0;
static TimeoutQueue gTimeoutQueue;

void printException(JSContextRef ctx, JSValueRef exception)
{
  JSStringRef exceptStr = JSValueToStringCopy(ctx, exception, nullptr);
  rtString errorStr = jsToRtString(exceptStr);
  JSStringRelease(exceptStr);
  rtLogError("Got Exception: %s", errorStr.cString());
}

rtString jsToRtString(JSStringRef str)
{
  if (!str)
    return rtString();
  size_t len = JSStringGetMaximumUTF8CStringSize(str);
  std::unique_ptr<char[]> buffer(new char[len]);
  len = JSStringGetUTF8CString(str, buffer.get(), len);
  return rtString(buffer.get(), len); // does a copy
}

void dispatchPending()
{
  std::unique_lock<std::mutex> lock(gDispatchMutex);
  std::list<std::function<void ()>> pending = std::move(gPendingFun);
  lock.unlock();
  for(auto& fun : pending)
    fun();
}

void dispatchOnMainLoop(std::function<void ()>&& fun)
{
  std::unique_lock<std::mutex> lock(gDispatchMutex);
  gPendingFun.push_back(std::move(fun));
}

void dispatchTimeouts()
{
  const auto currentTime = std::chrono::steady_clock::now();

  std::vector<TimeoutInfo*> timeoutsToRepeat;
  while(!gTimeoutQueue.empty()) {
    TimeoutInfo* info = gTimeoutQueue.top();
    if (!info->canceled && info->fireTime > currentTime) {
      break;
    }
    gTimeoutQueue.pop();
    if (info->canceled) {
      delete info;
      continue;
    }
    int rc = info->callback();
    if (rc != 0 || !info->repeat || info->canceled) {
      if (!info->canceled)
        gTimeoutMap.erase(info->tag);
      delete info;
      continue;
    }
    info->fireTime = currentTime + info->interval;
    timeoutsToRepeat.push_back(info);
  }
  gTimeoutQueue.pushTimeouts(timeoutsToRepeat);
}

void assertIsMainThread()
{
  assert(std::this_thread::get_id() == gMainThreadId);
}

  rtHttpRequestEx::rtHttpRequestEx(const rtString& url)
    : rtHttpRequest(url)
  {
  }

  rtHttpRequestEx::rtHttpRequestEx(const rtObjectRef& options)
    : rtHttpRequest(options)
  {
  }

  void rtHttpRequestEx::onDownloadProgressImpl(double progress) 
  {

    AddRef();
      dispatchOnMainLoop(
        [this, downloadedsize = progress] ()
        {
          rtObjectRef e = new rtMapObject;
          e.set("progress", downloadedsize);

          mEmit.send("loadprogress", e);
          Release();
        });
  }

  void rtHttpRequestEx::onDownloadCompleteImpl(rtFileDownloadRequest* downloadRequest) 
  {
    AddRef();
    if (!downloadRequest->errorString().isEmpty()) {
      dispatchOnMainLoop(
        [this, errorString = downloadRequest->errorString(), statusCode = downloadRequest->downloadStatusCode()] ()
	{
          if (statusCode == 28)
          {
            mEmit.send("error", "TIMEDOUT");
          }
	  else
	  {
            mEmit.send("error", errorString);
	  }
          Release();
        });
    } else {
      rtHttpResponse* resp = new rtHttpResponse();
      resp->setStatusCode((int32_t)downloadRequest->httpStatusCode());
      resp->setErrorMessage(downloadRequest->errorString());
      resp->setHeaders(downloadRequest->headerData(), downloadRequest->headerDataSize());
      resp->setDownloadedData(downloadRequest->downloadedData(), downloadRequest->downloadedDataSize());

      if (mMetricsListener && envValue)
      {
 	rtLogWarn("metriclistener");
        NetworkMetrics *metrics = new NetworkMetrics();
        if (!metrics){
            rtLogError("Failed to allocate NetworkMetrics");
            return;
        }
	metrics->url = this->url();
	metrics->method = this->method();
	std::vector<rtString> headerValues = this->headers();
	metrics->headers = headerValues;
       metrics->statusCode = downloadRequest->httpStatusCode();

	rtObjectRef timeMetrics = downloadRequest->downloadMetrics();
        rtValue keys;
        if (timeMetrics->Get("allKeys", &keys) == RT_OK) {
            rtArrayObject* keysArray = (rtArrayObject*) keys.toObject().getPtr();
            for (size_t i = 0; i < keysArray->length(); ++i) {
                rtString key = keysArray->get<rtString>(i);
                rtValue value;
                if (timeMetrics->Get(key, &value) == RT_OK) {
                    metrics->timeMetricsData[key] = value;
        	}
    	    }
	}

	mMetricsListener->onMetricsData(metrics);
	delete metrics;
	metrics = nullptr;
      }

      rtObjectRef protectedRef = resp;
      dispatchOnMainLoop(
        [this, resp = resp, ref = protectedRef] () {
          mEmit.send("response", ref);
          resp->onData();
          resp->onEnd();
          Release();
        });
       }
     }

rtDefineObject(rtHttpRequestEx, rtHttpRequest);

rtError rtHttpGetBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  UNUSED_PARAM(context);
  if (numArgs < 1) {
    rtLogError("%s: invalid args", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  JavaScriptContext* jscContext = (JavaScriptContext*)context;
  if (!jscContext) {
    rtLogError("context is null!");
  }

  rtHttpRequest* req;
  if (args[0].getType() == RT_stringType) {
    req = new rtHttpRequestEx(args[0].toString());
  }
  else {
    if (args[0].getType() != RT_objectType) {
      rtLogError("%s: invalid arg type", __FUNCTION__);
      return RT_ERROR_INVALID_ARG;
    }
    rtLogInfo("new rtHttpRequest");
    req = new rtHttpRequestEx(args[0].toObject());
  }

  if (numArgs > 1 && args[1].getType() == RT_functionType) {
    req->addListener("response", args[1].toFunction());
  }

  req->setNetworkMetricsListener(jscContext);

  rtObjectRef ref = req;
  *result = ref;
  return RT_OK;
}

rtError rtSetVideoStartTimeBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    if (context == nullptr || numArgs < 1) {
        rtLogError("Invalid context or missing arguments");
        return RT_ERROR_INVALID_ARG;
    }

    JavaScriptContext* jscContext = (JavaScriptContext*)context;

    if (args[0].getType() != RT_intType && args[0].getType() != RT_doubleType) {
    	rtLogError("%s: invalid argument type", __FUNCTION__);
    	return RT_ERROR_INVALID_ARG;
    }
    double playbackStartTime = args[0].toDouble();
    jscContext->setPlaybackStartTime(playbackStartTime);

    if (result)
        *result = true;

    return RT_OK;
}

rtError rtReadBinaryBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  char *buffer = nullptr;
  FILE *ptr = nullptr;
  ptr = fopen("hello.wasm","rb");  // r for read, b for binary

  const char *fd = "hello.wasm";
  struct stat buf;

  if (stat(fd, &buf) != 0)
  {
    rtLogError("Failed to stat file: %s", fd);
    fclose(ptr);
    return RT_ERROR;
  }
  
  int size = buf.st_size;

  buffer = (char*)malloc(size);
  size_t bytesRead = fread(buffer, size, 1, ptr);
  fclose(ptr);

  if (bytesRead != 1)
  {
    rtLogError("Failed to read file: expected 1 item, read %zu items", bytesRead);
    free(buffer);
    return RT_ERROR;
  }

  if (result)
  {
      result->setString(buffer);
  }

  free(buffer);
  return RT_OK;
}

rtError rtWebSocketBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  UNUSED_PARAM(context);
  if (numArgs < 1) {
    rtLogError("%s: is invalid args", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  if (args[0].getType() != RT_objectType) {
    rtLogError("%s: is invalid arg type ", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  rtObjectRef ref = new rtWebSocket(args[0].toObject());
  *result = ref;

  return RT_OK;
}
#ifdef WS_SERVER_ENABLED
std::vector<rtIObject*> gWebSocketServers;
rtError rtWebSocketServerBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  UNUSED_PARAM(context);
  if (numArgs < 1) {
    rtLogError("%s: invalid args", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  if (args[0].getType() != RT_objectType) {
    rtLogError("%s: invalid arg type", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  rtObjectRef ref = new rtWebSocketServer(args[0].toObject());
  *result = ref;
  gWebSocketServers.push_back(ref.getPtr());
  return RT_OK;
}
#endif
rtError rtInstallTimeout(int numArgs, const rtValue* args, rtValue* result, bool repeat)
{
  if (numArgs < 1) {
    rtLogError("%s: invalid args", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  if (args[0].getType() != RT_functionType) {
    rtLogError("%s: invalid arg type", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }

  double interval = 0;
  if (numArgs >= 2)
    interval = args[1].toDouble();

  rtFunctionRef timeoutCb = args[0].toFunction();

  std::vector<rtValue> timeoutArgs;
  if (numArgs > 2) {
    timeoutArgs.reserve(numArgs - 2);
    for (int i = 2; i < numArgs; ++i)
      timeoutArgs.push_back(args[i]);
  }

  uint32_t tag = installTimeout(interval, repeat,
    [timeoutCbRef = std::move(timeoutCb), timeoutArgs = std::move(timeoutArgs)] () mutable {
      rtError rc = timeoutCbRef.rtFunctionBase::Send(timeoutArgs.size(), timeoutArgs.data());
      if (rc != RT_OK) {
        rtLogError("timer callback send failed, rc = %d", rc);
        return 1;
      }
      return 0;
    });

  if (result)
    *result = tag;

  return RT_OK;
}

rtError rtSetItervalBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  UNUSED_PARAM(context);
  return rtInstallTimeout(numArgs, args, result, true);
}

rtError rtSetTimeoutBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  UNUSED_PARAM(context);
  return rtInstallTimeout(numArgs, args, result, false);
}

rtError rtClearTimeoutBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  UNUSED_PARAM(context);
  if (numArgs < 1) {
    rtLogError("%s: invalid args", __FUNCTION__);
    return RT_ERROR_INVALID_ARG;
  }
  if (args[0].isEmpty()) {
    rtLogWarn("%s: cannot remove time for 'null' or 'undefined' tag", __FUNCTION__);
    return RT_OK;
  }
  uint32_t tag = args[0].toUInt32();
  clearTimeout(tag);
  return RT_OK;
}

rtError getThunderTokenBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  #ifdef ENABLE_JSRUNTIME_THUNDER_SECURITYAGENT
  JavaScriptContext* jsccontext = (JavaScriptContext*) context;
  if (!jsccontext)
  {
      rtLogError("lost context !!!");
      return RT_ERROR;
  }
  if (result)
  {
      result->setString("");
  }
  else
  {
      rtLogError("lost return value !!!");
      return RT_ERROR;
  }
  unsigned char tokenBuffer[MAX_TOKEN_BUFFER_LENGTH];
  memset(tokenBuffer, 0, MAX_TOKEN_BUFFER_LENGTH);
  std::string appUrl = jsccontext->getUrl();
  rtString params(appUrl.c_str());
  size_t paramsLength = (size_t)params.length();
  if(!memcpy(tokenBuffer,params.cString(),paramsLength))
  {
    rtLogError("unable to copy url buffer for token");
    return RT_FAIL;
  }

  rtLogInfo("thunder request: %s length: %d", (char*)tokenBuffer, (int)paramsLength);
  int tokenResult = GetToken(MAX_TOKEN_BUFFER_LENGTH, paramsLength, tokenBuffer);
  if (tokenResult < 0)
  {
    rtLogError("unable to get token for app");
    return RT_FAIL;
  }
  rtString tokenString = (const char*) tokenBuffer;

  if (nullptr != result)
  {
    result->setString(tokenString);
  }
  return RT_OK;
  #else
  {
    rtLogError("thunder security agent is disabled");
    return RT_OK;
  }
  #endif
}

void clearTimeout(uint32_t tag)
{
  auto it = gTimeoutMap.find(tag);
  if (it != gTimeoutMap.end()) {
    TimeoutInfo* info = it->second;
    constexpr auto steadyMinimal = std::chrono::steady_clock::time_point::min();
    info->fireTime = steadyMinimal;
    info->canceled = true;
    gTimeoutMap.erase(it);
    gTimeoutQueue.updateForInfo(info);
  }
}

uint32_t installTimeout(double intervalMs, bool repeat, std::function<int ()>&& fun)
{
  if (intervalMs < 0)
    intervalMs = 0;

  auto currentTime = std::chrono::steady_clock::now();
  auto interval = std::chrono::milliseconds(static_cast<uint32_t>(intervalMs));

  TimeoutInfo *info = new TimeoutInfo;
  info->interval = interval;
  info->fireTime = currentTime + info->interval;
  info->repeat = repeat;
  info->callback = std::move(fun);
  info->canceled = false;
  info->tag = ++gTimeoutIdx;  // FIXME:

  gTimeoutMap[info->tag] = info;
  gTimeoutQueue.push(info);
  return info->tag;
}

static std::string readFile(const char *file)
{
  std::ifstream       src_file(file);
  std::stringstream   src_script;
  src_script << src_file.rdbuf();
  return src_script.str();
}

static bool fileExists(const char* name)
{
  struct stat buffer;
  bool ret = (stat (name, &buffer) == 0);
  return ret;
}

JSValueRef requireCallback(JSContextRef ctx, JSObjectRef, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef *exception)
{
  if (argumentCount != 1)
      return JSValueMakeNull(ctx);

  static const auto resolveModulePath = [](const rtString &name, rtString &data) -> bool
  {
    std::list<rtString> dirs;
    std::list<rtString> endings;
    bool found = false;
    rtString path;

    dirs.push_back(""); // this dir
    dirs.push_back("/home/root/"); // this dir
    dirs.push_back("modules/");
    dirs.push_back("modules/lib");
    dirs.push_back("/home/root/modules/");
    dirs.push_back("/home/root/modules/lib");
    endings.push_back(".js");

    std::list<rtString>::const_iterator current, next;
    for (current = dirs.begin(); !found && current != dirs.end(); ++current) {
      rtString s = *current;
      if (!s.isEmpty() && !s.endsWith("/"))
        s.append("/");
      s.append(name.beginsWith("./") ? name.substring(2) : name);
      for (next = endings.begin(); !found && next != endings.end(); ++next) {
        path = s;
        if (!path.endsWith((*next).cString()))
          path.append(*next);
        found = fileExists(path.cString());
      }
    }

    if (found)
      data = path;
    return found;
  };

  do {
    JSStringRef reqArgStr = JSValueToStringCopy(ctx, arguments[0], exception);
    if (exception && *exception)
      break;

    rtString moduleName = jsToRtString(reqArgStr);
    rtString path;
    if (!resolveModulePath(moduleName, path)) {
      JSStringRelease(reqArgStr);
      rtLogError("Module '%s' not found", moduleName.cString());
      break;
    }

    JSGlobalContextRef globalCtx = JSContextGetGlobalContext(ctx);
    rtJSCContextPrivate* priv = rtJSCContextPrivate::fromCtx(globalCtx);
    if (!priv) {
      rtLogError(" %s  ... no priv object.",__PRETTY_FUNCTION__);
      break;
    }

    if (JSObjectRef moduleObj = priv->findModule(path)) {
      JSStringRelease(reqArgStr);
      return moduleObj;
    }

    rtLogInfo("Loading %s", path.cString());
    std::string codeStr = readFile(path.cString());
    if (codeStr.empty()) {
      JSStringRelease(reqArgStr);
      rtLogError(" %s  ... load error / not found.",__PRETTY_FUNCTION__);
      break;
    }

    codeStr =
        "(function(){ let m = {}; m.exports = {}; \n"
        "  (function(module, exports){\n"
        + codeStr +
        "  \n}).call(undefined, m, m.exports); return m;})()";

    JSStringRef jsstr = JSStringCreateWithUTF8CString(codeStr.c_str());
    JSValueRef module = JSEvaluateScript(globalCtx, jsstr, nullptr, reqArgStr, 0, exception);
    JSStringRelease(jsstr);
    JSStringRelease(reqArgStr);

    if (exception && *exception) {
      JSStringRef exceptStr = JSValueToStringCopy(globalCtx, *exception, nullptr);
      rtString errorStr = jsToRtString(exceptStr);
      JSStringRelease(exceptStr);
      rtLogError("Failed to eval, \n\terror='%s'\n\tmodule=%s\n\tscript='...'", errorStr.cString(), path.cString());
      break;
    }

    JSObjectRef moduleObj = JSValueToObject(globalCtx, module, exception);
    if (exception && *exception) {
      JSStringRef exceptStr = JSValueToStringCopy(globalCtx, *exception, nullptr);
      rtString errorStr = jsToRtString(exceptStr);
      JSStringRelease(exceptStr);
      rtLogError("Failed to convert module to object, \n\terror='%s'\n\tmodule=%s", errorStr.cString(), path.cString());
      break;
    }

    static JSStringRef exportsStr = JSStringCreateWithUTF8CString("exports");
    JSValueRef exportsVal = JSObjectGetProperty(globalCtx, moduleObj, exportsStr, exception);
    if (exception && *exception) {
      JSStringRef exceptStr = JSValueToStringCopy(globalCtx, *exception, nullptr);
      rtString errorStr = jsToRtString(exceptStr);
      JSStringRelease(exceptStr);
      rtLogError("Failed to get exports module to object, \n\terror='%s'\n\tmodule=%s", errorStr.cString(), path.cString());
      break;
    }

    JSObjectRef exportsObj = JSValueToObject(globalCtx, exportsVal, exception);
    if (exception && *exception) {
      printException(globalCtx, *exception);
      break;
    }
    priv->addToModuleCache(path, globalCtx, exportsObj);
    return exportsVal;
  } while(0);

  return JSValueMakeNull(ctx);
}

char* JSValueToCString(JSContextRef context, JSValueRef value, JSValueRef* exception)
{
        JSStringRef jsstr = JSValueToStringCopy(context, value, exception);
        size_t len = JSStringGetMaximumUTF8CStringSize(jsstr);
        char* src = new char[len];
        JSStringGetUTF8CString(jsstr, src, len);
        JSStringRelease(jsstr);
        return src;
}

rtError rtJSRuntimeDownloadMetrics(int numArgs, const rtValue* args, rtValue* result, void* context)
{
  JavaScriptContext* jscContext = (JavaScriptContext*)context;
  if (jscContext == nullptr) {
    rtLogError("context null");
    return RT_FAIL;
  }

  rtMapObject* map = jscContext->getNetworkMetricsData();
  if (!map) {
    return RT_FAIL;
  }

  rtArrayObject* netMetricsArray = new rtArrayObject();

  rtValue keys;
  if (map->Get("allKeys", &keys) != RT_OK) {
    rtLogWarn("Could not retrieve url for network metrics data.");
    delete netMetricsArray; //newly added
    return RT_FAIL;
  }
  rtObjectRef objRef = keys.toObject();
  rtArrayObject* keysArray = static_cast<rtArrayObject*>(objRef.getPtr());

  if (!keysArray) {
    rtLogWarn("No url found in the network metrics data.");
    delete netMetricsArray; //newly added
    return RT_FAIL;
  }

  size_t count = keysArray->length();

  for (size_t i = 0; i < count; ++i) {
    rtValue keyValue;
    rtString key = keysArray->get<rtString>(i);

    keysArray->Get(key, &keyValue);
    rtValue storedValue;
    if (map->Get(key.cString(), &storedValue) == RT_OK) {
      NetworkMetrics* metrics = (NetworkMetrics*)storedValue.toVoidPtr();
      if (!metrics) {
        rtLogError("Failed to cast stored value to NetworkMetrics structure for url: %s.", key.cString());
        delete netMetricsArray; //newly added
        return RT_FAIL;
      }
      rtMapObject* metricsMap = new rtMapObject();
      metricsMap->set("url", metrics->url);
      metricsMap->set("method", metrics->method);

      rtArrayObject* headersArray = new rtArrayObject();
      for (const auto& header : metrics->headers) {
        headersArray->pushBack(rtValue(header));
      }
      metricsMap->set("headers", rtValue(headersArray));
      delete headersArray;

      rtArrayObject* timeMetricsArray = new rtArrayObject();
      for (const auto& metric : metrics->timeMetricsData) {
        rtObjectRef timeMetricObj = new rtMapObject();
        timeMetricObj->Set(metric.first.cString(), &metric.second);
        timeMetricsArray->pushBack(rtValue(timeMetricObj));
	delete timeMetricObj;
	timeMetricObj = nullptr;
      }

      metricsMap->set("timeMetricsData", rtValue(timeMetricsArray));
      delete timeMetricsArray;

      netMetricsArray->pushBack(rtValue(metricsMap));
    }
    else {
      rtLogWarn("Url not found in network metrics data: %s", key.cString());
    }
  }

  *result = rtValue(netMetricsArray);

  return RT_OK;
}

rtError rtSetExternalAppHandlerBinding(int numArgs, const rtValue* args, rtValue* result, void* context)
{
    if (context == nullptr || numArgs < 1) {
        rtLogError("Invalid context or missing arguments");
        return RT_ERROR_INVALID_ARG;
    }

    JavaScriptContext* jscContext = (JavaScriptContext*)context;

    if (args[0].getType() != RT_stringType) {
        rtLogError("Requires a string argument");
        return RT_ERROR_INVALID_ARG;
    }

    rtString url = args[0].toString();
    jscContext->handleExternalApplication(url.cString());

    if (result)
        *result = true;

    return RT_OK;
}

rtError rtGetRandomValuesBinding(int numArgs, const rtValue* args, rtValue* /*result*/, void* /*context*/) 
{
    if (numArgs < 1 || args[0].getType() != RT_objectType) {
        rtLogError("rtGetRandomValuesBinding: Invalid arguments");
        return RT_ERROR_INVALID_ARG;
    }

    rtObjectRef arrObj = args[0].toObject();
    
    uint32_t len = arrObj.get<uint32_t>("length");
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<uint32_t> dist;
    
    for (uint32_t i = 0; i < len; i++) {
        arrObj.set(i, (double)dist(gen)); 
    }
    return RT_OK;
}
