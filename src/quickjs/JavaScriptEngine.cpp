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

#include "JavaScriptEngine.h"
#include "JavaScriptContext.h"
#include <gst/gst.h>
#include <uv.h>
#include <glib.h>

GMainLoop *gMainLoop = nullptr;

JavaScriptEngine::JavaScriptEngine(): mInspectorEnabled(false), mRuntime(nullptr)
{
}

JavaScriptEngine::~JavaScriptEngine()
{
}

bool JavaScriptEngine::initialize()
{
    gst_init(0, NULL);
    mRuntime = JS_NewRuntime();
    if (!gMainLoop && g_main_depth() == 0)
    {
        gMainLoop = g_main_loop_new (nullptr, false);
    }
    return true;
}

bool JavaScriptEngine::terminate()
{
    gst_deinit();
    JS_FreeRuntime(mRuntime);
    mContexts.clear();
    return true;
}

void JavaScriptEngine::run()
{
  static bool isProcessing = false;
  if (isProcessing)
    return;
  isProcessing = true;

  uv_run(uv_default_loop(), UV_RUN_NOWAIT);
  if (gMainLoop && g_main_depth() == 0)
  {
      gboolean ret;
      do
      {
          ret = g_main_context_iteration(nullptr, false);
      }   while(ret);
  }
  for (int i=0; i<mContexts.size(); i++)

  {
      JavaScriptContext* context = (JavaScriptContext*) mContexts[i];
      if (context)
      {
          context->run();
      }
  }
  isProcessing = false;
}

void JavaScriptEngine::collectGarbage()
{
}

JSRuntime* JavaScriptEngine::getRuntime()
{
    return mRuntime;	
}

void JavaScriptEngine::addContext(IJavaScriptContext* context)
{
    mContexts.push_back(context);
}

void JavaScriptEngine::removeContext(IJavaScriptContext* context)
{
    int index = -1;
    bool found = false;
    for (int i=0; i<mContexts.size(); i++)
    {
        if (mContexts[i] == context)
        {
            index = i;
            found = true;
            break;
        }
    }

    if (found)
    {
        mContexts.erase(mContexts.begin()+index);
    }
}
