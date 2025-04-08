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
#include <NativeJSLogger.h>
#include <fstream>
#include <iostream>
#include <sstream>
#ifdef ENABLE_ESSOS
#include <EssosInstance.h>
#endif

//newly added
#include <sys/stat.h>
#include <cstdlib>

std::string JavaScriptContextBase::sThunderJSCode = "";
std::string JavaScriptContextBase::sWebBridgeCode = "";

//newly added
std::string JavaScriptContextBase::sModulesPath = "" ;

JavaScriptContextFeatures::JavaScriptContextFeatures(bool embedThunderJS, bool embedWebBridge, bool enableWebSockerServer, ModuleSettings& moduleSettings):mEmbedThunderJS(embedThunderJS), mEmbedWebBridge(embedWebBridge), mEnableWebSockerServer(enableWebSockerServer), mModuleSettings(moduleSettings)
{
}

JavaScriptContextBase::JavaScriptContextBase(JavaScriptContextFeatures& features, std::string url, IJavaScriptEngine* jsEngine): mApplicationUrl(url), mEngine(jsEngine), mEmbedThunderJS(features.mEmbedThunderJS), mEmbedWebBridge(features.mEmbedWebBridge), mEnableWebSockerServer(features.mEnableWebSockerServer), mModuleSettings(features.mModuleSettings)
{
//newly added
    JavaScriptContextBase::setEnvVariable("JSRUNTIME_MODULES_PATH", "");
    
    sModulesPath = std::getenv("JSRUNTIME_MODULES_PATH");
    getModulesPath();

    if (mEmbedThunderJS)
    {
        if (sThunderJSCode.empty())
        {		
            sThunderJSCode = readFile(sModulesPath.c_str());
        }
    }
    if (mEmbedWebBridge)
    {
        if (sWebBridgeCode.empty())
        {		
            sWebBridgeCode = readFile(sModulesPath.c_str());
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
        NativeJSLogger::log(INFO, "Executing Thunder JS code\n");
        runScript(sThunderJSCode.c_str());
    }
    if (mEmbedWebBridge)
    {
        NativeJSLogger::log(INFO, "Executing rdk webbridge JS code\n");
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
        NativeJSLogger::log(WARN, "%s ... no script given.\n", __PRETTY_FUNCTION__);
        fflush(stdout);
        return false;
    }

    std::string scriptToRun;
    scriptToRun = readFile(file);
    if(scriptToRun.empty())
    {
        //newly added
	std::string fileName=sModulesPath;
	fileName.append(file);
        scriptToRun = readFile(fileName.c_str());
        NativeJSLogger::log(INFO, "Checking in [%s]\n", fileName.c_str());
        if(scriptToRun.empty())
        {
            NativeJSLogger::log(ERROR, "%s ... load error / not found. %s\n", __PRETTY_FUNCTION__, file);
            fflush(stdout);
            return false;
        }
    }
    //newly added
    std::cout<<"File:" << file << std::endl;
        
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

//newly added
void JavaScriptContextBase::setEnvVariable(const char* name, const char* value) {
    if (setenv(name, value, 1) == 0) {
        std::cout << "Environment variable set: " << name << std::endl;
    } else {
        std::cerr << "Failed to set environment variable!" << std::endl;
    }
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
		if (stat("/home/root/modules/", &info) == 0 && (info.st_mode & S_IFDIR)){
			home =PWD + "/home/root/modules/";
		}
		else if(stat("/runtime/modules/", &info) == 0 && (info.st_mode & S_IFDIR)){
			home = PWD + "/runtime/modules/";
		}
		else if("/modules/", &info) == 0 && (info.st_mode & S_IFDIR)){
			home = PWD +"/modules/";
		}
        else{
                std::cerr << "Modules Directory not found!" << std::endl;
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
