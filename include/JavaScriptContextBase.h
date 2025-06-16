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

#ifndef JAVASCRIPTCONTEXTBASE_H
#define JAVASCRIPTCONTEXTBASE_H
#include <unistd.h>
#include <errno.h>

#include <algorithm>
#include <string>
#include <vector>
#include <utility>
#include <KeyListener.h>

#include <IJavaScriptContext.h>
#include <IJavaScriptEngine.h>
#include <ModuleSettings.h>

//#include <JavaScriptCore/JavaScript.h>

//extern "C" JS_EXPORT void JSSynchronousGarbageCollectForDebugging(JSContextRef);

struct JavaScriptContextFeatures
{
    JavaScriptContextFeatures(bool embedThunderJS, bool embedWebBridge, bool enableWebSockerServer, ModuleSettings& moduleSettings);
    bool mEmbedThunderJS;
    bool mEmbedWebBridge;
    bool mEnableWebSockerServer;
    ModuleSettings mModuleSettings;
};

class JavaScriptContextBase:public IJavaScriptContext, public JavaScriptKeyListener
{
  public:
    JavaScriptContextBase(JavaScriptContextFeatures& features, std::string url, IJavaScriptEngine* jsEngine);
    virtual ~JavaScriptContextBase();
    virtual bool runScript(const char *script, bool isModule=true, std::string name="", const char *args = nullptr, bool isApplication=false);
    virtual bool runFile(const char *file, const char* args, bool isApplication=false);
    std::string getUrl();
    virtual void onKeyPress(struct JavaScriptKeyDetails& details);
    virtual void onKeyRelease(struct JavaScriptKeyDetails& details);
  protected:
    virtual void processKeyEvent(struct JavaScriptKeyDetails& details, bool keyPress) = 0;
    virtual bool evaluateScript(const char* script, const char* name, const char *args, bool module=false) = 0;
    void registerCommonUtils();
    std::string readFile(const char *file);
    static std::string sThunderJSCode;
    static std::string sWebBridgeCode;
    std::string mApplicationUrl;
    IJavaScriptEngine* mEngine;
    bool mEmbedThunderJS;
    bool mEmbedWebBridge;
    bool mEnableWebSockerServer;
    ModuleSettings mModuleSettings;

    static std::string sModulesPath;
    static std::string getModulesPath();

};
#endif
