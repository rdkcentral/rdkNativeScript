// JSRuntimeContainer.cpp
#include "JSRuntimeContainer.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sched.h>
#include <errno.h>

// Thread helper function for namespace switching
void JSRuntimeContainer::nsThread(int newNsFd, int nsType, bool* success, const std::function<void()>& func)
{
    // Switch into the new namespace
    if (setns(newNsFd, nsType) != 0)
    {
        std::cerr << "Failed to switch into new namespace: " << strerror(errno) << std::endl;
        *success = false;
        return;
    }

    // Execute the caller's function
    func();
    *success = true;
}

// Enter namespace using PID
bool JSRuntimeContainer::nsEnterWithPid(pid_t pid, int nsType, const std::function<void()>& func)
{
    const char* nsName;
    
    // Determine namespace type
    switch (nsType)
    {
        case CLONE_NEWIPC:
            nsName = "ipc";
            break;
        case CLONE_NEWNET:
            nsName = "net";
            break;
        case CLONE_NEWNS:
            nsName = "mnt";
            break;
        case CLONE_NEWPID:
            nsName = "pid";
            break;
        case CLONE_NEWUSER:
        case CLONE_NEWUTS:
            std::cerr << "Unsupported namespace type: " << nsType << std::endl;
            return false;
        default:
            std::cerr << "Invalid namespace type: " << nsType << std::endl;
            return false;
    }

    // Construct namespace path
    char nsPath[64];
    snprintf(nsPath, sizeof(nsPath), "/proc/%d/ns/%s", pid, nsName);

    // Open namespace file
    int newNsFd = open(nsPath, O_RDONLY | O_CLOEXEC);
    if (newNsFd < 0)
    {
        std::cerr << "Failed to open container namespace: " << nsPath << " - " << strerror(errno) << std::endl;
        return false;
    }

    bool success = false;
    
    // Create thread to run in the namespace
    std::thread thread([=, &success, &func]() {
        nsThread(newNsFd, nsType, &success, func);
    });
    
    // Wait for thread completion
    thread.join();
    
    // Close namespace fd
    close(newNsFd);
    
    return success;
}

// Find a PID inside the container
pid_t JSRuntimeContainer::findContainerPid(const std::string& containerId)
{
    std::string cgroupPath = "/sys/fs/cgroup/memory/" + containerId + "/cgroup.procs";
    
    std::ifstream file(cgroupPath);
    if (!file.is_open())
    {
        if (errno == ENOENT)
        {
            std::cout << "No cgroup file at: " << cgroupPath << std::endl;
        }
        else
        {
            std::cerr << "Failed to open cgroup file: " << cgroupPath << " - " << strerror(errno) << std::endl;
        }
        return -1;
    }
    
    std::string line;
    if (!std::getline(file, line) || line.empty())
    {
        std::cerr << "Cgroup procs file is empty - no pids in container?" << std::endl;
        return -1;
    }
    
    long pid = std::strtol(line.c_str(), nullptr, 10);
    if (pid >= INT32_MAX || pid <= 0)
    {
        return -1;
    }
    
    return static_cast<pid_t>(pid);
}

// Main namespace entry implementation
bool JSRuntimeContainer::nsEnterImpl(const std::string& containerId, Namespace type, const std::function<void()>& func)
{
    // Find a PID in the container
    pid_t containerPid = findContainerPid(containerId);
    if (containerPid <= 0)
    {
        std::cerr << "No container found with id: " << containerId << std::endl;
        return false;
    }
    
    // Enter the appropriate namespace
    switch (type)
    {
        case NetworkNamespace:
            return nsEnterWithPid(containerPid, CLONE_NEWNET, func);
        case MountNamespace:
            return nsEnterWithPid(containerPid, CLONE_NEWNS, func);
        case IpcNamespace:
            return nsEnterWithPid(containerPid, CLONE_NEWIPC, func);
        default:
            std::cerr << "Unknown namespace type" << std::endl;
            return false;
    }
}

// Public namespace entry function
bool JSRuntimeContainer::nsEnter(const std::string& containerId, Namespace type, const std::function<void()>& func)
{
    return nsEnterImpl(containerId, type, func);
}

// Get container IP address - main functionality you wanted
std::string JSRuntimeContainer::getContainerIpAddress(const std::string& containerId)
{
    std::string ipAddress = "";
    
    // Lambda to run in container network namespace
    auto getIpAddress = [&ipAddress]() {
        // Create socket for ioctl
        int sock = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (sock < 0)
        {
            std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
            return;
        }
        
        // Setup interface request for eth0
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, "eth0");
        
        // Get interface IP address
        if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
        {
            std::cerr << "Failed to get interface IP address: " << strerror(errno) << std::endl;
            close(sock);
            return;
        }
        
        close(sock);
        
        // Extract IP address
        struct sockaddr_in* ifaceAddr = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr);
        char* ip = inet_ntoa(ifaceAddr->sin_addr);
        if (ip != nullptr)
        {
            ipAddress = std::string(ip);
        }
    };
    
    // Run the lambda in container network namespace
    if (nsEnterImpl(containerId, NetworkNamespace, getIpAddress) && !ipAddress.empty())
    {
        return ipAddress;
    }
    
    return "";
}

// Check if container exists
bool JSRuntimeContainer::isContainer(const std::string& containerId)
{
    std::string cgroupPath = "/sys/fs/cgroup/memory/" + containerId + "/cgroup.procs";
    return (access(cgroupPath.c_str(), F_OK) == 0);
}

// Get list of processes in container
std::vector<std::pair<pid_t, std::string>> JSRuntimeContainer::getContainerProcesses(const std::string& containerId)
{
    std::vector<std::pair<pid_t, std::string>> processes;
    
    std::string cgroupPath = "/sys/fs/cgroup/memory/" + containerId + "/cgroup.procs";
    std::ifstream file(cgroupPath);
    
    if (!file.is_open())
    {
        std::cerr << "Failed to open cgroup file: " << cgroupPath << std::endl;
        return processes;
    }
    
    std::string line;
    while (std::getline(file, line))
    {
        int pid = std::stoi(line);
        if (pid < 2) continue;
        
        // Read process name from /proc/<pid>/comm
        std::string procPath = "/proc/" + std::to_string(pid) + "/comm";
        std::ifstream commFile(procPath);
        
        if (commFile.is_open())
        {
            std::string processName;
            if (std::getline(commFile, processName) && !processName.empty())
            {
                processes.emplace_back(pid, processName);
                std::cout << "Found process: " << processName << ":" << pid << " in container" << std::endl;
            }
        }
    }
    
    return processes;
}

