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

#include <NativeJSRenderer.h>
#include <thread>
#include <string>
#include <iostream>
#include <memory>
#include <string.h>
#include <thread>
#include <unistd.h>

using namespace std;
using namespace JsRuntime;

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "pass the url to run" << std::endl;
	return -1;
    }
    std::string waylanddisplay("test");
    int i = 1, appendindex=argc-1;
    std::vector<std::string> applications;
    while (i<argc)
    {	    
        if (strcmp(argv[i], "--display") == 0)
        {
            appendindex = i-1;
	    i++;
            waylanddisplay = argv[i];
        }
        else
	{
            applications.push_back(argv[i]);
        }
	i++;
    }
    std::shared_ptr<NativeJSRenderer> renderer = std::make_shared<NativeJSRenderer>(waylanddisplay);
    if (!renderer)
    {
        std::cout << "unable to run application" << std::endl;
	return -1;
    }
    for (int j=1; j<=appendindex; j++)
    {
        std::string url = argv[j];
        std::cout << "application url is " << url.c_str() << std::endl;
        renderer->launchApplication(url);
    } 
    renderer->run();
    return 0;
}
