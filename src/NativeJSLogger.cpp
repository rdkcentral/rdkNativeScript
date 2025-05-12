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

static const char* logLevelNames[] = {
        "DEBUG", "INFO", "WARN", "ERROR", "FATAL"
};

LogLevel NativeJSLogger::sLogLevel = INFO;

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

      int threadId = syscall(__NR_gettid);
      const char* levelStr = logLevelNames[level];
      char buffer[512];

      va_list args;
      va_start(args, format);
      vsnprintf(buffer, sizeof(buffer), format, args);
      va_end(args);

      printf("\n[%s] JsRuntime Thread-%d: %s\n", levelStr, threadId, buffer);
}

