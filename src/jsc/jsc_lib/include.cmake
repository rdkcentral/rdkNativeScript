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

set(JSRUNTIME_ENGINE_DEPENDENT_LIBRARY_NAME "jsclib")
set (JSRUNTIME_ENGINE_DEPENDENT_FILES
        ${JSRUNTIME_SOURCE_DIRECTORY}/jsclib/jsclib.cpp
)

set(JSRUNTIME_ENGINE_DEPENDENT_LIBRARIES -ljsclib)

add_library(${JSRUNTIME_ENGINE_DEPENDENT_LIBRARY_NAME} SHARED
	        ${JSRUNTIME_ENGINE_DEPENDENT_FILES}
)
target_include_directories(${JSRUNTIME_ENGINE_DEPENDENT_LIBRARY_NAME} PRIVATE ${JSRUNTIME_INCLUDE_DIRECTORIES})
