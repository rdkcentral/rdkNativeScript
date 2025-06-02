#include <cjson/cJSON.h>
#include <NativeJSRenderer.h>
#include <NativeJSLogger.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <set>
#include <unistd.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "rapidjson/document.h"
#include <thread>
#include <memory>
#include <string.h>
#include <vector>

using namespace rapidjson;
using namespace std;
using namespace JsRuntime;

string source="/package/index.js";
std::vector<std::string> flags;

void extractJSFilePaths(const std::string& jsFilePath) {
    std::ifstream file(jsFilePath);

    if (!file.is_open()) {
        std::cerr << "Error: Could not open file '" << jsFilePath << "'\n";
        return;
    }

    std::string line;
    while (std::getline(file, line)) {
        if (!line.empty()) {
            std::cout<<"line:"<<line<<std::endl;
            flags.push_back(line);
        }
    }

    file.close();
}

void Json_parsing(const string &fileName)
{
    ifstream fd(fileName);
    if (!fd.is_open()) {
        cerr << "Can't open the config file: " << fileName << endl;
        return;
    }

    stringstream buffer;
    buffer << fd.rdbuf();
    string configData = buffer.str();
    fd.close();

    Document configDoc;
    if (configDoc.Parse(configData.c_str()).HasParseError()) {
        cerr << "Invalid JSON data in: " << fileName << endl;
        return;
    }

    if (!configDoc.IsObject()) {
        cerr << "JSON root is not an object in: " << fileName << endl;
        return;
    }

    if (configDoc.HasMember("features") && configDoc["features"].IsArray()) 
    {	    
	const Value& flagsarr = configDoc["features"];     
        set<string> options; 
        for (SizeType i = 0; i < flagsarr.Size(); i++) {
            const Value& flagOption = flagsarr[i];
            if (!flagOption.IsObject())
                continue;

            if (flagOption.HasMember("name") && flagOption.HasMember("enable")) {
                const Value& name = flagOption["name"];
                const Value& enable = flagOption["enable"];

                if(name=="player" && enable==true)
                {
                    flags.push_back("--enablePlayer");
                }
                else if(name=="xhr" && enable==true)
                {
                    flags.push_back("--enableXHR");
                }
                else if(name=="websocket" && enable==true)
                {
                    flags.push_back("--enableWebSocket");
                }
                else if(name=="http" && enable==true)
                {
                    flags.push_back("--enableHttp");
                }
                else if(name=="websocketenhanced" && enable==true)
                {
                    flags.push_back("--enableWebSocketEnhanced");
                }
                else if(name=="fetch" && enable==true)
                {
                    flags.push_back("--enableFetch");
                }
                else if(name=="jsdom" && enable==true)
                {
                    flags.push_back("--enableJSDOM");
                }
                else if(name=="window" && enable==true)
                {
                    flags.push_back("--enableWindow");
                }
                else if(name=="console" && enable==true)
                {
                    flags.push_back("--console");
                }
                else if(name=="display" && enable==true)
                {
                    flags.push_back("--display");
                }
                else{
                    continue;
                }
                
            }
        }        
    }
}

int main(int argc, char *argv[]) {
    const char* filename = "/package/rdk.config";
    std::ifstream file(filename);
    if (file.good())
    {
    std::string jsonStr, line;
    while (std::getline(file, line)) {
    	jsonStr += line + "\n";
    }
    file.close();
    cJSON* root = cJSON_Parse(jsonStr.c_str());
    if (root){
    	cJSON* envs = cJSON_GetObjectItem(root, "configuration");
   	cJSON* envArray = cJSON_GetObjectItem(envs, "envs");
    	cJSON* env;
    	cJSON_ArrayForEach(env, envArray) {
        cJSON* key = cJSON_GetObjectItem(env, "key");
        cJSON* value = cJSON_GetObjectItem(env, "value");
        if (key && value && cJSON_IsString(key) && cJSON_IsString(value)) {
            setenv(key->valuestring, value->valuestring, 1);
     	}
    }
    cJSON_Delete(root);
    }       
    }	
    vector<char*> c_flags;
    std::string fileName = "/package/app.config";
    Json_parsing(fileName);
    extractJSFilePaths(source);
    for(auto& flag : flags)
    {
        c_flags.push_back((char*)(flag.c_str()));
    }
    if (c_flags.size() < 2)
    {
        NativeJSLogger::log(WARN, "Pass the URL to run\n");
	return -1;
    }
    bool runServer = false;
    std::string waylanddisplay("");
    bool enableJSDOM = false, enableWindow = false, enablePlayer = false, enableWebSocketEnhanced = false, enableFetch = false;
    int i = 0, appendindex=c_flags.size()-1;
    std::vector<std::string> applications;
    ModuleSettings moduleSettings;
    bool consoleMode = false;
    
    while (i<c_flags.size())
    {	    
        std::cout<<c_flags[i]<<std::endl;
        if (strcmp(c_flags[i], "--display") == 0)
        {
            appendindex = i-1;
	    i++;
            waylanddisplay = c_flags[i];
        }
	else if (strcmp(c_flags[i], "--enableHttp") == 0)
        {
            moduleSettings.enableHttp = true;
        }
	else if (strcmp(c_flags[i], "--enableXHR") == 0)
        {
            moduleSettings.enableXHR = true;
        }
	else if (strcmp(c_flags[i], "--enableWebSocket") == 0)
        {
            moduleSettings.enableWebSocket = true;
        }
	else if (strcmp(c_flags[i], "--enableWebSocketEnhanced") == 0)
        {
            moduleSettings.enableWebSocketEnhanced = true;
        }
	else if (strcmp(c_flags[i], "--enableFetch") == 0)
        {
            moduleSettings.enableFetch = true;
        }
	else if (strcmp(c_flags[i], "--enableJSDOM") == 0)
        {
            moduleSettings.enableJSDOM = true;
        }
	else if (strcmp(c_flags[i], "--enableWindow") == 0)
        {
            moduleSettings.enableWindow = true;
        }
	else if (strcmp(c_flags[i], "--enablePlayer") == 0)
        {
            std::cout<<"player enabled"<<std::endl;        
	    moduleSettings.enablePlayer = true;
        }
    else if (strcmp(c_flags[i], "--console") == 0)
        {
            consoleMode = true;
        }
        else
        {
            applications.push_back(c_flags[i]);
        }
	i++;
    }

    std::shared_ptr<NativeJSRenderer> renderer = std::make_shared<NativeJSRenderer>(waylanddisplay);
    if (consoleMode) {
        renderer->setEnvForConsoleMode(moduleSettings);
    }
    if (!renderer)
    {
        NativeJSLogger::log(ERROR, "Unable to run application\n");
        return -1;
    }

    uint32_t id = renderer->createApplication(moduleSettings);
    for (int j = 0; j < applications.size(); j++) {
        std::string url = applications[j];
        NativeJSLogger::log(INFO, "Application URL is %s\n", (url.size() ? url.c_str() : "empty"));
        renderer->runApplication(id, url);
    }

    renderer->run();
    return 0;    
} 
