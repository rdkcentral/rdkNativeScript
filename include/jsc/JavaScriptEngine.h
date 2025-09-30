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

#ifndef JAVASCRIPTENGINE_H
#define JAVASCRIPTENGINE_H

#include <unistd.h>
#include <errno.h>
#include <cstdint>

//#include <algorithm>
#include <string>
#include <vector>
#include <utility>

#include <JavaScriptCore/JavaScript.h>
#include <IJavaScriptEngine.h>
#include <IJavaScriptContext.h>

class JavaScriptEngine:public IJavaScriptEngine
{
  public:
    JavaScriptEngine();
    virtual ~JavaScriptEngine();
  
    bool initialize() override;
    bool terminate() override;
    void run() override;
    void collectGarbage() override;

  private:
    bool mInspectorEnabled;
    uint32_t mGarbageCollectionTag;
};
#endif
