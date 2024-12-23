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
#include <list>
#include <string.h>
#include <sys/stat.h>
#include <fstream>
#include <sstream>
#include <quickjs-libc.h>
#ifdef ENABLE_JSRUNTIME_THUNDER_SECURITYAGENT
#include <securityagent/securityagent.h>
#endif
using namespace std;

#define MAX_TOKEN_BUFFER_LENGTH 2048

static bool fileExists(const char* name)
{
    struct stat buffer;
    bool ret = (stat (name, &buffer) == 0);
    return ret;
}

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

static std::string readFile(const char *file)
{
  std::ifstream       src_file(file);
  std::stringstream   src_script;
  src_script << src_file.rdbuf();
  return src_script.str();
}

static JSValue requireCallback(JSContext *ctx, JSValueConst thisObject, int argc, JSValueConst *argv)
{
  const char* module;
  void* data = JS_GetContextOpaque(ctx);
  JavaScriptContext* jsccontext = (JavaScriptContext*) data;
  if (!ctx)
  {
      printf("context is empty !!! \n");
      fflush(stdout);
      return JS_NewString(ctx, "");
  }

  if (argc != 1)
      return JS_NewString(ctx, "");
      //return JSValueMakeNull(ctx);

  static const auto resolveModulePath = [](const string &name, string &data) -> bool
  {
    std::list<string> dirs;
    std::list<string> endings;
    bool found = false;
    string path;

    dirs.push_back("");
    dirs.push_back("modules/");
    dirs.push_back("modules/lib");
    endings.push_back(".js");

    std::list<string>::const_iterator it, jt;
    for (it = dirs.begin(); !found && it != dirs.end(); ++it)
    {
        string s = *it;
        if (!s.empty() && (s.back() != '/'))
          s.append("/");
        s.append((name.find("./") == 0)? name.substr(2) : name);
        for (jt = endings.begin(); !found && jt != endings.end(); ++jt) {
          path = s;
          int pos = path.find((*jt).c_str());
          int pathLen = path.length();
          int endingsLen = (*jt).length();
          bool endingHere = false;
          if ((pathLen - pos) == endingsLen)
          {
              endingHere = true;		  
          }		  
          if (!endingHere)
            path.append(*jt);
          found = fileExists(path.c_str());
        }
    }

    if (found)
      data = path;
    return found;
  };

  do {
    if (JS_IsString(argv[0]))
    {
        module = JS_ToCString(ctx, argv[0]);
    }
    string moduleName(module);
    string path;
    if (!resolveModulePath(moduleName, path)) {
      //need to release memory for modulename ??
      printf("Module '%s' not found", moduleName.c_str());
      break;
    }
    /*
    JSGlobalContextRef globalCtx = JSContextGetGlobalContext(ctx);
    rtJSCContextPrivate* priv = rtJSCContextPrivate::fromCtx(globalCtx);
    if (!priv) {
      rtLogError(" %s  ... no priv object.",__PRETTY_FUNCTION__);
      break;
    }

    if (JSObjectRef moduleObj = priv->findModule(path)) {
      JSStringRelease(reqArgStr);
      return moduleObj;
    }
    */
    //printf("Loading %s", path.c_str());
    std::string codeStr = readFile(path.c_str());
    if (codeStr.empty())
    {
      //need to see if to release ??
      //JSStringRelease(reqArgStr);
      printf(" %s  ... load error / not found.",__PRETTY_FUNCTION__);
      break;
    }

    codeStr =
        "(function(){ let m = {}; m.exports = {}; \n"
        "  (function(module, exports){\n"
        + codeStr +
        "  \n}).call(undefined, m, m.exports); return m;})()";

    JSValue retValue = JS_Eval(ctx, codeStr.c_str(), codeStr.length(), module==nullptr?"":module, 0);
    if (JS_IsException(retValue))
    {
        printf("Error evaluating script\r\n");
        js_std_dump_error(ctx);
        return JS_NewString(ctx, "");
        //return false;
    }
    js_std_loop(ctx);

    JSValue exportsVal = JS_GetPropertyStr(ctx, retValue, "exports");
    return exportsVal;
    /*
    JSObjectRef moduleObj = JSValueToObject(globalCtx, module, exception);
    if (exception && *exception) {
      JSStringRef exceptStr = JSValueToStringCopy(globalCtx, *exception, nullptr);
      rtString errorStr = jsToRtString(exceptStr);
      JSStringRelease(exceptStr);
      rtLogError("Failed to convert module to object, \n\terror='%s'\n\tmodule=%s", errorStr.cString(), path.cString());
      break;
    }

    static JSStringRef exportsStr = JSStringCreateWithUTF8CString("exports");
    JSValueRef exportsVal = JSObjectGetProperty(globalCtx, moduleObj, exportsStr, exception);
    if (exception && *exception) {
      JSStringRef exceptStr = JSValueToStringCopy(globalCtx, *exception, nullptr);
      rtString errorStr = jsToRtString(exceptStr);
      JSStringRelease(exceptStr);
      rtLogError("Failed to get exports module to object, \n\terror='%s'\n\tmodule=%s", errorStr.cString(), path.cString());
      break;
    }
    */
    /*
    JSObjectRef exportsObj = JSValueToObject(globalCtx, exportsVal, exception);
    if (exception && *exception) {
      printException(globalCtx, *exception);
      break;
    }
    priv->addToModuleCache(path, globalCtx, exportsObj);
    return exportsVal;
*/
  } while(0);
  //return JSValueMakeNull(ctx);
  return JS_NewString(ctx, "");
}

void registerThunderTokenInterface(JSContext *ctx, void* data)
{
    auto global_obj=JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global_obj, "thunderToken",
                      JS_NewCFunction(ctx, getThunderToken, "thunderToken", 0));
}

void registerRequireInterface(JSContext *ctx, void* data)
{
    auto global_obj=JS_GetGlobalObject(ctx);
    JS_SetPropertyStr(ctx, global_obj, "require",
                      JS_NewCFunction(ctx, requireCallback, "require", 0));
}
