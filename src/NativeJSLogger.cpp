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

#include "NativeJSLogger.h"
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdlib.h>

#ifdef USE_ETHANLOG
#include <ethanlog.h>

static const int ethanLogLevelMap[] = {
    ETHAN_LOG_DEBUG, ETHAN_LOG_INFO, ETHAN_LOG_WARNING, ETHAN_LOG_ERROR, ETHAN_LOG_FATAL   
};
#endif

static const char* logLevelNames[] = {
        "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};


LogLevel NativeJSLogger::sLogLevel = INFO;
bool NativeJSLogger::mEthanLogEnabled = false;

void NativeJSLogger::isEthanLogEnabled()
{
    mEthanLogEnabled  = (getenv("ETHAN_LOGGING_PIPE") != nullptr);
}

void NativeJSLogger::setLogLevel(const char* loglevel)
{
      if (!loglevel) return;

      if (strcasecmp(loglevel, "debug") == 0)
            sLogLevel = DEBUG;
      else if (strcasecmp(loglevel, "info") == 0)
            sLogLevel = INFO;
      else if (strcasecmp(loglevel, "warn") == 0)
            sLogLevel = WARN;
      else if (strcasecmp(loglevel, "error") == 0)
            sLogLevel = ERROR;
      else if (strcasecmp(loglevel, "fatal") == 0)
            sLogLevel = FATAL;
}

void NativeJSLogger::log(LogLevel level, const char* format, ...)
{
    if (level < sLogLevel)
        return;

    va_list args;
    va_start(args, format);
	int threadId = syscall(__NR_gettid);

#ifdef USE_ETHANLOG
    if (mEthanLogEnabled)
    {
	int ethanLogLevel = ethanLogLevelMap[level];
        char* newFormat = nullptr;
        int len = snprintf(nullptr, 0, "JSRuntime [Thread-%d] %s", threadId, format);
        if (len > 0) {
            newFormat = (char*)alloca(len + 1);
            snprintf(newFormat, len + 1, "JSRuntime [Thread-%d] %s", threadId, format);
        } else {
            newFormat = (char*)format;
        }

        vethanlog(ethanLogLevel, "JsRuntime", nullptr, -1, newFormat, args);
    }
#else
    
    const char* levelStr = logLevelNames[level];
    char buffer[512];
    vsnprintf(buffer, sizeof(buffer), format, args);
    printf("\n[%s] JsRuntime Thread-%d: %s\n", levelStr, threadId, buffer);

#endif //USE_ETHANLOG    

    va_end(args);
}
