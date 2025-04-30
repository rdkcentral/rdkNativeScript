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

##!/bin/bash
set -e
# Any subsequent(*) commands which fail will cause the shell script to exit immediately

banner() {
  msg="# $* #"
  edge=$(echo "$msg" | sed 's/./#/g')
  echo " "
  echo "$edge"
  echo "$msg"
  echo "$edge"
  echo " "
}

#--------- Args

PXCORE_VER="rdk"
#WPEWEBKIT_VER="583d02964d606c0f600ce5a3df98e017c8712931"
WPEWEBKIT_VER="44cb95d724470c1180d1f4942d49a0d3154a734e"	
OPENSSL_VER="1.1.1g"
ZLIB_VER="1.2.11"
GLIB_VER="2.62.4"
CURL_VER="7.69.1"

#Download files
wget https://www.openssl.org/source/openssl-1.1.1g.tar.gz
wget https://sourceforge.net/projects/libpng/files/zlib/1.2.11/zlib-1.2.11.tar.xz
wget https://gitlab.gnome.org/GNOME/glib/-/archive/2.62.4/glib-2.62.4.tar.gz
wget https://curl.se/download/curl-7.69.1.tar.gz
git clone https://github.com/libuv/libuv.git
git clone https://github.com/Tencent/rapidjson.git

git clone https://github.com/uNetworking/uWebSockets.git
cd uWebSockets
git checkout v0.14
cd ..

#extract files
tar -xzf openssl-1.1.1g.tar.gz
tar -xf zlib-1.2.11.tar.xz
tar -xzf glib-2.62.4.tar.gz
tar -xzf curl-7.69.1.tar.gz

#remove the files
rm -rf openssl-1.1.1g.tar.gz
rm -rf zlib-1.2.11.tar.xz
rm -rf glib-2.62.4.tar.gz
rm -rf curl-7.69.1.tar.gz

make_parallel=3
if [ "$(uname)" = "Darwin" ]; then
    make_parallel="$(sysctl -n hw.ncpu)"
    LIBEXTN=dylib
elif [ "$(uname)" = "Linux" ]; then
    make_parallel="$(cat /proc/cpuinfo | grep '^processor' | wc --lines)"
    LIBEXTN=so
fi

EXT_INSTALL_PATH=`pwd`/extlibs
EXT_DIR=`pwd`
export EXT_INSTALL_PATH=`pwd`/extlibs

mkdir -p $EXT_INSTALL_PATH

if [ "$(uname)" = "Darwin" ]; then
    make_parallel="$(sysctl -n hw.ncpu)"
    LIBEXTN=dylib
elif [ "$(uname)" = "Linux" ]; then
    make_parallel="$(cat /proc/cpuinfo | grep '^processor' | wc --lines)"
    LIBEXTN=so
fi

cd ${EXT_DIR}
#--------- OPENSSL
OPENSSL_DIR="`pwd`/openssl-${OPENSSL_VER}"
if [ ! -e ${EXT_INSTALL_PATH}/lib/libcrypto.so.1.1 ] || [ ! -e ${EXT_INSTALL_PATH}/lib/libssl.so.1.1 ]
then

  cd ${OPENSSL_DIR}

  if [ "$(uname)" != "Darwin" ]
  then
    ./config -shared  --prefix=${EXT_INSTALL_PATH}
  else
    ./Configure darwin64-x86_64-cc -shared --prefix=${EXT_INSTALL_PATH}
  fi

  make clean
  make "-j${make_parallel}"
  make install -i

  rm -rf libcrypto.a
  rm -rf libssl.a
  rm -rf lib/libcrypto.a
  rm -rf lib/libssl.a
  cd ..
fi
#
##export LD_LIBRARY_PATH="${OPENSSL_DIR}/:$LD_LIBRARY_PATH"
##export DYLD_LIBRARY_PATH="${OPENSSL_DIR}/:$DYLD_LIBRARY_PATH"
export PKG_CONFIG_PATH=$EXT_INSTALL_PATH/lib/pkgconfig:$EXT_INSTALL_PATH/lib/x86_64-linux-gnu/pkgconfig
#
cd ${EXT_DIR}
#--------- GLIB

GLIB_DIR="`pwd`/glib-${GLIB_VER}"
if [ ! -e ${EXT_INSTALL_PATH}/lib/x86_64-linux-gnu/libglib-2.0.so.0.6200.4 ]
then

 banner "GLIB"

 cd ${GLIB_DIR}
 meson --prefix=${EXT_INSTALL_PATH} build
 meson install -C build
 cd ..

fi
##---------
#
cd ${EXT_DIR}
##--------- ZLIB

ZLIB_DIR="`pwd`/zlib-${ZLIB_VER}"
if [ ! -e ${EXT_INSTALL_PATH}/lib/libz.so.1.2.11 ]
then

  banner "ZLIB"

  cd ${ZLIB_DIR}
  ./configure --prefix=$EXT_INSTALL_PATH
  make all "-j${make_parallel}"
  make install
  cd ..

fi
#---------

cd ${EXT_DIR}
#--------- CURL
CURL_DIR="`pwd`/curl-${CURL_VER}"
if [ ! -e $EXT_INSTALL_PATH/lib/libcurl.so.4.6.0 ]; then

  banner "CURL"

  cd ${CURL_DIR}

  CPPFLAGS="-I${OPENSSL_DIR} -I${OPENSSL_DIR}/include" LDFLAGS="-L${OPENSSL_DIR}/lib -Wl,-rpath,${OPENSSL_DIR}/lib " LIBS="-ldl -lpthread" PKG_CONFIG_PATH=$EXT_INSTALL_PATH/lib/pkgconfig:$PKG_CONFIG_PATH ./configure --with-ssl="${OPENSSL_DIR}" --prefix=$EXT_INSTALL_PATH
  ./configure --prefix=$EXT_INSTALL_PATH --enable-versioned-symbols

  if [ "$(uname)" = "Darwin" ]; then
    #Removing api definition for Yosemite compatibility.
    sed -i '' '/#define HAVE_CLOCK_GETTIME_MONOTONIC 1/d' lib/curl_config.h
  fi

  make all "-j${make_parallel}"
  make install
  cd ..

fi
##---------

cd ${EXT_DIR}
#--------- libuv
LIBUV_DIR="`pwd`/libuv"
if [ ! -e "$EXT_INSTALL_PATH/lib/libuv.so.1.0.0" ]
then

  banner "libuv"

  cd "${LIBUV_DIR}"
  mkdir -p build
  cd build
  cmake -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH ..

  make "-j${make_parallel}"
  make install
  cd ../..
fi
##---------

cd ${EXT_DIR}
if [ ! -e $EXT_INSTALL_PATH/lib/librtCoreExt.so ]
then

 banner "RDK NativeScript"
 
 git clone https://github.com/rdkcentral/pxCore.git
 cd pxCore
 git checkout ${PXCORE_VER}
 mkdir -p build
 cd build
 cmake -DPREFER_SYSTEM_LIBRARIES=ON -DPREFER_PKGCONFIG=ON -DCMAKE_INSTALL_PREFIX=${EXT_INSTALL_PATH} -DBUILD_RTCORE_LIBS=OFF -DBUILD_PXCORE_LIBS=OFF -DBUILD_PXSCENE=OFF -DSUPPORT_NODE=OFF -DBUILD_RTCORE_EXT_LIBS=ON ..
 make
 mkdir -p ${EXT_INSTALL_PATH}/include/pxcore/
 mkdir -p ${EXT_INSTALL_PATH}/include/pxcore/unix

 cp -R ../src/*.h ${EXT_INSTALL_PATH}/include/pxcore/.
 cp -R ../src/unix/*.h ${EXT_INSTALL_PATH}/include/pxcore/unix/.

 if [ "$(uname)" == "Darwin" ]
 then
   cp mac/librtCoreExt.dylib ${EXT_INSTALL_PATH}/lib/.
 else
   cp glut/librtCoreExt.so ${EXT_INSTALL_PATH}/lib/.
 fi
 cd ../../

fi
#--------

cd ${EXT_DIR}
# uWebSockets
if [ ! -e $EXT_INSTALL_PATH/lib/libuWS.so ]
then
    banner "uWebSockets"
    
    cd uWebSockets
    EXTERNALSPATH=`pwd`/../extlibs FILEPATH=`pwd` make -f Makefile Linux
    mkdir ../extlibs/include/uwebsockets
    cp src/*.h ../extlibs/include/uwebsockets/.
    cp libuWS.so ../extlibs/lib/.
fi
#--------

cd ${EXT_DIR}
# WPEWebkit
if [ ! -e $EXT_INSTALL_PATH/lib/libJavaScriptCore.so.1.1.1 ]
then
    banner "Javascriptcore"
    git clone https://github.com/WebPlatformForEmbedded/WPEWebKit.git
    #git clone git@github.com:WebPlatformForEmbedded/WPEWebKit.git
    cd WPEWebKit
    git checkout ${WPEWEBKIT_VER}
    git apply --whitespace=fix ../jsconly_buildissues.diff 
    git apply --whitespace=fix ../es6support.diff
    rm -rf build
    mkdir build
  
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=${EXT_INSTALL_PATH} -DPORT="JSCOnly" -DUSE_CAPSTONE=OFF -DBUILD_SHARED_LIBS=OFF -DUSE_LD_GOLD=OFF -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Release -G Ninja -DDEVELOPER_MODE=ON -DENABLE_JIT=ON  -DENABLE_FTL_JIT=ON -DENABLE_REMOTE_INSPECTOR=ON ..
    cmake --build .

    cp -a lib/lib* ${EXT_INSTALL_PATH}/lib/.

    mkdir -p ${EXT_INSTALL_PATH}/include/JavaScriptCore
    mkdir -p ${EXT_INSTALL_PATH}/include/wtf
    echo "${EXT_INSTALL_PATH}/include/JavaScriptCore"
    cp -R DerivedSources/ForwardingHeaders/JavaScriptCore/*.h ${EXT_INSTALL_PATH}/include/JavaScriptCore/.
    cp -R DerivedSources/ForwardingHeaders/wtf/* ${EXT_INSTALL_PATH}/include/wtf/.
    cp -R JavaScriptCore/PrivateHeaders/JavaScriptCore/*.h ${EXT_INSTALL_PATH}/include/JavaScriptCore/.
    cp -R JavaScriptCore/Headers/JavaScriptCore/*.h ${EXT_INSTALL_PATH}/include/JavaScriptCore/.
    cp -R WTF/Headers/wtf/* ${EXT_INSTALL_PATH}/include/wtf/.
    cd ${EXT_DIR}
fi

if [ "$(uname)" != "Darwin" ]
then
    cd ${EXT_DIR}
    if [ ! -e westeros/essos/.libs/libessos.so ] ||
       [ ! -e westeros/.libs/libwesteros_compositor.so.0.0.0 ]
    then
        banner "westeros & essos"
        #rm -rf westeros
        git clone https://github.com/rdkcmf/westeros.git
        cd westeros
        git checkout 84760abb6df0135de2a25eaf655cf1385a55a0f9
        export PKG_CONFIG_PATH=/usr/lib/x86_64-linux-gnu/pkgconfig/:$PKG_CONFIG_PATH
        git apply ../westeros.diff
        cd essos
        autoreconf --install
        autoconf
        ./configure --enable-essoswesterosfree --enable-shared --prefix=$EXT_INSTALL_PATH
        make
        make install
        cd ..
        export LD_LIBRARY_PATH=`pwd`/../extlibs/lib:`pwd`/../extlibs/lib/x86_64-linux-gnu
        unset PKG_CONFIG_PATH
        make -f Makefile.ubuntu
    fi
fi

cd ${EXT_DIR}
# aamp
if [ ! -e $EXT_INSTALL_PATH/lib/libaampjsbindings.so ]
then
    banner "aamp"
    mkdir -p aamp
    cd aamp

    git clone -b dev_sprint_24_2 https://code.rdkcentral.com/r/rdk/components/generic/aamp
    git clone -b dev_sprint_24_2 https://code.rdkcentral.com/r/rdk/components/generic/aampabr
    git clone -b dev_sprint_24_2 https://code.rdkcentral.com/r/rdk/components/generic/gst-plugins-rdk-aamp
    git clone -b dev_sprint_24_2 https://code.rdkcentral.com/r/rdk/components/generic/aampmetrics
    git clone https://github.com/DaveGamble/cJSON.git
    git clone -b rdk-next "https://code.rdkcentral.com/r/rdk/components/generic/rdk-oe/meta-rdk-ext"
    
    cd aampabr
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH ..
    make
    make install
    cd ../..

    cd cJSON
    mkdir -p build
    cd build
    cmake -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH ..
    make
    make install
    cd ../..

    cd aampmetrics
    git apply ../../aampmetrics.diff
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH ..
    make
    make install
    cd ../..

    git clone https://github.com/bitmovin/libdash.git
    cd libdash/libdash
    git checkout stable_3_0
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

    cd aamp
    git apply ../../aamp.diff
    mkdir -p build
    cd build
    cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_PLATFORM_UBUNTU=1 -DCMAKE_INSTALL_PREFIX=$EXT_INSTALL_PATH -DCMAKE_WPEWEBKIT_JSBINDINGS=ON ..
    make
    make install
    cd ../../

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
#--------------------------------------------
banner ">>>>>  BUILD COMPLETE  <<<<<"
#--------------------------------------------

#-------
exit 0    #success
#-------
