#!/bin/bash
#
#GENERIC VERSION
echo "launching Runtime App"

cd ../
cd runtime
echo "The current working directory is: $(pwd)"
#ls -l
mkdir /tmp/jsruntime
ln -s /usr/lib/libocdmRialto.so.1 /tmp/jsruntime/libocdm.so.4
ls -lrt /tmp/jsruntime/

export LD_LIBRARY_PATH=/runtime/usr/lib/javascriptcore:/runtime/usr/lib:/tmp/jsruntime/:/usr/lib
export SKY_DBUS_DISABLE_UID_IN_EXTERNAL_AUTH=1
export LD_PRELOAD=/usr/lib/realtek/libVOutWrapper.so:/usr/lib/realtek/libjpu.so:/usr/lib/realtek/libvpu.so:/usr/lib/libwesteros_gl.so.0

./JSRuntimeLauncher

