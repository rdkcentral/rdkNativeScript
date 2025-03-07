# Copyright (c) 2024 RDK Management.
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation;
# version 2.1 of the License.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; See the file COPYING.LGPL. if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA


set(JSRUNTIME_ENGINE_DEPENDENT_LIBRARY_NAME "jsclib")
set (JSRUNTIME_ENGINE_DEPENDENT_FILES
        ${JSRUNTIME_SOURCE_DIRECTORY}/jsc_lib/jsc_lib.cpp
)

set(JSRUNTIME_ENGINE_DEPENDENT_LIBRARIES -ljsclib)

add_library(${JSRUNTIME_ENGINE_DEPENDENT_LIBRARY_NAME} SHARED
	        ${JSRUNTIME_ENGINE_DEPENDENT_FILES}
)
target_include_directories(${JSRUNTIME_ENGINE_DEPENDENT_LIBRARY_NAME} PRIVATE ${JSRUNTIME_INCLUDE_DIRECTORIES})
