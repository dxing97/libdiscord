#!/usr/bin/env bash

#libwebsockets v3.0.0

LWS_VERSION="3.0.0"

echo "Installing libwebsockets"
git clone https://github.com/warmcat/libwebsockets
cd libwebsockets
    git checkout v"$LWS_VERSION"
    mkdir build
    cd build
        cmake .. -DCMAKE_BUILD_TYPE=DEBUG -DLWS_WITH_LIBUV=ON -DLWS_WITH_LATENCY=ON -DLWS_WITHOUT_EXTENSIONS=OFF -DLWS_WITH_PLUGINS=ON -DLWS_WITH_ZLIB=ON -DLWS_WITHOUT_DAEMONIZE=ON
        make
        sudo checkinstall --pkgname libwebsockets-dev --pkgversion="$LWS_VERSION"
        sudo ldconfig
    cd ..
cd ..