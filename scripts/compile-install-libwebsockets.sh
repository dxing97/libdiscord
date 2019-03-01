#!/usr/bin/env bash

#libwebsockets v3.1.0

LWS_VERSION="3.1.0"

echo "Installing libwebsockets"
git clone https://github.com/warmcat/libwebsockets
cd libwebsockets
    git checkout v"$LWS_VERSION"
    mkdir build
    cd build
        cmake .. -DCMAKE_BUILD_TYPE=DEBUG -DLWS_WITH_LIBUV=ON -DLWS_WITH_LATENCY=ON -DLWS_WITH_IPV6=ON -DLWS_WITHOUT_EXTENSIONS=OFF -DLWS_WITH_PLUGINS=ON -DLWS_WITH_ZLIB=ON -DLWS_WITH_UNIX_SOCK=ON -DLWS_WITH_MINIMAL_EXAMPLES=ON
        make
        sudo checkinstall --pkgname libwebsockets-dev --pkgversion=$LWS_VERSION
        sudo ldconfig
    cd ..
cd ..