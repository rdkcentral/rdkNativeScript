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

#include <JavaScriptContextBase.h>
#include <fstream>
#include <iostream>
#include <sstream>
#ifdef ENABLE_ESSOS
#include <EssosInstance.h>
#endif

std::string JavaScriptContextBase::sThunderJSCode = "";
std::string JavaScriptContextBase::sWebBridgeCode = "";

JavaScriptContextFeatures::JavaScriptContextFeatures(bool embedThunderJS, bool embedWebBridge, bool enableWebSockerServer, ModuleSettings& moduleSettings):mEmbedThunderJS(embedThunderJS), mEmbedWebBridge(embedWebBridge), mEnableWebSockerServer(enableWebSockerServer), mModuleSettings(moduleSettings)
{
}

JavaScriptContextBase::JavaScriptContextBase(JavaScriptContextFeatures& features, std::string url, IJavaScriptEngine* jsEngine): mApplicationUrl(url), mEngine(jsEngine), mEmbedThunderJS(features.mEmbedThunderJS), mEmbedWebBridge(features.mEmbedWebBridge), mEnableWebSockerServer(features.mEnableWebSockerServer), mModuleSettings(features.mModuleSettings)
{
    if (mEmbedThunderJS)
    {
        if (sThunderJSCode.empty())
        {		
            sThunderJSCode = readFile("modules/thunderJS.js");
        }
    }
    if (mEmbedWebBridge)
    {
        if (sWebBridgeCode.empty())
        {		
            sWebBridgeCode = readFile("modules/webbridgesdk.js");
        }
    }
#ifdef ENABLE_ESSOS
    EssosInstance::instance()->registerKeyListener(this);
#endif
}

JavaScriptContextBase::~JavaScriptContextBase()
{
    mEngine = nullptr;
}

void JavaScriptContextBase::registerCommonUtils()
{
    if (mEmbedThunderJS)
    {
        std::cout << "executing thunder js code " << std::endl;
        runScript(sThunderJSCode.c_str());
    }
    if (mEmbedWebBridge)
    {
        std::cout << "executing rdk webbridge js code " << std::endl;
        runScript(sWebBridgeCode.c_str());
    }
}

std::string JavaScriptContextBase::readFile(const char *file)
{
    std::ifstream       src_file(file);
    std::stringstream   src_script;
    src_script << src_file.rdbuf();
    return src_script.str();
}

bool JavaScriptContextBase::runFile(const char *file, const char* args, bool isApplication)
{
    if (!file)
    {
        printf(" %s  ... no script given.",__PRETTY_FUNCTION__);
        fflush(stdout);
        return false;
    }

    std::string scriptToRun;
    scriptToRun = readFile(file);
    if(scriptToRun.empty())
    {
        std::string fileName("/home/root/");
	fileName.append(file);
        scriptToRun = readFile(fileName.c_str());
        printf("checking in [%s] \n", fileName.c_str());
        if(scriptToRun.empty())
        {
            printf(" %s  ... load error / not found. %s",__PRETTY_FUNCTION__, file);
            fflush(stdout);
            return false;
        }
    }
    return evaluateScript(scriptToRun.c_str(), isApplication?file:nullptr, args, isApplication);
}

bool JavaScriptContextBase::runScript(const char *script, bool isModule, std::string name, const char *args, bool isApplication)
{
    return evaluateScript(script, isApplication?name.c_str():nullptr, args, isModule);
}

std::string JavaScriptContextBase::getUrl()
{ 
    return mApplicationUrl;
}

void JavaScriptContextBase::onKeyPress(struct JavaScriptKeyDetails& details)
{
    processKeyEvent(details, true);
}

void JavaScriptContextBase::onKeyRelease(struct JavaScriptKeyDetails& details)
{
    processKeyEvent(details, false);
}
