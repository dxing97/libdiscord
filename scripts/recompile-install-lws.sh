#!/usr/bin/env bash

LWS_VERSION="3.1.0"

sudo apt remove libwebsockets-dev

cd build
    cmake .. -DCMAKE_BUILD_TYPE=DEBUG -DLWS_WITH_LIBUV=ON -DLWS_WITH_LATENCY=ON -DLWS_WITH_IPV6=ON -DLWS_WITHOUT_EXTENSIONS=OFF -DLWS_WITH_PLUGINS=ON -DLWS_WITH_ZLIB=ON -DLWS_WITH_UNIX_SOCK=ON -DLWS_WITH_MINIMAL_EXAMPLES=ON
    make
    sudo checkinstall --pkgname libwebsockets-dev --pkgversion="$LWS_VERSION"
    sudo ldconfig
cd ..