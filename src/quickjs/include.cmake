# If not stated otherwise in this file or this component's license file the
# following copyright and licenses apply:
#
# Copyright 2024 RDK Management
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

set (JSRUNTIME_ENGINE_FILES
        ${JSRUNTIME_SOURCE_DIRECTORY}/JavaScriptEngine.cpp
        ${JSRUNTIME_SOURCE_DIRECTORY}/JavaScriptContext.cpp
        ${JSRUNTIME_SOURCE_DIRECTORY}/JavaScriptUtils.cpp
        ${JSRUNTIME_SOURCE_DIRECTORY}/JSWebSocket.cpp
        ${JSRUNTIME_SOURCE_DIRECTORY}/JSPlayer.cpp
)
#${JSRUNTIME_SOURCE_DIRECTORY}/JSHttp.cpp

add_definitions("-DDISABLE_RTUTILS")
set(JSRUNTIME_ENGINE_LIBRARIES -lquickjs.lto -laamp -labr -lcjson -lsubtec -lmetrics -ldash -lglib-2.0 -lgstreamer-1.0)

set(JSRUNTIME_ENGINE_LIBRARY_LINK_DIRECTORIES "")
set(JSRUNTIME_ENGINE_INCLUDE_DIRECTORIES "")

if (BUILD_JSRUNTIME_DESKTOP)
    set(JSRUNTIME_ENGINE_INCLUDE_DIRECTORIES $ENV{PKG_CONFIG_SYSROOT_DIR}/include/quickjs $ENV{PKG_CONFIG_SYSROOT_DIR}/include/aamp /usr/include/gstreamer-1.0)
else ()
    set(JSRUNTIME_ENGINE_INCLUDE_DIRECTORIES $ENV{PKG_CONFIG_SYSROOT_DIR}/usr/include/quickjs)
endif (BUILD_JSRUNTIME_DESKTOP)
