#!/bin/bash
set -e

banner() {
  msg="# $* #"
  edge=$(echo "$msg" | sed 's/./#/g')
  echo " "
  echo "$edge"
  echo "$msg"
  echo "$edge"
  echo " "
}

EXT_INSTALL_PATH="$(pwd)/extlibs"
pwd
EXT_DIR="$(pwd)"
make_parallel=3

if [ "$(uname)" = "Darwin" ]; then
    make_parallel="$(sysctl -n hw.ncpu)"
    LIBEXTN=dylib
elif [ "$(uname)" = "Linux" ]; then
    make_parallel="$(grep '^processor' /proc/cpuinfo | wc --lines)"
    LIBEXTN=so
fi

mkdir -p "$EXT_INSTALL_PATH"
cd "$EXT_DIR"

# aamp
if [ ! -e $EXT_INSTALL_PATH/lib/libaampjsbindings.so ]
then
pwd
    banner "aamp"
    mkdir -p aamp

    cd aamp
    echo "[DEBUG] Cloning aamp related repositories"
    git clone -b dev_sprint_24_2 https://code.rdkcentral.com/r/rdk/components/generic/aamp
    git clone -b dev_sprint_24_2 https://code.rdkcentral.com/r/rdk/components/generic/aampabr
    git clone -b dev_sprint_24_2 https://code.rdkcentral.com/r/rdk/components/generic/gst-plugins-rdk-aamp
    git clone -b dev_sprint_24_2 https://code.rdkcentral.com/r/rdk/components/generic/aampmetrics
    git clone https://github.com/DaveGamble/cJSON.git
    git clone -b rdk-next "https://code.rdkcentral.com/r/rdk/components/generic/rdk-oe/meta-rdk-ext"
    echo "[DEBUG] Building aampabr"
    cd aampabr
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH ..
    make
    make install
    cd ../..

    echo "[DEBUG] Building cJSON"
    cd cJSON
    mkdir -p build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH ..
    make
    make install
    cd ../..

  echo "[DEBUG] Building aampmetrics"
     cd aampmetrics
     git apply ../../aampmetrics.diff

    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH ..
    make
    make install
    cd ../..

echo "[DEBUG] Cloning libdash"
    git clone https://github.com/bitmovin/libdash.git
    cd libdash/libdash
    git checkout stable_3_0

    echo "[DEBUG] Checkout libdash"
    patch -p1 < ../../meta-rdk-ext/recipes-multimedia/libdash/libdash/0001-libdash-build.patch
    patch -p1 < ../../meta-rdk-ext/recipes-multimedia/libdash/libdash/0002-libdash-starttime-uint64.patch
    patch -p1 < ../../meta-rdk-ext/recipes-multimedia/libdash/libdash/0003-libdash-presentationTimeOffset-uint64.patch
    patch -p1 < ../../meta-rdk-ext/recipes-multimedia/libdash/libdash/0004-Support-of-EventStream.patch
    patch -p1 < ../../meta-rdk-ext/recipes-multimedia/libdash/libdash/0005-DELIA-39460-libdash-memleak.patch
    patch -p1 < ../../meta-rdk-ext/recipes-multimedia/libdash/libdash/0006-RDK-32003-LLD-Support.patch
    patch -p1 < ../../meta-rdk-ext/recipes-multimedia/libdash/libdash/0007-DELIA-51645-Event-Stream-RawAttributes-Support.patch
    patch -p1 < ../../meta-rdk-ext/recipes-multimedia/libdash/libdash/0008-DELIA-53263-Use-Label-TAG.patch
    patch -p1 < ../../meta-rdk-ext/recipes-multimedia/libdash/libdash/0009-RDK-35134-Support-for-FailoverContent.patch
    patch -p1 < ../../meta-rdk-ext/recipes-multimedia/libdash/libdash/0010-RDKAAMP-121-Failover-Tag-on-SegmentTemplate.patch
    patch -p1 < ../../meta-rdk-ext/recipes-multimedia/libdash/libdash/0011-RDKAAMP-61-AAMP-low-latency-dash-stream-evaluation.patch
    patch -p1 < ../../meta-rdk-ext/recipes-multimedia/libdash/libdash/0012-To-retrieves-the-text-content-of-CDATA-section.patch
    echo "[DEBUG] PAtches applied for libdash"
    mkdir -p build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH ..
    make
    make install
    cp bin/libdash.so $EXT_INSTALL_PATH/lib/
    ../../../../copydashheaders.sh
    ../../../../preparedashpc.sh
    cd ../../../

    if [ ! -e $EXT_INSTALL_PATH/lib/libsystemd.so ]
    then
        ln -s /usr/lib/x86_64-linux-gnu/libsystemd.so.0 $EXT_INSTALL_PATH/lib/libsystemd.so
    fi

    echo "[DEBUG}Going to apply aamp patches"
    cd aamp
    git apply ../../aamp.diff
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PLATFORM_UBUNTU=1 -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH -DCMAKE_WPEWEBKIT_JSBINDINGS=ON -DCMAKE_INBUILT_AAMP_DEPENDENCIES=1 ..
    make
    echo "[DEBUG] Install aamp after aamp.diff"
    make install
    cd ../../

echo "[DEBUG}Going to apply gstplugins patches"
    cd gst-plugins-rdk-aamp
    git apply ../../gstplugins.diff
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH ..
    make
    make install
    cd ../../
fi
cd ${EXT_DIR}

banner ">>>>>  BUILD COMPLETE  <<<<<"
 
