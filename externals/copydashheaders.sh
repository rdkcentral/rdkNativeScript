###
# If not stated otherwise in this file or this component's LICENSE
# file the following copyright and licenses apply:
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
###

sudo cp bin/libdash.so ${EXT_INSTALL_PATH}/lib/
sudo mkdir -p ${EXT_INSTALL_PATH}/include
sudo mkdir -p ${EXT_INSTALL_PATH}/include/libdash
sudo mkdir -p ${EXT_INSTALL_PATH}/include/libdash/xml
sudo mkdir -p ${EXT_INSTALL_PATH}/include/libdash/mpd
sudo mkdir -p ${EXT_INSTALL_PATH}/include/libdash/helpers
sudo mkdir -p ${EXT_INSTALL_PATH}/include/libdash/network
sudo mkdir -p ${EXT_INSTALL_PATH}/include/libdash/portable
sudo mkdir -p ${EXT_INSTALL_PATH}/include/libdash/metrics
sudo cp -pr ../libdash/include/*.h ${EXT_INSTALL_PATH}/include/libdash
sudo cp -pr ../libdash/source/xml/*.h ${EXT_INSTALL_PATH}/include/libdash/xml
sudo cp -pr ../libdash/source/mpd/*.h ${EXT_INSTALL_PATH}/include/libdash/mpd
sudo cp -pr ../libdash/source/network/*.h ${EXT_INSTALL_PATH}/include/libdash/network
sudo cp -pr ../libdash/source/portable/*.h ${EXT_INSTALL_PATH}/include/libdash/portable
sudo cp -pr ../libdash/source/helpers/*.h ${EXT_INSTALL_PATH}/include/libdash/helpers
sudo cp -pr ../libdash/source/metrics/*.h ${EXT_INSTALL_PATH}/include/libdash/metrics

