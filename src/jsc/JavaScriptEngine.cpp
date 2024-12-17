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

#include <JavaScriptCore/JSExportMacros.h>
#ifdef REMOTE_INSPECTOR_ENABLED
#include <JavaScriptCore/RemoteInspectorServer.h>
#endif
#include <unistd.h>
#include <errno.h>

#include <sstream>
#include <string>
#include <vector>
#include <utility>

#define USE(WTF_FEATURE) (defined USE_##WTF_FEATURE  && USE_##WTF_FEATURE)
#define ENABLE(WTF_FEATURE) (defined ENABLE_##WTF_FEATURE  && ENABLE_##WTF_FEATURE)
#define JS_EXPORT_PRIVATE 
#include <JavaScriptCore/JavaScript.h>

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

#ifdef REMOTE_INSPECTOR_ENABLED
#ifdef __cplusplus
extern "C" {
#endif
  JS_EXPORT void JSRemoteInspectorStart(void);
  JS_EXPORT void JSRemoteInspectorSetLogToSystemConsole(bool logToSystemConsole);
  JS_EXPORT void JSRemoteInspectorSetInspectionEnabledByDefault(bool);
#ifdef __cplusplus
}
#endif

extern std::vector<rtIObject*> gWebSocketServers;
#endif
namespace WTF {
void initializeMainThread();
};

JavaScriptEngine::JavaScriptEngine(): mInspectorEnabled(false)
{
}

JavaScriptEngine::~JavaScriptEngine()
{
}

bool JavaScriptEngine::initialize()
{
    char* disableGstStartValue = getenv("NATIVEJS_GST_START_DISABLE");
    if (disableGstStartValue)
    {
        sDisableGstStart = true;
        std::cout << "disabled gst start " << std::endl;
    }

    if (!sDisableGstStart)
    {	    
      gst_init(0, nullptr);
    }
  WTF::initializeMainThread();
  if (!gMainLoop && g_main_depth() == 0) {
    gMainLoop = g_main_loop_new (nullptr, false);
  }
#ifdef REMOTE_INSPECTOR_ENABLED
  char* inspectorDetails = getenv("NATIVEJS_INSPECTOR_SERVER");
  if (inspectorDetails)
  {
    std::string host("0.0.0.0");
    uint32_t port = 9226;
    bool isFailedParsing = false;
    std::string details(inspectorDetails);
    int portIndex = details.find(":");
    if (portIndex != -1)
    {
        port = atoi(details.substr(portIndex+1).c_str());
        host = details.substr(0, portIndex);
    }
    else
    {
        isFailedParsing = true;
        std::cout << "failed to start remote inspector server due to parsing issues" << std::endl;
    }		
    if (!isFailedParsing)
    {	    
        std::stringstream serverDetails;
	serverDetails << host.c_str() << ":" << port;
        setenv("WEBKIT_INSPECTOR_SERVER", serverDetails.str().c_str(),1);
        Inspector::RemoteInspectorServer::singleton().start(host.c_str(), port);
        JSRemoteInspectorStart();
        JSRemoteInspectorSetInspectionEnabledByDefault(true);
        JSRemoteInspectorSetLogToSystemConsole(true);
    }
  }
#endif
  gMainThreadId = std::this_thread::get_id();
  return true;
}

bool JavaScriptEngine::terminate()
{
  if (!sDisableGstStart)
  {	    
      gst_deinit();
  }
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
    JSSynchronousGarbageCollectForDebugging(gTopLevelContext);
  }
}
