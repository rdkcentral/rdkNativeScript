# /**
#  * If not stated otherwise in this file or this component's LICENSE
#  * file the following copyright and licenses apply:
#  *
#  * Copyright 2024 RDK Management
#  *
#  * Licensed under the Apache License, Version 2.0 (the "License");
#  * you may not use this file except in compliance with the License.
#  * You may obtain a copy of the License at
#  *
#  * http://www.apache.org/licenses/LICENSE-2.0
#  *
#  * Unless required by applicable law or agreed to in writing, software
#  * distributed under the License is distributed on an "AS IS" BASIS,
#  * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
#  * See the License for the specific language governing permissions and
#  * limitations under the License.
#  **/

#!/bin/bash
mount --bind /opt/WebKitBrowser.json /etc/WPEFramework/plugins/WebKitBrowser.json
systemctl daemon-reload
systemctl restart wpeframework
# Set environment variables
export XDG_RUNTIME_DIR=/tmp
export WAYLAND_DISPLAY=test
export LD_LIBRARY_PATH=/usr/lib:/media/apps/netflix/usr/lib:/tmp/netflix/usr/lib:/media/apps/libcobalt/usr/lib:/tmp/libcobalt/usr/lib
export GST_REGISTRY=/opt/.gstreamer/registry.bin
export GST_REGISTRY_UPDATE=no
export ENABLE_WEBKITBROWSER_PLUGIN_ACCESSIBILITY=1
export TTS_USE_THUNDER_CLIENT=1
export LD_PRELOAD=/usr/lib/realtek/libVOutWrapper.so:/usr/lib/realtek/libjpu.so:/usr/lib/realtek/libvpu.so:/usr/lib/libwesteros_gl.so.0
export XDG_DATA_HOME=/opt/QT/home
export RDKSHELL_EASTER_EGG_FILE=/etc/rdkshell_eastereggs.conf
export FORCE_SVP=TRUE
export FORCE_SAP=TRUE

echo "Commands executed successfully."
