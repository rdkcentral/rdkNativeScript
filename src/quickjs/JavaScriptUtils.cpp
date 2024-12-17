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

#include "JavaScriptUtils.h"
#include "JavaScriptContext.h"
#include <string>
#include <string.h>
#ifdef ENABLE_JSRUNTIME_THUNDER_SECURITYAGENT
#include <securityagent/securityagent.h>
#endif
using namespace std;

#define MAX_TOKEN_BUFFER_LENGTH 2048

static JSValue getThunderToken(JSContext *ctx, JSValueConst this_val, int argc, JSValueConst *argv)
{
#ifdef ENABLE_JSRUNTIME_THUNDER_SECURITYAGENT
    void* data = JS_GetContextOpaque(ctx);
    JavaScriptContext* jsccontext = (JavaScriptContext*) data;
    if (!ctx)
    {
        printf("context is empty !!! \n");
        fflush(stdout);
        return JS_NewString(ctx, "");
    }
    unsigned char tokenBuffer[MAX_TOKEN_BUFFER_LENGTH];
    memset(tokenBuffer, 0, MAX_TOKEN_BUFFER_LENGTH);
    std::string appUrl = jsccontext->getUrl();
    size_t paramsLength = (size_t)appUrl.size();
    if(!memcpy(tokenBuffer,appUrl.c_str(),paramsLength))
    {
        printf("unable to copy url buffer for token");
        return JS_NewString(ctx, "");
    }

    printf("thunder request: %s length: %d", (char*)tokenBuffer, (int)paramsLength);
    fflush(stdout);
    int tokenResult = GetToken(MAX_TOKEN_BUFFER_LENGTH, paramsLength, tokenBuffer);
    if (tokenResult < 0)
    {
        printf("unable to get token for app\n");
        fflush(stdout);
        return JS_NewString(ctx, "");
    }
    return JS_NewString(ctx, (const char*)tokenBuffer);
#else
    return JS_NewString(ctx, "");
#endif
}

void registerThunderTokenInterface(JSContext *ctx, void* data)
{
    auto global_obj=JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global_obj, "thunderToken",
                      JS_NewCFunction(ctx, getThunderToken, "thunderToken", 0));
}
