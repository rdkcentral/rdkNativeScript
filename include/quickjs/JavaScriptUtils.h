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

#ifndef JAVASCRIPTUTILS_H
#define JAVASCRIPTUTILS_H

#include <quickjs.h>
void printException(JSContext* ctx, JSValue& exception_val);
void registerHttpInterface(JSContext *ctx);
void registerWebSocketInterface(JSContext *ctx);
void registerPlayerInterface(JSContext *ctx);
void registerThunderTokenInterface(JSContext *ctx, void* data);
#endif
