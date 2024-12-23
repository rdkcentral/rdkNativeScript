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

#ifndef JAVASCRIPTCONTEXT_H
#define JAVASCRIPTCONTEXT_H

#include <unistd.h>
#include <errno.h>

#include <algorithm>
#include <string>
#include <vector>
#include <utility>

#include <quickjs.h>
#include <JavaScriptContextBase.h>
#include <KeyListener.h>
#include <KeyInput.h>

class JavaScriptContext: public JavaScriptContextBase
{
  public:
    JavaScriptContext(JavaScriptContextFeatures& features, std::string url, IJavaScriptEngine* jsEngine);
    virtual ~JavaScriptContext();
    JSContext* getContext() { return mContext; }
    void run();

  private:
    virtual bool evaluateScript(const char* script, const char* name, const char *args, bool module=false);
    virtual void processKeyEvent(struct JavaScriptKeyDetails& details, bool keyPress);
    JSContext* mContext;  
    JSRuntime *mRuntime;
};
#endif
