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
#include <sys/stat.h>                                                                                                                                                       
#include <cstdlib>    

std::string JavaScriptContextBase::sThunderJSCode = "";
std::string JavaScriptContextBase::sWebBridgeCode = "";
std::string JavaScriptContextBase::sModulesPath = "" ;

JavaScriptContextFeatures::JavaScriptContextFeatures(bool embedThunderJS, bool embedWebBridge, bool enableWebSockerServer, ModuleSettings& moduleSettings):mEmbedThunderJS(embedThunderJS), mEmbedWebBridge(embedWebBridge), mEnableWebSockerServer(enableWebSockerServer), mModuleSettings(moduleSettings)
{
}

JavaScriptContextBase::JavaScriptContextBase(JavaScriptContextFeatures& features, std::string url, IJavaScriptEngine* jsEngine): mApplicationUrl(url), mEngine(jsEngine), mEmbedThunderJS(features.mEmbedThunderJS), mEmbedWebBridge(features.mEmbedWebBridge), mEnableWebSockerServer(features.mEnableWebSockerServer), mModuleSettings(features.mModuleSettings)
{
    getModulesPath();
    if (mEmbedThunderJS)
    {
        if (sThunderJSCode.empty())
        {
            sThunderJSCode = readFile("thunderJS.js");
        }
    }
    if (mEmbedWebBridge)
    {
        if (sWebBridgeCode.empty())
        {
            sWebBridgeCode = readFile("webbridgesdk.js");
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
    if(src_script.str().empty())
    {
        std::string fileName=sModulesPath;
        fileName.append(file);
        struct stat path;
        if (stat(fileName.c_str(), &path) == 0) {
            std::cout << "File exists at: " << fileName << std::endl;
            std::ifstream       src_file(fileName.c_str());
            src_script.str("");
            src_script.clear();
            src_script << src_file.rdbuf();
        }
        else {
            std::cout << file << "does not exist at: " << fileName << std::endl;
        }
    }
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
            printf(" %s  ... load error / not found. %s",__PRETTY_FUNCTION__, file);
            fflush(stdout);
            return false;
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

std::string JavaScriptContextBase::getModulesPath(){
    if(!sModulesPath.empty()){
            return sModulesPath;
    }
    else{
            struct stat info;
            std::string home;
            char* cwd = getcwd(nullptr,0);
            std::string PWD=cwd;	    
	        PWD=PWD+"/modules/";
            if (stat(PWD.c_str(), &info) == 0 && (info.st_mode & S_IFDIR)){
                    home = PWD; // "/home/root/modules/"
            }
            else if(stat(PWD.c_str(), &info) == 0 && (info.st_mode & S_IFDIR)){
                    home = PWD; // "/runtime/modules/"
            }
            else if(stat(PWD.c_str(), &info) == 0 && (info.st_mode & S_IFDIR)){
                    home = PWD; 
            }
            sModulesPath = home;
            if(setenv("JSRUNTIME_MODULES_PATH",home.c_str(),1)==0){
                    std::cout<<"JSRUNTIME_MODULES_PATH:"<<std::getenv("JSRUNTIME_MODULES_PATH")<<std::endl;
                    std::cout<<"Modules path variable set successfully"<<std::endl;
            }
            else{
                    std::cout<<"Modules path variable cannot be set!"<<std::endl;
            }
            return sModulesPath;
    }

}