#ifndef JSRUNTIMECONTAINER_H
#define JSRUNTIMECONTAINER_H

#include <string>
#include <vector>
#include <functional>
#include <map>

class JSRuntimeContainer
{
public:
    enum Namespace {
        NetworkNamespace = 0x01,
        MountNamespace = 0x02,
        IpcNamespace = 0x04,
        PidNamespace = 0x08,
        UserNamespace = 0x10,
        UtsNamespace = 0x20
    };

    // Get container IP address
    static std::string getContainerIpAddress(const std::string& containerId);
    
    // Check if container exists
    static bool isContainer(const std::string& containerId);
    
    // Get list of processes in container
    static std::vector<std::pair<pid_t, std::string>> getContainerProcesses(const std::string& containerId);
    
    // Execute function in container namespace
    static bool nsEnter(const std::string& containerId, Namespace type, const std::function<void()>& func);

private:
    // Internal implementation functions
    static bool nsEnterImpl(const std::string& containerId, Namespace type, const std::function<void()>& func);
    static pid_t findContainerPid(const std::string& containerId);
    static bool nsEnterWithPid(pid_t pid, int nsType, const std::function<void()>& func);
    static void nsThread(int newNsFd, int nsType, bool* success, const std::function<void()>& func);
};

#endif // JSRUNTIMECONTAINER_H

