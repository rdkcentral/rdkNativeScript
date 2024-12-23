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
        ${JSRUNTIME_SOURCE_DIRECTORY}/JavaScriptWrapper.cpp
        ${JSRUNTIME_SOURCE_DIRECTORY}/rtScriptJSCPrivate.cpp
        ${JSRUNTIME_SOURCE_DIRECTORY}/jsc_lib.cpp
        ${JSRUNTIME_COMMON_SOURCE_DIRECTORY}/rtWebSocket.cpp
)

#set(PORT "JSCOnly")
#set(ENABLE_STATIC_JSC ON)
#set(USE_CAPSTONE OFF)
#set(BUILD_SHARED_LIBS OFF)
#set(USE_LD_GOLD OFF)
#set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
#set(CMAKE_BUILD_TYPE Release)
#set(CMAKE_GENERATOR "Ninja")
#set(DEVELOPER_MODE ON)
#set(ENABLE_JIT ON)
#set(ENABLE_STATIC_JSC ON)
#set(ENABLE_REMOTE_INSPECTOR ON)

if (ENABLE_JSRUNTIME_PLAYER)
if (NOT ENABLE_AAMP_JSBINDINGS)
    set (JSRUNTIME_ENGINE_FILES ${JSRUNTIME_ENGINE_FILES} ${JSRUNTIME_SOURCE_DIRECTORY}/PlayerWrapper.cpp ${JSRUNTIME_SOURCE_DIRECTORY}/PlayerEventHandler.cpp ${JSRUNTIME_SOURCE_DIRECTORY}/PlayerEventPropsHandler.cpp)
endif (NOT ENABLE_AAMP_JSBINDINGS)
endif (ENABLE_JSRUNTIME_PLAYER)

if ( ENABLE_WEBSOCKET_SERVER )
    set (JSRUNTIME_ENGINE_FILES ${JSRUNTIME_ENGINE_FILES} ${JSRUNTIME_COMMON_SOURCE_DIRECTORY}/rtWebSocketServer.cpp ${JSRUNTIME_COMMON_SOURCE_DIRECTORY}/rtWebSocketClient.cpp)
    add_definitions("-DWS_SERVER_ENABLED")
endif (ENABLE_WEBSOCKET_SERVER )
add_definitions("-DRT_PLATFORM_LINUX -DUSE_UV  -DNDEBUG -DRELEASE_WITHOUT_OPTIMIZATIONS -fno-rtti -DSTATICALLY_LINKED_WITH_WTF -DUSE_GLIB")

if (NOT BUILD_JSRUNTIME_DESKTOP)
    set(JAVASCRIPT_CORE_LIBRARIES -lJavaScriptCore -lpthread -ldl  -lgio-2.0 -licui18n)
else ()
    if (APPLE)
        set(CMAKE_FRAMEWORK_PATH $ENV{PKG_CONFIG_SYSROOT_DIR}/lib)
        find_library(JAVASCRIPT_CORE_LIBRARIES NAMES JavaScriptCore)
    else ()
        set(JAVASCRIPT_CORE_LIBRARIES -lJavaScriptCore -lpthread -ldl)
    endif (APPLE)
endif (NOT BUILD_JSRUNTIME_DESKTOP)

set(JSRUNTIME_ENGINE_LIBRARIES ${JAVASCRIPT_CORE_LIBRARIES} -lglib-2.0 -lgio-2.0 -lrtCoreExt)

if (ENABLE_JSRUNTIME_PLAYER)
    set(JSRUNTIME_ENGINE_LIBRARIES ${JSRUNTIME_ENGINE_LIBRARIES} -lgstreamer-1.0)
    if (ENABLE_AAMP_JSBINDINGS_STATIC)
        set(JSRUNTIME_ENGINE_LIBRARIES ${JSRUNTIME_ENGINE_LIBRARIES} -laamp -labr -lcjson -lsubtec -lmetrics -ldash -laampjsbindings)
    endif (ENABLE_AAMP_JSBINDINGS_STATIC)
    if (NOT ENABLE_AAMP_JSBINDINGS)
        set(JSRUNTIME_ENGINE_LIBRARIES ${JSRUNTIME_ENGINE_LIBRARIES} -laamp -labr -lcjson -lsubtec -lmetrics -ldash)
    endif (NOT ENABLE_AAMP_JSBINDINGS)
endif (ENABLE_JSRUNTIME_PLAYER)

set(JSRUNTIME_ENGINE_LIBRARY_LINK_DIRECTORIES "-L$ENV{PKG_CONFIG_SYSROOT_DIR}/usr/lib/javascriptcore")
set(JSRUNTIME_ENGINE_INCLUDE_DIRECTORIES "")

if (BUILD_JSRUNTIME_DESKTOP)
    set(JSRUNTIME_ENGINE_INCLUDE_DIRECTORIES $ENV{PKG_CONFIG_SYSROOT_DIR}/include/pxcore $ENV{PKG_CONFIG_SYSROOT_DIR}/include $ENV{PKG_CONFIG_SYSROOT_DIR}/include/JavaScriptCore $ENV{PKG_CONFIG_SYSROOT_DIR}/include/wtf/icu $ENV{PKG_CONFIG_SYSROOT_DIR}/include/aamp /usr/include/gstreamer-1.0)
else ()
    set(JSRUNTIME_ENGINE_INCLUDE_DIRECTORIES ${JSRUNTIME_ENGINE_INCLUDE_DIRECTORIES} $ENV{PKG_CONFIG_SYSROOT_DIR}/usr/include/wpe-webkit-1.0  $ENV{PKG_CONFIG_SYSROOT_DIR}/usr/include/wpe-webkit-1.0/JavaScriptCore $ENV{PKG_CONFIG_SYSROOT_DIR}/usr/include/JavaScriptCore $ENV{PKG_CONFIG_SYSROOT_DIR}/include/wtf/icu)
endif (BUILD_JSRUNTIME_DESKTOP)
