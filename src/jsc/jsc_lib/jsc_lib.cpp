/*
 * Copyright (c) 2024 RDK Management.
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation;
 * version 2.0 of the License.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; See the file COPYING.LGPL. if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */
 
#ifdef USE_JSCLIB_MOCK
#include "jsc_lib_mock.h"
#else
#include "jsc_lib.h"
#endif

#include "NativeJSLogger.h"

using namespace JSC;

static bool supportsRichSourceInfo(const JSGlobalObject*) { return true; }
static bool shouldInterruptScript(const JSGlobalObject*) { return true; }
static bool shouldInterruptScriptBeforeTimeout(const JSGlobalObject*) { return false; }
static RuntimeFlags javaScriptRuntimeFlags(const JSGlobalObject*) { return RuntimeFlags(); }
static void reportUncaughtExceptionAtEventLoop(JSGlobalObject*, Exception* exception)
{
    NativeJSLogger::log(ERROR, "Uncaught Exception at run loop: %s\n", exception->value());
}
static JSObject* currentScriptExecutionOwner(JSGlobalObject* global) { return global; }
static ScriptExecutionStatus scriptExecutionStatus(JSGlobalObject*, JSObject*) { return ScriptExecutionStatus::Running; }
static void reportViolationForUnsafeEval(JSGlobalObject*, JSString*) { }


JSInternalPromise* moduleLoaderImportModule(JSGlobalObject*, JSModuleLoader*, JSString*, JSValue, const SourceOrigin&);
Identifier moduleLoaderResolve(JSGlobalObject*, JSModuleLoader*, JSValue, JSValue, JSValue);
JSInternalPromise* moduleLoaderFetch(JSGlobalObject*, JSModuleLoader*, JSValue, JSValue, JSValue);
JSObject* moduleLoaderCreateImportMetaProperties(JSGlobalObject*, JSModuleLoader*, JSValue, JSModuleRecord*, JSValue);

const GlobalObjectMethodTable s_globalObjectMethodTableMadan = {
    &supportsRichSourceInfo,
    &shouldInterruptScript,
    &javaScriptRuntimeFlags,
    nullptr, // queueMicrotaskToEventLoop
    &shouldInterruptScriptBeforeTimeout,
    &moduleLoaderImportModule,
    &moduleLoaderResolve,
    &moduleLoaderFetch,
    &moduleLoaderCreateImportMetaProperties,
    nullptr, // moduleLoaderEvaluate
    nullptr, // promiseRejectionTracker
    &reportUncaughtExceptionAtEventLoop,
    &currentScriptExecutionOwner,
    &scriptExecutionStatus,
    &reportViolationForUnsafeEval,
    nullptr, // defaultLanguage
    nullptr, // compileStreaming
    nullptr, // instantiateStreaming
    nullptr, // deriveShadowRealmGlobalObject
};

#ifndef USE_JSCLIB_MOCK
struct MemoryStruct
{
    MemoryStruct()
        : headerSize(0)
        , headerBuffer(NULL)
        , contentsSize(0)
        , contentsBuffer(NULL)
        , readSize(0)
    {
        headerBuffer = (char*)malloc(1);
        contentsBuffer = (char*)malloc(1);
    }

    ~MemoryStruct()
    {
      if (headerBuffer != NULL)
      {
        free(headerBuffer);
        headerBuffer = NULL;
      }
      if (contentsBuffer != NULL)
      {
        free(contentsBuffer);
        contentsBuffer = NULL;
      }
    }

    size_t headerSize;
    char* headerBuffer;
    size_t contentsSize;
    char* contentsBuffer;
    size_t readSize;
};
#endif // !USE_JSCLIB_MOCK

static size_t CallbackHeader(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t downloadSize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->headerBuffer = (char*)realloc(mem->headerBuffer, mem->headerSize + downloadSize + 1);
  if(mem->headerBuffer == NULL) {
    NativeJSLogger::log(ERROR, "out of memory when downloading image\n");
    return 0;
  }

  memcpy(&(mem->headerBuffer[mem->headerSize]), contents, downloadSize);
  mem->headerSize += downloadSize;
  mem->headerBuffer[mem->headerSize] = 0;

  return downloadSize;
}

static size_t CallbackOnMemoryWrite(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t downloadSize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;

  mem->contentsBuffer = (char*)realloc(mem->contentsBuffer, mem->contentsSize + downloadSize + 1);
  if(mem->contentsBuffer == NULL) {
    NativeJSLogger::log(ERROR, "out of memory when downloading image\n");
    return 0;
  }

  memcpy(&(mem->contentsBuffer[mem->contentsSize]), contents, downloadSize);
  mem->contentsSize += downloadSize;
  mem->contentsBuffer[mem->contentsSize] = 0;

  return downloadSize;
}

bool downloadFile(std::string& url, MemoryStruct& chunk)
{
    bool ret = false;
    CURL *curl;
    CURLcode res;
#ifdef USE_JSCLIB_MOCK
    if (MockControl::simulate_curl_init_failure)
        curl = nullptr;
    else
#endif
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CallbackHeader);
        curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CallbackOnMemoryWrite);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30);
        curl_easy_setopt(curl, CURLOPT_NOSIGNAL, 1L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, true);
        curl_easy_setopt(curl, CURLOPT_USERAGENT, "libcurl-agent/1.0");
        curl_easy_setopt(curl, CURLOPT_PROXY, "");


        //curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo (curl, CURLINFO_RESPONSE_CODE, &httpCode);
        curl_easy_cleanup(curl);
        if ((res == 0) && (httpCode == 200))
        {
            NativeJSLogger::log(INFO, "download operation success\n");
	    ret = true;
        }
        else
        {
	    NativeJSLogger::log(ERROR, "download operation failed\n");
        }
    }
    else
    {
	NativeJSLogger::log(ERROR, "unable to perform download\n");
    }
    return ret;
}

#ifdef USE_JSCLIB_MOCK
// Non-static versions for testing
UChar pathSeparator()
{
#if OS(WINDOWS)
    return '\\';
#else
    return '/';
#endif
}

bool isAbsolutePath(StringView path)
{
#if OS(WINDOWS)
    // Just look for local drives like C:\.
    return path.length() > 2 && isASCIIAlpha(path[0]) && path[1] == ':' && (path[2] == '\\' || path[2] == '/');
#else
    return path.startsWith('/');
#endif
}

bool isDottedRelativePath(StringView path)
{
#if OS(WINDOWS)
    auto length = path.length();
    if (length < 2 || path[0] != '.')
        return false;

    if (path[1] == '/' || path[1] == '\\')
        return true;

    return length > 2 && path[1] == '.' && (path[2] == '/' || path[2] == '\\');
#else
    return path.startsWith("./"_s) || path.startsWith("../"_s);
#endif
}

void convertShebangToJSCommentWrapper(char* buffer, size_t size)
{
    if (size >= 2) {
        if (buffer[0] == '#' && buffer[1] == '!')
            buffer[0] = buffer[1] = '/';
    }
}

bool fetchScriptFromLocalFileSystemWrapper(const char* fileName, char** outBuffer, size_t* outSize)
{
    String fileNameStr = String::fromUTF8(fileName);
    Vector<char> buffer;
    FILE* f = fopen(fileName, "rb");
    if (!f)
        return false;
    
#ifdef USE_JSCLIB_MOCK
    if (MockControl::simulate_fseek_failure) {
        fclose(f);
        return false;
    }
#endif
    if (fseek(f, 0, SEEK_END) == -1) {
        fclose(f);
        return false;
    }
#ifdef USE_JSCLIB_MOCK
    if (MockControl::simulate_ftell_failure) {
        fclose(f);
        return false;
    }
#endif
    long bufferCapacity = ftell(f);
    if (bufferCapacity == -1) {
        fclose(f);
        return false;
    }
    if (fseek(f, 0, SEEK_SET) == -1) {
        fclose(f);
        return false;
    }
    buffer.resize(bufferCapacity);
#ifdef USE_JSCLIB_MOCK
    size_t readSize;
    if (MockControl::simulate_fread_short) {
        readSize = 0;  // Simulate partial/failed read
    } else {
        readSize = fread(buffer.data(), 1, bufferCapacity, f);
    }
#else
    size_t readSize = fread(buffer.data(), 1, bufferCapacity, f);
#endif
    fclose(f);
    
    if (readSize != static_cast<size_t>(bufferCapacity))
        return false;
    
    // Convert shebang
    if (buffer.size() >= 2) {
        if (buffer[0] == '#' && buffer[1] == '!')
            buffer[0] = buffer[1] = '/';
    }
    
    *outSize = buffer.size();
    *outBuffer = (char*)malloc(*outSize + 1);
    memcpy(*outBuffer, buffer.data(), *outSize);
    (*outBuffer)[*outSize] = 0;
    return true;
}

#endif // USE_JSCLIB_MOCK

template<typename Vector>

class GlobalObject;
class Workers;

static void checkException1(JSGlobalObject* globalObject, bool hasException, JSValue value, bool& success);

struct Script {
    enum class StrictMode {
        Strict,
        Sloppy
    };

    enum class ScriptType {
        Script,
        Module
    };

    enum class CodeSource {
        File,
        CommandLine
    };

    StrictMode strictMode;
    CodeSource codeSource;
    ScriptType scriptType;
    char* argument;

    Script(StrictMode strictMode, CodeSource codeSource, ScriptType scriptType, char *argument)
        : strictMode(strictMode)
        , codeSource(codeSource)
        , scriptType(scriptType)
        , argument(argument)
    {
        if (strictMode == StrictMode::Strict)
            ASSERT(codeSource == CodeSource::File);
    }
};

static const char interactivePrompt[] = ">>> ";

template<typename Vector>
static inline String stringFromUTF(const Vector& utf8)
{
    return String::fromUTF8WithLatin1Fallback(utf8.data(), utf8.size());
}

/*
Copyright (C) 1999-2000 Harri Porten ([porten@kde.org](mailto:porten@kde.org))
Copyright (C) 2004-2020 Apple Inc. All rights reserved.
Copyright (C) 2006 Bjoern Graf ([bjoern.graf@gmail.com](mailto:bjoern.graf@gmail.com))
Licensed under the LGPL License, Version 2.0
*/

#ifndef USE_JSCLIB_MOCK
static UChar pathSeparator()
{
#if OS(WINDOWS)
    return '\\';
#else
    return '/';
#endif
}
#endif // !USE_JSCLIB_MOCK

static URL currentWorkingDirectory()
{
#if OS(WINDOWS)
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa364934.aspx
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247.aspx#maxpath
    // The _MAX_PATH in Windows is 260. If the path of the current working directory is longer than that, _getcwd truncates the result.
    // And other I/O functions taking a path name also truncate it. To avoid this situation,
    //
    // (1). When opening the file in Windows for modules, we always use the abosolute path and add "\\?\" prefix to the path name.
    // (2). When retrieving the current working directory, use GetCurrentDirectory instead of _getcwd.
    //
    // In the path utility functions inside the JSC shell, we does not handle the UNC and UNCW including the network host name.
    DWORD bufferLength = ::GetCurrentDirectoryW(0, nullptr);
    if (!bufferLength)
        return { };
    // In Windows, wchar_t is the UTF-16LE.
    // https://msdn.microsoft.com/en-us/library/dd374081.aspx
    // https://msdn.microsoft.com/en-us/library/windows/desktop/ff381407.aspx
    Vector<wchar_t> buffer(bufferLength);
    DWORD lengthNotIncludingNull = ::GetCurrentDirectoryW(bufferLength, buffer.data());
    String directoryString(buffer.data(), lengthNotIncludingNull);
    // We don't support network path like \\host\share\<path name>.
    if (directoryString.startsWith("\\\\"_s))
        return { };

#else
    Vector<char> buffer(PATH_MAX);
    if (!getcwd(buffer.data(), PATH_MAX))
        return { };
    String directoryString = String::fromUTF8(buffer.data());
#endif
    if (directoryString.isEmpty())
        return { };

    // Add a trailing slash if needed so the URL resolves to a directory and not a file.
    if (directoryString[directoryString.length() - 1] != pathSeparator())
        directoryString = makeString(directoryString, pathSeparator());

    return URL::fileURLWithFileSystemPath(directoryString);
}

// FIXME: We may wish to support module specifiers beginning with a (back)slash on Windows. We could either:
// - align with V8 and SM:  treat '/foo' as './foo'
// - align with PowerShell: treat '/foo' as 'C:/foo'
#ifndef USE_JSCLIB_MOCK
static bool isAbsolutePath(StringView path)
{
#if OS(WINDOWS)
    // Just look for local drives like C:\.
    return path.length() > 2 && isASCIIAlpha(path[0]) && path[1] == ':' && (path[2] == '\\' || path[2] == '/');
#else
    return path.startsWith('/');
#endif
}

static bool isDottedRelativePath(StringView path)
{
#if OS(WINDOWS)
    auto length = path.length();
    if (length < 2 || path[0] != '.')
        return false;

    if (path[1] == '/' || path[1] == '\\')
        return true;

    return length > 2 && path[1] == '.' && (path[2] == '/' || path[2] == '\\');
#else
    return path.startsWith("./"_s) || path.startsWith("../"_s);
#endif
}
#endif // !USE_JSCLIB_MOCK

static URL absoluteFileURL(const String& fileName)
{
    if (isAbsolutePath(fileName))
        return URL::fileURLWithFileSystemPath(fileName);

    auto directoryName = currentWorkingDirectory();
    if (!directoryName.isValid())
        return URL::fileURLWithFileSystemPath(fileName);

    return URL(directoryName, fileName);
}

template<typename Vector>
static void convertShebangToJSComment(Vector& buffer)
{
    if (buffer.size() >= 2) {
        if (buffer[0] == '#' && buffer[1] == '!')
            buffer[0] = buffer[1] = '/';
    }
}

static RefPtr<Uint8Array> fillBufferWithContentsOfFile(FILE* file)
{
    if (fseek(file, 0, SEEK_END) == -1)
        return nullptr;
    long bufferCapacity = ftell(file);
    if (bufferCapacity == -1)
        return nullptr;
    if (fseek(file, 0, SEEK_SET) == -1)
        return nullptr;
    auto result = Uint8Array::tryCreate(bufferCapacity);
    if (!result)
        return nullptr;
    size_t readSize = fread(result->data(), 1, bufferCapacity, file);
    if (readSize != static_cast<size_t>(bufferCapacity))
        return nullptr;
    return result;
}

static RefPtr<Uint8Array> fillBufferWithContentsOfFile(const String& fileName)
{
    FILE* f = fopen(fileName.utf8().data(), "rb");
    if (!f) {
        fprintf(stderr, "1 Could not open file: %s\n", fileName.utf8().data());
        return nullptr;
    }

    RefPtr<Uint8Array> result = fillBufferWithContentsOfFile(f);
    fclose(f);

    return result;
}

template<typename Vector>
static bool fillBufferWithContentsOfFile(FILE* file, Vector& buffer)
{
    // We might have injected "use strict"; at the top.
    size_t initialSize = buffer.size();
    if (fseek(file, 0, SEEK_END) == -1)
        return false;
    long bufferCapacity = ftell(file);
    if (bufferCapacity == -1)
        return false;
    if (fseek(file, 0, SEEK_SET) == -1)
        return false;
    buffer.resize(bufferCapacity + initialSize);
    size_t readSize = fread(buffer.data() + initialSize, 1, buffer.size(), file);
    return readSize == buffer.size() - initialSize;
}

static bool fillBufferWithContentsOfFile(const String& fileName, Vector<char>& buffer)
{
    FILE* f = fopen(fileName.utf8().data(), "rb");
    if (!f) {
        fprintf(stderr, "2 Could not open file: %s\n", fileName.utf8().data());
        return false;
    }

    bool result = fillBufferWithContentsOfFile(f, buffer);
    fclose(f);

    return result;
}

static bool fetchScriptFromLocalFileSystem(const String& fileName, Vector<char>& buffer)
{
    if (!fillBufferWithContentsOfFile(fileName, buffer))
        return false;
    convertShebangToJSComment(buffer);
    return true;
}

#ifdef USE_JSCLIB_MOCK
// Wrapper functions for testing - must be after static function definitions
String stringFromUTFWrapper(const char* data, size_t size)
{
    Vector<char> vec;
    vec.resize(size);
    memcpy(vec.data(), data, size);
    return stringFromUTF(vec);
}

bool fillBufferWithContentsOfFileWrapper(const char* fileName, char** outBuffer, size_t* outSize)
{
    String fileNameStr = String::fromUTF8(fileName);
    Vector<char> buffer;
    
    if (!fillBufferWithContentsOfFile(fileNameStr, buffer))
        return false;
    
    *outSize = buffer.size();
    *outBuffer = (char*)malloc(*outSize + 1);
    memcpy(*outBuffer, buffer.data(), *outSize);
    (*outBuffer)[*outSize] = 0;
    return true;
}

bool absoluteFileURLWrapper(const char* fileName, char** outURL, size_t* outSize)
{
    String fileNameStr = String::fromUTF8(fileName);
    URL url = absoluteFileURL(fileNameStr);
    
    if (!url.isValid())
        return false;
    
    String urlString = url.string();
    CString utf8 = urlString.utf8();
    *outSize = utf8.length();
    *outURL = (char*)malloc(*outSize + 1);
    memcpy(*outURL, utf8.data(), *outSize);
    (*outURL)[*outSize] = 0;
    return true;
}

bool fetchScriptFromLocalFileSystemVectorWrapper(const char* fileName, char** outBuffer, size_t* outSize)
{
    String fileNameStr = String::fromUTF8(fileName);
    Vector<char> buffer;
    
    if (!fetchScriptFromLocalFileSystem(fileNameStr, buffer))
        return false;
    
    *outSize = buffer.size();
    *outBuffer = (char*)malloc(*outSize + 1);
    memcpy(*outBuffer, buffer.data(), *outSize);
    (*outBuffer)[*outSize] = 0;
    return true;
}

bool fillBufferWithContentsOfFileUint8Wrapper(const char* fileName, unsigned char** outBuffer, size_t* outSize)
{
    String fileNameStr = String::fromUTF8(fileName);
    RefPtr<Uint8Array> result = fillBufferWithContentsOfFile(fileNameStr);
    
    if (!result) {
        *outBuffer = nullptr;
        *outSize = 0;
        return false;
    }
    
    *outSize = result->byteLength();
    *outBuffer = (unsigned char*)malloc(*outSize);
    memcpy(*outBuffer, result->data(), *outSize);
    return true;
}

bool currentWorkingDirectoryWrapper(char** outPath, size_t* outSize)
{
    URL cwd = currentWorkingDirectory();
    
    if (!cwd.isValid())
        return false;
    
    String cwdString = cwd.string();
    CString utf8 = cwdString.utf8();
    *outSize = utf8.length();
    *outPath = (char*)malloc(*outSize + 1);
    memcpy(*outPath, utf8.data(), *outSize);
    (*outPath)[*outSize] = 0;
    return true;
}
#endif // USE_JSCLIB_MOCK

class ShellSourceProvider final : public StringSourceProvider {
public:
    static Ref<ShellSourceProvider> create(const String& source, const SourceOrigin& sourceOrigin, String&& sourceURL, const TextPosition& startPosition, SourceProviderSourceType sourceType)
    {
        return adoptRef(*new ShellSourceProvider(source, sourceOrigin, WTFMove(sourceURL), startPosition, sourceType));
    }

    ~ShellSourceProvider() final
    {
        commitCachedBytecode();
    }

    RefPtr<CachedBytecode> cachedBytecode() const final
    {
        if (!m_cachedBytecode)
            loadBytecode();
        return m_cachedBytecode.copyRef();
    }

    void updateCache(const UnlinkedFunctionExecutable* executable, const SourceCode&, CodeSpecializationKind kind, const UnlinkedFunctionCodeBlock* codeBlock) const final
    {
        if (!cacheEnabled() || !m_cachedBytecode)
            return;
        BytecodeCacheError error;
        RefPtr<CachedBytecode> cachedBytecode = encodeFunctionCodeBlock(executable->vm(), codeBlock, error);
        if (cachedBytecode && !error.isValid())
            m_cachedBytecode->addFunctionUpdate(executable, kind, *cachedBytecode);
    }

    void cacheBytecode(const BytecodeCacheGenerator& generator) const final
    {
        if (!cacheEnabled())
            return;
        if (!m_cachedBytecode)
            m_cachedBytecode = CachedBytecode::create();
        auto update = generator();
        if (update)
            m_cachedBytecode->addGlobalUpdate(*update);
    }

    void commitCachedBytecode() const final
    {
        if (!cacheEnabled() || !m_cachedBytecode || !m_cachedBytecode->hasUpdates())
            return;

        auto clearBytecode = makeScopeExit([&] {
            m_cachedBytecode = nullptr;
        });

        String filename = cachePath();
        auto fd = FileSystem::openAndLockFile(filename, FileSystem::FileOpenMode::Write, {FileSystem::FileLockMode::Exclusive, FileSystem::FileLockMode::Nonblocking});
        if (!FileSystem::isHandleValid(fd))
            return;

        auto closeFD = makeScopeExit([&] {
		FileSystem::unlockAndCloseFile(fd);
        });

        auto fileSize = FileSystem::fileSize(fd);
        if (!fileSize)
            return;

        size_t cacheFileSize;
        if (!WTF::convertSafely(*fileSize, cacheFileSize) || cacheFileSize != m_cachedBytecode->size()) {
            // The bytecode cache has already been updated
            return;
        }

        if (!FileSystem::truncateFile(fd, m_cachedBytecode->sizeForUpdate()))
            return;

        m_cachedBytecode->commitUpdates([&] (off_t offset, const void* data, size_t size) {
            long long result = FileSystem::seekFile(fd, offset, FileSystem::FileSeekOrigin::Beginning);
            ASSERT_UNUSED(result, result != -1);
            size_t bytesWritten = static_cast<size_t>(FileSystem::writeToFile(fd, data, size));
            ASSERT_UNUSED(bytesWritten, bytesWritten == size);
        });
    }

private:
    String cachePath() const
    {
        if (!cacheEnabled())
            return { };
        const char* cachePath = Options::diskCachePath();
        String filename = FileSystem::encodeForFileName(FileSystem::lastComponentOfPathIgnoringTrailingSlash(sourceOrigin().url().fileSystemPath()));
        return FileSystem::pathByAppendingComponent(StringView::fromLatin1(cachePath), makeString(source().hash(), '-', filename, ".bytecode-cache"_s));
    }

    void loadBytecode() const
    {
        if (!cacheEnabled())
            return;

        String filename = cachePath();
        if (filename.isNull())
            return;

        auto fd = FileSystem::openAndLockFile(filename, FileSystem::FileOpenMode::Read, {FileSystem::FileLockMode::Shared, FileSystem::FileLockMode::Nonblocking});
        if (!FileSystem::isHandleValid(fd))
            return;

        auto closeFD = makeScopeExit([&] {
            FileSystem::unlockAndCloseFile(fd);
        });

        bool success;
        FileSystem::MappedFileData mappedFileData(fd, FileSystem::MappedFileMode::Private, success);

        if (!success)
            return;

        m_cachedBytecode = CachedBytecode::create(WTFMove(mappedFileData));
    }

    ShellSourceProvider(const String& source, const SourceOrigin& sourceOrigin, String&& sourceURL, const TextPosition& startPosition, SourceProviderSourceType sourceType)
        : StringSourceProvider(source, sourceOrigin, WTFMove(sourceURL), startPosition, sourceType)
        // Workers started via $.agent.start are not shut down in a synchronous manner, and it
        // is possible the main thread terminates the process while a worker is writing its
        // bytecode cache, which results in intermittent test failures. As $.agent.start is only
        // a rarely used testing facility, we simply do not cache bytecode on these threads.
        //, m_cacheEnabled(Worker::current().isMain() && !!Options::diskCachePath())
        , m_cacheEnabled(false)

    {
    }

    bool cacheEnabled() const { return m_cacheEnabled; }

    mutable RefPtr<CachedBytecode> m_cachedBytecode;
    const bool m_cacheEnabled;
};

static inline SourceCode jscSource(const String& source, const SourceOrigin& sourceOrigin, String sourceURL = String(), const TextPosition& startPosition = TextPosition(), SourceProviderSourceType sourceType = SourceProviderSourceType::Program)
{
    return SourceCode(ShellSourceProvider::create(source, sourceOrigin, WTFMove(sourceURL), startPosition, sourceType), startPosition.m_line.oneBasedInt(), startPosition.m_column.oneBasedInt());
}

template<typename Vector>
static inline SourceCode jscSource(const Vector& utf8, const SourceOrigin& sourceOrigin, const String& filename)
{
    // FIXME: This should use an absolute file URL https://bugs.webkit.org/show_bug.cgi?id=193077
    String str = stringFromUTF(utf8);
    return jscSource(str, sourceOrigin, filename);
}

template<typename Vector>
static bool fetchModuleFromLocalFileSystem(const URL& fileURL, Vector& buffer)
{
    String fileName = fileURL.fileSystemPath();
#if OS(WINDOWS)
    // https://msdn.microsoft.com/en-us/library/windows/desktop/aa365247.aspx#maxpath
    // Use long UNC to pass the long path name to the Windows APIs.
    auto pathName = makeString("\\\\?\\", fileName).wideCharacters();
    struct _stat status { };
    if (_wstat(pathName.data(), &status))
        return false;
    if ((status.st_mode & S_IFMT) != S_IFREG)
        return false;

    FILE* f = _wfopen(pathName.data(), L"rb");
#else
    auto pathName = fileName.utf8();
    struct stat status { };
    if (stat(pathName.data(), &status))
        return false;
    if ((status.st_mode & S_IFMT) != S_IFREG)
        return false;

    FILE* f = fopen(pathName.data(), "r");
#endif
    if (!f) {
        fprintf(stderr, "Could not open file: %s\n", fileName.utf8().data());
        return false;
    }

    bool result = fillBufferWithContentsOfFile(f, buffer);
    if (result)
        convertShebangToJSComment(buffer);
    fclose(f);

    return result;
}

template <typename T>
static CString toCString(JSGlobalObject* globalObject, ThrowScope& scope, T& string)
{
    Expected<CString, UTF8ConversionError> expectedString = string.tryGetUtf8();
    if (expectedString)
        return expectedString.value();
    switch (expectedString.error()) {
    case UTF8ConversionError::OutOfMemory:
        throwOutOfMemoryError(globalObject, scope);
        break;
    case UTF8ConversionError::IllegalSource:
        scope.throwException(globalObject, createError(globalObject, "Illegal source encountered during UTF8 conversion"_s));
        break;
    case UTF8ConversionError::SourceExhausted:
        scope.throwException(globalObject, createError(globalObject, "Source exhausted during UTF8 conversion"_s));
        break;
    default:
        RELEASE_ASSERT_NOT_REACHED();
    }
    return { };
}

static void dumpException1(JSGlobalObject* globalObject, JSValue exception)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_CATCH_SCOPE(vm);

#define CHECK_EXCEPTION() do { \
        if (scope.exception()) { \
            scope.clearException(); \
            return; \
        } \
    } while (false)

    auto exceptionString = exception.toWTFString(globalObject);
    CHECK_EXCEPTION();
    Expected<CString, UTF8ConversionError> expectedCString = exceptionString.tryGetUtf8();
    if (expectedCString)
    	NativeJSLogger::log(ERROR, "Exception: %s\n", expectedCString.value().data());
    else
    	NativeJSLogger::log(ERROR, "Exception: <out of memory while extracting exception string>\n");

    Identifier nameID = Identifier::fromString(vm, "name"_s);
    CHECK_EXCEPTION();
    Identifier fileNameID = Identifier::fromString(vm, "sourceURL"_s);
    CHECK_EXCEPTION();
    Identifier lineNumberID = Identifier::fromString(vm, "line"_s);
    CHECK_EXCEPTION();
    Identifier stackID = Identifier::fromString(vm, "stack"_s);
    CHECK_EXCEPTION();

    JSValue nameValue = exception.get(globalObject, nameID);
    CHECK_EXCEPTION();
    JSValue fileNameValue = exception.get(globalObject, fileNameID);
    CHECK_EXCEPTION();
    JSValue lineNumberValue = exception.get(globalObject, lineNumberID);
    CHECK_EXCEPTION();
    JSValue stackValue = exception.get(globalObject, stackID);
    CHECK_EXCEPTION();
    
    auto nameString = nameValue.toWTFString(globalObject);
    CHECK_EXCEPTION();

    if (nameString == "SyntaxError"_s && (!fileNameValue.isUndefinedOrNull() || !lineNumberValue.isUndefinedOrNull())) {
        auto fileNameString = fileNameValue.toWTFString(globalObject);
        CHECK_EXCEPTION();
        auto lineNumberString = lineNumberValue.toWTFString(globalObject);
        CHECK_EXCEPTION();
	NativeJSLogger::log(INFO, "at %s:%s\n", fileNameString.utf8().data(), lineNumberString.utf8().data());
    }
    
    if (!stackValue.isUndefinedOrNull()) {
        auto stackString = stackValue.toWTFString(globalObject);
        CHECK_EXCEPTION();
        if (stackString.length())
            printf("%s\n", stackString.utf8().data());
	    NativeJSLogger::log(INFO, "%s\n", stackString.utf8().data());
    }

#undef CHECK_EXCEPTION
}

static void checkException1(JSGlobalObject* globalObject, bool hasException, JSValue value, bool& success)
{
    //printf("End: %s\n", value.toWTFString(globalObject).utf8().data());
        if (hasException)
            dumpException1(globalObject, value);
}


JSInternalPromise* moduleLoaderImportModule(JSGlobalObject* globalObject, JSModuleLoader*, JSString* moduleNameValue, JSValue parameters, const SourceOrigin& sourceOrigin)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    auto* promise = JSInternalPromise::create(vm, globalObject->internalPromiseStructure());

    auto rejectWithError = [&](JSValue error) {
        promise->reject(globalObject, error);
        return promise;
    };

    auto referrer = sourceOrigin.url();
    auto specifier = moduleNameValue->value(globalObject);
    RETURN_IF_EXCEPTION(scope, promise->rejectWithCaughtException(globalObject, scope));

    if (!referrer.isLocalFile())
        RELEASE_AND_RETURN(scope, rejectWithError(createError(globalObject, makeString("Could not resolve the referrer's path '", referrer.string(), "', while trying to resolve module '", specifier, "'."))));

    bool specifierIsAbsolute = isAbsolutePath(specifier);
    if (!specifierIsAbsolute && !isDottedRelativePath(specifier))
        RELEASE_AND_RETURN(scope, rejectWithError(createTypeError(globalObject, makeString("Module specifier, '"_s, specifier, "' is not absolute and does not start with \"./\" or \"../\". Referenced from: "_s, referrer.fileSystemPath()))));

    auto moduleURL = specifierIsAbsolute ? URL::fileURLWithFileSystemPath(specifier) : URL(referrer, specifier);
    if (!moduleURL.isLocalFile())
        RELEASE_AND_RETURN(scope, rejectWithError(createError(globalObject, makeString("Module url, '", moduleURL.string(), "' does not map to a local file."))));

    auto result = JSC::importModule(globalObject, Identifier::fromString(vm, moduleURL.string()), parameters, jsUndefined());
    RETURN_IF_EXCEPTION(scope, promise->rejectWithCaughtException(globalObject, scope));

    return result;
}

Identifier moduleLoaderResolve(JSGlobalObject* globalObject, JSModuleLoader*, JSValue keyValue, JSValue referrerValue, JSValue)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    scope.releaseAssertNoException();
    const Identifier key = keyValue.toPropertyKey(globalObject);
    RETURN_IF_EXCEPTION(scope, { });

    if (key.isSymbol())
        return key;

    auto resolvePath = [&] (const URL& directoryURL) -> Identifier {
        String specifier = key.impl();
        bool specifierIsAbsolute = isAbsolutePath(specifier);
        if (!specifierIsAbsolute && !isDottedRelativePath(specifier)) {
            throwTypeError(globalObject, scope, makeString("Module specifier, '"_s, specifier, "' is not absolute and does not start with \"./\" or \"../\". Referenced from: "_s, directoryURL.fileSystemPath()));
            return { };
        }

        if (!directoryURL.isLocalFile()) {
            throwException(globalObject, scope, createError(globalObject, makeString("Could not resolve the referrer's path: ", directoryURL.string())));
            return { };
        }

        auto resolvedURL = specifierIsAbsolute ? URL::fileURLWithFileSystemPath(specifier) : URL(directoryURL, specifier);
        if (!resolvedURL.isValid()) {
            throwException(globalObject, scope, createError(globalObject, makeString("Resolved module url is not valid: ", resolvedURL.string())));
            return { };
        }
        ASSERT(resolvedURL.isLocalFile());

        return Identifier::fromString(vm, resolvedURL.string());
    };

    if (referrerValue.isUndefined())
        return resolvePath(currentWorkingDirectory());

    const Identifier referrer = referrerValue.toPropertyKey(globalObject);
    RETURN_IF_EXCEPTION(scope, { });

    if (referrer.isSymbol())
        return resolvePath(currentWorkingDirectory());

    // If the referrer exists, we assume that the referrer is the correct file url.
    URL url = URL({ }, referrer.impl());
    ASSERT(url.isLocalFile());
    return resolvePath(url);
}

JSObject* moduleLoaderCreateImportMetaProperties(JSGlobalObject* globalObject, JSModuleLoader*, JSValue key, JSModuleRecord*, JSValue)
{
    VM& vm = globalObject->vm();
    auto scope = DECLARE_THROW_SCOPE(vm);

    JSObject* metaProperties = constructEmptyObject(vm, globalObject->nullPrototypeObjectStructure());
    RETURN_IF_EXCEPTION(scope, nullptr);

    metaProperties->putDirect(vm, Identifier::fromString(vm, "filename"_s), key);
    RETURN_IF_EXCEPTION(scope, nullptr);

    return metaProperties;
}

JSInternalPromise* moduleLoaderFetch(JSGlobalObject* globalObject, JSModuleLoader*, JSValue key, JSValue, JSValue)
{
    VM& vm = globalObject->vm();
    JSInternalPromise* promise = JSInternalPromise::create(vm, globalObject->internalPromiseStructure());

    auto scope = DECLARE_THROW_SCOPE(vm);

    auto rejectWithError = [&](JSValue error) {
        promise->reject(globalObject, error);
        return promise;
    };

    String moduleKey = key.toWTFString(globalObject);
    RETURN_IF_EXCEPTION(scope, promise->rejectWithCaughtException(globalObject, scope));

    URL moduleURL({ }, moduleKey);
    ASSERT(moduleURL.isLocalFile());
    // Strip the URI from our key so Errors print canonical system paths.
    moduleKey = moduleURL.fileSystemPath();

    Vector<uint8_t> buffer;
    if (!fetchModuleFromLocalFileSystem(moduleURL, buffer))
        RELEASE_AND_RETURN(scope, rejectWithError(createError(globalObject, makeString("Could not open file '", moduleKey, "'."))));

#if ENABLE(WEBASSEMBLY)
    // FileSystem does not have mime-type header. The JSC shell recognizes WebAssembly's magic header.
    if (buffer.size() >= 4) {
        if (buffer[0] == '\0' && buffer[1] == 'a' && buffer[2] == 's' && buffer[3] == 'm') {
            auto source = SourceCode(WebAssemblySourceProvider::create(WTFMove(buffer), SourceOrigin { moduleURL }, WTFMove(moduleKey)));
            scope.releaseAssertNoException();
            auto sourceCode = JSSourceCode::create(vm, WTFMove(source));
            scope.release();
            promise->resolve(globalObject, sourceCode);
            return promise;
        }
    }
#endif

    auto sourceCode = JSSourceCode::create(vm, jscSource(stringFromUTF(buffer), SourceOrigin { moduleURL }, WTFMove(moduleKey), TextPosition(), SourceProviderSourceType::Module));
    scope.release();
    promise->resolve(globalObject, sourceCode);
    return promise;
}

void functionLoadModule(JSGlobalContextRef ref, JSObjectRef globalObjectRef, char *buffer, int len, char* name)
{
    bool isRemoteModule = false;
    if (strncmp(name, "http", 4) == 0)
    {
        isRemoteModule = true;
    }
    String fileName = String::fromLatin1(name);
    Vector<char> scriptBuffer;
    scriptBuffer.append(buffer, len);
    bool success = false;
    SourceOrigin sourceOrigin { absoluteFileURL(fileName) };
    JSInternalPromise* promise = loadAndEvaluateModule(toJS(ref), jscSource(stringFromUTF(scriptBuffer), sourceOrigin, fileName, TextPosition(), SourceProviderSourceType::Module), jsUndefined());
//    JSInternalPromise* promise = loadAndEvaluateModule(toJS(ref), makeSource(stringFromUTF(scriptBuffer), sourceOrigin,  URL({ }, fileName), TextPosition(), SourceProviderSourceType::Module), jsUndefined());
    //JSInternalPromise* promise = loadAndEvaluateModule(toJS(ref), makeSource(stringFromUTF(scriptBuffer), sourceOrigin, URL({ }, fileName), TextPosition(), SourceProviderSourceType::Module), jsUndefined());
    //JSInternalPromise* promise = loadAndEvaluateModule(toJS(ref), makeSource(stringFromUTF(scriptBuffer), SourceOrigin { absolutePath(fileName) }, URL({ }, fileName), TextPosition(), SourceProviderSourceType::Module), funcObj->value());
    //JSInternalPromise* promise = loadAndEvaluateModule(toJS(ref), makeSource(stringFromUTF(scriptBuffer), SourceOrigin { isRemoteModule? fileName:absolutePath(fileName) }, URL({ }, fileName), TextPosition(), SourceProviderSourceType::Module), jsNumber(1000));
    //JSInternalPromise* promise = loadAndEvaluateModule(toJS(ref), makeSource(stringFromUTF(scriptBuffer), SourceOrigin { isRemoteModule? fileName:absolutePath(fileName) }, URL({ }, fileName), TextPosition(), SourceProviderSourceType::Module), jsNumber(1000));
    //JSInternalPromise* promise = loadAndEvaluateModule(toJS(ref), fileName, jsUndefined(), jsNumber(1000));

    JSGlobalObject* globalObject = toJS(ref);
    VM& vm = globalObject->vm();
    //JSFunction* fulfillHandler = JSNativeStdFunction::create(vm, globalObject, 1, String(), [&success](JSGlobalObject* globalObject, CallFrame* callFrame) {
    JSC::JSLockHolder locker(vm);
    JSFunction* fulfillHandler = JSNativeStdFunction::create(vm, globalObject, 1, String(), [&success](JSGlobalObject* globalObject, CallFrame* callFrame) {
        checkException1(jsCast<JSGlobalObject*>(globalObject), false, callFrame->argument(0), success);
        return JSValue::encode(jsUndefined());
    });
    //RETURN_IF_EXCEPTION(scope, encodedJSValue());
    JSFunction* rejectHandler = JSNativeStdFunction::create(vm, globalObject, 1, String(), [&success](JSGlobalObject* globalObject, CallFrame* callFrame) {
        checkException1(jsCast<JSGlobalObject*>(globalObject), true, callFrame->argument(0), success);
        return JSValue::encode(jsUndefined());
    });
    //promise->then(exec, nullptr, errorHandler);
    promise->then(toJS(ref), fulfillHandler, rejectHandler);
}

