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

#include <unistd.h>
#include <errno.h>

#include <sstream>
#include <string>
#include <vector>
#include <utility>

#define USE(WTF_FEATURE) (defined USE_##WTF_FEATURE  && USE_##WTF_FEATURE)
#define ENABLE(WTF_FEATURE) (defined ENABLE_##WTF_FEATURE  && ENABLE_##WTF_FEATURE)

#include <JavaScriptCore/JavaScript.h>
#ifdef REMOTE_INSPECTOR_ENABLE
#include <InspectorHTTPServer.h>
#endif
#include <NativeJSLogger.h>

#include "JavaScriptEngine.h"
#include "JavaScriptUtils.h"
#include <glib.h>
#include <uv.h>
#ifdef WS_SERVER_ENABLED
#include <rtWebSocketServer.h>
#endif
#include <gst/gst.h>

GMainLoop *gMainLoop = nullptr;
static bool sDisableGstStart = false;

extern "C" JS_EXPORT void JSSynchronousGarbageCollectForDebugging(JSContextRef);
extern JSGlobalContextRef gTopLevelContext;
extern std::thread::id gMainThreadId;

#define NATIVEJS_GC_DEFAULT_INTERVAL 60000



extern std::vector<rtIObject*> gWebSocketServers;
namespace WTF {
void initializeMainThread();
};

JavaScriptEngine::JavaScriptEngine(): mInspectorEnabled(false), mGarbageCollectionTag(0)
{
}

JavaScriptEngine::~JavaScriptEngine()
{
}

bool JavaScriptEngine::initialize()
{
#ifdef ENABLE_JSRUNTIME_PLAYER
    char* disableGstStartValue = getenv("NATIVEJS_GST_START_DISABLE");
    if (disableGstStartValue)
    {
        sDisableGstStart = true;
	NativeJSLogger::log(INFO, "disabled gst start\n");
    }

    if (!sDisableGstStart)
    {	    
      gst_init(0, nullptr);
    }
#endif

  setenv("JSC_useBigInt", "1", 1);
  
  WTF::initializeMainThread();
  
  if (!gMainLoop && g_main_depth() == 0) {
    gMainLoop = g_main_loop_new (nullptr, false);
  }
  gMainThreadId = std::this_thread::get_id();
  JavaScriptEngine* engine = this;
  char* garbageCollectInterval = getenv("NATIVEJS_GC_INTERVAL");
  double garbageCollectIntervalValue = NATIVEJS_GC_DEFAULT_INTERVAL;
  if (garbageCollectInterval)
  {
      garbageCollectIntervalValue = atof(garbageCollectInterval);
      NativeJSLogger::log(INFO, "garbage collection interval value: %f\n", garbageCollectIntervalValue);
  }
  mGarbageCollectionTag = installTimeout(garbageCollectIntervalValue, true,
    [engine] () mutable {
      engine->collectGarbage();
      NativeJSLogger::log(INFO, "Collecting Garbage !!!!!!!!!!!!!\n");

      fflush(stdout);
      return 0;
    });

#ifdef REMOTE_INSPECTOR_ENABLE
  // Start the custom web inspector server with HTTP/WebSocket endpoints
  const char* inspectorServer = getenv("NATIVEJS_INSPECTOR_SERVER");
  if (inspectorServer)
  {
    NativeJSLogger::log(INFO, "Starting Web Inspector Server on: %s\n", inspectorServer);
    
    // Parse the address (expecting "0.0.0.0:9226" format)
    char* colonPos = strchr(const_cast<char*>(inspectorServer), ':');
    int port = 9226; // default
    if (colonPos) {
      port = atoi(colonPos + 1);
    }
    
    // Start the server (context will be registered later when created)
    if (InspectorHTTPServer::singleton().start(inspectorServer, port)) {
      mInspectorEnabled = true;
      NativeJSLogger::log(INFO, "Web Inspector Server started successfully\n");
    } else {
      NativeJSLogger::log(ERROR, "Failed to start Web Inspector Server\n");
    }
  }
#endif

  return true;
}

bool JavaScriptEngine::terminate()
{
#ifdef ENABLE_JSRUNTIME_PLAYER
  if (!sDisableGstStart)
  {	    
      gst_deinit();
  }
#endif

#ifdef REMOTE_INSPECTOR_ENABLE
  if (mInspectorEnabled) {
    InspectorHTTPServer::singleton().stop();
    NativeJSLogger::log(INFO, "Web Inspector Server stopped\n");
  }
#endif

  clearTimeout(mGarbageCollectionTag);
  return true;
}

void JavaScriptEngine::run()
{
  static bool isProcessing = false;
  if (isProcessing)
    return;
  isProcessing = true;

  dispatchPending();
  dispatchTimeouts();

  uv_run(uv_default_loop(), UV_RUN_NOWAIT);

  if (gMainLoop && g_main_depth() == 0) {
    gboolean ret;
    do {
      ret = g_main_context_iteration(nullptr, false);
    } while(ret);
  }
  
  dispatchPending();
#ifdef WS_SERVER_ENABLED
  for (int i=0; i<gWebSocketServers.size(); i++)
  {
    rtWebSocketServer* server = (rtWebSocketServer*)gWebSocketServers[i];
    server->poll();
  }
#endif
  isProcessing = false;
}

void JavaScriptEngine::collectGarbage()
{
  if (gTopLevelContext)
  {
    //JSSynchronousGarbageCollectForDebugging(gTopLevelContext);
    JSGarbageCollect(gTopLevelContext);
  }
}

