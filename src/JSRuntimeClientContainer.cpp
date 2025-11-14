#include "JSRuntimeContainer.h"
#include <iostream>

int main()
{
    std::string containerId = "com.sky.as.apps_com.xumo.ipa";
    
    // Check if container exists
    if (JSRuntimeContainer::isContainer(containerId))
    {
        std::cout << "Container " << containerId << " is running" << std::endl;
        
        // Get container IP address
        std::string ipAddress = JSRuntimeContainer::getContainerIpAddress(containerId);
        if (!ipAddress.empty())
        {
            std::cout << "Container IP address: " << ipAddress << std::endl;
        }
        else
        {
            std::cout << "Failed to get container IP address" << std::endl;
        }
        
        // Get container processes
        auto processes = JSRuntimeContainer::getContainerProcesses(containerId);
        std::cout << "Container has " << processes.size() << " processes:" << std::endl;
        for (const auto& proc : processes)
        {
            std::cout << "  PID: " << proc.first << ", Name: " << proc.second << std::endl;
        }
    }
    else
    {
        std::cout << "Container " << containerId << " not found" << std::endl;
    }
    
    return 0;
}

