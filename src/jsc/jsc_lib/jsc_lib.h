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

//#include "config.h"
#define JSC_API_AVAILABLE(...)
#define JSC_CLASS_AVAILABLE(...) JS_EXPORT
#define JSC_API_DEPRECATED(...)
// Use zero since it will be less than any possible version number.
#define JSC_MAC_VERSION_TBA 0
#define JSC_IOS_VERSION_TBA 0

#include "JSExportMacros.h"

#ifdef __cplusplus
#undef new
#undef delete
#include <wtf/FastMalloc.h>
#endif

#include <wtf/DisallowCType.h>


#include <VM.h>
#include "JSInternalPromise.h"
#include "JSObjectInlines.h"
#include "APICast.h"
#include "ArrayBuffer.h"
#include "BytecodeCacheError.h"
#include "CatchScope.h"
//#include "CodeBlock.h"
#include "Completion.h"
#include "Exception.h"
#include "ExceptionHelpers.h"
#include "ExceptionScope.h"
#include "HeapSnapshotBuilder.h"
//#include "JITOperationList.h"
#include "JSArray.h"
#include "JSArrayBuffer.h"
#include "JSBasePrivate.h"
#include "JSBigInt.h"
#include "JSFunction.h"
#include "JSFunctionInlines.h"
#include "JSLock.h"
#include "JSNativeStdFunction.h"
#include "JSSourceCode.h"
#include "JSString.h"
#include "JSTypedArrays.h"
#include "ObjectConstructor.h"
#include "TypedArrayInlines.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <type_traits>
#include <wtf/Scope.h>
/*
#include <wtf/CPUTime.h>
#include <wtf/FileSystem.h>
#include <wtf/MainThread.h>
#include <wtf/MemoryPressureHandler.h>
#include <wtf/MonotonicTime.h>
#include <wtf/SafeStrerror.h>
#include <wtf/Span.h>
#include <wtf/StringPrintStream.h>
#include <wtf/URL.h>
#include <wtf/WallTime.h>
#include <wtf/text/StringBuilder.h>
#include <wtf/threads/BinarySemaphore.h>
#include <wtf/threads/Signals.h>
*/
#include <sys/stat.h>
#include <iostream>
#include <curl/curl.h>
#include <unistd.h>

#include <unistd.h>

#include <JavaScriptCore/JSGlobalObject.h>

#if HAVE(READLINE)
// readline/history.h has a Function typedef which conflicts with the WTF::Function template from WTF/Forward.h
// We #define it to something else to avoid this conflict.
#define Function ReadlineFunction
#include <readline/history.h>
#include <readline/readline.h>
#undef Function
#endif

#if COMPILER(MSVC)
#include <crtdbg.h>
#include <mmsystem.h>
#include <windows.h>
#endif

#if !defined(PATH_MAX)
#define PATH_MAX 4096
#endif

void functionLoadModule(JSGlobalContextRef ref, JSObjectRef globalObjectRef, char *buffer, int len, char* name);

#ifdef USE_JSCLIB_MOCK

typedef unsigned short UChar;
UChar pathSeparator();
bool isAbsolutePath(JSC::StringView path);
bool isDottedRelativePath(JSC::StringView path);
void convertShebangToJSCommentWrapper(char* buffer, size_t size);
bool fetchScriptFromLocalFileSystemWrapper(const char* fileName, char** outBuffer, size_t* outSize);
JSC::String stringFromUTFWrapper(const char* data, size_t size);
bool fillBufferWithContentsOfFileWrapper(const char* fileName, char** outBuffer, size_t* outSize);
bool absoluteFileURLWrapper(const char* fileName, char** outURL, size_t* outSize);
bool fetchScriptFromLocalFileSystemVectorWrapper(const char* fileName, char** outBuffer, size_t* outSize);
bool fillBufferWithContentsOfFileUint8Wrapper(const char* fileName, unsigned char** outBuffer, size_t* outSize);
bool currentWorkingDirectoryWrapper(char** outPath, size_t* outSize);


struct MemoryStruct;

bool downloadFile(std::string& url, MemoryStruct& chunk);
#endif 
