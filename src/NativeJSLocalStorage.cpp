#include <NativeJSLocalStorage.h>
#include <NativeJSLogger.h>

#include <atomic>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <string>
#include <system_error>
#include <sys/stat.h>
#include <unistd.h>

namespace
{
std::filesystem::path storageDirectory()
{
    const char *home = std::getenv("HOME");
    return std::filesystem::path(home && *home ? home : "/home/root") /
           ".local/share/data/Comcast/NativeXREReceiver";
}

std::filesystem::path storageFile(const std::string &nameSpace)
{
    std::uint64_t hash = 14695981039346656037ULL;
    for (unsigned char character : nameSpace)
    {
        hash ^= character;
        hash *= 1099511628211ULL;
    }

    char suffix[17];
    std::snprintf(suffix, sizeof(suffix), "%016llx",
                  static_cast<unsigned long long>(hash));
    return storageDirectory() / (std::string("localStorage-") + suffix + ".json");
}

void logFileError(const char *operation, const std::filesystem::path &path,
                  const std::string &error)
{
    NativeJSLogger::log(ERROR, "NativeJSLocalStorage: %s '%s': %s\n",
                        operation, path.c_str(), error.c_str());
}
}

NativeJSLocalStorage::NativeJSLocalStorage()
{
    const char *applicationName = std::getenv("APPLICATION_NAME");
    if (applicationName && *applicationName)
        m_namespace = applicationName;
}
bool NativeJSLocalStorage::load(std::string &snapshot, std::string &error) const
{
    const std::filesystem::path path = storageFile(m_namespace);
    std::ifstream input(path, std::ios::binary);
    if (!input)
    {
        std::error_code filesystemError;
        if (!std::filesystem::exists(path, filesystemError) && !filesystemError)
        {
            snapshot = "{}";
            return true;
        }

        error = filesystemError ? filesystemError.message() : "cannot open snapshot";
        logFileError("cannot read", path, error);
        return false;
    }

    snapshot.assign(std::istreambuf_iterator<char>(input),
                    std::istreambuf_iterator<char>());
    if (input.bad())
    {
        error = "read failed";
        logFileError("cannot read", path, error);
        return false;
    }

    return true;
}

bool NativeJSLocalStorage::save(const std::string &snapshot, std::string &error)
{
    const std::filesystem::path path = storageFile(m_namespace);
    const std::filesystem::path directory = path.parent_path();
    std::error_code filesystemError;
    std::filesystem::create_directories(directory, filesystemError);
    if (filesystemError)
    {
        error = filesystemError.message();
        logFileError("cannot create directory for", path, error);
        return false;
    }
    chmod(directory.c_str(), 0700);

    static std::atomic<unsigned long long> nextTempId{0};
    std::filesystem::path tempPath = path;
    tempPath += "." + std::to_string(getpid()) + "." +
                std::to_string(nextTempId.fetch_add(1)) + ".tmp";

    std::ofstream output(tempPath, std::ios::binary | std::ios::trunc);
    if (!output)
    {
        error = "cannot open temporary snapshot";
        logFileError("cannot write", tempPath, error);
        return false;
    }
    output << snapshot;
    output.close();
    if (!output)
    {
        error = "write failed";
        logFileError("cannot write", tempPath, error);
        std::filesystem::remove(tempPath, filesystemError);
        return false;
    }
    chmod(tempPath.c_str(), 0600);

    std::filesystem::rename(tempPath, path, filesystemError);
    if (filesystemError)
    {
        error = filesystemError.message();
        logFileError("cannot replace snapshot", path, error);
        std::filesystem::remove(tempPath, filesystemError);
        return false;
    }

    return true;
}
