#include "JSRuntimeContainer.h"
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>
#include "NativeJSLogger.h"
int main()
{
    std::string containerId = "com.sky.as.apps_TestApp";
    const std::string basePath = "/opt/twocontext"; // constant base path
    const std::vector<std::string> apps = {"app1", "app2"};
    
    std::string ipAddress = JSRuntimeContainer::getContainerIpAddress(containerId);
    if (ipAddress.empty()) {
	NativeJSLogger::log(ERROR, "Failed to retrieve IP address for container");
	return 1; 
    }

    for (const auto &app : apps) {
        std::string url = basePath + std::string("/") + app + std::string("/index.html");
        if (access(url.c_str(), F_OK) == 0) {
            std::string pathAppConfig = basePath + std::string("/") + app + std::string("/app.config");
            std::string options = JSRuntimeContainer::parseAppConfig(pathAppConfig);
            std::string message = JSRuntimeContainer::buildLaunchMessage(url, options);
            JSRuntimeContainer::connectAndSend(ipAddress, message);
        }
    }

    return 0;
}

