#ifndef NATIVEJS_LOCAL_STORAGE_H
#define NATIVEJS_LOCAL_STORAGE_H

#include <string>

class NativeJSLocalStorage
{
public:
    NativeJSLocalStorage();

    NativeJSLocalStorage(const NativeJSLocalStorage &) = delete;
    NativeJSLocalStorage &operator=(const NativeJSLocalStorage &) = delete;

    bool load(std::string &snapshot, std::string &error) const;
    bool save(const std::string &snapshot, std::string &error);

private:
    std::string m_namespace = "default";
};

#endif
