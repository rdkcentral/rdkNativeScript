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
#include <vector>
#include "rapidjson/document.h"
#ifndef USE_WEBSOCKET_MOCK
#include <websocketpp/config/asio_no_tls_client.hpp>
#include <websocketpp/client.hpp>
#endif

using namespace rapidjson;

#ifndef USE_WEBSOCKET_MOCK
typedef websocketpp::client<websocketpp::config::asio_client> SimpleClient;
#endif

void JSRuntimeContainer::nsThread(int newNsFd, int nsType, bool* success, const std::function<void()>& func)
{
    if (setns(newNsFd, nsType) != 0)
    {
        std::cerr << "Failed to switch into new namespace: " << strerror(errno) << std::endl;
        *success = false;
        return;
    }

    func();
    *success = true;
}

// Enter namespace using PID
bool JSRuntimeContainer::nsEnterWithPid(pid_t pid, int nsType, const std::function<void()>& func)
{
    const char* nsName;
    
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

    char nsPath[64];
    snprintf(nsPath, sizeof(nsPath), "/proc/%d/ns/%s", pid, nsName);

    int newNsFd = open(nsPath, O_RDONLY | O_CLOEXEC);
    if (newNsFd < 0)
    {
        std::cerr << "Failed to open container namespace: " << nsPath << " - " << strerror(errno) << std::endl;
        return false;
    }

    bool success = false;
    
    std::thread thread([=, &success, &func]() {
        nsThread(newNsFd, nsType, &success, func);
    });
    thread.join();
    
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
        return -1;
    }
    
    std::string line;
    if (!std::getline(file, line) || line.empty())
    {
        return -1;
    }
    
    long pid = std::strtol(line.c_str(), nullptr, 10);
    if (pid >= INT32_MAX || pid <= 0)
    {
        return -1;
    }
    
    return static_cast<pid_t>(pid);
}

bool JSRuntimeContainer::nsEnterImpl(const std::string& containerId, Namespace type, const std::function<void()>& func)
{
    pid_t containerPid = findContainerPid(containerId);
    if (containerPid <= 0)
    {
        return false;
    }
    
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

bool JSRuntimeContainer::nsEnter(const std::string& containerId, Namespace type, const std::function<void()>& func)
{
    return nsEnterImpl(containerId, type, func);
}

std::string JSRuntimeContainer::getContainerIpAddress(const std::string& containerId)
{
    std::string ipAddress = "";
    
    auto getIpAddress = [&ipAddress]() {
        int sock = socket(AF_INET, SOCK_STREAM | SOCK_CLOEXEC, 0);
        if (sock < 0)
        {
            return;
        }
        
        struct ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strcpy(ifr.ifr_name, "eth0");
        
        if (ioctl(sock, SIOCGIFADDR, &ifr) < 0)
        {
            close(sock);
            return;
        }
        
        close(sock);
        
        struct sockaddr_in* ifaceAddr = reinterpret_cast<struct sockaddr_in*>(&ifr.ifr_addr);
        char* ip = inet_ntoa(ifaceAddr->sin_addr);
        if (ip != nullptr)
        {
            ipAddress = std::string(ip);
        }
    };
    
    if (nsEnterImpl(containerId, NetworkNamespace, getIpAddress) && !ipAddress.empty())
    {
        return ipAddress;
    }
    
    return "";
}

bool JSRuntimeContainer::isContainer(const std::string& containerId)
{
    std::string cgroupPath = "/sys/fs/cgroup/memory/" + containerId + "/cgroup.procs";
    return (access(cgroupPath.c_str(), F_OK) == 0);
}

bool JSRuntimeContainer::connectAndSend(const std::string& ip, const std::string& message)
{
#ifdef USE_WEBSOCKET_MOCK
    // In mock builds, just return true
    return true;
#else
    websocketpp::lib::error_code ec;
    SimpleClient c;
    c.clear_access_channels(websocketpp::log::alevel::all);
    c.clear_error_channels(websocketpp::log::elevel::all);
    c.init_asio();

    std::string uri = std::string("ws://") + ip + std::string(":") + std::to_string(WS_SERVER_PORT);
    SimpleClient::connection_ptr con = c.get_connection(uri, ec);
    if (ec) {
        return false;
    }

    websocketpp::connection_hdl hdl = con->get_handle();
    c.connect(con);

    std::thread t(&SimpleClient::run, &c);

    // Wait briefly for open
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    try {
        c.send(hdl, message, websocketpp::frame::opcode::text);
    } catch (...) {}

    c.close(hdl, websocketpp::close::status::going_away, "bye", ec);
    t.join();
    return true;
#endif
}

// Build launchApplication JSON payload for a given URL and options
std::string JSRuntimeContainer::buildLaunchMessage(const std::string &url, const std::string& options)
{
    std::ostringstream oss;
    oss << "{\"method\": \"launchApplication\", \"params\":{\"url\":\"" << url << "\",\"moduleSettings\":\"" << options << "\"}}";   
    std::string message = oss.str();
    return message;
}

std::string JSRuntimeContainer::parseAppConfig(const std::string& configPath)
{

    std::string optionsStr;  
    std::ifstream fd(configPath);
    if (!fd.is_open()) {
        return optionsStr; 
    }

    std::stringstream buffer;
    buffer << fd.rdbuf();
    std::string configData = buffer.str();
    fd.close();
    
    Document configDoc;
    if (configDoc.Parse(configData.c_str()).HasParseError()) {
        return optionsStr;
    }

    if (!configDoc.IsObject()) {
        return optionsStr;
    }

    if (configDoc.HasMember("features") && configDoc["features"].IsArray())
    {
        const Value& flagsarr = configDoc["features"];        
        for (SizeType i = 0; i < flagsarr.Size(); i++) {
            const Value& flagOption = flagsarr[i];
            if (!flagOption.IsObject())
                continue;

            if (flagOption.HasMember("name") && flagOption.HasMember("enable")) {
                const Value& name = flagOption["name"];
                const Value& enable = flagOption["enable"];

                if (enable == true) {
                    std::string featureName = name.GetString();
                    std::cout << "[DEBUG] parseAppConfig - processing enabled feature: " << featureName << std::endl;
                    
                    if (name == "player")
                        optionsStr += "player,";
                    else if (name == "xhr")
                        optionsStr += "xhr,";
                    else if (name == "websocket")
                        optionsStr += "ws,";
                    else if (name == "http")
                        optionsStr += "http,";
                    else if (name == "websocketenhanced")
                        optionsStr += "wsenhanced,";
                    else if (name == "fetch")
                        optionsStr += "fetch,";
                    else if (name == "jsdom")
                        optionsStr += "jsdom,";
                    else if (name == "window")
                        optionsStr += "window,";
                    
                }
            }
        }
    }
    
    if (!optionsStr.empty() && optionsStr.back() == ',')
        optionsStr.pop_back();
        
    return optionsStr;
}


